#include "FileManager.h"

#include <ctime>
#include <filesystem>
#include <fstream>
#include <locale>
#include <iostream>
#include <Windows.h>

std::string FileManager::receivedFile(const std::string & extension)
{
	uint32_t cnt = 1;
	time_t current_time;
	struct tm  local_time;

	time(&current_time);
	localtime_s(&local_time, &current_time);
	uint32_t Year = local_time.tm_year + 1900;
	uint32_t Month = local_time.tm_mon + 1;
	uint32_t Day = local_time.tm_mday;
	std::string month, day;
	if (Month < 10)
		month = "0" + std::to_string(Month);
	else
		month = std::to_string(Month);
	if (Day < 10)
		day = "0" + std::to_string(Day);
	else
		day = std::to_string(Day);

	std::string savedFile = "C:\\tmp\\" + std::to_string(Year) + "-" + month + "-" + day + "-" + "ReceivedFile." + extension;
	std::fstream fileTest(savedFile.c_str());

	while (!fileTest.good())
	{
		savedFile.clear();
		savedFile = "C:\\tmp\\" + std::to_string(Year) + "-" + month + "-" + day + "-" + "ReceivedFile(" + std::to_string(cnt) + ")." + extension;
		std::fstream fileTest(savedFile.c_str());
		++cnt;
	}

	return savedFile;
}

std::string FileManager::logFile(const std::string & name, const std::string & extension)
{
	uint32_t cnt = 1;
	wchar_t result[MAX_PATH];
	GetModuleFileName(NULL, result, MAX_PATH);
	std::wstring ws(result);
	std::string pathFileName(ws.begin(), ws.end()), myLogFile;
	while (1)
	{
		if (pathFileName[pathFileName.size() - 1] != '\\')
			pathFileName.pop_back();
		else
			break;
	}
	pathFileName =+ "logfiles\\";
	std::string testFileName = pathFileName + name + extension;
	std::fstream fileTest(testFileName.c_str());

	while (!fileTest.good())
	{
		myLogFile.clear();
		myLogFile = pathFileName + name + "(" + std::to_string(cnt) + ")" + extension;
		std::fstream fileTest(myLogFile.c_str());
		++cnt;
	}

	std::ofstream logFile;
	logFile.open(myLogFile.c_str(), std::ios::out);
	if (!logFile.is_open())
	{
		myLogFile = "no";
	}
	else
		logFile.close();
	return myLogFile;
}