#include "RA8876.h"
#include "pins_electra.h"

uint32_t RA8876::statusWaitTimeouts = 0;

/* The write path keeps up at the full clock, so bursts need no throttling.
 * Left configurable because the benchmark proves it rather than assuming it. */
size_t RA8876::bulkWriteChunk = 0;

uint32_t RA8876::writeClock = RA8876_SPI_SPEED;
uint32_t RA8876::readClock = RA8876_SPI_READ_SPEED;

RA8876::SdramInfo defaultSdramInfo = {
    120, // 120 MHz
    3, // CAS latency 3
    4, // 4 banks
    12, // 12-bit row addresses
    9, // 9-bit column addresses
    64 // 64 millisecond refresh time
};

RA8876::DisplayInfo defaultDisplayInfo = {
    1024, // Display width
    600, // Display height
    50000, // Pixel clock in kHz

    160, // Horizontal front porch
    160, // Horizontal back porch
    70, // HSYNC pulse width

    12, // Vertical front porch
    23, // Vertical back porch
    10 // VSYNC pulse width
};

RA8876::RA8876(int csPin, int resetPin)
    : activeWindowX(0),
      activeWindowY(0),
      activeWindowWidth(0),
      activeWindowHeight(0),
      foregroundColor(0)
{
    m_csPin = csPin;
    m_resetPin = resetPin;

    m_width = 0;
    m_height = 0;
    m_memoryMode = DEFAULT_BPP;

    m_oscClock = 10000; // 10MHz
    m_sdramInfo = &defaultSdramInfo;
    m_displayInfo = &defaultDisplayInfo;
    m_fontRomInfo.present = false;
}

// Trigger a hardware reset.
void RA8876::hardReset(void)
{
    noInterrupts();
    delay(5);
    LCD_RESET_CLEAR;
    delay(50);
    LCD_RESET_SET;
    delay(5);
    interrupts();

    return;
}

void RA8876::softReset(void)
{
    // Trigger soft reset
    writeReg(RA8876_REG_SRR, 0x01);
    delay(5);

    // Wait for status register to show "normal operation".
    waitNormalOperation();

    return;
}

bool RA8876::calcPllParams(uint32_t targetFreq, int kMax, PllParams *pll)
{
    bool found = false;
    int foundk, foundn;
    uint32_t foundFreq;
    uint32_t foundError; // Amount lower than requested frequency

    // k of 0 (i.e. 2 ** 0 = 1) is possible, but not sure if it's a good idea.
    for (int testk = 1; testk <= kMax; testk++) {
        if (m_oscClock % (1 << testk)) {
            continue; // Step size with this k would be fractional
        }
        int testn = (targetFreq / (m_oscClock / (1 << testk))) - 1;
        if ((testn < 1) || (testn > 63)) {
            continue; // Param n out of range for this k
        }
        // Fvco constraint found in data sheet section 6.1.2
        uint32_t fvco = m_oscClock * (testn + 1);
        if ((fvco < 100000) && (fvco > 600000)) {
            continue; // Fvco out of range
        }
        // Found some usable params, but perhaps at a lower frequency than requested.
        uint32_t freq = (m_oscClock * (testn + 1)) / (1 << testk);
        uint32_t error = targetFreq - freq;
        if ((!found) || (found && (foundError > error))) {
            found = true;
            foundk = testk;
            foundn = testn;
            foundFreq = freq;
            foundError = error;

            // No need to keep searching if the frequency match was exact
            if (foundError == 0) {
                break;
            }
        }
    }

    if (found) {
        pll->freq = foundFreq;
        pll->k = foundk;
        pll->n = foundn;
    }

    return (found);
}

bool RA8876::calcClocks(void)
{
    // Data sheet section 5.2 gives max clocks:
    //  memClock : 166 MHz
    //  coreClock: 120 MHz (133MHz if not using internal font)
    //  scanClock: 100 MHz

    // Mem clock target is the same as SDRAM speed, but capped at 166 MHz
    uint32_t memClock = m_sdramInfo->speed * 1000;

    if (memClock > 166000) {
        memClock = 166000;
    }

    if (!calcPllParams(memClock, 3, &m_memPll)) {
        return (false);
    }

    // Core clock target will be the same as the mem clock, but capped to
    //  120 MHz, because that is the max frequency if we want to use the
    //  internal font.
    uint32_t coreClock = m_memPll.freq;
    if (coreClock > 120000) {
        coreClock = 120000;
    }

    if (!calcPllParams(coreClock, 3, &m_corePll)) {
        return (false);
    }

    // Scan clock target will be the display's dot clock, but capped at 100 MHz
    uint32_t scanClock = m_displayInfo->dotClock;
    if (scanClock > 100000) {
        scanClock = 100000;
    }

    if (!calcPllParams(scanClock, 7, &m_scanPll)) {
        return (false);
    }

    dumpClocks();

    // Data sheet section 6.1.1 rules:
    // 1. Core clock must be less than or equal to mem clock
    if (m_corePll.freq > m_memPll.freq) {
        return (false);
    }

    // 2. Core clock must be greater than half mem clock
    if ((m_corePll.freq * 2) <= m_memPll.freq) {
        return (false);
    }

    // 3. Core clock must be greater than (scan clock * 1.5)
    if (m_corePll.freq <= (m_scanPll.freq + (m_scanPll.freq >> 1))) {
        return (false);
    }

    return (true);
}

// Dump clock info to serial monitor.
void RA8876::dumpClocks(void)
{
#ifdef DEBUG
    Serial.println("\nMem\n---");
    Serial.print("Requested kHz: ");
    Serial.println(m_sdramInfo->speed * 1000);
    Serial.print("Actual kHz   : ");
    Serial.println(m_memPll.freq);
    Serial.print("PLL k        : ");
    Serial.println(m_memPll.k);
    Serial.print("PLL n        : ");
    Serial.println(m_memPll.n);

    Serial.println("\nCore\n----");
    Serial.print("kHz          : ");
    Serial.println(m_corePll.freq);
    Serial.print("PLL k        : ");
    Serial.println(m_corePll.k);
    Serial.print("PLL n        : ");
    Serial.println(m_corePll.n);

    Serial.println("\nScan\n----");
    Serial.print("Requested kHz: ");
    Serial.println(m_displayInfo->dotClock);
    Serial.print("Actual kHz   : ");
    Serial.println(m_scanPll.freq);
    Serial.print("PLL k        : ");
    Serial.println(m_scanPll.k);
    Serial.print("PLL n        : ");
    Serial.println(m_scanPll.n);
#endif /* DEBUG */

    // TODO: Frame rate?

    return;
}

