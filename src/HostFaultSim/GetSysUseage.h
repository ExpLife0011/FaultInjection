#ifndef GETSYSUSAGE_H 
#define GETSYSUSAGE_H
#include "ace/Task.h"
#include "ace/OS.h"
#include "ConstDef.h"

//#ifdef _WINDOWS
//#include "Qt\QThread.h"
//#include "Qt\qmutex.h"
//#else
//#include <qthread.h>
//#include <qmutex.h>
//#endif
//class QMutex;
class CGetSysUseage: public ACE_Task_Base
{
public:
	CGetSysUseage(void);
	~CGetSysUseage(void);
	void GetSysUseage(double &CPUUsage,double &MemUsage,unsigned long &IOKByte);
private:
	//QMutex m_QM;
	double m_CPUUsage;   //CPUʹ����
	double m_MemUsage;   //�ڴ�ʹ����
	unsigned long m_IOKByte;    //IO��д�ֽ���ÿs
	bool m_bStop;
	//void run();

	/*!
	*	\brief	�߳�ִ����
	*/
	int svc(void);

	void StopThread();

};
#endif
