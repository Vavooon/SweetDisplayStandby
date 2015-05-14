#include "sweetdisplaystandby.h"
#include "ui_sweetdisplaystandby.h"


std::vector<HANDLE> MonHandles;



BOOL QT_WIN_CALLBACK EnumProc(
    HMONITOR hMonitor,
    HDC,
    LPRECT,
    LPARAM
    )
{
    LPPHYSICAL_MONITOR pMons = NULL;
    DWORD i, mcnt;
    if (!GetNumberOfPhysicalMonitorsFromHMONITOR(hMonitor, &mcnt))
    {
        return TRUE;
    }
    pMons = (LPPHYSICAL_MONITOR)malloc(mcnt * sizeof(PHYSICAL_MONITOR));
    if (GetPhysicalMonitorsFromHMONITOR(hMonitor, mcnt, pMons))
    {
        MonHandles.clear();
        qDebug() << QTime::currentTime().toString() <<"Adding monitors";
        for (i = 0; i < mcnt; i++)
        {
            MonHandles.push_back(pMons[i].hPhysicalMonitor);

            qDebug() << QTime::currentTime().toString() <<pMons[i].hPhysicalMonitor;
//            foreach(QWidget *widget, QApplication::topLevelWidgets()) {
//              if(widget->objectName() == "SweetDisplayStandby")
//              {
//                  SweetDisplayStandby *w = qobject_cast<SweetDisplayStandby *>(widget);
//                  w->addMonitorToList(pMons[i]);
//              }
//            }
        }
    }
    free(pMons);
    return TRUE;
}


SweetDisplayStandby::SweetDisplayStandby(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SweetDisplayStandby)
{

    displayWasDimmed = false;
    displayWasTurnedOff = false;
    forceDisplayTurnOff = false;
    turnOffHotKeyId = 1;
    restoreHotKeyId = 2;
    lastInputTime = 0;
    const GUID GUID_DEVINTERFACE_MONITOR = {0xe6f07b5f, 0xee97, 0x4a90, 0xb0, 0x76, 0x33, 0xf5, 0x7b, 0xf4, 0xea, 0xa7};


    ui->setupUi(this);

    appIcon = QIcon(":/main/icon.png");

    createTrayIcon();
    setWindowFlags(Qt::WindowTitleHint | Qt::WindowSystemMenuHint |Qt:: WindowMinimizeButtonHint | Qt::WindowCloseButtonHint  | Qt::MSWindowsFixedSizeDialogHint );

    QCoreApplication::setOrganizationName("Vavooon");
    QCoreApplication::setApplicationName("SweetDisplayStandby");
    settings = new QSettings();
    displayDimTime = settings->value("dimTime", 30).toInt();
    displayOffTime = settings->value("turnOffTime", 3*60).toInt();
    normalBrightnessLevel = settings->value("normalBrightnessLevel", 60).toInt();
    dimmedBrightnessLevel = settings->value("dimmedBrightnessLevel", 0).toInt();
    enableBrighnessManagement = settings->value("enableBrighnessManagement", true).toBool();
    turnOffSequence = QKeySequence( settings->value("turnOffSequence", "Ctrl+F1").toString() );
    restoreSequence = QKeySequence( settings->value("restoreSequence", "Ctrl+F12").toString() );

    ui->dimTimeInput->setValue( displayDimTime );
    ui->turnOffTimeInput->setValue( displayOffTime );
    ui->normalBrightnessLevelInput->setValue( normalBrightnessLevel );
    ui->dimmedBrightnessLevelInput->setValue( dimmedBrightnessLevel );
    ui->brightnessStateCheckBox->setChecked( enableBrighnessManagement );
    ui->brightnessGroupBox->setDisabled(!enableBrighnessManagement);
    ui->dimTimeInput->setDisabled(!enableBrighnessManagement);

    ui->statusBar->showMessage("Display is active");

    connect( ui->actionHideToTray, SIGNAL(triggered()), this, SLOT(hide()));
    connect( ui->actionHideCompletely, SIGNAL(triggered()), this, SLOT(hideCompletely()) );
    connect( ui->actionAbout, SIGNAL(triggered(bool)), this, SLOT(aboutPopup()));
    connect( ui->actionProjectSite, SIGNAL(triggered(bool)), this, SLOT(openSite()) );
    connect( ui->actionExit, SIGNAL(triggered(bool)), this, SLOT( exit() ) );

    connect( ui->dimTimeInput, SIGNAL(valueChanged(int)), this, SLOT( onDimTimeChange(int)) );
    connect( ui->turnOffTimeInput, SIGNAL(valueChanged(int)), this, SLOT( onTurnOffTimeChange(int)) );
    connect( ui->normalBrightnessLevelInput, SIGNAL( valueChanged(int)), this, SLOT(onNormalBrightnessLevelChange(int)) );
    connect( ui->dimmedBrightnessLevelInput, SIGNAL( valueChanged(int)), this, SLOT(onDimmedBrightnessLevelChange(int)) );
    connect( ui->brightnessStateCheckBox, SIGNAL(toggled(bool)), this, SLOT(onBrightnessStateChange(bool)) );
//    connect( ui->allDisplaysRadio, SIGNAL(toggled(bool)), ui->displaysList, SLOT(setEnabled(bool)) );

    connect( ui->turnOffSequenceEdit, SIGNAL(keySequenceChanged(QKeySequence)), this, SLOT(updateTurnOffSequence(QKeySequence)) );
    connect( ui->restoreSequenceEdit, SIGNAL(keySequenceChanged(QKeySequence)), this, SLOT(updateRestoreSequence(QKeySequence)) );

    ui->turnOffSequenceEdit->setKeySequence(turnOffSequence);
    ui->restoreSequenceEdit->setKeySequence(restoreSequence);

    EnumDisplayMonitors(0, 0, EnumProc, 0);


    DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;
    ZeroMemory(&NotificationFilter, sizeof(NotificationFilter));

    NotificationFilter.dbcc_size = sizeof(NotificationFilter);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    NotificationFilter.dbcc_reserved = 0;

    NotificationFilter.dbcc_classguid = GUID_DEVINTERFACE_MONITOR;

    RegisterDeviceNotification((HWND)SweetDisplayStandby::winId(),&NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE );



    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerTick()));
    timer->setInterval(100);
    timer->start();

    QTimer *tickTimer = new QTimer(this);
    tickTimer->setInterval(1000);
    connect(tickTimer, SIGNAL(timeout()), this, SLOT(queueTimerTick()));
    tickTimer->start();

    addToQueue("turnOn");
    addToQueue("illuminate");
}

