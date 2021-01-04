#include "TcpServer.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

TcpServer::TcpServer()
{
}


TcpServer::~TcpServer()
{
}

//监听
int TcpServer::setListen(unsigned short port)
{
	int ret = 0;

	m_lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_lfd < 0)
	{
		ret = errno;
		m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "func socket() err");
		return ret;
	}

	int opt = 1;
	ret = setsockopt(m_lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
	if (ret < 0)
	{
		ret = errno;
		m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "func setsockopt() err");
		return ret;
	}

	struct sockaddr_in servaddr;
	memset(&servaddr, 0x00, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	ret = bind(m_lfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
	if (ret < 0)
	{
		ret = errno;
		m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "func bind() err");
		return ret;
	}

	ret = listen(m_lfd, SOMAXCONN);
	if (ret < 0)
	{
		ret = errno;
		m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "func listen() err");
		return ret;
	}

	return ret;
}

TcpSocket* TcpServer::acceptConn(int timeout)
{

	int connfd = acceptTimeout(timeout);
	if (connfd < 0)
	{
		if (connfd == -1 && errno == ETIMEDOUT)
		{
			//printf("func accept_timeout() timeout err:%d \n", ret);
			m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, connfd, "func acceptConn() TimeOutError");
		}
		else
		{
			m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, connfd, "func acceptConn() OtherError");
		}
		return NULL;
	}

	return new TcpSocket(connfd);//记得释放
}

void TcpServer::closefd()
{
	if (m_lfd >= 0)
	{
		close(m_lfd);
	}
}

int TcpServer::acceptTimeout(int wait_seconds)
{
	int ret = 0;
	socklen_t len = sizeof(struct sockaddr_in);

	if (wait_seconds > 0)
	{
		fd_set accept_fdset;
		FD_ZERO(&accept_fdset);
		FD_SET(m_lfd, &accept_fdset);
		struct timeval timeout;
		timeout.tv_sec = wait_seconds;
		timeout.tv_usec = 0;

		//连接超时检测
		do
		{
			//监听读事件；有连接请求到来--读
			ret = select(m_lfd + 1, &accept_fdset, NULL, NULL, &timeout);
		} while (ret < 0 && errno == EINTR);
		if (ret < 0)
		{
			return -1;
		}
		else if (ret == 0)
		{
			errno = ETIMEDOUT;
			return -1;
		}
	}
	//设置超时且有连接请求,accept不阻塞
	ret = accept(m_lfd, (struct sockaddr*)&m_client_addr, &len);
	if (ret < 0)
	{
		ret = errno;
		m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "func accept() err");
		return ret;
	}

	return ret;
}
