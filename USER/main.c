#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "sim800c.h"	
#include "usart2.h"
#include "usart3.h"
#include "gps.h"
#include "stdio.h"
#include "string.h"
#include "hc-sr04.h"
#include "wt-588d.h"
#include "mpu6050.h"
#include "mpuiic.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h" 
#include "timer.h"
#include "exti.h"

/*************************************************
*
*Ӳ������
*      ATK-SIM800C       ALIENTEK STM32������        HC-SR04        gps                         
*        STXD<------------>PA3(RXD)  |  PC1  ----- TRIG            TXD<------------>PB11(RXD)
*        SRXD<------------>PA2(TXD)  |  PA0  ----- ECHO            RXD<------------>PB10(TXD)
*         GND<------------>GND       |  VCC5 ----- VCC
*	   VCC_IN<------------>VCC5	     |
*  
*      MPU6050                      WT588D
*      IIC_SCL ------ PC12            P03 ------------ PA12
*      IIC_SDA ------ PC11
*
************************************************/
char sendCom[2] = {0x1A};	//������ֹ��
//static unsigned char *SMS_Num="\"+8613800759500\"";//�������ĺ� �������ĺŵĽ��ܿ��Կ����ڵ��Աʼǵĵڶ�����
//static unsigned char *phone="\"18925407827\"";     //���Ž��շ��ֻ���
//static unsigned char *content="HELP!";//���Ͷ�������
//char sendCom1[2] = {0x1A};	//������ֹ��
u8 mes;
 u8 hlp;

u8 USART1_TX_BUF[USART3_MAX_RECV_LEN]; 					//����1,���ͻ�����
nmea_msg gpsx; 											//GPS��Ϣ
__align(4) u8 dtbuf[50];   								//��ӡ������
const u8*fixmode_tbl[4]={"Fail","Fail"," 2D "," 3D "};	//fix mode�ַ��� 


char OneNetServer[] = "183.230.40.34";       //����Ҫ�޸�
char device_id[] = "518366602";    //�޸�Ϊ�Լ����豸ID
char API_KEY[] = "MbgA4hh=S5Wy=pmd5=t7Rd4yGMs=";    //�޸�Ϊ�Լ���API_KEY
char sensor_gps[] = "location";				//����Ҫ�޸�


extern u8  TIM2CH1_CAPTURE_STA;		//���벶��״̬		    				
extern u16	TIM2CH1_CAPTURE_VAL;	//���벶��ֵ

//��λ
//void Sys_Soft_Reset(void)
//{  
//    SCB->AIRCR =0X05FA0000|(u32)0x04;      
//}

//void errorLog(int num)
//{
////	printf("ERROR%d\r\n",num);
//	while (1)
//	{
//		if (sendCommand("AT\r\n", "OK", 100, 10) == 1)
//		{
//			Sys_Soft_Reset();
//		}
//		delay_ms(200);
//	}
//}



