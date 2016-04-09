#include "NetManager.h"
#include "common.h"
#include "NetSimulates.h"
#include "NetSimMgr.h"

#ifdef WIN32
	#include <Windows.h>
	#include "Psapi.h"  
	#pragma comment (lib,"Psapi.lib")    
	#include <Iprtrmib.h>  
	typedef struct  
	{  
		DWORD dwState;      //����״̬  
		DWORD dwLocalAddr;  //���ص�ַ  
		DWORD dwLocalPort;  //���ض˿�  
		DWORD dwRemoteAddr; //Զ�̵�ַ  
		DWORD dwRemotePort; //Զ�̶˿�  
		DWORD dwProcessId;  //���̱�ʶ  

	}MIB_TCPEXROW,*PMIB_TCPEXROW;  


	typedef struct  
	{  
		DWORD dwLocalAddr;  //���ص�ַ  
		DWORD dwLocalPort;  //���ض˿�  
		DWORD dwProcessId;  //���̱�ʶ  

	}MIB_UDPEXROW,*PMIB_UDPEXROW;  


	typedef struct  
	{  
		DWORD dwState;      //����״̬  
		DWORD dwLocalAddr;  //���ص�ַ  
		DWORD dwLocalPort;  //���ض˿�  
		DWORD dwRemoteAddr; //Զ�̵�ַ  
		DWORD dwRemotePort; //Զ�̶˿�  
		DWORD dwProcessId;  //���̱�ʶ  
		DWORD Unknown;      //������ʶ  

	}MIB_TCPEXROW_VISTA,*PMIB_TCPEXROW_VISTA;  


	typedef struct  
	{  
		DWORD dwNumEntries;  
		MIB_TCPEXROW table[ANY_SIZE];  

	}MIB_TCPEXTABLE,*PMIB_TCPEXTABLE;  


	typedef struct  
	{  
		DWORD dwNumEntries;  
		MIB_TCPEXROW_VISTA table[ANY_SIZE];  

	}MIB_TCPEXTABLE_VISTA,*PMIB_TCPEXTABLE_VISTA;  


	typedef struct  
	{  
		DWORD dwNumEntries;  
		MIB_UDPEXROW table[ANY_SIZE];  

	}MIB_UDPEXTABLE,*PMIB_UDPEXTABLE;  


	//=====================================================================================//  
	//Name: DWORD AllocateAndGetTcpExTableFromStack()                                      //  
	//                                                                                     //  
	//Descripion: �ú�������ֻ�� Windows XP��Windows Server 2003 ����Ч                           //  
	//                                                                                     //  
	//=====================================================================================//  
	typedef DWORD (WINAPI *PFNAllocateAndGetTcpExTableFromStack)(  
		PMIB_TCPEXTABLE *pTcpTabel,  
		bool bOrder,  
		HANDLE heap,  
		DWORD zero,  
		DWORD flags  
		);  

	//=====================================================================================//  
	//Name: DWORD AllocateAndGetUdpExTableFromStack()                                      //  
	//                                                                                     //  
	//Descripion: �ú�������ֻ�� XP��Windows Server 2003 ����Ч                                   //  
	//                                                                                     //  
	//=====================================================================================//  
	typedef DWORD (WINAPI *PFNAllocateAndGetUdpExTableFromStack)(  
		PMIB_UDPEXTABLE *pUdpTable,  
		bool bOrder,  
		HANDLE heap,  
		DWORD zero,  
		DWORD flags  
		);  

	//=====================================================================================//  
	//Name: DWORD InternalGetTcpTable2()                                                   //  
	//                                                                                     //  
	//Descripion: �ú����� Windows Vista �Լ� Windows 7 ����Ч                                  //  
	//                                                                                     //  
	//=====================================================================================//  
	typedef DWORD (WINAPI *PFNInternalGetTcpTable2)(  
		PMIB_TCPEXTABLE_VISTA *pTcpTable_Vista,  
		HANDLE heap,  
		DWORD flags  
		);  

	//=====================================================================================//  
	//Name: DWORD InternalGetUdpTableWithOwnerPid()                                        //  
	//                                                                                     //  
	//Descripion: �ú����� Windows Vista �Լ� Windows 7 ����Ч                                  //  
	//                                                                                     //  
	//=====================================================================================//  
	typedef DWORD (WINAPI *PFNInternalGetUdpTableWithOwnerPid)(  
		PMIB_UDPEXTABLE *pUdpTable,  
		HANDLE heap,  
		DWORD flags  
		); 

	typedef DWORD (WINAPI *PFNGetModuleFileNameEx)(
		_In_     HANDLE  hProcess,
		_In_opt_ HMODULE hModule,
		_Out_    LPTSTR  lpFilename,
		_In_     DWORD   nSize
		);
