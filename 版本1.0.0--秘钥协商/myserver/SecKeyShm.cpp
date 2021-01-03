#include <string.h>
#include <iostream>
#include "SecKeyShm.h"
using namespace std;

//调用父类构造初始化
SecKeyShm::SecKeyShm(int key) :ShareMemory(key)
{

}

SecKeyShm::SecKeyShm(int key, int maxNode) : ShareMemory(key, maxNode * sizeof(NodeSHMInfo))
, m_maxNode(maxNode)
{
	cout << "maxNode=" << maxNode << endl;
#if 0
	//关联共享内存
	void* p = mapShm();
	//将maxNode写入前四个字节
	memcpy(p, &m_maxNode, 4);
	//初始化
	NodeSHMInfo* pNode = (NodeSHMInfo*)((char*)p + 4);//pNode指向第一个Node
	memset(pNode, 0x00, m_maxNode * sizeof(NodeSHMInfo));
	//断开关联
	unmapShm();
#endif
}

SecKeyShm::SecKeyShm(const char* pathName) : ShareMemory(pathName)
{

}

SecKeyShm::SecKeyShm(const char* pathName, int maxNode) : ShareMemory(pathName, maxNode * sizeof(NodeSHMInfo))
, m_maxNode(maxNode)
{
	cout << "maxNode=" << maxNode << endl;
#if 0
	//关联共享内存
	void* p = mapShm();
	//将maxNode写入前四个字节
	memcpy(p, &m_maxNode, 4);
	//初始化
	NodeSHMInfo* pNode = (NodeSHMInfo*)((char*)p + 4);//pNode指向第一个Node
	memset(pNode, 0x00, m_maxNode * sizeof(NodeSHMInfo));
	//断开关联
	unmapShm();
#endif
}

SecKeyShm::~SecKeyShm()
{
}

//将秘钥信息写入共享内存
int SecKeyShm::shmWrite(NodeSHMInfo* pNodeInfo)
{
#if 0
	int ret = 0;
	//关联共享内存
	void* p = mapShm();

	//获得Node个数
	memcpy(&m_maxNode, p, 4);
	//获得第一个Node地址
	NodeSHMInfo* pNode = (NodeSHMInfo*)((char*)p + 4);

	//遍历共享内存
	//根据clientID和serverID查找是否已经存在
	int i = 0;	//记录Node下标
	for (i = 0; i < m_maxNode; i++)
	{
		if (strcmp(pNode[i].clientID, pNodeInfo->clientID) == 0 && \
			strcmp(pNode[i].serverID, pNodeInfo->serverID) == 0)
		{	//存在，覆盖这个位置
			memcpy(&pNode[i], pNodeInfo, sizeof(NodeSHMInfo));
			unmapShm();
			return ret;
		}
	}
	//没有找到，遍历找空闲位置(初始化0)写入
	NodeSHMInfo tmpNode;
	memset(&tmpNode, 0x00, sizeof(NodeSHMInfo));
	if (i == m_maxNode)
	{
		for (i = 0; i < m_maxNode; i++)
		{
			if (memcmp(&pNode[i], &tmpNode, sizeof(NodeSHMInfo)) == 0)
			{	//写入
				memcpy(&pNode[i], pNodeInfo, sizeof(NodeSHMInfo));
				break;
			}
		}
	}

	//没有空间可用	
	if (i == m_maxNode)
	{
		cout << "no space to left" << endl;
		ret = -1;
	}
	//断开关联
	unmapShm();

	return ret;

#else
	int ret = -1;
	// 关联共享内存
	NodeSHMInfo* pAddr = static_cast<NodeSHMInfo*>(mapShm());
	if (pAddr == NULL)
	{
		return ret;
	}

	// 判断传入的网点密钥是否已经存在
	NodeSHMInfo* pNode = NULL;
	for (int i = 0; i < m_maxNode; i++)
	{
		// pNode依次指向每个节点的首地址
		pNode = pAddr + i;
		if (strcmp(pNode->clientID, pNodeInfo->clientID) == 0 &&
			strcmp(pNode->serverID, pNodeInfo->serverID) == 0)
		{
			// 如果找到了该网点秘钥已经存在, 使用新秘钥覆盖旧的值
			memcpy(pNode, pNodeInfo, sizeof(NodeSHMInfo));
			unmapShm();
			return 0;
		}
	}

	// 若没有找到对应的信息, 找一个空节点将秘钥信息写入
	int i = 0;
	NodeSHMInfo  tmpNodeInfo; //空结点
	memset(&tmpNodeInfo, 0, sizeof(NodeSHMInfo));
	for (i = 0; i < m_maxNode; i++)
	{
		pNode = pAddr + i;
		if (memcmp(&tmpNodeInfo, pNode, sizeof(NodeSHMInfo)) == 0)
		{
			ret = 0;
			memcpy(pNode, pNodeInfo, sizeof(NodeSHMInfo));
			break;
		}
	}
	if (i == m_maxNode)
	{
		ret = -1;
	}

	unmapShm();
	return ret;
#endif
}

