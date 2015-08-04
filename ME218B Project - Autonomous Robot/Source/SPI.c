/**
* SPI.c
* This file contains the SPI initialization and end of Tx 
* interrupt service routine.
*/

#include <stdint.h>

#include "ES_Configure.h"
#include "ES_Framework.h"

#include "inc/hw_memmap.h"
#include "inc/hw_ssi.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "bitdefs.h"

#include "UpdateSM.h"

#include "SPI.h"

#define BITS_PER_NIBBLE 4
#define SSI_MUX 2
#define PRESCALE 100
#define CLOCK_RATE 49
//#define PRESCALE 1
//#define CLOCK_RATE 2

static uint8_t receiveData = 0xff; // module-level variable to store received data
static uint16_t count = 0;

/**
* Initializes SSI0 to function as SPI.
*/
void initSPI(void) {
	/* Enable clock to GPIO Ports A and F */
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R0;
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R5;

	/* Enable clock to SSI0 module */
	HWREG(SYSCTL_RCGCSSI) |= SYSCTL_RCGCSSI_R0;

	/* Wait for GPIO port to be ready */
	while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R0) != SYSCTL_PRGPIO_R0);

	/* Program SSI GPIO pins to use alternate functions*/
	HWREG(GPIO_PORTA_BASE+GPIO_O_AFSEL) |= (BIT2HI | BIT3HI | BIT4HI | BIT5HI);

	/* Select mux value (2) in GPIOPCTL to select SSI */
	/* First clear PA2, 3, 4, and 5 and then write 2 */
	HWREG(GPIO_PORTA_BASE+GPIO_O_PCTL) = (HWREG(GPIO_PORTA_BASE+GPIO_O_PCTL) & 0xff0000ff) 
											+ (SSI_MUX << (2*BITS_PER_NIBBLE)) 
											+ (SSI_MUX << (3*BITS_PER_NIBBLE)) 
											+ (SSI_MUX << (4*BITS_PER_NIBBLE)) 
											+ (SSI_MUX << (5*BITS_PER_NIBBLE));

	/* Program the port lines for digital I/O (PA2, 3, 4, and 5) */
	HWREG(GPIO_PORTA_BASE+GPIO_O_DEN) |= (BIT2HI | BIT3HI | BIT4HI | BIT5HI);

	/* Program the required data directions on the port lines 
	PA2 = SSI0Clk, PA3 = SSI0Fss, PA4 = SSI0Rx, and PA5 = SSI0Tx 
	(p. 954 in datasheet) */
	HWREG(GPIO_PORTA_BASE+GPIO_O_DIR) |= (BIT2HI | BIT3HI | BIT5HI);
	HWREG(GPIO_PORTA_BASE+GPIO_O_DIR) &= BIT4LO;
	
	//HWREG(GPIO_PORTA_BASE+GPIO_O_DEN) |= BIT6HI;
	//HWREG(GPIO_PORTA_BASE+GPIO_O_DIR) |= BIT6HI; // use for manual Fss
	//HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA+ALL_BITS)) |= BIT6HI; // set manual Fss high
	//HWREG(GPIO_PORTA_BASE+GPIO_O_ODR) |= BIT6HI;
	
	HWREG(GPIO_PORTF_BASE+GPIO_O_DEN) |= BIT4HI;
	HWREG(GPIO_PORTF_BASE+GPIO_O_DIR) |= BIT4HI; // use for manual Fss
	HWREG(GPIO_PORTF_BASE+(GPIO_O_DATA+ALL_BITS)) |= BIT4HI; // set manual Fss high
	HWREG(GPIO_PORTF_BASE+GPIO_O_ODR) |= BIT4HI;

	/* Program pull-ups on clock and receive lines */
	//HWREG(GPIO_PORTA_BASE+GPIO_O_PUR) |= (BIT2HI | BIT4HI);
	HWREG(GPIO_PORTA_BASE+GPIO_O_PUR) |= BIT2HI;
	
	/* Program pull-down on receive line */
	HWREG(GPIO_PORTA_BASE+GPIO_O_PDR) |= BIT4HI;
	
	/* Set the SSI0Fss line to be open-drain */
	HWREG(GPIO_PORTA_BASE+GPIO_O_ODR) |= BIT3HI;

	/* Wait for SSI0 to be ready */
	while ((HWREG(SYSCTL_PRSSI) & SYSCTL_PRSSI_R0) != SYSCTL_PRSSI_R0);

	/* Disable SSI before programming mode bits */
	HWREG(SSI0_BASE+SSI_O_CR1) &= ~SSI_CR1_SSE;

	/* Select master mode (MS) and TXRIS to indicate end
	of transmit (EOT) - from Tiva to Command Generator */
	HWREG(SSI0_BASE+SSI_O_CR1) &= ~SSI_CR1_MS; // master mode
	HWREG(SSI0_BASE+SSI_O_CR1) |= SSI_CR1_EOT; // EOT interrupt	

	/* Configure SSI clock source to system clock */
	HWREG(SSI0_BASE+SSI_O_CC) = (HWREG(SSI0_BASE+SSI_O_CC) & ~SSI_CC_CS_M) | SSI_CC_CS_SYSPLL;

	/* Configure the clock pre-scaler; must be an even number (p. 976) */
	HWREG(SSI0_BASE+SSI_O_CPSR) = (HWREG(SSI0_BASE+SSI_O_CPSR) & ~SSI_CPSR_CPSDVSR_M) + PRESCALE;

	/* Configure the clock rate */
	/* BR = SysClk/(PRESCALE * (1 + CLOCK_RATE)) */
	HWREG(SSI0_BASE+SSI_O_CR0) = (HWREG(SSI0_BASE+SSI_O_CR0) & ~SSI_CR0_SCR_M) | 
									(CLOCK_RATE << (2*BITS_PER_NIBBLE)); // bit rate = 40 MHz/(100*(1+49)) = 8 kHz

	/* Configure the phase, polarity, mode, and data size */
	HWREG(SSI0_BASE+SSI_O_CR0) |= (SSI_CR0_SPH | SSI_CR0_SPO); // phase = 1, polarity = 1
	HWREG(SSI0_BASE+SSI_O_CR0) = (HWREG(SSI0_BASE+SSI_O_CR0) & ~(SSI_CR0_FRF_M | SSI_CR0_DSS_M)) | 
									(SSI_CR0_FRF_MOTO | SSI_CR0_DSS_8); // SPI mode and 8-bit data

	/* Locally enable interrupts on TXRIS */
	HWREG(SSI0_BASE+SSI_O_IM) |= SSI_IM_TXIM;
	
	/* Enable the SSI0 NVIC */
	/* It is interrupt number 7, so it appears in EN0 at bit 7  */
	/* Refer to pp. 104 and 142 in datasheet */
	HWREG(NVIC_EN0) = BIT7HI;
	
	/* Globally enable interrupts */
	__enable_irq();

	/* Enable SSI0 */
	HWREG(SSI0_BASE+SSI_O_CR1) |= SSI_CR1_SSE;
}

/**
* This is the EOT interrupt service routine (ISR).
*/
void transmitEndResponse(void) {
	/* Mask EOT interrupt */
	HWREG(SSI0_BASE+SSI_O_IM) &= ~SSI_IM_TXIM;

	/* Now grab the data sent by the slave from the data register */
	receiveData = HWREG(SSI0_BASE+SSI_O_DR);
	
	/* Post EndOfTransmit event */
	ES_Event ThisEvent;
	ThisEvent.EventType = EndOfTransmit;
	PostUpdateSM(ThisEvent);
	
	count++; // increment count
	//printf("\r\n Z");
}

/**
* Gets the data last received from Command Generator.
* @return receiveData
*/
uint8_t getReceiveData(void) {
	return receiveData;
}

/**
* Sends data to slave by writing to the data register.
* Also unmasks the interrupt right after writing data.
* @param data: the 8-bit data to send
*/
void sendData(uint8_t data) {
	HWREG(SSI0_BASE+SSI_O_DR) = data;
	HWREG(SSI0_BASE+SSI_O_IM) |= SSI_IM_TXIM; // unmask the EOT interrupt
}

/**
* Returns the interrupt count.
* @return count
*/
uint16_t getCount(void) {
	return count;
}
