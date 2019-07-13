#include "sim800c.h"
#include "usart.h"		
#include "delay.h"		 	 	 	 	 
//#include "touch.h" 	 
//#include "malloc.h"
#include "string.h"    
//#include "text.h"		
#include "usart2.h" 
#include "exti.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//ATK-SIM800C GSM/GPRS模块驱动	  
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2016/4/1
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved	
//********************************************************************************
//无


//定义变量
unsigned long  Time_Cont = 0;       //定时器计数器
static unsigned char *SMS_Num="\"+8613800759500\"";//短信中心号 关于中心号的介绍可以看串口调试笔记的第二部分
static unsigned char *phone="\"18925407827\"";     //短信接收方手机号
static unsigned char *content="HELP!";//发送短信内容
char sendCom1[2] = {0x1A};	//发送终止符
//void Send_Message(u8 *num,u8 *Msg)
//{
//	char cmd[20];
//	//u2_printf("AT+CMGF=1\r\n");
//	sim800c_send_cmd("AT+CMGF=1","OK",200);
////	printf("duanxin1");
//	//u2_printf("AT+CSCS=\"GSM\"\r\n");
//	sim800c_send_cmd("AT+CSCS=\"GSM\"","OK",200);
//	//u2_printf("AT+CSMP=17,167,0,240\r\n");
//	//u2_printf("AT+CMGS=\"%s\"\r\n",num);
//	sprintf((char*)cmd,"AT+CMGS=\"%s\"",num);
//	sim800c_send_cmd ((u8*)cmd,">",200);
//	//printf("duanxin2");
//	//delay_ms(90);
//	u2_printf("%s",Msg);
//	sim800c_send_cmd ((u8*)0x1A,"+CMGS",1000);
//	printf("%s\r\n",Msg);
////	USART2_SendByte(0x1A);
//	 //while((USART2->SR&0X40)==0);
//   //USART2->DR = 0x1A;	 
//}

//usmart支持部分 
//将收到的AT指令应答数据返回给电脑串口
//mode:0,不清零USART2_RX_STA;
//     1,清零USART2_RX_STA;
void sim_at_response(u8 mode)
{
	if(USART2_RX_STA&0X8000)		//接收到一次数据了
	{ 
		USART2_RX_BUF[USART2_RX_STA&0X7FFF]=0;//添加结束符
		printf("%s",USART2_RX_BUF);	//发送到串口
		if(mode)USART2_RX_STA=0;
	} 
}
//////////////////////////////////////////////////////////////////////////////////
//ATK-SIM800C 各项测试(拨号测试、短信测试、GPRS测试、蓝牙测试)共用代码

//sim800C发送命令后,检测接收到的应答
//str:期待的应答结果
//返回值:0,没有得到期待的应答结果
//其他,期待应答结果的位置(str的位置)
u8* sim800c_check_cmd(u8 *str)
{
	char *strx=0;
	if(USART2_RX_STA&0X8000)		//接收到一次数据了
	{ 
		USART2_RX_BUF[USART2_RX_STA&0X7FFF]=0;//添加结束符
		strx=strstr((const char*)USART2_RX_BUF,(const char*)str);
	} 
	return (u8*)strx;
}
//向sim800C发送命令
//cmd:发送的命令字符串(不需要添加回车了),当cmd<0XFF的时候,发送数字(比如发送0X1A),大于的时候发送字符串.
//ack:期待的应答结果,如果为空,则表示不需要等待应答
//waittime:等待时间(单位:10ms)
//返回值:1,发送成功(得到了期待的应答结果)
//       0,发送失败
unsigned int sendCommand(char *Command, char *Response, unsigned long Timeout, unsigned char Retry)
{
	unsigned char n;
	USART2_CLR_Buf();
	for (n = 0; n < Retry; n++)
	{
		u2_printf(Command); 		//发送GPRS指令
		

		Time_Cont = 0;
		while (Time_Cont < Timeout)
		{
			delay_ms(100);
			Time_Cont += 100;
			if (strstr(USART2_RX_BUF, Response) != NULL)
			{				
//				printf("\r\n***************receive****************\r\n");
//				printf(USART2_RX_BUF);
				return 1;
			}
			
		}
		Time_Cont = 0;
	}

	USART2_CLR_Buf();
	return 0;
} 