#else
	#include <iostream>
	#include <fstream>
	#include <errno.h>
#endif

extern NETSIM_API CNetManager* g_pNetManager = NULL;

CNetManager::CNetManager(void)
{
	m_bIsSimulating = false;
}


CNetManager::~CNetManager(void)
{
}

#ifdef WIN32
//! ��ȡϵͳ��ǰ��ռ�õĶ˿���Ϣ
int CNetManager::GetPortInfo(std::map<unsigned int, portInfo*>& mapTcpPorts,
	std::map<unsigned int, portInfo*>& mapUdpPorts)
{
	HMODULE hModule = LoadLibraryW(L"iphlpapi.dll");
	if (hModule == NULL)
	{
		return -1;
	}

	// ������ѯ���� TCP ��Ϣ
	PFNAllocateAndGetTcpExTableFromStack pAllocateAndGetTcpExTableFromStack;
	pAllocateAndGetTcpExTableFromStack = 
		(PFNAllocateAndGetTcpExTableFromStack)GetProcAddress(hModule, "AllocateAndGetTcpExTableFromStack");
	if (pAllocateAndGetTcpExTableFromStack != NULL)
	{
		// ����Ϊ XP ���� Server 2003 ����ϵͳ
		PMIB_TCPEXTABLE pTcpExTable = NULL;
		if (pAllocateAndGetTcpExTableFromStack(&pTcpExTable, TRUE, GetProcessHeap(), 0, AF_INET) != 0)
		{
			if (pTcpExTable)
			{
				HeapFree(GetProcessHeap(), 0, pTcpExTable);
			}

			FreeLibrary(hModule);
			hModule = NULL;

			return -1;
		}

		for (UINT i = 0; i < pTcpExTable->dwNumEntries; i++)
		{
			portInfo* info = new portInfo;
			info->uiPid = pTcpExTable->table[i].dwProcessId;
			info->iLocalPort = pTcpExTable->table[i].dwLocalPort;
			info->nType = TcpType;
			if ( info->uiPid == 4 )
			{
				info->strPname = "System";
			}
			else
			{
				//!��ȡ������
				HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, info->uiPid);
				if ( hProcess == 0 )
				{
					delete info;
					break;
				}
				char* procName = new char[MAX_PATH];
				GetModuleFileNameEx(hProcess, NULL, (LPWSTR)procName, MAX_PATH);
				info->strPname = procName;
			}

			mapTcpPorts.insert(std::make_pair(info->iLocalPort, info));
		}

		if (pTcpExTable)
		{
			HeapFree(GetProcessHeap(), 0, pTcpExTable);
		}

		FreeLibrary(hModule);
		hModule = NULL;

		return 0;
	}
	else
	{
		// ����Ϊ Vista ���� 7 ����ϵͳ
		PMIB_TCPEXTABLE_VISTA pTcpExTable = NULL;
		PFNInternalGetTcpTable2 pInternalGetTcpTable2 = 
			(PFNInternalGetTcpTable2)GetProcAddress(hModule, "InternalGetTcpTable2");
		if (pInternalGetTcpTable2 == NULL)
		{
			if (pTcpExTable)
			{
				HeapFree(GetProcessHeap(), 0, pTcpExTable);
			}

			FreeLibrary(hModule);
			hModule = NULL;

			return 0;
		}

		if (pInternalGetTcpTable2(&pTcpExTable, GetProcessHeap(), 1))
		{
			if (pTcpExTable)
			{
				HeapFree(GetProcessHeap(), 0, pTcpExTable);
			}
			FreeLibrary(hModule);
			hModule = NULL;
			return 0;
		}

		for (UINT i = 0;i < pTcpExTable->dwNumEntries; i++)
		{
			portInfo* info = new portInfo;
			info->uiPid = pTcpExTable->table[i].dwProcessId;
			info->iLocalPort = pTcpExTable->table[i].dwLocalPort;
			info->nType = TcpType;
			if ( info->uiPid == 4 )
			{
				info->strPname = "System";
			}
			else
			{
				//!��ȡ������
				HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, info->uiPid);
				if ( hProcess == 0 )
				{
					delete info;
					break;
				}
				char* procName = new char[MAX_PATH];
				GetModuleFileNameEx(hProcess, NULL, (LPWSTR)procName, MAX_PATH);
				//GetModuleBaseName(hProcess, NULL, procName, MAX_PATH);
				info->strPname = procName;
			}

			mapTcpPorts.insert(std::make_pair(info->iLocalPort, info));
		}

		if (pTcpExTable)
		{
			HeapFree(GetProcessHeap(), 0, pTcpExTable);
		}

		FreeLibrary(hModule);
		hModule = NULL;
		return 0;
	}

	// ������ѯ���� UDP ��Ϣ
	PMIB_UDPEXTABLE pUdpExTable = NULL;
	PFNAllocateAndGetUdpExTableFromStack pAllocateAndGetUdpExTableFromStack;
	pAllocateAndGetUdpExTableFromStack = 
		(PFNAllocateAndGetUdpExTableFromStack)GetProcAddress(hModule,"AllocateAndGetUdpExTableFromStack");
	if (pAllocateAndGetUdpExTableFromStack != NULL)
	{
		// ����Ϊ XP ���� Server 2003 ����ϵͳ
		if (pAllocateAndGetUdpExTableFromStack(&pUdpExTable, TRUE, GetProcessHeap(), 0, AF_INET) != 0)
		{
			if (pUdpExTable)
			{
				HeapFree(GetProcessHeap(), 0, pUdpExTable);
			}

			FreeLibrary(hModule);
			hModule = NULL;

			return -1;
		}

		for (UINT i = 0; i < pUdpExTable->dwNumEntries; i++)
		{
			portInfo* info = new portInfo;
			info->uiPid = pUdpExTable->table[i].dwProcessId;
			info->iLocalPort = pUdpExTable->table[i].dwLocalPort;
			info->nType = UdpType;
			if ( info->uiPid == 4 )
			{
				info->strPname = "System";
			}
			else
			{
				//!��ȡ������
				HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, info->uiPid);
				if ( hProcess == 0 )
				{
					delete info;
					break;
				}
				char* procName = new char[MAX_PATH];
				GetModuleFileNameEx(hProcess, NULL, (LPWSTR)procName, MAX_PATH);
				//GetModuleBaseName(hProcess, NULL, procName, MAX_PATH);
				info->strPname = procName;
			}

			mapUdpPorts.insert(std::make_pair(info->iLocalPort, info));
		}

		if (pUdpExTable)
		{
			HeapFree(GetProcessHeap(), 0, pUdpExTable);
		}

		FreeLibrary(hModule);
		hModule = NULL;

		return 0;
	}
	else
	{
		// ����Ϊ Vista ���� 7 ����ϵͳ
		PFNInternalGetUdpTableWithOwnerPid pInternalGetUdpTableWithOwnerPid;
		pInternalGetUdpTableWithOwnerPid = 
			(PFNInternalGetUdpTableWithOwnerPid)GetProcAddress(hModule, "InternalGetUdpTableWithOwnerPid");
		if (pInternalGetUdpTableWithOwnerPid != NULL)
		{
			if (pInternalGetUdpTableWithOwnerPid(&pUdpExTable, GetProcessHeap(), 1))
			{
				if (pUdpExTable)
				{
					HeapFree(GetProcessHeap(), 0, pUdpExTable);
				}

				FreeLibrary(hModule);
				hModule = NULL;

				return -1;
			}

			for (UINT i = 0; i < pUdpExTable->dwNumEntries; i++)
			{
				portInfo* info = new portInfo;
				info->uiPid = pUdpExTable->table[i].dwProcessId;
				info->iLocalPort = pUdpExTable->table[i].dwLocalPort;
				info->nType = UdpType;
				if ( info->uiPid == 4 )
				{
					info->strPname = "System";
				}
				else
				{
					//!��ȡ������
					HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, info->uiPid);
					if ( hProcess == 0 )
					{
						delete info;
						break;
					}
					char* procName = new char[MAX_PATH];
					GetModuleFileNameEx(hProcess, NULL, (LPWSTR)procName, MAX_PATH);
					//GetModuleBaseName(hProcess, NULL, procName, MAX_PATH);
					info->strPname = procName;
				}

				mapUdpPorts.insert(std::make_pair(info->iLocalPort, info));
			}
		}

		if (pUdpExTable)
		{
			HeapFree(GetProcessHeap(), 0, pUdpExTable);
		}

		FreeLibrary(hModule);
		hModule = NULL;

		return 0;
	}

	return 0;
}

