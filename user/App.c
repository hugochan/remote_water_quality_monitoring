#include "App.h"

void NVIC_Config(void);
void TIM2_Config(void);
void TIM3_Config(void);
void TIM5_Config(void);
void USART1_Config(void);
void SendChar(unsigned char ch);
void SendString(unsigned char *p);
unsigned char GetChar(void);
bool gsmConfig(void);
bool handShake(void);
void atAdjust(void);
void registration(void);
void handleFeedback(void);
bool WriteMsg(char* dst, char *content);
bool ReadMsg(unsigned char* readMsg[], char* msgIndex);
bool ReadMsg2(unsigned char *(*readMsg2[])[], uint8_t *readCount2);
bool DeleteMsg(char* deleteNum);
bool DeleteAllMsgs(void);
bool synTime(void);
bool setRT(char *settime);
bool getRT(char *timeStr);
void IO_Init(void);

#define timerCounterFreq ((uint16_t)2000)
char machineNum[13]  = "8618328356422";


__IO uint32_t TIM2_Val = 36000;//jbӦ��ʱ��60s(���ȼ����,����uart)
__IO uint32_t TIM5_Val = 36000;//ע��׶�sjӦ��ʱ��||njӦ��ʱ��60s
__IO uint32_t TIM3_Val = 36000;//sj����ʱ��60s�����ȼ���ͣ�


__IO uint16_t TimeDev = 100;//sys�ж�����Ϊ1000/TimeDev=10ms
char sj[80] = "sj?ph=08&flow=050&state=1#15542893440_201312091200";
char zt[80] = "zt?ph=08&flow=050&state=1#15542893440_201312091200";
char kz[80] = "kz?ack#15542893440_201312091200";
char jb[80] = "jb?ph=13&state=0#15542893440_201312091200";
char nj[80] = "nj?ph=08&state=1#15542893440_201312091200";


bool jb_njControlFlag = false;//���ھ���&����������ƣ�true for �о����� false for ���������ֻ���ھ���״̬�½����������Ч
bool firstTime = false;//�״�ע��ʹ��
bool machineState = true;//����״̬��־λ��flase for �ػ���true for ������
bool gsmConfigFlag = false;//gsm���óɹ���־λ
bool onlineFlag = false;//���߱�־λ��������λ��ע��ɹ���
bool sjFlag = false;//���������־λ
bool jbFlag = false;//������־λ
bool njFlag = false;//���������־λ
bool waitingsjAckFlag = false;//�ȴ�����Ӧ���־λ
bool waitingjbAckFlag = false;//�ȴ�����Ӧ���־λ
bool waitingnjAckFlag = false;//�ȴ��������Ӧ���־λ
bool sjAckTimeoutFlag = false;//�ȴ�����Ӧ��ʱ��־λ
bool jbAckTimeoutFlag = false;//�ȴ�����Ӧ��ʱ��־λ
bool njAckTimeoutFlag = false;//�ȴ��������Ӧ��ʱ��־λ
bool waitingCmdAck = false;//�ȴ�AT�����Ӧ��
bool newMsgAdvertiseFlag = false;//�յ�����Ϣ��־λ

bool Success = false;//�ɹ���־λ
unsigned char recvCmdAck[1000];
uint16_t recvCmdAckCount = 0;
unsigned char recvNewMsgAdvertise[20];
uint16_t recvNewMsgAdvertiseCount = 0;
char newMsgIndex[3];
unsigned char *readMsg[3];
char generalCounter = 0;
char timeStr[14];
char flow = 50;
char ph = 7;


uint8_t jbNum = 0;
uint8_t njNum = 0;




