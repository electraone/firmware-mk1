#pragma once

#include <Arduino.h>
#include <SPI.h>

#define COLOR_BLACK 0x0000
#define COLOR_WHITE 0xFFFF
#define SCREEN_BPP 16
#define DEFAULT_BPP Bpp16

#define RA8876_FONT_FLAG_XLAT_FULLWIDTH                                        \
    0x01 // Translate ASCII to Unicode fullwidth forms

/* The peripheral divides the bus clock, so anything at or above F_BUS/2 gives
 * the same 30 MHz. Asking for more is silently rounded down, not an error. */
#define RA8876_SPI_SPEED 32000000

/* Burst reads run slower than writes: the controller cannot serve back to
 * back reads at the write clock and drops bytes without any indication that
 * it did. Measured on the hardware, see tools/compat/SPI-PERFORMANCE.md. */
#define RA8876_SPI_READ_SPEED 10000000

#define RA8876_DATA_WRITE 0x80
#define RA8876_DATA_READ 0xC0
#define RA8876_CMD_WRITE 0x00
#define RA8876_STATUS_READ 0x40

// Data sheet 19.2: Chip configuration registers
#define RA8876_REG_SRR 0x00 // Software Reset Register
#define RA8876_REG_CCR 0x01 // Chip Configuration Register
#define RA8876_REG_MACR 0x02 // Memory Access Control Register
#define RA8876_REG_ICR 0x03 // Input Control Register
#define RA8876_REG_MRWDP 0x04 // Memory Read/Write Data Port

// Data sheet 19.3: PLL setting registers
#define RA8876_REG_PPLLC1 0x05 // SCLK PLL control register 1
#define RA8876_REG_PPLLC2 0x06 // SCLK PLL control register 2
#define RA8876_REG_MPLLC1 0x07 // MCLK PLL control register 1
#define RA8876_REG_MPLLC2 0x08 // MCLK PLL control register 2
#define RA8876_REG_SPLLC1 0x09 // CCLK PLL control register 1
#define RA8876_REG_SPLLC2 0x0A // CCLK PLL control register 2

// Data sheet 19.5: LCD display control registers
#define RA8876_REG_MPWCTR 0x10 // Main/PIP Window Control Register
#define RA8876_REG_PIPCDEP 0x11 // PIP Window Color Depth register
#define RA8876_REG_DPCR 0x12 // Display configuration register
#define RA8876_REG_PCSR 0x13 // Panel scan clock and data setting register
#define RA8876_REG_HDWR 0x14 // Horizontal Display Width Register
#define RA8876_REG_HDWFTR 0x15 // Horizontal Display Width Fine Tuning Register
#define RA8876_REG_HNDR 0x16 // Horizontal Non-Display Period Register
#define RA8876_REG_HNDFTR                                                      \
    0x17 // Horizontal Non-Display Period Fine Tuning Register
#define RA8876_REG_HSTR 0x18 // HSYNC start position register
#define RA8876_REG_HPWR 0x19 // HSYNC Pulse Width Register
#define RA8876_REG_VDHR0 0x1A // Vertical Display Height Register 0
#define RA8876_REG_VDHR1 0x1B // Vertical Display Height Register 1
#define RA8876_REG_VNDR0 0x1C // Vertical Non-Display Period Register 0
#define RA8876_REG_VNDR1 0x1D // Vertical Non-Display Period Register 1
#define RA8876_REG_VSTR 0x1E // VSYNC start position register
#define RA8876_REG_VPWR 0x1F // VSYNC pulse width register
#define RA8876_REG_MISA0 0x20 // Main Image Start Address 0
#define RA8876_REG_MISA1 0x21 // Main Image Start Address 1
#define RA8876_REG_MISA2 0x22 // Main Image Start Address 2
#define RA8876_REG_MISA3 0x23 // Main Image Start Address 3
#define RA8876_REG_MIW0 0x24 // Main Image Width 0
#define RA8876_REG_MIW1 0x25 // Main Image Width 1
#define RA8876_REG_MWULX0 0x26 // Main Window Upper-Left X coordinate 0
#define RA8876_REG_MWULX1 0x27 // Main Window Upper-Left X coordinate 1
#define RA8876_REG_MWULY0 0x28 // Main Window Upper-Left Y coordinate 0
#define RA8876_REG_MWULY1 0x29 // Main Window Upper-Left Y coordinate 1

