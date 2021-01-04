#pragma once
#include <iostream>

const char RandX = 'x';
//共享内存操作类
class ShareMemory
{
public:
	//重载构造方法,创建、使用共享内存
	ShareMemory(int key);
	ShareMemory(int key, int size);
	ShareMemory(const char* name);
	ShareMemory(const char* name, int size);

	virtual ~ShareMemory();

	//关联共享内存，获得共享内存首地址
	void* mapShm();
	//断开共享内存关联
	int unmapShm();
	//删除共享内存
	int delShm();

private:
	//获得共享内存ID
	int getShmID(key_t key, int shmSize, int flag);
private:
	//共享内存ID
	int m_shmID;
	//共享内存首地址
	void* m_shmAddr = NULL;
};
