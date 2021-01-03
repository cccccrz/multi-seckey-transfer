#include "TcpSocket.h"
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

TcpSocket::TcpSocket()
{
}


TcpSocket::TcpSocket(int connfd)
{
	m_socket = connfd;
}

TcpSocket::~TcpSocket()
{
}

//连接服务器,返回通信套接字
int TcpSocket::connectToHost(char* ip, unsigned short port, int timeout)
{
	int ret = 0;
	//输入参数检查，确保建立连接
	if (ip == NULL || port <= 0 || port > 65535 || timeout < 0)
	{
		ret = ParamError;//输入参数错误
		m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "func sckClient_connect() err, check  (ip==NULL || connfd==NULL || port<=0 || port>65537 || connecttime < 0)");
		return ret;
	}

	//创建套接字
	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket < 0)
	{
		ret = errno;
		printf("func socket() err:%d\n", ret);
		return ret;
	}

	//准备服务端地址
	struct sockaddr_in server_addr;
	memset(&server_addr, 0x00, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(ip);
	//连接检测
	ret = connectTimeout(&server_addr, TIMEOUT);
	if (ret < 0)//连接失败
	{
		if (ret == -1 && errno == ETIMEDOUT)//连接超时
		{
			ret = TimeoutError;//err:3002
			return ret;
		}
		else//函数出错
		{
			m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "func connect_timeout() err");
		}
	}

	return ret;
}

//发送数据 - 字节流 ret<0 失败
int TcpSocket::sendMsg(char* sendData, int dataLen, int timeout)
{
	int ret = 0;
	if (sendData == NULL || dataLen <= 0)
	{
		ret = ParamError;
		m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "func sckClient_send() err, check (data == NULL || datalen <= 0)");
		return ret;
	}

	ret = writeTimeout(timeout);
	if (ret < 0)
	{
		if (ret == -1 && errno == ETIMEDOUT)
		{
			ret = TimeoutError;
			printf("func sckClient_send() timeout Err:%d\n ", ret);
			return ret;
		}
		return ret;
	}

	//发送字节流:为防止粘包，设定发送方式【4字节（数据长度）+数据】
	//ret记录发送字节数
	//重新分配数据，长度+数据
	unsigned char* netdata = (unsigned char*)malloc(dataLen + 4);
	if (netdata == NULL)
	{
		ret = MallocError;//分配内存失败
		printf("func sckClient_send() malloc Err:%d\n ", ret);
		return ret;
	}
	int netLen = htonl(dataLen);//转成大端传输
	memcpy(netdata, &netLen, 4);
	memcpy(netdata + 4, sendData, dataLen);

	//发送字节流
	int n = writen(netdata, dataLen + 4);
	if (n < dataLen + 4)//发送数据缺失？
	{
		if (netdata != NULL) //释放内存
		{
			free(netdata);
			netdata = NULL;
		}
		return n;
	}
	if (netdata != NULL)//发送完成释放内存
	{
		free(netdata);
		netdata = NULL;
	}

	return ret;
}

//接受数据  ret<0 失败
int TcpSocket::recvMsg(char** recvData, int& recvLen, int timeout)
{
	int ret = 0;
	if (recvData == NULL)
	{
		ret = ParamError;
		printf("func sckClient_rev(), err:%d  check(recvData == NULL)\n", ParamError);
		return ret;
	}

	ret = readTimeout(timeout);
	if (ret < 0)
	{
		if (ret == -1 && errno == ETIMEDOUT)
		{
			m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "func read_timeout() timeout");
			return ret;
		}
		else
		{
			m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "func read_timeout() err");
			return ret;
		}
	}

	int netdatalen = 0;
	ret = readn(&netdatalen, 4);//读包头4个字节-数据长度
	if (ret == -1)
	{
		//printf("func readn() err:%d \n", ret);
		m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "func readn() err");
		return ret;
	}
	else if (ret < 4)//对方异常关闭错误 
	{
		ret = PeerCloseError;
		//printf("func readn() err peer closed:%d \n", ret);
		m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "func readn() err, peer closed");
		return ret;
	}

	int datalen;
	datalen = ntohl(netdatalen);//转成小端字节序-得到数据长度
	char* tmpBuf = (char*)malloc(datalen + 1);//重新分配空间存放数据
	if (tmpBuf == NULL)
	{
		ret = MallocError;
		m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "malloc() err");
		return ret;
	}

	ret = readn(tmpBuf, datalen);//根据长度读数据
	if (ret == -1)
	{
		m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "readn() err");
		return ret;
	}
	else if (ret < datalen)
	{
		ret = PeerCloseError;
		//printf("func readn() err peer closed:%d \n", ret);
		m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "func readn() err,  peer closed");
		return ret;
	}

	tmpBuf[datalen] = '\0';//多分配一个字节内容，兼容可见字符串 字符串的真实长度仍然为datalen
	//传出参数赋值
	*recvData = tmpBuf;
	recvLen = datalen;

	return ret;
}

