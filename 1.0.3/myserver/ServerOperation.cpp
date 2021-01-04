#include "ServerOperation.h"
#include <iostream>
#include <pthread.h>
#include <string.h>
#include "RequestFactory.h"
#include "RespondFactory.h"
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <signal.h>
using namespace std;

ServerOperation::ServerOperation(ServerInfo* info)
{
	memcpy(&m_info, info, sizeof(ServerInfo));
	m_shm = new SecKeyShm(info->shmKey, info->maxNode);

	bool ret = m_occi.connectDB("SECMNG", "SECMNG", "192.168.146.1:1521/orcl");
	if (!ret)
	{
		cout << "oracle数据库连接失败......" << endl;
	}
}

ServerOperation::~ServerOperation()
{
	//释放资源
	//m_shm->delShm();
	//delete m_shm;
	m_occi.closeDB();
}

// 服务器开始工作
void ServerOperation::startwork()
{
	//设置监听
	m_server.setListen(m_info.serverPort);

	pthread_t thread;
	//循环接受连接,创建子线程执行任务
	while (1)
	{
		m_socket = m_server.acceptConn(5);
		if (m_socket == NULL)
		{
			//cout << "accept 超时或失败" << endl;
			continue;
		}
		cout << "客户端成功连接服务器..." << endl;

		//将this传入回调函数--查找线程对应的通信类
		pthread_create(&thread, NULL, working, this);

		pthread_detach(thread);
		//将线程ID和通信类存入 map 方便线程查找调用
		m_listSocket.insert(make_pair(thread, m_socket));
	}
}

// 秘钥协商
int ServerOperation::secKeyAgree(RequestMsg* reqMsg, char** outData, int& dataLen)
{
	int ret = 0;
	RespondMsg rspMsg;
	memset(&rspMsg, 0x00, sizeof(RespondMsg));
	//reqMsg->r1[sizeof(reqMsg->r1) - 1] = '\0';	//update 2021.1.3 
#if 0
	//test r1,server粘包问题  + '\0' 不加偶尔粘包？
	cout << "\033[32m开始秘钥协商......" << endl;
	cout << "clientID:\033[0m" << reqMsg->clientID << endl;;
	cout << "\033[32mserverID:\033[0m" << reqMsg->serverID << endl;
	cout << "\033[32mr1:\033[0m" << reqMsg->r1 << endl;
	cout << "\033[31m----------------------------------------------------------\033[0m" << endl;
#endif
	//检验消息认证码--HMAC  @serverID+clientID@,r1 
	char key[1024];
	unsigned int len_HMAC = -1;
	unsigned char md_HMAC[SHA256_DIGEST_LENGTH] = { 0 };
	memset(key, 0x00, sizeof(key));
	sprintf(key, "@%s+%s@", reqMsg->serverID, reqMsg->clientID);
	cout << "[key]==" << key << endl;
	HMAC(EVP_sha256(), key, strlen(key), \
		(unsigned char*)reqMsg->r1, strlen(reqMsg->r1), md_HMAC, &len_HMAC);
	for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
	{
		sprintf(&key[2 * i], "%02x", md_HMAC[i]);
	}

	//cout << "服务端消息认证码：" << key << endl;
	//cout << "客户端消息认证码：" << reqMsg->authCode << endl;
	strcpy(rspMsg.clientID, reqMsg->clientID);
	strcpy(rspMsg.serverID, reqMsg->serverID);
	if (strcmp(key, reqMsg->authCode) != 0)
	{
		cout << "消息认证码错误。。。" << endl;
		rspMsg.rv = 1;
		strcpy(rspMsg.r2, " ");
		rspMsg.seckeyID = 0;

		CodecFactory* factory = new RespondFactory(&rspMsg);
		Codec* pCodec = factory->createCodec();
		pCodec->EncodeMsg(outData, dataLen);

		return -1;
	}
	cout << "消息认证码成功！" << endl;

	//生成秘钥，准备应答信息
	rspMsg.rv = 0;

	//秘钥：SHA1(r1+r2)
	getRandString(sizeof(rspMsg.r2), rspMsg.r2);
	char buf[1024] = { 0 };
	unsigned char md_SHA[SHA_DIGEST_LENGTH] = { 0 };
	sprintf(buf, "%s%s", reqMsg->r1, rspMsg.r2);
	SHA1((unsigned char*)buf, strlen(buf), md_SHA);
	char secKey[SHA_DIGEST_LENGTH * 2 + 1] = { 0 };
	for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
	{
		sprintf(&secKey[i * 2], "%02x", md_SHA[i]);
	}
	cout << "秘钥：" << secKey << endl;

	//查找数据库，给秘钥一个ID
	rspMsg.seckeyID = m_occi.getSecKeyID();
	cout << "seckeyID:" << rspMsg.seckeyID << endl;

	//秘钥结构体赋值
	NodeSHMInfo node;
	node.status = 0;
	node.seckeyID = rspMsg.seckeyID;
	strcpy(node.clientID, rspMsg.clientID);
	strcpy(node.serverID, rspMsg.serverID);
	strcpy(node.seckey, secKey);

	//编码发送应答数据
	CodecFactory* factory = new RespondFactory(&rspMsg);
	Codec* pCodec = factory->createCodec();
	pCodec->EncodeMsg(outData, dataLen);

	//写入数据库
	cout << "秘钥写入数据库......" << endl;;
	ret = m_occi.writeSecKey(&node);
	if (!ret)
	{
		cout << "err:" << ret << endl;
	}
	else
	{
		cout << "done!" << endl;
	}

	//写入共享内存
	cout << "正在写入共享内存......";
	ret = m_shm->shmWrite(&node);
	if (ret != 0)
	{
		cout << "err:" << ret << endl;
	}
	else
	{
		cout << "done!" << endl;
	}

	//释放资源
	//delete factory;

	return 0;
}

