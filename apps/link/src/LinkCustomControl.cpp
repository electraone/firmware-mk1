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

#include "LinkCustomControl.h"
#include "LookAndFeel.h"
#include "Colours.h"
#include "TextBase.h"
#include "Hardware.h"
#include "src/Link/LinkCodec.h"
#include "src/Link/LinkPrimitives.h"

LinkCustomControl::LinkCustomControl(const Control &control,
                                     MainDelegate &newDelegate,
                                     LinkGestureSink &newGestures)
    : ControlComponent(control, newDelegate),
      frame(nullptr),
      gestures(newGestures)
{
}

void LinkCustomControl::setFrame(const std::vector<uint8_t> *newFrame)
{
    frame = newFrame;
}

void LinkCustomControl::onTouchDown(const TouchEvent &touchEvent)
{
    gestures.touchGesture(control.getId(), touchEvent);
}

void LinkCustomControl::onTouchMove(const TouchEvent &touchEvent)
{
    gestures.touchGesture(control.getId(), touchEvent);
}

void LinkCustomControl::onTouchUp(const TouchEvent &touchEvent)
{
    gestures.touchGesture(control.getId(), touchEvent);
}

void LinkCustomControl::onTouchClick(const TouchEvent &touchEvent)
{
    gestures.touchGesture(control.getId(), touchEvent);
}

void LinkCustomControl::onTouchDoubleClick(const TouchEvent &touchEvent)
{
    gestures.touchGesture(control.getId(), touchEvent);
}

/* The long hold of ControlComponent opens a detail. A custom control has
 * none, on either panel, and the gesture is the script's if it wants it. */
void LinkCustomControl::onTouchLongHold(const TouchEvent &touchEvent)
{
    gestures.touchGesture(control.getId(), touchEvent);
}

void LinkCustomControl::onMidiValueChange(
    [[maybe_unused]] const ControlValue &value,
    [[maybe_unused]] int16_t midiValue,
    [[maybe_unused]] uint8_t handle)
{
}

/* The same shape CustomControl::paint has: clear, the drawing, then the
 * chrome - so a control the primary has not painted yet looks exactly like a
 * custom control on a preset with no script, which is what it is. */
void LinkCustomControl::paint(Graphics &g)
{
    g.fillAll(getUseAltBackground() ? LookAndFeel::altBackgroundColour
                                    : LookAndFeel::backgroundColour);

    if ((frame != nullptr) && !frame->empty()) {
        replay();
    }

    ControlComponent::paint(g);
}

/*
 * The op vocabulary is the Lua graphics library, one to one, and each op is
 * drawn with the call the library's binding would have made on this unit -
 * see luaGraphics.cpp - so a frame replayed here lands as the script's own
 * drawing would have.
 */