#define RA8876_REG_

// Data sheet 19.6: Geometric engine control registers
#define RA8876_REG_CVSSA0 0x50 // Canvas Start Address 0
#define RA8876_REG_CVSSA1 0x51 // Canvas Start Address 1
#define RA8876_REG_CVSSA2 0x52 // Canvas Start Address 2
#define RA8876_REG_CVSSA3 0x53 // Canvas Start Address 3
#define RA8876_REG_CVS_IMWTH0 0x54 // Canvas image width 0
#define RA8876_REG_CVS_IMWTH1 0x55 // Canvas image width 1
#define RA8876_REG_AWUL_X0 0x56 // Active Window Upper-Left X coordinate 0
#define RA8876_REG_AWUL_X1 0x57 // Active Window Upper-Left X coordinate 1
#define RA8876_REG_AWUL_Y0 0x58 // Active Window Upper-Left Y coordinate 0
#define RA8876_REG_AWUL_Y1 0x59 // Active Window Upper-Left Y coordinate 1
#define RA8876_REG_AW_WTH0 0x5A // Active Window Width 0
#define RA8876_REG_AW_WTH1 0x5B // Active Window Width 1
#define RA8876_REG_AW_HT0 0x5C // Active Window Height 0
#define RA8876_REG_AW_HT1 0x5D // Active Window Height 1
#define RA8876_REG_AW_COLOR 0x5E // Color Depth of canvas & active window
#define RA8876_REG_CURH0 0x5F // Graphic read/write horizontal position 0
#define RA8876_REG_CURH1 0x60 // Graphic read/write horizontal position 1
#define RA8876_REG_CURV0 0x61 // Graphic read/write vertical position 0
#define RA8876_REG_CURV1 0x62 // Graphic read/write vertical position 1
#define RA8876_REG_F_CURX0 0x63 // Text cursor X-coordinate register 0
#define RA8876_REG_F_CURX1 0x64 // Text cursor X-coordinate register 1
#define RA8876_REG_F_CURY0 0x65 // Text cursor Y-coordinate register 0
#define RA8876_REG_F_CURY1 0x66 // Text cursor Y-coordinate register 1

#define RA8876_REG_DCR0 0x67 // Draw shape control register 0

#define RA8876_REG_DLHSR0 0x68 // Draw shape point 1 X coordinate register 0
#define RA8876_REG_DLHSR1 0x69 // Draw shape point 1 X coordinate register 1
#define RA8876_REG_DLVSR0 0x6A // Draw shape point 1 Y coordinate register 0
#define RA8876_REG_DLVSR1 0x6B // Draw shape point 1 Y coordinate register 1

#define RA8876_REG_DLHER0 0x6C // Draw shape point 2 X coordinate register 0
#define RA8876_REG_DLHER1 0x6D // Draw shape point 2 X coordinate register 1
#define RA8876_REG_DLVER0 0x6E // Draw shape point 2 Y coordinate register 0
#define RA8876_REG_DLVER1 0x6F // Draw shape point 2 Y coordinate register 1

#define RA8876_REG_DTPH0 0x70 // Draw shape point 3 X coordinate register 0
#define RA8876_REG_DTPH1 0x71 // Draw shape point 3 X coordinate register 1
#define RA8876_REG_DTPV0 0x72 // Draw shape point 3 Y coordinate register 0
#define RA8876_REG_DTPV1 0x73 // Draw shape point 3 Y coordinate register 1

#define RA8876_REG_DCR1 0x76 // Draw shape control register 1

#define RA8876_REG_ELL_A0 0x77 // Draw ellipse major radius 0
#define RA8876_REG_ELL_A1 0x78 // Draw ellipse major radius 1
#define RA8876_REG_ELL_B0 0x79 // Draw ellipse minor radius 0
#define RA8876_REG_ELL_B1 0x7A // Draw ellipse minor radius 1

