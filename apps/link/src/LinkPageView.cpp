/*
* Electra One MIDI Controller Firmware
* See COPYRIGHT file at the top of the source tree.
*
* This product includes software developed by the
* Electra One Project (http://electra.one/).
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.
*/

#include "LinkPageView.h"
#include "LinkCustomControl.h"
#include "Window.h"
#include "Colours.h"
#include "Base64.h"
#include "ParameterMap.h"
#include "System.h"

namespace
{
    /**
     * @brief What the controls sit on: the page's black ground.
     *
     * The same bounds the controller's PageView uses, so a preset laid out
     * for one panel lands at the same pixels on the other.
     */
    class LinkPageFrame : public Component
    {
    public:
        LinkPageFrame()
        {
            setName("linkPage");
        }

        void paint(Graphics &g) override
        {
            g.fillAll(Colours565::black);
        }
    };
} // namespace

LinkPageView::LinkPageView(MainDelegate &newDelegate,
                           LinkGestureSink &newGestures)
    : delegate(newDelegate),
      gestures(newGestures),
      preset(),
      frame(nullptr),
      bottomBar(nullptr),
      count(0),
      nextParameterNumber(1),
      builtStructureHash(0)
{
}

size_t LinkPageView::getCount(void) const
{
    return (count);
}

void LinkPageView::clear(void)
{
    if (frame != nullptr) {
        /* The primary sent a new layout while a finger may be on one of
         * these controls. */
        if (Window *window = frame->getWindow()) {
            window->resetActiveTouch();
        }
        frame->deleteAllChildren();
    }

    count = 0;
    customComponents.clear();
    customFrames.clear();
    builtStructureHash = 0;

    /* The map's entries point at values in the preset's pools, so the map
     * lets go of them first. Numbers are not reused within a run: a value
     * still on its way for a control that has gone would otherwise land on
     * whichever control inherited its key. */
    parameterMap.clear();
    preset.reset();
}

uint32_t LinkPageView::structureHash(const Link::View &view)
{
    uint32_t hash = 2166136261u;

    auto mix = [&hash](const void *data, size_t length) {
        const uint8_t *bytes = (const uint8_t *)data;

        for (size_t i = 0; i < length; i++) {
            hash = (hash ^ bytes[i]) * 16777619u;
        }
    };
    auto mixString = [&mix](const std::string &text) {
        mix(text.data(), text.size());
        mix("\x1f", 1);
    };

    const uint8_t presetId = view.getPresetId();
    const uint8_t pageNumber = view.getPageNumber();

    mix(&presetId, sizeof(presetId));
    mix(&pageNumber, sizeof(pageNumber));
    mixString(view.getTitle());

    for (const Link::ViewPage &page : view.getPages()) {
        mix(&page.id, sizeof(page.id));
        mixString(page.name);
    }

    for (const Link::ViewOverlay &overlay : view.getOverlays()) {
        mix(&overlay.id, sizeof(overlay.id));

        for (const Link::OverlayItem &item : overlay.items) {
            mix(&item.value, sizeof(item.value));
            mix(&item.colour, sizeof(item.colour));
            mix(&item.colourIsSet, sizeof(item.colourIsSet));
            mixString(item.label);
            mix(item.bitmap.data(), item.bitmap.size());
        }
    }

    for (const Link::ViewElement &element : view.getElements()) {
        mix(&element.id, sizeof(element.id));
        mix(&element.nativeType, sizeof(element.nativeType));
        mix(&element.nativeVariant, sizeof(element.nativeVariant));
        mix(&element.nativeMode, sizeof(element.nativeMode));
        mix(&element.visible, sizeof(element.visible));
        mix(&element.active, sizeof(element.active));
        mix(&element.bounds, sizeof(element.bounds));
        mix(&element.colour, sizeof(element.colour));
        mix(&element.potId, sizeof(element.potId));
        mix(&element.potMask, sizeof(element.potMask));
        mixString(element.name);

        for (const Link::ViewValue &value : element.values) {
            mix(&value.handle, sizeof(value.handle));
            mix(&value.overlayId, sizeof(value.overlayId));
            mix(&value.min, sizeof(value.min));
            mix(&value.max, sizeof(value.max));
        }
    }

    return (hash);
}

