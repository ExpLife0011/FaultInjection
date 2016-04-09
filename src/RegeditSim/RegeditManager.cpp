#include "RegeditManager.h"
#define MAX_LOG_LEN 256

#ifndef WIN32
#define LOG(op, fmt, ...)   \
	sprintf(op,/*"%04d-%02d-%02d %02d:%02d:%02d    "*/": " fmt , \
	/*tf->tm_year, tf->tm_mon, tf->tm_yday, tf->tm_hour, tf->tm_min, tf->tm_sec,*/ ##__VA_ARGS__); //\
	//if(g_pFaultInjection) g_pFaultInjection->AddLog(op);
#else
#define LOG(op, fmt, ...)   \
	(sprintf(op,/*"%04d-%02d-%02d %02d:%02d:%02d    "*/__FUNCTION__ ": " fmt , \
	/*tf->tm_year, tf->tm_mon, tf->tm_yday, tf->tm_hour, tf->tm_min, tf->tm_sec,*/ ##__VA_ARGS__)); //\
	//if(g_pFaultInjection) g_pFaultInjection->AddLog(op);
#endif


extern REGSIM_API CRegeditManager* g_regeditManager = NULL;

CRegeditManager::CRegeditManager(void)
{
}


CRegeditManager::~CRegeditManager(void)
{
}

int CRegeditManager::RegeditOccupy(std::vector<RegItem>& vtReg)
{
	char pLog[MAX_LOG_LEN];
	int iRet = 0;
	for (int j = 0; j < vtReg.size(); j++)
	{
		HKEY hKey;  
		DWORD dwDisposition;  
		//��ע�����  
		LONG lRet = ::RegCreateKeyEx(
			vtReg[j].mainPath, 
			vtReg[j].strPath.c_str(), 
			NULL, NULL, REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS, 
			NULL, &hKey, &dwDisposition);  
		//LONG lRet = ::RegCreateKeyA(HKEY_LOCAL_MACHINE, lpSubKey, &hKey);  
		if ( ERROR_SUCCESS != lRet )  
		{
			LOG(pLog, "����ע����� %s ʧ��!!", vtReg[j].strPath.c_str() ); 
			::RegCloseKey(hKey);
			iRet = -1;
			break;
		}
		//else
		//{
		//	LOG(pLog, "RegCreateKeyEx %s Successed!!", listReg[*it]->strPath.toStdString().c_str() );
		//}

		//! type
		BYTE* pby = NULL;
		int iLength = 0;
		std::string strVal;
		DWORD dwVal;
		unsigned long long qwVal;
		switch ( vtReg[j].dwType )
		{
		case REG_SZ:
		case REG_MULTI_SZ:
		case REG_EXPAND_SZ: 
			strVal = vtReg[j].strValue;
			pby = (BYTE*)strVal.c_str();
			iLength = vtReg[j].strValue.length() + 1;
			break;
		case REG_DWORD:
			
			dwVal = atol(vtReg[j].strValue.c_str());
			iLength = sizeof(DWORD);
			break;
		case REG_QWORD:
			qwVal = atol(vtReg[j].strValue.c_str());
			iLength = sizeof(unsigned long long);
			break;		
		}

		lRet = ::RegSetValueEx( hKey,
			vtReg[j].strKey.c_str(), 
			NULL, 
			vtReg[j].dwType, 
			pby, 
			iLength ); //����ע�����  
		if ( ERROR_SUCCESS != lRet)  
		{  
			LOG(pLog, "����������ֵ %s ʧ��!!", vtReg[j].strKey.c_str() ); 
		}  
		else
		{
			LOG(pLog, "ע�����Դ %s ռ�óɹ�!!", vtReg[j].strKey.c_str() ); 
			//bRegs[*it] = true;
		}

		::RegCloseKey(hKey); //��RegCreateKeyEx���д  
	}

	return iRet;
}

int CRegeditManager::RegeditRelease(std::vector<RegItem>& vtReg)
{
	char pLog[MAX_LOG_LEN];
	int iRet = 0;
	for (int j = 0; j < vtReg.size(); j++)
	{
		HKEY hKey;  
		DWORD dwDisposition;  
		//��ע�����  
		LONG lRet = ::RegCreateKeyEx(
			vtReg[j].mainPath, 
			vtReg[j].strPath.c_str(), 
			NULL, NULL, REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS, 
			NULL, &hKey, &dwDisposition);  
		//LONG lRet = ::RegCreateKeyA(HKEY_LOCAL_MACHINE, lpSubKey, &hKey);  
		if ( ERROR_SUCCESS != lRet )  
		{
			LOG(pLog, "RegCreateKeyEx %s Failed!!", vtReg[j].strPath.c_str() ); 
			::RegCloseKey(hKey);
			iRet = -1;
			break;
		}
		else
		{
			LOG(pLog, "RegCreateKeyEx %s Successed!!", vtReg[j].strPath.c_str() );
		}

		lRet = ::RegDeleteValue( hKey,
			vtReg[j].strKey.c_str()	); //����ע�����  
		if ( ERROR_SUCCESS != lRet)  
		{  
			LOG(pLog, "ɾ��ֵ %s ʧ��!!", vtReg[j].strKey.c_str() ); 
		}  
		else
		{
			LOG(pLog, "ע�����Դ %s �ͷųɹ�!!", vtReg[j].strKey.c_str() ); 
			//bRegs[*it] = false;
		}

		::RegCloseKey(hKey); //��RegCreateKeyEx���д
	}

	return iRet;
}
