#include "UsageManager.h"
#include "ace/OS.h"
#include "CpuUsage.h"
#include "MemUsage.h"
#include "IoUsage.h"
//USAGEMANAGER_API CUsageManager* g_pUsageManager = NULL;  //��ʼ��ȫ�ֱ���
CUsageManager::CUsageManager(void)
{
 	m_pCpuUsage = NULL;
 	m_pCpuUsage = new CCpuUsage;
	m_pMemUsage = NULL;
	m_pMemUsage = new CMemUsage;
	m_IoUsage = NULL;
	m_IoUsage = new CIoUsage;
	m_vecHostCpuSimPar.clear();
	m_vecHostMemSimPar.clear();
	m_vecHostIOSimPar.clear();
	m_bCpuExeJob = false;
	m_bMemExeJob = false;
	m_bIOExeJob = false;
	m_bIsStop = false;
	activate();
}


CUsageManager::~CUsageManager(void)
{
	if (m_pCpuUsage != NULL)
	{
		delete m_pCpuUsage;
		m_pCpuUsage = NULL;
	}

	if (m_pMemUsage != NULL)
	{
		delete m_pMemUsage;
		m_pMemUsage = NULL;
	}

	if (m_IoUsage != NULL)
	{
		delete m_IoUsage;
		m_IoUsage = NULL;
	}
	m_vecHostCpuSimPar.clear();
	m_vecHostMemSimPar.clear();
	m_vecHostIOSimPar.clear();
}

int CUsageManager::SetHostCpuSimParm(HostCpuSimPar & newSimPar)
{
	int iRet = 0; //0��ʾ�ɹ�
	mtxThread.acquire();
	//�˴�Ŀǰ�ϸ�����ֻ����һ����ʼģ��������ִ�У�����Ĵ����߼��Ƚϸ��ӣ�ֻ�Ƕ��ģ�������ʱ������
	if (m_vecHostCpuSimPar.size() > 0)   //��ǰ�������
	{
		if (newSimPar.bIsCPUSim == true)
		{
			iRet = -1;   //��ǰ�п�ʼģ��������
			mtxThread.release();
			return iRet;
		}
	}
	////�����ȡ��ģ�⣬ֱ������m_bIsStopΪtrue
	if (newSimPar.bIsCPUSim == false)   //�����ȡ��ģ��
	{
		m_bIsStop = true;
		m_vecHostCpuSimPar.clear();
		mtxThread.release();
		return iRet;
	}


	if (newSimPar.bIsCPUSim == true && newSimPar.tvBeginTime >= newSimPar.tvEndTime)  //�����ʱ�����
	{
		//TODO  �ϱ�������־
		iRet = -2;
		mtxThread.release();
		return iRet;
	}

	
	//��ǰ����ִ�е������Ҳ������m_vecHostSimPar��
	if (m_vecHostCpuSimPar.size() == 0)   //һ�������Ҳû��ʱ��ֱ�����
	{
		m_vecHostCpuSimPar.push_back(newSimPar);
	}
	else  //�������
	{
		HostCpuSimPar hostsimPar = m_vecHostCpuSimPar[0];

		//������������ִ�У��ж���������Ƿ������ִ�е��������ͻ��ʱ�䣩
		bool bIsExecute = false;
		if (m_bCpuExeJob == true)  // �Ѵ��ڵ���������ִ��
		{
			bIsExecute = true;
			if (newSimPar.tvBeginTime < hostsimPar.tvEndTime)   //������Ŀ�ʼʱ��С������ִ������Ľ���ʱ��
			{
				//TODO  �ϱ�������־
				iRet = -2;
				mtxThread.release();
				return iRet;
			}
		}

		int iStartIndex = 0;   //����������������������ȽϵĿ�ʼ����
		if (bIsExecute == true)  //�������ִ�У����Ҳ���ͻ���������ֻ��ͺ����������Ƚϼ���
		{
			iStartIndex = 1;
		}

		for (int iIndex = iStartIndex;iIndex < m_vecHostCpuSimPar.size();iIndex++)
		{
			HostCpuSimPar hostsimPar = m_vecHostCpuSimPar[iIndex];
			if (iIndex == iStartIndex)
			{
				if (newSimPar.tvEndTime <= hostsimPar.tvBeginTime) //����Ҫ��
				{
					m_vecHostCpuSimPar.insert(m_vecHostCpuSimPar.begin() + iIndex,newSimPar);
					iRet = 0;
					break;
				}
			}
			else if (iIndex == m_vecHostCpuSimPar.size() - 1)
			{
				if (newSimPar.tvBeginTime >= hostsimPar.tvEndTime) //����Ҫ��
				{
					m_vecHostCpuSimPar.insert(m_vecHostCpuSimPar.begin() + iIndex,newSimPar);
					iRet = 0;
					break;
				}
			}
			else
			{
				HostCpuSimPar proHostsimPar = m_vecHostCpuSimPar[iIndex - 1];
				if (newSimPar.tvBeginTime >= proHostsimPar.tvEndTime && 
					newSimPar.tvEndTime <= hostsimPar.tvBeginTime) //����Ҫ��
				{
					m_vecHostCpuSimPar.insert(m_vecHostCpuSimPar.begin() + iIndex,newSimPar);
					iRet = 0;
					break;
				}
			}
		}
		if (iRet != 0)
		{
			//TODO  �ϱ�������־
		}
	}
	mtxThread.release();
	return iRet;
}

