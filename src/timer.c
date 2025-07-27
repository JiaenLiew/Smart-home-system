// timer.c
#include "lpc24xx.h"

volatile unsigned int seconds = 0;

// Forward declaration of ISR
__irq void timer_ISR(void);

void init_timer(void) {
    T0TCR = 0x02;           // Reset Timer
    T0PR  = 58000000 - 1;   // Prescaler for 1Hz (assuming 58 MHz PCLK)
    T0MR0 = 1;              // Match every second
    T0MCR = 0x03;           // Interrupt + Reset on MR0
    T0TCR = 0x01;           // Enable Timer

    // Set interrupt vector slot 4 manually
    *((volatile unsigned long*) 0xFFFFF110) = (unsigned long)timer_ISR;  // VICVectAddr4
    *((volatile unsigned long*) 0xFFFFF210) = 0x20 | 4;                   // VICVectCntl4 (enable + Timer0)
    VICIntEnable = (1 << 4);                                             // Enable Timer0 interrupt
}


__irq void timer_ISR(void) {
    seconds++;
    T0IR = 1;               // Clear interrupt flag
    VICVectAddr = 0;        // Acknowledge interrupt
}
