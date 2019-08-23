#pragma once

#include <regex>
#include <string>
#include "EncryptionClass.h"
#include "FileManager.h"
#include "ParserClass.h"
#include "QtBcApp.h"

#define FILE_MAXSIZE 102400 //100KB

class CommunicationClass : public QObject
{
	Q_OBJECT
public:
	CommunicationClass(QObject *parent = Q_NULLPTR);
	~CommunicationClass();
	void setup(void * sPort, bool usingCipher, bool usingLogFile, QtBcApp * ptrGUI);
	bool sendInstruction(std::string & data);
	bool sendText(std::string & input);
	bool sendFile(const char * file);
	void setRegex(std::string regexToSetup);
	bool sendRegex(std::string data);
	bool receiveInstructions();
	bool receiveText();
	bool receiveFile();
	void PortEvent();
	void startCommunication();
private:
	EncryptionClass myEncrypter;
	QtBcApp * ptrGUI;
	std::string myLogFile;
	std::regex myRegex;
	bool cipher;
	bool oppositeCipher;
	bool readyToStop;
	bool ready = false;
	bool logF;
	void * Port;
public slots:
	void signalToStop();
signals:
	void sendTextToGui(const QString & textToSend);
	void connectionOpened();
	void sendInstructionToGui(QString);
	void sendFileName(const QString & fileName);
	void sendErrorMsg(const QString & msg);
};