int CUsageManager::SetHostMemSimParm(HostMemSimPar & newSimPar)
{
	int iRet = 0; //0��ʾ�ɹ�
	mtxThread.acquire();
	//�˴�Ŀǰ�ϸ�����ֻ����һ����ʼģ��������ִ�У�����Ĵ����߼��Ƚϸ��ӣ�ֻ�Ƕ��ģ�������ʱ������
	if (m_vecHostMemSimPar.size() > 0)   //��ǰ�������
	{
		if (newSimPar.bIsMemSim == true)
		{
			iRet = -1;   //��ǰ�п�ʼģ��������
			mtxThread.release();
			return iRet;
		}
	}
	////�����ȡ��ģ�⣬ֱ������m_bIsStopΪtrue
	if (newSimPar.bIsMemSim == false)   //�����ȡ��ģ��
	{
		m_bIsStop = true;
		m_vecHostMemSimPar.clear();
		mtxThread.release();
		return iRet;
	}

	if (newSimPar.tvBeginTime >= newSimPar.tvEndTime)  //�����ʱ�����
	{
		//TODO  �ϱ�������־
		mtxThread.release();
		return iRet;
	}
	//��ǰ����ִ�е������Ҳ������m_vecHostSimPar��
	if (m_vecHostMemSimPar.size() == 0)   //һ�������Ҳû��ʱ��ֱ�����
	{
		m_vecHostMemSimPar.push_back(newSimPar);
	}
	else  //�������
	{
		HostMemSimPar hostsimPar = m_vecHostMemSimPar[0];

		//������������ִ�У��ж���������Ƿ������ִ�е��������ͻ��ʱ�䣩
		bool bIsExecute = false;
		if (m_bMemExeJob == true)  // �Ѵ��ڵ���������ִ��
		{
			bIsExecute = true;
			if (newSimPar.tvBeginTime < hostsimPar.tvEndTime)   //������Ŀ�ʼʱ��С������ִ������Ľ���ʱ��
			{
				//TODO  �ϱ�������־
				mtxThread.release();
				return iRet;
			}
		}

		int iStartIndex = 0;   //����������������������ȽϵĿ�ʼ����
		if (bIsExecute == true)  //�������ִ�У����Ҳ���ͻ���������ֻ��ͺ����������Ƚϼ���
		{
			iStartIndex = 1;
		}

		for (int iIndex = iStartIndex;iIndex < m_vecHostMemSimPar.size();iIndex++)
		{
			HostMemSimPar hostsimPar = m_vecHostMemSimPar[iIndex];
			if (iIndex == iStartIndex)
			{
				if (newSimPar.tvEndTime <= hostsimPar.tvBeginTime) //����Ҫ��
				{
					m_vecHostMemSimPar.insert(m_vecHostMemSimPar.begin() + iIndex,newSimPar);
					iRet = 0;
					break;
				}
			}
			else if (iIndex == m_vecHostMemSimPar.size() - 1)
			{
				if (newSimPar.tvBeginTime >= hostsimPar.tvEndTime) //����Ҫ��
				{
					m_vecHostMemSimPar.insert(m_vecHostMemSimPar.begin() + iIndex,newSimPar);
					iRet = 0;
					break;
				}
			}
			else
			{
				HostMemSimPar proHostsimPar = m_vecHostMemSimPar[iIndex - 1];
				if (newSimPar.tvBeginTime >= proHostsimPar.tvEndTime && 
					newSimPar.tvEndTime <= hostsimPar.tvBeginTime) //����Ҫ��
				{
					m_vecHostMemSimPar.insert(m_vecHostMemSimPar.begin() + iIndex,newSimPar);
					iRet = 0;
					break;
				}
			}
		}
		if (iRet != 0)
		{
			//TODO  �ϱ�������־
		}
	}
	mtxThread.release();
	return iRet;
}


