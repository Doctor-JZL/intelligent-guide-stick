#include "gps.h" 
#include "delay.h" 								   
#include "usart3.h" 								   
#include "stdio.h"	 
#include "stdarg.h"	 
#include "string.h"	 
#include "math.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F103������
//ATK-S1216F8 GPSģ����������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2015/04/11
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved							  
////////////////////////////////////////////////////////////////////////////////// 	   


const u32 BAUD_id[9]={4800,9600,19200,38400,57600,115200,230400,460800,921600};//ģ��֧�ֲ���������
//��buf����õ���cx���������ڵ�λ��
//����ֵ:0~0XFE,����������λ�õ�ƫ��.
//       0XFF,�������ڵ�cx������							  
u8 NMEA_Comma_Pos(u8 *buf,u8 cx)
{	 		    
	u8 *p=buf;
	while(cx)
	{		 
		if(*buf=='*'||*buf<' '||*buf>'z')return 0XFF;//����'*'���߷Ƿ��ַ�,�򲻴��ڵ�cx������
		if(*buf==',')cx--;
		buf++;
	}
	return buf-p;	 
}
//m^n����
//����ֵ:m^n�η�.
u32 NMEA_Pow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;    
	return result;
}
//strת��Ϊ����,��','����'*'����
//buf:���ִ洢��
//dx:С����λ��,���ظ����ú���
//����ֵ:ת�������ֵ
int NMEA_Str2num(u8 *buf,u8*dx)
{
	u8 *p=buf;
	u32 ires=0,fres=0;
	u8 ilen=0,flen=0,i;
	u8 mask=0;
	int res;
	while(1) //�õ�������С���ĳ���
	{
		if(*p=='-'){mask|=0X02;p++;}//�Ǹ���
		if(*p==','||(*p=='*'))break;//����������
		if(*p=='.'){mask|=0X01;p++;}//����С������
		else if(*p>'9'||(*p<'0'))	//�зǷ��ַ�
		{	
			ilen=0;
			flen=0;
			break;
		}	
		if(mask&0X01)flen++;
		else ilen++;
		p++;
	}
	if(mask&0X02)buf++;	//ȥ������
	for(i=0;i<ilen;i++)	//�õ�������������
	{  
		ires+=NMEA_Pow(10,ilen-1-i)*(buf[i]-'0');
	}
	if(flen>5)flen=5;	//���ȡ5λС��
	*dx=flen;	 		//С����λ��
	for(i=0;i<flen;i++)	//�õ�С����������
	{  
		fres+=NMEA_Pow(10,flen-1-i)*(buf[ilen+1+i]-'0');
	} 
	res=ires*NMEA_Pow(10,flen)+fres;
	if(mask&0X02)res=-res;		   
	return res;
}	  							 
//����GPGSV��Ϣ
//gpsx:nmea��Ϣ�ṹ��
//buf:���յ���GPS���ݻ������׵�ַ
void NMEA_GPGSV_Analysis(nmea_msg *gpsx,u8 *buf)
{
	u8 *p,*p1,dx;
	u8 len,i,j,slx=0;
	u8 posx;   	 
	p=buf;
	p1=(u8*)strstr((const char *)p,"$GPGSV");
	len=p1[7]-'0';								//�õ�GPGSV������
	posx=NMEA_Comma_Pos(p1,3); 					//�õ��ɼ���������
	if(posx!=0XFF)gpsx->svnum=NMEA_Str2num(p1+posx,&dx);
	for(i=0;i<len;i++)
	{	 
		p1=(u8*)strstr((const char *)p,"$GPGSV");  
		for(j=0;j<4;j++)
		{	  
			posx=NMEA_Comma_Pos(p1,4+j*4);
			if(posx!=0XFF)gpsx->slmsg[slx].num=NMEA_Str2num(p1+posx,&dx);	//�õ����Ǳ��
			else break; 
			posx=NMEA_Comma_Pos(p1,5+j*4);
			if(posx!=0XFF)gpsx->slmsg[slx].eledeg=NMEA_Str2num(p1+posx,&dx);//�õ��������� 
			else break;
			posx=NMEA_Comma_Pos(p1,6+j*4);
			if(posx!=0XFF)gpsx->slmsg[slx].azideg=NMEA_Str2num(p1+posx,&dx);//�õ����Ƿ�λ��
			else break; 
			posx=NMEA_Comma_Pos(p1,7+j*4);
			if(posx!=0XFF)gpsx->slmsg[slx].sn=NMEA_Str2num(p1+posx,&dx);	//�õ����������
			else break;
			slx++;	   
		}   
 		p=p1+1;//�л�����һ��GPGSV��Ϣ
	}   
}
//����BDGSV��Ϣ
//gpsx:nmea��Ϣ�ṹ��
//buf:���յ���GPS���ݻ������׵�ַ
void NMEA_BDGSV_Analysis(nmea_msg *gpsx,u8 *buf)
{
	u8 *p,*p1,dx;
	u8 len,i,j,slx=0;
	u8 posx;   	 
	p=buf;
	p1=(u8*)strstr((const char *)p,"$BDGSV");
	len=p1[7]-'0';								//�õ�BDGSV������
	posx=NMEA_Comma_Pos(p1,3); 					//�õ��ɼ�������������
	if(posx!=0XFF)gpsx->beidou_svnum=NMEA_Str2num(p1+posx,&dx);
	for(i=0;i<len;i++)
	{	 
		p1=(u8*)strstr((const char *)p,"$BDGSV");  
		for(j=0;j<4;j++)
		{	  
			posx=NMEA_Comma_Pos(p1,4+j*4);
			if(posx!=0XFF)gpsx->beidou_slmsg[slx].beidou_num=NMEA_Str2num(p1+posx,&dx);	//�õ����Ǳ��
			else break; 
			posx=NMEA_Comma_Pos(p1,5+j*4);
			if(posx!=0XFF)gpsx->beidou_slmsg[slx].beidou_eledeg=NMEA_Str2num(p1+posx,&dx);//�õ��������� 
			else break;
			posx=NMEA_Comma_Pos(p1,6+j*4);
			if(posx!=0XFF)gpsx->beidou_slmsg[slx].beidou_azideg=NMEA_Str2num(p1+posx,&dx);//�õ����Ƿ�λ��
			else break; 
			posx=NMEA_Comma_Pos(p1,7+j*4);
			if(posx!=0XFF)gpsx->beidou_slmsg[slx].beidou_sn=NMEA_Str2num(p1+posx,&dx);	//�õ����������
			else break;
			slx++;	   
		}   
 		p=p1+1;//�л�����һ��BDGSV��Ϣ
	}   
}
//����GNGGA��Ϣ
//gpsx:nmea��Ϣ�ṹ��
//buf:���յ���GPS���ݻ������׵�ַ
void NMEA_GNGGA_Analysis(nmea_msg *gpsx,u8 *buf)
{
	u8 *p1,dx;			 
	u8 posx;    
	p1=(u8*)strstr((const char *)buf,"$GNGGA");
	posx=NMEA_Comma_Pos(p1,6);								//�õ�GPS״̬
	if(posx!=0XFF)gpsx->gpssta=NMEA_Str2num(p1+posx,&dx);	
	posx=NMEA_Comma_Pos(p1,7);								//�õ����ڶ�λ��������
	if(posx!=0XFF)gpsx->posslnum=NMEA_Str2num(p1+posx,&dx); 
	posx=NMEA_Comma_Pos(p1,9);								//�õ����θ߶�
	if(posx!=0XFF)gpsx->altitude=NMEA_Str2num(p1+posx,&dx);  
}
//����GNGSA��Ϣ
//gpsx:nmea��Ϣ�ṹ��
//buf:���յ���GPS���ݻ������׵�ַ
void NMEA_GNGSA_Analysis(nmea_msg *gpsx,u8 *buf)
{
	u8 *p1,dx;			 
	u8 posx; 
	u8 i;   
	p1=(u8*)strstr((const char *)buf,"$GNGSA");
	posx=NMEA_Comma_Pos(p1,2);								//�õ���λ����
	if(posx!=0XFF)gpsx->fixmode=NMEA_Str2num(p1+posx,&dx);	
	for(i=0;i<12;i++)										//�õ���λ���Ǳ��
	{
		posx=NMEA_Comma_Pos(p1,3+i);					 
		if(posx!=0XFF)gpsx->possl[i]=NMEA_Str2num(p1+posx,&dx);
		else break; 
	}				  
	posx=NMEA_Comma_Pos(p1,15);								//�õ�PDOPλ�þ�������
	if(posx!=0XFF)gpsx->pdop=NMEA_Str2num(p1+posx,&dx);  
	posx=NMEA_Comma_Pos(p1,16);								//�õ�HDOPλ�þ�������
	if(posx!=0XFF)gpsx->hdop=NMEA_Str2num(p1+posx,&dx);  
	posx=NMEA_Comma_Pos(p1,17);								//�õ�VDOPλ�þ�������
	if(posx!=0XFF)gpsx->vdop=NMEA_Str2num(p1+posx,&dx);  
}
//����GNRMC��Ϣ
//gpsx:nmea��Ϣ�ṹ��
//buf:���յ���GPS���ݻ������׵�ַ
void NMEA_GNRMC_Analysis(nmea_msg *gpsx,u8 *buf)
{
	u8 *p1,dx;			 
	u8 posx;     
	u32 temp;	   
	float rs;  
	p1=(u8*)strstr((const char *)buf,"$GNRMC");//"$GNRMC",������&��GNRMC�ֿ������,��ֻ�ж�GPRMC.
	posx=NMEA_Comma_Pos(p1,1);								//�õ�UTCʱ��
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx)/NMEA_Pow(10,dx);	 	//�õ�UTCʱ��,ȥ��ms
		gpsx->utc.hour=temp/10000;
		gpsx->utc.min=(temp/100)%100;
		gpsx->utc.sec=temp%100;	 	 
	}	
	posx=NMEA_Comma_Pos(p1,3);								//�õ�γ��
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx);		 	 
		gpsx->latitude=temp/NMEA_Pow(10,dx+2);	//�õ���
		rs=temp%NMEA_Pow(10,dx+2);				//�õ�'		 
		gpsx->latitude=gpsx->latitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60;//ת��Ϊ�� 
	}
	posx=NMEA_Comma_Pos(p1,4);								//��γ���Ǳ�γ 
	if(posx!=0XFF)gpsx->nshemi=*(p1+posx);					 
 	posx=NMEA_Comma_Pos(p1,5);								//�õ�����
	if(posx!=0XFF)
	{												  
		temp=NMEA_Str2num(p1+posx,&dx);		 	 
		gpsx->longitude=temp/NMEA_Pow(10,dx+2);	//�õ���
		rs=temp%NMEA_Pow(10,dx+2);				//�õ�'		 
		gpsx->longitude=gpsx->longitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60;//ת��Ϊ�� 
	}
	posx=NMEA_Comma_Pos(p1,6);								//������������
	if(posx!=0XFF)gpsx->ewhemi=*(p1+posx);		 
	posx=NMEA_Comma_Pos(p1,9);								//�õ�UTC����
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx);		 				//�õ�UTC����
		gpsx->utc.date=temp/10000;
		gpsx->utc.month=(temp/100)%100;
		gpsx->utc.year=2000+temp%100;	 	 
	} 
}
//����GNVTG��Ϣ
//gpsx:nmea��Ϣ�ṹ��
//buf:���յ���GPS���ݻ������׵�ַ
void NMEA_GNVTG_Analysis(nmea_msg *gpsx,u8 *buf)
{
	u8 *p1,dx;			 
	u8 posx;    
	p1=(u8*)strstr((const char *)buf,"$GNVTG");							 
	posx=NMEA_Comma_Pos(p1,7);								//�õ���������
	if(posx!=0XFF)
	{
		gpsx->speed=NMEA_Str2num(p1+posx,&dx);
		if(dx<3)gpsx->speed*=NMEA_Pow(10,3-dx);	 	 		//ȷ������1000��
	}
}  
//��ȡNMEA-0183��Ϣ
//gpsx:nmea��Ϣ�ṹ��
//buf:���յ���GPS���ݻ������׵�ַ
void GPS_Analysis(nmea_msg *gpsx,u8 *buf)
{
	NMEA_GPGSV_Analysis(gpsx,buf);	//GPGSV����
	NMEA_BDGSV_Analysis(gpsx,buf);	//BDGSV����
	NMEA_GNGGA_Analysis(gpsx,buf);	//GNGGA���� 	
	NMEA_GNGSA_Analysis(gpsx,buf);	//GPNSA����
	NMEA_GNRMC_Analysis(gpsx,buf);	//GPNMC����
	NMEA_GNVTG_Analysis(gpsx,buf);	//GPNTG����
}
///////////////////////////////////////////UBLOX ���ô���/////////////////////////////////////
////���CFG����ִ�����
////����ֵ:0,ACK�ɹ�
////       1,���ճ�ʱ����
////       2,û���ҵ�ͬ���ַ�
////       3,���յ�NACKӦ��
u8 SkyTra_Cfg_Ack_Check(void)
{			 
	u16 len=0,i;
	u8 rval=0;
	while((USART3_RX_STA&0X8000)==0 && len<100)//�ȴ����յ�Ӧ��   
	{
		len++;
		delay_ms(5);
	}		 
	if(len<100)   	//��ʱ����.
	{
		len=USART3_RX_STA&0X7FFF;	//�˴ν��յ������ݳ��� 
		for(i=0;i<len;i++)
		{
			if(USART3_RX_BUF[i]==0X83)break;
			else if(USART3_RX_BUF[i]==0X84)
			{
				rval=3;
				break;
			}
		}
		if(i==len)rval=2;						//û���ҵ�ͬ���ַ�
	}else rval=1;								//���ճ�ʱ����
    USART3_RX_STA=0;							//�������
	return rval;  
}
//����SkyTra_GPS/����ģ�鲨����
//baud_id:0~8����Ӧ������,4800/9600/19200/38400/57600/115200/230400/460800/921600	  
//����ֵ:0,ִ�гɹ�;����,ִ��ʧ��(���ﲻ�᷵��0��)
u8 SkyTra_Cfg_Prt(u32 baud_id)
{
	SkyTra_baudrate *cfg_prt=(SkyTra_baudrate *)USART3_TX_BUF;
	cfg_prt->sos=0XA1A0;		//��������(С��ģʽ)
	cfg_prt->PL=0X0400;			//��Ч���ݳ���(С��ģʽ)
	cfg_prt->id=0X05;		    //���ò����ʵ�ID 
	cfg_prt->com_port=0X00;			//��������1
	cfg_prt->Baud_id=baud_id;	 	////�����ʶ�Ӧ���
	cfg_prt->Attributes=1; 		  //���浽SRAM&FLASH
	cfg_prt->CS=cfg_prt->id^cfg_prt->com_port^cfg_prt->Baud_id^cfg_prt->Attributes;
	cfg_prt->end=0X0A0D;        //���ͽ�����(С��ģʽ)
	SkyTra_Send_Date((u8*)cfg_prt,sizeof(SkyTra_baudrate));//�������ݸ�SkyTra   
	delay_ms(200);				//�ȴ�������� 
	USART3_Init(BAUD_id[baud_id]);	//���³�ʼ������3  
	return SkyTra_Cfg_Ack_Check();//���ﲻ�ᷴ��0,��ΪUBLOX��������Ӧ���ڴ������³�ʼ����ʱ���Ѿ���������.
} 
//����SkyTra_GPSģ���ʱ��������
//width:������1~100000(us)
//����ֵ:0,���ͳɹ�;����,����ʧ��.
u8 SkyTra_Cfg_Tp(u32 width)
{
	u32 temp=width;
	SkyTra_pps_width *cfg_tp=(SkyTra_pps_width *)USART3_TX_BUF;
	temp=(width>>24)|((width>>8)&0X0000FF00)|((width<<8)&0X00FF0000)|((width<<24)&0XFF000000);//С��ģʽ
	cfg_tp->sos=0XA1A0;		    //cfg header(С��ģʽ)
	cfg_tp->PL=0X0700;        //��Ч���ݳ���(С��ģʽ)
	cfg_tp->id=0X65	;			    //cfg tp id
	cfg_tp->Sub_ID=0X01;			//����������Ϊ20���ֽ�.
	cfg_tp->width=temp;		  //������,us
	cfg_tp->Attributes=0X01;  //���浽SRAM&FLASH	
	cfg_tp->CS=cfg_tp->id^cfg_tp->Sub_ID^(cfg_tp->width>>24)^(cfg_tp->width>>16)&0XFF^(cfg_tp->width>>8)&0XFF^cfg_tp->width&0XFF^cfg_tp->Attributes;    	//�û���ʱΪ0ns
	cfg_tp->end=0X0A0D;       //���ͽ�����(С��ģʽ)
	SkyTra_Send_Date((u8*)cfg_tp,sizeof(SkyTra_pps_width));//�������ݸ�NEO-6M  
	return SkyTra_Cfg_Ack_Check();
}
//����SkyTraF8-BD�ĸ�������	 	    
//Frep:��ȡֵ��Χ:1,2,4,5,8,10,20,25,40,50������ʱ��������λΪHz������ܴ���50Hz
//����ֵ:0,���ͳɹ�;����,����ʧ��.
u8 SkyTra_Cfg_Rate(u8 Frep)
{
	SkyTra_PosRate *cfg_rate=(SkyTra_PosRate *)USART3_TX_BUF;
 	cfg_rate->sos=0XA1A0;	    //cfg header(С��ģʽ)
	cfg_rate->PL=0X0300;			//��Ч���ݳ���(С��ģʽ)
	cfg_rate->id=0X0E;	      //cfg rate id
	cfg_rate->rate=Frep;	 	  //��������
	cfg_rate->Attributes=0X01;	   	//���浽SRAM&FLASH	.
	cfg_rate->CS=cfg_rate->id^cfg_rate->rate^cfg_rate->Attributes;//������,us
	cfg_rate->end=0X0A0D;       //���ͽ�����(С��ģʽ)
	SkyTra_Send_Date((u8*)cfg_rate,sizeof(SkyTra_PosRate));//�������ݸ�NEO-6M 
	return SkyTra_Cfg_Ack_Check();
}

//����һ�����ݸ�SkyTraF8-BD������ͨ������3����
//dbuf�����ݻ����׵�ַ
//len��Ҫ���͵��ֽ���
void SkyTra_Send_Date(u8* dbuf,u16 len)
{
	u16 j;
	for(j=0;j<len;j++)//ѭ����������
	{
		while((USART3->SR&0X40)==0);//ѭ������,ֱ���������   
		USART3->DR=dbuf[j];  
	}	
}