#define RA8876_REG_DEHR0 0x7B // Draw ellipse centre X coordinate register 0
#define RA8876_REG_DEHR1 0x7C // Draw ellipse centre X coordinate register 1
#define RA8876_REG_DEVR0 0x7D // Draw ellipse centre Y coordinate register 0
#define RA8876_REG_DEVR1 0x7E // Draw ellipse centre Y coordinate register 1

// Data sheet 19.7: PWM timer control registers
#define RA8876_REG_PSCLR 0x84 // PWM prescaler register
#define RA8876_REG_PMUXR 0x85 // PWM clock mux register
#define RA8876_REG_PCFGR 0x86 // PWM configuration register
#define RA8876_REG_DZ_LENGTH 0x87 // PWM Dead zone length
#define RA8876_REG_TCMPB0L 0x88 // PWM Timer 0 compare buffer register low
#define RA8876_REG_TCMPB0H 0x89 // PWM Timer 0 compare buffer register high
#define RA8876_REG_TCNTB0L 0x8A // PWM Timer 0 count buffer register low
#define RA8876_REG_TCNTB0H 0x8B // PWM Timer 0 count buffer register high

// Data sheet 19.?: BTE operations registers
#define RA8876_REG_BTE_CTRL0 0x90 // BTE Control Register 0
#define RA8876_REG_BTE_CTRL1 0x91 // BTE Control Register 1
#define RA8876_REG_BTE_COLR                                                    \
    0x92 // BTE Source 1, Source 2, Destination color depth

#define RA8876_REG_S0_STR0 0x93 // BTE Source 0 Address 0
#define RA8876_REG_S0_STR1 0x94 // BTE Source 0 Address 1
#define RA8876_REG_S0_STR2 0x95 // BTE Source 0 Address 2
#define RA8876_REG_S0_STR3 0x96 // BTE Source 0 Address 3

#define RA8876_REG_S0_WTH0 0x97 // BTE Source 0 Image Width 0
#define RA8876_REG_S0_WTH1 0x98 // BTE Source 0 Image Width 1

#define RA8876_REG_S0_X0 0x99 // BTE Source 0 Address Upper-Left X coordinate 0
#define RA8876_REG_S0_X1 0x9A // BTE Source 0 Address Upper-Left X coordinate 1
#define RA8876_REG_S0_Y0 0x9B // BTE Source 0 Address Upper-Left X coordinate 0
#define RA8876_REG_S0_Y1 0x9C // BTE Source 0 Address Upper-Left X coordinate 1

#define RA8876_REG_S1_STR0 0x9D // BTE Source 1 Address 0
#define RA8876_REG_S1_STR1 0x9E // BTE Source 1 Address 1
#define RA8876_REG_S1_STR2 0x9F // BTE Source 1 Address 2
#define RA8876_REG_S1_STR3 0xA0 // BTE Source 1 Address 3

#define RA8876_REG_S1_WTH0 0xA1 // BTE Source 1 Image Width 0
#define RA8876_REG_S1_WTH1 0xA2 // BTE Source 1 Image Width 1

#define RA8876_REG_S1_X0 0xA3 // BTE Source 1 Address Upper-Left X coordinate 0
#define RA8876_REG_S1_X1 0xA4 // BTE Source 1 Address Upper-Left X coordinate 1
#define RA8876_REG_S1_Y0 0xA5 // BTE Source 1 Address Upper-Left X coordinate 0
#define RA8876_REG_S1_Y1 0xA6 // BTE Source 1 Address Upper-Left X coordinate 1

#define RA8876_REG_DT_STR0 0xA7 // BTE Destination Address 0
#define RA8876_REG_DT_STR1 0xA8 // BTE Destination Address 1
#define RA8876_REG_DT_STR2 0xA9 // BTE Destination Address 2
#define RA8876_REG_DT_STR3 0xAA // BTE Destination Address 3

#define RA8876_REG_DT_WTH0 0xAB // BTE Destination Image Width 0
#define RA8876_REG_DT_WTH1 0xAC // BTE Destination Image Width 1

#define RA8876_REG_DT_X0                                                       \
    0xAD // BTE Destination Address Upper-Left X coordinate 0
#define RA8876_REG_DT_X1                                                       \
    0xAE // BTE Destination Address Upper-Left X coordinate 1
