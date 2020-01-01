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
#include "guimainwindow.h"
#include "ui_guimainwindow.h"

GuiMainWindow::GuiMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::GuiMainWindow)
{
    ui->setupUi(this);

    setWindowTitle(QString("%1 v%2").arg(X_APPLICATIONNAME).arg(X_APPLICATIONVERSION));

    setAcceptDrops(true);

    Utils::loadOptions(&options);

    if(!XBinary::isDirectoryExists(XBinary::convertPathName(options.sRootPath)))
    {
        options.sRootPath="";
    }

    if(options.sRootPath=="")
    {
        QMessageBox::information(this,tr("Information"),tr("Please select a root directory for x64dbg"));

        QString sDirectoryName=QFileDialog::getExistingDirectory(this,tr("Select root directory"));

        if(sDirectoryName!="")
        {
            options.sRootPath=sDirectoryName;
        }
        else
        {
            exit(1);
        }
    }

    if(options.sRootPath=="")
    {
        exit(1);
    }

    XBinary::createDirectory(XBinary::convertPathName(options.sDataPath));
    XBinary::createDirectory(XBinary::convertPathName(options.sDataPath)+QDir::separator()+"installed");
    XBinary::createDirectory(XBinary::convertPathName(options.sDataPath)+QDir::separator()+"modules");

    setAcceptDrops(true);

    if(QCoreApplication::arguments().count()>1)
    {
        QString sFileName=QCoreApplication::arguments().at(1);

        openPlugin(sFileName);
    }
    else
    {
        if(!XBinary::isFileExists(Utils::getServerListFileName(&options)))
        {
            updateJsonList();
        }
    }

    adjustTable(ui->tableWidgetServerList);
    adjustTable(ui->tableWidgetInstalled);

    getModules();
}

GuiMainWindow::~GuiMainWindow()
{
    Utils::saveOptions(&options);

    delete ui;
}

void GuiMainWindow::adjustTable(QTableWidget *pTableWidget)
{
    pTableWidget->setColumnCount(CN_size);
    pTableWidget->setRowCount(0);

    pTableWidget->setHorizontalHeaderItem(CN_NAME,          new QTableWidgetItem(tr("Name")));
    pTableWidget->setHorizontalHeaderItem(CN_INFO,          new QTableWidgetItem(tr("Information")));
    pTableWidget->setHorizontalHeaderItem(CN_32,            new QTableWidgetItem(tr("32")));
    pTableWidget->setHorizontalHeaderItem(CN_64,            new QTableWidgetItem(tr("64")));
    pTableWidget->setHorizontalHeaderItem(CN_VERSION,       new QTableWidgetItem(tr("Version")));
    pTableWidget->setHorizontalHeaderItem(CN_INSTALL,       new QTableWidgetItem(""));
    pTableWidget->setHorizontalHeaderItem(CN_REMOVE,        new QTableWidgetItem(""));

    pTableWidget->setColumnWidth(CN_NAME,                   100);
    pTableWidget->setColumnWidth(CN_INFO,                   300);
    pTableWidget->setColumnWidth(CN_32,                     10);
    pTableWidget->setColumnWidth(CN_64,                     10);
    pTableWidget->setColumnWidth(CN_VERSION,                80);
    pTableWidget->setColumnWidth(CN_INSTALL,                60);
    pTableWidget->setColumnWidth(CN_REMOVE,                 60);

    pTableWidget->horizontalHeader()->setSectionResizeMode(CN_NAME,         QHeaderView::ResizeToContents);
    pTableWidget->horizontalHeader()->setSectionResizeMode(CN_INFO,         QHeaderView::Stretch);
    pTableWidget->horizontalHeader()->setSectionResizeMode(CN_32,           QHeaderView::ResizeToContents);
    pTableWidget->horizontalHeader()->setSectionResizeMode(CN_64,           QHeaderView::ResizeToContents);
    pTableWidget->horizontalHeader()->setSectionResizeMode(CN_VERSION,      QHeaderView::ResizeToContents);
    pTableWidget->horizontalHeader()->setSectionResizeMode(CN_INSTALL,      QHeaderView::Interactive);
    pTableWidget->horizontalHeader()->setSectionResizeMode(CN_REMOVE,       QHeaderView::Interactive);
}

