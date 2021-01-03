#include "RequestFactory.h"

//解码生产工厂
RequestFactory::RequestFactory()
{
	m_flag = false;
}

//编码生产工厂
RequestFactory::RequestFactory(RequestMsg* reqMsg)
{
	m_flag = true;
	m_reqMsg = reqMsg;
}

RequestFactory::~RequestFactory()
{
	cout << "RequestFactory destruct..." << endl;
}

//生产 编解码类 
Codec* RequestFactory::createCodec()
{
	if (m_flag)//生产编码类
	{
		m_ptr = make_shared<RequestCodec>(m_reqMsg);
	}
	else//生产解码类
	{
		m_ptr = make_shared<RequestCodec>();
	}

	//从智能指针中取值
	return m_ptr.get();
}