//extern bool recvFlag;
void main()
{
  /* Initialize User_Button and Leds mounted on STM32F4-Discovery board */
  STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_EXTI); 
  STM_EVAL_LEDInit(LED4);//����ָʾ�ƣ��ƣ�
  STM_EVAL_LEDInit(LED3);//״ָ̬ʾ��
  STM_EVAL_LEDInit(LED5);//ph����ָʾ��
  STM_EVAL_LEDInit(LED6);//flow����ָʾ��
  STM_EVAL_LEDOff(LED4);
  STM_EVAL_LEDOn(LED3);
  STM_EVAL_LEDOff(LED5);
  STM_EVAL_LEDOff(LED6);
  IO_Init();
  
  RCC_ClocksTypeDef RCC_Clocks;
  /* SysTick end of count event each 10ms */
  RCC_GetClocksFreq(&RCC_Clocks);
  SysTick_Config(RCC_Clocks.HCLK_Frequency / TimeDev);
  NVIC_SetPriority (SysTick_IRQn, 1); //����systickΪ������ȼ�
  SysTick->CTRL  &= ~SysTick_CTRL_ENABLE_Msk;//ʧ��systick
 
  USART1_Config();
  NVIC_Config();
  TIM2_Config();
  TIM5_Config();
  TIM3_Config();
  //WriteMsg(upperComputerNum, "agsdg");
  
  //gsmConfigFlag = true;
  //onlineFlag = true;
 memcpy(strstr(sj, "#")+1, machineNum+2, 11);//���Ļ�����
 memcpy(strstr(zt, "#")+1, machineNum+2, 11);
 memcpy(strstr(jb, "#")+1, machineNum+2, 11);
 memcpy(strstr(nj, "#")+1, machineNum+2, 11);
 memcpy(strstr(kz, "#")+1, machineNum+2, 11);
 firstTime = false;
  while(1)
  {
    if (gsmConfigFlag)
    {
       if (firstTime == false) 
       {
          registration();
          firstTime = true;
       }
      
        if (onlineFlag)
       {
         if (newMsgAdvertiseFlag)
         {
           
           generalCounter = 0; 
           Success = ReadMsg(readMsg, newMsgIndex);
           while (!Success) 
           {
              Success = ReadMsg(readMsg, newMsgIndex);
              generalCounter++;
              if(generalCounter>2) break;
           }
           newMsgAdvertiseFlag = false;
           if (Success&&(strcmp((char*)readMsg[0], upperComputerNum)==0))
           {
             if (strcmp((char*)readMsg[2], "sj?ack#")==0)//sjӦ����
             {
               
             }
             else if (strcmp((char*)readMsg[2], "zt?#")==0)//zt������Ӧ
             {
               generalCounter = 0; 
               if (machineState) zt[24] = '1';
               else zt[24] = '0';
               if(ph<10)
               {
                 zt[6] = '0';
                 zt[7] = ph+48;
               
               }
               else if(ph<15)
               {
                  zt[6] = '1';
                  zt[7] = ph-10+48;
               }
               
               if(flow<10) 
               {
                  zt[15] = '0';
                  zt[16] = flow +48;
                }
                else if(flow<100)
                {
                  zt[15] = (flow/10)+48;
                  zt[16] = (flow%10)+48;
                }
               getRT(timeStr);
               memcpy(strstr(zt, "_")+1, timeStr, 14);
            
               Success = WriteMsg(upperComputerNum, zt);
               while (!Success) 
               {
                 Success = WriteMsg(upperComputerNum, zt);
                 generalCounter++;
                 if(generalCounter>2) break;
               }
             }
             else if (strcmp((char*)readMsg[2], "jb?ack#")==0)//jbӦ����
             {
                jbFlag = false;
                waitingjbAckFlag = false;//jbӦ��ɹ������ټ��jbӦ��
                jbAckTimeoutFlag = false;
             }
             
             else if (strcmp((char*)readMsg[2], "nj?ack#")==0)//njӦ����
             {
                njFlag = false;
                waitingnjAckFlag = false;//njӦ��ɹ������ټ��njӦ��
                njAckTimeoutFlag = false;
             }
             
             else if ((strcmp((char*)readMsg[2], "kz?state=0#")==0)||(strcmp((char *)readMsg[2], "kz?state=1#")==0))
             {//kz������Ӧ
               
               if (*(readMsg[2]+9) == '0')
               { 
                 machineState = false;
                 STM_EVAL_LEDOff(LED3);//ָʾ״̬Ϊͣ��
               }
               else if (*(readMsg[2]+9) == '1') 
               {
                 machineState = true;
                 STM_EVAL_LEDOn(LED3);//ָʾ״̬Ϊ����
               }
               getRT(timeStr);
               memcpy(strstr(kz, "_")+1, timeStr, 14);
               generalCounter = 0; 
               Success = WriteMsg(upperComputerNum, kz);
               while (!Success) 
               {
                 Success = WriteMsg(upperComputerNum, kz);
                 generalCounter++;
                 if(generalCounter>2) break;
               }
             }
             else
             {
               
             }
             
           }
            //delete the sj?ack message
            Success = DeleteMsg(newMsgIndex);
            generalCounter = 0;
            while (!Success) 
            {
              Success = DeleteMsg(newMsgIndex);
              generalCounter++;
              if(generalCounter>2) break;
            }
    
         }
         
         else
         {  
           /*�����������,����״̬����Ч*/
           if((GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == Bit_SET)&&(jb_njControlFlag))
           {
              while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == Bit_SET);
              STM_EVAL_LEDOff(LED4);//�������ָʾ
              STM_EVAL_LEDOn(LED3);//ָʾ״̬Ϊ����
              njFlag = true;
              jb_njControlFlag = false;
           }
           /*ph������*/
           if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == Bit_SET)
           {
             STM_EVAL_LEDOn(LED5);//ph����ָʾ
             while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == Bit_SET);
             STM_EVAL_LEDOff(LED5);
             ph = (ph+1)%15; 
           }
           /*ph������*/
           if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13) == Bit_SET)
           {
             STM_EVAL_LEDOn(LED5);//ph����ָʾ
             while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13) == Bit_SET);
             STM_EVAL_LEDOff(LED5);
             if (ph==0) ph = 14;
             else ph -= 1;
           }
           /*flow������*/
           if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14) == Bit_SET)
           {
             STM_EVAL_LEDOn(LED6);//flow����ָʾ
             while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14) == Bit_SET);
             STM_EVAL_LEDOff(LED6);
             flow = (flow+1)%100;
           }
           /*flow������*/
           if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_15) == Bit_SET)
           {
             STM_EVAL_LEDOn(LED6);//flow����ָʾ
             while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_15) == Bit_SET);
             STM_EVAL_LEDOff(LED6);
             if (flow == 0) flow = 99;
             else flow -= 1;
           }
           
           
           
           
           if (njFlag&&(!waitingnjAckFlag)&&(!newMsgAdvertiseFlag))//����nj��Ϣ
           {
              njNum = 1;
              machineState = true;
              atAdjust();
              getRT(timeStr);
              memcpy(strstr(nj, "_")+1, timeStr, 14);
             if(ph<10)
             {
                nj[6] = '0';
                nj[7] = ph+48;
                 
             }
             else if(ph<15)
             {
                nj[6] = '1';
                nj[7] = ph-10+48;
              }
              generalCounter = 0;
              Success = WriteMsg(upperComputerNum, nj);
              while (!Success) 
              {
                atAdjust();
                Success = WriteMsg(upperComputerNum, nj);
                generalCounter++;
                if(generalCounter>2) break;
              }
              waitingnjAckFlag = true;
              njFlag = false;
              TIM_Cmd(TIM5, ENABLE);//��njӦ��ʱ��
           }
           
           if (njAckTimeoutFlag&&(!newMsgAdvertiseFlag))//njӦ��ʱ�ط�
           {
              njNum++;
              if (njNum<4)
              {  
               atAdjust();
               getRT(timeStr);
               memcpy(strstr(nj, "_")+1, timeStr, 14);
               if(ph<10)
               {
                nj[6] = '0';
                nj[7] = ph+48;
                   
                }
               else if(ph<15)
               {
                  nj[6] = '1';
                  nj[7] = ph-10+48;
                }
               generalCounter = 0;
               Success = WriteMsg(upperComputerNum, nj);
               while (!Success) 
               {
                  atAdjust();
                  Success = WriteMsg(upperComputerNum, nj);
                  generalCounter++;
                  if(generalCounter>2) break;
                }
               njAckTimeoutFlag = false;
               TIM_Cmd(TIM5, ENABLE);//��njӦ��ʱ��
              }
              else njNum = 0;
           }
           
         }  
           
           
           
           if (jbFlag&&(!waitingjbAckFlag)&&(!newMsgAdvertiseFlag))//����jb��Ϣ
           {
             jbNum = 1;
             machineState = false;
             atAdjust();
             getRT(timeStr);
             memcpy(strstr(jb, "_")+1, timeStr, 14);
             if(ph<10)
             {
                jb[6] = '0';
                jb[7] = ph+48;
                 
             }
             else if(ph<15)
             {
                jb[6] = '1';
                jb[7] = ph-10+48;
              }
             
             generalCounter = 0;
             Success = WriteMsg(upperComputerNum, jb);
             while (!Success) 
             {
                atAdjust();
                Success = WriteMsg(upperComputerNum, jb);
                generalCounter++;
                if(generalCounter>2) break;
             }
             waitingjbAckFlag = true;
             jbFlag = false;
             TIM_Cmd(TIM2, ENABLE);//��jbӦ��ʱ��
           }
           
           if (jbAckTimeoutFlag&&(!newMsgAdvertiseFlag))//jbӦ��ʱ�ط�
           {
              jbNum++;
              if (jbNum < 4)
              {
               atAdjust();
               getRT(timeStr);
               memcpy(strstr(jb, "_")+1, timeStr, 14);
               
               if(ph<10)
               {
                  jb[6] = '0';
                  jb[7] = ph+48;
                   
               }
               else if(ph<15)
               {
                  jb[6] = '1';
                  jb[7] = ph-10+48;
                }
        
               generalCounter = 0;
               Success = WriteMsg(upperComputerNum, jb);
               while (!Success) 
               {
                  atAdjust();
                  Success = WriteMsg(upperComputerNum, jb);
                  generalCounter++;
                  if(generalCounter>2) break;
                }
               jbAckTimeoutFlag = false;
               TIM_Cmd(TIM2, ENABLE);//��jbӦ��ʱ��
              }
              else jbNum = 0;
           }
           
           
           if (sjFlag&&(!newMsgAdvertiseFlag))//���ڷ���sj��Ϣ��ע��ɹ�����֤Ӧ����Ϣ
           {
             if (machineState) sj[24] = '1';
             else sj[24] = '0';
             
             if(ph<10)
             {
                sj[6] = '0';
                sj[7] = ph+48;
                 
             }
             else if(ph<15)
             {
               sj[6] = '1';
               sj[7] = ph-10+48;
              }
              
             if(flow<10) 
             {
                sj[15] = '0';
                sj[16] = flow +48;
              }
              else if(flow<100)
              {
                sj[15] = (flow/10)+48;
                sj[16] = (flow%10)+48;
              }


             generalCounter = 0;
             atAdjust();//����ָ���
             getRT(timeStr);
             memcpy(strstr(sj, "_")+1, timeStr, 14);
             Success = WriteMsg(upperComputerNum, sj);
             while (!Success) 
             {
                atAdjust();
                Success = WriteMsg(upperComputerNum, sj);
                generalCounter++;
                if(generalCounter>2) break;//һ����������
              }
             sjFlag = false;
           }

       }
       
       else
       {
          if (newMsgAdvertiseFlag)
          {  
              newMsgAdvertiseFlag = false;
              generalCounter = 0;
              Success = ReadMsg(readMsg, newMsgIndex);
              while (!Success) 
              {
                 Success = ReadMsg(readMsg, newMsgIndex);
                 generalCounter++;
                 if(generalCounter>2) break;
              }
              if (Success&&(strcmp((char*)readMsg[0], upperComputerNum)==0))
              {
                if (strcmp((char*)readMsg[2], "sj?ack#")==0)//sjӦ����
                {
                  TIM_SetCounter(TIM3, (uint32_t)0);//�������sj����ʱ������ֹע��ɹ������������ظ���Ϣ
                  /*ע��ɹ�*/
                  onlineFlag = true;              
                  sjAckTimeoutFlag = false;
                 }
                else if (strcmp((char*)readMsg[2], "zt?#")==0)//zt������Ӧ
                {
                 TIM_SetCounter(TIM3, (uint32_t)0);//�������sj����ʱ������ֹע��ɹ������������ظ���Ϣ
                  /*ע��ɹ�*/
                 onlineFlag = true;              
                 sjAckTimeoutFlag = false;
                 

                 if (machineState) zt[24] = '1';
                 else zt[24] = '0';
                 if(ph<10)
                 {
                   zt[6] = '0';
                   zt[7] = ph+48;
                 
                 }
                 else if(ph<15)
                 {
                    zt[6] = '1';
                    zt[7] = ph-10+48;
                 }
                 
                 if(flow<10) 
                 {
                    zt[15] = '0';
                    zt[16] = flow +48;
                  }
                  else if(flow<100)
                  {
                    zt[15] = (flow/10)+48;
                    zt[16] = (flow%10)+48;
                  }
                 
                 getRT(timeStr);
                 memcpy(strstr(sj, "_")+1, timeStr, 14);  
                 generalCounter = 0; 
                 Success = WriteMsg(upperComputerNum, zt);
                 while (!Success) 
                 {
                   Success = WriteMsg(upperComputerNum, zt);
                   generalCounter++;
                   if(generalCounter>2) break;
                 }
                }
              }
              //delete the sj?ack message
              Success = DeleteMsg(newMsgIndex);
              generalCounter = 0;
              while (!Success) 
              {
                 Success = DeleteMsg(newMsgIndex);
                 generalCounter++;
                 if(generalCounter>2) break;
              }
            
            
          }
          
          if(sjAckTimeoutFlag&&(!newMsgAdvertiseFlag))
          {
            registration();
            sjAckTimeoutFlag = false;
          }
       }
       
    }
    else
    {
      gsmConfigFlag = gsmConfig();
      if (gsmConfigFlag) 
      {
        //deleate all messages,ɾ�����ܵ���Ϣ����
        DeleteAllMsgs();
      }  
    }
    
  }
}

