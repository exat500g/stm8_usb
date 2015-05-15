/*
 * File: main.c
 * Date: 02.01.2013
 * Denis Zheleznyakov aka ZiB @ http://ziblog.ru
 */

#include "main.h"

uint32_t watchdog_timeout=0;
uint32_t reset_timeout=0;
#define MAX_WDG_TIMEOUT 300000

void delay(uint32_t n)
{
	unsigned int i;
	while(n>0)
	{
		for(i=0;i<3000;i++)
			;
		n--;
	}
}

@inline static void usb_reset(void){
	GPIOC->DDR = 0xFF;	//output
	GPIOC->CR1 = 0xFF;	//PP output when DDR=1
	GPIOC->CR2 = 0;
	GPIOC->ODR = 0x00;
}

@inline static void gpio_init(void)
{
	//input pull up
	GPIOA->CR1 = 0xFF;
	GPIOB->CR1 = 0xFF;
	GPIOC->CR1 = 0xFF;
	GPIOD->CR1 = 0xFF;
	
	// Входные линии USB
	GPIOC->DDR = 0x3F;
	GPIOC->ODR = 0x00;
	GPIOC->CR1 = 0x3F;
	GPIOC->CR2 = 0;
}

@inline static void clock_init(void)
{
	//use 12Mhz HSE
	CLK_ClockSwitchConfig(CLK_SWITCHMODE_AUTO, CLK_SOURCE_HSE, DISABLE, CLK_CURRENTCLOCKSTATE_DISABLE);
}

@inline static void timers_init(void)
{
	CLK->PCKENR1 |= CLK_PCKENR1_TIM1;

	// 定时器1 - “捕获”的第一个脉冲的USB包裹
	TIM1_TimeBaseInit(0, TIM1_PSCRELOADMODE_UPDATE, 1000, 0);
	// 信号采集，通过USB D-
	TIM1_ICInit(TIM1_CHANNEL_2, TIM1_ICPOLARITY_RISING, TIM1_ICSELECTION_DIRECTTI, TIM1_ICPSC_DIV1, 0x02);
	TIM1_SelectInputTrigger(TIM1_TS_TI2FP2);
	TIM1_SelectSlaveMode(TIM1_SLAVEMODE_TRIGGER);
	TIM1_ClearFlag(TIM1_FLAG_CC2);
	TIM1_ITConfig(TIM1_IT_CC2, ENABLE);
}
@inline static void IWDG_Config(void)
{
    IWDG_Enable();
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    //LSI=128kHz =>/2 =>64kHz => IWDG_PR(7bit Prescaler) => IWDG_RLR(8bit Counter) => WDG_RESET
    //Detail @ RM14.2
    IWDG_SetPrescaler(IWDG_Prescaler_256);  //64kHz / 256 = 250Hz = 4ms/tick
    IWDG_SetReload(255);  //250*4ms = 1.0s timeout
    IWDG_ReloadCounter();
}

uint8_t data_buffer[10]={0};
@inline static void send_key(uint8_t key){
    data_buffer[0] = key;
    usb_prepare_data(&data_buffer[0],1,0);
}

void main(void)
{
	disableInterrupts();
	CLK->CKDIVR = 0;	//cpu=16M
	usb_reset();		//set USB_DP+- =0
	clock_init();		//cpu=12M
	delay(100);         //set USB_DP=0 for a while to reset usb
	gpio_init();		//init usb gpio
	timers_init();		//enable usb timer
	IWDG_Config();
    usb_init();			//init usb flag
	enableInterrupts();
	while(usb.enum_complete==0){
        usb_process();
        if( (GPIOC->IDR&0xC0) == 0){    //detect host reset
            if(reset_timeout<1000){
                reset_timeout++;
            }else{
                reset_timeout=0;
                usb_init();
            }
        }
        //extend watchdog reset time
        if(watchdog_timeout<MAX_WDG_TIMEOUT){
            watchdog_timeout++;
            IWDG->KR = IWDG_KEY_REFRESH;    //feed watchdog
        }
    }
    delay(10);
    while(1){
        send_key(0x1E);     //will block until USR_PID_IN
    }
}

