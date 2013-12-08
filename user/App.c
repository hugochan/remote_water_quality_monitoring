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
bool registration(void);
void handleFeedback(void);
bool WriteMsg(char* dst, char *content);
bool ReadMsg(unsigned char* readMsg[], char msgIndex);
bool ReadMsg2(unsigned char *(*readMsg2[])[], uint8_t *readCount2);
bool DeleteMsg(char* deleteNum);
void getRT(void);


#define timerCounterFreq ((uint16_t)2000)

__IO uint32_t TIM2_Val = 6000;//jbӦ��ʱ��10s(���ȼ����)
__IO uint32_t TIM5_Val = 12000;//sjӦ��ʱ��20s
__IO uint32_t TIM3_Val = 18000;//sj����ʱ��30s�����ȼ���ͣ�


__IO uint16_t TimeDev = 100;//sys�ж�����Ϊ1000/TimeDev=10ms

unsigned char *data[3]={"8", "100", "0"};//ph flow state



bool gsmConfigFlag = false;
bool sjFlag = false;//���������־λ
bool waitingsjAckFlag = false;//�ȴ�����Ӧ���־λ
bool waitingjbAckFlag = false;//�ȴ�����Ӧ��־λ
bool sjAckTimeoutFlag = false;//�ȴ�����Ӧ��ʱ��־λ
bool waitingCmdAck = false;//�ȴ�AT�����Ӧ��
bool newMsgAdvertiseFlag = false;//����Ϣ��־λ
bool readSuccess = false;//��Ϣ��ȡ�ɹ���־λ
unsigned char recvCmdAck[1000];
uint16_t recvCmdAckCount = 0;
unsigned char recvNewMsgAdvertise[100];
uint16_t recvNewMsgAdvertiseCount = 0;
char newMsgIndex;
unsigned char *readMsg[3];

//extern bool recvFlag;
void main()
{
  /* Initialize Leds mounted on STM32F4-Discovery board */
  STM_EVAL_LEDInit(LED4);
  STM_EVAL_LEDInit(LED3);
  STM_EVAL_LEDInit(LED5);
  STM_EVAL_LEDInit(LED6);
  
  
  RCC_ClocksTypeDef RCC_Clocks;
  /* SysTick end of count event each 10ms */
  RCC_GetClocksFreq(&RCC_Clocks);
  SysTick_Config(RCC_Clocks.HCLK_Frequency / TimeDev);
  NVIC_SetPriority (SysTick_IRQn, 1); //����systickΪ������ȼ�
  SysTick->CTRL  &= ~SysTick_CTRL_ENABLE_Msk;//ʧ��systick
 
  //getRT();//��õ�ǰʱ��
  USART1_Config();
  NVIC_Config();
  TIM2_Config();
  TIM5_Config();
  TIM3_Config();
  
    
  while(1)
  {
    if (gsmConfigFlag)
    {
       if (newMsgAdvertiseFlag)
       {
         readSuccess = ReadMsg(readMsg, newMsgIndex);
         newMsgAdvertiseFlag = false;
         if (readSuccess&&(strcmp(readMsg[0],upperComputerNum)==0))
         {
           if (strcmp((char*)readMsg[2], "sj?ack#")==0)
           {  

              waitingsjAckFlag = false;
           }
           else if (strcmp((char*)readMsg[2], "zt?#")==0)
           {
           
           }
           else if (strcmp((char*)readMsg[2], "jb?ack#")==0)
           {
           
           }
           else if ((strcmp((char*)readMsg[2], "kz?state=0#")==0)||(strcmp(readMsg[2],(unsigned char*)"kz?state=1#")==0))
           {
           
           }
           else
           {
             
           }
           
         }
  
       }
       if (sjFlag)
       {
         WriteMsg(upperComputerNum,"sj?ph=[data]&flow=[data]&temp=[data]&state=[state]#[num]");
          /*��������*/
         waitingsjAckFlag = true;
         sjAckTimeoutFlag = false;
         TIM_SetCounter(TIM5, (uint32_t)0);
         TIM_Cmd(TIM5, ENABLE);//��sjӦ��ʱ�������� 
         sjFlag = false;
       }
       
       if (sjAckTimeoutFlag)
       {
         WriteMsg(upperComputerNum,"sj?ph=[data]&flow=[data]&temp=[data]&state=[state]#[num]");
          /*��������*/
         sjAckTimeoutFlag = false;
         TIM_SetCounter(TIM5, (uint32_t)0);
         TIM_Cmd(TIM5, ENABLE);//��sjӦ��ʱ��������
       }
       
       
    }
    else
    {
      gsmConfigFlag = gsmConfig();
    }
    
  }
}

/*********************************NVIC����*******************************/
void NVIC_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
    
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
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
  //TIM_Cmd(TIM5, ENABLE);
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
    {//��ɾ�����ж���
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
      
      }
    
    }
    return true;
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
    SendString((unsigned char*)dst);//destination call number
    SendChar(0x22);//"
    SendString((unsigned char*)"\r\n");
    Delay(10);
    SendString((unsigned char*)content);
    SendChar(0x1A);
    Delay(10);
    waitingCmdAck = false;//�ȴ�ATָ��Ӧ���־λ��λ
    if (recvCmdAck[24] != '>')
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
bool ReadMsg(unsigned char* readMsg[], char msgIndex)
{
  bool readFlag = false;
  waitingCmdAck = true;//�ȴ�ATָ��Ӧ���־λ��λ
  recvCmdAckCount = 0;
  SendString((unsigned char*)"AT+CMGR=");
  SendChar((unsigned char)msgIndex);
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
  if(recvCmdAck[recvCmdAckCount-4]!='O'||recvCmdAck[recvCmdAckCount-3]!='K')
  {
    return false;
  }
  else
  {
    return true;
  }
}
/******************************************************************************
//��õ�ǰʱ��
//���������
//���������
*******************************************************************************/
void getRT(void)//???
{
    //AT+CCLK
}
/******************************************************************************
//����λ����������ע����Ϣ
//���������
//���������
*******************************************************************************/
bool registration(void)
{   
    unsigned char *readMsg[3];
    uint8_t readCount = 0;
    WriteMsg("8618200259160", "zc?#1");
    Delay(500);//concrete delay time should be adjusted
    //ReadMsg(readMsg, &readCount);
    if(1)//�жϷ�����Ϣ
    {
      return true;
    }
    else
    {
      return false;
    }
}

/******************************************************************************
//����������
//���������
//���������
*******************************************************************************/
void handleFeedback(void)
{
  
}