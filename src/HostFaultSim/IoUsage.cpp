#include "IoUsage.h"
#ifdef _WINDOWS
#pragma  comment(lib,"pdh") 
#include <malloc.h> 
#include <locale.h> 
#include <pdh.h> 
#include <pdhmsg.h> 
#include <tchar.h> 
#else
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#include <vector>
using namespace std;
CIoUsage::CIoUsage(void)
{
	m_IoUsage = 0;
	m_IoWCount = 0;
	m_IoRCount = 0;
	m_bStop = false;
}


CIoUsage::~CIoUsage(void)
{
}


int CIoUsage::SetIoUsage1(bool bIsSet,unsigned long Usage)
{
	int iRet = 0;
	//m_QM.lock();
	m_IoUsage = Usage;

	if (bIsSet == true )
	{
		//start();
		activate();
	}
	else
	{
		m_bStop = true;
	}
	//m_QM.unlock();
	return iRet;
}
unsigned long CIoUsage::GetIoCount1()
{
	unsigned long IOKByte = 0;
#ifdef _WINDOWS
	//ϵͳ�ڴ�����     
	MEMORYSTATUSEX statex;       
	statex.dwLength = sizeof(statex);      
	GlobalMemoryStatusEx(&statex);       
	//m_lSysTotalMem = statex.ullTotalPhys / 1024; 
	HQUERY  hQuery;       
	HCOUNTER hcWSysIO,hcRSysIO;             
	PDH_STATUS lStatus = PdhOpenQuery(NULL, NULL, &hQuery);       
	if (lStatus != ERROR_SUCCESS)     
	{      
		//TODO ���������־
		return IOKByte;     
	}       
	PdhAddCounter(hQuery, _T("\\PhysicalDisk(_Total)\\Avg. Disk Bytes/Write"), NULL, &hcWSysIO);
	PdhAddCounter(hQuery, _T("\\PhysicalDisk(_Total)\\Avg. Disk Bytes/Read"), NULL, &hcRSysIO);
	PDH_FMT_COUNTERVALUE cv;  
	DWORD dTime1 = GetTickCount();
	memset(&cv,0,sizeof(PDH_FMT_COUNTERVALUE));       
	lStatus = PdhCollectQueryData(hQuery);         
	if (lStatus != ERROR_SUCCESS)     
	{   
		//���������־
// 		char pLog[MAX_LOG_LEN];
// 		LOG(pLog, "GetIoCount PdhCollectQueryData ret=%l", lStatus);
		return IOKByte;     
	}       
	lStatus = PdhGetFormattedCounterValue(hcWSysIO, PDH_FMT_LONG, NULL, &cv);       
	if (lStatus == ERROR_SUCCESS)     
	{         
		IOKByte = cv.longValue;    
	}
	lStatus = PdhGetFormattedCounterValue(hcRSysIO, PDH_FMT_LONG, NULL, &cv);       
	if (lStatus == ERROR_SUCCESS)     
	{         
		IOKByte += cv.longValue/1024;    
	}
	PdhRemoveCounter(hcWSysIO);  
	PdhRemoveCounter(hcRSysIO);     
	PdhCloseQuery(hQuery);
#else
	std::vector<string> resvec;
	resvec.clear();
	char chCmd[] = "iostat -d";
	FILE *pp = popen(chCmd, "r"); //�����ܵ�
	if (!pp) 
	{
		//���������־
// 		char pLog[MAX_LOG_LEN];
// 		LOG(pLog, "GetIoCount popen iostat -d error");
		return 0;
	}
	char tmp[1024]; //����һ�����ʵĳ��ȣ��Դ洢ÿһ�����
	while (fgets(tmp, sizeof(tmp), pp) != NULL) 
	{
		if (tmp[strlen(tmp) - 1] == '\n') 
		{
			tmp[strlen(tmp) - 1] = '\0'; //ȥ�����з�
		}
		resvec.push_back(tmp);
	}
	if (resvec.size() >= 3)
	{
		char name[50];
		float ftps;
                float IORByte = 0;
                float IOWByte = 0;
		unsigned long TIORByte = 0;
		unsigned long TIOWByte = 0;
                string sIOInfo = resvec[3];
                sscanf (sIOInfo.c_str(), "%s%f%f%f%u%u", name, &ftps,&IORByte, &IOWByte,&TIORByte, &TIOWByte);
                IOKByte = (unsigned long)(IORByte + IOWByte);
	}
        resvec.clear();
	pclose(pp); //�رչܵ�

#endif
	return IOKByte;
}

