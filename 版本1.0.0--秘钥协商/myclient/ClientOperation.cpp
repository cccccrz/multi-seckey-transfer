#include "ClientOperation.h"
#include "RequestCodec.h"
#include <string.h>
#include <time.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include "CodecFactory.h"
#include "RequestFactory.h"
#include "RespondFactory.h"

using namespace std;

ClientOperation::ClientOperation(ClientInfo* info)
{
	memcpy(&m_info, info, sizeof(ClientInfo));

	//创建共享内存,客户端只有1个
	m_shm = new SecKeyShm(m_info.shmKey, 1);
}

ClientOperation::~ClientOperation()
{

}

//密钥协商
int ClientOperation::secKeyAgree()
{
	int ret = 0;

	//准备请求数据
	RequestMsg reqMsg;
	memset(&reqMsg, 0x00, sizeof(RequestMsg));
	reqMsg.cmdType = RequestCodec::NEWORUPDATE;
	strcpy(reqMsg.clientID, m_info.clientID);
	strcpy(reqMsg.serverID, m_info.serverID);
	getRandString(sizeof(reqMsg.r1), reqMsg.r1);

	//使用HMAC 生成哈希值--消息认证码
	char key[64];
	unsigned int mdlen;
	unsigned char md[SHA256_DIGEST_LENGTH];
	memset(&key, 0x00, sizeof(key));
	sprintf(key, "@%s+%s@", m_info.serverID, m_info.clientID);
	HMAC(EVP_sha256(), key, strlen(key), \
		(unsigned char*)reqMsg.r1, strlen(reqMsg.r1), md, &mdlen);
	for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
	{
		sprintf(&reqMsg.authCode[2 * i], "%02x", md[i]);
	}
	cout << "key:" << key << endl;
	cout << "r1:" << reqMsg.r1 << endl;
	cout << "authCode:" << reqMsg.authCode << endl;

	//将要发送的数据进行编码
	char* outData = NULL;
	int dataLen;
	CodecFactory* factory = new RequestFactory(&reqMsg);
	Codec* pCodec = factory->createCodec();
	pCodec->EncodeMsg(&outData, dataLen);
	//delete factory;
	//delete pCodec;

	//连接服务端
	ret = m_socket.connectToHost(m_info.serverIP, m_info.serverPort);
	if (ret != 0)
	{
		cout << "连接失败！" << endl;
		return ret;
	}
	cout << "建立连接！ 开始发送请求......" << endl;
	//发送数据
	ret = m_socket.sendMsg(outData, dataLen);
	if (ret < 0)
	{
		cout << "发送失败！" << endl;
		return ret;
	}
	cout << "发送成功！等待服务端应答......" << endl;

	//等待接收服务端应答
	char* inData;
	ret = m_socket.recvMsg(&inData, dataLen);
	if (ret < 0)
	{
		cout << "接收信息失败！" << endl;
		return ret;
	}
	cout << "信息接收成功！开始解码......" << endl;
	//解码
	factory = new RespondFactory();
	pCodec = factory->createCodec();
	RespondMsg* rspMsg = (RespondMsg*)pCodec->DecodeMsg(inData, dataLen);
	//判断服务端秘钥协商是否成功
	if (rspMsg->rv == -1)
	{
		cout << "秘钥协商失败" << endl;
		return -1;
	}
	else
	{
		cout << "秘钥协商成功" << endl;
	}

	//将服务端的r2和客户端的r1拼接生成秘钥 -- SHA1
	char buf[1024] = { 0 };
	unsigned char md_SHA1[SHA_DIGEST_LENGTH] = { 0 };
	sprintf(buf, "%s%s", reqMsg.r1, rspMsg->r2);
	SHA1((unsigned char*)buf, strlen(buf), md_SHA1);
	char secKey[SHA_DIGEST_LENGTH * 2 + 1];
	for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
	{
		sprintf(&secKey[i * 2], "%02x", md_SHA1[i]);
	}
	cout << "秘钥: " << secKey << endl;

	//给秘钥结构体赋值
	NodeSHMInfo node;
	memset(&node, 0x00, sizeof(NodeSHMInfo));
	node.status = 0;
	strcpy(node.seckey, secKey);
	strcpy(node.clientID, m_info.clientID);
	strcpy(node.serverID, m_info.serverID);
	node.seckeyID = rspMsg->seckeyID;

	//将秘钥信息写入共享内存
	m_shm->shmWrite(&node);

	//关闭网络连接
	m_socket.disConnect();

	delete factory;
	return 0;
}

//秘钥校验
int ClientOperation::secKeyCheck()
{

}

//秘钥注销
int ClientOperation::secKeyRevoke()
{

}

//秘钥查看
int ClientOperation::secKeyView()
{

}


// char randBuf[64]; , 参数 64, randBuf
void ClientOperation::getRandString(int len, char* randBuf)
{
	int flag = -1;
	srand((unsigned int)time(NULL));
	// 随机字符串: A-Z, a-z, 0-9, 特殊字符(!@#$%^&*()_+-=<>?)
	char chars[] = "!@#$%^&*()_+-=<>?";
	for (int i = 0; i < len - 1; i++)
	{
		flag = rand() % 4;//0~3
		switch (flag)
		{
		case 0:
			randBuf[i] = rand() % 26 + 'a';
			break;
		case 1:
			randBuf[i] = rand() % 26 + 'A';
			break;
		case 2:
			randBuf[i] = rand() % 10 + '0';
			break;
		case 3:
			randBuf[i] = chars[rand() % strlen(chars)];
			break;
		default:
			break;
		}
	}
	randBuf[len - 1] = '\0';
}
