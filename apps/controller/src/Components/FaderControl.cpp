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
#include "FaderControl.h"
#include "ControlMetrics.h"
#include "LookAndFeel.h"

FaderControl::FaderControl(const Control &control, MainDelegate &newDelegate)
    : ControlComponent(control, newDelegate), thresholdCrossed(false)
{
    const auto &controlValue = control.getValue(0);

    setMinimum(controlValue.getMin());
    setMaximum(controlValue.getMax());
    list.assignListData(controlValue.getOverlay());
    updateValueFromParameterMap();
}

void FaderControl::syncComponentProperties(void)
{
    const auto &controlValue = control.getValue(0);
    setMinimum(controlValue.getMin());
    setMaximum(controlValue.getMax());
    list.assignListData(controlValue.getOverlay());
    setNumValues(controlValue.getNumSteps());
    ControlComponent::syncComponentProperties();
}

void FaderControl::onTouchDown(const TouchEvent &touchEvent)
{
    previousScreenX = touchEvent.getScreenX();
    thresholdCrossed = false;
}

void FaderControl::onTouchMove(const TouchEvent &touchEvent)
{
    int16_t delta = touchEvent.getScreenX() - previousScreenX;
    const auto &cv = control.getValue(0);

    if (cv.isRelative()) {
        // @todo accelleration ignored for now
        delta = ControlValue::sign(delta);
        emitValueChange(delta, cv);
    } else {
        int16_t max = value.getMax();
        int16_t min = value.getMin();
        float step = getWidth() / (float)(max - min);
        if (!thresholdCrossed) {
            if (delta < 10) {
                return;
            } else {
                thresholdCrossed = true;
                delta = 0;
            }
        }
        previousScreenX = touchEvent.getScreenX();

        int16_t newDisplayValue =
            constrain(ceil(getValue() + ((float)delta / step)), min, max);
        emitValueChange(newDisplayValue, cv);
    }
}

void FaderControl::onPotTouchDown(const PotEvent &potEvent)
{
    ControlComponent::onPotTouchDown(potEvent);
}

void FaderControl::onPotChange(const PotEvent &potEvent)
{
    if (int16_t delta = potEvent.getAcceleratedChange()) {
        const auto &cv = control.getValue(0);
        int16_t newDisplayValue = cv.isRelative()
                                      ? cv.calculateRelativeDelta(delta)
                                      : getValue() + delta;
        emitValueChange(newDisplayValue, cv);
    }
}

void FaderControl::onPotTouchUp(const PotEvent &potEvent)
{
    ControlComponent::onPotTouchUp(potEvent);
}

void FaderControl::onMidiValueChange(const ControlValue &value,
                                     int16_t midiValue,
                                     [[maybe_unused]] uint8_t handle)
{
    int16_t newDisplayValue = value.translateMidiValue(midiValue);
    setValue(newDisplayValue);
}

void FaderControl::paint(Graphics &g)
{
    Rectangle sliderBounds = getBounds();
    const ListData *items = list.getList();

    /* An overlay entry the fader has landed on may name a colour of its own.
     * getByValue answers an empty item when the value matches none, and an
     * empty item names no colour, so this falls back on the control's. */
    const uint32_t colour = items ? colourOf(items->getByValue(value.get()))
                                  : control.getColour565();

    if (control.getVariant() == Control::Variant::Dial) {
        paintDial(g,
                  sliderBounds,
                  colour,
                  value.getMin(),
                  value.getMax(),
                  value.get(),
                  items,
                  control.getMode());
    } else {
        paintFader(g,
                   sliderBounds,
                   colour,
                   value.getMin(),
                   value.getMax(),
                   value.get(),
                   items,
                   control.getMode(),
                   control.getVariant());
    }

    /* A dial on a cell with no room for both prints no name: the knob has
     * taken the whole of it, and the name would be drawn over the ring. */
    if ((control.getVariant() != Control::Variant::Dial)
        || !controlMetrics.faderDialHidesName) {
        g.user().printText(0,
                           getHeight() - controlMetrics.nameInset,
                           getName(),
                           getWidth(),
                           TextAlign::center,
                           Colours565::white,
                           TextSize::medium);
    }

    ControlComponent::paint(g);
}