void LinkPageView::refreshValues(const Link::View &view)
{
    for (const Link::ViewElement &element : view.getElements()) {
        for (const Link::ViewValue &value : element.values) {
            updateValue(element.id,
                        value.handle,
                        (int16_t)value.current,
                        value.label.empty() ? nullptr : value.label.c_str());
        }
    }
}

bool LinkPageView::rebuild(const Link::View &view, Component &parent)
{
    const uint32_t hash = structureHash(view);

    if ((frame != nullptr) && (count > 0) && (hash == builtStructureHash)) {
        refreshValues(view);

        for (const auto &[elementId, bytes] : view.getCustomFrames()) {
            applyCustomFrame(elementId, bytes.data(), bytes.size());
        }

        return (false);
    }

    if (frame == nullptr) {
        frame = new LinkPageFrame();
        frame->setBounds(0, 0, 1024, 575);
        frame->setVisible(true);
        parent.addChildComponent(frame);
    }

    if (bottomBar == nullptr) {
        bottomBar = new BottomBar("", "");
        bottomBar->setBounds(12, 543, 1000, 22);
        bottomBar->setVisible(true);
        parent.addChildComponent(bottomBar);
    }

    clear();

    /* Before the controls: a value binds to an overlay by pointer, and the
     * overlay has to exist and keep its address for as long as the control
     * does. The preset's overlays are a map, so its entries are stable. */
    for (const Link::ViewOverlay &overlay : view.getOverlays()) {
        Overlay &target = preset.addOverlay(overlay.id);

        for (const Link::OverlayItem &item : overlay.items) {
            /* An entry with a picture goes through the same door a preset
             * document's does: the item takes the bitmap base64-encoded and
             * saves it into this unit's own bitmap pool. */
            char encodedBitmap[((Link::maxOverlayBitmapBytes + 2) / 3) * 4
                               + 4] = {};

            if (!item.bitmap.empty()
                && (item.bitmap.size() <= Link::maxOverlayBitmapBytes)) {
                base64_encode(encodedBitmap,
                              (char *)(uintptr_t)item.bitmap.data(),
                              (int)item.bitmap.size());
            }

            target.addItem(item.value,
                           item.label.c_str(),
                           (encodedBitmap[0] != '\0') ? encodedBitmap
                                                      : nullptr,
                           Colours888::toRGB565(item.colour),
                           item.colourIsSet);
        }
    }

    for (const Link::ViewElement &element : view.getElements()) {
        addElement(element, *frame);
    }

    /* Frames that arrived while the view was still assembling waited inside
     * it; the components exist now, so they can be drawn. */
    for (const auto &[elementId, bytes] : view.getCustomFrames()) {
        applyCustomFrame(elementId, bytes.data(), bytes.size());
    }

    const char *pageName = "";

    for (const Link::ViewPage &page : view.getPages()) {
        if (page.id == view.getPageNumber()) {
            pageName = page.name.c_str();
            break;
        }
    }

    bottomBar->update(view.getTitle().c_str(), pageName);

    builtStructureHash = hash;

    parent.repaint();

    return (true);
}

void LinkPageView::repaintBottomBar(void)
{
    if (bottomBar != nullptr) {
        bottomBar->repaint();
    }
}

void LinkPageView::setInfoText(const char *text)
{
    if (bottomBar == nullptr) {
        return;
    }

    bottomBar->setInfoText(text);
    bottomBar->repaint();
}

Overlay *LinkPageView::findOverlay(uint8_t overlayId)
{
    if (overlayId == 0) {
        return (nullptr);
    }

    return (preset.getOverlay(overlayId));
}

