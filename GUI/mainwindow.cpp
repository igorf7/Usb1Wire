#include "mainwindow.h"
#include "ui_mainwindow.h"

/**
 * @brief MainWindow Class Constructor
 * @param parent_object
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(QApplication::applicationName());

    /* Create USB Custom HID device object */
    hidDevice = new CustomHid(0x0483, 0xdf11);

    /* Connecting signals to slots */
    connect(hidDevice, &CustomHid::showStatusBar,
            this, &MainWindow::onShowStatusBar);
    connect(hidDevice, &CustomHid::deviceConnected,
            this, &MainWindow::onUsbConnected);
    connect(hidDevice, &CustomHid::deviceDisconnected,
            this, &MainWindow::onUsbDisconnected);
    connect(ui->connectPushButton, SIGNAL(clicked()),
            this, SLOT(onConnectButtonClicked()));
    connect(ui->searchPushButton, SIGNAL(clicked()),
            this, SLOT(onSearchButtonClicked()));
    connect(ui->deviceComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onDeviceComboBoxChanged(int)));
    connect(ui->startPollingButton, SIGNAL(clicked()),
            this, SLOT(onStartButtonClicked()));
    connect(ui->clearPushButton, SIGNAL(clicked()),
            this, SLOT(onClearButtonClicked()));

    /* Activate status bar */
    QFont font;
    font.setItalic(true);
    statusBar()->setFont(font);

    this->onStart();
}

/**
 * @brief MainWindow Class Destructor
 */
MainWindow::~MainWindow()
{
    if (isConnected) {
        this->onConnectButtonClicked();
    }

    delete hidDevice;
    delete ui;
}

/**
 * @brief intToByteArray
 * @param value
 * @return
 */
template<typename Type>
QByteArray intToByteArray(Type value)
{
    QByteArray array;

    while (array.size() != sizeof(value))
    {
        array.append((quint8)value);
        value >>= 8;
    }

    return array;
}

/**
 * @brief MainWindow::Connection::onStart
 */
void MainWindow::onStart()
{
    this->onConnectButtonClicked();
}

/**
 * @brief onShowStatusBar
 * @param str
 * @param timeout
 */
void MainWindow::onShowStatusBar(const QString &str, int timeout)
{
    statusBar()->showMessage(str, timeout);
}

/**
 * @brief MainWindow::onConnectButtonClicked
 */
void MainWindow::onConnectButtonClicked()
{
    isOwSearchDone = false;

    if (!isConnected) {
        hidDevice->Connect();
    }
    else {
        hidDevice->Disconnect();
        if (usbPollingEvent != 0) {
            killTimer(usbPollingEvent);
            usbPollingEvent = 0;
        }
        if (owPollingEvent != 0) {
            killTimer(owPollingEvent);
            owPollingEvent = 0;
        }
    }
}

/**
 * @brief MainWindow::onUsbConnected
 */
void MainWindow::onUsbConnected()
{
    isConnected = true;
    ui->connectPushButton->setIcon(QIcon(":/images/sw_on.png"));
    statusBar()->showMessage(tr("USB Connected"));

    if (!isUsbPollRunning) {
        isUsbPollRunning = true;
        usbPollingEvent = startTimer(10);
    }
}

/**
 * @brief MainWindow::onUsbDisconnected
 */
void MainWindow::onUsbDisconnected()
{
    ui->connectPushButton->setIcon(QIcon(":/images/sw_off.png"));
    isConnected = false;
    isUsbPollRunning = false;
    isOwPollRunning = false;
    statusBar()->showMessage(tr("USB Disconnected"));
    selDevices.clear();
    owDeviceAddressList.clear();
    ui->deviceComboBox->clear();
    ui->startPollingButton->setEnabled(false);
}

/**
 * @brief MainWindow::onSearchButtonClicked
 */
void MainWindow::onSearchButtonClicked()
{
    if (!isConnected) return;

    isOwSearchDone = false;
    owDeviceAddressList.clear();
    this->usbSend(eOwSearchRom, nullptr);
}

/**
 * @brief MainWindow::onStartButtonClicked
 */
