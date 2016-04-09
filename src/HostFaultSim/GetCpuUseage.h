/*!
*	\file	CpuUsage.h
*
*	\brief	��ȡCPUʹ���ʶ���ͷ�ļ�
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
#ifndef GETCPUUSAGE_H 
#define GETCPUUSAGE_H
#include "ConstDef.h"
#include <stdlib.h>
#include <vector>
#include <string>
#include <map>

#ifdef _WINDOWS
#include <psapi.h>
#include <pdh.h> 
#include "tlhelp32.h"
//#pragma comment(lib,"Psapi.lib")
#else
typedef struct PACKEDOCCUPY    //����һ��cpu occupy�Ľṹ��
{
	char name[20];      //����һ��char���͵�������name��20��Ԫ��
	unsigned int user; //����һ���޷��ŵ�int���͵�user
	unsigned int nice; //����һ���޷��ŵ�int���͵�nice
	unsigned int system;//����һ���޷��ŵ�int���͵�system
	unsigned int idle; //����һ���޷��ŵ�int���͵�idle
	unsigned int iowait; //����һ���޷��ŵ�int���͵�idle
	unsigned int irq ; //����һ���޷��ŵ�int���͵�idle
	unsigned int softirq; //����һ���޷��ŵ�int���͵�idle
	unsigned int stealstolen; //����һ���޷��ŵ�int���͵�idle
	unsigned int guest; //����һ���޷��ŵ�int���͵�idle
}CPU_OCCUPY;
#endif

#ifndef _WINDOWS
#define PROCESS_ITEM 14
typedef long long __int64_;
#include <unistd.h>
#include <stdint.h>
#include <dirent.h>
#include <sys/types.h>
typedef struct				
{
	__int64_ user;	
	__int64_ nice;	
	__int64_ system;	
	__int64_ idle;	
}total_cpu_occupy_t;

typedef struct
{
	pid_t pid;		
	__int64_ utime;	
	__int64_ stime;	
	__int64_ cutime;	
	__int64_ cstime;	
}process_cpu_occupy_t;
#endif



class CGetCpuUseage
{
public:
	CGetCpuUseage(void);
	~CGetCpuUseage(void);
	float GetCpuUsage();

#ifdef _WINDOWS
public:
	float WinCpuUseage();
	//!�õ�CPU�ĸ���
	/*int GetProcessorNumber();*/
	__int64 CompareFileTime ( FILETIME time1, FILETIME time2);
	//uint64_t file_time_2_utc(const FILETIME* ftime);

private:
	FILETIME m_preidleTime;
	FILETIME m_prekernelTime;
	FILETIME m_preuserTime;
#else
	float get_cpu_usage_rate();
	float cal_cpuoccupy (CPU_OCCUPY *o, CPU_OCCUPY *n);
	//��������get��������һ���βνṹ����Ū��ָ��O
	void get_cpuoccupy (CPU_OCCUPY *cpust);
#endif
};
#endif
