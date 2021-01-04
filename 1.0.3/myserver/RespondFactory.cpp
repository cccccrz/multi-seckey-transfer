#include "RespondFactory.h"

//解码生产工厂
RespondFactory::RespondFactory()
{
	m_flag = false;
}

//编码生产工厂
RespondFactory::RespondFactory(RespondMsg* rspMsg)
{
	m_flag = true;
	m_rspMsg = rspMsg;
}

RespondFactory::~RespondFactory()
{
	cout << "RespondFactory destruct..." << endl;
}

//生产 编解码类
Codec* RespondFactory::createCodec()
{
#if 0
	//无法释放内存？？ 使用智能指针
	if (m_flag)//生产编码
	{
		return new RespondCodec(m_rspMsg);
	}
	else//生产解码
	{
		return new RespondCodec();
	}
#else
	if (m_flag)//生产编码
	{
		m_ptr = make_shared<RespondCodec>(m_rspMsg);
	}
	else//生产解码 
	{
		m_ptr = make_shared<RespondCodec>();
	}

	//从智能指针中取值
	return m_ptr.get();
#endif
}
