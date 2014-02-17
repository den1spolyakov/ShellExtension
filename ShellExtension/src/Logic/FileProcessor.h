#pragma once

#define NOMINMAX

#include <fstream>
#include <iomanip>
#include <locale>
#include <codecvt>
#include <limits>
#include <boost/crc.hpp>

#include "WinInfoGetter.h"
#include "Threadpool.h"

std::wstring extractName(const std::wstring & path);
std::wstring extractPath(const std::wstring & path);
bool compareLength(const std::wstring & lhs, const std::wstring & rhs);

std::streamsize const buffer_size     = 1024;
const unsigned int checksumWidth      = 10;
const unsigned int additionalSpace    = 15;
const unsigned int minAmountOfThreads = 2;
const unsigned int defaultSleepTime   = 100;

class FileProcessor
{
private:
	std::vector<std::wstring> paths;
	std::vector<std::future<int>> checksums;
	std::wstring logfile;

	void initialWrite(unsigned int nameWidth);
	unsigned int getMaxWidth();
	void threadIO();
	int threadSum(const std::wstring path);
public:
	FileProcessor(std::vector<std::wstring> & p);
	~FileProcessor() {}
	void log();
};