/*********************************NVIC����*******************************/
void NVIC_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
    
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannel  = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        
    NVIC_Init(&NVIC_InitStructure);
}


/*********************************��ʱ����ʼ��*******************************/
//32bit timer
void TIM2_Config(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* TIM2 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  /* Enable the TIM2 gloabal Interrupt */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  /* Compute the prescaler value */
  uint16_t PrescalerValue = (uint16_t) ((SystemCoreClock / 2) / timerCounterFreq) - 1;//2khz timer counter
  
  
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = TIM2_Val;
  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
  
  TIM_ClearFlag(TIM2, TIM_FLAG_Update);   //�������������ʱ������ĸ��±�־
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);   //ʹ���жϣ��ж��¼�Ϊ��ʱ�������¼�

  /* TIM2 enable counter */
  //TIM_Cmd(TIM2, ENABLE);
}

//16bit timer
void TIM3_Config(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* TIM3 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  /* Enable the TIM3 gloabal Interrupt */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  /* Compute the prescaler value */
  uint16_t PrescalerValue = (uint16_t) ((SystemCoreClock / 2) / timerCounterFreq) - 1;//2khz timer counter
  
  
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = TIM3_Val;
  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
 


  TIM_ClearFlag(TIM3, TIM_FLAG_Update);   //�������������ʱ������ĸ��±�־
  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);   //ʹ���жϣ��ж��¼�Ϊ��ʱ�������¼�

  /* TIM3 enable counter */
  TIM_Cmd(TIM3, ENABLE);
}