#else

//! ��ȡϵͳ��ǰ��ռ�õĶ˿���Ϣ
int CNetManager::GetPortInfo(std::map<unsigned int, portInfo*>& mapTcpPorts,
                             std::map<unsigned int, portInfo*>& mapUdpPorts)
{
    char pLog[MAX_LOG_LEN];
    char buffer[512];
    //! TCP
    int iRet = system("netstat -anpt4 >/tmp/tcpnet.tmp");
    LOG(pLog, "%s", "netstat -anpt4 >/tmp/tcpnet.tmp");
    if( iRet != 0 ) {
        LOG(pLog, "system Failed, errmsg:%s!!(%d)", strerror(errno),errno);
        return iRet;
    }
    else {
        LOG(pLog, "system Successed!");
    }
    std::ifstream ifTcp("/tmp/tcpnet.tmp");
    if(!ifTcp){
        LOG(pLog, "file /tmp/tcpnet.tmp not found");
        ifTcp.close();
    }
    else{
        //! step over the first two lines
        while (! ifTcp.eof() )
        {            
            ifTcp.getline(buffer, 512);
            QString strBuf(buffer);
            int iIndex = strBuf.indexOf(':');
            if( iIndex != -1 )
            {
                portInfo* infoTcp = new portInfo;
                QString strRight = strBuf.right(strBuf.length()-iIndex-1);
                iIndex = strRight.indexOf(' ');
                if ( iIndex != -1 )
                {
                    infoTcp->iLocalPort = strRight.left(iIndex).toUInt();
                }
                iIndex = strRight.lastIndexOf('/');
                if ( iIndex != -1 )
                {
                    infoTcp->strPname = strRight.right(strRight.length()-iIndex-1).trimmed().toStdString();
                    strBuf = strRight.left(iIndex);
                    iIndex = strBuf.lastIndexOf(' ');
                    if( iIndex != -1 )
                    {
                        infoTcp->uiPid = strBuf.right(strBuf.length()-iIndex-1).toUInt();
                    }
                }
                infoTcp->nType = TcpType;
                mapTcpPorts.insert(std::make_pair(infoTcp->iLocalPort, infoTcp));
            }

        }
        ifTcp.close();
    }

    //! UDP
    iRet = system("netstat -anpu4 >/tmp/udpnet.tmp");
    LOG(pLog, "%s", "netstat -anpu4 >/tmp/udpnet.tmp");
    if( iRet != 0 ) {
        LOG(pLog, "system Failed, errmsg:%s!!(%d)", strerror(errno),errno);
        return iRet;
    }
    else {
        LOG(pLog, "system Successed!");
    }
    std::ifstream ifUdp("/tmp/udpnet.tmp");
    if(!ifUdp){
        LOG(pLog, "file /tmp/udpnet.tmp not found");
        ifUdp.close();
    }
    else{
        //! step over the first two lines
        while (! ifUdp.eof() )
        {
            ifUdp.getline(buffer,512);
            QString strBuf(buffer);
            int iIndex = strBuf.indexOf(':');
            if( iIndex != -1 )
            {
                portInfo* infoUdp = new portInfo;
                QString strRight = strBuf.right(strBuf.length()-iIndex-1);
                iIndex = strRight.indexOf(' ');
                if ( iIndex != -1 )
                {
                    infoUdp->iLocalPort = strRight.left(iIndex).toUInt();
                }
                iIndex = strRight.lastIndexOf('/');
                if ( iIndex != -1 )
                {
                    infoUdp->strPname = strRight.right(strRight.length()-iIndex-1).trimmed().toStdString();
                    strBuf = strRight.left(iIndex);
                    iIndex = strBuf.lastIndexOf(' ');
                    if( iIndex != -1 )
                    {
                        infoUdp->uiPid = strBuf.right(strBuf.length()-iIndex-1).toUInt();
                    }
                }
                infoUdp->nType = UdpType;
                mapUdpPorts.insert(std::make_pair(infoUdp->iLocalPort, infoUdp));
            }
        }
        ifUdp.close();
    }
	return 0;
}

