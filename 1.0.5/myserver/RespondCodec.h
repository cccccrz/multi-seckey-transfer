#pragma once
#include "Codec.h"
//服务器信息应答结构体
struct RespondMsg
{
	char    serverID[12];   //服务端ID
	char    clientID[12];   //客户端ID
	char    r2[64];         //服务端随机字符串 
	int     seckeyID;       //秘钥ID          
	int     rv;             //服务器接收信息结果：0 成功；非零失败
};

//服务端应答信息编解码类
class RespondCodec :
	public Codec
{
public:
	//解码构造函数
	RespondCodec();
	//编码构造函数
	RespondCodec(RespondMsg* rspMsg);

	~RespondCodec();

	//重载Codec编解码方法
	//应答结构体编码：传出字节流；成功返回 0
	int EncodeMsg(char** outData, int& outLen);
	//应答结构体解码：传入字节流，返回应答结构体
	void* DecodeMsg(char* intData, int inLen);

	//编码构造保存为类属性，编码使用
	RespondMsg m_rspMsg;
};