int CIoUsage::SetIoCount1(bool bIsSet,unsigned long ulIOCount,unsigned long ulRIOCount)
{
	int iRet = 0;
	//m_QM.lock();
	m_IoWCount = ulIOCount;
	m_IoRCount = ulRIOCount;
	if (bIsSet == true )
	{
        m_bStop = false;
		activate();

	}
	else
	{
		m_bStop = true;
	}
	//m_QM.unlock();
	unsigned long unIOCount = GetIoCount1();
	return iRet;
}
int  CIoUsage::svc()
{
	int iRet = 0;
	//m_QM.lock();
	if (m_IoUsage != 0)   // ����IOʹ����
	{
		unsigned long ulSize = 1024*1024;
		int x = 10;
        char *Buffer = GetRandomBuffer(ulSize*x);
	#ifdef _WINDOWS
		CHAR* pFileName = "D:\\IO.dat";
		HANDLE FileHandle = CreateFile(pFileName, GENERIC_READ | GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH | FILE_FLAG_NO_BUFFERING | FILE_ATTRIBUTE_NORMAL, NULL);
	
		while (1)
		{
			DWORD Writed;
			unsigned ta = GetTickCount();   
			while(GetTickCount() - ta < m_IoUsage*10) 
			{
				WriteFile(FileHandle, Buffer, ulSize*x, &Writed, NULL);
			}  
			SetFilePointer(FileHandle, 0,0, FILE_BEGIN);
			Sleep((100 - m_IoUsage) * 10);  
			if (m_bStop)
			{
				m_bStop = false;
				break;
			}
		}
		CloseHandle(FileHandle);
		DeleteFile(pFileName);
	#else
		FILE *fp;
		if((fp = fopen("IO.dat","r+")) == NULL)
		{
// 			char pLog[MAX_LOG_LEN];
// 			LOG(pLog, " fopen  IO.dat error");
			//printf("open file failer!\n");
			return;
		}
		fseek(fp,0,SEEK_END);

		int busyTime = 10;   //æ��ʱ��
		int idleTime = 0;    //����ʱ��
		long timeuse = 0;    //ʵ��ѭ���õ�ʱ��
		struct timeval tpstart,tpend; 
		gettimeofday(&tpstart,NULL); //�õ���ǰ��ϵͳʱ��
		while(1)  
		{  
			gettimeofday(&tpstart,NULL); //�õ���ǰ��ϵͳʱ��
			while (timeuse <= busyTime)
			{
				gettimeofday(&tpend,NULL);
				timeuse = 1000000 * (tpend.tv_sec - tpstart.tv_sec) + (tpend.tv_usec - tpstart.tv_usec);  
				timeuse /= 1000;               //ת����ms   
				fwrite(Buffer,1,strlen(Buffer),fp);
			}
			rewind(fp);//ʹָ�����·��ص��ļ��Ŀ�ͷ
            //idleTime = ((100 * busyTime) / m_dRatio) - busyTime;

			sleep(idleTime / 1000);    //ת����ms
            if (true == m_bStop)
			{
				break;
			}
		}
		fclose(fp);
	#endif
		free(Buffer);
	}
	else if (m_IoWCount != 0 || m_IoRCount != 0)
	{
		unsigned long ulWSize = m_IoWCount*1024;
		unsigned long ulRSize = m_IoRCount*1024;
        char * WBuffer = GetRandomBuffer(ulWSize);
        char * RBuffer = GetRandomBuffer(ulRSize+1024);
	#ifdef _WINDOWS
		//����д�ļ�
		CHAR* pWFileName = "D:\\IOW.dat";
		HANDLE FileHandleW = CreateFile(pWFileName, GENERIC_READ | GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH | FILE_FLAG_NO_BUFFERING | FILE_ATTRIBUTE_NORMAL, NULL);
		//�������ļ�
		CHAR* pRFileName = "D:\\IOR.dat";
		HANDLE FileHandleR = CreateFile(pRFileName, GENERIC_READ | GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH | FILE_FLAG_NO_BUFFERING | FILE_ATTRIBUTE_NORMAL, NULL);
		SetFilePointer(FileHandleR, 0,0, FILE_BEGIN);
		DWORD Writed;
		WriteFile(FileHandleR, RBuffer, ulRSize+1024, &Writed, NULL);
		FlushFileBuffers(FileHandleR);

		while (1)
		{	
			DWORD Readed;
			DWORD dTime1 = GetTickCount();   
			SetFilePointer(FileHandleW, 0,0, FILE_BEGIN);
			WriteFile(FileHandleW, WBuffer, ulWSize, &Writed, NULL);
			FlushFileBuffers(FileHandleW);
			SetFilePointer(FileHandleR, 0,0, FILE_BEGIN);
			ReadFile(FileHandleR,RBuffer,ulRSize,&Readed,NULL);
			DWORD dTime2 = GetTickCount();
			if ((dTime2 - dTime1) < 1000)
			{
				Sleep(1000- (dTime2 - dTime1));
			}
			  
			if (m_bStop)
			{
				m_bStop = false;
				break;

			}
		}
		CloseHandle(FileHandleW);
		DeleteFile(pWFileName);
		CloseHandle(FileHandleR);
		DeleteFile(pRFileName);
	#else
		//TODO linux
		while(1)  
		{
			long timeuse = 0;    //ʵ��ѭ���õ�ʱ��
			struct timeval tpstart,tpend;
			timeuse = 0;
			gettimeofday(&tpstart,NULL); //�õ���ǰ��ϵͳʱ��
			rewind(fpW);//ʹָ��ص��ļ��Ŀ�ͷ
			int fd = open ("IOW.dat", O_WRONLY | O_CREAT | O_APPEND, 0660);
			int iWsize = strlen(WBuffer);
			write (fd, WBuffer, strlen (WBuffer));
			write (fd, "\n", 1);
			fsync (fd);
			close (fd);

			gettimeofday(&tpend,NULL);
			timeuse = 1000000 * (tpend.tv_sec - tpstart.tv_sec) + (tpend.tv_usec - tpstart.tv_usec);  
			//timeuse /= 1000;               //ת����ms
			if(timeuse < 1000000)
			{
				usleep(1000000 - timeuse);
			}
			if(remove("IOW.dat"))
			{
// 				char pLog[MAX_LOG_LEN];
// 				LOG(pLog, " remove  IOW.dat error");
				//�ϱ���־
			}
			if (true == m_bStop)
			{
				break;
			}

		}

#endif
	free(WBuffer);
	free(RBuffer);
	}
	return iRet;
	//m_QM.unlock();
}
void CIoUsage::StopThread()
{
	m_bStop = true;
}
char * CIoUsage::GetRandomBuffer(unsigned long Len)
{
    char *Buffer = (char *)malloc(Len);
	if (Buffer==0)
	{
		return 0;
	}

	for (unsigned long i = 0; i < Len; i++)
	{
        Buffer[i] = '1';//rand()%256;
	}

	return Buffer;
}

