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
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//ATK-SIM800C GSM/GPRSģ������	  
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2016/4/1
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved	
//********************************************************************************
//��


//�������
unsigned long  Time_Cont = 0;       //��ʱ��������
static unsigned char *SMS_Num="\"+8613800759500\"";//�������ĺ� �������ĺŵĽ��ܿ��Կ����ڵ��Աʼǵĵڶ�����
static unsigned char *phone="\"18925407827\"";     //���Ž��շ��ֻ���
static unsigned char *content="HELP!";//���Ͷ�������
char sendCom1[2] = {0x1A};	//������ֹ��
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

//usmart֧�ֲ��� 
//���յ���ATָ��Ӧ�����ݷ��ظ����Դ���
//mode:0,������USART2_RX_STA;
//     1,����USART2_RX_STA;
void sim_at_response(u8 mode)
{
	if(USART2_RX_STA&0X8000)		//���յ�һ��������
	{ 
		USART2_RX_BUF[USART2_RX_STA&0X7FFF]=0;//��ӽ�����
		printf("%s",USART2_RX_BUF);	//���͵�����
		if(mode)USART2_RX_STA=0;
	} 
}
//////////////////////////////////////////////////////////////////////////////////
//ATK-SIM800C �������(���Ų��ԡ����Ų��ԡ�GPRS���ԡ���������)���ô���

//sim800C���������,�����յ���Ӧ��
//str:�ڴ���Ӧ����
//����ֵ:0,û�еõ��ڴ���Ӧ����
//����,�ڴ�Ӧ������λ��(str��λ��)
u8* sim800c_check_cmd(u8 *str)
{
	char *strx=0;
	if(USART2_RX_STA&0X8000)		//���յ�һ��������
	{ 
		USART2_RX_BUF[USART2_RX_STA&0X7FFF]=0;//��ӽ�����
		strx=strstr((const char*)USART2_RX_BUF,(const char*)str);
	} 
	return (u8*)strx;
}
//��sim800C��������
//cmd:���͵������ַ���(����Ҫ��ӻس���),��cmd<0XFF��ʱ��,��������(���緢��0X1A),���ڵ�ʱ�����ַ���.
//ack:�ڴ���Ӧ����,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��
//waittime:�ȴ�ʱ��(��λ:10ms)
//����ֵ:1,���ͳɹ�(�õ����ڴ���Ӧ����)
//       0,����ʧ��
unsigned int sendCommand(char *Command, char *Response, unsigned long Timeout, unsigned char Retry)
{
	unsigned char n;
	USART2_CLR_Buf();
	for (n = 0; n < Retry; n++)
	{
		u2_printf(Command); 		//����GPRSָ��
		

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

//����SIM800C�������ݣ���������ģʽ��ʹ�ã�
//request:�ڴ����������ַ���
//waittimg:�ȴ�ʱ��(��λ��10ms)
//����ֵ:0,���ͳɹ�(�õ����ڴ���Ӧ����)
//       1,����ʧ��
//u8 sim800c_wait_request(u8 *request ,u16 waittime)
//{
//	 u8 res = 1;
//	 u8 key;
//	 if(request && waittime)
//	 {
//		while(--waittime)
//		{   
//		   key=KEY_Scan(0);
//		   if(key==WKUP_PRES) return 2;//������һ��
//		   delay_ms(10);
//		   if(USART2_RX_STA &0x8000)//���յ��ڴ���Ӧ����
//		   {
//			  if(sim800c_check_cmd(request)) break;//�õ���Ч����
//			  USART2_RX_STA=0;
//		   }
//		}
//		if(waittime==0)res=0;
//	 }
//	 return res;
//}

//��1���ַ�ת��Ϊ16��������
//chr:�ַ�,0~9/A~F/a~F
//����ֵ:chr��Ӧ��16������ֵ
u8 sim800c_chr2hex(u8 chr)
{
	if(chr>='0'&&chr<='9')return chr-'0';
	if(chr>='A'&&chr<='F')return (chr-'A'+10);
	if(chr>='a'&&chr<='f')return (chr-'a'+10); 
	return 0;
}
//��1��16��������ת��Ϊ�ַ�
//hex:16��������,0~15;
//����ֵ:�ַ�
u8 sim800c_hex2chr(u8 hex)
{
	if(hex<=9)return hex+'0';
	if(hex>=10&&hex<=15)return (hex-10+'A'); 
	return '0';
}

/*******************************************************************************
* ������ : Send_Text_Sms
* ����   : ����TEXT�ı�����
* ����   : 
* ���   : 
* ����   : 
* ע��   : 
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
	
	strcat(num,(char*)SMS_Num); //�ַ���ƴ�Ӻ������⺯����
	strcat(num, "\r\n");
	if(sendCommand(num,"OK",2000,2))
		printf("Mesag--OK2\r\n");		
	else {printf("ERROR--Mesag2\r\n");}
	
	strcat(temp,(char*)phone); //�ַ���ƴ�Ӻ������⺯����
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
	{printf("mes���ͳɹ�\r\n");return 1;}
	else {printf("mesag����ERROR--11\r\n");return 0;}

}










