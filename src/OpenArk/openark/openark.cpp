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
#include "openark.h"
#include "common/common.h"
#include "process-mgr/process-mgr.h"
#include "scanner/scanner.h"
#include "coderkit/coderkit.h"
#include "bundler/bundler.h"
#include "settings/settings.h"
#include "about/about.h"
#include "cmds/cmds.h"

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

#define APP_CHKUPT_SERVER "http://upt.blackint3.com/openark/version.txt"
#define APP_MESSAGE_PATTERN \
"%{if-debug}<font color=blue>%{endif}"\
"%{if-info}<font color=black>%{endif}"\
"%{if-warning}<font color=red>%{endif}"\
"%{if-critical}<font color=red>%{endif}"\
"[%{function}:%{line}]"\
"%{if-debug}[DBG]%{endif}%{if-info}[INFO]%{endif}%{if-warning}[WARN]%{endif}%{if-critical}[ERR]%{endif}"\
"%{message}</font>"
void QtMessageHandlerCallback(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	QString fmt = msg;
	if (!msg.isEmpty()) {
		fmt = qFormatLogMessage(type, context, msg);
	}
	app->onLogOutput(fmt);
}

OpenArk::OpenArk(QWidget *parent) : 
	QMainWindow(parent)
{
	app = this;
	qSetMessagePattern(APP_MESSAGE_PATTERN);
	qInstallMessageHandler(QtMessageHandlerCallback);
	UNONE::InterRegisterLogger([&](const std::wstring &log) {
		onLogOutput(WStrToQ(log));
	});

	ui.setupUi(this);
	resize(1300, 800);
	ui.splitter->setStretchFactor(0, 1);
	ui.splitter->setStretchFactor(1, 5);
	QString title = QString(tr("OpenArk v%1 ").arg(AppVersion()));
	title.append(tr(" [build:%1]  ").arg(AppBdTime()));

	UNONE::PeX64((CHAR*)GetModuleHandleW(NULL)) ? title.append(tr("64-Bit")) : title.append(tr("32-Bit"));

	setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);
	setWindowTitle(title);

	QWidget *widget = new QWidget();
	QLabel *link = new QLabel(widget);
	link->setText("<a style='color:blue;a{text-decoration: none}' href=\"https://github.com/BlackINT3/OpenArk\">"+ tr("Project on Github")+"</a>&nbsp;");
	link->setOpenExternalLinks(true);

	stool_ = new QToolBar(widget);
	stool_->setObjectName(QStringLiteral("statusToolBar"));
	stool_->setIconSize(QSize(16, 16));
	stool_->addAction(ui.actionConsole);
	//stool_->setStyleSheet("background-color:red");

	QGridLayout *layout = new QGridLayout(widget);
	layout->setMargin(0);
	layout->addWidget(stool_, 0, 0, 1, 1, Qt::AlignVCenter | Qt::AlignLeft);
	layout->addWidget(link, 0, 1, 1, 1, Qt::AlignVCenter | Qt::AlignRight);
	//ui.statusBar->setStyleSheet("QToolButton: {background-color: red; }");
	ui.statusBar->setSizeGripEnabled(false);
	ui.statusBar->setFixedHeight(28);
	ui.statusBar->addWidget(widget, 1);
	connect(ui.actionConsole, SIGNAL(triggered(bool)), this, SLOT(onActionConsole(bool)));

	ui.consoleWidget->hide();
	ui.actionOnTop->setCheckable(true);
	ui.actionExit->setShortcut(QKeySequence(Qt::ALT + Qt::Key_F4));
	connect(ui.actionExit, &QAction::triggered, this, [=]() { QApplication::quit(); });
	connect(ui.actionAbout, SIGNAL(triggered(bool)), this, SLOT(onActionAbout(bool)));
	connect(ui.actionSettings, SIGNAL(triggered(bool)), this, SLOT(onActionSettings(bool)));
	connect(ui.actionOpen, SIGNAL(triggered(bool)), this, SLOT(onActionOpen(bool)));
	connect(ui.actionRefresh, SIGNAL(triggered(bool)), this, SLOT(onActionRefresh(bool)));
	connect(ui.actionReset, SIGNAL(triggered(bool)), this, SLOT(onActionReset(bool)));
	connect(ui.actionOnTop, SIGNAL(triggered(bool)), this, SLOT(onActionOnTop(bool)));
	connect(ui.actionGithub, SIGNAL(triggered(bool)), this, SLOT(onActionGithub(bool)));
	connect(ui.actionManuals, SIGNAL(triggered(bool)), this, SLOT(onActionManuals(bool)));
	connect(ui.actionCheckUpdate, SIGNAL(triggered(bool)), this, SLOT(onActionCheckUpdate(bool)));
	connect(ui.actionCoderKit, SIGNAL(triggered(bool)), this, SLOT(onActionCoderKit(bool)));
	connect(ui.actionScanner, SIGNAL(triggered(bool)), this, SLOT(onActionScanner(bool)));
	connect(ui.actionBundler, SIGNAL(triggered(bool)), this, SLOT(onActionBundler(bool)));

	cmds_ = new Cmds(ui.cmdOutWindow);
	cmds_->hide();
	cmds_->setParent(ui.cmdOutWindow);
	ui.cmdInput->installEventFilter(this);
	ui.cmdOutWindow->installEventFilter(this);
	ui.cmdOutWindow->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui.cmdInput, SIGNAL(returnPressed()), this, SLOT(onCmdInput()));
	connect(ui.cmdButton, SIGNAL(clicked()), this, SLOT(onCmdHelp()));
	connect(ui.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onTabChanged()));
	connect(ui.cmdOutWindow, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onShowConsoleMenu(const QPoint &)));

	auto CreateTabPage = [&](QWidget *widget, QWidget *origin) {
		int idx = ui.tabWidget->indexOf(origin);
		QString text = ui.tabWidget->tabText(idx);
		ui.tabWidget->removeTab(idx);
		ui.tabWidget->insertTab(idx, widget, text);
	};

	CreateTabPage(new ProcessMgr(this), ui.tabProcessMgr);
	CreateTabPage(new Scanner(this), ui.tabScanner);
	CreateTabPage(new CoderKit(this), ui.tabCoderKit);
	CreateTabPage(new Bundler(this), ui.tabBundler);
	ActivateTab(1);

	chkupt_timer_ = new QTimer();
	chkupt_timer_->setInterval(5000);
	chkupt_timer_->start();
	connect(chkupt_timer_, &QTimer::timeout, this, [&]() {
		onActionCheckUpdate(false);
		chkupt_timer_->stop();
	});
}

