#include "customhid.h"

/**
 * @brief Class constructor
 * @param vid
 * @param pid
 */
CustomHid::CustomHid(unsigned short vid,
                     unsigned short pid) : VID(vid), PID(pid)
{
    hid_init();
}

/**
 * @brief Class destructor
 */
CustomHid::~CustomHid()
{
    this->closeHidDevice();
}

/**
 * @brief CustomHid::onConnect
 * @param product_string
 */
void CustomHid::Connect()
{
    struct hid_device_info *devs, *cur_dev;

    /* Find devices with specified VID/PID */
    devs = hid_enumerate(VID, PID);
    cur_dev = devs;

    while (cur_dev)
    {
        /* Find USB device with a given product string */
        if (QString::fromWCharArray(cur_dev->product_string) == ProductString)
        {
            break;
        }
        cur_dev = cur_dev->next;
    }

    if (!cur_dev) return;

    /* Open found device */
    deviceHandle = hid_open_path(cur_dev->path);


    if (!deviceHandle) {
        hid_exit();
        return;
    }

    hid_set_nonblocking(deviceHandle, 1);
    emit deviceConnected();
}

/**
 * @brief CustomHid::onDisconnect
 */
void CustomHid::Disconnect()
{
    this->closeHidDevice();
    emit deviceDisconnected();
}

/**
 * @brief CustomHid::Read
 * @param buff - pointer to rx data buffer
 * @param len - received data lenght
 * @return number of bytes received
 */
int CustomHid::Read(unsigned char *buff, size_t len)
{
    /* Read rx data */
    return hid_read(deviceHandle, buff, len);
}

/**
 * @brief CustomHid::getFeatureReport
 * @param buff - pointer to rx data buffer
 * @param len - received data lenght
 * @return  number of bytes received
 */
int CustomHid::getFeatureReport(unsigned char *buff, size_t len)
{
    return hid_get_feature_report(deviceHandle, buff, len);
}

/**
 * @brief CustomHid::Write
 * @param buff - pointer to tx data buffer
 * @param len - transmitted data lenght
 * @return number of bytes transmitted
 */
int CustomHid::Write(unsigned char *buff, size_t len)
{
    /* Send data to device */
    return hid_write(deviceHandle, buff, len);
}

/**
 * @brief CustomHid::sendFeatureReport
 * @param buff - pointer to tx data buffer
 * @param len - transmitted data lenght
 * @return  number of bytes transmitted
 */
int CustomHid::sendFeatureReport(unsigned char *buff, size_t len)
{
    return hid_send_feature_report(deviceHandle, buff, len);
}

/**
 * @brief CustomHid::closeHidDevice
 */
void CustomHid::closeHidDevice()
{
    /* Close the device */
    if (deviceHandle != nullptr) {
        hid_close(deviceHandle);
        deviceHandle = nullptr;
    }
    /* Finalize the hidapi library */
    hid_exit();
}