//接收SIM800C返回数据（蓝牙测试模式下使用）
//request:期待接收命令字符串
//waittimg:等待时间(单位：10ms)
//返回值:0,发送成功(得到了期待的应答结果)
//       1,发送失败
//u8 sim800c_wait_request(u8 *request ,u16 waittime)
//{
//	 u8 res = 1;
//	 u8 key;
//	 if(request && waittime)
//	 {
//		while(--waittime)
//		{   
//		   key=KEY_Scan(0);
//		   if(key==WKUP_PRES) return 2;//返回上一级
//		   delay_ms(10);
//		   if(USART2_RX_STA &0x8000)//接收到期待的应答结果
//		   {
//			  if(sim800c_check_cmd(request)) break;//得到有效数据
//			  USART2_RX_STA=0;
//		   }
//		}
//		if(waittime==0)res=0;
//	 }
//	 return res;
//}

//将1个字符转换为16进制数字
//chr:字符,0~9/A~F/a~F
//返回值:chr对应的16进制数值
u8 sim800c_chr2hex(u8 chr)
{
	if(chr>='0'&&chr<='9')return chr-'0';
	if(chr>='A'&&chr<='F')return (chr-'A'+10);
	if(chr>='a'&&chr<='f')return (chr-'a'+10); 
	return 0;
}
//将1个16进制数字转换为字符
//hex:16进制数字,0~15;
//返回值:字符
u8 sim800c_hex2chr(u8 hex)
{
	if(hex<=9)return hex+'0';
	if(hex>=10&&hex<=15)return (hex-10+'A'); 
	return '0';
}

/*******************************************************************************
* 函数名 : Send_Text_Sms
* 描述   : 发送TEXT文本短信
* 输入   : 
* 输出   : 
* 返回   : 
* 注意   : 
*******************************************************************************/
u8 Send_Text_Sms(u8 wh)
{
	char temp[50]="AT+CMGS=";
	char num[50]="AT+CSCA=";
	
	if(sendCommand("AT\r\n","OK",2000,2))
	printf("AT--OK1\r\n");
	else {printf("ERROR--1\r\n");}
	
	if(sendCommand("AT+CMGF=1\r\n","OK",2000,2))
			printf("Mesag--OK1\r\n");		
	else {printf("ERROR--Mesag1\r\n");}
	
	strcat(num,(char*)SMS_Num); //字符串拼接函数（库函数）
	strcat(num, "\r\n");
	if(sendCommand(num,"OK",2000,2))
		printf("Mesag--OK2\r\n");		
	else {printf("ERROR--Mesag2\r\n");}
	
	strcat(temp,(char*)phone); //字符串拼接函数（库函数）
	strcat(temp, "\r\n");
	if(sendCommand(temp,">",2000,2))
		printf("Mesag--OK3\r\n");		
	else {printf("ERROR--Mesag3\r\n");}
	
	if(wh==1)
	{
		if (sendCommand("HELP", "HELP", 1000, 2))
			printf("%s\r\n",content);
		else {printf("Mesa-H-ERROR--4\r\n");}
	}
	else 
	{
		if (sendCommand("DOWN", "DOWN", 1000, 2))
			printf("%s\r\n",content);
		else {printf("Mesa-D-ERROR--4\r\n");}
	}
//	strcat(sendCom1, "\r\n");
	if(sendCommand(sendCom1,"OK",3000,3))
	{printf("mes发送成功\r\n");return 1;}
	else {printf("mesag发送ERROR--11\r\n");return 0;}

}










