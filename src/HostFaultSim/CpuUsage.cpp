#include "CpuUsage.h"
//#include<iostream>  
#include<cmath>
#ifndef _WINDOWS
typedef struct CPUThreadPar
{
    CCpuUsage * pCpuUsage;
    int iCpuNo;
}CPUThreadPar;

CPUThreadPar CpuThreadParStruct[32];
#endif
CCpuUsage::CCpuUsage()
{
	m_pGetCpuUseage = NULL;
	m_pGetCpuUseage = new CGetCpuUseage;

#ifndef _WINDOWS
    for(int i=0;i<32;i++)
    {
        CpuThreadParStruct[i].pCpuUsage = NULL;
    }
#endif
}
CCpuUsage::~CCpuUsage()
{
	if (m_pGetCpuUseage != NULL)
	{
		delete m_pGetCpuUseage;
		m_pGetCpuUseage = NULL;
	}
}
float CCpuUsage::GetCpuUsage1()
{
	float fCurCpuUsage = 0;
	if (m_pGetCpuUseage)
	{
		fCurCpuUsage = m_pGetCpuUseage->GetCpuUsage(); //��ǰCPUʹ����
	}
	return fCurCpuUsage;
}
int CCpuUsage::SetCpuUsage1(bool bIsSet,double dCpu)
{
	int iRet = 0;
	double f100 = 100;
	m_dRatio = dCpu/f100;
	if (bIsSet == true)
	{
		float fCurCpuUsage = m_pGetCpuUseage->GetCpuUsage(); //��ǰCPUʹ����
		if (fCurCpuUsage > dCpu)
		{
			iRet = -1;  //��ǰ��CPUʹ���ʴ�������ֵ
// 			char pLog[MAX_LOG_LEN];
// 			LOG(pLog, "SetCpuUsage setval %.2f < curval %.2f", dCpu, fCurCpuUsage);
			return iRet;
		}
        m_bIsStop = false;
#ifdef _WINDOWS
		SYSTEM_INFO info;  
		GetSystemInfo(&info);   //�õ�cpu��Ŀ  
		m_iCpuNum = info.dwNumberOfProcessors; 
		Run();
#else
		// sysconf() ����ѡ��(����)�ĵ�ǰֵ��
		// ���ֵ�����õĵ�Ҳ����ϵͳ���Ƶġ�
		// _SC_NPROCESSORS_CONF ��ֵΪ CPU ������������(0)��ʼ���
		// CPU ��ŷ�Χ��0 �� ( sysconf( _SC_NPROCESSORS_CONF ) - 1 )
		//uiCPUTotal = sysconf( _SC_NPROCESSORS_CONF ); 
		m_iCpuNum = sysconf( _SC_NPROCESSORS_CONF );
		printf( "Notice: current system has %i CPU(s). ~ thread: %lu\n", m_iCpuNum, pthread_self() );
        Run_Linux();
#endif

		
	}
	else
	{
		m_bIsStop = true; 
	}

	return iRet;
}
#ifdef _WINDOWS
void CCpuUsage::Run()  
{   
	HANDLE handle[32];    
	DWORD thread_id[32]; //�߳�id  

	for (int i = 0; i < m_iCpuNum; i++)  
	{  
		if ((handle[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc,   
			this, 0, &thread_id[i])) != NULL)  //�������߳�  
			SetThreadAffinityMask(handle[i], i+1); //�޶��߳��������ĸ�cpu��  
	}  

}   
DWORD WINAPI CCpuUsage::ThreadProc (LPVOID pParam)
{
	CCpuUsage* ThisObj = (CCpuUsage*)pParam;
	if (ThisObj == NULL)
	{
		return 0;
	}
	unsigned BUSY =  ThisObj->m_dRatio * GAP_LINEAR;  
	unsigned IDLE = (1 - ThisObj->m_dRatio) * GAP_LINEAR;  
	while(1)  
	{  
 		if (true == ThisObj->m_bIsStop)
 		{
 			break;
 		}
		unsigned ta = GetTickCount(); //����ϵͳ������������������ʱ��ms;  
		while(GetTickCount() - ta < BUSY); 
		Sleep(IDLE);
	}
	return 0;
}
#else

