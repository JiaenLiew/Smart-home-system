#include "touch.h"
#include "lpc24xx.h"

#define CS_PIN            0x00100000        //P0.20
#define CS_LOW()          IOCLR0 = CS_PIN
#define CS_HIGH()         IOSET0 = CS_PIN

// Control bytes 
#define X_POSITION     0xD8  // 11011000
#define Y_POSITION     0x98  // 10011000

// Control bytes for pressure
#define Z1_POSITION    0xB0  // 10110000
#define Z2_POSITION    0xC0  // 11000000

static unsigned char touch_read(unsigned char command);
static int pressure = 0;

void touch_init(void)
{
	//Implement this as you see fit
	//Remember to setup CS_TP as a GPIO output
	
	// Set CS_TP (P0.20) as GPIO output and set high (inactive)
	IODIR0 |= CS_PIN;
	CS_HIGH();

	// Configure SPI pins
	// P0.15 (SCK), P0.17 (MISO), P0.18 (MOSI)
	PINSEL0 |= (3 << 30); // P0.15 = SCK (function 3)
	PINSEL1 |= (3 << 2) | (3 << 4); // P0.17 = MISO, P0.18 = MOSI

	// Configure SPI controller
	S0SPCCR = 0x24;      // SPI clock = PCLK / 36 = 2 MHz
	S0SPCR = 0x093C;     // 9-bit transfer, master, CPOL=1, CPHA=1, MSB first
}

void touch_read_xy(char *x, char* y)
{
	
	unsigned char z1 = touch_read(Z1_POSITION);
	unsigned char z2 = touch_read(Z2_POSITION);

	if (z1 > 0 && z2 > 0 && z2 > z1) {
		pressure = (z2 / z1) - 1;
		if (pressure < 0) pressure = 0;
	} else {
		pressure = 0;
	}

	if (pressure > 1) {
		*x = touch_read(X_POSITION);
		*y = touch_read(Y_POSITION);
	} 
}

int touch_get_pressure(void)
{
	return pressure;
}	

static unsigned char touch_read(unsigned char command)
{
	unsigned short result;

	//Set CS_TP pin low to begin SPI transmission
	CS_LOW();
	
	//Transmit command byte on MOSI, ignore MISO (full read write cycle)
	S0SPDR = command;
	while (!(S0SPSR & (1 << 7))); // Wait for SPIF
  (volatile unsigned char)S0SPDR; // Dummy read to clear SPIF
	
	//Transmit 0x00 on MOSI, read in requested result on MISO (another full read write cycle)
	S0SPDR = 0x00;
	while (!(S0SPSR & (1 << 7))); // Wait for SPIF
	result = S0SPDR;
	
	//Transmission complete, set CS_TP pin back to high
	CS_HIGH();
	
	//Return 8 bit result.
	return (unsigned char) result;
}	

