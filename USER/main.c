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
*硬件连接
*      ATK-SIM800C       ALIENTEK STM32开发板        HC-SR04        gps                         
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
char sendCom[2] = {0x1A};	//发送终止符
//static unsigned char *SMS_Num="\"+8613800759500\"";//短信中心号 关于中心号的介绍可以看串口调试笔记的第二部分
//static unsigned char *phone="\"18925407827\"";     //短信接收方手机号
//static unsigned char *content="HELP!";//发送短信内容
//char sendCom1[2] = {0x1A};	//发送终止符
u8 mes;
 u8 hlp;

u8 USART1_TX_BUF[USART3_MAX_RECV_LEN]; 					//串口1,发送缓存区
nmea_msg gpsx; 											//GPS信息
__align(4) u8 dtbuf[50];   								//打印缓存器
const u8*fixmode_tbl[4]={"Fail","Fail"," 2D "," 3D "};	//fix mode字符串 


char OneNetServer[] = "183.230.40.34";       //不需要修改
char device_id[] = "518366602";    //修改为自己的设备ID
char API_KEY[] = "MbgA4hh=S5Wy=pmd5=t7Rd4yGMs=";    //修改为自己的API_KEY
char sensor_gps[] = "location";				//不需要修改


extern u8  TIM2CH1_CAPTURE_STA;		//输入捕获状态		    				
extern u16	TIM2CH1_CAPTURE_VAL;	//输入捕获值

//软复位
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
	//连接服务器
	memset(send_buf, 0, 400);    //清空
	strcpy(send_buf, "AT+CIPSTART=\"TCP\",\"");
	strcat(send_buf, OneNetServer);      //连接两个字符串
	strcat(send_buf, "\",\"80\"\r\n");
	if(sendCommand(send_buf, "CONNECT",10000,2))
		printf("服务器连接成功\r\n");
	else printf("ERROR--8\r\n");
	if(sendCommand("AT+CIPSEND\r\n",">",3000,1))
		printf("准备发送\r\n");
	else printf("ERROR--9\r\n");
	memset(send_buf, 0, 400);    //清空
	/*准备JSON串*/
	sprintf(text, "{\"loc\":{\"lon\":%f,\"lat\":%f}}"
	       ,  lon_temp, lat_temp);			//转换数据
					
	/*准备HTTP报头*/
	send_buf[0] = 0;
	strcat(send_buf, "POST http://api.heclouds.com/devices/");
	strcat(send_buf, device_id_temp);
	strcat(send_buf, "/datapoints?type=3 HTTP/1.1\r\n"); //注意后面必须加上\r\n
	strcat(send_buf, "User-Agent:Fiddler\r\n");
	strcat(send_buf, "api-key:");
	strcat(send_buf, API_VALUE_temp);
	strcat(send_buf, "\r\n");
	strcat(send_buf, "Host:api.heclouds.com");
	strcat(send_buf, "\r\n");
	sprintf(tmp, "Content-Length:%d\r\n\r\n", strlen(text)); //计算JSON串长度
	strcat(send_buf, tmp);
	strcat(send_buf, text);