void TcpSocket::disConnect()
{
	if (m_socket >= 0)
	{
		close(m_socket);
	}
}
void TcpSocket::freeMemory(char** buf)
{
	if (*buf != NULL)
	{
		free(*buf);
		*buf = NULL;
	}
}

/////////////////////////////////////////////////
//////             子函数                   //////
/////////////////////////////////////////////////
/*
* NoneBlockIO - 设置I/O为非阻塞模式
* @fd: 文件描符符
*/
int TcpSocket::NoneBlockIO(int fd)
{
	int ret = 0;
	int flag = fcntl(fd, F_GETFL);//获取文件属性标志
	if (flag == -1)
	{
		m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, flag, "func activate_nonblock() err");
		return -1;
	}

	flag |= O_NONBLOCK;				//设置非阻塞
	ret = fcntl(fd, F_SETFL, flag);
	if (ret == -1)
	{
		m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "func activate_nonblock() err");
		return ret;
	}
}

/*
* BlockIO - 设置I/O为阻塞模式
* @fd: 文件描符符
*/
int TcpSocket::BlockIO(int fd)
{
	int ret = 0;
	int flag = fcntl(fd, F_GETFL);//获取文件属性标志
	if (flag == -1)
	{
		m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, flag, "func activate_block() err");
		return -1;
	}

	flag &= ~O_NONBLOCK;		   //设置阻塞
	ret = fcntl(fd, F_SETFL, flag);
	if (ret == -1)
	{
		m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret, "func activate_block() err");
		return ret;
	}
}