uint16_t LinkPageView::midiFor(const Control &control,
                               const ControlValue &value,
                               int16_t current)
{
    switch (control.getType()) {
        case Control::Type::List:
            /* The item's value, which is what ListControl indexes the overlay
             * by. Negative values round-trip through the unsigned store. */
            return ((uint16_t)current);

        case Control::Type::Pad:
            return ((current != 0) ? 1 : 0);

        default: {
            int32_t offset = (int32_t)current - (int32_t)value.getMin();
            const int32_t range =
                (int32_t)value.getMax() - (int32_t)value.getMin();

            if (offset < 0) {
                offset = 0;
            } else if (offset > range) {
                offset = range;
            }

            return ((uint16_t)offset);
        }
    }
}

void LinkPageView::applyCustomFrame(Link::ElementId elementId,
                                    const uint8_t *bytes,
                                    size_t length)
{
    auto entry = customComponents.find(elementId);

    if (entry == customComponents.end()) {
        return;
    }

    customFrames[elementId].assign(bytes, bytes + length);
    entry->second->repaint();
}

void LinkPageView::highlightElement(Link::ElementId elementId, bool on)
{
    Control *control = preset.controls.find(elementId);

    if (control == nullptr) {
        return;
    }

    if (ControlComponent *cc =
            dynamic_cast<ControlComponent *>(control->getComponent())) {
        cc->setActive(on);
    }
}

#if defined(LINK_BENCH) && LINK_BENCH
/* Read over J-Link: how many value updates reached this, and how many found
 * no control or no component to land on. */
volatile uint32_t linkBenchUpdates = 0;
volatile uint32_t linkBenchUpdateMisses = 0;
volatile uint32_t linkBenchUpdateNoHandle = 0;
#endif

void LinkPageView::updateValue(Link::ElementId elementId,
                               uint8_t handle,
                               int16_t current,
                               const char *label)
{
#if defined(LINK_BENCH) && LINK_BENCH
    linkBenchUpdates++;
#endif
    Control *control = preset.controls.find(elementId);

    if (control == nullptr) {
#if defined(LINK_BENCH) && LINK_BENCH
        linkBenchUpdateMisses++;
#endif
        return;
    }

    ControlComponent *cc =
        dynamic_cast<ControlComponent *>(control->getComponent());

    if (cc == nullptr) {
#if defined(LINK_BENCH) && LINK_BENCH
        linkBenchUpdateMisses++;
#endif
        return;
    }

    for (ControlValue &value : control->values) {
        if (value.getHandle() != (int16_t)handle) {
            continue;
        }

        const uint16_t midiValue = midiFor(*control, value, current);
        const Message &message = value.message;

        /* Into the store without follow-ups: the change is the primary's,
         * already sent and already applied there. */
        parameterMap.setValueSimple(message.getDeviceId(),
                                    message.getType(),
                                    message.getParameterNumber(),
                                    midiValue);

        if ((label != nullptr) && (label[0] != '\0')) {
            value.setLabel(label);
        } else {
            value.resetLabel();
        }

        /* The controller's own path from a changed value to a repainted
         * control, called directly rather than found by the repaint pass. */
        cc->onMidiValueChange(value, midiValue, handle);
        return;
    }

#if defined(LINK_BENCH) && LINK_BENCH
    linkBenchUpdateNoHandle++;
#endif
}