#define RA8876_REG_DT_Y0                                                       \
    0xAF // BTE Destination Address Upper-Left X coordinate 0
#define RA8876_REG_DT_Y1                                                       \
    0xB0 // BTE Destination Address Upper-Left X coordinate 1

#define RA8876_REG_BTE_WTH0 0xB1 // BTE Destination Window Width 0
#define RA8876_REG_BTE_WTH1 0xB2 // BTE Destination Window Width 1
#define RA8876_REG_BTE_HIG0 0xB3 // BTE Destination Window Height 0
#define RA8876_REG_BTE_HIG1 0xB4 // BTE Destination Window Height 1

#define RA8876_REG_BTE_APB 0xB5 // BTE Color Blending

// Data sheet 19.9: Serial flash & SPI master control registers
#define RA8876_REG_SFL_CTRL 0xB7 // Serial flash/ROM control register
#define RA8876_REG_SPI_DIVSOR 0xBB // SPI clock period

// Data sheet 19.10: Text engine
#define RA8876_REG_CCR0 0xCC // Character Control Register 0
#define RA8876_REG_CCR1 0xCD // Character Control Register 1
#define RA8876_REG_GTFNT_SEL 0xCE // Genitop character ROM select
#define RA8876_REG_GTFNT_CR 0xCF // Genitop character ROM control register

#define RA8876_REG_FLDR 0xD0 // Chracter line gap register
#define RA8876_REG_F2FSSR 0xD1 // Chracter to character space setting register
#define RA8876_REG_FGCR 0xD2 // Foreground colour register - red
#define RA8876_REG_FGCG 0xD3 // Foreground colour register - green
#define RA8876_REG_FGCB 0xD4 // Foreground colour register - blue

// Chroma key color registers
#define RA8876_REG_BGCR 0xD5 // Chroma key backgroud register - red
#define RA8876_REG_BGCG 0xD6 // Chroma key backgroud register - green
#define RA8876_REG_BGCB 0xD7 // Chroma key backgroud register - blue

// CGRAM address
#define RA8876_REG_CGRAM_STR0 0xDB // CGRAM address 0
#define RA8876_REG_CGRAM_STR1 0xDC // CGRAM address 1
#define RA8876_REG_CGRAM_STR2 0xDD // CGRAM address 2
#define RA8876_REG_CGRAM_STR3 0xDE // CGRAM address 3

// Data sheet 19.12: SDRAM control registers
#define RA8876_REG_SDRAR 0xE0 // SDRAM attribute register
#define RA8876_REG_SDRMD 0xE1 // SDRAM mode & extended mode register
#define RA8876_REG_SDR_REF_ITVL0 0xE2 // SDRAM auto refresh interval 0
#define RA8876_REG_SDR_REF_ITVL1 0xE3 // SDRAM auto refresh interval 1
#define RA8876_REG_SDRCR 0xE4 // SDRAM control register

#define RGB332(r, g, b) (((r)&0xE0) | (((g)&0xE0) >> 3) | (((b)&0xE0) >> 6))
#define RGB565(r, g, b)                                                        \
    ((((r)&0xF8) << 8) | (((g)&0xFC) << 3) | (((b)&0xF8) >> 3))

typedef uint8_t FontFlags;

class RA8876
{
public:
    struct SdramInfo {
        int speed; // MHz
        int casLatency; // CAS latency (2 or 3)
        int banks; // Banks (2 or 4)
        int rowBits; // Row addressing bits (11-13)
        int colBits; // Column addressing bits (8-12)
        int refresh; // Refresh time in microseconds
    };

    struct DisplayInfo {
        int width; // Display width
        int height; // Display height

        uint32_t dotClock; // Pixel clock in kHz

        int hFrontPorch; // Will be rounded to the nearest multiple of 8
        int hBackPorch;
        int hPulseWidth; // Will be rounded to the nearest multiple of 8

        int vFrontPorch;
        int vBackPorch;
        int vPulseWidth;
    };

    struct PllParams {
        uint32_t freq; // Frequency in kHz
        int n; // Multiplier less 1 (range 1..63)
        int k; // Divisor power of 2 (range 0..3 for CCLK/MCLK; range 0..7 for SCLK)
    };

