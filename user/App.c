#include "stm32f4_discovery.h"
#include <stdbool.h>

void NVIC_Config(void);
void USART1_Config(void);
void SendChar(unsigned char ch);
void SendString(unsigned char *p);
unsigned char GetChar(void);
bool GSM_Config(void);
bool handShake(void);
bool login(void);
bool SendMesg(void);
void getRT(void);

bool onlineFlag = false;//在线状态标志位
bool loginFlag = false;
extern char recvMesg[100];
extern bool recvFlag;
void main()
{ 
  RCC_ClocksTypeDef RCC_Clocks;
  RCC_GetClocksFreq(&RCC_Clocks);
  
  getRT();//获得当前时间
  USART1_Config();
  NVIC_Config();
  GSM_Config();
  while(!login());//直到注册成功退出循环
  SendString("atd18200259160;\r\n");

  
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
bool GSM_Config(void)//????
{   
    while(!handShake());
    //SendString('at');
    //USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);//使能串口接收中断
    onlineFlag = true;
    return onlineFlag;
}
/******************************************************************************
//握手测试
//输入参数：
//输出参数：状态（true or false）
*******************************************************************************/
bool handShake(void)//????
{
    SendString("at\r\n");
    //USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);//使能串口接收中断
    if(recvFlag&&(recvMesg=="ok\r\n")){}
    onlineFlag = true;
    return onlineFlag;
}
/******************************************************************************
//发送一条短信
//输入参数：content——内容
            dest——目标对象
//输出参数：状态（true or false）
*******************************************************************************/
bool SendMesg(void)//???
{
    return true;
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