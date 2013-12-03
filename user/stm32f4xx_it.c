/**
  ******************************************************************************
  * @file    TIM_ComplementarySignals/stm32f4xx_it.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    19-September-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and peripherals
  *          interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"
#include "stm32f4_discovery.h"
#include <stdbool.h>
#include "Delay.h"
/** @addtogroup STM32F4_Discovery_Peripheral_Examples
  * @{
  */

/** @addtogroup TIM_ComplementarySignals
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t capture = 0;
extern bool onlineFlag;
extern __IO uint32_t CCR1_Val;
extern __IO uint32_t CCR2_Val;
extern __IO uint32_t CCR3_Val;
extern __IO uint32_t CCR4_Val;
extern unsigned char recvMsg[100];
extern uint8_t recvCount;
extern unsigned char *data[4];
//bool recvFlag = false;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
extern bool WriteMsg(char* dst, char *content);
extern bool ReadMsg(unsigned char *(*readMsg[])[], uint8_t *readCount);
extern bool DeleteMsg(char* deleteNum);
/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {}
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {}
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {}
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {}
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{}

/**
  * @brief  This function handles PendSV_Handler exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
  TimingDelay_Decrement();
}

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @brief  This function handles USART1 interrupt request.
  * @param  None
  * @retval None
  */
void USART1_IRQHandler(void)
{
    if(USART_GetFlagStatus(USART1,USART_FLAG_RXNE)==SET)
    {
        USART_ClearFlag(USART1,USART_FLAG_RXNE);
        recvMsg[recvCount] = USART_ReceiveData(USART1);
        recvCount++;
    }
}

/**
  * @brief  This function handles TIM5 global interrupt request.
  * @param  None
  * @retval None
  */
void TIM5_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM5, TIM_IT_CC1) != RESET)//处理短信查询定时中断
  {
    TIM_ClearITPendingBit(TIM5, TIM_IT_CC1);

    /* LED4 toggling with frequency = 4.57 Hz */
    STM_EVAL_LEDToggle(LED4);
   //unsigned char *(*readMsg[10])[7];
   //uint8_t readCount = 0;
   //ReadMsg(readMsg, &readCount);
    
    capture = TIM_GetCapture1(TIM5);
    TIM_SetCompare1(TIM5, capture + CCR1_Val);
    
  }
  else if (TIM_GetITStatus(TIM5, TIM_IT_CC2) != RESET)//处理反馈定时中断
  {
    TIM_ClearITPendingBit(TIM5, TIM_IT_CC2);

    /* LED3 toggling with frequency = 9.15 Hz */
    STM_EVAL_LEDToggle(LED3);
    capture = TIM_GetCapture2(TIM5);
    TIM_SetCompare2(TIM5, capture + CCR2_Val);
  }
  else if (TIM_GetITStatus(TIM5, TIM_IT_CC3) != RESET)//处理事务定时中断
  {
    TIM_ClearITPendingBit(TIM5, TIM_IT_CC3);

    /* LED5 toggling with frequency = 18.31 Hz */
    STM_EVAL_LEDToggle(LED5);
    
    if (onlineFlag)
    {
     //WriteMsg("8618200259160","sj?ph=[data]&flow=[data]&temp=[data]&state=[state]#[num]");
    /*反馈处理*/
    }
    
    
    capture = TIM_GetCapture3(TIM5);
    TIM_SetCompare3(TIM5, capture + CCR3_Val);
  }
  else                                                //备用定时中断
  {
    TIM_ClearITPendingBit(TIM5, TIM_IT_CC4);

    /* LED6 toggling with frequency = 36.62 Hz */
    STM_EVAL_LEDToggle(LED6);
    capture = TIM_GetCapture4(TIM5);
    TIM_SetCompare4(TIM5, capture + CCR4_Val);
  }
}

/**
  * @}
  */ 

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