/*
* readTimeout - 读超时检测函数，不含读操作
* @wait_seconds: 等待超时秒数，如果为0表示不检测超时
* 成功（未超时）返回0，失败返回-1，超时返回-1并且errno = ETIMEDOUT
*/
int TcpSocket::readTimeout(unsigned int wait_seconds)
{
	int ret = 0;

	if (wait_seconds > 0)
	{
		fd_set read_fdset;
		FD_ZERO(&read_fdset);
		FD_SET(m_socket, &read_fdset);
		struct timeval timeout;
		timeout.tv_sec = wait_seconds;
		timeout.tv_usec = 0;

		//超时检测
		do
		{
			ret = select(m_socket + 1, &read_fdset, NULL, NULL, &timeout);
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
		//读事件发生
		//return 0;
	}
	//读事件发生 或 没有设置超时
	return 0;
}

/*
* writeTimeout - 写超时检测函数，不含写操作
* @wait_seconds: 等待超时秒数，如果为0表示不检测超时
* 成功（未超时）返回0，失败返回-1，超时返回-1并且errno = ETIMEDOUT
*/
int TcpSocket::writeTimeout(unsigned int wait_seconds)
{
	int ret = 0;

	if (wait_seconds > 0)
	{
		fd_set write_fdset;
		FD_ZERO(&write_fdset);
		FD_SET(m_socket, &write_fdset);
		struct timeval timeout;
		timeout.tv_sec = wait_seconds;
		timeout.tv_usec = 0;

		//超时检测
		do
		{
			ret = select(m_socket + 1, NULL, &write_fdset, NULL, &timeout);
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
	//写事件发生 或 没有设置超时
	return 0;
}

/*
* connectTimeout - connect
* @addr: 要连接的对方地址
* @wait_seconds: 等待超时秒数，如果为0表示正常模式
* 成功（未超时）返回0，失败返回-1，超时返回-1并且errno = ETIMEDOUT
*/
int TcpSocket::connectTimeout(sockaddr_in* addr, unsigned int wait_seconds)
{
	int ret = 0;
	socklen_t addrlen = sizeof(struct sockaddr_in);

	if (wait_seconds > 0)
	{
		NoneBlockIO(m_socket);//有超时检测，设置非阻塞
	}

	//连接服务端
	//已经将套接口设置为非阻塞了，如果不能够立即连接成功，则直接返回EINPROGRESS错误。
	ret = connect(m_socket, (struct sockaddr*)addr, addrlen);
	if (ret < 0 && errno == EINPROGRESS)//进入超时检测处理
	{
		fd_set connect_fdset;	//文件描述符集
		struct timeval timeout;	//超时时间

		FD_ZERO(&connect_fdset);
		FD_SET(m_socket, &connect_fdset);//准备监听通信套接字
		timeout.tv_sec = wait_seconds;
		timeout.tv_usec = 0;

		do
		{
			//一旦连接建立，套接字就可写；监听写事件
			ret = select(m_socket + 1, NULL, &connect_fdset, NULL, &timeout);
		} while (ret < 0 && errno == EINTR);//忽略中断信号
		//处理超时情况
		if (ret < 0)//select失败
		{
			return -1;
		}
		else if (ret == 0)//超时
		{
			errno = ETIMEDOUT;
			return -1;
		}
		else if (ret == 1)
		{  /*ret返回1，可能有两种情况。一种是fd有事件发生，connect建立连接可写了。
			*另一种情况是套接字本身产生错误.套接口上发生一个错误待处理，错误可以通过getsockopt指定SO_ERROR选项来获取
			*但是select函数没有出错，所以错误信息不能保存到errno
			*变量中。只有通过getsockopt来获取套接口fd的错误。
			*/
			int err;	//接收错误编号
			socklen_t socklen = sizeof(err);
			int sockoptret = getsockopt(m_socket, SOL_SOCKET, SO_ERROR, &err, &socklen);
			if (sockoptret == -1)//获取失败
			{
				return -1;
			}
			if (err == 0)//没有错误；已建立连接
			{
				ret = 0;
			}
			else//套接字错误
			{
				errno = err;
				ret = -1;
			}

		}
	}

	if (wait_seconds > 0)
	{
		BlockIO(m_socket);//处理完超时，设置阻塞
	}

	return ret;
}

/*
* readn - 读取固定字节数
* @fd: 文件描述符
* @buf: 接收缓冲区
* @count: 要读取的字节数
* 成功返回读取的字节数，失败返回-1，读到EOF返回<count
*/
int TcpSocket::readn(void* buf, int count)
{
	size_t nleft = count;
	ssize_t nread;				//一次读取的字节数
	char* bufp = (char*)buf;	//读缓冲区

	while (nleft > 0)
	{
		if ((nread = read(m_socket, bufp, nleft)) < 0) //nread<0
		{
			if (errno == EINTR)//信号中断继续读
			{
				continue;
			}
			return -1;
		}
		else if (nread == 0)	//读完了
		{
			return count - nleft;//返回读取字节数
		}
		bufp += nread;			//缓冲区首地址后移
		nleft -= nread;			//剩余要读取的字节数
	}
}

/*
* writen - 发送固定字节数
* @buf: 发送缓冲区
* @count: 要写的字节数
* 成功返回已写的字节数，失败返回-1
*/
int TcpSocket::writen(const void* buf, int count)
{
	size_t nleft = count;
	ssize_t nwrite;				//一次读取的字节数
	char* bufp = (char*)buf;	//读缓冲区

	while (nleft > 0)
	{
		if ((nwrite = write(m_socket, bufp, nleft)) < 0) //nwrite<0
		{
			if (errno == EINTR)//忽略中断信号
			{
				continue;
			}
			return -1;
		}
		else if (nwrite == 0)	//写了0个字节？ 继续写，写完跳出循环
		{
			continue;
		}
		bufp += nwrite;			//缓冲区首地址后移
		nleft -= nwrite;			//剩余要读取的字节数
	}

	return count;
}
