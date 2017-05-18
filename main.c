#if defined(__dsPIC33F__)
#include <p33Fxxxx.h>
#elif defined(__PIC24H__)
#include <p24hxxxx.h>
#endif
#include "timer.h"
#include "adc.h"
#include "sci.h"
#include "IC.h"
#include "DS1302.h"
#include "FRAM.h"
#include "ecan.h"

_FBS(0xCF);
_FSS(0xCF);
_FGS(0x07);
_FOSCSEL(0xA2);  //Select HS without PLL
_FOSC(0x46);  // HS oscilator, OSC2 is clock output
_FWDT(0xDF);
_FPOR(0xE7);
//_FICD(0xC3);
#define CLRWDT {__asm__ volatile ("CLRWDT");}

#define CEA LATGbits.LATG0
#define CEB LATGbits.LATG1
#define CEC LATGbits.LATG2
#define CED LATGbits.LATG3
#define CEE LATDbits.LATD7
#define CEF LATDbits.LATD2
#define CEG LATDbits.LATD1
#define CEH LATDbits.LATD3

#define   ZAN LATGbits.LATG13


#define POWER LATCbits.LATC13
#define POWER1 LATCbits.LATC14
#define POWER2 LATDbits.LATD0
//#define WORK_LED LATDbits.LATD1
//#define BOARD_ID 1  // Board ID




unsigned char BOARD_ID=0x01;// Board ID




unsigned int freq[8],lq; 
//unsigned char BOARD_ID=0x03 // Board ID
unsigned int temp_freq[8];	
float freq_float;			  
unsigned int temp[8]; // Temperature
unsigned int bat; 
unsigned int dummy;
short bat_high, bat_mid, bat_low; // �����������ڶԵ�ص�ѹ�����˲�
short halt; // 1-ֹͣ������0-��ʼ����
//unsigned int TxBuffer[16] __attribute__((space(dma)));

int CE_Enable; // 1->����Ƶ  0->��ֹ��Ƶ 
int Read_Enable; // 1->ʰƵ��ɣ����Զ�ȡ  0->��ֹ��ȡ
int Tick; // ������¼��ʱ�жϴ���
unsigned int halt_Tick; // ������¼ʱ�䣬��ʱ��Ϊ���η���֮���ʱ����
unsigned int halt_Timeout; // ���η���֮��ļ������λΪms����ͨ����λ������
unsigned char Send_Enable; // ����ָʾ�Ƿ���Է������ݣ��ϵ翪ʼʱ���������ݣ��յ�ZigBeeоƬ���͵�'n'�ַ����������
unsigned char Save_Enable; // ������ʾ�Ƿ���FRAM�д�����
unsigned char Tran_Enable; // ������ʾ�Ƿ�FRAM�е����ݶ���
unsigned char halt_Enable; // ����
unsigned char Read_Timer_En;
//unsigned char ID = 1; // ���******************************************************


unsigned int filter_Tick; // �˲�����
unsigned char filter_Enable; // �Ƿ����˲������У�1-�ǣ�0-��
unsigned int filter_freq[8]; // �˲����4������ֵ
//unsigned int filter_freq[8]; // �˲����4������ֵ
unsigned int filter[8][4]; // ����ƽ���˲����� 

unsigned char stat1;

unsigned int end_addr[2]; // �洢������ַ���洢��FRAM 0��1��2��ַ��
unsigned char temp_addr[3];
unsigned int curr_addr[2];
unsigned int read_count; //������¼��FRAM�ֶ�ȡ���ַ�������������������ͷ���������������

unsigned char state;

mID canTxMessage;
mID canRxMessage;
ECAN1MSGBUF ecan1msgBuf __attribute__((space(dma),aligned(ECAN1_MSG_BUF_LENGTH*16)));


void inc_addr(void); // ��curr_addr��1
void Tran_Data(void); // ����λ����������



void DELAY(unsigned int t)
{
	unsigned int i,j;
	
  	for(i=0;i<50;i++)
  	{
	  	for(j=0;j<t;j++)
   		{
	   		asm("nop");
	   		
    	}
   } 	
 }

void __attribute__((interrupt,no_auto_psv)) _T6Interrupt(void)  // 1ms interrupt
{
	IFS2bits.T6IF = 0;
//	if(Read_Timer_En)
	{
			Tick++;
	//***********		if(Tick>59) // 60ms
			if(Tick>19)
			{
				Tick = 0;
				Read_Enable = 1;
				Read_Timer_En = 0;	
			}
	}	
		
}


