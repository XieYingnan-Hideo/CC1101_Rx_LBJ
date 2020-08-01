/*-----------------------------------------------------------------------
*@file     system_utils.c
*@brief    ϵͳ��ʵ�ù���
*@author   лӢ��(xieyingnan1994@163.com��
*@version  1.0
*@date     2020/7/27
-----------------------------------------------------------------------*/
#include "CC1101_Rx_LBJ.h"

float Rf_Freq = 821.2375f;		//����Ƶ��821.2375MHz
volatile bool bDataArrivalFlag = false;	//��ʶ���ݵ�����־
/*-----------------------------------------------------------------------
*@brief		��������������
*@param		ָ���п����ݻ�����
*@retval	��
-----------------------------------------------------------------------*/
void ParseSerialCmdLine(char *Rxbuff)
{
	char *pos;

	if(Rxbuff[0] == '$')
	{
		if(Rxbuff[1] == '\0')	//$��ʾ������Ϣ
		{
			MSG("$ (View this help tips again)\r\n"
				"$$ (List current settings)\r\n"
				"$V (View version info)\r\n"
				"$F=xxx.xxxx (Setting frenquency to xxx.xxxx MHz)\r\n");
		}
		else if(Rxbuff[1] == '$')//$$��ӡ������Ŀ
		{
			ShowSettings();			//���ڴ�ӡ������Ŀ
		}
		else if(Rxbuff[1] == 'V')//$V��ʾ�汾��Ϣ
		{
			ShowBuildInfo();		//���ڴ�ӡ�汾��Ϣ	
		}		
		else if(Rxbuff[1] == 'F')//$F=xxx.xxxx���ý���Ƶ��
		{
			pos = strchr(Rxbuff,'=') + 1;	//ȷ���Ⱥź�ĵ�һ���ַ���λ��
			if(sscanf(pos,"%f",&Rf_Freq)!=1)	//�����������
			{	
				MSG("Wrong RF frenquency formate.\r\n");
			}
			else
			{
				MSG("RF freq was set to %f MHz.\r\n",Rf_Freq);
				CC1101_Initialize();	//���³�ʼ������CC1101
				CC1101_StartReceive(Rx_Callback);	//���¿�ʼ����
			}
		}
		else
			MSG("Unsupported command type.\r\n");
		BeeperMode = BEEP_ONCE;	//�������������һ��
	}
	else
		MSG("Wrong Command Format! Type '$' for help.\r\n");
}
/*-----------------------------------------------------------------------
*@brief		���CC1101����������
*@param		��
*@retval	��
-----------------------------------------------------------------------*/
void CC1101_Initialize(void)
{
	int8_t cc1101_state;	//����CC1101ʱ���ص�״̬��
	uint8_t delay_count = 0;	//��ʱ����

	MSG("CC1101 Initializing...\r\n");
	//1200bps 2FSKƵƫ4.5khz ���ջ�����58.0kHz ǰ����16�ֽ�
	//�̶�������64�ֽڣ�������ͬ������λ���������ز���⣬�ر�CRC����
	//ͬ����0x15D8����׼POCSAG�ĵ�16λ��
	cc1101_state = CC1101_Setup(Rf_Freq,1.2f,4.5f,58.0f,0,16);
	MSG("CC1101 initialize ");
	if(cc1101_state == RADIO_ERR_NONE)	//���ҵ����������óɹ�
	{
		MSG("OK!\r\n");
		BeeperMode = BEEP_ONCE;//��һ��
		StatusBlinkMode = BLINK_SLOW;//����ָʾ������մ���״̬
		ShowFixedPattern();	//OLED����ʾ�̶��ַ�(�����Ρ������ٶȡ��Ⱥ���)
	}
	else
	{
		MSG("failed! StateCode:%d\r\n",cc1101_state);
		StatusBlinkMode = BLINK_FAST;//����ָʾ�����쳣״̬
		MSG("System halt!\r\n");
		OLED_ShowString(0*8,0,"   Attention!   ",16);
		OLED_ShowString(0*8,2," CC1101 Invalid!",16);
		OLED_ShowString(0*8,4,"  Please Check! ",16);
		OLED_ShowString(0*8,6,"System Halting..",16);
		while(true)
		{
			Delay_ms(10);
			if(++delay_count == 100)
			{
				delay_count = 0;
				BeeperMode = DBL_BEEP;	//ÿ1����2��
			}
		}
	}
}
/*-----------------------------------------------------------------------
*@brief		��OLED��Ļ����ʾLBJ������Ϣ
*@param		LBJ_Msg - ָ���ѽ���POCSAG��Ϣ��ָ��
*           rssi,lqi - ��CCC1101�����ı��ν���ʱRSSI��LQIˮƽ
*@retval	��
-----------------------------------------------------------------------*/
void ShowMessageLBJ(POCSAG_RESULT* POCSAG_Msg,float rssi,uint8_t lqi)
{
	char LBJ_Info[3][7] = {{0},{0},{0}};//�洢���Ρ��ٶȡ��������Ϣ��ÿ��Ԥ��6�ַ�
	char Link_Info[2][6] = {{0},{0}};	//RSSI/LQI����������Ϣ

	for(uint8_t i = 0;i < 3;i++)
	{
		strncpy(LBJ_Info[i],POCSAG_Msg->txtMsg+i*5,5);//txtMsgÿ5���ַ�һ��ֱ�洢
	}
	//LBJ_Info[0]���Σ�12345��LBJ_Info[1]�ٶȣ�C100C��LBJ_Info[2]����꣺23456
	//"C"����ո񣬳������͹���겻��5λʱ�ڸ�λ��C,λ��Чʱ��ʾ"-"
	LBJ_Info[2][5] = LBJ_Info[2][4];	//����������λ�͵���2λ�����С����"."
	LBJ_Info[2][4] = '.';
	sprintf(Link_Info[0],"%.1f",rssi);	//��������ָʾ
	sprintf(Link_Info[1],"%hhu",lqi);

	OLED_ShowString(6*8,0,LBJ_Info[0],16);	//1.1��ʾ����
	if(POCSAG_Msg->FuncCode == FUNC_SHANGXING)	//1.2��ʾ������
		OLED_ShowPattern16x16(6*16,0,7); //��
	else if(POCSAG_Msg->FuncCode == FUNC_XIAXING)
		OLED_ShowPattern16x16(6*16,0,8); //��
	OLED_ShowOneChar(6*8,2,LBJ_Info[1][1],16);	//2.1��ʾ�ٶ�-��λ
	OLED_ShowOneChar(7*8,2,LBJ_Info[1][2],16); //2.2��ʾ�ٶ�-ʮλ
	OLED_ShowOneChar(8*8,2,LBJ_Info[1][3],16); //2.3��ʾ�ٶ�-��λ
	OLED_ShowString(7*8,4,LBJ_Info[2],16);	//3.��ʾ�����
	OLED_ShowString(5*8,6,Link_Info[0],16);	//4.1��ʾRSSI
	OLED_ShowString(14*8,6,Link_Info[1],16);//4.2��ʾLQI

}
/*-----------------------------------------------------------------------
*@brief		OLED����ʾ�̶��ַ�(�����Ρ������ٶȡ��Ⱥ���)
**@detail   ����: ----- ����	���ִ�С16*16 Ӣ��16x8
*           �ٶ�: --- km/h   ���ִ�С16*16 Ӣ��16x8
*           �����: ---.- km ���ִ�С16*16 Ӣ��16x8
*           RSSI:---.-LQI:-- Ӣ��16x8
*@param		��
*@retval	��
-----------------------------------------------------------------------*/
void ShowFixedPattern(void)
{
	//��һ�У��к�0-1
	OLED_ShowPattern16x16(0*16,0,0); //��
	OLED_ShowPattern16x16(1*16,0,1); //��
	OLED_ShowPattern16x16(6*16,0,7); //��
	OLED_ShowPattern16x16(7*16,0,9); //��
	OLED_ShowString(4*8,0,": ----- ",16);
	//�ڶ���,�к�2-3
	OLED_ShowPattern16x16(0*16,2,2); //��
	OLED_ShowPattern16x16(1*16,2,3); //��
	OLED_ShowString(4*8,2,": --- km/h",16);
	//�����У��к�4-5
	OLED_ShowPattern16x16(0*16,4,4); //��
	OLED_ShowPattern16x16(1*16,4,5); //��
	OLED_ShowPattern16x16(2*16,4,6); //��
	OLED_ShowString(6*8,4,": ---.- km",16);
	//�����У��к�6-7
	OLED_ShowString(0*8,6,"RSSI:---.- LQI--",16);	
}
/*-----------------------------------------------------------------------
*@brief		��ʾ�汾��Ϣͨ�����ڴ�ӡ
*@param		��
*@retval	��
-----------------------------------------------------------------------*/
void ShowBuildInfo(void)
{
	MSG("%s %s (Build %s %s) [Type '$' for help.]\r\n",APP_NAME_STR,
		      VERTION_STR,BUILD_DATE_STR,BUILD_TIME_STR);//���ڴ�ӡ�汾��Ϣ
	MSG("Xie Yingnan Works.<xieyingnan1994@163.com>\r\n");
}

