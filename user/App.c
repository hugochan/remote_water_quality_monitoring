#include "App.h"

void NVIC_Config(void);
void USART1_Config(void);
void SendChar(unsigned char ch);
void SendString(unsigned char *p);
unsigned char GetChar(void);
bool gsmConfig(void);
bool handShake(void);
bool login(void);
bool WriteMsg(char* dst, char *content);
bool ReadMsg(unsigned char *(*readMsg[])[], uint8_t readCount);
void getRT(void);


__IO uint16_t TimeDev = 100;//sys中断周期为1000/TimeDev=10ms 
bool gsmConfigFlag = false;
bool onlineFlag = false;//在线状态标志位
bool loginFlag = false;
unsigned char recvMsg[1000];

uint8_t recvCount = 0;
//extern bool recvFlag;
void main()
{ 
  unsigned char *(*readMsg[10])[7];
  uint8_t readCount = 0;
  RCC_ClocksTypeDef RCC_Clocks;
  /* SysTick end of count event each 10ms */
  RCC_GetClocksFreq(&RCC_Clocks);
  SysTick_Config(RCC_Clocks.HCLK_Frequency / TimeDev);
   
  //getRT();//获得当前时间
  USART1_Config();
  NVIC_Config();
  gsmConfigFlag = gsmConfig();
  //gsmConfigFlag = WriteMsg("18200259160", "hi");
  ReadMsg(readMsg, readCount);
  
  
  
  while(1)
  {
    
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
//接收一条英文短信
//输入参数：content——内容
            dst——目标对象
//输出参数：状态（true or false）
*******************************************************************************/
bool ReadMsg(unsigned char *(*readMsg[])[], uint8_t readCount)
{
  bool recvFlag = false;
  recvCount = 0;
  SendString((unsigned char*)"AT+CMGL=");
  SendChar(0x22);
  SendString((unsigned char*)"ALL");
  SendChar(0x22);
  SendString((unsigned char*)"\r\n");
  Delay(100);
  if (recvMsg[recvCount-4]!='O'||recvMsg[recvCount-3]!='K')
  {
    recvFlag = false;
  }
  else
  {
    uint8_t i = 0;
    char* ptr = strstr((char*)recvMsg, "+CMGL:");
    while(ptr != NULL)
    {//????待完善
      (*readMsg[i])[0] = (unsigned char*)(ptr+7);//readMsg index start addr
      (*readMsg[i])[1] = (unsigned char*)(ptr+21);//readMsg call Number start addr
      (*readMsg[i])[2] = (unsigned char*)(ptr+38);//readMsg timestamp start addr
      (*readMsg[i])[3] = (unsigned char*)(ptr+67);//readMsg content start addr
      ptr = strstr(ptr+strlen("+CMGL:"), "+CMGL:");//match the next substring
      i++;
    }
    recvFlag = true;
  }
  return recvFlag;
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
bool login(void)
{
    loginFlag = true;
    return loginFlag;
}