#endif

int CNetManager::PortsOccupy( std::vector<int> vtPorts )
{
	for (int iRow = 0; iRow < vtPorts.size(); iRow++)
	{
		//! �ж϶˿ں��Ƿ�Ϸ�
		if ( vtPorts[iRow] < 1 || vtPorts[iRow] > 65534 )
		{
//			LOG(pLog, "�˿ں� %d ���Ϸ������ܱ�ռ�ã�", listPort[iRow]);
			break;
		}
#ifdef WIN32
		//! ///�����׽���
		SOCKET Listen_Sock = socket(AF_INET, SOCK_STREAM, 0);
		if (Listen_Sock == INVALID_SOCKET)
		{
//			LOG(pLog, "�����׽���ʧ�ܣ�");
			break;
		}
#else
		int Listen_Sock = socket(AF_INET, SOCK_STREAM, 0);
		if (Listen_Sock == -1)
		{
			LOG(pLog, "�����׽���ʧ�ܣ�");
			break;
		}
#endif

#ifdef WIN32
		SOCKADDR_IN serverAddr;
		ZeroMemory((char *)&serverAddr,sizeof(serverAddr));
#else
		struct sockaddr_in serverAddr;
#endif
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(vtPorts[iRow]);        /*���ؼ����˿�:1234*/
		serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);		/*��IP*/
#ifdef WIN32
		//��
		if(bind(Listen_Sock, (SOCKADDR *)&serverAddr, sizeof(SOCKADDR)) != 0)
		{
			//LOG(pLog, "�󶨶˿� %d ʧ�ܣ�", vtPorts[iRow]);
			break;
		}

		if ( SOCKET_ERROR == listen(Listen_Sock, 1) )
		{
			//LOG(pLog, "�����˿� %d ʧ�ܣ������룺%d��", vtPorts[iRow], WSAGetLastError());
			break;
		}
		else
		{
			//LOG(pLog, "ռ������˿� %d �ɹ���", vtPorts[iRow]);
		}

		socketList.insert(std::map<int,SOCKET>::value_type(vtPorts[iRow],Listen_Sock));
#else
		//��
		if(bind(Listen_Sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
		{
			//LOG(pLog, "�󶨶˿� %d ʧ�ܣ�", vtPorts[iRow]);
			break;
		}

		if ( -1 == listen(Listen_Sock, 1) )
		{
			//LOG(pLog, "�����˿� %d ʧ�ܣ�����%s��(errno:%d)", listPort[iRow], strerror(errno), errno);
			break;
		}
		else
		{
			//LOG(pLog, "ռ������˿� %d �ɹ���", vtPorts[iRow]);
		}

		socketList.insert(std::map<int,int>::value_type(vtPorts[iRow],Listen_Sock));
#endif
	}
	return 0;
}

int CNetManager::PortsRelease( std::vector<int> vtPorts )
{
	//char pLog[MAX_LOG_LEN];
	for (int iRow = 0; iRow < vtPorts.size(); iRow++)
	{
#ifdef WIN32
		std::map<int, SOCKET>::iterator it = socketList.find(vtPorts[iRow]);
		if ( it != socketList.end() )
		{
			closesocket(it->second);
			//LOG(pLog, "�ͷ�����˿� %d �ɹ���", listPort[iRow]);
		}
#else
		std::map<int, int>::iterator it = socketList.find(vtPorts[iRow]);
		if ( it != socketList.end() )
		{
			::close(it->second);
			//LOG(pLog, "�ͷ�����˿� %d �ɹ���", listPort[iRow]);
		}
#endif
	}
	return 0;
}

int CNetManager::StartNetSim(NetSimParam& simParam)
{
	//g_simParam = simParam;
	if ( m_bIsSimulating )
	{
		return -99;
	}

	char buf[MSG_BUFSIZE];
	std::string pch = simParam.strFilter;
#ifndef WIN32
	char pchTemp[MSG_BUFSIZE];
	//!
	std::string pOutOpr = "tc qdisc add dev eth0 parent 1:4 handle 40: netem";
	std::string pInOpr = "tc qdisc add dev ifb0 parent 1:4 handle 40: netem";
	if(simParam.ood.bEnable && simParam.lag.bEnable)
	{
		//! in
		if(ui.oodin->checkState() == Qt::Checked && ui.lagin->checkState() == Qt::Checked) {
			sprintf(pchTemp, " delay %sms reorder %s%% 50%%",
				ui.lagtime->text().toStdString().c_str(),
				ui.oodchance->text().toStdString().c_str());
		}
		else if(ui.oodin->checkState() == Qt::Checked) {
			sprintf(pchTemp, " delay 10ms reorder %s%% 50%%",
				ui.oodchance->text().toStdString().c_str());
		}
		else if(ui.lagin->checkState() == Qt::Checked) {
			sprintf(pchTemp, " delay %sms", ui.lagtime->text().toStdString().c_str());
		}
		pInOpr += std::string(pchTemp);

		//! out
		if(ui.oodout->checkState() == Qt::Checked && ui.lagout->checkState() == Qt::Checked) {
			sprintf(pchTemp, " delay %sms reorder %s%% 50%%",
				ui.lagtime->text().toStdString().c_str(),
				ui.oodchance->text().toStdString().c_str());
		}
		else if(ui.oodout->checkState() == Qt::Checked) {
			sprintf(pchTemp, " delay 10ms reorder %s%% 50%%",
				ui.oodchance->text().toStdString().c_str());
		}
		else if(ui.lagout->checkState() == Qt::Checked) {
			sprintf(pchTemp, " delay %sms", ui.lagtime->text().toStdString().c_str());
		}
		pOutOpr += std::string(pchTemp);
	}
	else if(LagEnable())
	{
		sprintf(pchTemp, " delay %sms", ui.lagtime->text().toStdString().c_str());
		if(ui.lagin->checkState() == Qt::Checked)
			pInOpr += std::string(pchTemp);
		if(ui.lagout->checkState() == Qt::Checked)
			pOutOpr += std::string(pchTemp);
	}
	else if(OODEnable())
	{
		sprintf(pchTemp, " delay 10ms reorder %s%% 50%%",
			ui.oodchance->text().toStdString().c_str());
		if(ui.lagin->checkState() == Qt::Checked)
			pInOpr += std::string(pchTemp);
		if(ui.lagout->checkState() == Qt::Checked)
			pOutOpr += std::string(pchTemp);
	}

	if(DropEnable())
	{
		sprintf(pchTemp, " loss %s%%", ui.dropchance->text().toStdString().c_str());
		if(ui.lagin->checkState() == Qt::Checked)
			pInOpr += std::string(pchTemp);
		if(ui.lagout->checkState() == Qt::Checked)
			pOutOpr += std::string(pchTemp);
	}
	if(DuplicateEnable())
	{
		sprintf(pchTemp, " duplicate %s %s%%",
			ui.dupcount->text().toStdString().c_str(),
			ui.dupchance->text().toStdString().c_str());
		if(ui.lagin->checkState() == Qt::Checked)
			pInOpr += std::string(pchTemp);
		if(ui.lagout->checkState() == Qt::Checked)
			pOutOpr += std::string(pchTemp);
	}
	if(TamperEnable())
	{
		sprintf(pchTemp, " corrupt %s%%",
			ui.tamperchance->text().toStdString().c_str());
		if(ui.lagin->checkState() == Qt::Checked)
			pInOpr += std::string(pchTemp);
		if(ui.lagout->checkState() == Qt::Checked)
			pOutOpr += std::string(pchTemp);
	}
	if(ThrottleEnable())
	{
	}
	
	sprintf(buf, "%s", pOutOpr.c_str());
	//LOG(pLog, "%s", pOpr.c_str());
	int iRet = g_pNetSimMgr->start(pch.c_str(), buf, pInOpr.c_str());
#else
	int iRet = g_pNetSimMgr->start(pch.c_str(), buf);
#endif


	if ( iRet != 0 )		//! ����ʧ��
	{
		return -1;
	}

	m_bIsSimulating = true;
	return 0;
}
int CNetManager::StopNetSim(NetSimParam& simParam)
{
	g_pNetSimMgr->stop();
	m_bIsSimulating = false;
	return 0;
}