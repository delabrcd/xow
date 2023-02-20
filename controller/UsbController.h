#include <stdint.h>
#include <vector>
#include <thread>
#include <atomic>
#include "usb.h"
#include "bytes.h"
#include "controller.h"

class UsbController {
public:
    UsbController(std::unique_ptr<UsbDevice> usbDevice);
    virtual ~UsbController();
    bool sendClientPacket(Bytes &data);

    void readBulkPackets(uint8_t endpoint);

    void handleBulkData(const Bytes &data);

private:
    std::vector<std::thread>    m_Threads;
    std::unique_ptr<UsbDevice>  m_UsbDevice;
    std::atomic<bool>           m_StopThreads;
    std::unique_ptr<Controller> m_Controller;
};