bool RA8876::initPLL(void)
{
#ifdef DEBUG
    Serial.println("init PLL");
#endif /* DEBUG */

    //Serial.print("DRAM_FREQ "); Serial.println(m_memPll.freq);
    //Serial.print("7: "); Serial.println(m_memPll.k << 1);
    //Serial.print("8: "); Serial.println(m_memPll.n);
    writeReg(RA8876_REG_MPLLC1, m_memPll.k << 1);
    writeReg(RA8876_REG_MPLLC2, m_memPll.n);

    //Serial.print("CORE_FREQ "); Serial.println(m_corePll.freq);
    //Serial.print("9: "); Serial.println(m_corePll.k << 1);
    //Serial.print("A: "); Serial.println(m_corePll.n);
    writeReg(RA8876_REG_SPLLC1, m_corePll.k << 1);
    writeReg(RA8876_REG_SPLLC2, m_corePll.n);

    // Per the data sheet, there are two divider fields for the scan clock,
    // but the math seems to work out if we treat k as a single 3-bit
    // number in bits 3..1.
    //Serial.print("SCAN_FREQ "); Serial.println(m_scanPll.freq);
    //Serial.print("5: "); Serial.println(m_scanPll.k << 1);
    //Serial.print("6: "); Serial.println(m_scanPll.n);
    writeReg(RA8876_REG_PPLLC1, m_scanPll.k << 1);
    writeReg(RA8876_REG_PPLLC2, m_scanPll.n);

    // Toggle bit 7 of the CCR register to trigger a reconfiguration of the PLLs
    writeReg(RA8876_REG_CCR, 0x00);
    delay(2);
    writeReg(RA8876_REG_CCR, 0x80);
    delay(2);

    uint8_t ccr = readReg(RA8876_REG_CCR);

    return ((ccr & 0x80) ? true : false);
}

// Initialize SDRAM interface.
bool RA8876::initMemory(SdramInfo *info)
{
    uint32_t sdramRefreshRate;
    uint8_t sdrar = 0x00;
    uint8_t sdrmd = 0x00;

    // Refresh rate
    sdramRefreshRate =
        ((uint32_t)info->refresh * info->speed * 1000) >> info->rowBits;

    // Number of banks
    if (info->banks == 2) {
        ; // NOP
    } else if (info->banks == 4) {
        sdrar |= 0x20;
    } else {
        return (false); // Unsupported number of banks
    }
    // Number of row bits
    if ((info->rowBits < 11) || (info->rowBits > 13)) {
        return (false); // Unsupported number of row bits
    } else {
        sdrar |= ((info->rowBits - 11) & 0x03) << 3;
    }

    // Number of column bits
    if ((info->colBits < 8) || (info->colBits > 12)) {
        return (false); // Unsupported number of column bits
    } else {
        sdrar |= info->colBits & 0x03;
    }

    // CAS latency
    if ((info->casLatency < 2) || (info->casLatency > 3)) {
        return (false); // Unsupported CAS latency
    } else {
        sdrmd |= info->casLatency & 0x03;
    }

#ifdef DEBUG
    Serial.print("SDRAR: ");
    Serial.println(sdrar); // Expected: 0x29 (41 decimal)
#endif

    writeReg(RA8876_REG_SDRAR, sdrar);

#ifdef DEBUG
    Serial.print("SDRMD: ");
    Serial.println(sdrmd);
#endif

    writeReg(RA8876_REG_SDRMD, sdrmd);

#ifdef DEBUG
    Serial.print("sdramRefreshRate: ");
    Serial.println(sdramRefreshRate);
#endif

    writeReg(RA8876_REG_SDR_REF_ITVL0, sdramRefreshRate & 0xFF);
    writeReg(RA8876_REG_SDR_REF_ITVL1, sdramRefreshRate >> 8);

    // Trigger SDRAM initialization
    writeReg(RA8876_REG_SDRCR, 0x01);

    bool sdramReady = waitSdramReady();

    return (sdramReady);
}

bool RA8876::initDisplay()
{
    // SPI.beginTransaction(m_spiSettings);

    // Set chip config register
    uint8_t ccr = readReg(RA8876_REG_CCR);

    ccr &= 0xE7; // 24-bit LCD output
    ccr &= 0xFE; // 8-bit host data bus
    writeReg(RA8876_REG_CCR, ccr);

    writeReg(RA8876_REG_MACR,
             0x00); // Direct write, left-to-right-top-to-bottom memory

    writeReg(RA8876_REG_ICR, 0x00); // Graphics mode, memory is SDRAM

    uint8_t dpcr = readReg(RA8876_REG_DPCR);
    dpcr &= 0xFB; // Vertical scan top to bottom
    dpcr &= 0xF8; // Colour order RGB
    dpcr |= 0x80; // Panel fetches PDAT at PCLK falling edge
    writeReg(RA8876_REG_DPCR, dpcr);

    uint8_t pcsr = readReg(RA8876_REG_PCSR);
    pcsr |= 0x80; // XHSYNC polarity high
    pcsr |= 0x40; // XVSYNC polarity high
    pcsr &= 0xDF; // XDE polarity high
    writeReg(RA8876_REG_PCSR, pcsr);

    // Set display width
    writeReg(RA8876_REG_HDWR, (m_displayInfo->width / 8) - 1);
    writeReg(RA8876_REG_HDWFTR, (m_displayInfo->width % 8));

    // Set display height
    writeReg(RA8876_REG_VDHR0, (m_displayInfo->height - 1) & 0xFF);
    writeReg(RA8876_REG_VDHR1, (m_displayInfo->height - 1) >> 8);

    // Set horizontal non-display (back porch)
    writeReg(RA8876_REG_HNDR, (m_displayInfo->hBackPorch / 8) - 1);
    writeReg(RA8876_REG_HNDFTR, (m_displayInfo->hBackPorch % 8));

    // Set horizontal start position (front porch)
    writeReg(RA8876_REG_HSTR, ((m_displayInfo->hFrontPorch + 4) / 8) - 1);

    // Set HSYNC pulse width
    writeReg(RA8876_REG_HPWR, ((m_displayInfo->hPulseWidth + 4) / 8) - 1);

    // Set vertical non-display (back porch)
    writeReg(RA8876_REG_VNDR0, (m_displayInfo->vBackPorch - 1) & 0xFF);
    writeReg(RA8876_REG_VNDR1, (m_displayInfo->vBackPorch - 1) >> 8);

    // Set vertical start position (front porch)
    writeReg(RA8876_REG_VSTR, m_displayInfo->vFrontPorch - 1);

    // Set VSYNC pulse width
    writeReg(RA8876_REG_VPWR, m_displayInfo->vPulseWidth - 1);

    setMainWindowControlRegister();
    setMainWindowAddress(0);
    setMainWindowWidth(m_width);
    setMainWindowOrigin(0, 0);
    setCanvasAddress(0);
    setCanvasWidth(m_width);
    setActiveWindowPosition(0, 0);
    setActiveWindowSize(m_width, m_height);
    setMemoryMode(Bpp16);
    setBteColorDepth();
    setBteDestinationWindowWidth(m_width);
    setBteSource0WindowWidth(m_width);
    setBteSource1WindowWidth(m_width);

    // Turn on display
    dpcr = readReg(RA8876_REG_DPCR);
    dpcr |= 0x40; // Display on
    writeReg(RA8876_REG_DPCR, dpcr);

    return (true);
}