void FaderControl::paintDial(Graphics &g,
                             const Rectangle &bounds,
                             uint32_t colour,
                             int16_t min,
                             int16_t max,
                             int16_t val,
                             const ListData *items,
                             Control::Mode mode)
{
    uint32_t backgroundColour = getUseAltBackground()
                                    ? LookAndFeel::altBackgroundColour
                                    : LookAndFeel::backgroundColour;

    g.fillAll(backgroundColour);

    /* Where the name has been dropped the knob takes the cell. Where it has
     * not, the ring's ink ends where every other control's graphics end, so
     * that the row has one bottom edge rather than two. */
    uint16_t dialHeight =
        controlMetrics.faderDialHidesName
            ? bounds.getHeight()
            : LookAndFeel::knobSizeForInkHeight(
                  (bounds.getHeight() > controlMetrics.graphicsBottomInset)
                      ? (uint16_t)(bounds.getHeight()
                                   - controlMetrics.graphicsBottomInset)
                      : 0,
                  std::min(bounds.getWidth(), bounds.getHeight()));

    uint16_t dialWidth = dialHeight;
    uint16_t dialX = (bounds.getWidth() - dialWidth) / 2;

    LookAndFeel::paintKnob(g,
                           dialX,
                           0,
                           dialWidth,
                           dialHeight,
                           colour,
                           min,
                           max,
                           val,
                           mode == Control::Mode::Unipolar);

    /* A dial reads its number from the hole in the middle of the ring, so the
     * knob's height is what the value is placed against. */
    paintValueFixed(
        g, bounds, colour, val, items, Control::Variant::Dial, dialHeight);
}

void FaderControl::paintFader(Graphics &g,
                              const Rectangle &bounds,
                              uint32_t colour,
                              int16_t min,
                              int16_t max,
                              int16_t val,
                              const ListData *items,
                              Control::Mode mode,
                              Control::Variant variant)
{
    uint32_t backgroundColour = getUseAltBackground()
                                    ? LookAndFeel::altBackgroundColour
                                    : LookAndFeel::backgroundColour;

    /* An overlay has a word to print and there is no room for one beside a
     * full-size track. */
    if (items && controlMetrics.faderListForcesThin) {
        variant = Control::Variant::Thin;
    }

    const bool thin = (variant == Control::Variant::Thin);
    const uint16_t barHeight = thin ? controlMetrics.faderThinBarHeight
                                    : controlMetrics.faderBarHeight;
    const uint16_t inset =
        thin ? controlMetrics.faderThinInset : controlMetrics.faderInset;

    /* Lua can set a control's bounds to anything, so a control shorter than
     * the track it is asked for puts the track at the top rather than
     * underflowing to the bottom of the screen. */
    const uint16_t trackY = (bounds.getHeight() > inset)
                                ? (uint16_t)(bounds.getHeight() - inset)
                                : 0;

    int16_t barX = 0;

    // For zero based faders, find the start position of the bar
    if (mode == Control::Mode::Bipolar) {
        barX = map(std::max((int16_t)0, min), min, max, 0, bounds.getWidth());
    }

    /* Signed: a bipolar fader below its zero has moved the end of the bar to
     * the left of where it starts. */
    const int16_t barWidth =
        (int16_t)(map(val, min, max, 0, bounds.getWidth()) - barX);

    // Clear the component area
    g.fillAll(backgroundColour);

    LookAndFeel::paintFaderTrack(
        g, bounds, colour, trackY, barHeight, barX, barWidth);

    if (!thin && controlMetrics.faderValueFloats) {
        paintValueFloating(
            g, bounds, colour, barX, barWidth, trackY, barHeight, val, items);
    } else {
        paintValueFixed(g, bounds, colour, val, items, variant, trackY);
    }
}