bool OpenArk::eventFilter(QObject *obj, QEvent *e)
{
	bool filtered = false;
	if (obj == ui.cmdInput) {
		if (e->type() == QEvent::KeyPress) {
			filtered = true;
			QKeyEvent* keyevt = dynamic_cast<QKeyEvent*>(e);
			if (keyevt->key() == Qt::Key_Up) {
				ui.cmdInput->setText(cmds_->CmdGetLast());
			}	else if (keyevt->key() == Qt::Key_Down) {
				ui.cmdInput->setText(cmds_->CmdGetNext());
			} else if (keyevt->key() == Qt::Key_Tab) {
				ui.cmdOutWindow->setFocus();
			} else {
				filtered = false;
			}
		}
	}	else if (obj == ui.cmdOutWindow) {
		if (e->type() == QEvent::KeyPress) {
			filtered = true;
			QKeyEvent* keyevt = dynamic_cast<QKeyEvent*>(e);
			if (keyevt->key() == Qt::Key_Tab) {
				ui.cmdInput->setFocus();
			} else {
				filtered = false;
			}
		}
	}
	if (filtered) {
		QKeyEvent* keyevt = dynamic_cast<QKeyEvent*>(e);
		keyevt->ignore();
		return true;
	}
	return QWidget::eventFilter(obj, e);
}

void OpenArk::onActionOpen(bool checked)
{
	QString file = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("All Files (*.*)"));
	if (file.isEmpty()) return;
	QMetaObject::invokeMethod(ui.tabWidget->currentWidget(), "onOpenFile", Qt::QueuedConnection, Q_ARG(QString, file));
}

void OpenArk::onActionRefresh(bool checked)
{
	QMetaObject::invokeMethod(ui.tabWidget->currentWidget(), "onRefresh", Qt::QueuedConnection);
}

void OpenArk::onActionReset(bool checked)
{
	QMetaObject::invokeMethod(ui.tabWidget->currentWidget(), "onReset", Qt::QueuedConnection);
}

void OpenArk::onActionOnTop(bool checked)
{
	HWND wnd = (HWND)winId();
	DWORD style = ::GetWindowLong(wnd, GWL_EXSTYLE);
	QAction* sender = qobject_cast<QAction*>(QObject::sender());
	if (sender->isChecked()) {
		style |= WS_EX_TOPMOST;
		::SetWindowLong(wnd, GWL_EXSTYLE, style);
		::SetWindowPos(wnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOREPOSITION | SWP_NOSIZE | SWP_SHOWWINDOW);
	}	else {
		style &= ~WS_EX_TOPMOST;
		::SetWindowLong(wnd, GWL_EXSTYLE, style);
		::SetWindowPos(wnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOREPOSITION | SWP_NOSIZE | SWP_SHOWWINDOW);
	}
}

void OpenArk::onActionAbout(bool checked)
{
	auto about = new About(this);
	about->raise();
	about->show();
}

void OpenArk::onActionSettings(bool checked)
{
	auto about = new Settings(this);
	about->raise();
	about->show();
}

void OpenArk::onActionConsole(bool checked)
{
	QAction* sender = qobject_cast<QAction*>(QObject::sender());
	if (sender->isChecked()) {
		ui.consoleWidget->show();
		ui.cmdInput->setFocus();
	}	else {
		ui.consoleWidget->hide();
	}
}

void OpenArk::onActionManuals(bool checked)
{
	OpenBrowserUrl("https://openark.blackint3.com/manuals/");
}

void OpenArk::onActionGithub(bool checked)
{
	OpenBrowserUrl("https://github.com/BlackINT3/OpenArk/");
}