    enum FontSource {
        RA8876_FONT_SOURCE_INTERNAL, // CGROM with four 8-bit ISO Latin variants
        RA8876_FONT_SOURCE_EXT_ROM // External font ROM chip
    };

    enum FontSize {
        RA8876_FONT_SIZE_16 = 0x00,
        RA8876_FONT_SIZE_24 = 0x01,
        RA8876_FONT_SIZE_32 = 0x02
    };

    enum FontEncoding {
        RA8876_FONT_ENCODING_GB2312 = 0x00, // GB2312 (Simplified Chinese)
        RA8876_FONT_ENCODING_GB18030 = 0x01, // GB12345/GB18030 (Chinese)
        RA8876_FONT_ENCODING_BIG5 = 0x02, // Big5 (Traditional Chinese)
        RA8876_FONT_ENCODING_UNICODE = 0x03, // Unicode (UCS-2?)
        RA8876_FONT_ENCODING_ASCII = 0x04, // ASCII
        RA8876_FONT_ENCODING_UNIJAPAN = 0x05, // Uni-Japanese (?)
        RA8876_FONT_ENCODING_JIS0208 = 0x06, // JIS X 0208 (Shift JIS?)
        RA8876_FONT_ENCODING_LGCATH =
            0x07, // Latin/Greek/Cyrillic/Arabic/Thai/Hebrew (?)
        RA8876_FONT_ENCODING_8859_1 = 0x11, // ISO 8859-1 (Latin 1)
        RA8876_FONT_ENCODING_8859_2 =
            0x12, // ISO 8859-2 (Latin 2: Eastern European)
        RA8876_FONT_ENCODING_8859_3 =
            0x13, // ISO 8859-3 (Latin 3: South European)
        RA8876_FONT_ENCODING_8859_4 =
            0x14, // ISO 8859-4 (Latin 4: Northern European)
        RA8876_FONT_ENCODING_8859_5 = 0x15, // ISO 8859-5 (Latin/Cyrillic)
        RA8876_FONT_ENCODING_8859_7 = 0x16, // ISO 8859-7 (Latin/Greek)
        RA8876_FONT_ENCODING_8859_8 = 0x17, // ISO 8859-8 (Latin/Hebrew)
        RA8876_FONT_ENCODING_8859_9 = 0x18, // ISO 8859-9 (Latin 5: Turkish)
        RA8876_FONT_ENCODING_8859_10 = 0x19, // ISO 8859-10 (Latin 6: Nordic)
        RA8876_FONT_ENCODING_8859_11 = 0x1A, // ISO 8859-11 (Latin/Thai)
        RA8876_FONT_ENCODING_8859_13 =
            0x1B, // ISO 8859-13 (Latin 7: Baltic Rim)
        RA8876_FONT_ENCODING_8859_14 = 0x1C, // ISO 8859-14 (Latin 8: Celtic)
        RA8876_FONT_ENCODING_8859_15 =
            0x1D, // ISO 8859-15 (Latin 9: Western European)
        RA8876_FONT_ENCODING_8859_16 =
            0x1E // ISO 8859-16 (Latin 10: South-Eastern European)
    };

    enum ExternalFontRom {
        RA8876_FONT_ROM_GT21L16T1W = 0,
        RA8876_FONT_ROM_GT30L16U2W = 1,
        RA8876_FONT_ROM_GT30L24T3Y = 2,
        RA8876_FONT_ROM_GT30L24M1Z = 3,
        RA8876_FONT_ROM_GT30L32S4W = 4,
        RA8876_FONT_ROM_GT20L24F6Y = 5,
        RA8876_FONT_ROM_GT21L24S1W = 6
    };

    struct ExternalFontRomInfo {
        bool present;
        int spiInterface; // SPI interface that font ROM is connected to (0 or 1)
        int spiClockDivisor; // SPI interface clock divisor (2..512 in steps of 2)
        enum ExternalFontRom chip; // Chip type
    };

    enum ExternalFontFamily {
        RA8876_FONT_FAMILY_FIXED = 0,
        RA8876_FONT_FAMILY_ARIAL = 1,
        RA8876_FONT_FAMILY_TIMES = 2,
        RA8876_FONT_FAMILY_FIXED_BOLD = 3
    };

