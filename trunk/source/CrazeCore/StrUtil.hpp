#pragma once

#include <string>
#include <algorithm>

inline std::wstring StrToW(const std::string& s)
{
	std::wstring temp(s.length(),L' ');
	std::copy(s.begin(), s.end(), temp.begin());
	return temp;
}

inline std::string WToStr(const std::wstring& s)
{
	std::string temp(s.length(), ' ');
	std::copy(s.begin(), s.end(), temp.begin());
	return temp;
}

inline std::string GetPath(std::string fileName)
{
    int last = fileName.find_last_of("/\\");
    return fileName.substr(0, last);
}
