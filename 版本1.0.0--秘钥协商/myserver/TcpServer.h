#pragma once
#include "TcpSocket.h"
#include <netinet/in.h>
#include <arpa/inet.h>

//TCP监听类
class TcpServer
{
public:
	TcpServer();
	~TcpServer();

	//服务端设置监听,默认本地所有IP
	int setListen(unsigned short port);
	//等待接受客户端连接请求，默认连接超时时间为10000s
	//关联TCP通信类，用于收发数据
	TcpSocket* acceptConn(int timeout = 10000);

	//关闭监听
	void closefd();

private:
	int acceptTimeout(int wait_seconds);

private:
	//监听文件描述符--服务端套接字
	int m_lfd;
	struct sockaddr_in m_client_addr;
	ItcastLog m_log;
};
