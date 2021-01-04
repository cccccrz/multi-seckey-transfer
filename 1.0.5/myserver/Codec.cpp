#include "Codec.h"

Codec::Codec()
{

}
Codec::~Codec()
{

}

//传出参数 传出编码序列
int Codec::EncodeMsg(char** outData, int& outLen)
{
	return 0;
}

//传入参数 返回解码对象
void* Codec::DecodeMsg(char* inData, int inLen)
{
	return NULL;
}
