#include "CommunicationClass.h"

#include <atlstr.h>
#include <conio.h>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <iomanip>
#include <mutex>
#include <QObject>
#include <vector>
#include <Windows.h>
#include <WinSock2.h>

CommunicationClass::CommunicationClass(QObject *parent) : QObject(parent)
{
	
}

CommunicationClass::~CommunicationClass()
{
	ptrGUI = NULL;
	Port = NULL;
}

void CommunicationClass::setup(void * sPort, bool usingCipher, bool usingLogFile, QtBcApp * ptrGUI)
{
	std::mutex setup;
	setup.lock();
	Port = sPort;
	cipher = usingCipher;
	logF = usingLogFile;
	readyToStop = false;
	

	if (logF)
	{
		std::string logFileName = "logfile";
		std::string logFileExt = ".txt";
		FileManager manager;
		myLogFile = manager.logFile(logFileName, logFileExt);
		if (myLogFile == "no")
		{
			logF = false;
			emit sendErrorMsg("Cannot create log file!");
		}
	}
	ready = true;
	setup.unlock();
	connect(this, SIGNAL(sendTextToGui(QString)) , ptrGUI, SLOT(showTextGui(QString)));
	connect(this, SIGNAL(connectionOpened()), ptrGUI, SLOT(showConnected()));
	connect(this, SIGNAL(sendFileName(QString)), ptrGUI, SLOT(showReceivedFileName(QString)));
	connect(this, SIGNAL(sendInstructionToGui(QString)), ptrGUI, SLOT(showInstruction(QString)));
	connect(this, SIGNAL(sendErrorMsg(QString)), ptrGUI, SLOT(showErrorMsg(QString)));
}

void CommunicationClass::setRegex(std::string regexToSetup)
{
	std::regex tmpRegex(regexToSetup);
	myRegex = tmpRegex;
}

bool CommunicationClass::sendRegex(std::string data)
{
	if (std::regex_match(data, myRegex))
	{
		DWORD bytesWrite;
		char typeBuffer[] = { 'r' };
		if (!WriteFile(Port, typeBuffer, 1, &bytesWrite, NULL))
		{
			emit sendErrorMsg("Regex send problem!");
		}
		if (!WriteFile(Port, data.c_str(), data.size(), &bytesWrite, NULL))
		{
			emit sendErrorMsg("Regex send problem!");
		}
	}
	else
		emit sendErrorMsg("Invalid regex format!");
	return true;
}

bool CommunicationClass::sendText(std::string & input)
{
	const char * charInput = input.c_str();
	char type[1] = { 'p' };
	DWORD bytesWritten;
	bool result = false;
	if (!WriteFile(Port, type, 1, &bytesWritten, NULL))
	{
		emit sendErrorMsg("Error send text data!");
		return false;
	}
	if (!WriteFile(Port, charInput, input.length(), &bytesWritten, NULL))
	{
		emit sendErrorMsg("Error send text data!");
		return false;
	}

	return true;
}

bool CommunicationClass::receiveText()
{
	DWORD bytesRead;
	char * buffer = new char[1024];
	WaitForSingleObject(Port, INFINITE);
	ReadFile(Port, buffer, 1024, &bytesRead, NULL);
	std::string output(buffer);
	output.resize(bytesRead);

	emit sendTextToGui(QString::fromStdString(output));

	if (logF)
	{
		std::ofstream logFile;
		logFile.open(myLogFile.c_str(), std::ios::app);
		logFile << "Received text: " << output << "\n";
		logFile.close();
	}
	delete[] buffer;
	return true;
}

