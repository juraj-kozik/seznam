#pragma once

#include <string>

class FileManager
{
public:
	std::string receivedFile(const std::string & extension);
	std::string logFile(const std::string & name, const std::string & extension);
};