void CCpuUsage::Run_Linux()
{
	for (int i = 0; i < m_iCpuNum; i++)  
	{
		pthread_t sThreadID; 
        CpuThreadParStruct[i].pCpuUsage = this;
        CpuThreadParStruct[i].iCpuNo = i;
		if ( 0 != pthread_create( &sThreadID,       // �߳�ID
			NULL,             // �߳����� - NULL ʹ��ȱ��ֵ
            ThreadProc_Linux, // �̺߳�����ַ
            &CpuThreadParStruct[i] ) )   // �̺߳������� - ָ���߳����ĸ� CPU
		{
// 			char pLog[MAX_LOG_LEN];
// 			LOG(pLog, "create %d CpuThread  error",i+1);
			//printf( "!!! Error: create thread failed !!!\n" ); 
		}
        /*else
		{
			pthread_join( sThreadID, // �߳�ID
				NULL );    // �̵߳ķ���ֵ - NULL ����÷���ֵ
        }*/
	}
}
// gcc main.c -o runatcpu -lpthread
void * CCpuUsage::ThreadProc_Linux( void* pvParameter )
{
    CPUThreadPar pTempPar = *(CPUThreadPar *)pvParameter;
    unsigned int uiWhichCPU = pTempPar.iCpuNo;
    CCpuUsage * pThis = pTempPar.pCpuUsage;
	//unsigned int uiCPUTotal = 0;
	unsigned int j          = 0; 
	unsigned int i          = 0;

	// CPU �׺���(CPU Affinity)�ĸ��
	// CPU �׺�������ָ�� Linux ϵͳ���ܹ���һ���������̰󶨵�һ�����������������С�
	// ���̿���ͨ�� CPU ���Ͼ��������ĸ����ļ��� CPU �����С�
	// cpu_set_t �ṹ������ʾһ�� CPU ���ϡ�
	cpu_set_t sCPUSet;

	//// sysconf() ����ѡ��(����)�ĵ�ǰֵ��
	//// ���ֵ�����õĵ�Ҳ����ϵͳ���Ƶġ�
	//// _SC_NPROCESSORS_CONF ��ֵΪ CPU ������������(0)��ʼ���
	//// CPU ��ŷ�Χ��0 �� ( sysconf( _SC_NPROCESSORS_CONF ) - 1 )
	//uiCPUTotal = sysconf( _SC_NPROCESSORS_CONF );   
	//printf( "Notice: current system has %i CPU(s). ~ thread: %lu\n", 
	//	uiCPUTotal, 
	//	pthread_self() );

	// ���һ�� CPU ����
	CPU_ZERO( &sCPUSet );
	// ��һ�������� CPU �����һ�� CPU ����������
	CPU_SET( uiWhichCPU, &sCPUSet );
	// ��һ�������� CPU ��Ŵ�һ�� CPU �������Ƴ�
	// CPU_CLR( uiWhichCPU, &sCPUSet );

	// ��ָ�� �߳�ID ��ָ���� CPU
	if ( -1 == pthread_setaffinity_np( pthread_self(),    // �߳�ID
		sizeof( sCPUSet ), // CPU ���Ͻṹ���С
		&sCPUSet ) )       // CPU ���Ͻṹ�����ָ��
	{
		printf( "!!! Error: bind current thread to specified CPU failed !!! ~ thread: %lu\n", 
			pthread_self() );
	}
	else
	{
		int busyTime = 10;   //æ��ʱ��
		int idleTime = 0;    //����ʱ��
		long timeuse = 0;    //ʵ��ѭ���õ�ʱ��
		//int cpucoe = 0;      //CPUռ����
		struct timeval tpstart,tpend; 
		while(1)  
		{  
            if (true == pThis->m_bIsStop)
			{
				break;
			}
			gettimeofday(&tpstart,NULL); //�õ���ǰ��ϵͳʱ��
			while (timeuse <= busyTime)
			{
				gettimeofday(&tpend,NULL);
				timeuse = 1000000 * (tpend.tv_sec - tpstart.tv_sec) + (tpend.tv_usec - tpstart.tv_usec);  
				timeuse /= 1000;               //ת����ms                
			}

            idleTime = ((100 * busyTime) / pThis->m_dRatio) - busyTime;

			sleep(idleTime / 1000);    //ת����ms
		}
	}
}


#endif