// 秘钥校验
int ServerOperation::secKeyCheck(RequestMsg* reqMsg, char** outData, int& dataLen)
{

}

// 秘钥注销
int ServerOperation::secKeyRevoke()
{}

// 秘钥查看
int ServerOperation::secKeyView(RequestMsg* reqMsg, char** outData, int& outLen)
{
	int ret = 0;

	pthread_t thread = pthread_self();
	TcpSocket* socket = m_listSocket[thread];
	RespondMsg rspMsg;
	char* sendData = NULL;
	int dataLen = 0;
	memset(&rspMsg, 0x00, sizeof(RespondMsg));
	CodecFactory* factory = NULL;
	Codec* pCodec = NULL;

	//从数据库中查找
	//reqMsg->clientID reqMsg->serverID
	m_occi.view_SecKeyInfo(reqMsg->clientID, reqMsg->serverID);

	cout << "\033[33m开始查找秘钥信息#########################################\033[0m" << endl;
	if (m_occi.m_queue.empty())
	{
		rspMsg.rv = 1;//没找到
		printf("\033[31mclientID[%s]:serverID[%s]\033[0m没有秘钥信息！\n", reqMsg->clientID, reqMsg->serverID);
	}
	else
	{
		rspMsg.rv = 0;
	}
	while (!m_occi.m_queue.empty())
	{
#if 0
		//test
		cout << "client:" << m_occi.m_queue.front().clientID << endl;
		cout << "serverID:" << m_occi.m_queue.front().serverID << endl;
		cout << "seckey:" << m_occi.m_queue.front().seckey << endl;
		cout << "seckeyID:" << m_occi.m_queue.front().seckeyID << endl;
		cout << "status:" << m_occi.m_queue.front().status << endl;
#endif

		/*组织应答信息*/
		cout << "组织应答信息......" << endl;
		rspMsg.seckeyID = m_occi.m_queue.front().seckeyID;
		strcpy(rspMsg.r2, " ");
		strcpy(rspMsg.clientID, " ");
		strcpy(rspMsg.serverID, " ");
		/*编码发送*/
		factory = new RespondFactory(&rspMsg);
		pCodec = factory->createCodec();
		pCodec->EncodeMsg(&sendData, dataLen);	//传出参数
		//发送
		cout << "发送应答数据......";
		ret = socket->sendMsg(sendData, dataLen);
		if (ret < 0)
		{
			cout << "\033[31merr\033[0m:" << ret << endl;
		}
		else
		{
			cout << "\033[32mdone！\033[0m" << endl;
		}
		cout << "\033[33m#########################################################\033[0m" << endl;

		//出队
		m_occi.m_queue.pop();
	}
	/*结束发送一个空包作为标志--seckeyID==0*/
	cout << "组织结束信息。。。。。。" << endl;
	rspMsg.seckeyID = 0;
	strcpy(rspMsg.r2, " ");
	strcpy(rspMsg.clientID, " ");
	strcpy(rspMsg.serverID, " ");
	/*编码*/
	factory = new RespondFactory(&rspMsg);
	pCodec = factory->createCodec();
	pCodec->EncodeMsg(outData, outLen);	//传出参数

	//delete socket;
	return 0;
}