bool CommunicationClass::sendInstruction(std::string & data)
{
	ParserClass parser;
	char * ptrData;
	char type[] = { 'i' };
	ptrData = parser.parseData(data);
	int dataLength = parser.getLength();

	DWORD bytesWritten;
	char * encryptedInst;
	char * finalData = new char[1024];

	if (cipher)
	{
		encryptedInst = myEncrypter.encryptInstruction(ptrData, dataLength);
		int encDataLength = myEncrypter.getInstDataLen();
		for (int i = 0; i < encDataLength; ++i)
		{
			finalData[i] = encryptedInst[i];
			if (encryptedInst[i] & (1 << (7)))
			{
				finalData[i + encDataLength] = 0x08;
			}
			else
				finalData[i + encDataLength] = 0x00;
		}
		delete[] ptrData;
		ptrData = encryptedInst;
		dataLength = encDataLength;
	}
	else
	{
		for (int i = 0; i < dataLength; ++i)
		{
			finalData[i] = ptrData[i];
			if (ptrData[i] & (1 << (7)))
			{
				finalData[i + dataLength] = 0x08;
			}
			else
				finalData[i + dataLength] = 0x00;
		}
	}

	if (!WriteFile(Port, type, 1, &bytesWritten, NULL))
	{
		emit sendErrorMsg("Send type instruction error!");
		return false;
	}
	if (!WriteFile(Port, finalData, dataLength * 2, &bytesWritten, NULL))
	{
		emit sendErrorMsg("Send type instruction error!");
		return false;
	}

	delete[] ptrData;
	return true;
}

bool CommunicationClass::receiveInstructions()
{
	DWORD bytesRead;
	std::mutex m_inst;
	char * buffer = new char[1024];
	bool flag = false;
	while (true)
	{
		if (!ReadFile(Port, buffer, 1024, &bytesRead, NULL))
		{
			emit sendErrorMsg("Receive instruction error!");
			return false;
		}
		if (bytesRead > 0)
		{
			for (unsigned int i = bytesRead / 2; i < bytesRead; ++i)
			{
				if ((buffer[i] >> (3)) & 1)
					buffer[i - (bytesRead / 2)] = buffer[i - (bytesRead / 2)] | 0x80;
			}
			std::string output(buffer);
			output.resize(bytesRead / 2);
			if (oppositeCipher)
			{
				BYTE * decInstr;
				char * decBuffer = new char[1024];
				decInstr = myEncrypter.decryptInstruction(buffer, bytesRead);
				int decInstLen = myEncrypter.getInstDecLen();
				if (logF)
				{
					std::ofstream logFile;
					logFile.open(myLogFile.c_str(), std::ios::app);
					logFile << "Received instruction: ";
					for (int i = 0; i < decInstLen; ++i)
					{
						decBuffer[i] = decInstr[i];
						decBuffer[i] = decInstr[i];
						if ((int)decInstr[i] > 31 && (int)decInstr[i] < 127)
							logFile << decInstr[i];
						else
							logFile << std::setfill('0') << std::setw(2) << std::hex << (int)decInstr[i];
					}
					logFile << "\n";
					logFile.close();
				}
				std::stringstream ss;
				for (int i = 0; i < decInstLen; ++i)
				{
					if ((int)decInstr[i] > 31 && (int)decInstr[i] < 127)
						ss << decInstr[i];
					else
						ss << std::setfill('0') << std::setw(2) << std::hex << (int)decInstr[i];
				}
				std::string dataString = ss.str();
				emit sendInstructionToGui(QString::fromStdString(dataString));
				delete[] decInstr;
				delete[] buffer;
			}
			else
			{
				if (logF)
				{
					std::ofstream logFile;
					
					logFile.open(myLogFile.c_str(), std::ios::app);
					logFile << "Received instruction: ";
					for (unsigned int i = 0; i < bytesRead; ++i)
					{
						if ((int)buffer[i] > 31 && (int)buffer[i] < 127)
							logFile << buffer[i];
						else
							logFile << std::setfill('0') << std::setw(2) << std::hex << (int)buffer[i];
					}
					logFile << "\n";
					logFile.close();
				}
				std::stringstream ss;
				for (unsigned int i = 0; i < bytesRead/2; ++i)
				{
					if ((int)buffer[i] > 31 && (int)buffer[i] < 127)
						ss << buffer[i];
					else 
					{
						char hexBuffer[2];
						int k = (int)((BYTE)buffer[i]);
						itoa(k, hexBuffer, 16);
						ss << hexBuffer;
						if (i + 1 < bytesRead)
							ss << " ";
					}
				}
				std::string dataString = ss.str();
				QString dataQstring = QString::fromStdString(dataString);
				emit sendInstructionToGui(dataQstring);
			}
			flag = true;
			break;
		}
		if (bytesRead == 0 && flag)
			break;
	}
	return true;
}

