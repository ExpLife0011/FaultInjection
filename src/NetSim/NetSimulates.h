#ifndef _NETWORK_SIMULATES_H_
#define _NETWORK_SIMULATES_H_

#include "common.h"

#ifdef WIN32
//! ��ʱ
class NetLag : public NetworkSim
{
public:
	NetLag(void);
	~NetLag(void);

	virtual void startUp();			// called when starting up the module
	virtual void closeDown(PacketNode *head, PacketNode *tail); // called when starting up the module
	virtual short process(PacketNode *head, PacketNode *tail);
	inline short isBufEmpty() {
		short ret = bufHead->next == bufTail;
		if (ret) assert(bufSize == 0);
		return ret;
	}
	virtual bool enable();
	virtual void UpdateParams();

	short shIn;
	short shOut;
	int lagTime;

	static PacketNode lagHeadNode, lagTailNode;
	static PacketNode *bufHead, *bufTail;
	static int bufSize;
};

///////////////////////////////////////////////////////////////////////////////////////////
//! ����
class NetDrop : public NetworkSim
{
public:
	NetDrop(void);
	~NetDrop(void);

	virtual void startUp();			// called when starting up the module
	virtual void closeDown(PacketNode *head, PacketNode *tail); // called when starting up the module
	virtual short process(PacketNode *head, PacketNode *tail);
	virtual bool enable();
	virtual void UpdateParams();

	short shIn;
	short shOut;
	int iChance;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//! ��������
class NetThrottle : public NetworkSim
{
public:
	NetThrottle(void);
	~NetThrottle(void);

	virtual void startUp();			// called when starting up the module
	virtual void closeDown(PacketNode *head, PacketNode *tail); // called when starting up the module
	virtual short process(PacketNode *head, PacketNode *tail);
	virtual bool enable();
	virtual void UpdateParams();

	inline short NetThrottle::isBufEmpty();
	void clearBufPackets(PacketNode *tail);
	void dropBufPackets();

	short shIn;
	short shOut;
	short dropThrottled;
	int iChance;
	int iTime;
	int bufSize;
	DWORD throttleStartTick;
	PacketNode *bufHead, *bufTail;
};

////////////////////////////////////////////////////////////////////////////////////////////////
//! �ظ�
class NetDuplicate : public NetworkSim
{
public:
	NetDuplicate(void);
	~NetDuplicate(void);

	virtual void startUp();			// called when starting up the module
	virtual void closeDown(PacketNode *head, PacketNode *tail); // called when starting up the module
	virtual short process(PacketNode *head, PacketNode *tail);
	virtual bool enable();
	virtual void UpdateParams();

	short shIn;
	short shOut;
	int iChance;
	int iCount;
};

////////////////////////////////////////////////////////////////////////////////////////////////
//! ����
class NetOOD : public NetworkSim
{
public:
	NetOOD(void);
	~NetOOD(void);

	virtual void startUp();			// called when starting up the module
	virtual void closeDown(PacketNode *head, PacketNode *tail); // called when starting up the module
	virtual short process(PacketNode *head, PacketNode *tail);
	virtual bool enable();
	virtual void UpdateParams();

	// find the next packet fits the direction check or null
	PacketNode* nextCorrectDirectionNode(PacketNode *p);
	// not really perfect swap since it assumes a is before b
	void swapNode(PacketNode *a, PacketNode *b);

	short shIn;
	short shOut;
	int iChance;

	int giveUpCnt;
	PacketNode *oodPacket;
};

////////////////////////////////////////////////////////////////////////////////////////////////
//! �۸�
class NetTamper : public NetworkSim
{
public:
	NetTamper(void);
	~NetTamper(void);

	virtual void startUp();			// called when starting up the module
	virtual void closeDown(PacketNode *head, PacketNode *tail); // called when starting up the module
	virtual short process(PacketNode *head, PacketNode *tail);
	virtual bool enable();
	virtual void UpdateParams();

	void tamper_buf(char *buf, UINT len);

	short shIn;
	short shOut;
	int iChance;
	short shReSum;

	int patIx;
};

extern NetSimParam* g_simParam;
#endif //WIN32

#endif

