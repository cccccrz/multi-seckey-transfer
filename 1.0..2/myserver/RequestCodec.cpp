#include "RequestCodec.h"
#include <iostream>
using namespace std;

//请求结构体解码构造
RequestCodec::RequestCodec()
{

}

//请求结构体编码构造 -- 传入请求结构体
RequestCodec::RequestCodec(RequestMsg* reqMsg)
{
#if 0 
	memcpy(&m_reqMsg, reqMsg, sizeof(RequestMsg));
#else 
	//可随意更换结构体数据赋值方法
	m_reqMsg.cmdType = reqMsg->cmdType;
	strcpy(m_reqMsg.clientID, reqMsg->clientID);
	strcpy(m_reqMsg.serverID, reqMsg->serverID);
	strcpy(m_reqMsg.authCode, reqMsg->authCode);
	strcpy(m_reqMsg.r1, reqMsg->r1);
#endif
}

RequestCodec::~RequestCodec()
{
	cout << "RequestCodec destruct ..." << endl;
}

/*
struct RequestMsg
{
	char     clientID[12];      //客户端ID
	char    authCode[65];       //消息验证码 ---- HMAC(@serverID+clientID@,r1)EVP_sha256()
	char    serverID[12];       //服务端ID
	char     r1[64];            //客户端随机数
	int     cmdType;            //请求类型  1.秘钥协商；2.秘钥查看；3.秘钥注销
};
*/
//编码请求信息：传出字节流；成功返回 0
int RequestCodec::EncodeMsg(char** outData, int& outLen)
{
	writeHeadNode(m_reqMsg.clientID, strlen(m_reqMsg.clientID));
	writeNextNode(m_reqMsg.authCode, strlen(m_reqMsg.authCode));
	writeNextNode(m_reqMsg.serverID, strlen(m_reqMsg.serverID));
	writeNextNode(m_reqMsg.r1, strlen(m_reqMsg.r1));
	writeNextNode(m_reqMsg.cmdType);
	//序列化
	packSequence(outData, outLen);

	cout << "reqMsg编码成功！" << endl;
	return 0;
}

//解码请求信息 传入字节流，返回请求结构体
void* RequestCodec::DecodeMsg(char* inData, int inLen)
{
	//反序列化
	unpackSequence(inData, inLen);

	//传出参数 给结构体赋值
	readHeadNode(m_reqMsg.clientID);
	readNextNode(m_reqMsg.authCode);
	readNextNode(m_reqMsg.serverID);
	readNextNode(m_reqMsg.r1);
	readNextNode(m_reqMsg.cmdType);

	cout << "reqMsg解码成功！" << endl;
	return &m_reqMsg;
}