bool CommunicationClass::sendFile(const char * file)
{
	std::ifstream inputFile;
	DWORD bytesWritten;
	inputFile.open(file, std::ios::in | std::ifstream::binary | std::ifstream::in | std::ifstream::ate);
	if (!inputFile.is_open())
	{
		emit sendErrorMsg("Cannot open selected file!");
		return false;
	}
	inputFile.seekg(0, inputFile.end);
	int fileLength = inputFile.tellg();
	inputFile.seekg(0, std::ios::beg);

	int blockNum = fileLength / FILE_MAXSIZE;
	int extra = fileLength % FILE_MAXSIZE;
	char buffer[(FILE_MAXSIZE * 2) + 100];
	char type[] = { 'f' };
	std::string stdFile(file);
	char fileExt[4];
	int cnt = 0;

	for (unsigned int i = stdFile.size() - 1; i > 0; --i)
	{

		if (stdFile[i] == '.' && 3 == cnt)
		{
			fileExt[3] = '5';
			break;
		}
		else if (stdFile[i] == '.')
			break;
		if (cnt > 3)
		{
			emit sendErrorMsg("Insert file name with extension, i.e. .txt.");
			return false;
		}
		fileExt[cnt] = stdFile[i];;
		++cnt;
	}
	if (!WriteFile(Port, type, 1, &bytesWritten, NULL))
	{
		emit sendErrorMsg("Send type file error!");
		return false;
	}
	if (!WriteFile(Port, fileExt, 4, &bytesWritten, NULL))
	{
		emit sendErrorMsg("Send type file error!");
		return false;
	}

	for (int i = 0; i < blockNum; ++i)
	{
		inputFile.read(buffer, FILE_MAXSIZE);
		if (cipher)
		{
			char * encryptedData;
			encryptedData = myEncrypter.encryptFile(buffer, FILE_MAXSIZE, false);
			for (unsigned int k = 0; k < FILE_MAXSIZE; ++k)
			{
				if (encryptedData[k] & 0x80)
					encryptedData[k + FILE_MAXSIZE] = 0x08;
				else
					encryptedData[k + FILE_MAXSIZE] = 0x00;
			}
			WriteFile(Port, encryptedData, FILE_MAXSIZE * 2, &bytesWritten, NULL);
			delete[] encryptedData;
		}
		else
		{
			for (unsigned int k = 0; k < FILE_MAXSIZE; ++k)
			{
				if (buffer[k] & 0x80)
					buffer[k + FILE_MAXSIZE] = 0x08;
				else
					buffer[k + FILE_MAXSIZE] = 0x00;
			}
			WriteFile(Port, buffer, FILE_MAXSIZE * 2, &bytesWritten, NULL);

		}
	}

	char * buffer2 = new char[(extra * 2) + 4];
	inputFile.read(buffer2, extra);

	if (cipher)
	{
		char * encryptedData;
		encryptedData = myEncrypter.encryptFile(buffer2, extra, true);
		int fileDataLen = myEncrypter.getFileDataLen();

		for (int k = 0; k < fileDataLen; ++k)
		{
			if (encryptedData[k] & 0x80)
				encryptedData[k + fileDataLen] = 0x08;
			else
				encryptedData[k + fileDataLen] = 0x00;
		}
		WriteFile(Port, encryptedData, fileDataLen * 2, &bytesWritten, NULL);
		delete[] encryptedData;
		delete[] buffer2;
	}
	else
	{
		for (int i = 0; i < extra; ++i)
		{
			if (buffer2[i] & 0x80)
				buffer2[i + extra] = 0x08;
			else
				buffer2[i + extra] = 0x00;
		}
		WriteFile(Port, buffer2, extra * 2, &bytesWritten, NULL);
		delete[] buffer2;
	}
	inputFile.close();
	return true;
}

