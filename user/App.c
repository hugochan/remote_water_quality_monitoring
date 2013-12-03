#include "App.h"

void NVIC_Config(void);
void TIM_Config(void);
void USART1_Config(void);
void SendChar(unsigned char ch);
void SendString(unsigned char *p);
unsigned char GetChar(void);
bool gsmConfig(void);
bool handShake(void);
bool registration(void);
void handleFeedback(void);
bool WriteMsg(char* dst, char *content);
bool ReadMsg(unsigned char *(*readMsg[])[], uint8_t *readCount);
bool DeleteMsg(char* deleteNum);
void getRT(void);

#define machineNum ((unsigned char)1)
#define timerCounterFreq ((uint16_t)100)
__IO uint32_t CCR1_Val = 2500;//查询短信定时器5s
__IO uint32_t CCR2_Val = 15000;//反馈定时器30s
__IO uint32_t CCR3_Val = 150000;//事务定时器5min
__IO uint32_t CCR4_Val = 450000;//事务定时器2 15min
__IO uint16_t TimeDev = 100;//sys中断周期为1000/TimeDev=10ms 
bool gsmConfigFlag = false;
bool onlineFlag = false;//在线状态标志位
bool waitingFeedbackFlag = false;//等待反馈标志位
unsigned char recvMsg[1000];
uint16_t recvCount = 0;
unsigned char *data[4];

//extern bool recvFlag;
void main()
{
  /* Initialize Leds mounted on STM32F4-Discovery board */
  STM_EVAL_LEDInit(LED4);
  STM_EVAL_LEDInit(LED3);
  STM_EVAL_LEDInit(LED5);
  STM_EVAL_LEDInit(LED6);
  
  unsigned char *(*readMsg[10])[7];
  uint8_t readCount = 0;
  RCC_ClocksTypeDef RCC_Clocks;
  /* SysTick end of count event each 10ms */
  RCC_GetClocksFreq(&RCC_Clocks);
  SysTick_Config(RCC_Clocks.HCLK_Frequency / TimeDev);
  NVIC_SetPriority (SysTick_IRQn, 1); //设置systick为最高优先级
  SysTick->CTRL  &= ~SysTick_CTRL_ENABLE_Msk;//失能systick
 
  //getRT();//获得当前时间
  USART1_Config();
  NVIC_Config();
  TIM_Config();
  gsmConfigFlag = gsmConfig();
  //gsmConfigFlag = WriteMsg("18200259160", "hi");
  ReadMsg(readMsg, &readCount);
  //DeleteMsg("1");
  //registration(machineNum);
  
  while(1)
  {
    if (onlineFlag)
    {
       TIM_ITConfig(TIM5, TIM_IT_CC3, ENABLE);//打开事务处理定时器通道
    }
    else
    {
      TIM_ITConfig(TIM5, TIM_IT_CC3, DISABLE);//关闭事务处理定时器通道
      //onlineFlag = registration();
    }
    
  }
}

/*********************************NVIC配置*******************************/
void NVIC_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x03;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;
    NVIC_InitStructure.NVIC_IRQChannel  = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        
    NVIC_Init(&NVIC_InitStructure);
}


/*********************************定时器初始化*******************************/
void TIM_Config(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* TIM5 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

  /* Enable the TIM5 gloabal Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef  TIM_OCInitStructure;
  /* Compute the prescaler value */
  uint32_t PrescalerValue = (uint32_t) ((SystemCoreClock / 2) / timerCounterFreq) - 1;//100hz timer counter
  
  
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 0xFFFFFFFF;
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
  
  
   /* Prescaler configuration */
  TIM_PrescalerConfig(TIM5, PrescalerValue, TIM_PSCReloadMode_Immediate);

  /* Output Compare Timing Mode configuration: Channel1 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = CCR1_Val;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

  TIM_OC1Init(TIM5, &TIM_OCInitStructure);
  
  TIM_OC1PreloadConfig(TIM5, TIM_OCPreload_Disable);
  
  /* Output Compare Timing Mode configuration: Channel2 */
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = CCR2_Val;

  TIM_OC2Init(TIM5, &TIM_OCInitStructure);

  TIM_OC2PreloadConfig(TIM5, TIM_OCPreload_Disable);

  /* Output Compare Timing Mode configuration: Channel3 */
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = CCR3_Val;

  TIM_OC3Init(TIM5, &TIM_OCInitStructure);

  TIM_OC3PreloadConfig(TIM5, TIM_OCPreload_Disable);

  /* Output Compare Timing Mode configuration: Channel4 */
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = CCR4_Val;

  TIM_OC4Init(TIM5, &TIM_OCInitStructure);

  TIM_OC4PreloadConfig(TIM5, TIM_OCPreload_Disable);
   
  /* TIM Interrupts enable */
  //TIM_ITConfig(TIM5, TIM_IT_CC1 | TIM_IT_CC2 | TIM_IT_CC3 | TIM_IT_CC4, ENABLE);
  TIM_ITConfig(TIM5, TIM_IT_CC1, ENABLE);
  /* TIM5 enable counter */
  TIM_Cmd(TIM5, ENABLE);
}


