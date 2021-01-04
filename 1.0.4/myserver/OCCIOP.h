#pragma once
#include <string>
#include <occi.h>
#include <queue>
#include "SecKeyShm.h"
using namespace std;
using namespace oracle::occi;

class OCCIOP
{
public:
	OCCIOP();
	~OCCIOP();

	//初始化环境，连接数据库
	bool connectDB(string user, string passwd, string sid);

	//校验客户端ID的合法性
	bool checkClinetID(char* clientID);

	//获取秘钥ID
	int getSecKeyID();
	//更新秘钥ID
	int update_SecKeyID(int seckeyID);
	//查看秘钥信息
	int view_SecKeyInfo(char* clientID, char* serverID);
	//写入共享内存
	int writeSecKey(NodeSHMInfo* pNode);
	//删除秘钥信息
	int delete_SecKeyInfo(int seckeyID);
	int delete_SecKeyInfo(char* clientID, char* serverID);

	//断开数据库连接，关闭环境
	void closeDB();

	queue<NodeSHMInfo>m_queue;
private:
	string getCurTime();

private:
	//环境
	Environment* m_env;
	//用于连接数据库
	Connection* m_conn;

};