bool CommunicationClass::receiveFile()
{
	bool result = false;
	DWORD bytesRead;
	std::mutex m_file;

	char fileExt[4];
	while (1)
	{
		if (!ReadFile(Port, fileExt, 4, &bytesRead, NULL))
		{

			emit sendErrorMsg("File recieve error!");
			return false;
		}
		if (bytesRead > 0)
			break;
	}
	std::string fileExtension;
	for (int i = 3; i >= 0; --i)
	{
		if (fileExt[i] != '5')
			fileExtension.push_back(fileExt[i]);
	}

	char dataBuffer[(FILE_MAXSIZE * 2) + 4];
	bool flag = false;
	FileManager manager;
	std::string savedFile = manager.receivedFile(fileExtension);
	std::ofstream output;
	output.open(savedFile.c_str(), std::ios::out | std::ios::binary);
	if (!output.is_open())
	{
		emit sendErrorMsg("Cannot create file!");
		//flush serial port
		while (1)
		{
			ReadFile(Port, dataBuffer, (FILE_MAXSIZE * 2) + 4, &bytesRead, NULL);
			if (bytesRead > 0)
			{
				flag = true;
			}
			if (flag && bytesRead < ((FILE_MAXSIZE * 2) + 4))
				break;
		}
		return false;
	}

	while (true)
	{
		ReadFile(Port, dataBuffer, (FILE_MAXSIZE * 2) + 4, &bytesRead, NULL);
		if (bytesRead > 0)
		{
			flag = true;
			if (oppositeCipher)
			{
				BYTE * decryptedData;
				bool finalBlock = false;
				if (bytesRead < (FILE_MAXSIZE * 2 + 4))
					finalBlock = true;
				for (unsigned int i = 0; i < bytesRead / 2; ++i)
				{
					if (dataBuffer[i + (bytesRead / 2)] == 0x08)
						dataBuffer[i] = dataBuffer[i] | 0x80;
				}

				decryptedData = myEncrypter.decryptFile(dataBuffer, bytesRead / 2, finalBlock);
				int decDataLen = myEncrypter.getFileDecDataLen();
				for (int i = 0; i < decDataLen; ++i)
				{
					output << decryptedData[i];
				}
				delete[] decryptedData;
			}
			else
			{
				BYTE * newData = new BYTE[(bytesRead / 2)];
				for (unsigned int i = 0; i < bytesRead / 2; ++i)
				{
					if (dataBuffer[i + (bytesRead / 2)] == 0x08)
						newData[i] = dataBuffer[i] | 0x80;
					else
						newData[i] = dataBuffer[i];
				}
				for (unsigned int i = 0; i < (bytesRead / 2); ++i)
				{
					output << newData[i];
				}
				delete[] newData;
			}
		}
		if (bytesRead < (FILE_MAXSIZE * 2) && flag)
			break;
	}
	output.close();
	if (logF)
	{
		std::ofstream logFile;
		logFile.open(myLogFile.c_str(), std::ios::app);
		logFile << "Received file: " << savedFile << "\n";
		logFile.close();
	}
	emit sendFileName(QString::fromStdString(savedFile));
	return true;
}

