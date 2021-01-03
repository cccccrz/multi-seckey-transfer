#include <cstdio>
#include "ServerOperation.h"
#include <sys/ipc.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <iostream>
#include <signal.h>
using namespace std;

void createDeamon();

int main()
{
	//设置为守护进程
	//createDeamon();

	//注册信号处理函数
	/*struct sigaction act;
	act.sa_flags = 0;
	act.sa_handler = ServerOperation::catchSignal;
	sigemptyset(&act.sa_mask);
	sigaction(SIGUSR1, &act, NULL);*/

	// 启动服务器
	ServerInfo info;
	info.maxNode = 20;
	info.serverPort = 9898;
	info.shmKey = ftok("/home/sa", 'w');
	strcpy(info.serverID, "0001");
	ServerOperation op(&info);
	op.startwork();

	cout << "good bye..." << endl;

	return 0;
}

void createDeamon()
{
	pid_t pid;
	pid = fork();
	if (pid != 0)
	{
		exit(0);
	}
	else if (pid == 0)//子进程
	{
		setsid();

		chdir("/home/sa");

		//修改当前进程文件掩码
		umask(0000);

		//关闭或者重定向标准输入、输出、错误
		//close(STDIN_FILENO);
		//close(STDOUT_FILENO);
		//close(STDERR_FILENO);

		int fd = open("/dev/null", O_RDWR);

		dup2(fd, STDIN_FILENO);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
	}
}
