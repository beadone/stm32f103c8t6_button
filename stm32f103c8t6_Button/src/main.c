//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include "Trace.h"

#include "Timer.h"
#include "BlinkLed.h"
#include "ExceptionHandlers.h"
#include "stm32f10x_conf.h"
#include "misc.h"
#include "stm32f10x.h"
#define BWAKEUP             GPIO_Pin_0
#define BWAKEUPPORT         GPIOA
#define BWAKEUPPORTCLK      RCC_APB2Periph_GPIOA
#define BTAMPER             GPIO_Pin_15
#define BTAMPERPORT         GPIOC
#define BTAMPERPORTCLK      RCC_APB2Periph_GPIOC
#define BUSER1              GPIO_Pin_8
#define BUSER1PORT          GPIOA
#define BUSER1PORTCLK       RCC_APB2Periph_GPIOA
#define BUSER2              GPIO_Pin_1
#define BUSER2PORT          GPIOA
#define BUSER2PORTCLK       RCC_APB2Periph_GPIOA

 //   GPIO_EXTILineConfig(BWAKEUPPORTSOURCE, BWAKEUPPINSOURCE);
 //   GPIO_EXTILineConfig(BTAMPERPORTSOURCE, BTAMPERPINSOURCE);
 //   GPIO_EXTILineConfig(BUSER1PORTSOURCE, BUSER1PINSOURCE);
 //   GPIO_EXTILineConfig(BUSER2PORTSOURCE, BUSER2PINSOURCE);


#define LED1      GPIO_Pin_6
#define LED2      GPIO_Pin_7
#define LED3      GPIO_Pin_8
#define LED4      GPIO_Pin_9
#define LED5      GPIO_Pin_10
#define LEDPORT     GPIOA
#define LEDPORTCLK    RCC_APB2Periph_GPIOA
//function prototypes
void LEDsInit(void);
void LEDOn(uint32_t);
void LEDOff(uint32_t);
void LEDToggle(uint32_t);
uint32_t ButtonRead(GPIO_TypeDef* Button_Port, uint16_t Button);
void ButtonsInitEXTI(void);
void ButtonsInit(void);
void EXTI0_IRQHandler(void);
void EXTI3_IRQHandler(void);
void EXTI9_5_IRQHandle(void);
void EXTI15_10_IRQHandler(void);

#define NUM 10
 int i,j;
  char name[NUM+1] = {'\0'};
  ErrorStatus HSEStartUpStatus;

  uint8_t Usart1Get(void);
  void Usart1Put(uint8_t ch);
  void Usart1Init(void);
  void UART1Send(const unsigned char *pucBuffer, unsigned long ulCount);
  void RCC_Configuration(void);
  void Delay(__IO uint32_t nCount);
  void GPIO_Configuration(void);
  void InitializeTimer(void);
  void EnableTimerInterrupt(void);
  void TIM2_IRQHandler(void);
  void TIM3_IRQHandler(void);


  volatile int test1;
  volatile int test2;

// ----------------------------------------------------------------------------
//
// Standalone STM32F1 led blink sample (trace via NONE).
//
// In debug configurations, demonstrate how to print a greeting message
// on the trace device. In release configurations the message is
// simply discarded.
//
// Then demonstrates how to blink a led with 1 Hz, using a
// continuous loop and SysTick delays.
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the NONE output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace_impl.c
// (currently OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//
// The external clock frequency is specified as a preprocessor definition
// passed to the compiler via a command line option (see the 'C/C++ General' ->
// 'Paths and Symbols' -> the 'Symbols' tab, if you want to change it).
// The value selected during project creation was HSE_VALUE=8000000.
//
// Note: The default clock settings take the user defined HSE_VALUE and try
// to reach the maximum possible system clock. For the default 8 MHz input
// the result is guaranteed, but for other values it might not be possible,
// so please adjust the PLL settings in system/src/cmsis/system_stm32f10x.c
//

// ----- Timing definitions -------------------------------------------------

// Keep the LED on for 2/3 of a second.
#define BLINK_ON_TICKS  (TIMER_FREQUENCY_HZ * 3 / 4)
#define BLINK_OFF_TICKS (TIMER_FREQUENCY_HZ - BLINK_ON_TICKS)

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

