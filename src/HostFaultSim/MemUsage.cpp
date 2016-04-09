#include "MemUsage.h"
//#include "ace/OS.h"
//#include "ace/OS_NS_unistd.h"
// #ifdef _WINDOWS
// #include "Qt\QThread.h"
// #include "Qt\qmutex.h"
// #else
// #include <qthread.h>
// #include <qmutex.h>
// #endif

CMemUsage::CMemUsage(void)
{
	m_ListHead = 0;
	m_VirListHead = 0;
	m_RamUsage = 0;
	m_VirUsage = 0;
	m_bStop = false;
	//m_MemThreadHandle = 0;
}


CMemUsage::~CMemUsage(void)
{
}

int CMemUsage::GetRamUsage1()
{
	int iMemUsage = GetRamCurUsage();
	return iMemUsage;
}
int CMemUsage::SetMemUsage1(bool bIsSet,unsigned long Usage,unsigned long VirUsage)
{
	int iRet = 0;
	m_RamUsage = Usage;
	m_VirUsage = VirUsage;
	int icurVal = GetRamCurUsage();
// 	if (bIsSet == true && GetRamCurUsage() >= m_RamUsage) //��ȡ��ǰ�ڴ�ռ���ʣ��Ƚϵ�ǰ�ڴ��������ڴ�Ĵ�С
// 	{
// 		iRet = -2; //���õ��ڴ�ռ���ʱȵ�ǰʵ��ռ�õĵ�
// 		char pLog[MAX_LOG_LEN];
// 		LOG(pLog, "SetMemUsage setval %ul < curval %ul", Usage, icurVal);
// 		return iRet;
// 	}

	//m_QM.lock();
	if (bIsSet == true)
	{
		activate();
	}
	else
	{
		m_bStop = true;
	}
	//m_QM.unlock();
	return iRet;
}


