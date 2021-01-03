#pragma once
#include "ItcastLog.h"

//超时时间设定 默认1000
static const int TIMEOUT = 1000;
//TCP通信类
class TcpSocket
{
public:
	//设定错误种类 -- :3001 参数错误; 3002 超时错误; 3003 对等关闭错误; 3004 Malloc错误
	enum ErrorType { ParamError = 3001, TimeoutError, PeerCloseError, MallocError };
	//默认构造，可使用成员方法--收发数据
	TcpSocket();
	//使用通信套接字构造，用于建立连接，进行通信
	TcpSocket(int connfd);
	~TcpSocket();

	//连接服务器
	int connectToHost(char* ip, unsigned short port, int timeout = TIMEOUT);
	//发送数据 - 字节流
	int sendMsg(char* sendData, int sendLen, int timeout = TIMEOUT);
	//接收数据
	int recvMsg(char** recvData, int& recvLen, int timeout = TIMEOUT);
	//断开连接
	void disConnect();
	//释放内存
	void freeMemory(char** buf);

private:
	//设置I/O为非阻塞模式
	int NoneBlockIO(int fd);
	//设置I/O为阻塞模式
	int BlockIO(int fd);
	//读超时检测
	int readTimeout(unsigned int wait_seconds);
	//写超时检测
	int writeTimeout(unsigned int wait_seconds);
	//连接超时检测；包含connect() 
	int connectTimeout(struct sockaddr_in* addr, unsigned int wait_seconds);
	//每次从缓冲区读取n个字符
	int readn(void* buf, int count);
	//每次从缓冲区写入n个字符
	int writen(const void* buf, int count);

private:
	//通信文件描述符--客户端套接字
	int m_socket;
	//日志对象
	ItcastLog m_log;

};
