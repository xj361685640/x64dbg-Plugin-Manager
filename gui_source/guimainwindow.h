// Copyright (c) 2019 hors<horsicq@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#ifndef GUIMAINWINDOW_H
#define GUIMAINWINDOW_H

#include <QMainWindow>
#include "../global.h"
#include "dialogcreatemodule.h"
#include "dialoginstallmodule.h"
#include "dialogremovemodule.h"
#include "dialogoptions.h"
#include "dialoggetfilefromserverprocess.h"
#include "dialogabout.h"
#include <QMessageBox>
#include <QCheckBox>
#include <QToolButton>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QTableWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class GuiMainWindow; }
QT_END_NAMESPACE

class GuiMainWindow : public QMainWindow
{
    Q_OBJECT

    enum CN
    {
        CN_NAME=0,
        CN_INFO,
        CN_32,
        CN_64,
        CN_VERSION,
        CN_INSTALL,
        CN_REMOVE,
        CN_size
    };

public:
    GuiMainWindow(QWidget *parent=nullptr);
    ~GuiMainWindow();

private:
    void adjustTable(QTableWidget *pTableWidget);
    void fillTable(QTableWidget *pTableWidget,QList<Utils::MDATA> *pMData,QMap<QString,Utils::STATUS> *pMapStatus);

private slots:
    void on_actionCreate_triggered();
    void on_pushButtonUpdateAll_clicked();
    void on_actionAbout_triggered();
    void on_actionOpen_triggered();
    void on_actionOptions_triggered();
    void on_pushButtonReload_clicked();
    void on_actionExit_triggered();
    void errorMessage(QString sMessage);
    void getModules();
    void openPlugin(QString sFileName);
    void updateJsonList();
    void installButtonSlot();
    void removeButtonSlot();
    void on_tableWidgetServerList_customContextMenuRequested(const QPoint &pos);
    void on_tableWidgetInstalled_customContextMenuRequested(const QPoint &pos);
    void _infoPluginServerList();
    void _infoPluginInstalled();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    Ui::GuiMainWindow *ui;
    XPLUGINMANAGER::OPTIONS options;
    Utils::MODULES_DATA modulesData;
};
#endif // GUIMAINWINDOW_H
