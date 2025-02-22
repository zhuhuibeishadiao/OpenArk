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
#include "config.h"
#include "../common.h"

QSettings *appconf = nullptr;

void ConfigInit()
{
	auto &&cpath = UNONE::OsEnvironmentW(L"%AppData%") + L"\\OpenArk\\openark.ini";
	UNONE::FsCreateDirW(UNONE::FsPathToDirW(cpath));
	appconf = new QSettings(WStrToQ(cpath), QSettings::IniFormat);
}

QString ConfigGetConsole(const QString &name)
{
	QString section = "/Console/";
	auto key = section + name;
	if (name == "History.MaxRecords") {
		return appconf->value(key, "2000").toString();
	}
	if (name == "History.FilePath") {
		auto &&default_path = UNONE::OsEnvironmentW(L"%AppData%") + L"\\OpenArk\\console\\history.txt";
		if (!UNONE::FsIsExistedW(default_path)) {
			UNONE::FsCreateDirW(UNONE::FsPathToDirW(default_path));
			UNONE::FsWriteFileDataW(default_path, "");
		}
		return appconf->value(key, WStrToQ(default_path)).toString();
	}
	return "";
}