void GuiMainWindow::fillTable(QTableWidget *pTableWidget, QList<Utils::MDATA> *pMData, QMap<QString, Utils::STATUS> *pMapStatus)
{
    int nCount=pMData->count();

    pTableWidget->setSortingEnabled(false);

    pTableWidget->setRowCount(0);
    pTableWidget->setRowCount(nCount);

    for(int i=0;i<nCount;i++)
    {
        QTableWidgetItem *pItemName=new QTableWidgetItem;
        pItemName->setText(pMData->at(i).sName);
        pItemName->setData(Qt::UserRole,i);
        pTableWidget->setItem(i,CN_NAME,pItemName);

        QTableWidgetItem *pItemInfo=new QTableWidgetItem;
        pItemInfo->setText(pMData->at(i).sInfo);
        pTableWidget->setItem(i,CN_INFO,pItemInfo);

        QCheckBox *pCheckBoxIs32=new QCheckBox(this);
        pCheckBoxIs32->setEnabled(false);
        pCheckBoxIs32->setChecked(pMData->at(i).bIs32);
        pTableWidget->setCellWidget(i,CN_32,pCheckBoxIs32);

        QCheckBox *pCheckBoxIs64=new QCheckBox(this);
        pCheckBoxIs64->setEnabled(false);
        pCheckBoxIs64->setChecked(pMData->at(i).bIs64);
        pTableWidget->setCellWidget(i,CN_64,pCheckBoxIs64);

        QTableWidgetItem *pItemVersion=new QTableWidgetItem;
        pItemVersion->setText(pMData->at(i).sVersion);
        pTableWidget->setItem(i,CN_VERSION,pItemVersion);

        Utils::STATUS status=pMapStatus->value(pMData->at(i).sName);

        if(status.bInstall||status.bUpdate)
        {
            QToolButton *pPushButtonInstall=new QToolButton(this);
            pPushButtonInstall->setProperty("Name",pMData->at(i).sName);
            connect(pPushButtonInstall,SIGNAL(clicked()),this,SLOT(installButtonSlot()));

            if(status.bInstall)
            {
                pPushButtonInstall->setText(tr("Install"));
            }
            else if(status.bUpdate)
            {
                pPushButtonInstall->setText(tr("Update"));
            }

            pTableWidget->setCellWidget(i,CN_INSTALL,pPushButtonInstall);
        }

        if(status.bRemove)
        {
            QToolButton *pPushButtonRemove=new QToolButton(this);
            pPushButtonRemove->setProperty("Name",pMData->at(i).sName);
            connect(pPushButtonRemove,SIGNAL(clicked()),this,SLOT(removeButtonSlot()));
            pPushButtonRemove->setText(tr("Remove"));
            pTableWidget->setCellWidget(i,CN_REMOVE,pPushButtonRemove);
        }
    }

    pTableWidget->setSortingEnabled(true);
}

void GuiMainWindow::on_actionCreate_triggered()
{
    DialogCreateModule dialogCreateModule(this);

    connect(&dialogCreateModule,SIGNAL(errorMessage(QString)),this,SLOT(errorMessage(QString)));

    dialogCreateModule.exec();
}

void GuiMainWindow::on_pushButtonUpdateAll_clicked()
{
    // TODO
}

void GuiMainWindow::on_actionAbout_triggered()
{
    DialogAbout dialogAbout(this);

    dialogAbout.exec();
}

void GuiMainWindow::on_actionOpen_triggered()
{
    QString sInitDirectory; // TODO

    QString sFileName=QFileDialog::getOpenFileName(this,tr("Open plugin"),sInitDirectory,"*.x64dbg.zip");

    if(sFileName!="")
    {
        openPlugin(sFileName);
    }
}

void GuiMainWindow::on_actionOptions_triggered()
{
    DialogOptions dialogOptions(this,&options);

    dialogOptions.exec();

    Qt::WindowFlags wf=windowFlags();

    if(options.bStayOnTop)
    {
        wf|=Qt::WindowStaysOnTopHint;
    }
    else
    {
        wf&=~(Qt::WindowStaysOnTopHint);
    }

    setWindowFlags(wf);

    show();
}

void GuiMainWindow::on_pushButtonReload_clicked()
{
    updateJsonList();

    getModules();
}

void GuiMainWindow::on_actionExit_triggered()
{
    this->close();
}

void GuiMainWindow::errorMessage(QString sMessage)
{
    QMessageBox::critical(this,tr("Error"),sMessage);
}