void LinkPageView::addElement(const Link::ViewElement &element,
                              Component &parent)
{
    const Rectangle bounds(element.bounds.x,
                           element.bounds.y,
                           element.bounds.width,
                           element.bounds.height);

    /* The native fields, which is what this program exists to use: the
     * control as the controller holds it, not a shape resembling one. */
    Control control(element.id,
                    1,
                    element.name.c_str(),
                    bounds,
                    (Control::Type)element.nativeType,
                    (Control::Mode)element.nativeMode,
                    element.colour,
                    0,
                    (Control::Variant)element.nativeVariant,
                    element.visible);

    std::vector<ControlValue> values;
    std::vector<Input> inputs;

    for (size_t i = 0; i < element.values.size(); i++) {
        const Link::ViewValue &value = element.values[i];
        const int32_t range = (int32_t)value.max - (int32_t)value.min;
        uint16_t midiMax = (range > 0) ? (uint16_t)range : 0;

        if (control.getType() == Control::Type::List) {
            midiMax = 16383;
        } else if (control.getType() == Control::Type::Pad) {
            midiMax = 1;
        }

        /* A virt message: nothing leaves this unit, and the parameter number
         * is only a key into this program's own map. */
        Message message(1,
                        Message::Type::virt,
                        nextParameterNumber++,
                        0,
                        midiMax,
                        0,
                        nullptr,
                        false,
                        false,
                        SignMode::noSign,
                        (midiMax > 127) ? 14 : 7,
                        false,
                        RelativeMode::noSign,
                        false);

        /* The handle as the type spells it - "attack", "value" - and the
         * ordinal for a custom control, whose values are numbered. The
         * control pointer is the one the constructor consults for that
         * distinction; the preset re-points the value at the stored copy. */
        const char *valueId = control.translateValueId(value.handle);

        values.emplace_back(&control,
                            (valueId != nullptr) ? valueId : "value",
                            (uint8_t)i,
                            (int16_t)value.current,
                            (int16_t)value.min,
                            (int16_t)value.max,
                            value.overlayId,
                            message,
                            0,
                            0,
                            findOverlay(value.overlayId));
    }

    /* The pot the control sits on, so the surface can find it by knob. */
    if (element.potId < NR_OF_HW_POTS) {
        const uint8_t valueId =
            element.values.empty() ? 0 : element.values[0].handle;

        inputs.push_back(Input(valueId, element.potId));
    }

    Control &stored = preset.addControl(control, values, inputs);

    if (!stored.isValid()) {
        System::logger.write(LOG_ERROR,
                             "LinkPageView: no room for control %d",
                             element.id);
        return;
    }

    for (size_t i = 0; i < stored.values.size(); i++) {
        ControlValue &value = stored.values[i];
        const Link::ViewValue &sent = element.values[i];

        /* Registers the value's lookup entry in the map. Without it a value
         * is written to a key nothing reads back, and the control draws zero
         * however much the primary sends. */
        stored.addToParameterMap(value);

        parameterMap.setValueSimple(
            value.message.getDeviceId(),
            value.message.getType(),
            value.message.getParameterNumber(),
            midiFor(stored, value, (int16_t)sent.current));

        /* What the value reads as, when it is not a number. The primary
         * sends the text because it is the only end that can produce it. */
        if (!sent.label.empty()) {
            value.setLabel(sent.label.c_str());
        }
    }

    ControlComponent *component = nullptr;

    if (stored.getType() == Control::Type::Custom) {
        LinkCustomControl *custom =
            new LinkCustomControl(stored, delegate, gestures);

        custom->syncComponentProperties();
        custom->setFrame(&customFrames[element.id]);
        customComponents[element.id] = custom;
        component = custom;
    } else {
        component = ControlComponent::createControlComponent(stored, delegate);
    }

    if (component == nullptr) {
        return;
    }

    /*
     * The knob belongs to this program, not to the control. A ControlComponent
     * is a PotListener, and one listening would move its own value when the
     * knob turned, behind the primary's back. The turn goes to the primary and
     * comes back as a value, so nothing here has any business listening.
     */
    component->releasePot();
    component->setDimmed(!element.active);
    parent.addChildComponent(component);
    stored.setComponent(component);

    /* The values as sent, through the same path a later change takes. */
    component->updateValueFromParameterMap();

    count++;
}
