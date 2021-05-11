#include <reg52.h>	      
#define uchar unsigned char  //�޷����ַ��� �궨��	������Χ0~255
#define uint  unsigned int	 //�޷������� �궨��	������Χ0~65535
#include <intrins.h>
#include "eeprom52.h"
//����ܶ�ѡ����      0     1    2    3    4    5  	6	   7	   8    9	
uchar code smg_du[]={0x28,0xee,0x32,0xa2,0xe4,0xa1,0x21,0xea,0x20,0xa0,
				  	         0x60,0x25,0x39,0x26,0x31,0x71,0xff};	 //����
uchar dis_smg[8]   ={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8};

//�����λѡ����
sbit smg_we1 = P3^4;	    //�����λѡ����
sbit smg_we2 = P3^5;
sbit smg_we3 = P3^6;
sbit smg_we4 = P3^7;

sbit c_send   = P3^2;		//����������  
sbit c_recive = P3^3;		//����������

sbit beep = P2^3;   //������IO�ڶ���
uchar smg_i = 3;    //��ʾ����ܵĸ�λ��
bit flag_300ms ;

long distance;	        //����
uint set_d;	            //����
uchar flag_csb_juli;    //��������������
uint  flag_time0;       //�������涨ʱ��0��ʱ���

uchar menu_1;           //�˵���Ƶı���


/***********************1ms��ʱ����*****************************/
void delay_1ms(uint q)
{
	uint i,j;
	for(i=0;i<q;i++)
		for(j=0;j<120;j++);
}

/***********************������뺯��****************************/
void smg_display()
{
	dis_smg[0] = smg_du[distance % 10];
	dis_smg[1] = smg_du[distance / 10 % 10];
	dis_smg[2] = smg_du[distance / 100 % 10] & 0xdf; ;	
}

/******************�����ݱ��浽��Ƭ���ڲ�eeprom��******************/
void write_eeprom()
{
	SectorErase(0x2000);
	byte_write(0x2000, set_d % 256);
	byte_write(0x2001, set_d / 256);
	byte_write(0x2058, a_a);	
}

/******************�����ݴӵ�Ƭ���ڲ�eeprom�ж�����*****************/
void read_eeprom()
{
	set_d  = byte_read(0x2001);
	set_d <<= 8;
	set_d  |= byte_read(0x2000);
	a_a      = byte_read(0x2058);
}

/**************�����Լ�eeprom��ʼ��*****************/
void init_eeprom()
{
	read_eeprom();		//�ȶ�
	if(a_a != 1)		//�µĵ�Ƭ����ʼ��Ƭ������eeprom
	{
		set_d = 50;
		a_a = 1;
		write_eeprom();	   //��������
	}	
}

/********************������������*****************/
uchar key_can;	 //����ֵ

void key()	 //������������
{
	static uchar key_new;
	key_can = 20;                   //����ֵ��ԭ
	P2 |= 0x07;
	if((P2 & 0x07) != 0x07)		//��������
	{
		delay_1ms(1);	     	//����������
		if(((P2 & 0x07) != 0x07) && (key_new == 1))
		{						//ȷ���ǰ�������
			key_new = 0;
			switch(P2 & 0x07)
			{
				case 0x06: key_can = 3; break;	   //�õ�k2��ֵ
				case 0x05: key_can = 2; break;	   //�õ�k3��ֵ
				case 0x03: key_can = 1; break;	   //�õ�k4��ֵ
			}
		}			
	}
	else 
		key_new = 1;	
}

/****************����������ʾ����***************/
void key_with()
{
	if(key_can == 1)		//���ü�
	{
		menu_1 ++;
		if(menu_1 >= 2)
		{
			menu_1 = 0;
			smg_i = 3;		//ֻ��ʾ3λ����� 
		}
		if(menu_1 == 1)
		{
			smg_i = 4;	   //ֻ��ʾ4λ����� 
		}
	}
	if(menu_1 == 1)			//���ñ���
	{
		if(key_can == 2)
		{
			set_d ++ ;		//��1
			if(set_d > 400)
				set_d = 400;
		}
		if(key_can == 3)
		{
			set_d -- ;		//��1
			if(set_d <= 1)
				set_d = 1;
		}
		dis_smg[0] = smg_du[set_d % 10];	           //ȡС����ʾ
		dis_smg[1] = smg_du[set_d / 10 % 10] ;         //ȡ��λ��ʾ
		dis_smg[2] = smg_du[set_d / 100 % 10] & 0xdf ; //ȡʮλ��ʾ
		dis_smg[3] = 0x60;	        //a
		write_eeprom();			   //��������
	}	
}  

