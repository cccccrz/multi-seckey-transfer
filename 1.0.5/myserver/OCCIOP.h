#pragma once
#include <string>
#include <occi.h>
#include <queue>
#include <vector>
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
	//查找秘钥ID
	int view_SecKeyID(char* clientID, char* serverID);
	//查看秘钥信息
	int view_SecKeyInfo(char* clientID, char* serverID);
	//写入共享内存
	int writeSecKey(NodeSHMInfo* pNode);
	//删除秘钥信息
	int delete_SecKeyInfo(int seckeyID);
	int delete_SecKeyInfo(char* clientID, char* serverID);
	//获得秘钥
	int get_secKey(int seckeyID, char** seckey, int& dataLen);
	//断开数据库连接，关闭环境
	void closeDB();

	//存放查询的秘钥信息
	queue<NodeSHMInfo>m_queue;
	//存放秘钥ID--可以根据用来获取和删除秘钥信息
	vector<int>m_seckeyID;

private:
	string getCurTime();

private:
	//环境
	Environment* m_env;
	//用于连接数据库
	Connection* m_conn;

};
