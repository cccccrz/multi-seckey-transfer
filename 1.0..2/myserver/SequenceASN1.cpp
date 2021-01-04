#include "SequenceASN1.h"

SequenceASN1::SequenceASN1() : BaseASN1()
{
}

int SequenceASN1::writeHeadNode(int iValue)
{
	int ret = DER_ItAsn1_WriteInteger(iValue, &m_header);
	if (ret != 0)
	{
		return ret;
	}
	m_next = m_header;
	return 0;
}

int SequenceASN1::writeHeadNode(char* sValue, int len)
{
	DER_ITCAST_String_To_AnyBuf(&m_temp, (unsigned char*)sValue, len);
	int ret = DER_ItAsn1_WritePrintableString(m_temp, &m_header);
	if (ret != 0)
	{
		return ret;
	}
	m_next = m_header;
	DER_ITCAST_FreeQueue(m_temp);

	return 0;
}

int SequenceASN1::writeNextNode(int iValue)
{
	int ret = DER_ItAsn1_WriteInteger(iValue, &m_next->next);
	if (ret != 0)
	{
		return ret;
	}
	// 指针后移
	m_next = m_next->next;

	return 0;
}

int SequenceASN1::writeNextNode(char* sValue, int len)
{
	int ret = EncodeChar(sValue, len, &m_next->next);
	if (ret != 0)
	{
		return ret;
	}
	m_next = m_next->next;

	return 0;
}

int SequenceASN1::readHeadNode(int& iValue)
{
	int ret = DER_ItAsn1_ReadInteger(m_header, (ITCAST_UINT32*)&iValue);
	if (ret != 0)
	{
		return ret;
	}
	// 指针后移
	m_next = m_header->next;

	return 0;
}

int SequenceASN1::readHeadNode(char* sValue)
{
	int ret = DER_ItAsn1_ReadPrintableString(m_header, &m_temp);
	if (ret != 0)
	{
		return ret;
	}
	memcpy(sValue, m_temp->pData, m_temp->dataLen);
	DER_ITCAST_FreeQueue(m_temp);
	// 指针后移
	m_next = m_header->next;

	return 0;
}

int SequenceASN1::readNextNode(int& iValue)
{
	int ret = DER_ItAsn1_ReadInteger(m_next, (ITCAST_UINT32*)&iValue);
	if (ret != 0)
	{
		return ret;
	}
	// 指针后移
	m_next = m_next->next;
	return 0;
}

int SequenceASN1::readNextNode(char* sValue)
{
	int ret = DER_ItAsn1_ReadPrintableString(m_next, &m_temp);
	if (ret != 0)
	{
		return ret;
	}
	memcpy(sValue, m_temp->pData, m_temp->dataLen);
	DER_ITCAST_FreeQueue(m_temp);
	// 指针后移
	m_next = m_next->next;

	return 0;
}

int SequenceASN1::packSequence(char** outData, int& outLen)
{
	DER_ItAsn1_WriteSequence(m_header, &m_temp);
	// 传出参数赋值
	*outData = (char*)m_temp->pData;
	outLen = m_temp->dataLen;
	// 释放链表
	DER_ITCAST_FreeQueue(m_header);

	return 0;
}

int SequenceASN1::unpackSequence(char* inData, int inLen)
{
	// char* -> itcast_anybuf
	DER_ITCAST_String_To_AnyBuf(&m_temp, (unsigned char*)inData, inLen);
	DER_ItAsn1_ReadSequence(m_temp, &m_header);
	DER_ITCAST_FreeQueue(m_temp);

	return 0;
}

void SequenceASN1::freeSequence(ITCAST_ANYBUF* node)
{
}