void CommunicationClass::PortEvent()
{
	std::mutex m_port;
	while (1)
	{
		m_port.lock();
		if (ready)
		{
			m_port.unlock();
			break;
		}
		m_port.unlock();
	}

	DWORD bytesRead, bytesReadEx, bytesWriteEx, bytesReadC, bytesWriteC;
	int exchange = 0;
	char myExBuffer[3], oppCipherBuffer[2];
	char exBuffer1[] = { 'e', 'x', '1' };
	char exBuffer2[] = { 'e', 'x', '2' };
	char cipherBufferY1[2];
	char cipherBufferY2[2];
	char myType[1];

	cipherBufferY1[1] = '1';
	cipherBufferY2[1] = '2';
	if (cipher)
		cipherBufferY1[0] = cipherBufferY2[0] = 'y';
	else
		cipherBufferY1[0] = cipherBufferY2[0] = 'n';
	bool toFlush = false;
	while (!readyToStop)
	{
		ReadFile(Port, oppCipherBuffer, 2, &bytesReadC, NULL);
		if (bytesReadC > 0)
		{
			if (oppCipherBuffer[0] == 'y' && oppCipherBuffer[1] == '1')
			{
				if (!WriteFile(Port, cipherBufferY2, 2, &bytesWriteC, NULL))
				{
					emit sendErrorMsg("Error sending data!");
					toFlush = true;
				}
				oppositeCipher = true;
			}
			else if (oppCipherBuffer[0] == 'n' && oppCipherBuffer[1] == '1')
			{
				if(! WriteFile(Port, cipherBufferY2, 2, &bytesWriteC, NULL))
				{
					emit sendErrorMsg("Error sending data!");
					toFlush = true;
				}
				oppositeCipher = false;
			}
			else if (oppCipherBuffer[0] == 'y' && oppCipherBuffer[1] == '2')
			{
				if (!WriteFile(Port, cipherBufferY2, 2, &bytesWriteC, NULL))
				{
					emit sendErrorMsg("Error sending data!");
					toFlush = true;
				}
				oppositeCipher = true;
			}
			else if (oppCipherBuffer[0] == 'n' && oppCipherBuffer[1] == '2')
			{
				if (!WriteFile(Port, cipherBufferY2, 2, &bytesWriteC, NULL))
				{
					emit sendErrorMsg("Error sending data!");
					toFlush = true;
				}
				oppositeCipher = false;
			}
			break;
		}
		if(! WriteFile(Port, cipherBufferY1, 2, &bytesWriteC, NULL) && !readyToStop)
		{
			emit sendErrorMsg("Error sending data!");
			toFlush = true;
		}
	}

	char flushBuffer[10];
	if(toFlush)
		ReadFile(Port, flushBuffer, 10, &bytesReadC, NULL);//<= FLUSH SERIAL PORT
	bool flag = false;
	while (!readyToStop)
	{
		ReadFile(Port, myExBuffer, 3, &bytesReadEx, NULL);
		if (bytesReadEx > 0)
		{
			std::string exInput(myExBuffer);
			exInput.resize(3);
			if (myExBuffer[0] == 'e' && myExBuffer[1] == 'x' && myExBuffer[2] == '1')
			{
				if(! WriteFile(Port, exBuffer2, 3, &bytesWriteEx, NULL))
					emit sendErrorMsg("Error receiving data!");
				exchange = 1;
				flag = true;
			}
			else if (myExBuffer[0] == 'e' && myExBuffer[1] == 'x' && myExBuffer[2] == '2')
			{
				exchange = 2;
				flag = true;
			}
			if (flag)
				break;
		}
		if(! WriteFile(Port, exBuffer1, 3, &bytesWriteEx, NULL))
			emit sendErrorMsg("Error receiving data!");
	}

	if (!myEncrypter.setup(Port))
		emit sendErrorMsg("Error setting secure connection!");
	else
	{
		bool result = false;
		if (exchange == 1)
			result = myEncrypter.exchange1(Port);
		else
			result = myEncrypter.exchange2(Port);
		if(!result)
			emit sendErrorMsg("Error setting secure connection!");
	}
	ReadFile(Port, flushBuffer, 10, &bytesReadC, NULL); //<= FLUSH SERIAL PORT
	emit connectionOpened();
	while (!readyToStop)
	{
		ReadFile(Port, myType, 1, &bytesRead, NULL);
		if (bytesRead > 0)
		{
			if (myType[0] == 'p')
			{
				receiveText();
				
			}	
			else if (myType[0] == 'i')
			{
				receiveInstructions();
			}
			else if (myType[0] == 'c')
				receiveInstructions();
			else if (myType[0] == 'f')
				receiveFile();
			else if (myType[0] == 'r')
			{
				char regex[255];
				DWORD bytesReadReg;
				ReadFile(Port, regex, 255, &bytesReadReg, NULL);
				std::string regexOutput(regex);
				regexOutput.resize(bytesReadReg);
				emit sendTextToGui(QString::fromStdString(regexOutput));
				if (logF)
				{
					std::ofstream logFile;
					logFile.open(myLogFile.c_str(), std::ios::app);
					logFile << "Received regex: " << regexOutput << "\n";
					logFile.close();
				}
			}
			else
			{
				emit sendErrorMsg("Unkown message received!");
				char newFlushBuffer[1024];
				ReadFile(Port, newFlushBuffer, 1024, &bytesReadC, NULL); //<= FLUSH SERIAL PORT
			}
		}
	}
}

// new version of this function from console version
void CommunicationClass::startCommunication()
{
	PortEvent();
}

void CommunicationClass::signalToStop()
{
	readyToStop = true;
}