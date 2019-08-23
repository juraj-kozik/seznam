#include "QtBcApp.h"

#include <regex>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <QtWidgets/QWidget>
#include <QtWidgets/QMainWindow>

QtBcApp::QtBcApp(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	connectionOpened = false;
	setRegex = false;
	connect(ui.pushButtonOpenConnection, SIGNAL(clicked()), this, SLOT(onClickOpenPort()));
}

void QtBcApp::fillComPorts(const std::vector<std::string> & ports)
{
	for (const auto & port : ports)
	{
		ui.comboBoxCom->addItem(QString::fromStdString(port));
	}
}

void QtBcApp::setUpApp()
{
	connect(ui.pushButtonCloseConnection, SIGNAL(clicked()), this, SLOT(onClickClosePort()));
	connect(ui.pushButtonOpenFile, SIGNAL(clicked()), this, SLOT(onClickOpenFile()));
	connect(ui.pushButtonSendText, SIGNAL(clicked()), this, SLOT(onClickSendText()));
	connect(ui.pushButtonSendFile, SIGNAL(clicked()), this, SLOT(onClickSendFile()));
	connect(ui.pushButtonSendInst1, SIGNAL(clicked()), this, SLOT(onClickSendInst1()));
	connect(ui.pushButtonSendInst2, SIGNAL(clicked()), this, SLOT(onClickSendInst2()));
	connect(ui.pushButtonSendInst3, SIGNAL(clicked()), this, SLOT(onClickSendInst3()));
	connect(ui.pushButtonSendInst4, SIGNAL(clicked()), this, SLOT(onClickSendInst4()));
	connect(ui.pushButtonSendInst5, SIGNAL(clicked()), this, SLOT(onClickSendInst5()));
	connect(ui.pushButtonSetRegex, SIGNAL(clicked()), this, SLOT(onClickSetRegex()));
	
}

void QtBcApp::onClickOpenPort()
{
	if (connectionOpened)
		return;
	QString comPort;
	int baudRate, dataSize, parity, stopBits;
	baudRate = std::stoi((ui.comboBoxBaud->currentText()).toStdString());
	dataSize = std::stoi((ui.comboBoxData->currentText()).toStdString());
	parity = std::stoi((ui.comboBoxParity->currentText()).toStdString());
	stopBits = std::stoi((ui.comboBoxStop->currentText()).toStdString());
	comPort = ui.comboBoxCom->currentText();
	bool secure = ui.checkBoxSecure->isChecked();
	bool logFile = ui.checkBoxLogFile->isChecked();

	serialPtr->serialSettings(comPort.toStdString(), baudRate, parity, dataSize, stopBits);

	if (!connectionOpened)
	{
		//myManager = new CommunicationClass;
		connectionOpened = true;
		ui.textBrowserCommunicator->append("Connecting...");
		if (!serialPtr->openConnection())
		{
			QMessageBox msgBox;
			msgBox.setWindowTitle("Error");
			msgBox.setText("Open connection problem! Try again later!");
			msgBox.exec();
			return;
		}
	}

	comPtr->setup(serialPtr->Port(), secure, logFile, this);
	connect(this, SIGNAL(sendReadyToStop()), comPtr.get(), SLOT(signalToStop()));
	std::thread appThread(&CommunicationClass::startCommunication, comPtr);
	commThread.push_back(std::move(appThread));
}

void QtBcApp::showConnected()
{
	ui.textBrowserCommunicator->append("Connected.");
}

void QtBcApp::onClickOpenFile()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "C://", "All files (*.*)");
	ui.lineEditFileName->setText(fileName);
}

void QtBcApp::onClickClosePort()
{
	if (!connectionOpened)
		return;
	if (connectionOpened)
	{
		emit sendReadyToStop();
		if (commThread[0].joinable())
			commThread[0].join();
		serialPtr->closeConnection();
		connectionOpened = false;
		setRegex = false;
		ui.textBrowserCommunicator->append("Disconnected.");
		comPtr.reset();
	}
	ui.lineEditFileName->clear();
	ui.plainTextEdit->clear();
	ui.lineEditInst1->clear();
	ui.lineEditInst2->clear();
	ui.lineEditInst3->clear();
	ui.lineEditInst4->clear();
	ui.lineEditInst5->clear();
}

void QtBcApp::onClickSendFile()
{
	if (!connectionOpened)
		return;
	QString fileName;
	fileName = ui.lineEditFileName->text();
	std::string stdFileName;
	stdFileName = fileName.toStdString();
	comPtr->sendFile(stdFileName.c_str());
	ui.lineEditFileName->clear();
}

void QtBcApp::onClickSetRegex()
{
	if (!connectionOpened)
		return;
	QString qRegex = ui.lineEditRegex->text();
	std::string stdRegex = qRegex.toStdString();
	comPtr->setRegex(stdRegex);
	ui.lineEditRegex->clear();
	setRegex = true;
	ui.textBrowserCommunicator->append("Regex is set.");
	ui.checkBoxSendRegex->setEnabled(true);
}

void QtBcApp::onClickSendText()
{
	if (!connectionOpened)
		return;
	if (ui.checkBoxSendRegex->isChecked())
	{
		QString regex;
		regex = ui.plainTextEdit->toPlainText();
		std::string stdRegex;
		stdRegex = regex.toStdString();
		comPtr->sendRegex(stdRegex);
		ui.plainTextEdit->clear();
	}
	else
	{
		QString text;
		text = ui.plainTextEdit->toPlainText();
		std::string stdText;
		stdText = text.toStdString();
		comPtr->sendText(stdText);
		ui.plainTextEdit->clear();
	}
}

void QtBcApp::onClickSendInst1()
{
	if (!connectionOpened)
		return;
	QString inst;
	inst = ui.lineEditInst1->text();
	std::string stdInst;
	stdInst = inst.toStdString();
	comPtr->sendInstruction(stdInst);
}

void QtBcApp::onClickSendInst2()
{
	if (!connectionOpened)
		return;
	QString inst;
	inst = ui.lineEditInst2->text();
	std::string stdInst;
	stdInst = inst.toStdString();
	comPtr->sendInstruction(stdInst);
}

void QtBcApp::onClickSendInst3()
{
	if (!connectionOpened)
		return;
	QString inst;
	inst = ui.lineEditInst3->text();
	std::string stdInst;
	stdInst = inst.toStdString();
	comPtr->sendInstruction(stdInst);
}

void QtBcApp::onClickSendInst4()
{
	if (!connectionOpened)
		return;
	QString inst;
	inst = ui.lineEditInst4->text();
	std::string stdInst;
	stdInst = inst.toStdString();
	comPtr->sendInstruction(stdInst);
}

void QtBcApp::onClickSendInst5()
{
	if (!connectionOpened)
		return;
	QString inst;
	inst = ui.lineEditInst5->text();
	std::string stdInst;
	stdInst = inst.toStdString();
	comPtr->sendInstruction(stdInst);
}

void QtBcApp::showTextGui(const QString & text)
{
	ui.textBrowserCommunicator->append(text);
}

void QtBcApp::showInstruction(const QString & data)
{
	ui.textBrowserCommunicator->append(data);
}

void QtBcApp::showReceivedFileName(const QString & fileName)
{
	ui.textBrowserCommunicator->append(fileName);
}

void QtBcApp::showErrorMsg(const QString & msg)
{
	QMessageBox msgBox;
	msgBox.setWindowTitle("Error");
	msgBox.setText(msg);
	msgBox.exec();
}