int
main(int argc, char* argv[])
{


  const unsigned char msg1[] = " UART1 running\r\n";
  const unsigned char msg2[] = " UART1 info inside loop\r\n";
  test1=0;
  test2=0;
  RCC_Configuration();
  GPIO_Configuration();
  InitializeTimer();
  EnableTimerInterrupt();
  // Send a greeting to the trace device (skipped on Release).
  //trace_puts("Hello ARM World!");

  // At this stage the system clock should have already been configured
  // at high speed.
  //trace_printf("System clock: %u Hz\n", SystemCoreClock);

  timer_start();

  blink_led_init();
  Usart1Init();

  uint32_t seconds = 0;
// the old way to send info

  Usart1Put(0xD); // CR
  Usart1Put(0xA); //LF
  Usart1Put('S');
  Usart1Put('T');
  Usart1Put('A');
  Usart1Put('R');
  Usart1Put('T');
  Usart1Put(0xD); // CR
  Usart1Put(0xA); //LF

  UART1Send(msg1, sizeof(msg1));


  // Infinite loop
  while (1)
    {
      blink_led_on();
      timer_sleep(seconds == 0 ? TIMER_FREQUENCY_HZ : BLINK_ON_TICKS);

      blink_led_off();
      timer_sleep(BLINK_OFF_TICKS);

      ++seconds;
      UART1Send(msg2, sizeof(msg2));
      // Count seconds on the trace device.
      //trace_printf("Second %u\n", seconds);


    }
  // Infinite loop, never return.
}



void blink_led_init()
{
  // Enable GPIO Peripheral clock
  RCC_APB2PeriphClockCmd(BLINK_RCC_MASKx(BLINK_PORT_NUMBER), ENABLE);

  GPIO_InitTypeDef GPIO_InitStructure;

  // Configure pin in output push/pull mode
  GPIO_InitStructure.GPIO_Pin = BLINK_PIN_MASK(BLINK_PIN_NUMBER);
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(BLINK_GPIOx(BLINK_PORT_NUMBER), &GPIO_InitStructure);

  // Start with led turned off
  blink_led_off();
  // Enable clock for GPIOA

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
// button stuff
  // Cofigure PA0 as open-drain output

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  // Cofigure PA1 as input with internal pull-up resistor
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOA, &GPIO_InitStructure);



}

#if defined(USE_HAL_DRIVER)
void HAL_IncTick(void);
#endif

// Forward declarations.

void
timer_tick (void);

// ----------------------------------------------------------------------------

volatile timer_ticks_t timer_delayCount;

// ----------------------------------------------------------------------------

void
timer_start (void)
{
  // Use SysTick as reference for the delay loops.
  SysTick_Config (SystemCoreClock / TIMER_FREQUENCY_HZ);
}

void
timer_sleep (timer_ticks_t ticks)
{
  timer_delayCount = ticks;

  // Busy wait until the SysTick decrements the counter to zero.
  while (timer_delayCount != 0u)
    ;
}

void
timer_tick (void)
{
  // Decrement to zero the counter used by the delay routine.
  if (timer_delayCount != 0u)
    {
      --timer_delayCount;
    }
}

// ----- SysTick_Handler() ----------------------------------------------------

void
SysTick_Handler (void)
{
#if defined(USE_HAL_DRIVER)
  HAL_IncTick();
#endif
  timer_tick ();
}





void Usart1Init(void){


  GPIO_InitTypeDef GPIO_InitStructure;

  USART_InitTypeDef USART_InitStructure;

  USART_ClockInitTypeDef USART_ClockInitStructure;
  /* Enable clock for USART1, AFIO and GPIOA */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

  //Set USART1 Tx (PA.09) as AF push-pull
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
   /* GPIOA PIN9 alternative function Tx */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  //GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);




   /* Baud rate 9600, 8-bit data, One stop bit
    * No parity, Do both Rx and Tx, No HW flow control
    */
   /* USART configuration structure for USART1 */
   USART_ClockStructInit(&USART_ClockInitStructure);
    USART_ClockInit(USART1, &USART_ClockInitStructure);
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    //Write USART1 parameters
    USART_Init(USART1, &USART_InitStructure);

    //Enable USART1

    USART_Cmd(USART1, ENABLE);
    //set up the interupt



}
void Usart1Put(uint8_t ch)
{
      USART_SendData(USART1, (uint8_t) ch);
      //Loop until the end of transmission
      while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
      {
      }
}

