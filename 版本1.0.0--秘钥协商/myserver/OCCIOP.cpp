#include "OCCIOP.h"
#include <iostream>
#include <string.h>
#include <time.h>
using namespace std;
using namespace oracle::occi;

OCCIOP::OCCIOP()
{
}


OCCIOP::~OCCIOP()
{
}

bool OCCIOP::connectDB(string user, string passwd, string sid)
{
	//初始化环境
	m_env = Environment::createEnvironment(Environment::DEFAULT);
	//创建连接
	m_conn = m_env->createConnection(user, passwd, sid);
	if (m_conn == NULL)
	{
		return false;
	}
	cout << "oracle数据库连接成功......" << endl;

	return true;
}

//获取新的秘钥ID  -- 返回keyID
int OCCIOP::getSecKeyID()
{
	//查询数据库 for update 手动加锁，防止多点接入同时访问	
	//--keysn序列号 该表只有一个序号，主键
	string sql = "select ikeysn from SECMNG.KEYSN for update";
	Statement* stmt = m_conn->createStatement(sql);
	//执行
	ResultSet* resSet = stmt->executeQuery();
	int keyID = -1;
	if (resSet->next())
	{
		keyID = resSet->getInt(1);//
	}
	m_conn->terminateStatement(stmt);

	return keyID;
}

// 秘钥ID在插入的时候会自动更新, 也可以手动更新
int OCCIOP::update_SecKeyID(int seckeyID)
{
	//查询数据库
	string sql = "update SECMNG.KEYSN set ikeysn = " + to_string(seckeyID);
	Statement* stmt = m_conn->createStatement(sql);
	//设置自动提交
	stmt->setAutoCommit(true);
	//执行
	int ret = stmt->executeUpdate();
	m_conn->terminateStatement(stmt);

	return ret;
}

// 将生成的秘钥写入数据库-秘钥信息表
//SECMNG.SECKEYINFO(clientid, serverid, keyid, createtime, state, seckey)
int OCCIOP::writeSecKey(NodeSHMInfo* pNode)
{
	//插入SECMNG.SECKEYINFO表
	char sql[1024] = { 0 };
	sprintf(sql, "Insert Into SECMNG.SECKEYINFO values ('%s', '%s', %d, to_date('%s', 'yyyy-mm-dd hh24:mi:ss') , %d, '%s') ",
		pNode->clientID, pNode->serverID, pNode->seckeyID,
		getCurTime().data(), 0, pNode->seckey);
	//测试
	cout << "insert sql: " << sql << endl;
	Statement* stmt = m_conn->createStatement();
	//数据初始化
	stmt->setSQL(sql);
	//执行
	stmt->setAutoCommit(true);
	int ret = stmt->executeUpdate(sql);
	m_conn->terminateStatement(stmt);

	return ret;
}

void OCCIOP::closeDB()
{
	//关闭连接
	m_env->terminateConnection(m_conn);
	//关闭环境
	Environment::terminateEnvironment(m_env);
}

string OCCIOP::getCurTime()
{
	time_t timep;
	time(&timep);
	char tmp[64];
	strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&timep));

	return tmp;
}