void postGpsDataToOneNet(char* API_VALUE_temp, char* device_id_temp, char* sensor_id_temp, float  lon_temp, float  lat_temp)
{
	char send_buf[400] = {0};
	char text[200] = {0};
	char tmp[25] = {0};

//	delay_ms(3000);
//	printf("%f\r\n",lon_temp );
	if(sendCommand("AT+CGCLASS=\"B\"\r\n","OK",2000,2))
		printf("B--OK3\r\n");		
	else printf("ERROR--3\r\n");	
	delay_ms(50);
	if(sendCommand("AT+CGDCONT=1,\"IP\",\"CMNET\"\r\n","OK",2000,2))
		printf("IP, CMNET--OK4\r\n");
	else printf("ERROR--4\r\n");
	delay_ms(50);
	if(sendCommand("AT+CGATT=1\r\n","OK",3000,2))
		printf("AT+CGATT--OK5\r\n");		
	else printf("ERROR--5\r\n");
	delay_ms(50);
	if(sendCommand("AT+CIPCSGP=1,\"CMNET\"\r\n","OK",2000,2))
		printf("AT+CIPCSGP--OK6\r\n");
	else printf("ERROR--6\r\n");
	delay_ms(100);
	if(sendCommand("AT+CIPHEAD=1\r\n","OK",3000,2))
		printf("AT+CIPHEAD--OK7\r\n");
	else printf("ERROR--7\r\n");
	delay_ms(100);
	//���ӷ�����
	memset(send_buf, 0, 400);    //���
	strcpy(send_buf, "AT+CIPSTART=\"TCP\",\"");
	strcat(send_buf, OneNetServer);      //���������ַ���
	strcat(send_buf, "\",\"80\"\r\n");
	if(sendCommand(send_buf, "CONNECT",10000,2))
		printf("���������ӳɹ�\r\n");
	else printf("ERROR--8\r\n");
	if(sendCommand("AT+CIPSEND\r\n",">",3000,1))
		printf("׼������\r\n");
	else printf("ERROR--9\r\n");
	memset(send_buf, 0, 400);    //���
	/*׼��JSON��*/
	sprintf(text, "{\"loc\":{\"lon\":%f,\"lat\":%f}}"
	       ,  lon_temp, lat_temp);			//ת������
					
	/*׼��HTTP��ͷ*/
	send_buf[0] = 0;
	strcat(send_buf, "POST http://api.heclouds.com/devices/");
	strcat(send_buf, device_id_temp);
	strcat(send_buf, "/datapoints?type=3 HTTP/1.1\r\n"); //ע�����������\r\n
	strcat(send_buf, "User-Agent:Fiddler\r\n");
	strcat(send_buf, "api-key:");
	strcat(send_buf, API_VALUE_temp);
	strcat(send_buf, "\r\n");
	strcat(send_buf, "Host:api.heclouds.com");
	strcat(send_buf, "\r\n");
	sprintf(tmp, "Content-Length:%d\r\n\r\n", strlen(text)); //����JSON������
	strcat(send_buf, tmp);
	strcat(send_buf, text);
//	u2_printf("%s\r\n",send_buf);
	
	if (sendCommand(send_buf, send_buf, 1000, 1))
		printf("%s\r\n",send_buf);
	else printf("ERROR--10\r\n");
	if(sendCommand(sendCom,"\"succ\"}",2000,1))
		printf("���ͳɹ�\r\n");
	else printf("����ERROR--11\r\n");
	//	else    SCB->AIRCR =0X05FA0000|(u32)0x04;      
	if(sendCommand("AT+CIPCLOSE=1\r\n","CLOSE OK",1000,1))	//�ر�����
		printf("�ر����ӳɹ�\r\n");
	else printf("�ر�����ERROR--12\r\n");
	if(sendCommand("AT+CIPSHUT\r\n","SHUT OK",6000,1))		//�ر��ƶ�����
		printf("�Ͽ�������\r\n");
	else printf("�Ͽ�������ERROR--12\r\n");
}
	  
 
 int main(void)
 {	 
	 // unsigned long	 count; //������������
//***************************************mpu And hc-sr04 ��������**************************************	 	 
//	u32 temp=0; 
//	 float t;
//	float UltrasonicWave_Distance=0;      //������ľ���
//	 
//***************************************gps��������******************************************************************************************
	  u16 i,rxlen;
	 u8 key=0XFF;	
    float tp, tp1,tp2;		

//***************************************��ʼ��******************************************************************************************	 
	NVIC_Configuration();	 
	delay_init();	    	         //��ʱ������ʼ��	  
	uart_init(115200);	 	         //���ڳ�ʼ��Ϊ9600														    
	USART2_Init(115200);	         //��ʼ������2
	USART3_Init(38400);  //��ʼ������2������Ϊ115200	
//	TIM5_Int_Init(14999,18999);//10Khz�ļ���Ƶ�ʣ�������5000Ϊ500ms 
	
	while(mpu_dmp_init())
	{
	  printf("mpu_init ERRROR!\n");
	}
//mpu_dmp_init();
	MPU6050_Config();
 EXTIX_Init();	 
	WT_Init();	 
// 	TIM1_PWM_Init(899,0); 			//����Ƶ��PWMƵ��=72000/(899+1)=80Khz
	UltrasonicWave_Configuration();
 	TIM2_Cap_Init(0XFFFF,72-1);		//��1Mhz��Ƶ�ʼ��� 
	
//***********���GPSģ���Ƿ���λ.*********
	if(SkyTra_Cfg_Rate(5)!=0)	//���ö�λ��Ϣ�����ٶ�Ϊ5Hz,˳���ж�GPSģ���Ƿ���λ. 
	{
		printf("S1216F8-BD Setting...");
		do
		{
			USART3_Init(9600);			//��ʼ������3������Ϊ9600
			SkyTra_Cfg_Prt(3);			//��������ģ��Ĳ�����Ϊ38400
			USART3_Init(38400);			//��ʼ������3������Ϊ38400
			key=SkyTra_Cfg_Tp(100000);	//������Ϊ100ms
		}while(SkyTra_Cfg_Rate(5)!=0&&key!=0);//����SkyTraF8-BD�ĸ�������Ϊ5Hz
		printf("S1216F8-BD Set Done!!");
		delay_ms(500);
	}
	printf("OjbK\r\n");
	while(1)
	{		
//*********************************************************GPS**************************************************************************************
		u32 temp=0; 
	float UltrasonicWave_Distance=0;      //������ľ���

		UltrasonicWave_StartMeasure();//��ʼ���
//	t=MPU_Get_Temperature()/100.0000;		//�õ��¶�
	if(TIM2CH1_CAPTURE_STA&0X80)//�ɹ�������һ�θߵ�ƽ
	{
		printf("measure\n");
		temp=TIM2CH1_CAPTURE_STA&0X3F;
		temp*=65536;					//���ʱ���ܺ�
		temp+=TIM2CH1_CAPTURE_VAL;		//�õ��ܵĸߵ�ƽʱ��
		UltrasonicWave_Distance=(float)temp*170/10000;//������㹫ʽ������=�ߵ�ƽ����ʱ��*����/2   
		if(UltrasonicWave_Distance<=300)
		{
			Data_Pro(UltrasonicWave_Distance);
		
		
			
		}
		printf("distance:%f cm\r\n",UltrasonicWave_Distance); //��ӡ����
		delay_ms(500);
		TIM2CH1_CAPTURE_STA=0;			//������һ�β���
	}	
		if(mes!=0)
		{
			Send_Text_Sms(2);
				mes=0;
//			printf("mes-------------%d\r\n", mes);
		}
		if(hlp==1)
		{
			Send_Text_Sms(1);
				hlp=0;
//			printf("mes-------------%d\r\n", mes);
		}
		if(USART3_RX_STA&0X8000)		//���յ�һ��������
		{
			rxlen=USART3_RX_STA&0X7FFF;	//�õ����ݳ���
			for(i=0;i<rxlen;i++)USART1_TX_BUF[i]=USART3_RX_BUF[i];	   
 			USART3_RX_STA=0;		   	//������һ�ν���
//			USART1_TX_BUF[i]=0;			//�Զ���ӽ�����
			GPS_Analysis(&gpsx,(u8*)USART1_TX_BUF);//�����ַ���  
			tp=gpsx.longitude;	   
			sprintf((char *)dtbuf,"Longitude:%.5f %1c   ",tp/=100000,gpsx.ewhemi);	//�õ������ַ���
			tp=gpsx.latitude;	   
			sprintf((char *)dtbuf,"Latitude:%.5f %1c   ",tp/=100000,gpsx.nshemi);	//�õ�γ���ַ���
			tp=gpsx.altitude;	   
			sprintf((char *)dtbuf,"Altitude:%.1fm     ",tp/=10);	    			//�õ��߶��ַ���
			tp=gpsx.speed;	   
			sprintf((char *)dtbuf,"Speed:%.3fkm/h     ",tp/=1000);		    		//�õ��ٶ��ַ���	 
			if(gpsx.fixmode<=3)														//��λ״̬
			{  
				sprintf((char *)dtbuf,"Fix Mode:%s",fixmode_tbl[gpsx.fixmode]);			   
			}	 	   
			sprintf((char *)dtbuf,"GPS+BD Valid satellite:%02d",gpsx.posslnum);	 		//���ڶ�λ��GPS������
			sprintf((char *)dtbuf,"GPS Visible satellite:%02d",gpsx.svnum%100);	 		//�ɼ�GPS������
			sprintf((char *)dtbuf,"BD Visible satellite:%02d",gpsx.beidou_svnum%100);	 		//�ɼ�����������						
		
			

			//*************************************���͵�������******************************************************************************
			while(!(sendCommand ("AT\r\n","OK",2000,4)))
				{
					printf("1\r\n");
				}
//				printf("AT--OK1\r\n");
//			else printf("ERROR--1\r\n");
	////		sendCommand("ATE0","OK",200);
			if(sendCommand ("AT+CPIN?\r\n","READY",3000,2))
				printf("AT+CPIN?--OK2\r\n");
			else printf("ERROR--2\r\n");
			delay_ms(50);

			tp1=gpsx.longitude;
			tp1/=100000;
			tp2=gpsx.latitude;
			tp2/=100000;
			
			tp1 = 22.905069;
			tp2 = 113.874222;
			postGpsDataToOneNet(API_KEY, device_id, sensor_gps,tp1 , tp2);		//�������ݵ�Onenet

		}
		
//		Line_1A(13);
//	delay_ms(1500);
  }

}
 