void MainWindow::onStartButtonClicked()
{
    if (!isConnected) return;

    if (!isOwPollRunning) {
        isOwPollRunning = true;
        this->startPolling();
        ui->startPollingButton->setText("Stop");
    }
    else {
        isOwPollRunning = false;
        if (owPollingEvent != 0) {
            killTimer(owPollingEvent);
            owPollingEvent = 0;
        }
        ui->startPollingButton->setText("Start");

    }
}

/**
 * @brief MainWindow::onClearButtonClicked
 */
void MainWindow::onClearButtonClicked()
{
    rxCounter = 0;
    ui->textEdit->clear();
}

/**
 * @brief MainWindow::initDeviceComboBox
 */
void MainWindow::initDeviceComboBox()
{
    ui->deviceComboBox->clear();

    QStringList ComboBoxItems;

    for (int i = 0; i < owDeviceAddressList.size(); ++i) {
        quint8 dev_family = owDeviceAddressList.at(i) & 0xFF;
        QString name = OneWire::getName(dev_family);
        if (!ComboBoxItems.contains(name)) {
            ComboBoxItems.append(name);
        }
    }
    if (!ComboBoxItems.isEmpty()) {
        ui->deviceComboBox->addItems(ComboBoxItems);
    }
}

/**
 * @brief onDeviceComboBoxChanged
 * @param index
 */
void MainWindow::onDeviceComboBoxChanged(int index)
{
    Q_UNUSED(index)

    if (!isConnected) return;

    selDevices.clear();
    owDevIndex = 0;

    quint8 dev_family = OneWire::getFamily(ui->deviceComboBox->currentText());

    for (int i = 0, j = 0; i < owDeviceAddressList.size(); ++i) {
        if (dev_family == (owDeviceAddressList.at(i) & 0xFF)) {
            selDevices.insert(owDeviceAddressList.at(i), j++);
        }
    }

    owSelDeviceCount = selDevices.size();
    statusBar()->showMessage(ui->deviceComboBox->currentText() +
                             " device found: " + QString::number(owSelDeviceCount));
}

/**
 * @brief MainWindow::timerEvent
 * @param event
 */
void MainWindow::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == owPollingEvent) {
        if (owPollingEvent != 0) {
            killTimer(owPollingEvent);
            owPollingEvent = 0;
            this->owDataRead(9);
        }
    }
    else if (event->timerId() == usbPollingEvent) {
        int len = hidDevice->Read(rxUsbBuffer, USB_BUFF_SIZE);
        if (len < 0) { // Lost connection
            statusBar()->showMessage(tr("USB Receive Error"));
            return;
        }
        else if (len != 0) {
            this->handleReceivedPacket();
        }
    }
}

/**
 * @brief MainWindow::usbSend
 * @param opcode
 * @param tx_data
 * @param tx_data_len
 */
void MainWindow::usbSend(TOpcode opcode, const QByteArray &tx_data)
{
    if (!isConnected) return;

    quint8 tx_size = 0;

    if (tx_data != nullptr) {
        tx_size = (quint8)tx_data.size();
    }

    txUsbBuffer[0] = eRepId_3;
    txUsbBuffer[1] = opcode;
    txUsbBuffer[2] = tx_size;

    for (auto i = 0; i < tx_size; i++) {
        txUsbBuffer[i+3] = (quint8)tx_data.at(i);
    }

    int len = hidDevice->Write(txUsbBuffer, USB_BUFF_SIZE);

    if (len < 0) { // Lost connection
        statusBar()->showMessage(tr("USB Transmit Error"));
    }
}

/**
 * @brief MainWindow::startPolling
 */
void MainWindow::startPolling()
{
    QByteArray tx_data;
    quint8 rom_cmd = (owSelDeviceCount > 1) ? OW_SKIPROM_CMD : OW_MATCHROM_CMD;

    // Init Reset/Presence procedure
    this->usbSend(eOwBusReset, nullptr);
    // Send MATCHROM or SKIPROM cmd
    tx_data.append(rom_cmd);
    this->usbSend(eOwBusWrite, tx_data);
    tx_data.clear();
    if (owSelDeviceCount == 1) { // this for DD84
        // Send address
        tx_data.append(intToByteArray(selDevices.key(owDevIndex)));
        this->usbSend(eOwBusWrite, tx_data);
        tx_data.clear();
    }
    // Send CONVERT cmd
    tx_data.append(OW_CONVERT_CMD);
    this->usbSend(eOwBusWrite, tx_data);
    owPollingEvent = startTimer(owPollingTime);
}

