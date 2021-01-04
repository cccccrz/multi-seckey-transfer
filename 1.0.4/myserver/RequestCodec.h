#pragma once
#include "Codec.h"

//客户端请求结构体
//考虑到内存对齐，一个合适的存放顺序
struct RequestMsg
{
	char     clientID[12];      //客户端ID
	char    authCode[65];       //消息验证码 ---- HMAC(@serverID+clientID@,r1)EVP_sha256() 
	char    serverID[12];       //服务端ID     
	char     r1[64];            //客户端随机数
	int     cmdType;            //请求类型  1.秘钥协商；2.秘钥查看；3.秘钥注销
};

//客户端请求信息编解码类
class RequestCodec :
	public Codec
{
public:
	enum CMDTYPE { NEWORUPDATE = 1, CHECK, REVOKE, VIEW };
	//请求结构体解码构造
	RequestCodec();
	//请求结构体编码构造 -- 传入请求结构体
	RequestCodec(RequestMsg* reqMsg);

	~RequestCodec();

	//重载Codec编解码方法
	//编码请求信息：传出字节流；成功返回 0
	int EncodeMsg(char** outData, int& outLen);
	//解码请求信息 传入字节流，返回请求结构体
	void* DecodeMsg(char* inData, int inLen);

	//编码构造保存为类属性，编码使用
	RequestMsg m_reqMsg;
};
