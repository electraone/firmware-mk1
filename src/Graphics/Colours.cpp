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

/**
 * @file Colours.cpp
 *
 * @brief Colour dfinitions and colour management.
 */

#include "Colours.h"

#include <cmath>
#include <string.h>
#include "RA8876.h"

namespace Colours565
{
    const uint32_t aliceblue{ 0xF7DF };
    const uint32_t antiquewhite{ 0xFF5A };
    const uint32_t aqua{ 0x07FF };
    const uint32_t aquamarine{ 0x7FFA };
    const uint32_t azure{ 0xF7FF };
    const uint32_t beige{ 0xF7BB };
    const uint32_t bisque{ 0xFF38 };
    const uint32_t black{ 0x0000 };
    const uint32_t blanchedalmond{ 0xFF59 };
    const uint32_t blue{ 0x001F };
    const uint32_t blueviolet{ 0x895C };
    const uint32_t brown{ 0xA145 };
    const uint32_t burlywood{ 0xDDD0 };
    const uint32_t cadetblue{ 0x5CF4 };
    const uint32_t chartreuse{ 0x7FE0 };
    const uint32_t chocolate{ 0xD343 };
    const uint32_t coral{ 0xFBEA };
    const uint32_t cornflowerblue{ 0x64BD };
    const uint32_t cornsilk{ 0xFFDB };
    const uint32_t crimson{ 0xD8A7 };
    const uint32_t cyan{ 0x07FF };
    const uint32_t darkblue{ 0x0011 };
    const uint32_t darkcyan{ 0x0451 };
    const uint32_t darkgoldenrod{ 0xBC21 };
    const uint32_t darkgrey{ 0x52AA };
    const uint32_t darkgreen{ 0x0320 };
    const uint32_t darkkhaki{ 0xBDAD };
    const uint32_t darkmagenta{ 0x8811 };
    const uint32_t darkolivegreen{ 0x5345 };
    const uint32_t darkorange{ 0xFC60 };
    const uint32_t darkorchid{ 0x9999 };
    const uint32_t darkred{ 0x8800 };
    const uint32_t darksalmon{ 0xECAF };
    const uint32_t darkseagreen{ 0x8DF1 };
    const uint32_t darkslateblue{ 0x49F1 };
    const uint32_t darkslategrey{ 0x2A69 };
    const uint32_t darkturquoise{ 0x067A };
    const uint32_t darkviolet{ 0x901A };
    const uint32_t deeppink{ 0xF8B2 };
    const uint32_t deepskyblue{ 0x05FF };
    const uint32_t dimgrey{ 0x6B4D };
    const uint32_t dodgerblue{ 0x1C9F };
    const uint32_t firebrick{ 0xB104 };
    const uint32_t floralwhite{ 0xFFDE };
    const uint32_t forestgreen{ 0x2444 };
    const uint32_t fuchsia{ 0xF81F };
    const uint32_t gainsboro{ 0xDEFB };
    const uint32_t ghostwhite{ 0xFFDF };
    const uint32_t gold{ 0xFEA0 };
    const uint32_t goldenrod{ 0xDD24 };
    const uint32_t grey{ 0x8410 };
    const uint32_t green{ 0x0400 };
    const uint32_t greenyellow{ 0xAFE5 };
    const uint32_t honeydew{ 0xF7FE };
    const uint32_t hotpink{ 0xFB56 };
    const uint32_t indianred{ 0xCAEB };
    const uint32_t indigo{ 0x4810 };
    const uint32_t ivory{ 0xFFFE };
    const uint32_t khaki{ 0xF731 };
    const uint32_t lavender{ 0xE73F };
    const uint32_t lavenderblush{ 0xFF9E };
    const uint32_t lawngreen{ 0x7FE0 };
    const uint32_t lemonchiffon{ 0xFFD9 };
    const uint32_t lightblue{ 0xAEDC };
    const uint32_t lightcoral{ 0xF410 };
    const uint32_t lightcyan{ 0xE7FF };
    const uint32_t lightgoldenrodyellow{ 0xFFDA };
    const uint32_t lightgreen{ 0x9772 };
    const uint32_t lightgrey{ 0xD69A };
    const uint32_t lightpink{ 0xFDB8 };
    const uint32_t lightsalmon{ 0xFD0F };
    const uint32_t lightseagreen{ 0x2595 };
    const uint32_t lightskyblue{ 0x867F };
    const uint32_t lightslategrey{ 0x7453 };
    const uint32_t lightsteelblue{ 0xB63B };
    const uint32_t lightyellow{ 0xFFFC };
    const uint32_t lime{ 0x07E0 };
    const uint32_t limegreen{ 0x3666 };
    const uint32_t linen{ 0xFF9C };
    const uint32_t magenta{ 0xF81F };
    const uint32_t maroon{ 0x8000 };
    const uint32_t mediumaquamarine{ 0x6675 };
    const uint32_t mediumblue{ 0x0019 };
    const uint32_t mediumorchid{ 0xBABA };
    const uint32_t mediumpurple{ 0x939B };
    const uint32_t mediumseagreen{ 0x3D8E };
    const uint32_t mediumslateblue{ 0x7B5D };
    const uint32_t mediumspringgreen{ 0x07D3 };
    const uint32_t mediumturquoise{ 0x4E99 };
    const uint32_t mediumvioletred{ 0xC0B0 };
    const uint32_t midnightblue{ 0x18CE };
    const uint32_t mintcream{ 0xF7FF };
    const uint32_t mistyrose{ 0xFF3C };
    const uint32_t moccasin{ 0xFF36 };
    const uint32_t navajowhite{ 0xFEF5 };
    const uint32_t navy{ 0x0010 };
    const uint32_t oldlace{ 0xFFBC };
    const uint32_t olive{ 0x8400 };
    const uint32_t olivedrab{ 0x6C64 };
    const uint32_t orange{ 0xFD20 };
    const uint32_t orangered{ 0xFA20 };
    const uint32_t orchid{ 0xDB9A };
    const uint32_t palegoldenrod{ 0xEF55 };
    const uint32_t palegreen{ 0x9FD3 };
    const uint32_t paleturquoise{ 0xAF7D };
    const uint32_t palevioletred{ 0xDB92 };
    const uint32_t papayawhip{ 0xFF7A };
    const uint32_t peachpuff{ 0xFED7 };
    const uint32_t peru{ 0xCC27 };
    const uint32_t pink{ 0xFE19 };
    const uint32_t plum{ 0xDD1B };
    const uint32_t powderblue{ 0xB71C };
    const uint32_t purple{ 0x8010 };
    const uint32_t rebeccapurple{ 0x6193 };
    const uint32_t red{ 0xF800 };
    const uint32_t rosybrown{ 0xBC71 };
    const uint32_t royalblue{ 0x435C };
    const uint32_t saddlebrown{ 0x8A22 };
    const uint32_t salmon{ 0xFC0E };
    const uint32_t sandybrown{ 0xF52C };
    const uint32_t seagreen{ 0x2C4A };
    const uint32_t seashell{ 0xFFBD };
    const uint32_t sienna{ 0xA285 };
    const uint32_t silver{ 0xC618 };
    const uint32_t skyblue{ 0x867D };
    const uint32_t slateblue{ 0x6AD9 };
    const uint32_t slategrey{ 0x7412 };
    const uint32_t snow{ 0xFFDF };
    const uint32_t springgreen{ 0x07EF };
    const uint32_t steelblue{ 0x4416 };
    const uint32_t tan{ 0xD5B1 };
    const uint32_t teal{ 0x0410 };
    const uint32_t thistle{ 0xDDFB };
    const uint32_t tomato{ 0xFB08 };
    const uint32_t turquoise{ 0x471A };
    const uint32_t violet{ 0xEC1D };
    const uint32_t wheat{ 0xF6F6 };
    const uint32_t white{ 0xFFFF };
    const uint32_t whitesmoke{ 0xF7BE };
    const uint32_t yellow{ 0xFFE0 };
    const uint32_t yellowgreen{ 0x9E66 };