/*

#include<iostream>
#include<cmath>
#include<windows.h>

static int PERIOD = 60 * 1000; //����ms
const int COUNT = 300;  //һ�����ڼ������
const double GAP_LINEAR = 100;  //���Ժ���ʱ����100ms
const double PI = 3.1415926535898; //PI
const double GAP = (double)PERIOD / COUNT; //���ں���ʱ����
const double FACTOR = 2 * PI / PERIOD;  //���ں�����ϵ��
static double Ratio = 0.5;  //���Ժ�����ֵ 0.5��50%
static double Max=0.9; //�������������ֵ
static double Min=0.1; //������������Сֵ

typedef double Func(double);  //����һ���������� Func*Ϊ����ָ��
typedef void Solve(Func *calc);//���庯�����ͣ�����Ϊ����ָ��Func*
inline DWORD get_time() 
{ 
	return GetTickCount(); //����ϵͳ������������������ʱ��ms
}
double calc_sin(double x)  //�������ں���solve_period�Ĳ���
{  
	return (1 + sin(FACTOR * x)) / 2; //y=1/2(1+sin(a*x))
}
double calc_fangbo(double x)  //�������ں���solve_period�Ĳ���
{
	//��������
	if(x<=PERIOD/2) return Max;
	else return Min;
}

void solve_period(Func *calc) //�̺߳���Ϊ���ں���
{
	double x = 0.0;
	double cache[COUNT];
	for (int i = 0; i < COUNT; ++i, x += GAP) 
		cache[i] = calc(x); 
	int count = 0;
	while(1)
	{
		unsigned ta = get_time();
		if (count >= COUNT) count = 0;
		double r = cache[count++];
		DWORD busy = r * GAP;
		while(get_time() - ta < busy) {}
		Sleep(GAP - busy);
  }
}

void solve_linear(Func*)  //�̺߳���Ϊ���Ժ���������Ϊ�� NULL
{
	const unsigned BUSY =  Ratio * GAP_LINEAR;
	const unsigned IDLE = (1 - Ratio) * GAP_LINEAR;
	while(1)
	{
		unsigned ta = get_time();
		while(get_time() - ta < BUSY) {}
		Sleep(IDLE);
	}
}
//void solve_nonperiod(Func *calc) //�����ں����Ĵ�����ûʵ��
//{
//  double tb = 0;
//  while(1)
//  {
//    unsigned ta = get_time();
//    double r = calc(tb);
//    if (r < 0 || r > 1) r = 1;
//    DWORD busy = r * GAP;
//    while(get_time() - ta < busy) {}
//    Sleep(GAP - busy);
//    //tb += GAP;
//    tb += get_time() - ta;
//  }
//}

void run(int i=1,double R=0.5,double T=60000,double max=0.9,double min=0.1)
     //iΪ���״̬��RΪֱ�ߺ�����ֵ��TΪ���ں��������ڣ�max�������ֵ��min������Сֵ
{
	Ratio=R; PERIOD=T; Max=max; Min=min;
	Func *func[] = {NULL ,calc_sin,calc_fangbo};  //����Solve�Ĳ���������ָ������
	Solve *solve_func[] = { solve_linear, solve_period};  //Solve����ָ������
	const int NUM_CPUS = 2;  //˫�ˣ�ͨ�õĿ���������GetSystemInfo�õ�cpu��Ŀ
	HANDLE handle[NUM_CPUS];  
	DWORD thread_id[NUM_CPUS]; //�߳�id
	//SYSTEM_INFO info;
	//GetSystemInfo(&info);   //�õ�cpu��Ŀ
	//const int num = info.dwNumberOfProcessors;
	switch(i)
	{
	case 1: //cpu1 ,cpu2�����ֱ��
		{
			for (int i = 0; i < NUM_CPUS; ++i)
			{
				Func *calc = func[0];
				Solve *solve = solve_func[0];
				if ((handle[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)solve, 
									(VOID*)calc, 0, &thread_id[i])) != NULL)  //�������߳�
				SetThreadAffinityMask(handle[i], i+1); //�޶��߳��������ĸ�cpu��
			}
			WaitForSingleObject(handle[0],INFINITE);   //�ȴ��߳̽���
			break;
		}
	case 2: //cpu1ֱ�ߣ�cpu2����
		{
			for (int i = 0; i < NUM_CPUS; ++i)
			{
				Func *calc = func[i];
				Solve *solve = solve_func[i];
				if ((handle[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)solve, 
									(VOID*)calc, 0, &thread_id[i])) != NULL)  //�������߳�
				SetThreadAffinityMask(handle[i], i+1); //�޶��߳��������ĸ�cpu��
			}
			WaitForSingleObject(handle[0],INFINITE);   //�ȴ��߳̽���
			break;
		}
		case 3: //cpu1ֱ�ߣ�cpu2����
		{

			//Func *calc = func[0];
			//Solve *solve = solve_func[0];
			if ((handle[0] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)solve_func[0], 
								(VOID*)func[0], 0, &thread_id[0])) != NULL)  //�������߳�
			SetThreadAffinityMask(handle[0], 1); //�޶��߳��������ĸ�cpu��
			Func *calc = func[2];
			Solve *solve = solve_func[1];
			if ((handle[1] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)solve, 
								(VOID*)calc, 0, &thread_id[1])) != NULL)  //�������߳�
			SetThreadAffinityMask(handle[1], 2); //�޶��߳��������ĸ�cpu��
			WaitForSingleObject(handle[0],INFINITE);   //�ȴ��߳̽���
			break;
		}
		case 4: //cpu1���ң�cpu2����
		{
			if ((handle[0] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)solve_func[1], 
								(VOID*)func[1], 0, &thread_id[0])) != NULL)  //�������߳�
			SetThreadAffinityMask(handle[0], 1); //�޶��߳��������ĸ�cpu��
			Func *calc = func[2];
			Solve *solve = solve_func[1];
			if ((handle[1] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)solve, 
								(VOID*)calc, 0, &thread_id[1])) != NULL)  //�������߳�
			SetThreadAffinityMask(handle[1], 2); //�޶��߳��������ĸ�cpu��
			WaitForSingleObject(handle[0],INFINITE);   //�ȴ��߳̽���
			break;
		}
		default: break;
	}
}

void main()
{
	run(1,0.5);  //cpu1 ,cpu2�����50%��ֱ��
	//run(2,0.5,30000); //cpu1 0.5ֱ�ߣ�cpu2��������30000
	//run(3);  //cpu1ֱ�ߣ�cpu2����
	//run(4,0.8,30000,0.95,0.5); //cpu1���ң�cpu2 0.95-0.5�ķ���
}*/