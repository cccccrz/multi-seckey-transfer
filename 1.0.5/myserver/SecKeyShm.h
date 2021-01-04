#pragma once
#include "ShareMemory.h"

//秘钥信息结构体
struct NodeSHMInfo
{
	int		 status;		//秘钥状态：-1可用；0已使用
	int		 seckeyID;		//秘钥ID
	char	 clientID[12];	//客户端ID
	char	 serverID[12];	//服务端ID
	char	 seckey[128];	//秘钥
};

//秘钥存储类--通过共享内存
//存储方式：4字节(Node长度)+Node
class SecKeyShm :public ShareMemory
{
public:
	SecKeyShm(int key);
	SecKeyShm(int key, int maxNode);
	SecKeyShm(const char* pathName);
	SecKeyShm(const char* pathName, int maxNode);

	~SecKeyShm();

	//写入共享内存
	int shmWrite(NodeSHMInfo* pNodeInfo);
	//读取共享内存
	int shmRead(const char* clientID, const char* serverID, NodeSHMInfo* pNodeInfo);

	//test
	void printShm();

private:
	int m_maxNode;
};
