#pragma once

#include <stdint.h>

#define __NOP(X)  for(uint8_t i = 0; i < X; i++) { __asm__ volatile ("nop"); }


#define LCD_CS_CLEAR GPIOD_PCOR = 0x80
#define LCD_CS_SET GPIOD_PSOR = 0x80

#define LCD_RESET_CLEAR GPIOD_PCOR = 0x10
#define LCD_RESET_SET GPIOD_PSOR = 0x10

#define POT_TOUCH_ENABLE GPIOD_PSOR = 0x800
#define POT_TOUCH_DISABLE GPIOD_PCOR = 0x800

#define MUX_ENABLE      GPIOB_PCOR = 0x10; __NOP(20);
#define MUX_DISABLE     GPIOB_PSOR = 0x10;
#define MUX_SET_ADDRESS(X)  GPIOB_PDOR = (GPIOB_PDOR & ~0xF) | (X & 0xF);
#define MUX_IS_PARKED_AT(X) ((GPIOB_PDOR & 0x1F) == ((X) & 0xF))

#define GET_POT_TOUCH_INT ((GPIOA_PDIR & 0x4000000) ? 1 : 0)

#define GET_BUTTON_STATE ((GPIOA_PDIR & 0x20000000) ? 1 : 0)

#ifdef __cplusplus
extern "C" {
#endif

void configurePins(void);

#ifdef __cplusplus
}
#endif


// For the compatibility with the Arduino libraries
const static uint8_t A10 = 64;
const static uint8_t SS = 10;



#define NUM_DIGITAL_PINS  CORE_NUM_DIGITAL
#define NUM_ANALOG_INPUTS CORE_NUM_ANALOG


#define NOT_AN_INTERRUPT -1

#define digitalPinToInterrupt(p)  ((p) < NUM_DIGITAL_PINS ? (p) : -1)




struct digital_pin_bitband_and_config_table_struct {
        volatile uint32_t *reg;
        volatile uint32_t *config;
};

extern const struct digital_pin_bitband_and_config_table_struct digital_pin_to_info_PGM[];

// compatibility macros
#define digitalPinToPort(pin) (pin)
#define digitalPinToBitMask(pin) (1)
#define portOutputRegister(pin) ((volatile uint8_t *)(digital_pin_to_info_PGM[(pin)].reg + 0))
#define portSetRegister(pin)    ((volatile uint8_t *)(digital_pin_to_info_PGM[(pin)].reg + 32))
#define portClearRegister(pin)  ((volatile uint8_t *)(digital_pin_to_info_PGM[(pin)].reg + 64))
#define portToggleRegister(pin) ((volatile uint8_t *)(digital_pin_to_info_PGM[(pin)].reg + 96))
#define portInputRegister(pin)  ((volatile uint8_t *)(digital_pin_to_info_PGM[(pin)].reg + 128))
#define portModeRegister(pin)   ((volatile uint8_t *)(digital_pin_to_info_PGM[(pin)].reg + 160))
#define portConfigRegister(pin) ((volatile uint32_t *)(digital_pin_to_info_PGM[(pin)].config))
#define digitalPinToPortReg(pin) (portOutputRegister(pin))
#define digitalPinToBit(pin) (1)


#define SerialUSB			Serial