/*********************************串口初始化*******************************/
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
    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF;     //设置为复用，必须为AF
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB,&GPIO_InitStructure);
    /* Configure USART Rx as alternate function  */
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_Init(GPIOB,&GPIO_InitStructure);
    /* USART1 mode config */
    USART_InitStructure.USART_BaudRate = 9600;  //9600HZ波特率
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);
    USART_ClockStructInit(&USART_ClockInitStruct);//之前没有填入缺省值，是不行的
    USART_ClockInit(USART6, &USART_ClockInitStruct);
    /*使能中断*/
    USART_ITConfig(USART1,USART_IT_RXNE,ENABLE); 
    USART_Cmd(USART1, ENABLE);
}
/************************************发送一个字节******************************/

void SendChar(unsigned char ch)
{  
   USART_SendData(USART1,ch);
   while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET); //等待发送完一个字节
}
/******************************************************************************
//串口发送一个字符串
//输入参数：待发送字符串首地址 p
*******************************************************************************/
void SendString(unsigned char *p)
{
    while(*p != '\0')	SendChar(*(p++));
}
/******************************************************************************
//串口接收一个字符
//输入参数：无
//输出参数：接收字符
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
//GSM模块初始化
//输入参数：
//输出参数：状态（true or false）
*******************************************************************************/
bool gsmConfig(void)//config text function of gsm
{   
    char i = 0;
    while(!handShake())
    {
      i++;
      if (i > 10) return false;
    }
    recvCount = 0;
    SendString("AT+CMGF=1\r\n");
    Delay(10);
    if(recvMsg[12]!='O'||recvMsg[13]!='K')
    {
      return false;
    }
    else
    {
    
    
    }
    return true;
}
/******************************************************************************
//握手测试
//输入参数：
//输出参数：状态（true or false）
*******************************************************************************/
bool handShake(void)
{
    recvCount = 0;
    SendString("AT\r\n");
    Delay(10);
    if(recvMsg[5]=='O'&&recvMsg[6]=='K')
    {
      return true;
    }
    return false;

}
/******************************************************************************
//发送一条英文短信
//输入参数：content——内容
            dst——目标对象
//输出参数：状态（true or false）
*******************************************************************************/
bool WriteMsg(char* dst, char* content)
{   
    recvCount = 0;
    SendString((unsigned char*)"AT+CMGS=");
    SendChar(0x22);//"
    SendString((unsigned char*)dst);//destination call number
    SendChar(0x22);//"
    SendString((unsigned char*)"\r\n");
    Delay(10);
    SendString((unsigned char*)content);
    SendChar(0x1A);
    Delay(10);
    if (recvMsg[24] != '>')
    {
      return false;
    }
    return true;
}

