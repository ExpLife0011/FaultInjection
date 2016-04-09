
#ifndef _DBFAULTSIM_H_
#define _DBFAULTSIM_H_

#include "ace/Task.h"
#include "ace/Thread_Mutex.h"
#include "DBFaultSimPublic.h"
#include <string>

class CDBAccessBase;
class DBFAULTSIM_API CDBFaultSim
{
private:
	CDBFaultSim(void);

public:
	static CDBFaultSim* GetInstance();
	virtual ~CDBFaultSim(void);

	int SetDBType(const char* pchDBType );

	int StartListener( const char* pchListener = NULL );
	int StopListener( const char* pchListener = NULL  );

	int StartService( const char* pchServiceName = NULL );
	int StopService( const char* pchServiceName = NULL  );

	int Connect( const char* pchConnString, int iConnCnt = 1 );
	int DisConnect();

	int ChangePWD( const char* pConnString, const char* pchUserName, const char* pchOldPWD, const char* pchNewPWD);
	int RecoverPWD( const char* pchUserName , const char* pchOldPWD );

	int FillTableSpace(const char* pchTabSpaceName);
	int FreeTableSpace(const char* pchTabSpaceName);

private:

	static CDBFaultSim*		sm_pSingleton;
	static ACE_Thread_Mutex	sm_Mutex ;

	std::string m_strDBType;

	CDBAccessBase* m_pDBAccess;

};


class DBFAULTSIM_API CTestTask : public ACE_Task_Base
{
public:
	/*!
	*	\brief	ȱʡ���캯��
	*/
	CTestTask(void);

	/*!
	*	\brief	ȱʡ��������
	*/
	virtual ~CTestTask(void);

	/*!
	*	\brief	��ȡ�ռ������Ƿ�ֹͣ����
	*
	*	\retval bool	ֹͣ���б�־,true:ֹͣ, false ����
	*/
	bool done (void)
	{
		return m_bShutDown;
	}

	/*!
	*	\brief	�߳��˳�
	*/
	void shutdown()
	{
		m_bShutDown = true;
		//!�ȴ��߳��˳�
// 		while(this->thr_count() >0 )
// 			ACE_OS::sleep(ACE_Time_Value(0,100*1000));
	}

	/*!
	*	\brief	���ݱ�����ݣ��ж��Ƿ�����ռ�
	*/
	//bool run(void);

	virtual int svc (void)
	{
		return 0;
	}



	//ֹͣ���б�־,true:ֹͣ, false ����
	bool m_bShutDown;

private:
	
	
	
};

#endif //!_DBFAULTSIM_H_
