#pragma once

#include <string>

const int BYTES_MEGABYTES = 1024 * 1024;

class WinInfoGetter
{
public:
	static std::wstring getCreationDate(const std::wstring & path);
	static double fileSize(const std::wstring & path);
};