    uint32_t fromString(const char *stringNumber)
    {
        return strtol(stringNumber, 0, 16);
    }

    uint32_t toRGB888(uint32_t rgb565)
    {
        uint8_t r = rgb565 >> 11;
        uint8_t g = (rgb565 >> 5) & 0x3F;
        uint8_t b = rgb565 & 0x1F;

        uint8_t r8 = (r * 527 + 23) >> 6;
        uint8_t g8 = (g * 259 + 33) >> 6;
        uint8_t b8 = (b * 527 + 23) >> 6;

        return ((r8 << 16) | (g8 << 8) | b8);
    }

    // Get colour with adjusted brightness
    uint32_t darker(uint32_t originalColour, float factor)
    {
        uint16_t red = (originalColour >> 11) & 0x1f;
        uint16_t green = (originalColour >> 5) & 0x3f;
        uint16_t blue = originalColour & 0x1f;
        uint16_t newRed = (uint16_t)(red * factor) & 0x1f;
        uint16_t newGreen = (uint16_t)(green * factor) & 0x3f;
        uint16_t newBlue = (uint16_t)(blue * factor) & 0x1f;
        return ((newRed << 11) | (newGreen << 5) | newBlue);
    }

    uint32_t lighter(uint32_t originalColour, float factor)
    {
        uint16_t red = (originalColour >> 11) & 0x1f;
        uint16_t green = (originalColour >> 5) & 0x3f;
        uint16_t blue = originalColour & 0x1f;
        uint16_t newRed = (uint16_t)(red + (0x1f - red) * factor) & 0x1f;
        uint16_t newGreen = (uint16_t)(green + (0x3f - green) * factor) & 0x3f;
        uint16_t newBlue = (uint16_t)(blue + (0x1f - blue) * factor) & 0x1f;
        return ((newRed << 11) | (newGreen << 5) | newBlue);
    }
    uint32_t mix(uint32_t colour, uint32_t other, float amount)
    {
        const auto channel = [amount](uint32_t a, uint32_t b) {
            return (
                (uint32_t)((float)b + ((float)a - (float)b) * amount + 0.5f));
        };

        const uint32_t red =
            channel((colour >> 11) & 0x1f, (other >> 11) & 0x1f) & 0x1f;
        const uint32_t green =
            channel((colour >> 5) & 0x3f, (other >> 5) & 0x3f) & 0x3f;
        const uint32_t blue = channel(colour & 0x1f, other & 0x1f) & 0x1f;

        return ((red << 11) | (green << 5) | blue);
    }