int CMemUsage::svc()
{
	int iRet = 0;
	while (1)
	{
		if (m_RamUsage != 0)
		{
			//m_QM.lock();
			while(GetRamCurUsage() < m_RamUsage)
			{
				RamChunkList* ptr = (RamChunkList*)malloc(sizeof(RamChunkList));
				if (ptr)
				{
					//memset(ptr->RamChunk, 0, 0x400000-8);
					memset(ptr->RamChunk, 0, 4*1024*1024);

					ptr->Next = m_ListHead;
					m_ListHead = ptr;
#ifdef _WINDOWS
					//ACE_OS::sleep(ACE_Time_Value(0,20*1000));
					Sleep(20);
#else
					usleep(20*1000);
#endif
				}

			}

			while (GetRamCurUsage() > m_RamUsage && m_ListHead != 0)
			{
				RamChunkList* ptr = m_ListHead;
				m_ListHead = m_ListHead->Next;
				free(ptr);

			}
#ifdef _WINDOWS
			//ACE_OS::sleep(ACE_Time_Value(0,20*1000));
			Sleep(1000);
#else
			sleep(1);
#endif
			//Sleep(1000);
			
		}


		if (m_VirUsage != 0)
		{
			//m_QM.lock();
			while(GetVirCurUsage() < m_VirUsage)
			{
 #ifdef _WINDOWS
				RamChunkList* ptr = (RamChunkList*)VirtualAlloc(NULL,sizeof(RamChunkList),
					MEM_COMMIT,
					PAGE_READWRITE);
				if (ptr)
				{
					//memset(ptr->RamChunk, 0, 0x400000-8);
					memset(ptr->RamChunk, 0, 4*1024*1024);

					ptr->Next = m_ListHead;
					m_ListHead = ptr;
					//ACE_OS::sleep(ACE_Time_Value(0,20*1000));
					Sleep(20);


				}
#else
            usleep(20*1000);
#endif
			}

			while (GetVirCurUsage() > m_VirUsage && m_VirListHead != 0)
			{
				RamChunkList* ptr = m_VirListHead;
				m_VirListHead = m_VirListHead->Next;
				free(ptr);

			}
#ifdef _WINDOWS
			//ACE_OS::sleep(ACE_Time_Value(0,20*1000));
			Sleep(1000);
#else
			sleep(1);
#endif
			//Sleep(1000);

		}



		if (m_bStop)   //�߳���ֹ
		{
			while (m_ListHead)
			{
				RamChunkList* ptr = m_ListHead;
				m_ListHead = m_ListHead->Next;
				free(ptr);
			}

			while (m_VirListHead)
			{
#ifdef _WINDOWS
				RamChunkList* ptr = m_VirListHead;
				m_VirListHead = m_VirListHead->Next;
				VirtualFree(ptr,sizeof(RamChunkList),MEM_DECOMMIT );
#else
#endif
			}

			m_bStop = false;
			break;
		}
		//m_QM.unlock();
	}
		
	return iRet;
}
void CMemUsage::StopThread()
{
	m_bStop = true;
}
#ifdef _WINDOWS
int CMemUsage::GetRamCurUsage()
{
	int iRet;
	MEMORYSTATUSEX MemoryStatus;
	MemoryStatus.dwLength = sizeof(MemoryStatus);
	GlobalMemoryStatusEx(&MemoryStatus);
	//!���ڴ�ռ�ñ�
	iRet = (int)MemoryStatus.dwMemoryLoad;
	return iRet;

}
int CMemUsage::GetVirCurUsage()
{
	int iRet;
	MEMORYSTATUSEX MemoryStatus;
	MemoryStatus.dwLength = sizeof(MemoryStatus);
	GlobalMemoryStatusEx(&MemoryStatus);
	//!���ڴ�ռ�ñ�
	float fLoad = ((float)MemoryStatus.ullAvailVirtual)/((float)MemoryStatus.ullTotalVirtual);
	iRet = 100 - (int)(fLoad * 100);
	return iRet;
}
#else
int CMemUsage::GetRamCurUsage()
{
	MEM_OCCUPY mem_stat;
	//��ȡ�ڴ�
	Get_Memoccupy ((MEM_OCCUPY *)&mem_stat);
    //unsigned long usememory = mem_stat.free + mem_stat.buffers + mem_stat.cached;

    int iRate = ((float)(mem_stat.total-mem_stat.free)/(float)mem_stat.total)*100;

	return iRate;
}
void CMemUsage::Get_Memoccupy (MEM_OCCUPY *mem) //��������get��������һ���βνṹ����Ū��ָ��O
{
	FILE *fp;
	ssize_t nread;
	size_t len = 0;
	char *buf=NULL;
	char *buffer=NULL;

	char *file = "/proc/meminfo";
	char content[64]="";
	fp = fopen(file, "rb");
	if (fp == NULL)
	{
		//printf("error to open: %s\n", file);
// 		char pLog[MAX_LOG_LEN];
// 		LOG(pLog, "Get_Memoccupy open %s error", file);
		return ;
	}
	int iCount = 0;
	while((nread = getline(&buffer, &len, fp)) != -1) 
	{
		if((buf = strstr(buffer, "MemTotal"))!=NULL)  /* ��ʵ��grep�Ĺ��� */
		{
			buffer[strlen(buffer)-1]=0;             /* ��ʵ��tr()�����Ĺ��� */
			sscanf(buffer, "%s%s", content, content);
			unsigned long memtotal_kb=(unsigned long)(strtof(content, NULL));
			iCount++;
			mem->total = memtotal_kb;
		}
		if((buf=strstr(buffer, "MemFree"))!=NULL)  /* ��ʵ��grep�Ĺ��� */
		{
			buffer[strlen(buffer)-1]=0;                 /* ��ʵ��tr()�����Ĺ��� */
			sscanf(buffer, "%s%s", content, content);
			unsigned long memfree_kb=(unsigned long)(strtof(content, NULL));
			mem->free = memfree_kb;
			iCount++;

		}
		if((buf=strstr(buffer, "Buffers"))!=NULL)  /* ��ʵ��grep�Ĺ��� */
		{
			buffer[strlen(buffer)-1]=0;                 /* ��ʵ��tr()�����Ĺ��� */
			sscanf(buffer, "%s%s", content, content);
			unsigned long membuffers_kb=(unsigned long)(strtof(content, NULL));
			mem->buffers = membuffers_kb;
			iCount++;

		}
		if((buf=strstr(buffer, "Cached"))!=NULL)  /* ��ʵ��grep�Ĺ��� */
		{
			buffer[strlen(buffer)-1]=0;                 /* ��ʵ��tr()�����Ĺ��� */
			sscanf(buffer, "%s%s", content, content);
			unsigned long memcached_kb=(unsigned long)(strtof(content, NULL));
			mem->cached = memcached_kb;
			iCount++;
		}
		//!ȡ���ĸ��ؼ���
		if (iCount>=4)
		{
			break;
		}
	}
	if (buffer)
	{
		free(buffer);
		buffer = NULL;
	}
	fclose(fp);     
}
int CMemUsage::GetVirCurUsage()
{
    int iRet = 0;

    return iRet;
}
#endif