bool RA8876::init(void)
{
    m_width = m_displayInfo->width;
    m_height = m_displayInfo->height;
    m_memoryMode = DEFAULT_BPP;

    LCD_CS_SET;
    LCD_RESET_SET;
    
    hardReset();

    if (!calcClocks()) {
        return (false);
    }

    SPI.begin();

    m_spiSettings = SPISettings(RA8876_SPI_SPEED, MSBFIRST, SPI_MODE3);
    SPI.beginTransaction(m_spiSettings);

    // SPI is now up, so we can do a soft reset if no hard reset was
    // possible earlier
    softReset();

    if (!initPLL()) {
        return (false);
    }

    if (!initMemory(m_sdramInfo)) {
        return (false);
    }

    if (!initDisplay()) {
        return (false);
    }

    return (true);
}

void RA8876::initExternalFontRom(int spiIf, enum ExternalFontRom chip)
{
    // See data sheet figure 16-10
    // TODO: GT30L24T3Y supports FAST_READ command (0x0B) and runs at 20MHz.
    // Are the other font chips the same?

    // Find a clock divisor. Values are in the range 2..512 in steps of 2.
    int divisor;
    uint32_t speed = 20000; // 20MHz target speed

    for (divisor = 2; divisor <= 512; divisor += 2) {
        if (m_corePll.freq / divisor <= speed) {
            break;
        }
    }

    m_fontRomInfo.present = true;
    m_fontRomInfo.spiInterface = spiIf;
    m_fontRomInfo.spiClockDivisor = divisor;
    m_fontRomInfo.chip = chip;

#ifdef DEBUG
    Serial.print("External font SPI divisor: ");
    Serial.println(divisor);
#endif

    // Ensure SPI is enabled in chip config register
    uint8_t ccr = readReg(RA8876_REG_CCR);
    if (!(ccr & 0x02)) {
        writeReg(RA8876_REG_CCR, ccr | 0x02);
    }

#ifdef DEBUG
    Serial.print("SFL_CTRL: ");
    Serial.println(((spiIf & 1) << 7) | 0x14, HEX);
#endif
    writeReg(RA8876_REG_SFL_CTRL,
             ((spiIf & 1) << 7)
                 | 0x14); // Font mode, 24-bit address, standard timing,
    //supports FAST_READ

#ifdef DEBUG
    Serial.print("SPI_DIVSOR: ");
    Serial.println((divisor >> 1) - 1, HEX);
#endif
    writeReg(RA8876_REG_SPI_DIVSOR, (divisor >> 1) - 1);

#ifdef DEBUG
    Serial.print("GTFNT_SEL: ");
    Serial.println((chip & 0x07) << 5, HEX);
#endif

    writeReg(RA8876_REG_GTFNT_SEL, (chip & 0x07) << 5);
}

uint8_t RA8876::readRAM8(void)
{
    return (readReg(RA8876_REG_MRWDP));
}

uint16_t RA8876::readRAM16(void)
{
    uint8_t byteLsb = readReg(RA8876_REG_MRWDP);
    uint8_t byteMsb = readReg(RA8876_REG_MRWDP);

    return ((byteMsb << 8) | byteLsb);
}

uint32_t RA8876::readRAM32(void)
{
    uint8_t byteLsb0 = readReg(RA8876_REG_MRWDP);
    uint8_t byteLsb1 = readReg(RA8876_REG_MRWDP);
    uint8_t byteMsb0 = readReg(RA8876_REG_MRWDP);
    uint8_t byteMsb1 = readReg(RA8876_REG_MRWDP);

    return ((byteMsb1 << 24) | (byteMsb0 << 16) | (byteLsb1 << 8) | byteLsb0);
}

void RA8876::writeRAM8(uint8_t byte)
{
    writeReg(RA8876_REG_MRWDP, byte);
}

void RA8876::writeRAM16(uint16_t byte)
{
    writeReg(RA8876_REG_MRWDP, byte & 0xFF);
    writeReg(RA8876_REG_MRWDP, (byte >> 8));
}

void RA8876::writeRAM32(uint32_t word)
{
    writeReg(RA8876_REG_MRWDP, word & 0xFF);
    writeReg(RA8876_REG_MRWDP, (word >> 8) & 0xFF);
    writeReg(RA8876_REG_MRWDP, (word >> 16) & 0xFF);
    writeReg(RA8876_REG_MRWDP, (word >> 24) & 0xFF);
}

void RA8876::colorBarTest(bool enabled)
{
    uint8_t dpcr = readReg(RA8876_REG_DPCR);

    if (enabled) {
        dpcr = dpcr | 0x20;
    } else {
        dpcr = dpcr & ~0x20;
    }

    writeReg(RA8876_REG_DPCR, dpcr);
}

