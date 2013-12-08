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
void getRT(void);
void IO_Init(void);

#define timerCounterFreq ((uint16_t)2000)

__IO uint32_t TIM2_Val = 6000;//jb应答定时器10s(优先级最高)
__IO uint32_t TIM5_Val = 12000;//注册阶段sj应答定时器||nj应答定时器20s
__IO uint32_t TIM3_Val = 18000;//sj事务定时器30s（优先级最低）


__IO uint16_t TimeDev = 100;//sys中断周期为1000/TimeDev=10ms
__IO uint8_t UserButtonPressed = 0x00;
unsigned char *data[3]={"8", "100", "0"};//ph flow state


bool machineState = false;//机器状态标志位（flase for 关机，true for 工作）
bool gsmConfigFlag = false;//gsm配置成功标志位
bool onlineFlag = false;//在线标志位（即向上位机注册成功）
bool sjFlag = false;//数据事务标志位
bool jbFlag = false;//警报标志位
bool njFlag = false;//解除警报标志位
bool waitingsjAckFlag = false;//等待数据应答标志位
bool waitingjbAckFlag = false;//等待警报应答标志位
bool waitingnjAckFlag = false;//等待解除警报应答标志位
bool sjAckTimeoutFlag = false;//等待数据应答超时标志位
bool jbAckTimeoutFlag = false;//等待警报应答超时标志位
bool njAckTimeoutFlag = false;//等待解除警报应答超时标志位
bool waitingCmdAck = false;//等待AT命令的应答
bool newMsgAdvertiseFlag = false;//收到新信息标志位