    enum MemoryMode { LinearBpp8, Bpp8, Bpp16, Bpp24 };

    struct SavedState {
        SavedState() :
            locked(false),
            CURH0(0),
            CURV0(0),
            AW_COLOR(0)
        {}

        bool locked;
        uint16_t CURH0;
        uint16_t CURV0;
        uint8_t AW_COLOR;
    };

    // Constructor
    RA8876(int csPin, int resetPin = 0);

    // Init
    bool init(void);
    void initExternalFontRom(int spiIf, enum ExternalFontRom chip);

    // Test
    void colorBarTest(bool enabled);

    // Text/graphics mode
    void setTextMode(void);
    void setGraphicsMode(void);

    // Color handling
    void setForegroundColor(uint32_t color);
    uint32_t getForegroundColor(void);

    // Drawing
    void drawPixel(uint16_t x, uint16_t y);
    void drawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
    void drawRect(uint16_t x1, uint16_t y1, uint16_t width, uint16_t height);
    void fillRect(uint16_t x1, uint16_t y1, uint16_t width, uint16_t height);
    void drawRoundRect(uint16_t x1,
                       uint16_t y1,
                       uint16_t width,
                       uint16_t height,
                       uint16_t radius);
    void fillRoundRect(uint16_t x1,
                       uint16_t y1,
                       uint16_t width,
                       uint16_t height,
                       uint16_t radius);
    void drawTriangle(uint16_t x1,
                      uint16_t y1,
                      uint16_t x2,
                      uint16_t y2,
                      uint16_t x3,
                      uint16_t y3);
    void fillTriangle(uint16_t x1,
                      uint16_t y1,
                      uint16_t x2,
                      uint16_t y2,
                      uint16_t x3,
                      uint16_t y3);
    void drawCircle(uint16_t x, uint16_t y, uint16_t radius);
    void fillCircle(uint16_t x, uint16_t y, uint16_t radius);
    void drawElipse(uint16_t x, uint16_t y, uint16_t radius1, uint16_t radius2);
    void fillElipse(uint16_t x, uint16_t y, uint16_t radius1, uint16_t radius2);
    void fillCurve(uint16_t x, uint16_t y, uint16_t radius, uint8_t segment);
    void clearScreen(uint32_t color);

    // Text cursor
    void setCursor(uint16_t x, uint16_t y);
    uint16_t getCursorX(void);
    uint16_t getCursorY(void);

    // Text
    void loadCGRAMFonts(void);
    void setCGRAMAddress(uint32_t address);
    void
        selectInternalFont(enum FontSize size,
                           enum FontEncoding enc = RA8876_FONT_ENCODING_8859_1);
    void selectCGRAMFont(void);
    void selectExternalFont(enum ExternalFontFamily family,
                            enum FontSize size,
                            enum FontEncoding enc,
                            FontFlags flags = 0);
    uint16_t getTextSizeY(void);
    void setTextScale(uint8_t xScale, uint8_t yScale);
    void setTextScale(uint8_t scale);
    void setTextColor(uint32_t color);
    void setCharacterSpacing(uint8_t numPixels);
    uint8_t internalFontEncoding(enum FontEncoding enc);
    void printFixed(uint16_t x, uint16_t y, const uint16_t *text, size_t size);
    void printProportional(uint16_t x, uint16_t y, const uint16_t *text, size_t size);

    // Frame buffer
    void setCanvasAddress(uint32_t address);
    uint32_t getCanvasAddress(void);
    void setMainWindowControlRegister(void);
    void setMainWindowAddress(uint32_t address);
    void setMainWindowOrigin(uint16_t x, uint16_t y);
    void setMainWindowWidth(uint16_t width);
    void setActiveWindowPosition(uint16_t x, uint16_t y);
    void setActiveWindowSize(uint16_t width, uint16_t height);
    uint16_t getActiveWindowX(void);
    uint16_t getActiveWindowY(void);
    uint16_t getActiveWindowWidth(void);
    uint16_t getActiveWindowHeight(void);