/******************************************************************************
//接收所有英文短信
//输入参数：content——内容
            dst——目标对象
//输出参数：状态（true or false）
*******************************************************************************/
bool ReadMsg(unsigned char *(*readMsg[])[], uint8_t *readCount)
{

  bool recvFlag = false;
  recvCount = 0;
  SendString((unsigned char*)"AT+CMGL=");
  SendChar(0x22);
  SendString((unsigned char*)"ALL");
  SendChar(0x22);
  SendString((unsigned char*)"\r\n");
  Delay(200);
  if (recvMsg[16]!='+'||recvMsg[17]!='C')
  {
    recvFlag = false;
  }
  else
  {
    *readCount = 0;
    char* ptr = strstr((char*)recvMsg, "+CMGL:");
    while(ptr != NULL)
    {//????待完善
      
      
      ptr += 7;
      /*decompose the readMsg: index*/
      uint16_t ptrCount = 0;
      (*readMsg[*readCount])[0] = (unsigned char*)ptr;//readMsg index start addr
      while(*(ptr++) != ',') ptrCount++;      
      *((*readMsg[*readCount])[0]+ptrCount) = (unsigned char)0x00;
      
      /*decompose the readMsg: status*/
      ptr += 1;
      ptrCount = 0;
      (*readMsg[*readCount])[1] = (unsigned char*)ptr;//readMsg status start addr
      while(*(ptr++) != '"') ptrCount++;
      *((*readMsg[*readCount])[1]+ptrCount) = (unsigned char)0x00;
      
      /*decompose the readMsg: call number*/
      ptr += 2;
      ptrCount = 0;
      (*readMsg[*readCount])[2] = (unsigned char*)ptr;//readMsg call number start addr
      while(*(ptr++) != '"') ptrCount++;
      *((*readMsg[*readCount])[2]+ptrCount) = (unsigned char)0x00;
      
      //adjust ptr
      while(*(ptr++) != '"');
      
      /*decompose the readMsg: timestamp number*/
      ptrCount = 0;
      (*readMsg[*readCount])[3] = (unsigned char*)ptr;//readMsg timestamp start addr
      while(*(ptr++) != '"') ptrCount++;
      *((*readMsg[*readCount])[3]+ptrCount) = (unsigned char)0x00;
      
      
      /*decompose the readMsg: num*/
      ptr += 1 ;
      ptrCount = 0;
      (*readMsg[*readCount])[4] = (unsigned char*)ptr;//readMsg num start addr
      while(*(ptr++) != ',') ptrCount++;
      *((*readMsg[*readCount])[4]+ptrCount) = (unsigned char)0x00;
      
      /*decompose the readMsg: length*/
      ptrCount = 0;
      (*readMsg[*readCount])[5] = (unsigned char*)ptr;//readMsg length start addr
      while(*(ptr++) != '\r') ptrCount++;
      *((*readMsg[*readCount])[5]+ptrCount) = (unsigned char)0x00;
      
      
      
      /*decompose the readMsg: content*/
      ptr += 1;
      ptrCount = 0;
      (*readMsg[*readCount])[6] = (unsigned char*)ptr;//readMsg content start addr
      while(*(ptr++) != '\r') ptrCount++;
      *((*readMsg[*readCount])[6]+ptrCount) = (unsigned char)0x00;
            
      
      ptr = strstr(ptr, "+CMGL:");//match the next substring
      *readCount++;
    }
    recvFlag = true;
  }
  return recvFlag;
}

/******************************************************************************
//删除一条英文短信
//输入参数：content——内容
            dst——目标对象
//输出参数：状态（true or false）
*******************************************************************************/
bool DeleteMsg(char* deleteNum)
{
  recvCount = 0;
  SendString((unsigned char*)"AT+CMGD=");
  SendString((unsigned char*)deleteNum);
  SendString((unsigned char*)"\r\n");
  Delay(20);
  if(recvMsg[recvCount-4]!='O'||recvMsg[recvCount-3]!='K')
  {
    return false;
  }
  else
  {
    return true;
  }
}
/******************************************************************************
//获得当前时间
//输入参数：
//输出参数：
*******************************************************************************/
void getRT(void)//???
{
    //AT+CCLK
}
/******************************************************************************
//向上位机控制中心注册信息
//输入参数：
//输出参数：
*******************************************************************************/
bool registration(void)//????????
{   
    unsigned char *(*readMsg[10])[4];
    uint8_t readCount = 0;
    WriteMsg("8618200259160", "zc?#1");
    Delay(500);//concrete delay time should be adjusted
    ReadMsg(readMsg, &readCount);
    if(1)//判断反馈信息
    {
      return true;
    }
    else
    {
      return false;
    }
}

/******************************************************************************
//反馈处理函数
//输入参数：
//输出参数：
*******************************************************************************/
void handleFeedback(void)
{
  
}