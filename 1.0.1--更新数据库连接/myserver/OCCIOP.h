#pragma once
#include <string>
#include <occi.h>
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
	//写入共享内存
	int writeSecKey(NodeSHMInfo* pNode);

	//断开数据库连接，关闭环境
	void closeDB();

private:
	string getCurTime();

private:
	//环境
	Environment* m_env;
	//用于连接数据库
	Connection* m_conn;
};
