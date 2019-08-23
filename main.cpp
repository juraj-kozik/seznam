#include "CommunicationClass.h"
#include "QtBcApp.h"
#include "SerialPortTester.h"

#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	auto ports = testAllPorts();
	QtBcApp w;
	w.fillComPorts(ports);
	w.setUpApp();
	w.show();
	return a.exec();
}