SweetDisplayStandby::~SweetDisplayStandby()
{
    delete ui;
}

void SweetDisplayStandby::exit()
{
    QCoreApplication::quit();
}

void SweetDisplayStandby::onDimTimeChange(int value)
{
    settings->setValue("dimTime", value);
    displayDimTime = value;
}

void SweetDisplayStandby::onTurnOffTimeChange(int value)
{
    settings->setValue("turnOffTime", value);
    displayOffTime = value;
}

void SweetDisplayStandby::onNormalBrightnessLevelChange(int value)
{
    settings->setValue("normalBrightnessLevel", value);
    normalBrightnessLevel = value;
}

void SweetDisplayStandby::onDimmedBrightnessLevelChange(int value)
{
    settings->setValue("dimmedBrightnessLevel", value);
    dimmedBrightnessLevel = value;
}

void SweetDisplayStandby::onBrightnessStateChange(bool value)
{
    settings->setValue("enableBrighnessManagement", value);
    enableBrighnessManagement = value;
    ui->brightnessGroupBox->setDisabled(!value);
    ui->dimTimeInput->setDisabled(!value);
}


void SweetDisplayStandby::setDisplayTurnedOff(bool state)
{
    int i;
    size_t cnt;
    cnt = MonHandles.size();
    for (i = 0; i < cnt; i++)
    {
        SetVCPFeature(MonHandles[i], 0xD6, state ? POWER_OFF : POWER_ON);
    }

    displayWasTurnedOff = state;

    if (state)
    {
        ui->statusBar->showMessage("Display is turned off");
    }
    else
    {
        ui->statusBar->showMessage("Display is active");
    }
}

void SweetDisplayStandby::setDisplayDimmed(bool state)
{
    int i;
    size_t cnt;
    cnt = MonHandles.size();
    for (i = 0; i < cnt; i++)
    {
        SetMonitorBrightness(MonHandles[i], state ? dimmedBrightnessLevel : normalBrightnessLevel);
    }
    displayWasDimmed = state;
    if (state)
    {
        ui->statusBar->showMessage("Display is dimmed");
    }
    else
    {
        ui->statusBar->showMessage("Display is active");
    }
}