void RA8876::drawPixel(uint16_t x, uint16_t y)
{
    setAddress(x + activeWindowX, y + activeWindowY);
    writeRAM16(foregroundColor);
}

void RA8876::drawRect(uint16_t x1, uint16_t y1, uint16_t width, uint16_t height)
{
    drawTwoPointShape(x1, y1, width, height, RA8876_REG_DCR1, 0xA0);
}

void RA8876::fillRect(uint16_t x1, uint16_t y1, uint16_t width, uint16_t height)
{
    drawTwoPointShape(x1, y1, width, height, RA8876_REG_DCR1, 0xE0);
}

void RA8876::drawRoundRect(uint16_t x1,
                           uint16_t y1,
                           uint16_t width,
                           uint16_t height,
                           uint16_t radius)
{
    drawTwoPointShapeWithRadius(
        x1, y1, width, height, radius, radius, RA8876_REG_DCR1, 0xB0);
}

void RA8876::fillRoundRect(uint16_t x1,
                           uint16_t y1,
                           uint16_t width,
                           uint16_t height,
                           uint16_t radius)
{
    drawTwoPointShapeWithRadius(
        x1, y1, width, height, radius, radius, RA8876_REG_DCR1, 0xF0);
}

void RA8876::drawTriangle(uint16_t x1,
                          uint16_t y1,
                          uint16_t x2,
                          uint16_t y2,
                          uint16_t x3,
                          uint16_t y3)
{
    drawThreePointShape(x1, y1, x2, y2, x3, y3, RA8876_REG_DCR0, 0x82);
}

void RA8876::fillTriangle(uint16_t x1,
                          uint16_t y1,
                          uint16_t x2,
                          uint16_t y2,
                          uint16_t x3,
                          uint16_t y3)
{
    drawThreePointShape(x1, y1, x2, y2, x3, y3, RA8876_REG_DCR0, 0xA2);
}

void RA8876::drawCircle(uint16_t x, uint16_t y, uint16_t radius)
{
    drawEllipseShape(x, y, radius, radius, 0x80);
}

void RA8876::fillCircle(uint16_t x, uint16_t y, uint16_t radius)
{
    drawEllipseShape(x, y, radius, radius, 0xC0);
}

void RA8876::drawElipse(uint16_t x,
                        uint16_t y,
                        uint16_t radius1,
                        uint16_t radius2)
{
    drawEllipseShape(x, y, radius1, radius2, 0x80);
}

void RA8876::fillElipse(uint16_t x,
                        uint16_t y,
                        uint16_t radius1,
                        uint16_t radius2)
{
    drawEllipseShape(x, y, radius1, radius2, 0xC0);
}

void RA8876::fillCurve(uint16_t x,
                       uint16_t y,
                       uint16_t radius,
                       uint8_t segment) // \todo use constatnt / enum
{
    drawEllipseShape(x, y, radius, radius, 0xD0 + segment);
}

void RA8876::clearScreen(uint32_t color)
{
    setForegroundColor(color);
    fillRect(0, 0, m_width, m_height);
}

void RA8876::setAddress(uint16_t x, uint16_t y)
{
    writeReg16(RA8876_REG_CURH0, x);
    writeReg16(RA8876_REG_CURV0, y);
    waitCompleted();
    waitSdramReady();
}

void RA8876::setAddress(uint32_t address)
{
    writeReg32(RA8876_REG_CURH0, address);
    waitCompleted();
    waitSdramReady();
    (void)readReg(RA8876_REG_MRWDP); // this is required, see datasheet
}

void RA8876::setForegroundColor(uint32_t color)
{
    foregroundColor = color;
    writeReg(RA8876_REG_FGCR, color >> 11 << 3);
    writeReg(RA8876_REG_FGCG, ((color >> 5) & 0x3F) << 2);
    writeReg(RA8876_REG_FGCB, (color & 0x1F) << 3);
}

uint32_t RA8876::getForegroundColor(void)
{
    return (foregroundColor);
}

void RA8876::setPointFirst(uint16_t x, uint16_t y)
{
    writeReg16(RA8876_REG_DLHSR0, x);
    writeReg16(RA8876_REG_DLVSR0, y);
}

void RA8876::setPointSecond(uint16_t x, uint16_t y)
{
    writeReg16(RA8876_REG_DLHER0, x);
    writeReg16(RA8876_REG_DLVER0, y);
}

void RA8876::setPointThird(uint16_t x, uint16_t y)
{
    writeReg16(RA8876_REG_DTPH0, x);
    writeReg16(RA8876_REG_DTPV0, y);
}

void RA8876::setPointCenter(uint16_t x, uint16_t y)
{
    writeReg16(RA8876_REG_DEHR0, x);
    writeReg16(RA8876_REG_DEVR0, y);
}

void RA8876::setRadius(uint16_t major, uint16_t minor)
{
    writeReg16(RA8876_REG_ELL_A0, major);
    writeReg16(RA8876_REG_ELL_B0, minor);
}
#include "helpers.h"
void RA8876::saveState(void)
{
    if (!savedState.locked) {
        savedState.locked = true;
        //System::logger.write(LOG_ERROR, "Saving state");
        waitCompleted();
        savedState.CURH0 = readReg16(RA8876_REG_CURH0);
        savedState.CURV0 = readReg16(RA8876_REG_CURV0);
    }
}

void RA8876::restoreState(void)
{
    //System::logger.write(LOG_ERROR, "Restoring state");
    if (savedState.locked) {
        waitCompleted();
        writeReg16(RA8876_REG_CURH0, savedState.CURH0);
        writeReg16(RA8876_REG_CURV0, savedState.CURV0);
        waitCompleted();
        savedState.locked = false;
    }
}

bool RA8876::isStateLocked(void)
{
    return (savedState.locked);
}

/**
 * @brief Waits for the given status bits to clear.
 *
 * @attention the wait is bounded. It used to spin forever, so a display that
 * stopped answering, or a bus reading all ones, took the whole firmware down
 * with it and left no trace of why. Giving up keeps the rest of the system
 * alive and counts the event so it can be seen.
 */
