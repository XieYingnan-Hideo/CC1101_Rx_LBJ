/*-----------------------------------------------------------------------
*@file     CC1101_Rx_LBJ.h
*@brief    ���̼�ͷ�ļ�����Ӧ�ò�������ļ����ɡ�
*@author   лӢ��(xieyingnan1994@163.com��
*@version  1.0
*@date     2020/07/27
-----------------------------------------------------------------------*/
#ifndef CC1101_RX_LBJ_H
#define CC1101_RX_LBJ_H
/*-------------оƬ��֧�ֿ�-------------*/
#include "stm32f10x.h"
/*--------------C���Ա�׼��-------------*/
#include <stdio.h>	//printf...
#include <string.h>	//memcpy...
#include <stdint.h>	//uint8_t...
#include <stdbool.h>//C99:true...false...
#include <stdarg.h> //C99:VA_ARGS...
#include <stdlib.h> //atoi...
/*----------�ײ�Ӳ��ͷ�ļ�����----------*/
#include "Hardware.h"
/*------------����ȫ��֧�ֿ�------------*/
#include "bit.h"	//����λ����
#include "delay.h"	//��ʱ
#include "POCSAG_ParseLBJ.h" //POCSAG�����г��ӽ�Ԥ����Ϣ����
#include "system_utils.h" //ϵͳ��ʵ�ù���
/*-----------������ȫ�ֺ궨��-----------*/
#ifndef MSG
#define MSG(...) printf(__VA_ARGS__)	//���������Ϣ�����
#endif
#ifdef DEBUG_ERR_CONSOLE_ON
	#define MSG_ERR(num, str, val) \
			MSG("%s,%d:0x%x %s 0x%x\n",__FILE__, __LINE__,num, str, val);
#else
	#define MSG_ERR(num, str, val)
#endif
/* ���徯����Ϣ�����MSG_WAR */
#ifdef DEBUG_WAR_CONSOLE_ON
	#define MSG_WAR(num, str, val) \
			MSG("%s,%d:0x%x %s 0x%x\n",__FILE__, __LINE__,num, str, val);
#else
	#define MSG_WAR(num, str, val)
#endif
/* ����汾�ź͹�����Ϣ�ַ��� */
#define APP_NAME_STR "CC1101_Rx_LBJ"
#define VERTION_STR "V1.0"
#define BUILD_DATE_STR __DATE__
#define BUILD_TIME_STR __TIME__
#endif
