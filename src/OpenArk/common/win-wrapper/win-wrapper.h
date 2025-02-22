/****************************************************************************
**
** Copyright (C) 2019 BlackINT3
** Contact: https://github.com/BlackINT3/OpenArk
**
** GNU Lesser General Public License Usage (LGPL)
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
****************************************************************************/
#pragma once
#include <vector>
#include <map>
#include <string>

#include <unone.h>
using namespace UNONE::Plugins;

bool RetrieveThreadTimes(DWORD tid, std::wstring& ct, std::wstring& kt, std::wstring& ut);
std::wstring FormatFileTime(FILETIME *file_tm);
std::wstring ProcessCreateTime(__in DWORD pid);
bool CreateDump(DWORD pid, const std::wstring& path, bool mini);
void ClipboardCopyData(const std::string &data);
std::vector<HWND> GetSystemWnds();
int64_t FileTimeToInt64(FILETIME tm);
double GetSystemUsageOfCPU();
double GetSystemUsageOfMemory();
SIZE_T GetProcessPrivateWorkingSet(DWORD pid);