void RA8876::waitForStatus(uint8_t status)
{
    uint32_t attempts = 0;

    while (readStatus() & status) {
        if (++attempts > statusWaitLimit) {
            statusWaitTimeouts++;
            return;
        }
    }
}

void RA8876::waitCompleted(void)
{
    waitForStatus(0x08);
}

void RA8876::waitWriteFifo(void)
{
    waitForStatus(0x80);
}

bool RA8876::waitForStatusSlow(uint8_t status)
{
    for (int i = 0; i < 255; i++) {
        if (readStatus() & status) {
            return (true);
        }
        delay(1);
    }

    return (false);
}

bool RA8876::waitNormalOperation(void)
{
    return (waitForStatusSlow(0x02));
}

bool RA8876::waitSdramReady(void)
{

	while ((readStatus() & 0x04) == 0) {
		//System::logger.write(LOG_ERROR, "sdrem status: %x", readStatus());
		asm("nop");
	}
	//System::logger.write(LOG_ERROR, "sdrem final status: %x", readStatus());
	return (true);
}

void RA8876::drawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    x1 += activeWindowX;
    y1 += activeWindowY;
    x2 += activeWindowX;
    y2 += activeWindowY;

    setPointFirst(x1, y1);
    setPointSecond(x2, y2);
    writeReg(RA8876_REG_DCR0, 0x80);
    waitCompleted();
}

void RA8876::drawTwoPointShape(uint16_t x1,
                               uint16_t y1,
                               uint16_t width,
                               uint16_t height,
                               uint8_t reg,
                               uint8_t cmd)
{
    x1 += activeWindowX;
    y1 += activeWindowY;
    uint16_t x2 = x1 + width - 1;
    uint16_t y2 = y1 + height - 1;

    setPointFirst(x1, y1);
    setPointSecond(x2, y2);
    writeReg(reg, cmd); // Start drawing
    waitCompleted();
}

void RA8876::drawTwoPointShapeWithRadius(uint16_t x1,
                                         uint16_t y1,
                                         uint16_t width,
                                         uint16_t height,
                                         uint16_t major,
                                         uint16_t minor,
                                         uint8_t reg,
                                         uint8_t cmd)
{
    x1 += activeWindowX;
    y1 += activeWindowY;
    uint16_t x2 = x1 + width - 1;
    uint16_t y2 = y1 + height - 1;

    setPointFirst(x1, y1);
    setPointSecond(x2, y2);
    setRadius(major, minor);
    writeReg(reg, cmd); // Start drawing
    waitCompleted();
}

void RA8876::drawThreePointShape(uint16_t x1,
                                 uint16_t y1,
                                 uint16_t x2,
                                 uint16_t y2,
                                 uint16_t x3,
                                 uint16_t y3,
                                 uint8_t reg,
                                 uint8_t cmd)
{
    x1 += activeWindowX;
    y1 += activeWindowY;
    x2 += activeWindowX;
    y2 += activeWindowY;
    x3 += activeWindowX;
    y3 += activeWindowY;

    setPointFirst(x1, y1);
    setPointSecond(x2, y2);
    setPointThird(x3, y3);
    writeReg(reg, cmd); // Start drawing
    waitCompleted();
}

void RA8876::drawEllipseShape(uint16_t x,
                              uint16_t y,
                              uint16_t xrad,
                              uint16_t yrad,
                              uint8_t cmd)
{
    x += activeWindowX;
    y += activeWindowY;

    setPointCenter(x, y);
    setRadius(xrad, yrad);
    writeReg(RA8876_REG_DCR1, cmd); // Start drawing
    waitCompleted();
}

void RA8876::setCGRAMAddress(uint32_t address)
{
    waitCompleted();

    writeReg32(RA8876_REG_CGRAM_STR0, address);
    writeCmd(RA8876_REG_MRWDP);

    waitCompleted();
}

void RA8876::setCursor(uint16_t x, uint16_t y)
{
    x += activeWindowX;
    y += activeWindowY;

    writeReg16(RA8876_REG_F_CURX0, x);
    writeReg16(RA8876_REG_F_CURY0, y);
}

uint16_t RA8876::getCursorX(void)
{
    return (readReg16(RA8876_REG_F_CURX0) - activeWindowX);
}

uint16_t RA8876::getCursorY(void)
{
    return (readReg16(RA8876_REG_F_CURY0) - activeWindowY);
}

// Given a font encoding value, returns the corresponding bit pattern for
//  use by internal fonts.
uint8_t RA8876::internalFontEncoding(enum FontEncoding enc)
{
    uint8_t e;

    switch (enc) {
        case RA8876_FONT_ENCODING_8859_2:
            e = 0x01;
            break;
        case RA8876_FONT_ENCODING_8859_4:
            e = 0x02;
            break;
        case RA8876_FONT_ENCODING_8859_5:
            e = 0x03;
            break;
        default:
            e = 0x00; // ISO-8859-1
            break;
    }

    return (e);
}

void RA8876::setTextMode(void)
{
    uint8_t icr = readReg(RA8876_REG_ICR);
    writeReg(RA8876_REG_ICR, icr | 0x04);
}

void RA8876::setGraphicsMode(void)
{
    waitCompleted();

    // Disable text mode
    uint8_t icr = readReg(RA8876_REG_ICR);
    writeReg(RA8876_REG_ICR, icr & ~0x04);
}

void RA8876::selectInternalFont(enum FontSize size, enum FontEncoding enc)
{
    m_fontSource = RA8876_FONT_SOURCE_INTERNAL;
    m_fontSize = size;
    m_fontFlags = 0;

    writeReg(RA8876_REG_CCR0,
             0x00 | ((size & 0x03) << 4) | internalFontEncoding(enc));

    uint8_t ccr1 = readReg(RA8876_REG_CCR1);
    ccr1 |= 0x40; // Transparent background
    writeReg(RA8876_REG_CCR1, ccr1);
}

void RA8876::selectCGRAMFont(void)
{
    writeReg(RA8876_REG_CCR0, 0x80);

    uint8_t ccr1 = readReg(RA8876_REG_CCR1);
    ccr1 |= 0x40; // Transparent background
    writeReg(RA8876_REG_CCR1, ccr1);
}

