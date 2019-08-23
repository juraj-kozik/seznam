#pragma once

#include <string>

class Serial
{
public:
	Serial();
	bool openConnection();
	void closeConnection();
	void serialSettings(std::string comPort, int baudRate, int parity, int dataSize, int stopBits);
	void * Port();
private:
	std::string com;
	int stopBits;
	int parity;
	int baudRate;
	int dataSize;
	void * port;
};