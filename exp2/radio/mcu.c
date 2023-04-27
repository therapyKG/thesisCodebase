#include <msp430.h>
#include <stdint.h>
#include "mcu.h"

int cycle_in_us = 0;

void mcu_init(int freq) {
  cycle_in_us = (int)(freq/1000);
}

void mcu_delayms(uint32_t ms) {
  while (ms) {
    //const unsigned long temp = cycle_in_us * 998;
    __delay_cycles(1000);
  	ms--;
  }
}

void mcu_delayus(uint32_t us) {
	while (us) {
		__delay_cycles(1);
		us--;
  }
}

void mcu_memcpy1(uint8_t *dst, const uint8_t *src, uint16_t size) {
    while(size--) *dst++ = *src++;
}