    /*
     * Relative luminance as WCAG 2.x defines it - each channel expanded to
     * eight bits, linearised through the sRGB curve, and weighted by how
     * bright it looks, green ten times blue.
     *
     * The linear values come from two small tables built on first use, so a
     * paint pays three lookups rather than three pow() calls.
     */
    float relativeLuminance(uint16_t colour)
    {
        static float fiveBit[32];
        static float sixBit[64];
        static bool built = false;

        if (!built) {
            auto linear = [](float c) {
                return ((c <= 0.04045f) ? (c / 12.92f)
                                        : powf((c + 0.055f) / 1.055f, 2.4f));
            };

            for (uint8_t v = 0; v < 32; v++) {
                fiveBit[v] = linear((float)((v << 3) | (v >> 2)) / 255.0f);
            }
            for (uint8_t v = 0; v < 64; v++) {
                sixBit[v] = linear((float)((v << 2) | (v >> 4)) / 255.0f);
            }
            built = true;
        }

        return ((0.2126f * fiveBit[(colour >> 11) & 0x1F])
                + (0.7152f * sixBit[(colour >> 5) & 0x3F])
                + (0.0722f * fiveBit[colour & 0x1F]));
    }

    /*
     * Black or white, whichever has the higher WCAG contrast ratio against the
     * background. The two ratios are equal at this luminance.
     */
    uint32_t textColourOn(uint16_t background)
    {
        constexpr float equalContrast = 0.1791f;

        return ((relativeLuminance(background) > equalContrast) ? black
                                                                : white);
    }
} // namespace Colours565

