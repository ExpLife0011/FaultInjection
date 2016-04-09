#ifndef MQCOMM_CLIENT_H
#define MQCOMM_CLIENT_H

#include <string>
#include <map>
#include <vector>
#include "PubFile.h"

class MQCOMM_API CClient
{
public:
	CClient(std::string strIP);
	~CClient(void);

	//! ����
	int Connect();

	//! ��������
	int SendJob(std::string strXMLJob, std::string& strRet);

	//! ��ȡ״̬
	int getStatus(std::string& strXMLStatus);

private:
	std::string m_strIP;
	void *m_context;
	void *m_socket;
	bool m_bConnected;
};

#endif

