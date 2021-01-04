#pragma once
#include "TcpSocket.h"
#include "SecKeyShm.h"
#include "TcpServer.h"
#include "RespondCodec.h"
#include "RequestCodec.h"
#include "OCCIOP.h"
#include <map>


//服务端本地信息
struct ServerInfo
{
	//本地信息
	char			serverID[12];	//编号
	unsigned short	serverPort;		//端口
	//共享内存
	int				maxNode;		//最大网点数
	int				shmKey;			//共享内存键值
	//数据库信息
	char			dbUser[24];		//数据库用户名
	char			dbPasswd[24];	//数据库密码
	char			dbSID[24];		//数据库SID
};

class ServerOperation
{
public:
	ServerOperation(ServerInfo* info);
	~ServerOperation();

	//服务端开始工作
	void startwork();

	// 秘钥协商--传入请求信息，传出应答信息
	int secKeyAgree(RequestMsg* reqMsg, char** outData, int& dataLen);
	// 秘钥校验
	int secKeyCheck(RequestMsg* reqMsg, char** outData, int& dataLen);
	// 秘钥注销
	int secKeyRevoke();
	// 秘钥查看
	int secKeyView(RequestMsg* reqMsg, char** outData, int& dataLen);
	int secKeyView(RequestMsg* reqMsg);
	// 传入信息无效
	void errInfo(RequestMsg* reqMsg, char** outData, int& dataLen);

	//线程回调函数--友元：访问类属性
	friend void* working(void* arg);

	//信号捕捉回调函数
	static void catchSignal(int num);

private:
	void getRandString(int len, char* randBuf);

private:
	//本地信息
	ServerInfo m_info;
	//操作共享内存
	SecKeyShm* m_shm;
	//通信
	TcpServer m_server;		//本地监听类
	TcpSocket* m_socket;	//关联通信类
	//数据库操作
	OCCIOP m_occi;

	//线程管理:map 存储 线程ID：通信类
	std::map<pthread_t, TcpSocket*> m_listSocket;

};

//线程回调函数
void* working(void* arg);