bool FaderControl::resolveValueText(char *text,
                                    size_t size,
                                    int16_t val,
                                    const ListDataItem &item,
                                    const ListData *items,
                                    bool padNegative) const
{
    if (items && !item.isBitmapEmpty()) {
        return (false);
    }

    if (items && !item.isLabelEmpty()) {
        copyString(text, item.getLabel(), size - 1);
        return (true);
    }

    if (control.getValue(0).isLabelSet()) {
        copyString(text, control.getValue(0).getLabel(), size - 1);
        return (true);
    }

    /* The trailing space on a negative number is what keeps a minus sign from
     * pulling a centred number off centre. A right aligned one wants no such
     * thing. */
    snprintf(text, size, (padNegative && (val < 0)) ? "%d " : "%d", val);

    return (true);
}

void FaderControl::paintValueFixed(Graphics &g,
                                   const Rectangle &bounds,
                                   uint32_t colour,
                                   int16_t val,
                                   const ListData *items,
                                   Control::Variant variant,
                                   uint16_t graphicsTop)
{
    /* Standing the value a gap above what is under it, so a preset asking for
     * a bigger face gets it without the number landing on the graphics. */
    uint16_t labelYPosition = valueLabelY(g, graphicsTop);

    if (variant == Control::Variant::Dial) {
        /* A dial's value goes in the hole in the middle of the ring, and it is
         * the digits that are centred there rather than the line box they sit
         * in - centring the box leaves the number sitting high. */
        labelYPosition =
            valueLabelYCentredOn(g, LookAndFeel::knobCentre(graphicsTop));
    }

    /* Looked up once. getByValue() walks the list to find the entry, so asking
     * it the same question twice searches it twice. */
    ListDataItem item = items ? items->getByValue(val) : ListDataItem();

    char stringValue[ControlValue::MaxLabelLength + 1];

    if (!resolveValueText(
            stringValue, sizeof(stringValue), val, item, items, true)) {
        uint16_t paddingBitmap = ((bounds.getWidth() - BITMAP_WIDTH)) / 2 - 1;
        item.paintBitmap(paddingBitmap, 0, colour);
        return;
    }

    printValue(g,
               0,
               labelYPosition,
               stringValue,
               bounds.getWidth(),
               TextAlign::center,
               Colours565::white);
}

void FaderControl::paintValueFloating(Graphics &g,
                                      const Rectangle &bounds,
                                      uint32_t colour,
                                      int16_t barX,
                                      int16_t barWidth,
                                      uint16_t trackY,
                                      uint16_t trackHeight,
                                      int16_t val,
                                      const ListData *items)
{
    ListDataItem item = items ? items->getByValue(val) : ListDataItem();

    char stringValue[ControlValue::MaxLabelLength + 1];

    if (!resolveValueText(
            stringValue, sizeof(stringValue), val, item, items, false)) {
        uint16_t paddingBitmap = ((bounds.getWidth() - BITMAP_WIDTH)) / 2 - 1;
        item.paintBitmap(paddingBitmap, 0, colour);
        return;
    }

    /* Centred on the bar by the digits' ink rather than set at a fixed row, so
     * a preset's bigger or smaller face still sits in the middle of it. */
    const Control::Font font = getValueFont();
    const uint16_t centreY = trackY + (trackHeight / 2);
    const uint16_t y = valueLabelYCentredOn(g, centreY);

    /* The whole number used to turn black once the end of the bar had passed
     * under any of it. It splits where the bar ends now, each half in the
     * colour that reads on what is under it. */
    const uint32_t track = Colours565::darker(colour, 0.3f);
    const int16_t barEnd = (int16_t)(barX + barWidth);
    const int16_t from = (barX < barEnd) ? barX : barEnd;
    const int16_t to = (barX < barEnd) ? barEnd : barX;

    g.user().printTextSplit(
        0,
        y,
        stringValue,
        (bounds.getWidth() > textPositionFromRight)
            ? (uint16_t)(bounds.getWidth() - textPositionFromRight)
            : 0,
        TextAlign::right,
        (uint16_t)std::max((int16_t)0, from),
        (uint16_t)std::max((int16_t)0, to),
        Colours565::textColourOn(colour),
        colour,
        Colours565::textColourOn(track),
        track,
        font.size,
        font.face,
        font.spacing);
}
