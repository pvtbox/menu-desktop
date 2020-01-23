/**
*  
*  Pvtbox. Fast and secure file transfer & sync directly across your devices. 
*  Copyright © 2020  Pb Private Cloud Solutions Ltd. 
*  
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*  
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*  
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*  
**/

#ifndef STRINGUTIL_H
#define STRINGUTIL_H

#pragma once

#include <string>
#include <cassert>

class __declspec(dllexport) StringUtil {
public:
    static std::string  toUtf8(const wchar_t* utf16, int len = -1);
    static std::wstring toUtf16(const char* utf8, int len = -1);

    template<class T>
    static bool begins_with(const T& input, const T& match)
    {
        return input.size() >= match.size()
            && std::equal(match.begin(), match.end(), input.begin());
    }

    static bool isContainedIn(const std::wstring& child, const std::wstring& parent) {
        return isContainedIn(child.c_str(), child.size(), parent.c_str(), parent.size());
    }

    static bool isContainedIn(PCWSTR child, size_t childLength, const std::wstring& parent) {
        return isContainedIn(child, childLength, parent.c_str(), parent.size());
    }

    static bool isContainedIn(PCWSTR child, size_t childLength, PCWSTR parent, size_t parentLength) {
        if (!parentLength)
            return false;
        return (childLength == parentLength || childLength > parentLength && (child[parentLength] == L'\\' || child[parentLength - 1] == L'\\'))
            && wcsncmp(child, parent, parentLength) == 0;
    }

    static bool ends_with(std::wstring const &fullString, std::wstring const &ending) {
        if (fullString.length() >= ending.length()) {
            return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
        }
        else {
            return false;
        }
    }


};

#endif // STRINGUTIL_H