bool Success = false;//成功标志位
unsigned char recvCmdAck[1000];
uint16_t recvCmdAckCount = 0;
unsigned char recvNewMsgAdvertise[20];
uint16_t recvNewMsgAdvertiseCount = 0;
char newMsgIndex[3];
unsigned char *readMsg[3];
char generalCounter = 0;
//extern bool recvFlag;
void main()
{
  /* Initialize User_Button and Leds mounted on STM32F4-Discovery board */
  STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_EXTI); 
  STM_EVAL_LEDInit(LED4);
  STM_EVAL_LEDInit(LED3);
  STM_EVAL_LEDInit(LED5);
  STM_EVAL_LEDInit(LED6);
  STM_EVAL_LEDOff(LED4);
  STM_EVAL_LEDOff(LED3);
  STM_EVAL_LEDOff(LED5);
  STM_EVAL_LEDOff(LED6);
  IO_Init();
  
  RCC_ClocksTypeDef RCC_Clocks;
  /* SysTick end of count event each 10ms */
  RCC_GetClocksFreq(&RCC_Clocks);
  SysTick_Config(RCC_Clocks.HCLK_Frequency / TimeDev);
  NVIC_SetPriority (SysTick_IRQn, 1); //设置systick为最高优先级
  SysTick->CTRL  &= ~SysTick_CTRL_ENABLE_Msk;//失能systick
 
  //getRT();//获得当前时间
  USART1_Config();
  NVIC_Config();
  TIM2_Config();
  TIM5_Config();
  TIM3_Config();
  
  gsmConfigFlag = true;
  onlineFlag = true;
  while(1)
  {
    if (gsmConfigFlag)
    {
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
             if (strcmp((char*)readMsg[2], "sj?ack#")==0)//sj应答处理
             {
               
             }
             else if (strcmp((char*)readMsg[2], "zt?#")==0)//zt命令响应
             {
               generalCounter = 0; 
               Success = WriteMsg(upperComputerNum,"zt?ph=[data]&flow=[data]&state=[state]#num_date");
               while (!Success) 
               {
                 Success = WriteMsg(upperComputerNum,"zt?ph=[data]&flow=[data]&state=[state]#num_date");
                 generalCounter++;
                 if(generalCounter>2) break;
               }
             }
             else if (strcmp((char*)readMsg[2], "jb?ack#")==0)//jb应答处理
             {
                jbFlag = false;
                waitingjbAckFlag = false;//jb应答成功，不再监测jb应答
                jbAckTimeoutFlag = false;
             }
             
             else if (strcmp((char*)readMsg[2], "nj?ack#")==0)//nj应答处理
             {
                njFlag = false;
                waitingnjAckFlag = false;//jb应答成功，不再监测jb应答
                njAckTimeoutFlag = false;
             }
             
             else if ((strcmp((char*)readMsg[2], "kz?state=0#")==0)||(strcmp((char *)readMsg[2], "kz?state=1#")==0))
             {//kz命令响应
               
               if (*(readMsg[2]+9) == '0') machineState = false;
               else if (*(readMsg[2]+9) == '1') machineState = true; 
               
               generalCounter = 0; 
               Success = WriteMsg(upperComputerNum,"kz?ack#num_date");
               while (!Success) 
               {
                 Success = WriteMsg(upperComputerNum,"kz?ack#num_date");
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
         
         if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == Bit_SET)//不太灵敏
         {
            while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == Bit_SET);
            STM_EVAL_LEDOff(LED4);//解除警报指示
            njFlag = true;  
         }
         if (njFlag&&(!waitingnjAckFlag))//发送nj信息
         {
            machineState = true;
            atAdjust();
            generalCounter = 0;
            Success = WriteMsg(upperComputerNum, "nj?ph=[data]&state=1#num_date");
            while (!Success) 
            {
              atAdjust();
              Success = WriteMsg(upperComputerNum, "nj?ph=[data]&state=1#num_date");
              generalCounter++;
              if(generalCounter>2) break;
            }
            waitingnjAckFlag = true;
            njFlag = false;
            TIM_Cmd(TIM5, ENABLE);//打开nj应答定时器
         }
         
         if (njAckTimeoutFlag)//nj应答超时重发
         {
           generalCounter = 0;
           atAdjust();
           Success = WriteMsg(upperComputerNum, "nj?ph=[data]&state=1#num_date");
           while (!Success) 
           {
              atAdjust();
              Success = WriteMsg(upperComputerNum, "nj?ph=[data]&state=1#num_date");
              generalCounter++;
              if(generalCounter>2) break;
            }
           njAckTimeoutFlag = false;
           TIM_Cmd(TIM5, ENABLE);//打开nj应答定时器
         }
         
         
         
         
         
         if (jbFlag&&(!waitingjbAckFlag))//发送jb信息
         {
           machineState = false;
           generalCounter = 0;
           atAdjust();
           Success = WriteMsg(upperComputerNum, "jb?ph=[data]&state=0#num_date");
           while (!Success) 
           {
              atAdjust();
              Success = WriteMsg(upperComputerNum, "jb?ph=[data]&state=0#num_date");
              generalCounter++;
              if(generalCounter>2) break;
           }
           waitingjbAckFlag = true;
           jbFlag = false;
           TIM_Cmd(TIM2, ENABLE);//打开jb应答定时器
         }
         
         if (jbAckTimeoutFlag)//jb应答超时重发
         {
           generalCounter = 0;
           atAdjust();
           Success = WriteMsg(upperComputerNum, "jb?ph=[data]&state=0#num_date");
           while (!Success) 
           {
              atAdjust();
              Success = WriteMsg(upperComputerNum, "jb?ph=[data]&state=0#num_date");
              generalCounter++;
              if(generalCounter>2) break;
            }
           jbAckTimeoutFlag = false;
           TIM_Cmd(TIM2, ENABLE);//打开jb应答定时器
         }
         
         
         if (sjFlag)//定期发送sj信息，注册成功后不验证应答信息
         {
           generalCounter = 0;
           atAdjust();//调整指令步伐
           Success = WriteMsg(upperComputerNum, "sj?ph=[data]&flow=[data]&state=[state]#num_date");
           while (!Success) 
           {
              atAdjust();
              Success = WriteMsg(upperComputerNum, "sj?ph=[data]&flow=[data]&state=[state]#num_date");
              generalCounter++;
              if(generalCounter>2) break;//一共尝试三次
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
                if (strcmp((char*)readMsg[2], "sj?ack#")==0)//sj应答处理
                {
                  TIM_SetCounter(TIM3, (uint32_t)0);//软件清零sj事务定时器，防止注册成功后立即发送重复信息
                  /*注册成功*/
                  onlineFlag = true;              
                  sjAckTimeoutFlag = false;
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
          
          if(sjAckTimeoutFlag)
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
        //deleate all messages,删除可能的信息积存
        DeleteAllMsgs();
      }  
    }
    
  }
}

/*********************************NVIC配置*******************************/
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


/*********************************定时器初始化*******************************/
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
  
  TIM_ClearFlag(TIM2, TIM_FLAG_Update);   //必须先清除配置时候产生的更新标志
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);   //使能中断，中断事件为定时器更新事件

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
 


  TIM_ClearFlag(TIM3, TIM_FLAG_Update);   //必须先清除配置时候产生的更新标志
  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);   //使能中断，中断事件为定时器更新事件

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
 


  TIM_ClearFlag(TIM5, TIM_FLAG_Update);   //必须先清除配置时候产生的更新标志
  TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);   //使能中断，中断事件为定时器更新事件

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
    waitingCmdAck = true;//等待AT指令应答标志位置位
    recvCmdAckCount = 0;
    SendString("AT+CMGF=1\r\n");
    Delay(10);
    waitingCmdAck = false;//等待AT指令应答标志位复位
    if(recvCmdAck[12]!='O'||recvCmdAck[13]!='K')
    {
      return false;
    }
    else
    {//先删除所有短信
      waitingCmdAck = true;//等待AT指令应答标志位置位
      recvCmdAckCount = 0;
      SendString("AT+CNMI=1,1,2\r\n");//打开新信息提示模式
      Delay(10);
      waitingCmdAck = false;//等待AT指令应答标志位复位
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
//握手测试
//输入参数：
//输出参数：状态（true or false）
*******************************************************************************/
bool handShake(void)
{       
    waitingCmdAck = true;//等待AT指令应答标志位置位
    recvCmdAckCount = 0;
    SendString("AT\r\n");
    Delay(10);
    waitingCmdAck = false;//等待AT指令应答标志位复位
    if(recvCmdAck[5]=='O'&&recvCmdAck[6]=='K')
    {
      return true;
    }
    return false;

}

/******************************************************************************
//调整指令步伐,防止at指令写入一错永错
//输入参数：
//输出参数：状态（无）
*******************************************************************************/
void atAdjust(void)
{       
    waitingCmdAck = true;//等待AT指令应答标志位置位
    recvCmdAckCount = 0;
    SendString("\r\n");
    Delay(10);
    waitingCmdAck = false;//等待AT指令应答标志位复位
}


/******************************************************************************
//发送一条英文短信
//输入参数：content——内容
            dst——目标对象
//输出参数：状态（true or false）
*******************************************************************************/
bool WriteMsg(char* dst, char* content)
{   
    waitingCmdAck = true;//等待AT指令应答标志位置位
    recvCmdAckCount = 0;
    SendString((unsigned char*)"AT+CMGS=");
    SendChar(0x22);//"
    SendString((unsigned char*)dst);//destination call number
    SendChar(0x22);//"
    SendString((unsigned char*)"\r\n");
    Delay(10);
    SendString((unsigned char*)content);
    SendChar(0x1A);
    Delay(100);
    waitingCmdAck = false;//等待AT指令应答标志位复位
    if (recvCmdAck[26] != '>')
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
bool ReadMsg(unsigned char* readMsg[], char* msgIndex)
{
  bool readFlag = false;
  waitingCmdAck = true;//等待AT指令应答标志位置位
  recvCmdAckCount = 0;
  SendString((unsigned char*)"AT+CMGR=");
  SendString((unsigned char*)msgIndex);
  SendString((unsigned char*)"\r\n");
  Delay(200);
  waitingCmdAck = false;//等待AT指令应答标志位复位
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
//接收所有英文短信
//输入参数：content——内容
            dst——目标对象
//输出参数：状态（true or false）
*******************************************************************************/
bool ReadMsg2(unsigned char *(*readMsg2[])[], uint8_t *readCount2)
{

  bool recvFlag = false;
  waitingCmdAck = true;//等待AT指令应答标志位置位
  recvCmdAckCount = 0;
  SendString((unsigned char*)"AT+CMGL=");
  SendChar(0x22);
  SendString((unsigned char*)"ALL");
  SendChar(0x22);
  SendString((unsigned char*)"\r\n");
  Delay(200);
  waitingCmdAck = false;//等待AT指令应答标志位复位
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
    {//????待完善
      
      
      ptr += 7;
      /*decompose the readMsg: index*/
      uint16_t ptrCount = 0;
      (*readMsg2[*readCount2])[0] = (unsigned char*)ptr;//readMsg index start addr
      while(*(ptr++) != ',') ptrCount++;
      *((*readMsg2[*readCount2])[1]+ptrCount) = (unsigned char)0x00;
      
      /*decompose the readMsg: status*/
      ptr += 1;//???第三次循环跑飞
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
//删除一条英文短信
//输入参数：content——内容
            dst——目标对象
//输出参数：状态（true or false）
*******************************************************************************/
bool DeleteMsg(char* deleteNum)
{
  waitingCmdAck = true;//等待AT指令应答标志位置位
  recvCmdAckCount = 0;
  SendString((unsigned char*)"AT+CMGD=");
  SendString((unsigned char*)deleteNum);
  SendString((unsigned char*)"\r\n");
  Delay(20);
  waitingCmdAck = false;//等待AT指令应答标志位复位
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
//删除所有英文短信
//输入参数：无
//输出参数：状态（true or false）
*******************************************************************************/
bool DeleteAllMsgs(void)
{
  waitingCmdAck = true;//等待AT指令应答标志位置位
  recvCmdAckCount = 0;
  SendString((unsigned char*)"AT+CMGD=1,4");
  SendString((unsigned char*)"\r\n");
  Delay(20);
  waitingCmdAck = false;//等待AT指令应答标志位复位
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
void registration(void)
{   
    atAdjust();
    generalCounter = 0;
    Success = WriteMsg(upperComputerNum, "sj?ph=[data]&flow=[data]&state=[state]#num_date");
    while (!Success) 
    {
        atAdjust();
        Success = WriteMsg(upperComputerNum, "sj?ph=[data]&flow=[data]&state=[state]#num_date");
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
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}