void RA8876::selectExternalFont(enum ExternalFontFamily family,
                                enum FontSize size,
                                enum FontEncoding enc,
                                FontFlags flags)
{
    m_fontSource = RA8876_FONT_SOURCE_EXT_ROM;
    m_fontSize = size;
    m_fontFlags = flags;

    writeReg(RA8876_REG_CCR0,
             0x40 | ((size & 0x03) << 4)); // Select external font ROM and size

    uint8_t ccr1 = readReg(RA8876_REG_CCR1);
    ccr1 |= 0x40; // Transparent background

    writeReg(RA8876_REG_CCR1, ccr1);
    writeReg(RA8876_REG_GTFNT_CR,
             (enc << 3) | (family & 0x03)); // Character encoding and family
}

uint16_t RA8876::getTextSizeY(void)
{
    return (((m_fontSize + 2) * 8) * m_textScaleY);
}

void RA8876::setTextScale(uint8_t xScale, uint8_t yScale)
{
    xScale = constrain(xScale, 1, 4);
    yScale = constrain(yScale, 1, 4);

    m_textScaleX = xScale;
    m_textScaleY = yScale;

    uint8_t ccr1 = readReg(RA8876_REG_CCR1);
    ccr1 = (ccr1 & 0xF0) | ((xScale - 1) << 2) | (yScale - 1);
    writeReg(RA8876_REG_CCR1, ccr1);
}

void RA8876::setTextScale(uint8_t scale)
{
    setTextScale(scale, scale);
}

void RA8876::setTextColor(uint32_t color)
{
    m_textColor = color;
}

void RA8876::setCharacterSpacing(uint8_t numPixels)
{
    writeReg(RA8876_REG_F2FSSR, numPixels & 0x3F);
}

size_t RA8876::write(const uint16_t *buffer, size_t size)
{
    setTextMode();
    writeCmd(RA8876_REG_MRWDP);

    for (size_t i = 0; i < size; i++) {
        uint16_t c = buffer[i];
        waitWriteFifo();
        writeData(c >> 8);
        writeData(c & 0xff);
    }
    setGraphicsMode();

    return (size);
}

void RA8876::writeChar(uint16_t c)
{
    writeCmd(RA8876_REG_MRWDP);
    waitWriteFifo();
    writeData((c >> 8) | 0x80);
    writeData(c & 0xff);
}

void RA8876::setCanvasAddress(uint32_t address)
{
    static uint32_t pAddress = 0; // move to the class

    if (address == pAddress) {
        return;
    }

    waitCompleted();

    writeReg32(RA8876_REG_CVSSA0, address);
    writeCmd(RA8876_REG_MRWDP);

    waitCompleted();

    pAddress = address;
    canvasAddress = address;
}

uint32_t RA8876::getCanvasAddress(void)
{
    return (canvasAddress);
}

void RA8876::setCanvasWidth(uint16_t width)
{
    writeReg16(RA8876_REG_CVS_IMWTH0, width);
}

void RA8876::setMainWindowControlRegister(void)
{
    // PIP windows disabled, enable sync signals
    uint8_t mpwctr = 0;

    if (m_memoryMode == Bpp8) {
        mpwctr = 0x00;
    } else if (m_memoryMode == Bpp16) {
        mpwctr = 0x04;
    } else {
        mpwctr = 0x08;
    }

    writeReg(RA8876_REG_MPWCTR, mpwctr);
}

void RA8876::setMainWindowAddress(uint32_t address)
{
    writeReg32(RA8876_REG_MISA0, address);
}

void RA8876::setMainWindowOrigin(uint16_t x, uint16_t y)
{
    writeReg16(RA8876_REG_MWULX0, x);
    writeReg16(RA8876_REG_MWULY0, y);
}

void RA8876::setMainWindowWidth(uint16_t width)
{
    writeReg16(RA8876_REG_MIW0, width);
}

void RA8876::setBteSource0Address(uint32_t address)
{
    writeReg32(RA8876_REG_S0_STR0, address);
}

void RA8876::setBteSource0WindowWidth(uint16_t width)
{
    writeReg16(RA8876_REG_S0_WTH0, width);
}

void RA8876::setBteSource0Position(uint16_t x, uint16_t y)
{
    writeReg16(RA8876_REG_S0_X0, x);
    writeReg16(RA8876_REG_S0_Y0, y);
}

void RA8876::setBteSource1Address(uint32_t address)
{
    writeReg32(RA8876_REG_S1_STR0, address);
}

void RA8876::setBteSource1WindowWidth(uint16_t width)
{
    writeReg16(RA8876_REG_S1_WTH0, width);
}

void RA8876::setBteSource1Position(uint16_t x, uint16_t y)
{
    x += activeWindowX;
    y += activeWindowY;

    writeReg16(RA8876_REG_S1_X0, x);
    writeReg16(RA8876_REG_S1_Y0, y);
}

void RA8876::setBteDestinationAddress(uint32_t address)
{
    writeReg32(RA8876_REG_DT_STR0, address);
}

void RA8876::setBteDestinationWindowWidth(uint16_t width)
{
    writeReg16(RA8876_REG_DT_WTH0, width);
}

void RA8876::setBteDestinationPosition(uint16_t x, uint16_t y)
{
    x += activeWindowX;
    y += activeWindowY;

    writeReg16(RA8876_REG_DT_X0, x);
    writeReg16(RA8876_REG_DT_Y0, y);
}

void RA8876::setBteDestinationSize(uint16_t width, uint16_t height)
{
    writeReg16(RA8876_REG_BTE_WTH0, width);
    writeReg16(RA8876_REG_BTE_HIG0, height);
}

void RA8876::executeBte(uint8_t mode)
{
    writeReg(RA8876_REG_BTE_CTRL1, mode);
    writeReg(RA8876_REG_BTE_CTRL0, 0x10);
}

void RA8876::bteFill(uint32_t destAddress,
                     uint16_t destX,
                     uint16_t destY,
                     uint16_t destWidth,
                     uint16_t destHeight)
{
    setBteDestinationAddress(destAddress);
    setBteDestinationPosition(destX, destY);
    setBteDestinationSize(destWidth, destHeight);
    setForegroundColor(foregroundColor);
    executeBte(0x0C);
}

