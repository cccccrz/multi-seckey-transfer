#pragma once
#include "TcpSocket.h"
#include "SecKeyShm.h"

//客户端本地信息
struct ClientInfo
{
	char			clientID[12];		//客户端ID
	//要连接的服务端
	char			serverID[12];		//服务端ID
	char			serverIP[32];	//服务端IP
	unsigned short	serverPort;		//服务端端口号
	//共享内存信息
	int				shmKey;			//共享内存键值
};

//客户端操作类
class ClientOperation
{
public:
	ClientOperation(ClientInfo* info);
	~ClientOperation();

	//密钥协商
	int secKeyAgree();
	//秘钥校验
	int secKeyCheck();
	//秘钥注销
	int secKeyRevoke();
	//秘钥查看
	int secKeyView();

private:
	//获得随机字符串
	void getRandString(int len, char* randBuf);

private:
	//客户端信息
	ClientInfo m_info;
	//通信套接字-收发信息
	TcpSocket m_socket;
	//共享内存操作指针
	SecKeyShm* m_shm;
};
