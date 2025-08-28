#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "usertypes.h"
#include "customhid.h"
#include "onewire.h"
#include <QMainWindow>
#include <QLayout>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QMap>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    int usbPollingEvent = 0;
    int owMeasureTimeEvent = 0;
    void timerEvent(QTimerEvent *event);

public slots:
    void onShowStatusBar(const QString &str, int timeout);
    void onUsbConnected();
    void onUsbDisconnected();

private slots:
    void onStart();
    void onConnectButtonClicked();
    void onSearchButtonClicked();
    void onStartButtonClicked();
    void onDeviceComboBoxChanged(int index);
    void onClearButtonClicked();

private:
    Ui::MainWindow *ui;

    static const size_t USB_BUFF_SIZE = 65;
    static const int USB_POLLING_PERIOD = 10;

    CustomHid *hidDevice = nullptr;
    QList<quint64> owDeviceAddressList;
    QHash<quint64, int> selDevices;

    int owMeasureTime = 750; // ms
    int owTotalDeviceCount = 0;
    int owSelDeviceCount = 0;
    int owDevIndex = 0;

    quint32 rxCounter = 0;

    bool isConnected = false;
    bool isUsbPollRunning = false;
    bool isOwSearchDone = false;
    bool isMeasureRunning = false;

    quint8 rxUsbBuffer[USB_BUFF_SIZE];
    quint8 txUsbBuffer[USB_BUFF_SIZE];

    void startMeasure();
    void owDataRead(quint16 read_data_len);
    void usbSend(TOpcode opcode, const QByteArray &tx_data);
    void handleReceivedPacket();
    void initDeviceComboBox();
};
#endif // MAINWINDOW_H
