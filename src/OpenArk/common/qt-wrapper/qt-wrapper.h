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
#include <unone.h>
#include <Windows.h>
#include <QtGui>
#include <QtCore>
#include <QtWidgets>
#include <QIcon>
#include <QPainter>
#include <QProxyStyle>
#include <QSize>
#include <QTreeView>
#include <QModelIndex>
#include <QStandardItem>
#include <QAbstractItemView>
#include <QMessageBox>
#include <QPoint>

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

class OpenArkTabStyle : public QProxyStyle {
public:
	QSize sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const;
	void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
};

#define CharsToQ(chars) QString::fromLocal8Bit(chars)
#define WCharsToQ(wchars) QString::fromWCharArray(wchars)
#define StrToQ(str) QString::fromStdString(str)
#define WStrToQ(wstr) QString::fromStdWString(wstr)
#define ByteToHexQ(w) StrToQ(UNONE::StrFormatA("%02X", w))
#define WordToHexQ(w) StrToQ(UNONE::StrFormatA("%04X", w))
#define DWordToDecQ(w) StrToQ(UNONE::StrFormatA("%d", w))
#define DWordToHexQ(w) StrToQ(UNONE::StrFormatA("%08X", w))
#define QWordToDecQ(w) StrToQ(UNONE::StrFormatA("%lld", w))
#define QWordToHexQ(w) StrToQ(UNONE::StrFormatA("%016llX", w))
inline QStringList VectorToQList(const std::vector<std::string>& vec)
{
	QStringList result;
	for (auto& s : vec) {
		result.append(StrToQ(s));
	}
	return result;
};
inline QStringList WVectorToQList(const std::vector<std::wstring>& vec)
{
	QStringList result;
	for (auto& s : vec) {
		result.append(WStrToQ(s));
	}
	return result;
};
inline std::vector<std::string> QListToVector(const QStringList& lst)
{
	std::vector<std::string> result;
	for (auto& s : lst) {
		result.push_back(s.toStdString());
	}
	return result;
};
inline std::vector<std::wstring> QListToWVector(const QStringList& lst)
{
	std::vector<std::wstring> result;
	for (auto& s : lst) {
		result.push_back(s.toStdWString());
	}
	return result;
};

__inline QString ByteArrayToHexQ(BYTE* arr, int len) {
	std::string str = UNONE::StrStreamToHexStrA(std::string((char*)arr, len));
	str = UNONE::StrInsertA(str, 2, " ");
	return StrToQ(str);
};
__inline QString WordArrayToHexQ(WORD* arr, int len) {
	std::string str = UNONE::StrStreamToHexStrA(std::string((char*)arr, len * 2));
	str = UNONE::StrInsertA(str, 4, " ");
	return StrToQ(str);
};

#define AppendTreeItem(root, name, value) \
	item = new QStandardItem(name); \
	root->appendRow(item); \
	root->setChild(row++, 1, new QStandardItem(value));

#define InitTableItem(root) \
	int column = 0;\
	int count = root->rowCount();\
	QStandardItem *item;\

#define InitTableItem2(root, cnt) \
	int column = 0;\
	count = cnt;\
	QStandardItem *item;\

#define AppendTableItem(root, value) \
	item = new QStandardItem(value);\
	root->setItem(count, column++, item);

#define AppendTableIconItem(root, ico, value) \
	item = new QStandardItem(ico, value);\
	root->setItem(count, column++, item);

#define AppendNameValue(root, name, value) \
	root->setItem(count, 0, new QStandardItem(name)); \
	root->setItem(count, 1, new QStandardItem(value)); \
	count++

#define AppendTableRowNameVaule(root, name, value) \
	count = root->rowCount();\
	root->setItem(count, 0, new QStandardItem(name)); \
	root->setItem(count, 1, new QStandardItem(value)); \

// MVC wrapper
QString GetItemModelData(QAbstractItemModel *model, int row, int column);
QString GetItemViewData(QAbstractItemView *view, int row, int column);
QString GetCurItemViewData(QAbstractItemView *view, int column);
int GetCurViewRow(QAbstractItemView *view);
void ClearItemModelData(QStandardItemModel* model, int pos = 0);
void ExpandTreeView(const QModelIndex& index, QTreeView* view);
void SetDefaultTableViewStyle(QTableView* view, QStandardItemModel* model);
void SetDefaultTreeViewStyle(QTreeView* view, QStandardItemModel* model);

// Others
QIcon LoadIcon(QString file_path);
bool IsContainAction(QMenu *menu, QObject *obj);
bool ExploreFile(QString file_path);
QString MsToTime(LONGLONG ms);


// Json
bool JsonParse(const QByteArray &data, QJsonObject &obj);
bool JsonGetValue(const QJsonObject &obj, const QString &key, QJsonValue &val);
bool JsonGetValue(const QByteArray &data, const QString &key, QJsonValue &val);

//
void OpenBrowserUrl(QString url);
QString PidFormat(DWORD pid);
QString NameFormat(QString name);