void RA8876::bteCopy(uint32_t srcAddress,
                     uint16_t srcX,
                     uint16_t srcY,
                     uint32_t destAddress,
                     uint16_t destX,
                     uint16_t destY,
                     uint16_t width,
                     uint16_t height)
{
    waitCompleted();

    setBteSource0Address(srcAddress);
    setBteSource0Position(srcX, srcY);
    setBteDestinationAddress(destAddress);
    setBteDestinationPosition(destX, destY);
    setBteDestinationSize(width, height);
    executeBte(0xC2);

    waitCompleted();
}

void RA8876::bteCopyNoWait(uint32_t srcAddress,
                           uint16_t srcX,
                           uint16_t srcY,
                           uint32_t destAddress,
                           uint16_t destX,
                           uint16_t destY,
                           uint16_t width,
                           uint16_t height)
{
    waitCompleted();

    setBteSource0Address(srcAddress);
    setBteSource0Position(srcX, srcY);
    setBteDestinationAddress(destAddress);
    setBteDestinationPosition(destX, destY);
    setBteDestinationSize(width, height);
    executeBte(0xC2);
}

void RA8876::bteCopyChroma(uint32_t srcAddress,
                           uint16_t srcX,
                           uint16_t srcY,
                           uint32_t destAddress,
                           uint16_t destX,
                           uint16_t destY,
                           uint16_t width,
                           uint16_t height)
{
    waitCompleted();

    setBteSource0Address(srcAddress);
    setBteSource0Position(srcX, srcY);
    setBteDestinationAddress(destAddress);
    setBteDestinationPosition(destX, destY);
    setBteDestinationSize(width, height);
    executeBte(0xC5);

    waitCompleted();
}

void RA8876::bteCopyRop(uint32_t src0Address,
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
                        uint8_t rop)
{
    setBteSource0Address(src0Address);
    setBteSource0Position(src0X, src0Y);
    setBteSource1Address(src1Address);
    setBteSource1Position(src1X, src1Y);
    setBteDestinationAddress(destAddress);
    setBteDestinationPosition(destX, destY);
    setBteDestinationSize(width, height);
    writeReg(RA8876_REG_BTE_APB,
             0x09); // This is alpha blending, not needed here
    executeBte(rop);

    waitCompleted();
}

void RA8876::bteExpansion(uint32_t srcAddress,
                          uint16_t srcX,
                          uint16_t srcY,
                          uint32_t destAddress,
                          uint16_t destX,
                          uint16_t destY,
                          uint16_t width,
                          uint16_t height,
                          uint32_t color)
{
    waitCompleted();

    setForegroundColor(color); // set the color for the expansion
    setBteSource0Address(srcAddress);
    setBteSource0Position(srcX, srcY);
    setBteDestinationAddress(destAddress);
    setBteDestinationPosition(destX, destY);
    setBteDestinationSize(width, height);
    writeReg(RA8876_REG_BTE_COLR, 0x05); // 8 bit S0
    executeBte(0xFF);

    waitCompleted();

    // Restore original BTE color depth.
    setBteColorDepth();
}

void RA8876::setBteChromaColor(uint32_t color)
{
    writeReg(RA8876_REG_BGCR, color >> 11 << 3);
    writeReg(RA8876_REG_BGCG, ((color >> 5) & 0x3F) << 2);
    writeReg(RA8876_REG_BGCB, (color & 0x1F) << 3);
}

void RA8876::setBacklight(boolean on)
{
    writeReg(RA8876_REG_DZ_LENGTH, 127); // set deadzone
    writeReg(RA8876_REG_PSCLR, 0); // set prescaler
    writeReg(RA8876_REG_PMUXR,
             0x00 | (on == true) ? 0x02 : 0x00); // route PWM output out
    writeReg(RA8876_REG_TCNTB0L, 0x00); // counter buffer low
    writeReg(RA8876_REG_TCNTB0H, 0x18); // counter buffer high
    writeReg(RA8876_REG_TCMPB0L, 0x00); // compare buffer low
    writeReg(RA8876_REG_TCMPB0H, 0x00); // compare buffer high
    writeReg(RA8876_REG_PCFGR, 0x0F); // enable timer with autoreload
}

void RA8876::setBacklightbrightness(uint16_t brightness)
{
    writeReg16(RA8876_REG_TCMPB0L, brightness);
}

void RA8876::setActiveWindowPosition(uint16_t x, uint16_t y)
{
    activeWindowX = x;
    activeWindowY = y;
    writeReg16(RA8876_REG_AWUL_X0, x);
    writeReg16(RA8876_REG_AWUL_Y0, y);
}

void RA8876::setActiveWindowSize(uint16_t width, uint16_t height)
{
    activeWindowWidth = width;
    activeWindowHeight = height;
    writeReg16(RA8876_REG_AW_WTH0, width);
    writeReg16(RA8876_REG_AW_HT0, height);
}

uint16_t RA8876::getActiveWindowX(void)
{
    return (activeWindowX);
}

uint16_t RA8876::getActiveWindowY(void)
{
    return (activeWindowY);
}

uint16_t RA8876::getActiveWindowWidth(void)
{
    return (activeWindowWidth);
}

uint16_t RA8876::getActiveWindowHeight(void)
{
    return (activeWindowHeight);
}

/** @attention every one of these two byte cycles is sent as a single 16 bit
 * frame rather than as two 8 bit transfers. The SPI peripheral stalls on the
 * transfer complete flag once per frame, so halving the number of frames
 * halves the stall count. The bytes on the wire are identical.
 */
void RA8876::writeCmd(uint8_t x)
{
    LCD_CS_CLEAR;
    SPI.transfer16(((uint16_t)RA8876_CMD_WRITE << 8) | x);
    LCD_CS_SET;
}

void RA8876::armMemoryWrite(void)
{
    writeCmd(RA8876_REG_MRWDP);
}

/**
 * @brief Streams a buffer into the memory port in one chip select cycle.
 *
 * @attention this used to walk the buffer in sixteen byte steps and transfer
 * four bytes at a time. That read past the end of the buffer whenever the
 * length was not a multiple of sixteen, and it also passed the caller's
 * buffer as both source and destination, so the received bytes overwrote it.
 * One transfer for the whole buffer is both correct and faster, because the
 * transmit queue is drained once instead of once per chunk.
 */
