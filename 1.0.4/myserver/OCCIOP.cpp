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
		cout << "oracle数据库连接失败......" << endl;
		return false;
	}
	cout << "oracle数据库连接成功......" << endl;

	return true;
}

bool OCCIOP::checkClinetID(char* clientID)
{
	bool bl;

	string sql = "select * from secmng.secnode where id=:1 and state=0";
	Statement* stmt = m_conn->createStatement(sql);
	stmt->setString(1, clientID);
	ResultSet* resSet = stmt->executeQuery();

	if (resSet->next())
	{
		bl = true;
	}
	else
	{
		bl = false;
	}

	m_conn->terminateStatement(stmt);

	return bl;
}

//获取新的秘钥ID  -- 返回keyID
int OCCIOP::getSecKeyID()
{
	//查询数据库 for update 手动加锁，防止多点接入同时访问	
	//--keysn序列号 该表只有一个序号，主键
	string sql = " select ikeysn from SECMNG.KEYSN for update ";
	Statement* stmt = m_conn->createStatement(sql);
	//执行
	ResultSet* resSet = stmt->executeQuery();
	int keyID = -1;
	if (resSet->next())
	{
		keyID = resSet->getInt(1);//
	}
	m_conn->terminateStatement(stmt);
	//更新keyID
	update_SecKeyID(keyID + 1);

	return keyID;
}

// 秘钥ID在插入的时候会自动更新, 也可以手动更新
int OCCIOP::update_SecKeyID(int seckeyID)
{
	//查询数据库
	string sql = " update SECMNG.KEYSN set ikeysn = " + to_string(seckeyID);
	Statement* stmt = m_conn->createStatement(sql);
	//设置自动提交
	stmt->setAutoCommit(true);
	//执行
	int ret = stmt->executeUpdate();
	m_conn->terminateStatement(stmt);

	return ret;
}

// 将生成的秘钥写入数据库 --插入SECMNG.SECKEYINFO表
//SECMNG.SECKEYINFO(clientid, serverid, keyid, createtime, state, seckey)
int OCCIOP::writeSecKey(NodeSHMInfo* pNode)
{
	//插入SECMNG.SECKEYINFO表
	char sql[1024] = { 0 };
	sprintf(sql, "Insert Into SECMNG.SECKEYINFO values ('%s', '%s', %d, to_date('%s', 'yyyy-mm-dd hh24:mi:ss') , %d, '%s') ",
		pNode->clientID, pNode->serverID, pNode->seckeyID,
		getCurTime().data(), 0, pNode->seckey);
	//测试
	//cout << "insert sql: " << sql << endl;
	Statement* stmt = m_conn->createStatement();
	//数据初始化
	stmt->setSQL(sql);
	//执行
	stmt->setAutoCommit(true);
	int ret = stmt->executeUpdate(sql);
	m_conn->terminateStatement(stmt);

	return ret;
}

//根据clientID 和 serverID 查找秘钥信息 --查找SECMNG.SECKEYINFO表
int OCCIOP::view_SecKeyInfo(char* clientID, char* serverID)
{
	string sql = "select * from SECMNG.SECKEYINFO where clientid=:1 and serverid=:2";
	Statement* stmt = m_conn->createStatement(sql);
	stmt->setString(1, clientID);
	stmt->setString(2, serverID);
	//查询
	ResultSet* resSet = stmt->executeQuery();
	NodeSHMInfo nodeTmp;
	while (resSet->next() == true)//找到了
	{
		//将数据保留到队列中
		memset(&nodeTmp, 0x00, sizeof(NodeSHMInfo));
		strcpy(nodeTmp.clientID, resSet->getString(1).data());
		strcpy(nodeTmp.serverID, resSet->getString(2).data());
		nodeTmp.seckeyID = resSet->getInt(3);
		nodeTmp.status = resSet->getInt(5);
		strcpy(nodeTmp.seckey, resSet->getString(6).data());
		m_queue.push(nodeTmp);
#if 0
		//test
		cout << "\033[32mclientID:\033[0m" << resSet->getString(1) << endl;;
		cout << "\033[32mserverID:\033[0m" << resSet->getString(2) << endl;
		cout << "\033[32mkeyID:\033[0m" << resSet->getInt(3) << endl;
		//string date = resSet->getDate(4).toText("YYYYMMDD");
		cout << "\033[32mcreatetime:\033[0m" << resSet->getDate(4).toText("YYYY-MM-DD") << endl;
		cout << "\033[32mstate:\033[0m" << resSet->getInt(5) << endl;
		cout << "\033[32mseckey:\033[0m" << resSet->getString(6) << endl;
		cout << "\033[31m----------------------------------------------------------\033[0m" << endl;
#endif
	}

	m_conn->terminateStatement(stmt);

	return 0;
}

//根据seckeyID 删除秘钥信息 -- 删除SECMNG.SECKEYINFO表数据
int OCCIOP::delete_SecKeyInfo(int seckeyID)
{
	//删除SECMNG.SECKEYINFO表数据
	string sql = " delete from SECMNG.SECKEYINFO where seckeyid=" + to_string(seckeyID);
	Statement* stmt = m_conn->createStatement(sql);
	//设置自动提交
	stmt->setAutoCommit(true);
	//执行
	int ret = stmt->executeUpdate();
	m_conn->terminateStatement(stmt);

	//清空queue
	while (!m_queue.empty())
	{
		m_queue.pop();
	}

	return ret;
}
//根据clientID 和 serverID 删除秘钥信息
int OCCIOP::delete_SecKeyInfo(char* clientID, char* serverID)
{
	//删除SECMNG.SECKEYINFO表数据
	string sql = " delete from SECMNG.SECKEYINFO where serverid=:1 and clientid=:2";
	Statement* stmt = m_conn->createStatement(sql);
	stmt->setString(1, serverID);
	stmt->setString(2, clientID);
	//设置自动提交
	stmt->setAutoCommit(true);
	//执行
	int ret = stmt->executeUpdate();
	m_conn->terminateStatement(stmt);

	//清空queue
	while (!m_queue.empty())
	{
		m_queue.pop();
	}

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