//32bit timer
void TIM5_Config(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* TIM5 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

  /* Enable the TIM5 gloabal Interrupt */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
  NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  /* Compute the prescaler value */
  uint16_t PrescalerValue = (uint16_t) ((SystemCoreClock / 2) / timerCounterFreq) - 1;//2khz timer counter
  
  
    /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = TIM5_Val;
  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
 


  TIM_ClearFlag(TIM5, TIM_FLAG_Update);   //�������������ʱ������ĸ��±�־
  TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);   //ʹ���жϣ��ж��¼�Ϊ��ʱ�������¼�

  /* TIM5 enable counter */
  TIM_Cmd(TIM5, ENABLE);
}


/*********************************���ڳ�ʼ��*******************************/
void USART1_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    USART_ClockInitTypeDef USART_ClockInitStruct;

    /* config USART1 clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    /* Enable GPIO clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
   
    GPIO_PinAFConfig(GPIOB,GPIO_PinSource6,GPIO_AF_USART1);  //PB6--USART1_TX
    GPIO_PinAFConfig(GPIOB,GPIO_PinSource7,GPIO_AF_USART1);  //PB7--USART1_RX   
    /* Configure USART Tx as alternate function  */
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_6;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF;     //����Ϊ���ã�����ΪAF
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB,&GPIO_InitStructure);
    /* Configure USART Rx as alternate function  */
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_Init(GPIOB,&GPIO_InitStructure);
    /* USART1 mode config */
    USART_InitStructure.USART_BaudRate = 9600;  //9600HZ������
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);
    USART_ClockStructInit(&USART_ClockInitStruct);//֮ǰû������ȱʡֵ���ǲ��е�
    USART_ClockInit(USART6, &USART_ClockInitStruct);
    /*ʹ���ж�*/
    USART_ITConfig(USART1,USART_IT_RXNE,ENABLE); 
    USART_Cmd(USART1, ENABLE);
}
/************************************����һ���ֽ�******************************/