namespace Colours888
{
    const uint32_t aliceblue{ 0x00f0f8ff };
    const uint32_t antiquewhite{ 0x00faebd7 };
    const uint32_t aqua{ 0x0000ffff };
    const uint32_t aquamarine{ 0x007fffd4 };
    const uint32_t azure{ 0x00f0ffff };
    const uint32_t beige{ 0x00f5f5dc };
    const uint32_t bisque{ 0x00ffe4c4 };
    const uint32_t black{ 0x00000000 };
    const uint32_t blanchedalmond{ 0x00ffebcd };
    const uint32_t blue{ 0x000000ff };
    const uint32_t blueviolet{ 0x008a2be2 };
    const uint32_t brown{ 0x00a52a2a };
    const uint32_t burlywood{ 0x00deb887 };
    const uint32_t cadetblue{ 0x005f9ea0 };
    const uint32_t chartreuse{ 0x007fff00 };
    const uint32_t chocolate{ 0x00d2691e };
    const uint32_t coral{ 0x00ff7f50 };
    const uint32_t cornflowerblue{ 0x006495ed };
    const uint32_t cornsilk{ 0x00fff8dc };
    const uint32_t crimson{ 0x00dc143c };
    const uint32_t cyan{ 0x0000ffff };
    const uint32_t darkblue{ 0x0000008b };
    const uint32_t darkcyan{ 0x00008b8b };
    const uint32_t darkgoldenrod{ 0x00b8860b };
    const uint32_t darkgrey{ 0x00555555 };
    const uint32_t darkgreen{ 0x00006400 };
    const uint32_t darkkhaki{ 0x00bdb76b };
    const uint32_t darkmagenta{ 0x008b008b };
    const uint32_t darkolivegreen{ 0x00556b2f };
    const uint32_t darkorange{ 0x00ff8c00 };
    const uint32_t darkorchid{ 0x009932cc };
    const uint32_t darkred{ 0x008b0000 };
    const uint32_t darksalmon{ 0x00e9967a };
    const uint32_t darkseagreen{ 0x008fbc8f };
    const uint32_t darkslateblue{ 0x00483d8b };
    const uint32_t darkslategrey{ 0x002f4f4f };
    const uint32_t darkturquoise{ 0x0000ced1 };
    const uint32_t darkviolet{ 0x009400d3 };
    const uint32_t deeppink{ 0x00ff1493 };
    const uint32_t deepskyblue{ 0x0000bfff };
    const uint32_t dimgrey{ 0x00696969 };
    const uint32_t dodgerblue{ 0x001e90ff };
    const uint32_t firebrick{ 0x00b22222 };
    const uint32_t floralwhite{ 0x00fffaf0 };
    const uint32_t forestgreen{ 0x00228b22 };
    const uint32_t fuchsia{ 0x00ff00ff };
    const uint32_t gainsboro{ 0x00dcdcdc };
    const uint32_t ghostwhite{ 0x00f8f8ff };
    const uint32_t gold{ 0x00ffd700 };
    const uint32_t goldenrod{ 0x00daa520 };
    const uint32_t grey{ 0x00808080 };
    const uint32_t green{ 0x00008000 };
    const uint32_t greenyellow{ 0x00adff2f };
    const uint32_t honeydew{ 0x00f0fff0 };
    const uint32_t hotpink{ 0x00ff69b4 };
    const uint32_t indianred{ 0x00cd5c5c };
    const uint32_t indigo{ 0x004b0082 };
    const uint32_t ivory{ 0x00fffff0 };
    const uint32_t khaki{ 0x00f0e68c };
    const uint32_t lavender{ 0x00e6e6fa };
    const uint32_t lavenderblush{ 0x00fff0f5 };
    const uint32_t lawngreen{ 0x007cfc00 };
    const uint32_t lemonchiffon{ 0x00fffacd };
    const uint32_t lightblue{ 0x00add8e6 };
    const uint32_t lightcoral{ 0x00f08080 };
    const uint32_t lightcyan{ 0x00e0ffff };
    const uint32_t lightgoldenrodyellow{ 0x00fafad2 };
    const uint32_t lightgreen{ 0x0090ee90 };
    const uint32_t lightgrey{ 0x00d3d3d3 };
    const uint32_t lightpink{ 0x00ffb6c1 };
    const uint32_t lightsalmon{ 0x00ffa07a };
    const uint32_t lightseagreen{ 0x0020b2aa };
    const uint32_t lightskyblue{ 0x0087cefa };
    const uint32_t lightslategrey{ 0x00778899 };
    const uint32_t lightsteelblue{ 0x00b0c4de };
    const uint32_t lightyellow{ 0x00ffffe0 };
    const uint32_t lime{ 0x0000ff00 };
    const uint32_t limegreen{ 0x0032cd32 };
    const uint32_t linen{ 0x00faf0e6 };
    const uint32_t magenta{ 0x00ff00ff };
    const uint32_t maroon{ 0x00800000 };
    const uint32_t mediumaquamarine{ 0x0066cdaa };
    const uint32_t mediumblue{ 0x000000cd };
    const uint32_t mediumorchid{ 0x00ba55d3 };
    const uint32_t mediumpurple{ 0x009370db };
    const uint32_t mediumseagreen{ 0x003cb371 };
    const uint32_t mediumslateblue{ 0x007b68ee };
    const uint32_t mediumspringgreen{ 0x0000fa9a };
    const uint32_t mediumturquoise{ 0x0048d1cc };
    const uint32_t mediumvioletred{ 0x00c71585 };
    const uint32_t midnightblue{ 0x00191970 };
    const uint32_t mintcream{ 0x00f5fffa };
    const uint32_t mistyrose{ 0x00ffe4e1 };
    const uint32_t moccasin{ 0x00ffe4b5 };
    const uint32_t navajowhite{ 0x00ffdead };
    const uint32_t navy{ 0x00000080 };
    const uint32_t oldlace{ 0x00fdf5e6 };
    const uint32_t olive{ 0x00808000 };
    const uint32_t olivedrab{ 0x006b8e23 };
    const uint32_t orange{ 0x00ffa500 };
    const uint32_t orangered{ 0x00ff4500 };
    const uint32_t orchid{ 0x00da70d6 };
    const uint32_t palegoldenrod{ 0x00eee8aa };
    const uint32_t palegreen{ 0x0098fb98 };
    const uint32_t paleturquoise{ 0x00afeeee };
    const uint32_t palevioletred{ 0x00db7093 };
    const uint32_t papayawhip{ 0x00ffefd5 };
    const uint32_t peachpuff{ 0x00ffdab9 };
    const uint32_t peru{ 0x00cd853f };
    const uint32_t pink{ 0x00ffc0cb };
    const uint32_t plum{ 0x00dda0dd };
    const uint32_t powderblue{ 0x00b0e0e6 };
    const uint32_t purple{ 0x00800080 };
    const uint32_t rebeccapurple{ 0x00663399 };
    const uint32_t red{ 0x00ff0000 };
    const uint32_t rosybrown{ 0x00bc8f8f };
    const uint32_t royalblue{ 0x004169e1 };
    const uint32_t saddlebrown{ 0x008b4513 };
    const uint32_t salmon{ 0x00fa8072 };
    const uint32_t sandybrown{ 0x00f4a460 };
    const uint32_t seagreen{ 0x002e8b57 };
    const uint32_t seashell{ 0x00fff5ee };
    const uint32_t sienna{ 0x00a0522d };
    const uint32_t silver{ 0x00c0c0c0 };
    const uint32_t skyblue{ 0x0087ceeb };
    const uint32_t slateblue{ 0x006a5acd };
    const uint32_t slategrey{ 0x00708090 };
    const uint32_t snow{ 0x00fffafa };
    const uint32_t springgreen{ 0x0000ff7f };
    const uint32_t steelblue{ 0x004682b4 };
    const uint32_t tan{ 0x00d2b48c };
    const uint32_t teal{ 0x00008080 };
    const uint32_t thistle{ 0x00d8bfd8 };
    const uint32_t tomato{ 0x00ff6347 };
    const uint32_t turquoise{ 0x0040e0d0 };
    const uint32_t violet{ 0x00ee82ee };
    const uint32_t wheat{ 0x00f5deb3 };
    const uint32_t white{ 0x00ffffff };
    const uint32_t whitesmoke{ 0x00f5f5f5 };
    const uint32_t yellow{ 0x00ffff00 };
    const uint32_t yellowgreen{ 0x009acd32 };