void LinkCustomControl::replay(void)
{
    Link::Reader reader(frame->data(), frame->size());
    Graphics &screen = Hardware::screen;

    while ((reader.getRemaining() > 0) && reader.isValid()) {
        uint8_t op = 0;

        if (!reader.readU8(op)) {
            return;
        }

        switch ((Link::Primitive)op) {
            case Link::Primitive::SetColour: {
                uint32_t colour = 0;

                reader.readU24(colour);
                screen.setColour(Colours888::toRGB565(colour));
                break;
            }

            case Link::Primitive::SetBackgroundColour: {
                uint32_t colour = 0;

                reader.readU24(colour);
                screen.user().setTextBackgroundColour(
                    Colours888::toRGB565(colour));
                break;
            }

            case Link::Primitive::Pixel: {
                int16_t x = 0, y = 0;

                reader.readI16(x);
                reader.readI16(y);

                if (reader.isValid()) {
                    screen.drawPixel(x, y);
                }
                break;
            }

            case Link::Primitive::Line: {
                int16_t x1 = 0, y1 = 0, x2 = 0, y2 = 0;

                reader.readI16(x1);
                reader.readI16(y1);
                reader.readI16(x2);
                reader.readI16(y2);

                if (reader.isValid()) {
                    screen.drawLine(x1, y1, x2, y2);
                }
                break;
            }

            case Link::Primitive::Rect:
            case Link::Primitive::FillRect: {
                int16_t x = 0, y = 0, width = 0, height = 0;

                reader.readI16(x);
                reader.readI16(y);
                reader.readI16(width);
                reader.readI16(height);

                if (reader.isValid()) {
                    if ((Link::Primitive)op == Link::Primitive::FillRect) {
                        screen.fillRect(x, y, width, height);
                    } else {
                        screen.drawRect(x, y, width, height);
                    }
                }
                break;
            }

            case Link::Primitive::RoundRect:
            case Link::Primitive::FillRoundRect: {
                int16_t x = 0, y = 0, width = 0, height = 0, radius = 0;

                reader.readI16(x);
                reader.readI16(y);
                reader.readI16(width);
                reader.readI16(height);
                reader.readI16(radius);

                if (reader.isValid()) {
                    if ((Link::Primitive)op
                        == Link::Primitive::FillRoundRect) {
                        screen.fillRoundRect(x, y, width, height, radius);
                    } else {
                        screen.drawRoundRect(x, y, width, height, radius);
                    }
                }
                break;
            }

            case Link::Primitive::Triangle:
            case Link::Primitive::FillTriangle: {
                int16_t x1 = 0, y1 = 0, x2 = 0, y2 = 0, x3 = 0, y3 = 0;

                reader.readI16(x1);
                reader.readI16(y1);
                reader.readI16(x2);
                reader.readI16(y2);
                reader.readI16(x3);
                reader.readI16(y3);

                if (reader.isValid()) {
                    if ((Link::Primitive)op
                        == Link::Primitive::FillTriangle) {
                        screen.fillTriangle(x1, y1, x2, y2, x3, y3);
                    } else {
                        screen.drawTriangle(x1, y1, x2, y2, x3, y3);
                    }
                }
                break;
            }

            case Link::Primitive::Circle:
            case Link::Primitive::FillCircle: {
                int16_t x = 0, y = 0, radius = 0;

                reader.readI16(x);
                reader.readI16(y);
                reader.readI16(radius);

                if (reader.isValid()) {
                    if ((Link::Primitive)op == Link::Primitive::FillCircle) {
                        screen.fillCircle(x, y, radius);
                    } else {
                        screen.drawCircle(x, y, radius);
                    }
                }
                break;
            }

            case Link::Primitive::Ellipse:
            case Link::Primitive::FillEllipse: {
                int16_t x = 0, y = 0, radiusX = 0, radiusY = 0;

                reader.readI16(x);
                reader.readI16(y);
                reader.readI16(radiusX);
                reader.readI16(radiusY);

                if (reader.isValid()) {
                    if ((Link::Primitive)op == Link::Primitive::FillEllipse) {
                        screen.fillElipse(x, y, radiusX, radiusY);
                    } else {
                        screen.drawElipse(x, y, radiusX, radiusY);
                    }
                }
                break;
            }

            case Link::Primitive::Curve: {
                int16_t x = 0, y = 0, radius = 0, segment = 0;

                reader.readI16(x);
                reader.readI16(y);
                reader.readI16(radius);
                reader.readI16(segment);

                if (reader.isValid()) {
                    screen.fillCurve(x, y, radius, segment);
                }
                break;
            }

            case Link::Primitive::Print: {
                int16_t x = 0, y = 0, width = 0;
                uint8_t align = 0, face = 0, size = 0, spacing = 0;
                char text[Link::maxStringLength + 1] = {};

                reader.readI16(x);
                reader.readI16(y);
                reader.readI16(width);
                reader.readU8(align);
                reader.readU8(face);
                reader.readU8(size);
                reader.readU8(spacing);
                reader.readString(text, sizeof(text));

                /* Checked, not cast blind: the primary validated what its
                 * Lua asked for, but these bytes crossed a MIDI cable. */
                if (reader.isValid() && isTextFace(face) && isTextSize(size)
                    && isTextSpacing(spacing) && (align <= 2)) {
                    screen.user().printText(x,
                                            y,
                                            text,
                                            width,
                                            (TextAlign)align,
                                            screen.Geometry::getColour(),
                                            (TextSize)size,
                                            (TextFace)face,
                                            (TextSpacing)spacing);
                }
                break;
            }

            default:
                /* An op this build does not know. Its size is unknown too,
                 * so there is no walking past it - the rest of the frame is
                 * given up rather than misread. */
                return;
        }
    }
}
