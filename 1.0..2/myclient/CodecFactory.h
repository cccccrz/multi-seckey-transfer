#pragma once
#include"Codec.h"
#include <memory>
#include <iostream>
using namespace std;

//工厂模式：每种产品由一种工厂来创建, 不同工厂创建不同的对象
//工厂虚基类:生产编解码类
class CodecFactory
{
public:
	CodecFactory();
	virtual ~CodecFactory();

	//生产编解码对象方法
	virtual Codec* createCodec();
};