uint8_t Usart1Get(void){
     while ( USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
        return (uint8_t)USART_ReceiveData(USART1);
}


void UART1Send(const unsigned char *pucBuffer, unsigned long ulCount)
{
    //
    // Loop while there are more characters to send.
    //
    while(ulCount--)
    {
        USART_SendData(USART1, *pucBuffer++);// Last Version USART_SendData(USART1,(uint16_t) *pucBuffer++);
        /* Loop until the end of transmission */
        while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
        {
        }
    }
}


void Delay(__IO uint32_t num)
{
  __IO uint32_t index = 0;

  /* default system clock is 72MHz */
  for(index = (72000 * num); index != 0; index--)
  {
  }
}

void RCC_Configuration(void)
{
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
}

void GPIO_Configuration(void)
{

  //timer interrupt led pins
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

  GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void InitializeTimer()
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  TIM_TimeBaseInitTypeDef timerInitStructure;

    timerInitStructure.TIM_Prescaler = 40000;
    timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    timerInitStructure.TIM_Period = 500;
    timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    timerInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &timerInitStructure);
    TIM_Cmd(TIM2, ENABLE);
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE); //automatically generate update


    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    timerInitStructure.TIM_Prescaler = 40000;
    timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    timerInitStructure.TIM_Period = 1000;
    timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    timerInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM3, &timerInitStructure);
    TIM_Cmd(TIM3, ENABLE);
  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE); //automatically generate update

}

void EnableTimerInterrupt()
{
    NVIC_InitTypeDef nvicStructure;

    nvicStructure.NVIC_IRQChannel = TIM2_IRQn;
    nvicStructure.NVIC_IRQChannelPreemptionPriority = 0;
    nvicStructure.NVIC_IRQChannelSubPriority = 1;
    nvicStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvicStructure);

    nvicStructure.NVIC_IRQChannel = TIM3_IRQn;
    nvicStructure.NVIC_IRQChannelPreemptionPriority = 0;
    nvicStructure.NVIC_IRQChannelSubPriority = 1;
    nvicStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvicStructure);

}

void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    if (test1)
      GPIO_SetBits(GPIOB, GPIO_Pin_12);
    else GPIO_ResetBits(GPIOB, GPIO_Pin_12);

    test1=!test1;
    }
}

void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    if (test2)
      GPIO_SetBits(GPIOB, GPIO_Pin_13);
    else GPIO_ResetBits(GPIOB, GPIO_Pin_13);

    test2=!test2;
    }
}

void ButtonsInitEXTI(void)
{
    //enable AFIO clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,  ENABLE);
    EXTI_InitTypeDef EXTI_InitStructure;
    //NVIC structure to set up NVIC controller
    NVIC_InitTypeDef NVIC_InitStructure;
    //GPIO structure used to initialize Button pins
    //Connect EXTI Lines to Button Pins

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource15);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource8);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);



    //select EXTI line0
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    //select interrupt mode
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    //generate interrupt on rising edge
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    //enable EXTI line
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    //send values to registers
    EXTI_Init(&EXTI_InitStructure);
    //select EXTI line13
    EXTI_InitStructure.EXTI_Line = EXTI_Line15;
    EXTI_Init(&EXTI_InitStructure);
    EXTI_InitStructure.EXTI_Line = EXTI_Line1;
    EXTI_Init(&EXTI_InitStructure);
    EXTI_InitStructure.EXTI_Line = EXTI_Line8;
    EXTI_Init(&EXTI_InitStructure);
    //disable AFIO clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,  DISABLE);
    //configure NVIC
    //select NVIC channel to configure
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
    //set priority to lowest
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
    //set subpriority to lowest
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
    //enable IRQ channel
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    //update NVIC registers
    NVIC_Init(&NVIC_InitStructure);
    //select NVIC channel to configure
    NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
    NVIC_Init(&NVIC_InitStructure);
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_Init(&NVIC_InitStructure);
    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_Init(&NVIC_InitStructure);
}


