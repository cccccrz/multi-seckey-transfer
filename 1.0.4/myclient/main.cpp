#include <string.h>
#include <stdlib.h>
#include <iostream>
#include "RequestCodec.h"
#include "RespondCodec.h"
#include "RequestFactory.h"
#include "RespondFactory.h"
#include "ClientOperation.h"
#include <sys/ipc.h>
#include <sys/shm.h>
using namespace std;

#if 0
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
//编解码测试 -- done
void codeTest()
{
	RequestMsg reqMsg;
	memset(&reqMsg, 0x00, sizeof(RequestMsg));
	strcpy(reqMsg.clientID, "111111");
	strcpy(reqMsg.serverID, "222222");
	strcpy(reqMsg.authCode, "lppmcvosoois9349q4q0r0qwr");
	strcpy(reqMsg.r1, "saodasijdjafdg5673534saknsf");
	reqMsg.cmdType = 1;

	char* outData;
	int outLen;
	//编码
	CodecFactory* factory = new RequestFactory(&reqMsg);
	Codec* c = factory->createCodec();
	c->EncodeMsg(&outData, outLen);

	//解码
	factory = new RequestFactory();
	c = factory->createCodec();
	RequestMsg* m = (RequestMsg*)c->DecodeMsg(outData, outLen);

	cout << "clientID==" << m->clientID << endl;
	cout << "serverID==" << m->serverID << endl;
	cout << "authCode==" << m->authCode << endl;
	cout << "r1==" << m->r1 << endl;
	cout << "cmdType==" << m->cmdType << endl;
}
#endif

int usage();

int main()
{
	ClientInfo info;
	memset(&info, 0x00, sizeof(ClientInfo));
	strcpy(info.clientID, "1111");
	strcpy(info.serverID, "0001");
	strcpy(info.serverIP, "127.0.0.1");
	info.serverPort = 9898;
	info.shmKey = 0x12345678;
	ClientOperation client(&info);

	//enum CmdType{NewOrUpdate=1, Check, Revoke, View};
	int nSel;
	while (1)
	{
		nSel = usage();
		switch (nSel)
		{
		case RequestCodec::NEWORUPDATE:
			client.secKeyAgree();
			break;

		case RequestCodec::CHECK:
			client.secKeyCheck();
			break;
		case RequestCodec::REVOKE:
			client.secKeyRevoke();
			break;
		case RequestCodec::VIEW:
			client.secKeyView();
			break;
		case 0:
			exit(0);

		default:
			break;
		}
	}
}

int usage()
{
	int nSel = -1;
	printf("\n  /*************************************************************/");
	printf("\n  /*************************************************************/");
	printf("\n  /*     1.密钥协商                                            */");
	printf("\n  /*     2.密钥校验                                            */");
	printf("\n  /*     3.密钥注销                                            */");
	printf("\n  /*     4.密钥查看                                            */");
	printf("\n  /*     0.退出系统                                            */");
	printf("\n  /*************************************************************/");
	printf("\n  /*************************************************************/");
	printf("\n\n  选择:");

	scanf("%d", &nSel);
	while (getchar() != '\n');

	return nSel;
}
