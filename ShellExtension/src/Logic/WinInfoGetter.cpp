#include "WinInfoGetter.h"

#include <windows.h>
#include <sstream>

std::wstring WinInfoGetter::getCreationDate(const std::wstring & path)
{
	std::wstringstream stream;
    WIN32_FILE_ATTRIBUTE_DATA attr;
    SYSTEMTIME time;
     
	GetFileAttributesEx(path.c_str(), GetFileExInfoStandard, &attr);
    FileTimeToSystemTime(&attr.ftCreationTime, &time);
    stream << time.wMonth << '/' << time.wDay << '/' << time.wYear;
    
	return stream.str();
}

double WinInfoGetter::fileSize(const std::wstring & path) 
{
	HANDLE hFile = CreateFile(path.c_str(), GENERIC_READ, 
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 
        FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile==INVALID_HANDLE_VALUE)
	{
        return -1; 
	}

    LARGE_INTEGER size;
    if (!GetFileSizeEx(hFile, &size))
    {
        CloseHandle(hFile);
        return -1;
    }
    CloseHandle(hFile);

    return double(size.QuadPart) / BYTES_MEGABYTES;
}