void SendChar(unsigned char ch)
{  
   USART_SendData(USART1,ch);
   while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET); //�ȴ�������һ���ֽ�
}
/******************************************************************************
//���ڷ���һ���ַ���
//����������������ַ����׵�ַ p
*******************************************************************************/
void SendString(unsigned char *p)
{
    while(*p != '\0')	SendChar(*(p++));
}
/******************************************************************************
//���ڽ���һ���ַ�
//�����������
//��������������ַ�
*******************************************************************************/
unsigned char GetChar(void)
{
    unsigned char ch;
    if(USART_GetFlagStatus(USART1,USART_FLAG_RXNE)==SET)
    {
      USART_ClearFlag(USART1,USART_FLAG_RXNE);
      ch = USART_ReceiveData(USART1);
    }
    return ch;
}
/******************************************************************************
//GSMģ���ʼ��
//���������
//���������״̬��true or false��
*******************************************************************************/
bool gsmConfig(void)//config text function of gsm
{
    char i = 0;
    while(!handShake())
    {
      i++;
      if (i > 10) return false;
    }
    waitingCmdAck = true;//�ȴ�ATָ��Ӧ���־λ��λ
    recvCmdAckCount = 0;
    SendString("AT+CMGF=1\r\n");
    Delay(10);
    waitingCmdAck = false;//�ȴ�ATָ��Ӧ���־λ��λ
    if(recvCmdAck[12]!='O'||recvCmdAck[13]!='K')
    {
      return false;
    }
    else
    {
      waitingCmdAck = true;//�ȴ�ATָ��Ӧ���־λ��λ
      recvCmdAckCount = 0;
      SendString("AT+CNMI=1,1,2\r\n");//������Ϣ��ʾģʽ
      Delay(10);
      waitingCmdAck = false;//�ȴ�ATָ��Ӧ���־λ��λ
      if(recvCmdAck[recvCmdAckCount-4]!='O'&&recvCmdAck[recvCmdAckCount-3]!='K')
      {
        return false;
      }
      else
      {
        if(!synTime()) return false;
        else return true;
      }
    
    }

}
/******************************************************************************
//���ֲ���
//���������
//���������״̬��true or false��
*******************************************************************************/
bool handShake(void)
{       
    waitingCmdAck = true;//�ȴ�ATָ��Ӧ���־λ��λ
    recvCmdAckCount = 0;
    SendString("AT\r\n");
    Delay(10);
    waitingCmdAck = false;//�ȴ�ATָ��Ӧ���־λ��λ
    if(recvCmdAck[5]=='O'&&recvCmdAck[6]=='K')
    {
      return true;
    }
    return false;

}