//根据clientID和serverID 查找秘钥信息，传出NodeShmInfo
int SecKeyShm::shmRead(const char* clientID, const char* serverID, NodeSHMInfo* pNodeInfo)
{
#if 0
	int ret = 0;
	//关联shm
	void* p = mapShm();

	//获取Node个数
	memcpy(&m_maxNode, p, 4);
	NodeSHMInfo* pNode = (NodeSHMInfo*)((char*)p + 4);

	//遍历查找
	int i = 0;
	for (i = 0; i < m_maxNode; i++)
	{
		if (strcmp(pNode[i].clientID, clientID) == 0 && \
			strcmp(pNode[i].serverID, serverID) == 0)
		{	//找到了，传出参数赋值
			memcpy(pNodeInfo, &pNode[i], sizeof(NodeSHMInfo));
			break;
		}
	}
	//没找到
	if (i == m_maxNode)
	{
		cout << "not found data" << endl;
		ret = -1;
	}
	//断开与共享内存的关联
	unmapShm();

	return ret;
#else
	int ret = 0;
	// 关联共享内存
	NodeSHMInfo* pAddr = NULL;
	pAddr = static_cast<NodeSHMInfo*>(mapShm());
	if (pAddr == NULL)
	{
		return -1;
	}

	//遍历网点信息
	int i = 0;
	NodeSHMInfo* pNode = NULL;
	// 通过clientID和serverID查找节点
	for (i = 0; i < m_maxNode; i++)
	{
		pNode = pAddr + i;
		if (strcmp(pNode->clientID, clientID) == 0 &&
			strcmp(pNode->serverID, serverID) == 0)
		{
			// 找到的节点信息, 拷贝到传出参数
			memcpy(pNodeInfo, pNode, sizeof(NodeSHMInfo));
			break;
		}
	}
	if (i == m_maxNode)
	{
		ret = -1;
	}

	unmapShm();
	return ret;
#endif
}

//test
void SecKeyShm::printShm()
{
	//连接共享内存
	void* p = mapShm();

	//获得共享内存大小--头4个字节表示共享内存大小
	memcpy(&m_maxNode, p, sizeof(int));
	cout << "m_maxNode==" << m_maxNode << endl;

	NodeSHMInfo* pNode = (NodeSHMInfo*)((char*)p + sizeof(int));
	for (int i = 0; i < m_maxNode; i++)
	{
		cout << "--------" << i << "-------" << endl;
		cout << "status:" << pNode[i].status << endl;
		cout << "seckeyID:" << pNode[i].seckeyID << endl;
		cout << "clientID:" << pNode[i].clientID << endl;
		cout << "serverID:" << pNode[i].serverID << endl;
		cout << "seckey:" << pNode[i].seckey << endl;
		cout << "--------------------" << endl;
	}

	//断开与共享内存的关联
	unmapShm();

	return;
}