void SweetDisplayStandby::timerTick()
{
    LASTINPUTINFO LastInput = {};
    LastInput.cbSize = sizeof(LastInput);
    GetLastInputInfo(&LastInput);
    int idleTime = (GetTickCount() - LastInput.dwTime) / 1000;

    if (idleTime>displayOffTime || forceDisplayTurnOff)
    {
        if (!displayWasTurnedOff)
        {
            EXECUTION_STATE cap;
            NTSTATUS status;
            status = CallNtPowerInformation(SystemExecutionState, NULL, 0, &cap, sizeof(cap));
            if (!(cap & ES_DISPLAY_REQUIRED))
            {
                addToQueue("turnOff");
                displayWasTurnedOff = true;
                //lastInputTime = LastInput.dwTime;
            }
        }
        forceDisplayTurnOff = false;
    }
    else if (idleTime>displayDimTime && enableBrighnessManagement)
    {
        if (!displayWasDimmed)
        {
            EXECUTION_STATE cap;
            NTSTATUS status;
            status = CallNtPowerInformation(SystemExecutionState, NULL, 0, &cap, sizeof(cap));
            if (!(cap & ES_DISPLAY_REQUIRED))
            {
                addToQueue("dim");
                displayWasDimmed = true;
            }
        }
    }
    else if (LastInput.dwTime > lastInputTime)
    {
        if (displayWasTurnedOff)
        {
            addToQueue("turnOn");
            displayWasTurnedOff = false;
        }
        if (displayWasDimmed)
        {
            addToQueue("illuminate");
            displayWasDimmed = false;
        }
        lastInputTime = LastInput.dwTime + 1000; //Give async function more time
    }
}



void SweetDisplayStandby::createTrayIcon()
{
    minimizeAction = new QAction(tr("Mi&nimize"), this);
    connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));

    restoreAction = new QAction(tr("&Restore"), this);
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, SIGNAL(triggered()), this, SLOT(exit()));

    trayIconMenu = new QMenu(this);

    trayIconMenu->addAction(minimizeAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);


    trayIcon = new QSystemTrayIcon( this );
    trayIcon->setIcon(appIcon);
    trayIcon->show();
    trayIcon->setContextMenu(trayIconMenu);
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(onTrayClick(QSystemTrayIcon::ActivationReason)) );
}


void SweetDisplayStandby::changeEvent(QEvent *e)
{
    if(e->type() == QEvent::WindowStateChange)
    {
        if(isMinimized())
        {
            // MINIMIZED
            hide();
        }
        else
        {
            // NORMAL/MAXIMIZED ETC
        }
    }
    e->accept();
}

void SweetDisplayStandby::closeEvent(QCloseEvent * e)
{
    hide();
    e->ignore();
}

