#include <stdio.h>
#include <stdint.h>

static int __attribute__((section(".ccmram"))) cnt;
#define NVIC_ISER_BASE	(0XE000E100)
#define NVIC_ICER_BASE	(0XE000E180)
#define NVIC_IPRX_BASE	(0xE000E400)
/* Flash config address*/
#define FLASH_ACR_BASE	(0x40023C00)
/* Clock config address */
#define RCC_BASE	(0x40023800)
#define RCC_CR 		(*(volatile uint32_t *)(RCC_BASE + 0x00))	
#define RCC_PLLCFGR	(*(volatile uint32_t *)(RCC_BASE + 0x04))
#define RCC_CFGR	(*(volatile uint32_t *)(RCC_BASE + 0x08))
/* Peripherals registers */
#define RCC_APB1ENR	(*(volatile uint32_t *)(0x40023840))
#define RCC_APB2ENR	(*(volatile uint32_t *)(0x40023844))
#define RCC_AHB1ENR	(*(volatile uint32_t *)(RCC_BASE + 0x30))
/* SysTick register */
#define STK_BASE	(0xE000E010)
#define STK_CSR		(*(volatile uint32_t *)(STK_BASE))
#define STK_LOAD	(*(volatile uint32_t *)(STK_BASE + 0x04))
#define STK_VAL		(*(volatile uint32_t *)(STK_BASE + 0x08))
#define STK_CALIB	(*(volatile uint32_t *)(STK_BASE + 0x0C))

/* RCC bit definition */
#define RCC_CR_PLLRDY 	(1 << 25)
#define RCC_CR_PLLON 	(1 << 24)
#define RCC_CR_HSERDY 	(1 << 17)
#define RCC_CR_HSEON 	(1 << 16)
#define RCC_CR_HSIRDY 	(1 << 1)
#define RCC_CR_HSION 	(1 << 0)
#define RCC_CFGR_SW_HSI	0x0
#define RCC_CFGR_SW_HSE	0x1
#define RCC_CFGR_SW_PLL	0x2
#define RCC_PLLCFGR_PLLSRC (1 << 22)
#define RCC_PRESCALER_DIV_NONE 0
#define RCC_PRESCALER_DIV_2 8
#define RCC_PRESCALER_DIV_4 9

#define CPU_FREQ (168000000)
#define PLL_FULL_MASK (0x7F037FFF)
#define PLLM 8
#define PLLN 336
#define PLLP 2
#define PLLQ 7
#define PLLR 0

/* GPIO Registers */
#define GPIOE_BASE	(0x40021000)
#define GPIOE_MODER	(*(volatile uint32_t *)(GPIOE_BASE + 0x00))
#define GPIOE_OTYPER	(*(volatile uint32_t *)(GPIOE_BASE + 0x04))
#define GPIOE_OSPEEDR	(*(volatile uint32_t *)(GPIOE_BASE + 0x08))
#define GPIOE_PUPDR	(*(volatile uint32_t *)(GPIOE_BASE + 0x0C))
#define GPIOE_ODR	(*(volatile uint32_t *)(GPIOE_BASE + 0x14))
#define GPIOE_BSRR	(*(volatile uint32_t *)(GPIOE_BASE + 0x18))

/* MACRO's */
/* Memory barrier, flush */
#define DMB() asm volatile ("dmb");
/* Wait for interrupt */
#define WFI()	asm volatile ("wfi");
/* NVIC Enable/Disable */
#define USR_LED_PORT	GPIOE_BASE
#define USR_LED_PIN	7
static inline void nvic_irq_enable(uint8_t n)
{
	uint8_t i = n/32;
	volatile uint32_t * nvic_reg = (volatile uint32_t *)(NVIC_ISER_BASE + 4 * i);
	*nvic_reg |= (1<< n%4);
}

static inline void nvic_irq_disable(uint8_t n)
{
	uint8_t i = n/32;
	volatile uint32_t * nvic_reg = (volatile uint32_t *)(NVIC_ICER_BASE + 4 * i);
	*nvic_reg |= (1<< n%4);
}

/**
 * n: Interrupt number 0 -239 for F4x
 * prio: Priority 0 - 255, but only [7:4] bits are effective
 */
static inline void nvic_irq_set_priority(uint8_t n, uint8_t prio)
{
	volatile uint32_t* nvic_reg = ((volatile uint32_t*)(NVIC_IPRX_BASE + n)) ;
	*nvic_reg = prio;
}

/**
 * Set delay based on CPU Freq (HCLK), for more info see table 11 in ref
 * Defualt value for 168Mhz --> 5 WS.
 * Enable Instruction (BIT 9) and Data (BIT 10) cache.
 */
