#include "SerialPortTester.h"

#include <atlstr.h>
#include <Windows.h>


std::vector<std::string> testAllPorts()
{
	TCHAR lpTarget[5000];
	DWORD test;
	std::vector<std::string> ports;
	for (int i = 0; i < 255; ++i)
	{
		CString str;
		str.Format(_T("%d"), i);
		CString comName = CString("COM") + CString(str);

		test = QueryDosDevice(comName, (LPWSTR)lpTarget, 5000);
		if (test != 0)
		{
			ports.push_back(CW2A(comName.GetString()));
		}
	}

	return ports;
}