void SweetDisplayStandby::onTrayClick(QSystemTrayIcon::ActivationReason reason)
{

    if (reason==QSystemTrayIcon::DoubleClick)
    {
        setWindowState( (windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
        show();
        activateWindow(); // for Windows
    }
}

void SweetDisplayStandby::hideCompletely()
{
    hide();
    trayIcon->hide();
    qDebug() << QTime::currentTime().toString() <<"trying to hide";
}

void SweetDisplayStandby::restore()
{
    setWindowState( (windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    show();
    activateWindow(); // for Windows
    trayIcon->show();
}

//void SweetDisplayStandby::addMonitorToList(PHYSICAL_MONITOR monitor )
//{
//    QString itemText = QString::fromWCharArray(monitor.szPhysicalMonitorDescription);
//    QListWidgetItem *displayItem = new QListWidgetItem(itemText, ui->displaysList);
//    displayItem->setFlags(displayItem->flags() | Qt::ItemIsUserCheckable);
//    displayItem->setCheckState(Qt::Unchecked);
//}

bool SweetDisplayStandby::nativeEvent(const QByteArray & eventType, void * message, long *result)
{
    Q_UNUSED(result);
      Q_UNUSED(eventType);

      MSG *msg = static_cast<MSG*>(message);
    if (msg->message == WM_HOTKEY) // Была нажата одна из горячих клавиш
    {

        if (msg->wParam == turnOffHotKeyId)// Идентификатор хоткея, который мы указали при
        {                           // регистрации (не код клавиши, это вам не WM_KEYUP!)

            if (!forceDisplayTurnOff)
            {
                forceDisplayTurnOff = true;
            }
            return true;
        }
        else if (msg->wParam == restoreHotKeyId)
        {
            restore();
        }
    }
    else if (msg->message == WM_POWERBROADCAST)
    {
        if ( msg->wParam == PBT_APMSUSPEND )
        {
            addToQueue("turnOff", true);
        }
//        else if ( msg->wParam == PBT_APMRESUMESUSPEND )
//        {
//            addToQueue("turnOn");
//        }
    }
    else if (msg->message == WM_ENDSESSION)
    {
        addToQueue("turnOff", true);
    }
    else if (msg->message == WM_DEVICECHANGE)
    {
        //DEV_BROADCAST_DEVICEINTERFACE* info = (DEV_BROADCAST_DEVICEINTERFACE*) msg->lParam;

        switch(msg->wParam)
        {
            case DBT_DEVICEARRIVAL:
            case DBT_DEVICEREMOVECOMPLETE:
                EnumDisplayMonitors(0, 0, EnumProc, 0);
                qDebug() << QTime::currentTime().toString() <<"Monitor was (dis)connected";
                break;
        }
    }
    return false;
}

QList<int> SweetDisplayStandby::getVirtualKey(QKeySequence keySequence)
{
    QList<int> keys;
    if ( !keySequence.toString().size() )
    {
        return keys;
    }

    QString sequenceString = keySequence.toString();
    QRegExp rx("\\+");
    QStringList query = sequenceString.split(rx);
    uint modifiers=0;
    int i = 0, vk =-1;
    for (; i <query.length()-1; i++ )
    {
        if( query[i] == "Ctrl" )
        {
            modifiers |= MOD_CONTROL;
        }
        else if( query[i] == "Alt" )
        {
            modifiers |= MOD_ALT;
        }
        else if( query[i] == "Shift" )
        {
            modifiers |= MOD_SHIFT;
        }
        else if( query[i] == "Meta" )
        {
            modifiers |= MOD_WIN;
        }
    }
    QString lastKey = query[i];


    QRegExp frx ( "^F(\\d+)$"); // F group keys
    QRegExp drx ( "^(\\d)$"); //digits
    QRegExp lrx ( "^([A-Z])$"); //capital letters
    frx.indexIn(lastKey);
    drx.indexIn(lastKey);
    lrx.indexIn(lastKey);
    if (frx.capturedTexts()[1].length())
    {
        vk += VK_F1 + frx.capturedTexts()[1].toInt();
    }
    else if (drx.capturedTexts()[1].length())
    {
        QChar c = drx.capturedTexts()[1][0];
        vk = c.toLatin1();
    }
    else if (lrx.capturedTexts()[1].length())
    {
        QChar c = lrx.capturedTexts()[1][0];
        vk = c.toLatin1();
    }
    keys.append(modifiers);
    keys.append(vk);
    return keys;
}

void SweetDisplayStandby::updateTurnOffSequence(QKeySequence keySequence)
{
    settings->setValue("turnOffSequence", keySequence.toString());
    QList<int> keys = getVirtualKey(keySequence);


    UnregisterHotKey((HWND)SweetDisplayStandby::winId(), turnOffHotKeyId);
    RegisterHotKey((HWND)SweetDisplayStandby::winId(), turnOffHotKeyId, keys[0], keys[1]);
}

void SweetDisplayStandby::updateRestoreSequence(QKeySequence keySequence)
{
    settings->setValue("restoreSequence", keySequence.toString());
    QList<int> keys = getVirtualKey(keySequence);


    UnregisterHotKey((HWND)SweetDisplayStandby::winId(), restoreHotKeyId);
    RegisterHotKey((HWND)SweetDisplayStandby::winId(), restoreHotKeyId, keys[0], keys[1]);
}

void SweetDisplayStandby::queueTimerTick()
{
    if (queue.size())
    {
        QString action = queue[0];
        queue.removeFirst();
        if (action=="dim")
        {
            setDisplayDimmed(true);
        }
        else if (action=="illuminate")
        {
            setDisplayDimmed(false);
        }
        else if (action=="turnOff")
        {
            setDisplayTurnedOff(true);
        }
        else if (action=="turnOn")
        {
            setDisplayTurnedOff(false);
        }
        qDebug() << QTime::currentTime().toString() <<"Removed action:"<<action;
        qDebug() << QTime::currentTime().toString() <<queue;
    }
}

void SweetDisplayStandby::addToQueue(QString action)
{
    queue.append(action);

    qDebug() << QTime::currentTime().toString() <<"Added action:"<<action;
    qDebug() << QTime::currentTime().toString() <<queue;
}

void SweetDisplayStandby::addToQueue(QString action, bool immediately)
{
    if (immediately)
    {
        queue.clear();

    }
    queue.append(action);

    qDebug() << QTime::currentTime().toString() <<"Added immediately action:"<<action;
    qDebug() << QTime::currentTime().toString() <<queue;
    if (immediately)
    {
        queueTimerTick();
    }
}

void SweetDisplayStandby::aboutPopup()
{
    QString appName = ("SweetDisplayStandby v");
    QString appVersion = QString::number( VERSION );
    appName.append(appVersion);
    appName.append("\n\n");
    appName.append("https://github.com/Vavooon/SweetDisplayStandby");

    QMessageBox::about( this, "About", appName );
}

void SweetDisplayStandby::openSite()
{
    QDesktopServices::openUrl(QUrl("https://github.com/Vavooon/SweetDisplayStandby"));
}
