#pragma once
#include "CodecFactory.h"
#include "RequestCodec.h"

//请求信息编解码类生产工厂
class RequestFactory :
	public CodecFactory
{
public:
	//解码生产工厂
	RequestFactory();
	//编码生产工厂
	RequestFactory(RequestMsg* reqMsg);

	~RequestFactory();

	//生产 编解码类
	Codec* createCodec();

private:
	//判断生产类型标志：true 编码；false 解码
	bool m_flag;
	//存放请求信息指针 准备构造：请求信息编码类
	RequestMsg* m_reqMsg;
	//编解码类智能指针，自动释放内存
	shared_ptr<Codec>m_ptr;
};