void GuiMainWindow::getModules()
{
    modulesData=Utils::getModulesData(&options);

    fillTable(ui->tableWidgetServerList,&(modulesData.listServerList),&(modulesData.mapStatus));
    fillTable(ui->tableWidgetInstalled,&(modulesData.listInstalled),&(modulesData.mapStatus));
}

void GuiMainWindow::openPlugin(QString sFileName)
{
    if(Utils::isPluginValid(sFileName))
    {
        Utils::MDATA mdata=Utils::getMDataFromZip(sFileName,XBinary::convertPathName(options.sRootPath));

        QString sDestFile=Utils::getModuleFileName(&options,mdata.sName);

        if(XBinary::isFileExists(sDestFile))
        {
            XBinary::removeFile(sDestFile); // TODO handle errors
        }

        if(XBinary::copyFile(sFileName,sDestFile))
        {
            DialogInstallModule dialogInstallModule(this,&options,sDestFile);

            dialogInstallModule.exec();

            getModules();
        }
    }
    else
    {
        errorMessage(tr("Invalid plugin file"));
    }
}

void GuiMainWindow::updateJsonList()
{
    Utils::WEB_RECORD record={};

    record.sFileName=Utils::getServerListFileName(&options);
    record.sLink=options.sJSONLink;

    DialogGetFileFromServerProcess dialogGetFileFromServer(this,QList<Utils::WEB_RECORD>()<<record);

    dialogGetFileFromServer.exec();
}

void GuiMainWindow::installButtonSlot()
{
    QToolButton *pPushButton=qobject_cast<QToolButton *>(sender());
    QString sName=pPushButton->property("Name").toString();

    if(sName!="") // TODO Check
    {
        Utils::MDATA mdata=Utils::getMDataByName(&(modulesData.listServerList),sName);

        if(mdata.sName!="")
        {
            QString sFileName=Utils::getModuleFileName(&options,mdata.sName);

            if(!XBinary::isFileHashValid(XBinary::HASH_SHA1,sFileName,mdata.sSHA1))
            {
                Utils::WEB_RECORD record={};

                record.sFileName=sFileName;
                record.sLink=mdata.sSrc;

                DialogGetFileFromServerProcess dialogGetFileFromServer(this,QList<Utils::WEB_RECORD>()<<record);

                dialogGetFileFromServer.exec();
            }

            if(XBinary::isFileHashValid(XBinary::HASH_SHA1,sFileName,mdata.sSHA1))
            {
                DialogInstallModule dialogInstallModule(this,&options,sFileName);

                dialogInstallModule.exec();

                getModules();
            }
            else
            {
                // TODO errors invalid SHA1
            }
        }
    }
}

void GuiMainWindow::removeButtonSlot()
{
    QToolButton *pPushButton=qobject_cast<QToolButton *>(sender());
    QString sName=pPushButton->property("Name").toString();

    if(sName!="") // TODO Check
    {
        DialogRemoveModule dialogRemoveModule(this,&options,sName);

        dialogRemoveModule.exec();

        getModules();
    }
}

void GuiMainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    const QMimeData* mimeData=event->mimeData();

    if(mimeData->hasUrls())
    {
        QList<QUrl> urlList=mimeData->urls();

        if(urlList.count())
        {
            QString sFileName=urlList.at(0).toLocalFile();

            if(Utils::isPluginValid(sFileName))
            {
                event->acceptProposedAction();
            }
        }
    }
}

void GuiMainWindow::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void GuiMainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData* mimeData=event->mimeData();

    if(mimeData->hasUrls())
    {
        QList<QUrl> urlList=mimeData->urls();

        if(urlList.count())
        {
            QString sFileName=urlList.at(0).toLocalFile();

            sFileName=XBinary::convertFileName(sFileName);

            openPlugin(sFileName);
        }
    }
}

void GuiMainWindow::on_tableWidgetServerList_customContextMenuRequested(const QPoint &pos)
{
    qDebug("on_tableWidgetServerList_customContextMenuRequested");
}

void GuiMainWindow::on_tableWidgetInstalled_customContextMenuRequested(const QPoint &pos)
{
    qDebug("on_tableWidgetInstalled_customContextMenuRequested");
}

void GuiMainWindow::_infoPluginServerList()
{

}

void GuiMainWindow::_infoPluginInstalled()
{

}