    // BTE
    void bteFill(uint32_t destAddress,
                 uint16_t destX,
                 uint16_t destY,
                 uint16_t destWidth,
                 uint16_t destHeight);
    void bteCopy(uint32_t srcAddress,
                 uint16_t srcX,
                 uint16_t srcY,
                 uint32_t destAddress,
                 uint16_t destX,
                 uint16_t destY,
                 uint16_t width,
                 uint16_t height);
    void bteCopyChroma(uint32_t srcAddress,
                       uint16_t srcX,
                       uint16_t srcY,
                       uint32_t destAddress,
                       uint16_t destX,
                       uint16_t destY,
                       uint16_t width,
                       uint16_t height);
    void bteCopyRop(uint32_t src0Address,
                    uint16_t src0X,
                    uint16_t src0Y,
                    uint32_t src1Address,
                    uint16_t src1X,
                    uint16_t src1Y,
                    uint32_t destAddress,
                    uint16_t destX,
                    uint16_t destY,
                    uint16_t width,
                    uint16_t height,
                    uint8_t rop);
    void bteCopyNoWait(uint32_t srcAddress,
                       uint16_t srcX,
                       uint16_t srcY,
                       uint32_t destAddress,
                       uint16_t destX,
                       uint16_t destY,
                       uint16_t width,
                       uint16_t height);
    void bteExpansion(uint32_t srcAddress,
                      uint16_t srcX,
                      uint16_t srcY,
                      uint32_t destAddress,
                      uint16_t destX,
                      uint16_t destY,
                      uint16_t width,
                      uint16_t height,
                      uint32_t color);
    void setBteChromaColor(uint32_t color);

    // Video RAM / SDRAM
    void setAddress(uint16_t x, uint16_t y);
    void setAddress(uint32_t address);
    uint16_t getAddressX(void);
    uint16_t getAddressY(void);
    uint8_t readRAM8(void);
    void writeRAM8(uint8_t byte);
    uint16_t readRAM16(void);
    void writeRAM16(uint16_t byte);
    uint32_t readRAM32(void);
    void writeRAM32(uint32_t byte);

    // Backlight control
    void setBacklight(boolean on);
    void setBacklightbrightness(uint16_t brightness);

    // Register access. \todo This should be actually private
    void writeReg(uint8_t reg, uint8_t x);

    // Print class
    virtual size_t write(const uint16_t *buffer, size_t size);
    void writeChar(uint16_t c);

    // To make it possible to restore the color depth
    void setMemoryMode(MemoryMode mode);

    // State saving for interrupt context switches
    void saveState(void);
    void restoreState(void);
    bool isStateLocked(void);

    // Support for bulk data transfers
    void armMemoryWrite(void);
    void writeData(const uint8_t *p_src, size_t length);
    void readDataBulk(uint8_t *p_dst, size_t length);

    /** Block access to the memory the address was last set to. These move one
     *  SPI byte per stored byte; the single value accessors cost four. */
    void writeRamBulk(const uint8_t *data, size_t length);
    void readRamBulk(uint8_t *data, size_t length);

private:
    void waitForStatus(uint8_t status);

public:
    /** How many status polls a wait may take before it gives up. At 30 MHz a
     *  poll is about one microsecond, so this is roughly a 50 ms ceiling:
     *  far longer than any legitimate operation, short enough to survive. */
    static const uint32_t statusWaitLimit = 50000;

    /** Number of waits that hit the limit. Non-zero means the display stopped
     *  answering at some point, and whatever was drawn then is suspect. */
    static uint32_t statusWaitTimeouts;

    /** How many bytes a single burst to the memory port may carry before the
     *  write FIFO is given a chance to drain. The bus can deliver bytes
     *  faster than the controller consumes them, and a burst that outruns it
     *  loses bytes with no indication. Zero disables the throttling, which is
     *  only useful for measuring how bad that is. */
    static size_t bulkWriteChunk;

    /** Clock used for everything except burst reads. The peripheral rounds
     *  this down to a divisor of the bus clock, so 30 MHz is the real ceiling
     *  whatever is asked for. */
    static uint32_t writeClock;

    /** Clock used for burst reads, which the controller cannot sustain at the
     *  write clock. Determined by measurement; see DisplayBenchmark. */
    static uint32_t readClock;

private:

    void writeCmd(uint8_t x);
    void writeData(uint8_t x);
    uint8_t readData(void);
    uint8_t readStatus(void);
    void writeRegNBytes(uint8_t reg, uint8_t numBytes, uint32_t x);
    void writeReg16(uint8_t reg, uint16_t x);
    void writeReg32(uint8_t reg, uint32_t x);
    uint32_t readRegNBytes(uint8_t reg, uint8_t numBytes);
    uint8_t readReg(uint8_t reg);
    uint16_t readReg16(uint8_t reg);
    uint32_t readReg32(uint8_t reg);

    void setPointFirst(uint16_t x, uint16_t y);
    void setPointSecond(uint16_t x, uint16_t y);
    void setPointThird(uint16_t x, uint16_t y);
    void setPointCenter(uint16_t x, uint16_t y);
    void setRadius(uint16_t major, uint16_t minor);

    void drawTwoPointShape(uint16_t x1,
                           uint16_t y1,
                           uint16_t width,
                           uint16_t height,
                           uint8_t reg,
                           uint8_t cmd); // drawRect, fillRect
    void drawThreePointShape(uint16_t x1,
                             uint16_t y1,
                             uint16_t x2,
                             uint16_t y2,
                             uint16_t x3,
                             uint16_t y3,
                             uint8_t reg,
                             uint8_t cmd); // drawTriangle, fillTriangle
    void drawEllipseShape(uint16_t x,
                          uint16_t y,
                          uint16_t xrad,
                          uint16_t yrad,
                          uint8_t cmd); // drawCircle, fillCircle
    void drawTwoPointShapeWithRadius(uint16_t x1,
                                     uint16_t y1,
                                     uint16_t width,
                                     uint16_t height,
                                     uint16_t major,
                                     uint16_t minor,
                                     uint8_t reg,
                                     uint8_t cmd);

    void hardReset(void);
    void softReset(void);

    bool calcPllParams(uint32_t targetFreq, int kMax, PllParams *pll);
    bool calcClocks(void);
    void dumpClocks(void);
    bool initPLL(void);
    bool initMemory(SdramInfo *info);
    bool initDisplay(void);
    void setCanvasWidth(uint16_t width);
    void setBteColorDepth(void);
    void setBteSource0Address(uint32_t address);
    void setBteSource0WindowWidth(uint16_t width);
    void setBteSource0Position(uint16_t x, uint16_t y);
    void setBteSource1Address(uint32_t address);
    void setBteSource1WindowWidth(uint16_t width);
    void setBteSource1Position(uint16_t x, uint16_t y);
    void setBteDestinationAddress(uint32_t address);
    void setBteDestinationWindowWidth(uint16_t width);
    void setBteDestinationPosition(uint16_t x, uint16_t y);
    void setBteDestinationSize(uint16_t width, uint16_t height);
    void executeBte(uint8_t mode);

    int m_csPin;
    int m_resetPin;

    int m_width;
    int m_height;
    MemoryMode m_memoryMode;

    uint32_t m_oscClock; // OSC clock (external crystal) frequency in kHz

    PllParams m_memPll; // MCLK (memory) PLL parameters
    PllParams m_corePll; // CCLK (core) PLL parameters
    PllParams m_scanPll; // SCLK (LCD panel scan) PLL parameters

    SPISettings m_spiSettings;
    SdramInfo *m_sdramInfo;
    DisplayInfo *m_displayInfo;
    ExternalFontRomInfo m_fontRomInfo;

    uint16_t m_textColor;
    uint8_t m_textScaleX;
    uint8_t m_textScaleY;

    enum FontSource m_fontSource;
    enum FontSize m_fontSize;
    FontFlags m_fontFlags;

    uint32_t canvasAddress;

    volatile SavedState savedState;

protected:
    void waitCompleted(void);
    void waitWriteFifo(void);
    bool waitForStatusSlow(uint8_t status);
    bool waitNormalOperation(void);
    bool waitSdramReady(void);

    uint16_t activeWindowX;
    uint16_t activeWindowY;
    uint16_t activeWindowWidth;
    uint16_t activeWindowHeight;
    uint16_t foregroundColor;
};
