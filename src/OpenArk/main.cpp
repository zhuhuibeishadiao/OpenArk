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
#include "../openark/openark.h"
#include "common/common.h"

//for qt static link
#include <QtWidgets/QApplication>
#include <QtPlugin>
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
Q_IMPORT_PLUGIN(QICOPlugin)

int OpenArkInit(int argc, char *argv[])
{
	UNONE::SeEnableDebugPrivilege();
	bool is_ark64 = UNONE::PeX64((CHAR*)GetModuleHandleW(NULL));
	if (!is_ark64 && UNONE::OsIs64()) {
		auto &&path = UNONE::PsGetProcessDirW() + L"\\OpenArk64.exe";
		if (UNONE::FsIsExistedW(path)) {
			UNONE::PsCreateProcessW(path);
			exit(0);
		}
	}

	ConfigInit();
	return 0;
}

int main(int argc, char *argv[])
{
	OpenArkInit(argc, argv);

	QApplication app(argc, argv);
	app.setWindowIcon(QIcon(":/OpenArk/OpenArk.ico"));

	QTranslator translator;
	translator.load(":/OpenArk/lang/openark_zh.qm");
	app.installTranslator(&translator);

	OpenArk w;
	w.show();

	return app.exec();
}