void RA8876::writeData(const uint8_t *p_src, size_t length)
{
    if ((p_src == nullptr) || (length == 0)) {
        return;
    }

    /* The display accepts bytes more slowly than the bus can deliver them, so
     * a burst is broken into pieces and the write FIFO is given the chance to
     * drain between them. Without this the controller silently drops roughly
     * three bytes in eight. */
    size_t remaining = length;
    const uint8_t *source = p_src;

    while (remaining > 0) {
        size_t chunk = remaining;

        if ((bulkWriteChunk != 0) && (chunk > bulkWriteChunk)) {
            chunk = bulkWriteChunk;
        }

        waitWriteFifo();

        LCD_CS_CLEAR;
        SPI.transfer(RA8876_DATA_WRITE);
        SPI.transfer(source, nullptr, chunk);
        LCD_CS_SET;

        source += chunk;
        remaining -= chunk;
    }

    waitWriteFifo();
}

/**
 * @brief Streams a buffer out of the memory port in one chip select cycle.
 *
 * The memory pointer advances by itself after every access, so a single read
 * cycle can carry any number of consecutive bytes.
 */
void RA8876::readDataBulk(uint8_t *p_dst, size_t length)
{
    if ((p_dst == nullptr) || (length == 0)) {
        return;
    }

    /* The controller cannot serve a continuous read at the clock it happily
     * accepts writes at: driven flat out it delivers only five bytes in
     * eight, and the ones it drops are indistinguishable from data. Reads
     * therefore run at their own, lower clock. Writes keep the full rate. */
    SPI.beginTransaction(SPISettings(readClock, MSBFIRST, SPI_MODE3));

    LCD_CS_CLEAR;
    SPI.transfer(RA8876_DATA_READ);
    SPI.transfer(nullptr, p_dst, length);
    LCD_CS_SET;

    SPI.beginTransaction(SPISettings(writeClock, MSBFIRST, SPI_MODE3));
}

/**
 * @brief Writes a block of bytes to the memory the address was set to.
 *
 * One command cycle arms the memory port, then the whole block travels in a
 * single data cycle. That is one SPI byte per stored byte, where writing the
 * same block a byte at a time costs four.
 */
void RA8876::writeRamBulk(const uint8_t *data, size_t length)
{
    armMemoryWrite();
    writeData(data, length);
}

/**
 * @brief Reads a block of bytes from the memory the address was set to.
 */
void RA8876::readRamBulk(uint8_t *data, size_t length)
{
    writeCmd(RA8876_REG_MRWDP);
    readDataBulk(data, length);
}

void RA8876::writeData(uint8_t x)
{
    LCD_CS_CLEAR;
    SPI.transfer16(((uint16_t)RA8876_DATA_WRITE << 8) | x);
    LCD_CS_SET;
}

uint8_t RA8876::readData(void)
{
    LCD_CS_CLEAR;
    uint16_t x = SPI.transfer16((uint16_t)RA8876_DATA_READ << 8);
    LCD_CS_SET;
    return ((uint8_t)(x & 0xFF));
}

// Reads the special status register.
// This register uses a special cycle type instead of having an address like other registers.
// See data sheet section 19.1.
uint8_t RA8876::readStatus(void)
{
    noInterrupts();
    LCD_CS_CLEAR;
    uint16_t x = SPI.transfer16((uint16_t)RA8876_STATUS_READ << 8);
    LCD_CS_SET;
    interrupts();

    return ((uint8_t)(x & 0xFF));
}

void RA8876::writeRegNBytes(uint8_t reg, uint8_t numBytes, uint32_t x)
{
    noInterrupts();

    for (uint8_t i = 0; i < numBytes; i++) {
        writeCmd(reg + i);
        writeData((x >> (i * 8)) & 0xFF);
    }

    interrupts();
}

void RA8876::writeReg(uint8_t reg, uint8_t x)
{
    writeRegNBytes(reg, 1, x);
}

void RA8876::writeReg16(uint8_t reg, uint16_t x)
{
    writeRegNBytes(reg, 2, x);
}

void RA8876::writeReg32(uint8_t reg, uint32_t x)
{
    writeRegNBytes(reg, 4, x);
}

uint32_t RA8876::readRegNBytes(uint8_t reg, uint8_t numBytes)
{
    uint32_t x = 0;

    noInterrupts();

    /** @attention the register number is advanced for every byte, exactly as
     * writeRegNBytes() does. Without it readReg16()/readReg32() read the same
     * low byte repeatedly, which silently corrupted saveState() and the
     * cursor accessors for any coordinate above 255.
     */
    for (uint8_t i = 0; i < numBytes; i++) {
        writeCmd(reg + i);
        x |= (readData() << (i * 8));
    }
    interrupts();

    return (x);
}

uint8_t RA8876::readReg(uint8_t reg)
{
    return (readRegNBytes(reg, 1) & 0xFF);
}

uint16_t RA8876::readReg16(uint8_t reg)
{
    return (readRegNBytes(reg, 2) & 0xFFFF);
}

uint32_t RA8876::readReg32(uint8_t reg)
{
    return (readRegNBytes(reg, 4));
}

void RA8876::setMemoryMode(MemoryMode mode)
{
    uint8_t colorDepth = 0x00;

    if (mode == LinearBpp8) {
        colorDepth = 0x04;
    } else if (mode == Bpp8) {
        colorDepth = 0x00;
    } else if (mode == Bpp16) {
        colorDepth = 0x01;
    } else if (mode == Bpp24) {
        colorDepth = 0x02;
    }

    m_memoryMode = mode;

    writeReg(RA8876_REG_AW_COLOR, colorDepth);
    waitCompleted();
}

void RA8876::setBteColorDepth(void)
{
    uint8_t colorDepth = 0;

    if (m_memoryMode == Bpp8) {
        colorDepth = 0x00;
    } else if (m_memoryMode == Bpp16) {
        colorDepth = 0x25;
    } else {
        colorDepth = 0x4A;
    }

    writeReg(RA8876_REG_BTE_COLR, colorDepth);
    waitCompleted();
}