//	u2_printf("%s\r\n",send_buf);
	
	if (sendCommand(send_buf, send_buf, 1000, 1))
		printf("%s\r\n",send_buf);
	else printf("ERROR--10\r\n");
	if(sendCommand(sendCom,"\"succ\"}",2000,1))
		printf("发送成功\r\n");
	else printf("发送ERROR--11\r\n");
	//	else    SCB->AIRCR =0X05FA0000|(u32)0x04;      
	if(sendCommand("AT+CIPCLOSE=1\r\n","CLOSE OK",1000,1))	//关闭连接
		printf("关闭连接成功\r\n");
	else printf("关闭连接ERROR--12\r\n");
	if(sendCommand("AT+CIPSHUT\r\n","SHUT OK",6000,1))		//关闭移动场景
		printf("断开服务器\r\n");
	else printf("断开服务器ERROR--12\r\n");
}
	  
 
 int main(void)
 {	 
	 // unsigned long	 count; //声明步数变量
//***************************************mpu And hc-sr04 声明变量**************************************	 	 
//	u32 temp=0; 
//	 float t;
//	float UltrasonicWave_Distance=0;      //计算出的距离
//	 
//***************************************gps声明变量******************************************************************************************
	  u16 i,rxlen;
	 u8 key=0XFF;	
    float tp, tp1,tp2;		

//***************************************初始化******************************************************************************************	 
	NVIC_Configuration();	 
	delay_init();	    	         //延时函数初始化	  
	uart_init(115200);	 	         //串口初始化为9600														    
	USART2_Init(115200);	         //初始化串口2
	USART3_Init(38400);  //初始化串口2波特率为115200	
//	TIM5_Int_Init(14999,18999);//10Khz的计数频率，计数到5000为500ms 
	
	while(mpu_dmp_init())
	{
	  printf("mpu_init ERRROR!\n");
	}
//mpu_dmp_init();
	MPU6050_Config();
 EXTIX_Init();	 
	WT_Init();	 
// 	TIM1_PWM_Init(899,0); 			//不分频。PWM频率=72000/(899+1)=80Khz
	UltrasonicWave_Configuration();
 	TIM2_Cap_Init(0XFFFF,72-1);		//以1Mhz的频率计数 
	
//***********检测GPS模块是否在位.*********
	if(SkyTra_Cfg_Rate(5)!=0)	//设置定位信息更新速度为5Hz,顺便判断GPS模块是否在位. 
	{
		printf("S1216F8-BD Setting...");
		do
		{
			USART3_Init(9600);			//初始化串口3波特率为9600
			SkyTra_Cfg_Prt(3);			//重新设置模块的波特率为38400
			USART3_Init(38400);			//初始化串口3波特率为38400
			key=SkyTra_Cfg_Tp(100000);	//脉冲宽度为100ms
		}while(SkyTra_Cfg_Rate(5)!=0&&key!=0);//配置SkyTraF8-BD的更新速率为5Hz
		printf("S1216F8-BD Set Done!!");
		delay_ms(500);
	}
	printf("OjbK\r\n");
	while(1)
	{		
//*********************************************************GPS**************************************************************************************
		u32 temp=0; 
	float UltrasonicWave_Distance=0;      //计算出的距离

		UltrasonicWave_StartMeasure();//开始测距
//	t=MPU_Get_Temperature()/100.0000;		//得到温度
	if(TIM2CH1_CAPTURE_STA&0X80)//成功捕获到了一次高电平
	{
		printf("measure\n");
		temp=TIM2CH1_CAPTURE_STA&0X3F;
		temp*=65536;					//溢出时间总和
		temp+=TIM2CH1_CAPTURE_VAL;		//得到总的高电平时间
		UltrasonicWave_Distance=(float)temp*170/10000;//距离计算公式：距离=高电平持续时间*声速/2   
		if(UltrasonicWave_Distance<=300)
		{
			Data_Pro(UltrasonicWave_Distance);
		
		
			
		}
		printf("distance:%f cm\r\n",UltrasonicWave_Distance); //打印距离
		delay_ms(500);
		TIM2CH1_CAPTURE_STA=0;			//开启下一次捕获
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
		if(USART3_RX_STA&0X8000)		//接收到一次数据了
		{
			rxlen=USART3_RX_STA&0X7FFF;	//得到数据长度
			for(i=0;i<rxlen;i++)USART1_TX_BUF[i]=USART3_RX_BUF[i];	   
 			USART3_RX_STA=0;		   	//启动下一次接收
//			USART1_TX_BUF[i]=0;			//自动添加结束符
			GPS_Analysis(&gpsx,(u8*)USART1_TX_BUF);//分析字符串  
			tp=gpsx.longitude;	   
			sprintf((char *)dtbuf,"Longitude:%.5f %1c   ",tp/=100000,gpsx.ewhemi);	//得到经度字符串
			tp=gpsx.latitude;	   
			sprintf((char *)dtbuf,"Latitude:%.5f %1c   ",tp/=100000,gpsx.nshemi);	//得到纬度字符串
			tp=gpsx.altitude;	   
			sprintf((char *)dtbuf,"Altitude:%.1fm     ",tp/=10);	    			//得到高度字符串
			tp=gpsx.speed;	   
			sprintf((char *)dtbuf,"Speed:%.3fkm/h     ",tp/=1000);		    		//得到速度字符串	 
			if(gpsx.fixmode<=3)														//定位状态
			{  
				sprintf((char *)dtbuf,"Fix Mode:%s",fixmode_tbl[gpsx.fixmode]);			   
			}	 	   
			sprintf((char *)dtbuf,"GPS+BD Valid satellite:%02d",gpsx.posslnum);	 		//用于定位的GPS卫星数
			sprintf((char *)dtbuf,"GPS Visible satellite:%02d",gpsx.svnum%100);	 		//可见GPS卫星数
			sprintf((char *)dtbuf,"BD Visible satellite:%02d",gpsx.beidou_svnum%100);	 		//可见北斗卫星数						
		
			

			//*************************************发送到服务器******************************************************************************
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
			postGpsDataToOneNet(API_KEY, device_id, sensor_gps,tp1 , tp2);		//发送数据到Onenet

		}
		
//		Line_1A(13);
//	delay_ms(1500);
  }

}
 

