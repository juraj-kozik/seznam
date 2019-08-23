#pragma once

#include <string>

class ParserClass
{
public:
	int getLength();
	char makeByte(char c, bool upper);
	char * parseData(const std::string & dataToParse);
private:
	int dataLength;
};