/******************************************************************************
//����ָ���,��ֹatָ��д��һ������
//���������
//���������״̬���ޣ�
*******************************************************************************/
void atAdjust(void)
{       
    waitingCmdAck = true;//�ȴ�ATָ��Ӧ���־λ��λ
    recvCmdAckCount = 0;
    SendString("\r\n");
    Delay(10);
    waitingCmdAck = false;//�ȴ�ATָ��Ӧ���־λ��λ
}


/******************************************************************************
//����һ��Ӣ�Ķ���
//���������content��������
            dst����Ŀ�����
//���������״̬��true or false��
*******************************************************************************/
bool WriteMsg(char* dst, char* content)
{   
    waitingCmdAck = true;//�ȴ�ATָ��Ӧ���־λ��λ
    recvCmdAckCount = 0;
    SendString((unsigned char*)"AT+CMGS=");
    SendChar(0x22);//"
    SendString((unsigned char*)(dst+2));//destination call number
    SendChar(0x22);//"
    SendString((unsigned char*)"\r\n");
    Delay(10);
    SendString((unsigned char*)content);
    SendChar(0x1A);
    Delay(500);
    waitingCmdAck = false;//�ȴ�ATָ��Ӧ���־λ��λ
    if (recvCmdAck[recvCmdAckCount-3] != 'K')
    {
      return false;
    }
    return true;
}

/******************************************************************************
//��������Ӣ�Ķ���
//���������content��������
            dst����Ŀ�����
//���������״̬��true or false��
*******************************************************************************/
bool ReadMsg(unsigned char* readMsg[], char* msgIndex)
{
  bool readFlag = false;
  waitingCmdAck = true;//�ȴ�ATָ��Ӧ���־λ��λ
  recvCmdAckCount = 0;
  SendString((unsigned char*)"AT+CMGR=");
  SendString((unsigned char*)msgIndex);
  SendString((unsigned char*)"\r\n");
  Delay(200);
  waitingCmdAck = false;//�ȴ�ATָ��Ӧ���־λ��λ
  if (recvCmdAck[recvCmdAckCount-4]!='O'||recvCmdAck[recvCmdAckCount-3]!='K')
  {
    readFlag = false;
  }
  else
  {
      char *ptr = strstr((char*)recvCmdAck, "+CMGR:");
    
    
      ptr += 21;
      /*decompose the readMsg: readMsg call number*/
      uint16_t ptrCount = 0;
      readMsg[0] = (unsigned char*)ptr;//readMsg call number start addr
      while(*(ptr++) != '"') ptrCount++;
      *(readMsg[0]+ptrCount) = (unsigned char)0x00;
      
      //adjust ptr
      while(*(ptr++) != '"');
      
      
      /*decompose the readMsg: readMsg timestamp*/
      ptrCount = 0;
      readMsg[1] = (unsigned char*)ptr;//readMsg timestamp start addr
      while(*(ptr++) != '+') ptrCount++;
      *(readMsg[1]+ptrCount) = (unsigned char)0x00;
      
      
      //adjust ptr
      while(*(ptr++) != '\n');
      
      /*decompose the readMsg: content*/
      ptrCount = 0;
      readMsg[2] = (unsigned char*)ptr;//readMsg content start addr
      while((*ptr != '#')&&(*ptr != '\r'))
      {
        ptr++;
        ptrCount++;
      }
      *(readMsg[2]+ptrCount+1) = (unsigned char)0x00;
    
      readFlag = true;
    
  }
  return readFlag;
}