/*-----------------------------------------------------------------------
*@brief		OLED��ʾ��������Ͱ汾��Ϣ
*@param		��
*@retval	��
-----------------------------------------------------------------------*/
void ShowSplashScreen(void)
{
	char buf[17] = {0};	//ÿ�����16�ַ�
	OLED_ShowBMP128x64(nBitmapDot);
	//nBitmapDot����HW_OLED_Font.c�ж���Ŀ�������λͼ
	Delay_ms(1500);
	OLED_Clear();//����
	sprintf(buf,"<%s>",APP_NAME_STR);
	OLED_ShowString(0,0,buf,12);//8x6���壬�п�6ռ��8���и�8
	sprintf(buf,"<Version:%s>",VERTION_STR);
	OLED_ShowString(0,1,buf,12);
	OLED_ShowString(0,2,"  <Build Date>  ",12);
	sprintf(buf,"Date:%s",BUILD_DATE_STR);
	OLED_ShowString(0,3,buf,12);
	sprintf(buf,"Time:%s",BUILD_TIME_STR);
	OLED_ShowString(0,4,buf,12);
	OLED_ShowString(0,5,"----------------",12);
	OLED_ShowString(0,6,">Author:Xie Y.N.",12);
	OLED_ShowString(0,7,">CallSign:BH2RPH",12);
	Delay_ms(1500);
	Delay_ms(1200);
	OLED_Clear();//����
}
/*-----------------------------------------------------------------------
*@brief		���ڴ�ӡ������Ŀ
*@param		��
*@retval	��
-----------------------------------------------------------------------*/
void ShowSettings(void)
{
	MSG("Settings:\r\n");
	MSG("Rf frequency:%.4fMHz\r\n",Rf_Freq);
}
/*-----------------------------------------------------------------------
*@brief		CC1101���ݰ��������ʱ�Ļص�����
*@param		���ճ�ʼ��ʱ�ļĴ������ã��������ʱCC1101����IDLE̬��
*        	���ݱ�����FIFO
*@retval	��
-----------------------------------------------------------------------*/
void Rx_Callback(void)
{
	if(!bDataArrivalFlag)
		bDataArrivalFlag = true;	//�����ݵ����־λ
}
/*-----------------------------------------------------------------------
*@brief		��ȡ�ѽ��յ����ݲ��������ʾ
*@param		���ճ�ʼ��ʱ�ļĴ������ã��������ʱCC1101����IDLE̬��
*        	���ݱ�����FIFO�С��������Ҫ�ٴη���RX������ɽ���
*@retval	��
-----------------------------------------------------------------------*/
void RxDataFeedProc(void)
{
	uint8_t* batch_buff = NULL;	//�������ԭʼ���ݵĻ�����
	uint32_t batch_len = CC1101_GetPacketLength(false);
			//��ȡ�����õİ�����,�ڱ��������ڳ�ʼ��������ΪFIFO�Ĵ�С64�ֽ�
	uint32_t actual_len;//ʵ�ʶ�����ԭʼ���ݳ��ȣ�����ģʽʱ��batch_len��ͬ
	POCSAG_RESULT PocsagMsg;//����POCSAG�������Ľṹ��

	if((batch_buff=(uint8_t*)malloc(batch_len*sizeof(uint8_t))) != NULL)
	{
		memset(batch_buff,0,batch_len);	//���batch����

		CC1101_ReadDataFIFO(batch_buff,&actual_len);//��FIFO����ԭʼ����
		float rssi = CC1101_GetRSSI();//���ڽ�����ɺ���IDLE̬
		uint8_t lqi = CC1101_GetLQI();//�����RSSI��LQI���᲻���뱾�����ݰ����Ӧ

		MSG("!!Received %u bytes of raw data.\r\n",actual_len);
		MSG("RSSI:%.1f LQI:%hhu\r\n",rssi,lqi);
		MSG("Raw data:\r\n");
		for(uint32_t i=0;i < actual_len;i++)
		{
			MSG("%02Xh ",batch_buff[i]);//��ӡԭʼ����
			if((i+1)%16 == 0)
				MSG("\r\n");	//ÿ��16��
		}
		//����LBJ��Ϣ����ַ�ѹ��ˣ�
		int8_t state = POCSAG_ParseCodeWordsLBJ(&PocsagMsg,batch_buff,
												 actual_len,true);							     		 
		if(state == POCSAG_ERR_NONE)
		{										
			MSG("Address:%u,Function:%hhd.\r\n",PocsagMsg.Address,PocsagMsg.FuncCode);
												//��ʾ��ַ�룬������
			MSG("LBJ Message:%s.\r\n",PocsagMsg.txtMsg);//��ʾ�ı���Ϣ
			ShowMessageLBJ(&PocsagMsg,rssi,lqi);	//��OLED��Ļ����ʾLBJ������Ϣ
			switch(PocsagMsg.FuncCode)	//���������ݹ����������Ӧ����
			{
				case FUNC_XIAXING:
					BeeperMode = BEEP_ONCE;//��һ��
					InfoBlinkMode = BLINK_ONCE;//��˸һ��
					break;
				case FUNC_SHANGXING:
					InfoBlinkMode = DBL_BLINK;//��˸����
					BeeperMode = DBL_BEEP;//������
					break;
				default: BeeperMode = DBL_BEEP; break;
			}
		}
		else
		{
			MSG("POCSAG parse failed! Errorcode:%d\r\n",state);
			BeeperMode = DBL_BEEP;
		}
		free(batch_buff);
	}
	CC1101_StartReceive(Rx_Callback);	//��������
}
/*-----------------------------------------------------------------------
*@brief		��ʱ���жϷ����������ṩʱ��
*@detail 	��ʱ������TIM��ʼ��������ȷ����������Ϊ10mS
*@param		��
*@retval	��
-----------------------------------------------------------------------*/
void INT_TIMER_IRQHandler(void)
{
	static uint8_t cnt_beep = 0,cnt_beeptimes = 0;
	static uint8_t cnt_blink = 0;
	static uint8_t cnt_infoblink = 0,cnt_infoblinktimes = 0;

	if(TIM_GetITStatus(INT_TIMER,TIM_IT_Update)!=RESET)
	{
		switch(BeeperMode)	//��ʾ�������Ŀ���
		{
		case BEEP_ONCE:							//��һ��
			BUZZER_ON();
			if(++cnt_beep >= 10)
			{ BUZZER_OFF(); BeeperMode = BEEP_OFF; }
			break;
		case DBL_BEEP: 
			if(cnt_beeptimes < 2)
			{
				if(cnt_beep <= 8) {BUZZER_ON();}	//��80ms
				else {BUZZER_OFF();}				//ͣ80ms
				if(++cnt_beep >= 16)				//����160ms
				{cnt_beep = 0; cnt_beeptimes++;}
			}
			else
				BeeperMode = BEEP_OFF;	//��2�κ�ͣ
			break;
		default:
			BUZZER_OFF();	//Ĭ�Ϲرշ�����
			cnt_beep = 0;	//���beep��ʱ����
			cnt_beeptimes = 0; //���beep�Ǵα���
			break;
		}

		switch(StatusBlinkMode)	//״̬LED
		{
		case BLINK_FAST:
			if(++cnt_blink >= 10)	//����200ms,50%duty
			{ STATUS_LED_TOGGLE(); cnt_blink = 0; }
			break;
		case BLINK_SLOW:
			if(cnt_blink <= 20) {STATUS_LED_ON();}	//��200ms
			else {STATUS_LED_OFF();}			//��800ms
			if(++cnt_blink >= 100)				//����1000ms
				cnt_blink = 0;
			break;
		default:
			STATUS_LED_OFF();
			cnt_blink = 0;
			break;
		}

		switch(InfoBlinkMode)
		{
		case BLINK_ONCE:
			INFO_LED_ON();
			if(++cnt_infoblink >= 15)
			{ 
				INFO_LED_OFF();
				InfoBlinkMode = BLINK_OFF;
			}
			break;
		case DBL_BLINK:
			if(cnt_infoblinktimes < 2)
			{
				if(cnt_infoblink <= 10) {INFO_LED_ON();}	//��100ms
				else {INFO_LED_OFF();}				//��100ms
				if(++cnt_infoblink >= 20)				//����200ms
				{cnt_infoblink = 0; cnt_infoblinktimes++;}
			}
			else
				InfoBlinkMode = BLINK_OFF;	//��2�κ�ͣ
			break;
		default:
			INFO_LED_OFF();
			cnt_infoblink = 0;
			cnt_infoblinktimes = 0;
			break;
		}
	}
	TIM_ClearITPendingBit(INT_TIMER,TIM_IT_Update);
}
