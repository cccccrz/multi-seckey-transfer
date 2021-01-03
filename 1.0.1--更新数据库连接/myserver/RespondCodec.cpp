#include "RespondCodec.h"
#include <iostream>
using namespace std;

//解码构造函数
RespondCodec::RespondCodec()
{

}

//编码构造函数
RespondCodec::RespondCodec(RespondMsg* rspMsg)
{
#if 1  
	memcpy(&m_rspMsg, rspMsg, sizeof(RespondMsg));
#else
	//可随意更换结构体数据赋值方法
	m_rspMsg.rv = rspMsg->rv;
	m_rspMsg.seckeyID = rspMsg->seckeyID;
	strcpy(m_rspMsg.clientID, rspMsg->clientID);
	strcpy(m_rspMsg.serverID, rspMsg->serverID);
	strcpy(m_rspMsg.r2, rspMsg->r2);
#endif
}

RespondCodec::~RespondCodec()
{
	cout << "RespondCodec destruct ..." << endl;
}

/*
struct RespondMsg
{
	char    serverID[12];   //服务端ID
	char    clientID[12];   //客户端ID
	char    r2[64];         //服务端随机字符串
	int     seckeyID;       //秘钥ID
	int     rv;             //服务器接收信息结果：0 成功；-1 失败
};
*/
//应答结构体编码：传出字节流；成功返回 0
int RespondCodec::EncodeMsg(char** outData, int& outLen)
{
	writeHeadNode(m_rspMsg.serverID, strlen(m_rspMsg.serverID));
	writeNextNode(m_rspMsg.clientID, strlen(m_rspMsg.clientID));
	writeNextNode(m_rspMsg.r2, strlen(m_rspMsg.r2));
	writeNextNode(m_rspMsg.seckeyID);
	writeNextNode(m_rspMsg.rv);

	//序列化
	packSequence(outData, outLen);
	cout << "rspMsg编码成功！" << endl;
	return 0;
}

//应答结构体解码：传入字节流，返回应答结构体
void* RespondCodec::DecodeMsg(char* intData, int inLen)
{
	//反序列化
	unpackSequence(intData, inLen);

	//读取数据
	readHeadNode(m_rspMsg.serverID);
	readNextNode(m_rspMsg.clientID);
	readNextNode(m_rspMsg.r2);
	readNextNode(m_rspMsg.seckeyID);
	readNextNode(m_rspMsg.rv);

	cout << "rspMsg解码成功！" << endl;
	return &m_rspMsg;
}