void ServerOperation::errInfo(RequestMsg* reqMsg, char** outData, int& dataLen)
{
	RespondMsg rspMsg;
	memset(&rspMsg, 0x00, sizeof(RespondMsg));
	rspMsg.rv = 404;
	rspMsg.seckeyID = 0;
	strcpy(rspMsg.clientID, reqMsg->clientID);
	strcpy(rspMsg.serverID, reqMsg->serverID);
	strcpy(rspMsg.r2, " ");


	CodecFactory* factory = new RespondFactory(&rspMsg);
	Codec* pCodec = factory->createCodec();
	pCodec->EncodeMsg(outData, dataLen);
}

void ServerOperation::getRandString(int len, char* randBuf)
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

//线程回调函数
//执行任务：读取客户端请求信息，执行任务，恢复应答信息
//友元函数：使用操作类实例调用map 找到线程对应的 通信类
void* working(void* arg)
{
	cout << "working..." << endl;
	int ret = 0;
	pthread_t thread = pthread_self();
	ServerOperation* op = (ServerOperation*)arg;
	TcpSocket* socket = op->m_listSocket[thread];

	char* recvData;
	int dataLen;

	//接受请求数据
	cout << "接收数据......\t";
	ret = socket->recvMsg(&recvData, dataLen);
	if (ret != 0)
	{
		cout << "err:" << ret << endl;
		//断开连接
		socket->disConnect();
		printf("断开客户端的连接......\n");
		auto it = op->m_listSocket.find(thread);
		op->m_listSocket.erase(it);
		delete socket;
		cout << "working over..." << endl;
		cout << "--------------------------------------------------" << endl;

		return NULL;
	}
	cout << "done!" << endl;

	//解码
	CodecFactory* factory = new RequestFactory();
	Codec* pCodec = factory->createCodec();
	RequestMsg* reqMsg = (RequestMsg*)pCodec->DecodeMsg(recvData, dataLen);
	//delete factory;

	//判断客户端ID合法性--在数据库中查找
	bool bl = op->m_occi.checkClinetID(reqMsg->clientID);
	if (!bl)
	{
		printf("客户端%s不允许服务！\n", reqMsg->clientID);
		//断开连接
		socket->disConnect();
		printf("断开客户端%s的连接......\n", reqMsg->clientID);
		auto it = op->m_listSocket.find(thread);
		op->m_listSocket.erase(it);
		cout << "working over..." << endl;
		cout << "--------------------------------------------------" << endl;

		return NULL;
	}
	printf("客户端%s允许服务！\n", reqMsg->clientID);

	//判断请求类型,准备应答信息
	char* sendData = NULL;
	cout << "请求类型为：" << reqMsg->cmdType << endl;
	switch (reqMsg->cmdType)
	{
	case 1://秘钥协商
		ret = op->secKeyAgree(reqMsg, &sendData, dataLen);
		if (ret < 0)
		{
			cout << "秘钥协商失败！" << endl;
		}
		break;
	case 2:
		//op->secKeyCheck(reqMsg, &sendData, dataLen);
		break;
	case 3:
		op->secKeyRevoke();
		break;
	case 4:
		//秘钥查看
		op->secKeyView(reqMsg, &sendData, dataLen);
		break;
	default:
		cout << "没有该服务......" << endl;
		op->errInfo(reqMsg, &sendData, dataLen);
		break;
	}

	cout << "发送应答数据......";
	ret = socket->sendMsg(sendData, dataLen);
	if (ret < 0)
	{
		cout << "\033[31merr\033[0m:" << ret << endl;
	}
	else
	{
		cout << "\033[32mdone！\033[0m" << endl;
	}

	//断开连接
	socket->disConnect();
	printf("断开客户端%s的连接......\n", reqMsg->clientID);
	//释放内存
	delete factory;

	//线程任务完成，自动释放
	//更新 map ，删除对应键值对
	auto it = op->m_listSocket.find(thread);
	op->m_listSocket.erase(it);
	delete socket;
	cout << "working over..." << endl;
	cout << "--------------------------------------------------" << endl;

	return NULL;
}
