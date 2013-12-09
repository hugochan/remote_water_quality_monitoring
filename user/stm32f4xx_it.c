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
#include "App.h"
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
uint8_t factor2 = 0;
uint8_t factor5 = 0;
uint8_t factor3 = 0;
extern __IO uint8_t UserButtonPressed;
bool newMsgComingFlag = false;//����Ϣ������־λ
extern bool onlineFlag;
extern bool sjFlag;
extern bool jbFlag;
extern bool waitingCmdAck;
extern bool newMsgAdvertiseFlag;
extern bool gsmConfigFlag;
extern bool waitingsjAckFlag;
extern bool waitingjbAckFlag;
extern bool waitingnjAckFlag;
extern bool sjAckTimeoutFlag;
extern bool jbAckTimeoutFlag;
extern bool njAckTimeoutFlag;
extern __IO uint32_t TIM5_Val;
extern __IO uint32_t TIM2_Val;
extern __IO uint32_t TIM3_Val;

extern unsigned char recvCmdAck[100];
extern uint16_t recvCmdAckCount;
extern unsigned char recvNewMsgAdvertise[20];
extern uint16_t recvNewMsgAdvertiseCount;
extern char newMsgIndex[3];
extern unsigned char *readMsg[7];
extern uint8_t readCount;
extern unsigned char *data[3];
//bool recvFlag = false;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
extern bool WriteMsg(char* dst, char *content);
extern bool ReadMsg(unsigned char* readMsg[], uint8_t *readCount, char msgIndex);
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
  * @brief  This function handles EXTI0_IRQ Handler.
  * @param  None
  * @retval None
  */
void EXTI0_IRQHandler(void)
{
 
     UserButtonPressed = 0x01;
     STM_EVAL_LEDOn(LED4);//ָʾ����
     STM_EVAL_LEDOff(LED3);//ָʾ״̬Ϊͣ��
     jbFlag = true;
     /* Clear the EXTI line pending bit */
     EXTI_ClearITPendingBit(USER_BUTTON_EXTI_LINE);

}


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
        if (waitingCmdAck == true)//��ȡATָ��Ӧ������ȼ����ڶ�ȡ����Ϣ�����ȼ�
        {
          recvCmdAck[recvCmdAckCount] = USART_ReceiveData(USART1);
          recvCmdAckCount++;
        }
        else
        {
          //��������Ϣ
          
          char tempdata = USART_ReceiveData(USART1);
          if ((recvNewMsgAdvertiseCount == 0)&&(tempdata == 'I'))//��+CNMI:��'I'��ʼ����ATָ���Ϊ�о�
          {
            newMsgComingFlag = true;
          }
          if(newMsgComingFlag)
          {
            recvNewMsgAdvertise[recvNewMsgAdvertiseCount] = tempdata;
            recvNewMsgAdvertiseCount++;
            if(recvNewMsgAdvertise[recvNewMsgAdvertiseCount-1] == '\r')
            {
              uint8_t i; 
              for(i=8;i<(recvNewMsgAdvertiseCount-1);i++)
              {
                newMsgIndex[i-8] = recvNewMsgAdvertise[i];
              }
              newMsgIndex[i] = (char)0x00;
              recvNewMsgAdvertiseCount = 0;
              newMsgAdvertiseFlag = true;//�յ�����Ϣ��־λ��λ��δ�����ж�Ƕ�ף��������������ڴ˴�����Ϣ��  
              newMsgComingFlag = false;//����Ϣ������־λ
            }
          }
        }
    }
}


/**
  * @brief  This function handles TIM2 global interrupt request.
  * @param  None
  * @retval None
  */
void TIM2_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)//����jbӦ��ʱ�ж�
  {
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    factor2++;
    if (factor2 == Factor)
    {
      factor2 = 0;
      /* LED4 toggling with frequency = 4.57 Hz */
      STM_EVAL_LEDToggle(LED4);
      if(waitingjbAckFlag)//����Ӧ��ʱ
      {
        jbAckTimeoutFlag = true;
      }
      TIM_Cmd(TIM2, DISABLE);//�ر�jbӦ��ʱ��
      TIM_SetCounter(TIM2, (uint32_t)0);
    
    }
   }
}


/**
  * @brief  This function handles TIM5 global interrupt request.
  * @param  None
  * @retval None
  */
void TIM5_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)//�����ʼע��׶�sjӦ��ʱ�ж�??||nj��ϢӦ��ʱ��
  {
    TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
    factor5++;
    if (factor5 == Factor)
    {
      factor5 = 0;
      /* LED4 toggling with frequency = 4.57 Hz */
      STM_EVAL_LEDToggle(LED3);
      if(gsmConfigFlag&&(!onlineFlag))
      {
        sjAckTimeoutFlag = true;
        //TIM_Cmd(TIM5, DISABLE);//�رճ�ʼע��׶�sjӦ��ʱ�������������رձ���������
      }
      
      if(gsmConfigFlag&&onlineFlag&&waitingnjAckFlag)    
      {
        njAckTimeoutFlag = true;
        TIM_Cmd(TIM5, DISABLE);//�ر�njӦ��ʱ��
      }
    }
    TIM_SetCounter(TIM5, (uint32_t)0);
    
  }
}

/**
  * @brief  This function handles TIM3 global interrupt request.
  * @param  None
  * @retval None
  */
void TIM3_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)//����sj����ʱ�ж�
  {
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    factor3++;
    if (factor3 == Factor)
    {
      factor3 = 0;
      /* LED4 toggling with frequency = 4.57 Hz */
      STM_EVAL_LEDToggle(LED5);
      if(gsmConfigFlag&&onlineFlag)//���ҽ���gsm���óɹ�&&sjack���������������Ż���λsjFlag��������sj��Ϣ
      {
        sjFlag = true;
      }
    }
    
    TIM_SetCounter(TIM3, (uint32_t)0);
  }
}



/**
  * @}
  */ 

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