void OpenArk::onActionCheckUpdate(bool checked)
{
	QString url = APP_CHKUPT_SERVER;
	QNetworkRequest req;
	req.setUrl(QUrl(url));
	QNetworkAccessManager *mgr = new QNetworkAccessManager(this);
	QNetworkReply *reply = mgr->get(req);
	
	INFO(L"requset server:%s", url.toStdWString().c_str());

	connect(reply, &QNetworkReply::finished, [this, reply, checked]() {
		if (reply->error() != QNetworkReply::NoError) {
			auto err = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
			ERR(L"request http.err:%d, net.err:%d", err.toInt(), (int)reply->error());
			return;
		}
		QByteArray data = reply->readAll();
		QJsonValue val;
		QJsonObject obj;
		
		INFO("server responsed:%s", data.toStdString().c_str());

		if (!JsonParse(data, obj) || !JsonGetValue(obj, "err", val)) {
			ERR(L"request app-err: json invalid");
			return;
		}
		if (val.toInt() != 0) {
			ERR(L"request app-err: %d", val.toInt());
			return;
		}
		QJsonValue appver, appbd, appurl;
		if (!JsonGetValue(obj, "appver", appver) || !JsonGetValue(obj, "appbd", appbd) || !JsonGetValue(obj, "appurl", appurl)) {
			ERR(L"request json err: %d", val.toInt());
			return;
		}
		auto local_ver = AppVersion();
		auto local_bd = AppBdTime();
		INFO(L"local appver:%s, build:%s", local_ver.toStdWString().c_str(), local_bd.toStdWString().c_str());
		if (local_ver.isEmpty() || local_bd.isEmpty()) {
			return;
		}
		if (local_ver < appver.toString() || local_bd < appbd.toString()) {
			QString tips = QString(tr("Found new version, app should be updated.\nappver: %1\nappbd: %2\nappurl: %3")
				.arg(appver.toString())
				.arg(appbd.toString())
				.arg(appurl.toString()));
			QMessageBox::StandardButton reply;
			reply = QMessageBox::question(this, tr("App Updates"), tips, QMessageBox::Yes | QMessageBox::No);
			if (reply == QMessageBox::Yes) {
				OpenBrowserUrl(appurl.toString());
			}
			return;
		}
		if (checked) MsgBoxInfo(tr("OpenArk is latest."));
		INFO(L"OpenArk is latest.");
		reply->deleteLater();
	});
}

void OpenArk::onActionCoderKit(bool checked)
{
	ActivateTab(1);
}

void OpenArk::onActionScanner(bool checked)
{
	ActivateTab(2);
}

void OpenArk::onActionBundler(bool checked)
{
	ActivateTab(3);
}

void OpenArk::onLogOutput(QString log)
{
	log.replace("\n", "<br/>");
	log.replace("err", "<font color=red>err</font>");
	log.replace("error", "<font color=red>error</font>");
	log.replace("ERR", "<font color=red>ERR</font>");
	log.replace("ERROR", "<font color=red>ERROR</font>");
	ui.cmdOutWindow->append(log);
}

void OpenArk::onExecCmd(const std::wstring &cmdline)
{
	cmds_->CmdDispatcher(cmdline);
}

void OpenArk::onOpen(QString path)
{
	QMetaObject::invokeMethod(ui.tabWidget->currentWidget(), "onOpenFile", Qt::QueuedConnection, Q_ARG(QString, path));
}

void OpenArk::onCmdHelp()
{
	onExecCmd(L".help");
}

void OpenArk::onShowConsoleMenu(const QPoint &pt)
{
	QMenu *menu = ui.cmdOutWindow->createStandardContextMenu();
	menu->addSeparator();
	menu->addAction(tr("History"), this, SLOT(onConsoleHistory()));
	menu->addAction(tr("Helps"), this, SLOT(onConsoleHelps()));
	menu->addAction(tr("Clear"), this, SLOT(onConsoleClear()));
	menu->exec(ui.cmdOutWindow->mapToGlobal(pt));
	delete menu;
}

void OpenArk::onConsoleHistory()
{
	onExecCmd(L".history");
}

void OpenArk::onConsoleClear()
{
	onExecCmd(L".cls");
}

void OpenArk::onConsoleHelps()
{
	onExecCmd(L".help");
}

void OpenArk::onCmdInput()
{
	QLineEdit* sender = qobject_cast<QLineEdit*>(QObject::sender());
	std::wstring input = sender->text().toStdWString();
	if (input.empty()) input = cmds_->CmdGetLast().toStdWString();
	onExecCmd(input);
	auto scroll = ui.cmdOutWindow->verticalScrollBar();
	scroll->setSliderPosition(scroll->maximum());
	sender->clear();
}

void OpenArk::onTabChanged()
{
}

void OpenArk::StatusBarClear()
{
}

void OpenArk::StatusBarAdd(QWidget *label)
{
	stool_->addSeparator();
	stool_->addWidget(label);
}

void OpenArk::ActivateTab(int idx)
{
	ui.tabWidget->setCurrentIndex(idx);
}