/******************************************************************************
//��������Ӣ�Ķ���
//���������content��������
            dst����Ŀ�����
//���������״̬��true or false��
*******************************************************************************/
bool ReadMsg2(unsigned char *(*readMsg2[])[], uint8_t *readCount2)
{

  bool recvFlag = false;
  waitingCmdAck = true;//�ȴ�ATָ��Ӧ���־λ��λ
  recvCmdAckCount = 0;
  SendString((unsigned char*)"AT+CMGL=");
  SendChar(0x22);
  SendString((unsigned char*)"ALL");
  SendChar(0x22);
  SendString((unsigned char*)"\r\n");
  Delay(200);
  waitingCmdAck = false;//�ȴ�ATָ��Ӧ���־λ��λ
  if (recvCmdAck[16]!='+'||recvCmdAck[17]!='C')
  {
    recvFlag = false;
  }
  else
  {
    *readCount2 = 0;
    char *ptr = (char*)malloc(1024*sizeof(char));
    ptr = strstr((char*)recvCmdAck, "+CMGL:");
    while(ptr != NULL)
    {//????������
      
      
      ptr += 7;
      /*decompose the readMsg: index*/
      uint16_t ptrCount = 0;
      (*readMsg2[*readCount2])[0] = (unsigned char*)ptr;//readMsg index start addr
      while(*(ptr++) != ',') ptrCount++;
      *((*readMsg2[*readCount2])[1]+ptrCount) = (unsigned char)0x00;
      
      /*decompose the readMsg: status*/
      ptr += 1;//???������ѭ���ܷ�
      ptrCount = 0;
      (*readMsg2[*readCount2])[1] = (unsigned char*)ptr;//readMsg status start addr
      while(*(ptr++) != '"') ptrCount++;
      *((*readMsg2[*readCount2])[1]+ptrCount) = (unsigned char)0x00;
      
      /*decompose the readMsg: call number*/
      ptr += 2;
      ptrCount = 0;
      (*readMsg2[*readCount2])[2] = (unsigned char*)ptr;//readMsg call number start addr
      while(*(ptr++) != '"') ptrCount++;
      *((*readMsg2[*readCount2])[2]+ptrCount) = (unsigned char)0x00;
      
      //adjust ptr
      while(*(ptr++) != '"');
      
      /*decompose the readMsg: timestamp number*/
      ptrCount = 0;
      (*readMsg2[*readCount2])[3] = (unsigned char*)ptr;//readMsg timestamp start addr
      while(*(ptr++) != '"') ptrCount++;
      *((*readMsg2[*readCount2])[3]+ptrCount) = (unsigned char)0x00;
      
      
      /*decompose the readMsg: num*/
      ptr += 1 ;
      ptrCount = 0;
      (*readMsg2[*readCount2])[4] = (unsigned char*)ptr;//readMsg num start addr
      while(*(ptr++) != ',') ptrCount++;
      *((*readMsg2[*readCount2])[4]+ptrCount) = (unsigned char)0x00;
      
      /*decompose the readMsg: length*/
      ptrCount = 0;
      (*readMsg2[*readCount2])[5] = (unsigned char*)ptr;//readMsg length start addr
      while(*(ptr++) != '\r') ptrCount++;
      *((*readMsg2[*readCount2])[5]+ptrCount) = (unsigned char)0x00;
      
      
      
      /*decompose the readMsg: content*/
      ptr += 1;
      ptrCount = 0;
      (*readMsg2[*readCount2])[6] = (unsigned char*)ptr;//readMsg content start addr
      while(*(ptr++) != '\r') ptrCount++;
      *((*readMsg2[*readCount2])[6]+ptrCount) = (unsigned char)0x00;
            
      
      ptr = strstr(ptr, "+CMGL:");//match the next substring
      *readCount2 += 1;
    }
    recvFlag = true;
  }
  return recvFlag;
}

/******************************************************************************
//ɾ��һ��Ӣ�Ķ���
//���������content��������
            dst����Ŀ�����
//���������״̬��true or false��
*******************************************************************************/
bool DeleteMsg(char* deleteNum)
{
  waitingCmdAck = true;//�ȴ�ATָ��Ӧ���־λ��λ
  recvCmdAckCount = 0;
  SendString((unsigned char*)"AT+CMGD=");
  SendString((unsigned char*)deleteNum);
  SendString((unsigned char*)"\r\n");
  Delay(20);
  waitingCmdAck = false;//�ȴ�ATָ��Ӧ���־λ��λ
  if(recvCmdAck[12] == 'E')
  {
    return false;
  }
  else
  {
    return true;
  }
}


