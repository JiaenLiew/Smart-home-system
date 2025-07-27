#include "doorbell.h"
#include "touch.h"
#include "lcd/lcd_grph.h"
#include "lpc24xx.h"
#include <stdio.h>

int main(void) {
	volatile int d = 0;

  	PINSEL1 &= ~(0x3 << 20);  // DAC P0.26
	PINSEL1 |=  (0x2 << 20);
	FIO0DIR &= ~(1 << 10);    // P0.10 doorbell button

	// Initialise LCD and Touchscreen
	init_lcd();
	touch_init();
	lcd_fillScreen(BLACK);
	lcd_fontColor(WHITE, BLACK);


	while (1) {
			doorbell();
	}
}