/****************��������***************/
void clock_h_l()
{
	static uchar value;
	if(distance <= set_d)
	{
		value ++;  //����ʵ�ʾ������趨�������ұ仯ʱ�ĸ���
		if(value >= 2)
		{
			beep = 0; 	  //����������	
		}
	}
	else 
	{
		value = 0; 
		beep = 1;		//ȡ������
	}	
}

/***********************����λѡ����*****************************/
void smg_we_switch(uchar i)
{
	switch(i)
	{
		case 0: smg_we1 = 0;  smg_we2 = 1; smg_we3 = 1;  smg_we4 = 1; break;
		case 1: smg_we1 = 1;  smg_we2 = 0; smg_we3 = 1;  smg_we4 = 1; break;
		case 2: smg_we1 = 1;  smg_we2 = 1; smg_we3 = 0;  smg_we4 = 1; break;
		case 3: smg_we1 = 1;  smg_we2 = 1; smg_we3 = 1;  smg_we4 = 0; break;
	}	
}

/***********************������ʾ����*****************************/
void display()
{
	static uchar i;   
	i++;
	if(i >= smg_i)
		i = 0;	
	smg_we_switch(i);		 //λѡ
	P1 = dis_smg[i];		 //��ѡ	        
}

/******************С��ʱ����*****************/
void delay()
{
	_nop_(); 		           //ִ��һ��_nop_()ָ�����1us
	_nop_(); 
	_nop_(); 
	_nop_(); 
	_nop_(); 
//	_nop_(); 
//	_nop_(); 
//	_nop_(); 
//	_nop_();  
//	_nop_(); 
}


/*********************������������*****************************/
void send_wave()
{
	c_send = 1;		           //10us�ĸߵ�ƽ���� 
	delay();
	c_send = 0;	 
	TH0 = 0;		          //����ʱ��0����
	TL0 = 0;
	TR0 = 0;				  //�ض�ʱ��0��ʱ
	while(!c_recive);		  //��c_reciveΪ��ʱ�ȴ�
	TR0=1;
	while(c_recive)		      //��c_reciveΪ1�������ȴ�
	{
		flag_time0 = TH0 * 256 + TL0;
		if((flag_time0 > 40000))      //������������������Χʱ����ʾ3��888
		{
			TR0 = 0;
			flag_csb_juli = 2;
			distance = 888;
			break ;		
		}
		else 
		{
			flag_csb_juli = 1;	
		}
	}
	if(flag_csb_juli == 1)
	{	
		TR0=0;							 //�ض�ʱ��0��ʱ
		distance =flag_time0;			 //������ʱ��0��ʱ��
		distance *= 0.017;               // 0.017 = 340M / 2 = 170M = 0.017M ���������
		if((distance > 500))				 //���� = �ٶ� * ʱ��
		{	
			distance = 888;				 //�������3.8m�ͳ��������������� 
		}
	}  
}


/*********************��ʱ��0����ʱ��1��ʼ��******************/
void time_init()	  
{
	EA  = 1;	 	  //�����ж�
	TMOD = 0X11;	//��ʱ��0����ʱ��1������ʽ1
	ET0 = 0;		  //�ض�ʱ��0�ж� 
	TR0 = 1;		  //����ʱ��0��ʱ
	ET1 = 1;		  //����ʱ��1�ж� 
	TR1 = 1;		  //����ʱ��1��ʱ	
}



/***************������*****************/
void main()
{
	beep = 1;		 //0������һ��   
	delay_1ms(150);//�е�ʱ��
	P0 = P1 = P2 = P3 = 0xff;	   //��ʼ����Ƭ��IO��Ϊ�ߵ�ƽ
	send_wave();	//����뺯��
	smg_display();	//���������ʾ����
	time_init();	//��ʱ����ʼ������
	init_eeprom();  //��ʼ��ʼ�����������
	send_wave();	//����뺯��
	send_wave();	//����뺯��
	while(1)
	{		  
		if(flag_300ms == 1)
		{		
			flag_300ms = 0;
			clock_h_l();    //��������
			if(beep == 1)
				send_wave();	//����뺯��
			if(menu_1 == 0)
				smg_display();	 //���������ʾ����
		}
		key();					 //��������
		if(key_can < 10)
		{
			key_with();			 //����������
		}
	}
}

/*********************��ʱ��1�жϷ������************************/
void time1_int() interrupt 3
{	
	static uchar value;			 //��ʱ2ms�ж�һ��
	TH1 = 0xf8;
	TL1 = 0x30;     //2ms
	display();		//�������ʾ����
	value++;
	if(value >= 150)
	{
		value = 0;
		flag_300ms = 1;
	}
}




