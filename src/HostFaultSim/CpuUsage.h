/*!
*	\file	CpuUsage.h
*
*	\brief	CPU����ע�붨��ͷ�ļ�
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
#ifndef CPUUSAGE_H 
#define CPUUSAGE_H
#include "ConstDef.h"
#include "GetCpuUseage.h"
class CCpuUsage
{
public:
	CCpuUsage(void);
	~CCpuUsage(void);
	int SetCpuUsage1(bool bIsSet,double dCpu); //����CPUʹ����
	float GetCpuUsage1();                      //��ȡCPUʹ����
private:
	bool m_bIsStop;   //�Ƿ�ֹͣģ�����ע���߳�
	int m_iCpuNum;    //CPU����
	double m_dRatio;  
	CGetCpuUseage * m_pGetCpuUseage;  //��ȡCPU��ǰʹ����
#ifdef _WINDOWS
	void Run();   //ִ��ģ��
	static DWORD WINAPI ThreadProc (LPVOID pParam);  //ģ���߳�ִ����
#else
   void Run_Linux();  //ִ��ģ��
   static void * ThreadProc_Linux( void* pvParameter );//ģ���߳�ִ����
#endif


	//int GetCpuUsage();
};
#endif