void __attribute__((interrupt,no_auto_psv)) _U2RXInterrupt(void)
{
	
	IFS1bits.U2RXIF = 0;
	return;	
}


void __attribute__((interrupt,no_auto_psv))_C1Interrupt(void)  
{
	/* check to see if the interrupt is caused by receive */     	 
    if(C1INTFbits.RBIF)
    {
	    /* check to see if buffer 1 is full */
	    if(C1RXFUL1bits.RXFUL1)
	    {			
			/* set the buffer full flag and the buffer received flag */
			canRxMessage.buffer_status=CAN_BUF_FULL;
			canRxMessage.buffer=1;	
		}		
		/* check to see if buffer 2 is full */
		else if(C1RXFUL1bits.RXFUL2)
		{
			/* set the buffer full flag and the buffer received flag */
			canRxMessage.buffer_status=CAN_BUF_FULL;
			canRxMessage.buffer=2;					
		}
		else;
		/* clear flag */
		C1INTFbits.RBIF = 0;
	}
	else if(C1INTFbits.TBIF)
    {
	    /* clear flag */
		C1INTFbits.TBIF = 0;	    
	}
	else;
	
	/* clear interrupt flag */
	IFS2bits.C1IF=0;
}

int main()
{
	int i = 1000; // ɨƵƵ�� 473Hz~7.042kHz
	int k,j;

   	TRISCbits.TRISC13 = 0;

   	TRISCbits.TRISC14 = 0;

   	TRISDbits.TRISD0 = 0;

CLRWDT
    
    canRxMessage.id=0;
	read_count = 0;
	//CE_Enable = 1;
	CE_Enable = 0; 
	Read_Enable = 0;
	Send_Enable = 0;
	Save_Enable = 0;
	Tran_Enable = 0;
	halt_Enable = 0;
	Read_Timer_En = 0;
	Tick = 0;
	bat_high = 0; 
	bat_mid = 0;
	bat_low = 0;
	halt = 0;
	halt_Timeout = 600; //**************************************************
	filter_Tick = 0;
	filter_Enable = 1; // ��ʼ������ʼ�˲�
	filter_freq[0] = 0; filter_freq[1] = 0; filter_freq[2] = 0; filter_freq[3] = 0; filter_freq[4] = 0; filter_freq[5] = 0;
	OSCCON = 0x2200;
	
	TRISGbits.TRISG1 = 0;
	TRISGbits.TRISG0 = 0;
	TRISGbits.TRISG2 = 0;
	TRISGbits.TRISG3 = 0;
	TRISDbits.TRISD7 = 0;
	TRISDbits.TRISD2 = 0;
	TRISDbits.TRISD1 = 0;
	TRISDbits.TRISD3 = 0;
	
	TRISFbits.TRISF1 = 0;
	TRISFbits.TRISF0 = 1; 
	
	TRISGbits.TRISG15 = 0;
	
	TRISDbits.TRISD8 = 1;
	TRISDbits.TRISD9 = 1;
	TRISDbits.TRISD10 = 1;
	TRISDbits.TRISD11 = 1;
	TRISDbits.TRISD4 = 1;
	TRISDbits.TRISD5 = 1;
	TRISBbits.TRISB5 = 1;
	TRISBbits.TRISB4 = 1;
     POWER1=0;
 POWER=0;
 POWER2=0;
  


/**
deng
**/


    TRISGbits.TRISG13 = 0;

    ZAN=1;	

	TRISGbits.TRISG14 = 0;
	AD1PCFGLbits.PCFG5 = 1;
	AD1PCFGLbits.PCFG4 = 1;
	AD2PCFGLbits.PCFG5 = 1;
	AD2PCFGLbits.PCFG4 = 1;

	InitTimer6();  // Timer6 �ṩ1ms�ж϶�ʱ
	InitTimer2();  // Timer2 �ṩ���벶׽ʱ�ӻ�׼
	InitIC();
	InitADC();
	InitSCI();
	InitSPI();
	ds1302_init();
	InitSCI();
	initECAN();
	initDMAECAN();
	IEC2bits.C1IE=1;
	C1INTEbits.RBIE=1;
 
  CLRWDT
  ZAN=0;

	while(1)
	{ 	  

        CLRWDT	
		if(canRxMessage.buffer_status==CAN_BUF_FULL)
		{
			rxECAN(&canRxMessage);
			if(canRxMessage.id==0X01)
				CE_Enable = 1;
		//	else if(canRxMessage.id == 0x032)
        	else if(canRxMessage.id ==((BOARD_ID<<4)+2))
				Send_Enable = 1;
			canRxMessage.buffer_status=CAN_BUF_EMPTY;
		}	
		

		if(CE_Enable) // ��ʼ��Ƶ
		{
            POWER1=1;
            POWER=1;
            POWER2=1;

            DELAY(2000);  // 600~9ms

			i = 1470;
			while(i>=440)
			{
				for(k=0;k<1;k++)
				{
            		j = i;
            		CEB = 1; //RG1���Ϊ��
            		CEA = 1;
            		CEC = 1;
            		CED = 1;
            		CEE = 0;
            		CEF = 0;
            		CEG = 0;
            		CEH = 0;
            		while(j)
               		{    
	               		asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
	               		asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
						j--;
                	}             
 
            		j = i;              
            		CEB = 0; //RG1���Ϊ��  
            		CEA = 0;
            		CEC = 0;
            		CED = 0;
            		CEE = 1;
            		CEF = 1;
            		CEG = 1;
            		CEH = 1;
            		while(j)
                	{  
	                	asm("nop");	asm("nop");asm("nop");asm("nop");asm("nop");
	                	asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
                    	j--;
                	}          
            	} 
				i = i-1;	 
			}
                	CEE = 0;
            		CEF = 0;
            		CEG = 0;
            		CEH = 0;
			DELAY(800);  // 600~9ms
			CE_Enable = 0;
			StartTimer6();
			StartTimer2();
			StartIC();
			while(!Read_Enable)	{asm("nop");}			
			Read_Enable = 0;
			freq[0] = GetPeriod(1);
			freq[1] = GetPeriod(2);
			freq[2] = GetPeriod(3);
			freq[3] = GetPeriod(4);
			freq[4] = GetPeriod(5);
			freq[5] = GetPeriod(6);
			freq[6] = GetPeriod(8);
			freq[7] = GetPeriod(7);
			StopIC();
			StopTimer6();
            StopTimer2();	
	        /*��6��ͨ���Ĵ������������践�صĵ�ѹ������ADת������������temp[]�� */
			/***YEMP1
              1	AN16		2	AN13	3 AN3		4	AN1
              5	AN9	    	6	AN0	    7 AN10		8   AN11
           

            *****/



	
           CLRWDT
				AD1CHS0bits.CH0SA = 16; //AN9
				AD1CON1bits.ADON = 1; // Turn on the A/D converter
				asm("nop");asm("nop");asm("nop");asm("nop");
				AD1CON1bits.SAMP = 1; 		//ADC����ʹ��λ
											//1: ADC����/���ַŴ������ڲ���
											//0: ADC ����/���ַŴ������ֲ������
				while(!AD1CON1bits.DONE){}; //ADC ת��״̬λ
											//1 = ADC ת�����
											//0 = ADC ת����δ��ʼ���ڽ�����			
				temp[0] = ADC1BUF0;	//ÿ��A/D ת���Ľ���洢��ADCxBUF0�Ĵ�����
									//ADC ģ�����һ������ֻ��˫�˿ڼĴ�����ADCxBUF0��
				AD1CON1bits.ADON = 0;//ADC ����ģʽλ
									//1 = ADC ģ�����ڹ���
									//0 = ADC �ر�	



	
				AD1CHS0bits.CH0SA = 13; //AN9
				AD1CON1bits.ADON = 1; // Turn on the A/D converter
				asm("nop");asm("nop");asm("nop");asm("nop");
				AD1CON1bits.SAMP = 1; 		//ADC����ʹ��λ
											//1: ADC����/���ַŴ������ڲ���
											//0: ADC ����/���ַŴ������ֲ������
				while(!AD1CON1bits.DONE){}; //ADC ת��״̬λ
											//1 = ADC ת�����
											//0 = ADC ת����δ��ʼ���ڽ�����			
				temp[1] = ADC1BUF0;	//ÿ��A/D ת���Ľ���洢��ADCxBUF0�Ĵ�����
									//ADC ģ�����һ������ֻ��˫�˿ڼĴ�����ADCxBUF0��
				AD1CON1bits.ADON = 0;//ADC ����ģʽλ
									//1 = ADC ģ�����ڹ���
									//0 = ADC �ر�		
				
      


				AD1CHS0bits.CH0SA = 3; //AN9
				AD1CON1bits.ADON = 1; // Turn on the A/D converter
				asm("nop");asm("nop");asm("nop");asm("nop");
				AD1CON1bits.SAMP = 1; 		//ADC����ʹ��λ
											//1: ADC����/���ַŴ������ڲ���
											//0: ADC ����/���ַŴ������ֲ������
				while(!AD1CON1bits.DONE){}; //ADC ת��״̬λ
											//1 = ADC ת�����
											//0 = ADC ת����δ��ʼ���ڽ�����			
				temp[2] = ADC1BUF0;	//ÿ��A/D ת���Ľ���洢��ADCxBUF0�Ĵ�����
									//ADC ģ�����һ������ֻ��˫�˿ڼĴ�����ADCxBUF0��
				AD1CON1bits.ADON = 0;//ADC ����ģʽλ
									//1 = ADC ģ�����ڹ���
									//0 = ADC �ر�		
		


				AD1CHS0bits.CH0SA = 1; //AN9
				AD1CON1bits.ADON = 1; // Turn on the A/D converter
				asm("nop");asm("nop");asm("nop");asm("nop");
				AD1CON1bits.SAMP = 1; 		//ADC����ʹ��λ
											//1: ADC����/���ַŴ������ڲ���
											//0: ADC ����/���ַŴ������ֲ������
				while(!AD1CON1bits.DONE){}; //ADC ת��״̬λ
											//1 = ADC ת�����
											//0 = ADC ת����δ��ʼ���ڽ�����			
				temp[3] = ADC1BUF0;	//ÿ��A/D ת���Ľ���洢��ADCxBUF0�Ĵ�����
									//ADC ģ�����һ������ֻ��˫�˿ڼĴ�����ADCxBUF0��
				AD1CON1bits.ADON = 0;//ADC ����ģʽλ
									//1 = ADC ģ�����ڹ���
									//0 = ADC �ر�		
		



				AD1CHS0bits.CH0SA = 9; //AN9
				AD1CON1bits.ADON = 1; // Turn on the A/D converter
				asm("nop");asm("nop");asm("nop");asm("nop");
				AD1CON1bits.SAMP = 1; 		//ADC����ʹ��λ
											//1: ADC����/���ַŴ������ڲ���
											//0: ADC ����/���ַŴ������ֲ������
				while(!AD1CON1bits.DONE){}; //ADC ת��״̬λ
											//1 = ADC ת�����
											//0 = ADC ת����δ��ʼ���ڽ�����			
				temp[4] = ADC1BUF0;	//ÿ��A/D ת���Ľ���洢��ADCxBUF0�Ĵ�����
									//ADC ģ�����һ������ֻ��˫�˿ڼĴ�����ADCxBUF0��
				AD1CON1bits.ADON = 0;//ADC ����ģʽλ
									//1 = ADC ģ�����ڹ���
									//0 = ADC �ر�		
		


				AD1CHS0bits.CH0SA = 0; //AN9
				AD1CON1bits.ADON = 1; // Turn on the A/D converter
				asm("nop");asm("nop");asm("nop");asm("nop");
				AD1CON1bits.SAMP = 1; 		//ADC����ʹ��λ
											//1: ADC����/���ַŴ������ڲ���
											//0: ADC ����/���ַŴ������ֲ������
				while(!AD1CON1bits.DONE){}; //ADC ת��״̬λ
											//1 = ADC ת�����
											//0 = ADC ת����δ��ʼ���ڽ�����			
				temp[5] = ADC1BUF0;	//ÿ��A/D ת���Ľ���洢��ADCxBUF0�Ĵ�����
									//ADC ģ�����һ������ֻ��˫�˿ڼĴ�����ADCxBUF0��
				AD1CON1bits.ADON = 0;//ADC ����ģʽλ
									//1 = ADC ģ�����ڹ���
									//0 = ADC �ر�		
		

				AD1CHS0bits.CH0SA = 10; //AN9
				AD1CON1bits.ADON = 1; // Turn on the A/D converter
				asm("nop");asm("nop");asm("nop");asm("nop");
				AD1CON1bits.SAMP = 1; 		//ADC����ʹ��λ
											//1: ADC����/���ַŴ������ڲ���
											//0: ADC ����/���ַŴ������ֲ������
				while(!AD1CON1bits.DONE){}; //ADC ת��״̬λ
											//1 = ADC ת�����
											//0 = ADC ת����δ��ʼ���ڽ�����			
				temp[6] = ADC1BUF0;	//ÿ��A/D ת���Ľ���洢��ADCxBUF0�Ĵ�����
									//ADC ģ�����һ������ֻ��˫�˿ڼĴ�����ADCxBUF0��
				AD1CON1bits.ADON = 0;//ADC ����ģʽλ
									//1 = ADC ģ�����ڹ���
									//0 = ADC �ر�		
		


                 
				AD1CHS0bits.CH0SA = 11; //AN9
				AD1CON1bits.ADON = 1; // Turn on the A/D converter
				asm("nop");asm("nop");asm("nop");asm("nop");
				AD1CON1bits.SAMP = 1; 		//ADC����ʹ��λ
											//1: ADC����/���ַŴ������ڲ���
											//0: ADC ����/���ַŴ������ֲ������
				while(!AD1CON1bits.DONE){}; //ADC ת��״̬λ
											//1 = ADC ת�����
											//0 = ADC ת����δ��ʼ���ڽ�����			
				temp[7] = ADC1BUF0;	//ÿ��A/D ת���Ľ���洢��ADCxBUF0�Ĵ�����
									//ADC ģ�����һ������ֻ��˫�˿ڼĴ�����ADCxBUF0��
				AD1CON1bits.ADON = 0;//ADC ����ģʽλ
									//1 = ADC ģ�����ڹ���
									//0 = ADC �ر�		

            POWER=0;
            POWER1=0;
            POWER2=0;									
		}

		if(Send_Enable)
		{
			canTxMessage.message_type=CAN_MSG_DATA;		
			canTxMessage.frame_type=CAN_FRAME_STD;
			canTxMessage.buffer=0;
			canTxMessage.id=1+(BOARD_ID<<8);
			canTxMessage.data[0]=freq[0]>>8;
			canTxMessage.data[1]=freq[0];
			canTxMessage.data[2]=temp[0]>>8;
			canTxMessage.data[3]=temp[0];
			canTxMessage.data[4]=freq[1]>>8;
			canTxMessage.data[5]=freq[1];
			canTxMessage.data[6]=temp[1]>>8;
			canTxMessage.data[7]=temp[1];
			canTxMessage.data_length=8;
			sendECAN(&canTxMessage);
			
			DELAY(900);
			
			canTxMessage.message_type=CAN_MSG_DATA;		
			canTxMessage.frame_type=CAN_FRAME_STD;
			canTxMessage.buffer=0;
			canTxMessage.id=2+(BOARD_ID<<8);;
			canTxMessage.data[0]=freq[2]>>8;
			canTxMessage.data[1]=freq[2];
			canTxMessage.data[2]=temp[2]>>8;
			canTxMessage.data[3]=temp[2];
			canTxMessage.data[4]=freq[3]>>8;
			canTxMessage.data[5]=freq[3];
			canTxMessage.data[6]=temp[3]>>8;
			canTxMessage.data[7]=temp[3];
			canTxMessage.data_length=8;
			sendECAN(&canTxMessage);



           	DELAY(900);

			canTxMessage.message_type=CAN_MSG_DATA;		
			canTxMessage.frame_type=CAN_FRAME_STD;
			canTxMessage.buffer=0;
			canTxMessage.id=3+(BOARD_ID<<8);;
			canTxMessage.data[0]=freq[4]>>8;
			canTxMessage.data[1]=freq[4];
			canTxMessage.data[2]=temp[4]>>8;
			canTxMessage.data[3]=temp[4];
			canTxMessage.data[4]=freq[5]>>8;
			canTxMessage.data[5]=freq[5];
			canTxMessage.data[6]=temp[5]>>8;
			canTxMessage.data[7]=temp[5];
			canTxMessage.data_length=8;
			sendECAN(&canTxMessage);
			
			DELAY(900);
			
			canTxMessage.message_type=CAN_MSG_DATA;		
			canTxMessage.frame_type=CAN_FRAME_STD;
			canTxMessage.buffer=0;
			canTxMessage.id=4+(BOARD_ID<<8);;
			canTxMessage.data[0]=freq[6]>>8;
			canTxMessage.data[1]=freq[6];
			canTxMessage.data[2]=temp[6]>>8;
			canTxMessage.data[3]=temp[6];
			canTxMessage.data[4]=freq[7]>>8;
			canTxMessage.data[5]=freq[7];
			canTxMessage.data[6]=temp[7]>>8;
			canTxMessage.data[7]=temp[7];
			canTxMessage.data_length=8;
			sendECAN(&canTxMessage);



			Send_Enable = 0;	
		} 		
	}
	
	return 0;
}
