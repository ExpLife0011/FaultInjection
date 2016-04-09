#ifndef COMMONDEF_H_
#define COMMONDEF_H_
typedef struct HostSimPar
{
	std::string sJobName;         //�����������
	time_t tvBeginTime;    //ģ�⿪ʼʱ��
	time_t tvEndTime;      //ģ�����ʱ��  ����ʱ�������
	bool bIsCPUSim;               //�Ƿ�CPUģ��
	bool bIsMemSim;               //�Ƿ�Memģ��
	bool bIsIOSim;                //�Ƿ�IOģ��
	int iCPUUsage;                //CPU�趨ֵ
	int iMemUsage;                //�ڴ��趨ֵ
	int iIORSim;                   //Ӳ�̶��趨ֵ
	int iIOWSim;                   //Ӳ��д�趨ֵ
}HostSimPar;
#endif