void LEDsInit(void)
{
  //GPIO structure used to initialize LED port
  GPIO_InitTypeDef GPIO_InitStructure;
  //Enable clock on APB2 pripheral bus where LEDs are connected
  RCC_APB2PeriphClockCmd(LEDPORTCLK,  ENABLE);
  //select pins to initialize LED
  GPIO_InitStructure.GPIO_Pin = LED1|LED2|LED3|LED4|LED5;
  //select output push-pull mode
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  //select GPIO speed
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(LEDPORT, &GPIO_InitStructure);
  //initially LEDs off
  GPIO_SetBits(LEDPORT, LED1|LED2|LED3|LED4|LED5);
}
void LEDOn(uint32_t LED_n)
{
  if (LED_n==1)
  {
    GPIO_ResetBits(LEDPORT, LED1);
  }
  if (LED_n==2)
  {
    GPIO_ResetBits(LEDPORT, LED2);
  }
  if (LED_n==3)
  {
    GPIO_ResetBits(LEDPORT, LED3);
  }
  if (LED_n==4)
  {
    GPIO_ResetBits(LEDPORT, LED4);
  }
  if (LED_n==5)
  {
    GPIO_ResetBits(LEDPORT, LED5);
  }
}
void LEDOff(uint32_t LED_n)
{
  if (LED_n==1)
  {
    GPIO_SetBits(LEDPORT, LED1);
  }
  if (LED_n==2)
  {
    GPIO_SetBits(LEDPORT, LED2);
  }
  if (LED_n==3)
  {
    GPIO_SetBits(LEDPORT, LED3);
  }
  if (LED_n==4)
  {
    GPIO_SetBits(LEDPORT, LED4);
  }
  if (LED_n==5)
  {
    GPIO_SetBits(LEDPORT, LED5);
  }
}
void LEDToggle(uint32_t LED_n)
{
  if (LED_n==1)
  {
    if(GPIO_ReadOutputDataBit(LEDPORT, LED1))  //toggle led
    {
      GPIO_ResetBits(LEDPORT, LED1); //set to zero
    }
       else
       {
         GPIO_SetBits(LEDPORT, LED1);//set to one
       }
  }
  if (LED_n==2)
  {
    if(GPIO_ReadOutputDataBit(LEDPORT, LED2))  //toggle led
    {
      GPIO_ResetBits(LEDPORT, LED2); //set to zero
    }
       else
       {
         GPIO_SetBits(LEDPORT, LED2);//set to one
       }
  }
  if (LED_n==3)
  {
    if(GPIO_ReadOutputDataBit(LEDPORT, LED3))  //toggle led
    {
      GPIO_ResetBits(LEDPORT, LED3); //set to zero
    }
       else
       {
         GPIO_SetBits(LEDPORT, LED3);//set to one
       }
  }
  if (LED_n==4)
  {
    if(GPIO_ReadOutputDataBit(LEDPORT, LED4))  //toggle led
    {
      GPIO_ResetBits(LEDPORT, LED4); //set to zero
    }
       else
       {
         GPIO_SetBits(LEDPORT, LED4);//set to one
       }
  }
  if (LED_n==5)
  {
    if(GPIO_ReadOutputDataBit(LEDPORT, LED5))  //toggle led
    {
      GPIO_ResetBits(LEDPORT, LED5); //set to zero
    }
       else
       {
         GPIO_SetBits(LEDPORT, LED5);//set to one
       }
  }
}

void ButtonsInit(void)
{
  //GPIO structure used to initialize Button pins
  GPIO_InitTypeDef GPIO_InitStructure;
  //Enable clock on APB2 pripheral bus where buttons are connected
  RCC_APB2PeriphClockCmd(BWAKEUPPORTCLK|BTAMPERPORTCLK|BUSER1PORTCLK|BUSER2PORTCLK,  ENABLE);
  //select pins to initialize WAKEUP and USER1 buttons
  GPIO_InitStructure.GPIO_Pin = BWAKEUP|BUSER1;
  //select floating input mode
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  //select GPIO speed
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(BWAKEUPPORT, &GPIO_InitStructure);
  //select pins to initialize TAMPER button
  GPIO_InitStructure.GPIO_Pin = BTAMPER;
  GPIO_Init(BTAMPERPORT, &GPIO_InitStructure);
  //select pins to initialize USER2 button
  GPIO_InitStructure.GPIO_Pin = BUSER2;
  GPIO_Init(BUSER2PORT, &GPIO_InitStructure);
}
uint32_t ButtonRead(GPIO_TypeDef* Button_Port, uint16_t Button)
{
  return !GPIO_ReadInputDataBit(Button_Port, Button);
}


void EXTI0_IRQHandler(void)
{

    //Check if EXTI_Line0 is asserted
    if(EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        LEDToggle(1);
    }
    //we need to clear line pending bit manually
    EXTI_ClearITPendingBit(EXTI_Line0);
}
void EXTI3_IRQHandler(void)
{

    //Check if EXTI_Line0 is asserted
    if(EXTI_GetITStatus(EXTI_Line1) != RESET)
    {
        LEDToggle(2);
    }
    //we need to clear line pending bit manually
    EXTI_ClearITPendingBit(EXTI_Line1);
}
void EXTI9_5_IRQHandle(void)
{

    //Check if EXTI_Line0 is asserted
    if(EXTI_GetITStatus(EXTI_Line8) != RESET)
    {
        LEDToggle(3);
    }
    //we need to clear line pending bit manually
    EXTI_ClearITPendingBit(EXTI_Line8);
}
void EXTI15_10_IRQHandler(void)
{

    //Check if EXTI_Line0 is asserted
    if(EXTI_GetITStatus(EXTI_Line15) != RESET)
    {
        LEDToggle(4);
    }
    //we need to clear line pending bit manually
    EXTI_ClearITPendingBit(EXTI_Line15);
}


#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
