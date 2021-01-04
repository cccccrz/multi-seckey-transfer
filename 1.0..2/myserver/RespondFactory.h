#pragma once
#include "CodecFactory.h"
#include "RespondCodec.h"


//应答信息编解码类生产工厂
class RespondFactory :
	public CodecFactory
{
public:
	//解码生产工厂
	RespondFactory();
	//编码生产工厂
	RespondFactory(RespondMsg* rspMsg);

	~RespondFactory();

	//生产 编解码类 
	Codec* createCodec();

private:
	//编解码标志：true 编码；false 解码
	bool m_flag;
	//存放应答信息指针，准备构造：应答信息编码类
	RespondMsg* m_rspMsg;
	//编解码类智能指针，自动释放内存
	shared_ptr<Codec> m_ptr;

};
