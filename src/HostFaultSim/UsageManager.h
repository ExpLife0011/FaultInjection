/*!
*	\file	UsageManager.h
*
*	\brief	ϵͳ��Դ����ע�������ͷ�ļ�
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
#ifndef USAGEMANAGER_H_
#define USAGEMANAGER_H_
#include "ace/Task.h"
#include "ConstDef.h"
#include <vector>
// #ifdef USAGEMANAGER_EXPORTS
// #define USAGEMANAGER_API  USAGEDLL_EXPORT
// #else
// #define USAGEMANAGER_API USAGEDLL_IMPORT
// #endif
class CCpuUsage;
class CMemUsage;
class CIoUsage;
class CUsageManager: public ACE_Task_Base
{
public:
	CUsageManager(void);
	~CUsageManager(void);
	int SetHostCpuSimParm(HostCpuSimPar & newSimPar);
	int SetHostMemSimParm(HostMemSimPar & newSimPar);
	int SetHostIOSimParm(HostIOSimPar & newSimPar);
	int GetSysResUseage(double &CPUUsage,double &MemUsage,unsigned long &IOKByte);

private:
	CCpuUsage * m_pCpuUsage;  //CPU����ע�����
	CMemUsage * m_pMemUsage;  //�ڴ����ע�����
	CIoUsage * m_IoUsage;     //IO����ע�����

	vector<HostCpuSimPar> m_vecHostCpuSimPar;   //CPU���������
	vector<HostMemSimPar> m_vecHostMemSimPar;   //Mem���������
	vector<HostIOSimPar> m_vecHostIOSimPar;     //IO���������
	//std::string m_sJobName;               //����ִ�е����������
	bool m_bCpuExeJob;                       //����ִ�е������
	bool m_bMemExeJob;                       //����ִ�е������
	bool m_bIOExeJob;                       //����ִ�е������
	bool m_bIsStop;                       //�߳�ֹͣ��־
	ACE_Thread_Mutex mtxThread;           //����


	
	//����CPUʹ����  ����ֵ��0��������-1
	int SetCpuUsage(bool bIsSet,float fCPU);
	//�����ڴ�ʹ����  ����ֵ��0��������-1�����õ��ڴ�ȵ�ǰʵ���ڴ��
	int SetRamUsage(bool bIsSet,unsigned long ulUsage,unsigned long ulVirUsage);
	//����IOʹ����  ����ֵ��0��������-1  ��������ʱ������
	int SetIoUsage(bool bIsSet,unsigned long ulUsage);

	//����IO������  ����ֵ��0��������-1  
	//����ulIOCount��д���ֽ���
	//����ulRIOCount����ȡ�ֽ���
	int SetIoCount(bool bIsSet,unsigned long ulIOCount = 0,unsigned long ulRIOCount = 0);
	
	/*!
	*	\brief	�߳�ִ����
	*/
	int svc(void);

	void StopThread();
};
//extern USAGEMANAGER_API CUsageManager* g_pUsageManager; //ϵͳ��Դ����ע�����ȫ�ֶ���
#endif

