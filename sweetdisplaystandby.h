#ifndef SWEETDISPLAYSTANDBY_H
#define SWEETDISPLAYSTANDBY_H

#pragma once

#include <QMainWindow>
#include <QString>
#include <QTimer>
#include <QTime>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QStringListModel>
#include <QMessageBox>
#include <QDesktopServices>

#include <Windows.h>
#include "PhysicalMonitorEnumerationAPI.h"
#include "HighLevelMonitorConfigurationAPI.h"
#include <lowlevelmonitorconfigurationapi.h>
#include <powrprof.h>
#include "Dbt.h"
//#include "Ntddvdeo.h"

//Test includes, should be removed after

#include <QDebug>

#pragma comment(lib,"dxva2.lib")
#pragma comment(lib,"PowrProf.lib")
#pragma comment(lib,"User32.lib")


#define POWER_ON                    0x01
#define POWER_STANDBY               0x02
#define POWER_SUSPEND               0x03
#define POWER_OFF                   0x04
#define VERSION                     0.1

//BOOL QT_WIN_CALLBACK EnumProc(
//    HMONITOR hMonitor,
//    HDC hdcMonitor,
//    LPRECT lprcMonitor,
//    LPARAM dwData
//);


namespace Ui {
    class SweetDisplayStandby;
}

class SweetDisplayStandby : public QMainWindow
{
    Q_OBJECT

    public:
        explicit SweetDisplayStandby(QWidget *parent = 0);
//        void addMonitorToList(PHYSICAL_MONITOR Monitor );
        ~SweetDisplayStandby();

    public slots:
        void exit();
        void onDimTimeChange(int value);
        void onTurnOffTimeChange(int value);
        void onNormalBrightnessLevelChange(int value);
        void onDimmedBrightnessLevelChange(int value);
        void onBrightnessStateChange(bool value);
        void timerTick();

    private:
        Ui::SweetDisplayStandby *ui;
        void setDisplayTurnedOff(bool state);
        void setDisplayDimmed(bool state);
        void createTrayIcon();
        void changeEvent(QEvent* e);
        void closeEvent(QCloseEvent *e);
        bool nativeEvent(const QByteArray & eventType, void * message, long *result);
        QList<int> getVirtualKey(QKeySequence keySequence);
        void addToQueue(QString action);
        void addToQueue(QString action, bool immediately);
        //QSettings settings;
        int screensaverTimeout;

        int displayDimTime;
        int displayOffTime;
        int normalBrightnessLevel;
        int dimmedBrightnessLevel;
        bool enableBrighnessManagement;
        QKeySequence turnOffSequence;
        QKeySequence restoreSequence;

        bool displayWasDimmed;
        bool displayWasTurnedOff;
        bool forceDisplayTurnOff;
        DWORD lastInputTime;

        int turnOffHotKeyId;
        int restoreHotKeyId;

        QKeySequence *displayToggleSequence;
        QSystemTrayIcon *tray;
        QSettings *settings;

        QAction *minimizeAction;
        QAction *maximizeAction;
        QAction *restoreAction;
        QAction *quitAction;

        QIcon appIcon;
        QSystemTrayIcon *trayIcon;
        QMenu *trayIconMenu;

        QStringListModel *displaysModel;
        QStringList queue;

        void restore();

    private slots:
        void updateTurnOffSequence(const QKeySequence keySequence);
        void updateRestoreSequence(QKeySequence keySequence);
        void onTrayClick(QSystemTrayIcon::ActivationReason reason);
        void queueTimerTick();
        void hideCompletely();
        void aboutPopup();
        void openSite();

};

#endif // SWEETDISPLAYSTANDBY_H