/******************************************************************************
//ɾ������Ӣ�Ķ���
//�����������
//���������״̬��true or false��
*******************************************************************************/
bool DeleteAllMsgs(void)
{
  waitingCmdAck = true;//�ȴ�ATָ��Ӧ���־λ��λ
  recvCmdAckCount = 0;
  SendString((unsigned char*)"AT+CMGD=1,4");
  SendString((unsigned char*)"\r\n");
  Delay(20);
  waitingCmdAck = false;//�ȴ�ATָ��Ӧ���־λ��λ
  if(recvCmdAck[13] == 'E')
  {
    return false;
  }
  else
  {
    return true;
  }
}
/******************************************************************************
//ͬ����ǰʱ��
//���������
//���������bool
*******************************************************************************/
bool synTime(void)
{
    WriteMsg("8610086","");
    while(!newMsgAdvertiseFlag);
    newMsgAdvertiseFlag = false;//�����־λ
    unsigned  char *syntime[3];
    char timeStr[18];
    ReadMsg(syntime, newMsgIndex);


    timeStr[0] = *syntime[1];
    timeStr[1] = *(syntime[1]+1);
    timeStr[2] = '/';
    timeStr[3] = *(syntime[1]+3);
    timeStr[4] = *(syntime[1]+4);
    timeStr[5] = '/';
    timeStr[6] = *(syntime[1]+6);
    timeStr[7] = *(syntime[1]+7);
    timeStr[8] = ',';
    timeStr[9] = *(syntime[1]+9);
    timeStr[10] = *(syntime[1]+10);
    timeStr[11] = ':';
    timeStr[12] = *(syntime[1]+12);
    timeStr[13] = *(syntime[1]+13);
    timeStr[14] = ':';
    timeStr[15] = *(syntime[1]+15);
    timeStr[16] = *(syntime[1]+16);
    timeStr[17] = (char)0x00;
    if (setRT(timeStr))//����ʱ��
    {
      return true;
    }
    else return false;
    
}
/******************************************************************************
//���õ�ǰʱ��
//���������
//���������bool
*******************************************************************************/
bool setRT(char *settime)//???
{
  
  waitingCmdAck = true;
  recvCmdAckCount = 0;
  SendString((unsigned char*)"AT+CCLK=");
  SendChar(0x22);
  SendString((unsigned char*)settime);
  SendChar(0x22);
  SendString((unsigned char*)"\r\n");
  Delay(20);
  waitingCmdAck = false;//�ȴ�ATָ��Ӧ���־λ��λ
  if((recvCmdAck[recvCmdAckCount-4]!='O')&&(recvCmdAck[recvCmdAckCount-3]!='K')) return false;
  else return true;
}

/******************************************************************************
//��õ�ǰʱ��
//���������
//���������bool
*******************************************************************************/
bool getRT(char *timeStr)
{
  waitingCmdAck = true;
  recvCmdAckCount = 0;
  SendString((unsigned char*)"AT+CCLK?");
  SendString((unsigned char*)"\r\n");
  Delay(50);
  waitingCmdAck = false;//�ȴ�ATָ��Ӧ���־λ��λ
  if(recvCmdAck[14]!='L') return false;
  else
  {
    char *ptr = strstr((char*)recvCmdAck, "+CCLK:");
    ptr += 6;
    timeStr[0] = '2';
    timeStr[1] = '0';
    timeStr[2] = *ptr;
    timeStr[3] = *(ptr+1);
    timeStr[4] = *(ptr+3);
    timeStr[5] = *(ptr+4);
    timeStr[6] = *(ptr+6);
    timeStr[7] = *(ptr+7);
    timeStr[8] = *(ptr+9);
    timeStr[9] = *(ptr+10);
    timeStr[10] = *(ptr+12);
    timeStr[11] = *(ptr+13);
    timeStr[12] = *(ptr+15);
    timeStr[13] = *(ptr+16);
    return true;
  }
}


/******************************************************************************
//����λ����������ע����Ϣ
//���������
//���������
*******************************************************************************/
void registration(void)
{   
    atAdjust();
    getRT(timeStr);
    memcpy(strstr(sj, "_")+1, timeStr, 14);
    if (machineState) sj[24] = '1';
    else sj[24] = '0';
    if(ph<10) sj[7] = ph+48;
    else if(ph<15)
    {
      sj[6] = 1;
      sj[7] = ph-10+48;
    }
    
    if(flow<10) 
    {
      sj[15] = '0';
      sj[16] = flow +48;
    }
    else if(flow<100)
    {
      sj[15] = (flow/10)+48;
      sj[16] = (flow%10)+48;
    }
    
    
    
    generalCounter = 0;
    if (newMsgAdvertiseFlag) Success = true;
    else Success = WriteMsg(upperComputerNum, sj);
    
    while (!Success) 
    {
        atAdjust();
        Success = WriteMsg(upperComputerNum, sj);
        generalCounter++;
        if(generalCounter>2) break;
    }
}

/**
  * @brief  Initialize IO for nj trigger
  * @param  None
  * @retval None
  */
void IO_Init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  /* GPIOD Periph clock enable */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
   
  /* Configure PB11 pin as input */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11||GPIO_Pin_12||GPIO_Pin_13||GPIO_Pin_14||GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}