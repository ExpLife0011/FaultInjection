/*!
*	\file	MemUsage.h
*
*	\brief	�ڴ����ע�붨��ͷ�ļ�
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
#ifndef MEMUSAGE_H 
#define MEMUSAGE_H
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
typedef struct RamChunkList
{
	RamChunkList* Next;
	//CHAR RamChunk[0x400000-8];
	char RamChunk[4*1024*1024];
}RamChunkList;

//class QMutex;
class CMemUsage: public ACE_Task_Base

{
public:
	CMemUsage(void);
	~CMemUsage(void);
    int SetMemUsage1(bool bIsSet,unsigned long Usage,unsigned long VirUsage = 0); //�����ڴ�ʹ����
	int GetRamUsage1(); //��ȡ�ڴ�ʹ����
private:
	//QMutex m_QM;
	unsigned long m_RamUsage;    //�ڴ�ʹ����
	unsigned long m_VirUsage;    //z�����ڴ�ʹ����
	RamChunkList* m_ListHead;    //�ڴ������б�
	RamChunkList* m_VirListHead;    //�ڴ������б�
	bool m_bStop;

	//void SetFlg(bool flg);

	//void run();   //ģ�����ע���߳�ʵ����

	/*!
	*	\brief	�߳�ִ����
	*/
	int svc(void);

	void StopThread();

#ifdef _WINDOWS
	int GetRamCurUsage(); //��ȡ�ڴ�ʹ����
	int GetVirCurUsage(); //��ȡ�����ڴ�ʹ����
#else
	typedef long long __int64_;
	typedef struct PACKEDMEM         //����һ��mem occupy�Ľṹ��
	{
		unsigned long free;          //����һ��char���͵�������name��20��Ԫ��
		unsigned long total; 
		unsigned long buffers;   
		unsigned long cached;                       
	}MEM_OCCUPY;
	int GetRamCurUsage();//��ȡ�ڴ�ʹ����
	int GetVirCurUsage(); //��ȡ�����ڴ�ʹ����
	void Get_Memoccupy (MEM_OCCUPY *mem);
#endif
};
#endif

