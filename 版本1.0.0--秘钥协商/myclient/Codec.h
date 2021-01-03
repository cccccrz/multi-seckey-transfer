#pragma once
#include "SequenceASN1.h"
class Codec :
	public SequenceASN1
{
public:
	Codec();
	virtual ~Codec();

	//序列化为字节流  -- ASN1
	virtual int EncodeMsg(char** outData, int& outLen);
	//字节流反序列化  -- ASN1
	virtual void* DecodeMsg(char* inData, int inLen);
};