static void flash_set_waitstates(uint8_t delay)
{
	volatile uint32_t * flash_acr = ((volatile uint32_t *)(FLASH_ACR_BASE + 0x00));
	*flash_acr |= (delay & 0x07)| (1 << 9) | (1 << 10);	

}

static void rcc_config(void)
{
	RCC_CR |= RCC_CR_HSION;
	DMB();
	/* Enable internal oscilator and wait until it get released. */
	while ((RCC_CR & RCC_CR_HSIRDY) == 0)
		;
	/* Temporily Select HSI as main clock */
	uint32_t reg32;
	reg32 = RCC_CFGR;
	reg32 &= ~((1 << 1) | (1 << 0));
	RCC_CFGR = (reg32 | RCC_CFGR_SW_HSI);
	DMB();

	/* Activate HSE */
	RCC_CR |= RCC_CR_HSEON;
	DMB();
	while((RCC_CR & RCC_CR_HSERDY) == 0)
		;
	/* Set Clock */
	reg32 = RCC_CFGR;
	reg32 &= ~0xF0;
	RCC_CFGR = (reg32 | (RCC_PRESCALER_DIV_NONE << 4));
	DMB();
	reg32 = RCC_CFGR;
	reg32 &= ~0x1C00;
	RCC_CFGR = (reg32 | (RCC_PRESCALER_DIV_2 << 10));
	DMB();
	reg32 = RCC_CFGR;
	reg32 &= ~0x07 << 13;
	RCC_CFGR = (reg32 | (RCC_PRESCALER_DIV_4 << 13));
	DMB();
	/* PLL setup */
	reg32 = RCC_PLLCFGR;
	reg32 &= ~PLL_FULL_MASK;
	RCC_PLLCFGR = reg32 | RCC_PLLCFGR_PLLSRC | PLLM |
		(PLLN << 6) | (((PLLP >> 1) - 1) << 16) |
		(PLLQ << 24);
	DMB();

	/* Activate PLL */
	RCC_CR |= RCC_CR_PLLON;
	DMB();
	while ((RCC_CR & RCC_CR_PLLRDY) == 0);

	reg32 = RCC_CFGR;
	reg32 &= ~((1 << 1) | (1 << 0));
	RCC_CFGR = (reg32 | RCC_CFGR_SW_PLL);
	DMB();
	while ((RCC_CFGR & ((1 << 1) | (1 << 0))) !=
			RCC_CFGR_SW_PLL);

	/* Activate clock for USART1 */
	/*reg32 = *(volatile uint32_t *)RCC_APB2ENR;
	reg32 |= (1<< 4);
	RCC_APB2ENR = reg32;
	*/
}

static void systick_enable(void)
{
	volatile uint32_t reg32;
	reg32 = ((CPU_FREQ/1000) -1);
	STK_LOAD = reg32;
	/* Clear current value */
	STK_VAL = 0x00;
	/* Set Processor clock as clock source
	 * No exception is required
	 * Enable the SYSTICK
	 */
	STK_CSR |= ((1 << 2) | (1 << 1) | (1 << 0));

}
volatile uint32_t jiffies = 0;
void isr_systick(void)
{
	++jiffies;
}

void usr_led_config(void)
{
	/* USER LED is connected to GPIOE.7 */
	/* Enable GPIOE Clock on AHB1 */ 
	RCC_AHB1ENR |= (1 << 4);
	/* Set GPIOE.7 as output */
	GPIOE_MODER &= ~(0x03 << USR_LED_PIN*2);
	GPIOE_MODER |= (1 << USR_LED_PIN*2);
	/* Set mode to Push-Pull */
	GPIOE_OTYPER |= (0 << 4);
	/* Low speed IO */
	GPIOE_OSPEEDR &= (0x03 << USR_LED_PIN*2);
	/* GPIOE_OSPEEDR |= (0 << 6) | (0 << 7);*/
	/* Pulldown config */
	GPIOE_PUPDR &= ~(0x03 << USR_LED_PIN*2);
	GPIOE_PUPDR |= (0x04 << USR_LED_PIN*2);//(1 << 15) | (0 << 14);
	/* Turn it on/off with atomic bit more BSRR register */
	//GPIOE_BSRR = (1 << 7);
}
static inline __attribute__((always_inline)) void usr_led_on(void)
{
	GPIOE_BSRR = (1 << 7);
}
static inline __attribute__((always_inline)) void usr_led_off(void)
{
	GPIOE_BSRR = ( 1 << (23));
}
int main(void)
{
	flash_set_waitstates(5);
	rcc_config();
	systick_enable();
	usr_led_config();
	usr_led_on();
	while(1)
	{
	//	cnt++;
	WFI();
	}
	return 0;
}