int CUsageManager::SetHostIOSimParm(HostIOSimPar & newSimPar)
{
	int iRet = 0; //0��ʾ�ɹ�
	mtxThread.acquire();
	//�˴�Ŀǰ�ϸ�����ֻ����һ����ʼģ��������ִ�У�����Ĵ����߼��Ƚϸ��ӣ�ֻ�Ƕ��ģ�������ʱ������
	if (m_vecHostIOSimPar.size() > 0)   //��ǰ�������
	{
		if (newSimPar.bIsIOSim == true)
		{
			iRet = -1;   //��ǰ�п�ʼģ��������
			mtxThread.release();
			return iRet;
		}
	}

	////�����ȡ��ģ�⣬ֱ������m_bIsStopΪtrue
	if (newSimPar.bIsIOSim == false)   //�����ȡ��ģ��
	{
		m_bIsStop = true;
		m_vecHostIOSimPar.clear();
		mtxThread.release();
		return iRet;
	}


	if (newSimPar.tvBeginTime >= newSimPar.tvEndTime)  //�����ʱ�����
	{
		//TODO  �ϱ�������־
		mtxThread.release();
		return iRet;
	}
	//��ǰ����ִ�е������Ҳ������m_vecHostSimPar��
	if (m_vecHostIOSimPar.size() == 0)   //һ�������Ҳû��ʱ��ֱ�����
	{
		m_vecHostIOSimPar.push_back(newSimPar);
	}
	else  //�������
	{
		HostIOSimPar hostsimPar = m_vecHostIOSimPar[0];

		//������������ִ�У��ж���������Ƿ������ִ�е��������ͻ��ʱ�䣩
		bool bIsExecute = false;
		if (m_bIOExeJob == true)  // �Ѵ��ڵ���������ִ��
		{
			bIsExecute = true;
			if (newSimPar.tvBeginTime < hostsimPar.tvEndTime)   //������Ŀ�ʼʱ��С������ִ������Ľ���ʱ��
			{
				//TODO  �ϱ�������־
				mtxThread.release();
				return iRet;
			}
		}

		int iStartIndex = 0;   //����������������������ȽϵĿ�ʼ����
		if (bIsExecute == true)  //�������ִ�У����Ҳ���ͻ���������ֻ��ͺ����������Ƚϼ���
		{
			iStartIndex = 1;
		}

		for (int iIndex = iStartIndex;iIndex < m_vecHostMemSimPar.size();iIndex++)
		{
			HostIOSimPar hostsimPar = m_vecHostIOSimPar[iIndex];
			if (iIndex == iStartIndex)
			{
				if (newSimPar.tvEndTime <= hostsimPar.tvBeginTime) //����Ҫ��
				{
					m_vecHostIOSimPar.insert(m_vecHostIOSimPar.begin() + iIndex,newSimPar);
					iRet = 0;
					break;
				}
			}
			else if (iIndex == m_vecHostIOSimPar.size() - 1)
			{
				if (newSimPar.tvBeginTime >= hostsimPar.tvEndTime) //����Ҫ��
				{
					m_vecHostIOSimPar.insert(m_vecHostIOSimPar.begin() + iIndex,newSimPar);
					iRet = 0;
					break;
				}
			}
			else
			{
				HostIOSimPar proHostsimPar = m_vecHostIOSimPar[iIndex - 1];
				if (newSimPar.tvBeginTime >= proHostsimPar.tvEndTime && 
					newSimPar.tvEndTime <= hostsimPar.tvBeginTime) //����Ҫ��
				{
					m_vecHostIOSimPar.insert(m_vecHostIOSimPar.begin() + iIndex,newSimPar);
					iRet = 0;
					break;
				}
			}
		}
		if (iRet != 0)
		{
			//TODO  �ϱ�������־
		}
	}
	mtxThread.release();
	return iRet;
}

