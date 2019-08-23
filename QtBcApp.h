#pragma once

#include "CommunicationClass.h"
#include "Serial.h"
#include "ui_QtBcApp.h"

#include <memory>
#include <string>
#include <thread>
#include <vector>


class QtBcApp : public QWidget
{
	Q_OBJECT

public:
	QtBcApp(QWidget *parent = Q_NULLPTR);
	void fillComPorts(const std::vector<std::string> & ports);
	void setUpApp();	

private:
	Ui::QtBcAppClass ui;
	bool connectionOpened;
	bool setRegex;
	std::unique_ptr<Serial> serialPtr = std::make_unique<Serial>();
	std::unique_ptr<CommunicationClass> comPtr = std::make_unique<CommunicationClass>();
	std::vector<std::thread> commThread;
public slots:
	void onClickOpenPort();
	void onClickClosePort();
	void onClickOpenFile();
	void onClickSendFile();
	void onClickSetRegex();
	void onClickSendText();
	void onClickSendInst1();
	void onClickSendInst2();
	void onClickSendInst3();
	void onClickSendInst4();
	void onClickSendInst5();
	void showConnected();
	void showTextGui(const QString & text);
	void showInstruction(const QString & data);
	void showReceivedFileName(const QString & fileName);
	void showErrorMsg(const QString & msg);

signals:
	void sendReadyToStop();
};