/**
 * @brief MainWindow::owDataRead
 * @param read_data_len
 */
void MainWindow::owDataRead(quint16 read_data_len)
{
    QByteArray tx_data;

    // Init Reset/Presence procedure
    this->usbSend(eOwBusReset, nullptr);
    // Send MATCHROM cmd
    tx_data.append(OW_MATCHROM_CMD);
    this->usbSend(eOwBusWrite, tx_data);
    tx_data.clear();
    // Send address
    tx_data.append(intToByteArray(selDevices.key(owDevIndex)));
    this->usbSend(eOwBusWrite, tx_data);
    tx_data.clear();
    // Send READ cmd
    tx_data.append(OW_READ_CMD);
    this->usbSend(eOwBusWrite, tx_data);
    tx_data.clear();
    // Init data read (read_data_len bytes)
    tx_data.append(intToByteArray(read_data_len));
    this->usbSend(eOwBusRead, tx_data);
}

/**
 * @brief MainWindow::handleReceivedPacket
 */
void MainWindow::handleReceivedPacket()
{
    TAppLayerPacket *rx_packet = (TAppLayerPacket*)rxUsbBuffer;
    quint64 dev_addr = 0;
    float ftemper = 0.0f;
    int dev_cnt = 0;
    quint16 utemper = 0, sign = 0;
    quint8 dev_family = 0;

    if (rx_packet->rep_id == eRepId_4)
    {
        switch (rx_packet->opcode)
        {
        case eOwBusReset:
            break;

        case eOwSearchRom:
            dev_addr = *((quint64*)rx_packet->data);
            if (dev_addr != 0) {
                owDeviceAddressList.append(dev_addr);
                this->usbSend(eOwSearchRom, nullptr);
            }
            else {
                isOwSearchDone = true;
                this->initDeviceComboBox();
                dev_cnt = owDeviceAddressList.size();
                if (dev_cnt > 0)
                    ui->startPollingButton->setEnabled(true);
                else
                    ui->startPollingButton->setEnabled(false);

                statusBar()->showMessage(tr("Total 1-Wire devices found: ") +
                                         QString::number(dev_cnt));
            }
            break;

        case eOwBusRead:
            dev_family = OneWire::getFamily(ui->deviceComboBox->currentText());
            ui->textEdit->append("Packet " + QString::number(++rxCounter));
            ui->textEdit->append("Device " + QString::number(owDevIndex + 1));
            ui->textEdit->append("Address: " +
                                 QString::number(selDevices.key(owDevIndex), 16).toUpper());

            switch (dev_family) {
                case 0x28: // DS18B20
                    if (rx_packet->data[8] != OneWire::calcCrc8(rx_packet->data, 8)) {
                        statusBar()->showMessage(tr("!!! CRC8 Error !!!"));
                    }
                    else {
                        utemper = (rx_packet->data[1] << 8) | rx_packet->data[0];
                        sign = utemper & DS18B20_SIGN_MASK;
                        if (sign != 0) utemper = (0xFFFF - utemper + 1);
                        ftemper = (float)utemper * 0.0625f;
                        ui->textEdit->append("Temperature: " + QString::number(ftemper, 'f', 1) + " °C");
                        ui->textEdit->append("Alarm High: " + QString::number((qint8)rx_packet->data[2]));
                        ui->textEdit->append("Alarm Low: " + QString::number((qint8)rx_packet->data[3]));
                        ui->textEdit->append("Resolution code: " + QString::number((qint8)rx_packet->data[4]));
                    }
                    break;
                default: // other devices
                    ui->textEdit->append(OneWire::getDescription(dev_family));
                    break;
            }

            ui->textEdit->append("");

            if (owDevIndex < (owSelDeviceCount - 1))
                owDevIndex++;
            else
                owDevIndex = 0;

            if (isOwPollRunning)
                this->startPolling();
            break;

        case eOwBusWrite:
            break;

        case eUsbHeartbeat:
            break;

        default:
            break;
        }
    }
}