int CUsageManager::svc(void)   //�߳�ִ����
{
	int iRet = 0;
	while (1)
	{
		printf("CUsageManager::svc");
		if (m_bIsStop == true)
		{
			break;  //�˳�ѭ��
		}
		mtxThread.acquire();
		if (m_vecHostCpuSimPar.size() > 0)
		{
			HostCpuSimPar hostCpuSimPar = m_vecHostCpuSimPar[0];
			time_t tvCurrentTime = time(NULL);
			//��������ж��Ƿ���ֹͣʱ��
			if (m_bCpuExeJob == true)
			{
				if (tvCurrentTime >= hostCpuSimPar.tvEndTime)  //���������
				{
					//TODO �ϱ�����xxx�������
					m_bCpuExeJob = false;
					SetCpuUsage(false,0);
					m_vecHostCpuSimPar.erase(m_vecHostCpuSimPar.begin());
				}
			}
			else  //û��������ж��Ƿ�ִ�е�һ�������
			{
				if (hostCpuSimPar.tvBeginTime <= tvCurrentTime)// && hostsimPar.tvEndTime > tvCurrentTime)
				{
					//TODO �ϱ�����xxx����ʼ
					m_bCpuExeJob = true;
					SetCpuUsage(hostCpuSimPar.bIsCPUSim,hostCpuSimPar.iCPUUsage);
				}
			}
		}

		if (m_vecHostMemSimPar.size() > 0)
		{
			HostMemSimPar hostMemSimPar = m_vecHostMemSimPar[0];
			time_t tvCurrentTime = time(NULL);
			//��������ж��Ƿ���ֹͣʱ��
			if (m_bMemExeJob == true)
			{
				if (tvCurrentTime >= hostMemSimPar.tvEndTime)  //���������
				{
					//TODO �ϱ�����xxx�������
					m_bMemExeJob = false;
					SetRamUsage(false,0,0);
					m_vecHostMemSimPar.erase(m_vecHostMemSimPar.begin());
				}
			}
			else  //û��������ж��Ƿ�ִ�е�һ�������
			{
				if (hostMemSimPar.tvBeginTime <= tvCurrentTime)// && hostsimPar.tvEndTime > tvCurrentTime)
				{
					//TODO �ϱ�����xxx����ʼ
					m_bMemExeJob = true;
					SetRamUsage(hostMemSimPar.bIsMemSim,hostMemSimPar.iMemUsage,0);
				}
			}
		}

		if (m_vecHostIOSimPar.size() > 0)
		{
			HostIOSimPar hostIOSimPar = m_vecHostIOSimPar[0];
			time_t tvCurrentTime = time(NULL);
			//��������ж��Ƿ���ֹͣʱ��
			if (m_bIOExeJob == true)
			{
				if (tvCurrentTime >= hostIOSimPar.tvEndTime)  //���������
				{
					//TODO �ϱ�����xxx�������
					m_bIOExeJob = false;
					SetIoCount(false,0,0);
					m_vecHostIOSimPar.erase(m_vecHostIOSimPar.begin());
				}
			}
			else  //û��������ж��Ƿ�ִ�е�һ�������
			{
				if (hostIOSimPar.tvBeginTime <= tvCurrentTime)// && hostsimPar.tvEndTime > tvCurrentTime)
				{
					//TODO �ϱ�����xxx����ʼ
					m_bIOExeJob = true;
					SetIoCount(hostIOSimPar.bIsIOSim,hostIOSimPar.iIOWSim,hostIOSimPar.iIOWSim);
				}
			}
		}
		mtxThread.release();
		ACE_OS::sleep(1);
	}
	return iRet;
}

void CUsageManager::StopThread()
{
	m_bIsStop = true;
}
int CUsageManager::SetCpuUsage(bool bIsSet,float fCPU)
{
	int iRet = 0;
	if (m_pCpuUsage != NULL)
	{
		iRet = m_pCpuUsage->SetCpuUsage1(bIsSet,fCPU);
	}
	else
	{
		iRet = -1;
	}
	return iRet;
}
int CUsageManager::SetRamUsage(bool bIsSet,unsigned long ulUsage,unsigned long ulVirUsage)
{
	int iRet = 0;
	if (m_pMemUsage != NULL)
	{
		iRet = m_pMemUsage->SetMemUsage1(bIsSet,ulUsage,ulVirUsage);
	}
	else
	{
		iRet = -1;
	}
	return iRet;
}
int CUsageManager::SetIoUsage(bool bIsSet,unsigned long ulUsage)
{
	int iRet = 0;
	if (m_IoUsage != NULL)
	{
		iRet = m_IoUsage->SetIoUsage1(bIsSet,ulUsage);
	}
	else
	{
		iRet = -1;
	}
	return iRet;
}
int CUsageManager::SetIoCount(bool bIsSet,unsigned long ulIOCount,unsigned long ulRIOCount)
{
	int iRet = 0;
	if (m_IoUsage != NULL)
	{
		iRet = m_IoUsage->SetIoCount1(bIsSet,ulIOCount,ulRIOCount);
	}
    double CPUUsage;
    double MemUsage;
    unsigned long IOKByte;
    GetSysResUseage(CPUUsage,MemUsage,IOKByte);
	return iRet;
}
int CUsageManager::GetSysResUseage(double &CPUUsage,double &MemUsage,unsigned long &IOKByte)
{
	int iRet = 0;
	if (m_pCpuUsage != NULL && m_pMemUsage != NULL && m_IoUsage != NULL)
	{
		CPUUsage = m_pCpuUsage->GetCpuUsage1();
		MemUsage = m_pMemUsage->GetRamUsage1();
		IOKByte = m_IoUsage->GetIoCount1();
	}
	else
	{
		iRet = -1;
	}
	return iRet;
}
