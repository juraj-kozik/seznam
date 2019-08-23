#include "Serial.h"

#include <atlstr.h>
#include <iostream>
#include <set>
#include <Windows.h>

Serial::Serial()
{

}

bool Serial::openConnection()
{
	CString comCString(com.c_str());
	DCB dcb;
	SecureZeroMemory(&dcb, sizeof(DCB));

	HANDLE hPort = CreateFile(
		comCString,								//lpFileName
		GENERIC_WRITE | GENERIC_READ,			//dwDesiredAccess
		0,										//dwShareMode               --must be zero...
		NULL,									//lpSecurityAttributes
		OPEN_EXISTING,							//dwCreationDisposition
		0,										//dwFlagsAndAttribudes
		NULL									//hTemplateFile
	);
	port = hPort;
	//CBR_9600 <- WinBase.h
	dcb.DCBlength = sizeof(DCB);
	dcb.BaudRate = baudRate;
	dcb.ByteSize = dataSize;
	dcb.Parity = parity;
	dcb.StopBits = stopBits;

	dcb.fBinary = TRUE; //pridano
	dcb.fParity = TRUE; //pridano

	if (!GetCommState(port, &dcb))
		return false;
	if (!SetCommState(port, &dcb))
		return false;

	COMMTIMEOUTS timeouts;
	timeouts.ReadIntervalTimeout = 100;
	timeouts.ReadTotalTimeoutMultiplier = 100;
	timeouts.ReadTotalTimeoutConstant = 100;
	timeouts.WriteTotalTimeoutMultiplier = 100;
	timeouts.WriteTotalTimeoutConstant = 100;
	if (!SetCommTimeouts(hPort, &timeouts))
		return false;

	return true;
}

void Serial::closeConnection()
{
	CloseHandle(port);
}

void * Serial::Port()
{
	return port;
}

void Serial::serialSettings(std::string comPort, int baudRate, int parity, int dataSize, int stopBits)
{
	com = comPort;
	this->baudRate = baudRate;
	this->parity = parity;
	this->stopBits = stopBits;
	this->dataSize = dataSize;
}