    uint32_t fromString(const char *stringNumber)
    {
        return strtol(stringNumber, 0, 16);
    }

    uint16_t toRGB565(uint32_t rgb888)
    {
        uint8_t r = rgb888 >> 16;
        uint8_t g = (rgb888 >> 8) & 0xFF;
        uint8_t b = rgb888 & 0xFF;
        return (((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3));
    }

    uint32_t darker(uint32_t originalColour, float factor)
    {
        uint16_t red = (originalColour >> 16) & 0xff;
        uint16_t green = (originalColour >> 8) & 0xff;
        uint16_t blue = originalColour & 0xff;
        uint16_t newRed = (uint16_t)(red * factor) & 0xff;
        uint16_t newGreen = (uint16_t)(green * factor) & 0xff;
        uint16_t newBlue = (uint16_t)(blue * factor) & 0xff;
        return ((newRed << 16) | (newGreen << 8) | newBlue);
    }

    uint32_t lighter(uint32_t originalColour, float factor)
    {
        uint16_t red = (originalColour >> 16) & 0xff;
        uint16_t green = (originalColour >> 8) & 0xff;
        uint16_t blue = originalColour & 0xff;
        uint16_t newRed = (uint16_t)(red + (0xff - red) * factor) & 0xff;
        uint16_t newGreen = (uint16_t)(green + (0xff - green) * factor) & 0xff;
        uint16_t newBlue = (uint16_t)(blue + (0xff - blue) * factor) & 0xff;
        return ((newRed << 16) | (newGreen << 8) | newBlue);
    }
} // namespace Colours888
