#include "ParserClass.h"

#include <sstream>
#include <vector>
#include <Windows.h>

char * ParserClass::parseData(const std::string & dataToParse)
{
	dataLength = dataToParse.length();
	std::stringstream os(dataToParse);
	int dataParsed = 0;
	char * data = new char[dataLength];
	char c;
	char d;
	bool mChar, mHex, mUnresolv;
	mChar = mHex = false;
	mUnresolv = true;

	while (os >> c)
	{
		if (c == '@')
		{
			mUnresolv = false;
			if (!(os >> c))
				break;
			if (c == 'c')
			{
				mHex = false;
				mChar = true;
			}
			else if (c == 'h')
			{
				mHex = true;
				mChar = false;
			}
		}
		else
		{
			if (mChar)
			{
				data[dataParsed] = c;
				++dataParsed;
			}
			else if (mHex)
			{
				if (!(os >> d))
					break;
				char result1, result2, result;
				result1 = makeByte(c, true);
				result2 = makeByte(d, false);
				result = result1 | result2;
				data[dataParsed] = result;
				++dataParsed;
			}
		}
	}
	dataLength = dataParsed;

	return data;
}

char ParserClass::makeByte(char c, bool upper)
{
	char result = ' ';
	switch (c)
	{
	case '0':
		result = 0x00;
		break;
	case '1':
		result = 0x01;
		break;
	case '2':
		result = 0x02;
		break;
	case '3':
		result = 0x03;
		break;
	case '4':
		result = 0x04;
		break;
	case '5':
		result = 0x05;
		break;
	case '6':
		result = 0x06;
		break;
	case '7':
		result = 0x07;
		break;
	case '8':
		result = 0x08;
		break;
	case '9':
		result = 0x09;
		break;
	case 'A':
	case 'a':
		result = 0x0A;
		break;
	case 'B':
	case 'b':
		result = 0x0B;
		break;
	case 'C':
	case 'c':
		result = 0x0C;
		break;
	case 'D':
	case 'd':
		result = 0x0D;
		break;
	case 'E':
	case 'e':
		result = 0x0E;
		break;
	case 'F':
	case 'f':
		result = 0x0F;
		break;
	default:
		break;
	}

	if (upper)
		result = result << 4;

	return result;
}

int ParserClass::getLength()
{
	return dataLength;
}
