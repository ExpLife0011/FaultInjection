/*!
*	\file	IoUsage.h
*
*	\brief	Ӳ��IO����ע�붨��ͷ�ļ�
*
*	\Author 
*
*	\date	2016-2-1 10:30
*
*	\version	1.0
*
*	\attention	Copyright (c) 2016,
*	\attention	All rights reserved.
*/
#ifndef IOUSAGE_H 
#define IOUSAGE_H
#include "ace/Task.h"
#include "ace/OS.h"
#include "ConstDef.h"
// #ifdef _WINDOWS
// #include "Qt\QThread.h"
// #include "Qt\qmutex.h"
// #else
// #include <qthread.h>
// #include <qmutex.h>
// #endif
class CIoUsage: public ACE_Task_Base
{
public:
	CIoUsage(void);
	~CIoUsage(void);
	int SetIoUsage1(bool bIsSet,unsigned long Usage); //����Ӳ��IOʹ����
	int SetIoCount1(bool bIsSet,unsigned long ulIOCount,unsigned long ulRIOCount); //����Ӳ��IO������
	unsigned long GetIoCount1();   //��ȡӲ��IO������
private:
	unsigned long m_IoWCount; //ÿsд���ļ����ֽ���
	unsigned long m_IoRCount; //ÿs���ļ���ȡ���ֽ���
	unsigned long m_IoUsage;  //Ӳ��IO��ʹ���ʣ�ʵ�ʹ���δ��ʹ�ã�
	//HANDLE m_IoThreadHandle;
	//bool m_bEndThread;
	//QMutex m_QM;
	bool m_bStop;     //�Ƿ�ֹͣģ�����ע���߳�
	//void run();

	/*!
	*	\brief	�߳�ִ����
	*/
	int svc(void);

	void StopThread();
	//void run();       //ģ�����ע���߳�ʵ����

    char *GetRandomBuffer(unsigned long Len);   //������д������
};

#endif
