//#include <stdio.h>
#include <stdint.h>

//CCMRAM
//
extern unsigned int _stored_ccmram;
extern unsigned int _start_ccmram;
extern unsigned int _end_ccmram;
extern unsigned int _stored_data;
extern unsigned int _start_data;
extern unsigned int _end_data;
extern unsigned int _start_bss;
extern unsigned int _end_bss;

extern void *END_STACK;

void main(void);
void isr_reset(void)
{
	unsigned int *src, *dst;
	
	src = (unsigned int *) &_stored_ccmram;
	dst = (unsigned int *) &_start_ccmram;
	while (dst < (unsigned int *)&_end_ccmram)
	{
		*dst = *src;
		src++;
		dst++;
	}
	/////////////
	src = (unsigned int *) &_stored_data;
	dst = (unsigned int *) &_start_data;
	while (dst < (unsigned int*)&_end_data) {
		*dst = *src;
		dst++;
		src++;
	}

	src = (unsigned int*) &_start_bss;
	while(dst < (unsigned int *) &_end_bss)
	{
		*dst = 0U;
		dst++;
	}
	/* Jump to main */
	main();
}

void isr_nmi(void)
{

}

void __attribute((weak)) isr_fault()
{
	while(1){}
}
void __attribute__((weak)) isr_empty()
{
	while(1){}
}

void __attribute__((weak)) isr_systick()
{
}
/*
void main(void)
{
	// Find app end stack
	uint32_t app_end_stack = *((uint32_t *)(APPOFFSET));
	// Find the APP entry
	void (*app_entry)(void);
	app_entry = (void*)(*((uint32_t *)(APPOFFSET + 4)));

	// Disable all interrupts
	asm volatile ("cpsid i");

	// Reset MSP to the end of the stack for the main app
	asm volatile ("msr msp, %0" :: "r"(app_end_stack));
	// Load the VTOR register with the new addres of the ISR
	uint32_t* VTOR = (uint32_t *)(0xE000ED08);
	(*VTOR) = (uint32_t )(APPOFFSET);
	asm volatile ("cpsie i");
	asm volatile ("mov pc, %0" :: "r"(app_entry));
	// Enable all interrupts
}
*/
/////////////////
__attribute__ ((section(".isr_vector")))
void (* const IV[])(void) = 
{
	(void (*)(void))(&END_STACK),
	isr_reset,                   // Reset
	isr_fault,                   // NMI
	isr_fault,                   // HardFault
	isr_fault,                   // MemFault
	isr_fault,                   // BusFault
	isr_fault,                   // UsageFault
	0, 0, 0, 0,                  // 4x reserved
	isr_empty,                   // SVC
	isr_empty,                   // DebugMonitor
	0,                           // reserved
	isr_empty,                   // PendSV
	isr_systick,                   // SysTick

	isr_empty,                     // GPIO Port A
	isr_empty,                     // GPIO Port B
	isr_empty,                     // GPIO Port C
	isr_empty,                     // GPIO Port D
	isr_empty,                     // GPIO Port E
	isr_empty,                     // UART0 Rx and Tx
	isr_empty,                     // UART1 Rx and Tx
	isr_empty,                     // SSI0 Rx and Tx
	isr_empty,                     // I2C0 Master and Slave
	isr_empty,                     // PWM Fault
	isr_empty,                     // PWM Generator 0
	isr_empty,                     // PWM Generator 1
	isr_empty,                     // PWM Generator 2
	isr_empty,                     // Quadrature Encoder 0
	isr_empty,                     // ADC Sequence 0
	isr_empty,                     // ADC Sequence 1
	isr_empty,                     // ADC Sequence 2
	isr_empty,                     // ADC Sequence 3
	isr_empty,                     // Watchdog timer
	isr_empty,                     // Timer 0 subtimer A
	isr_empty,                     // Timer 0 subtimer B
	isr_empty,                     // Timer 1 subtimer A
	isr_empty,                     // Timer 1 subtimer B
	isr_empty,                     // Timer 2 subtimer A
	isr_empty,                     // Timer 3 subtimer B
	isr_empty,                     // Analog Comparator 0
	isr_empty,                     // Analog Comparator 1
	isr_empty,                     // Analog Comparator 2
	isr_empty,                     // System Control (PLL, OSC, BO)
	isr_empty,                     // FLASH Control
	isr_empty,                     // GPIO Port F
	isr_empty,                     // GPIO Port G
	isr_empty,                     // GPIO Port H
	isr_empty,                     // UART2 Rx and Tx
	isr_empty,                     // SSI1 Rx and Tx
	isr_empty,                     // Timer 3 subtimer A
	isr_empty,                     // Timer 3 subtimer B
	isr_empty,                     // I2C1 Master and Slave
	isr_empty,                     // Quadrature Encoder 1
	isr_empty,                     // CAN0
	isr_empty,                     // CAN1
	isr_empty,                     // CAN2
	isr_empty,                     // Ethernet
	isr_empty,                     // Hibernate
};

/*int main(void)
  {
  IV[1]();
  ist_reset();
  }
  */
