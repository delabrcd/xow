#include "UsbController.h"
#include "../utils/log.h"
#include <memory>

#define USB_MAX_BULK_TRANSFER_SIZE 32
#define bmREQ_HID_OUT \
    USB_SETUP_HOST_TO_DEVICE | USB_SETUP_TYPE_CLASS | USB_SETUP_RECIPIENT_INTERFACE
#define HID_REQUEST_SET_REPORT 0x09

UsbController::UsbController(std::unique_ptr<UsbDevice> usbDevice)
    : m_UsbDevice(std::move(usbDevice)), m_StopThreads(false) {
    GipDevice::SendPacket sendPacket =
        std::bind(&UsbController::sendClientPacket, this, std::placeholders::_1);
    m_Controller = std::make_unique<Controller>(sendPacket);
    UsbDevice::ControlPacket pkt;
    pkt.type    = 0x40;
    pkt.request = 0xa9;
    pkt.value   = 0xa30c;
    pkt.index   = 0x4423;
    pkt.length  = 0;
    controlWrite(pkt, true);

    UsbDevice::ControlPacket pkt;
    pkt.type    = 0x40;
    pkt.request = 0xa9;
    pkt.value   = 0x2344;
    pkt.index   = 0x7f03;
    pkt.length  = 0;
    controlWrite(pkt, true);

    UsbDevice::ControlPacket pkt;
    pkt.type    = 0x40;
    pkt.request = 0xa9;
    pkt.value   = 0x5839;
    pkt.index   = 0x6832;
    pkt.length  = 0;
    controlWrite(pkt, true);

    uint8_t code[2] = {0x01, 0x02};

    UsbDevice::ControlPacket pkt;
    pkt.type    = 0xc0;
    pkt.request = 0xa1;
    pkt.value   = 0x0000;
    pkt.index   = 0xe416;
    pkt.length  = 2;
    pkt.data    = code;
    controlWrite(pkt, true);

    UsbDevice::ControlPacket pkt;
    pkt.type    = 0x40;
    pkt.request = 0xa1;
    pkt.value   = 0x0000;
    pkt.index   = 0xe416;
    pkt.length  = 2;
    pkt.data    = code;
    controlWrite(pkt, true);

    UsbDevice::ControlPacket pkt;
    pkt.type    = 0xc0;
    pkt.request = 0xa1;
    pkt.value   = 0x0000;
    pkt.index   = 0xe416;
    pkt.length  = 2;
    pkt.data    = code;
    controlWrite(pkt, true);

    m_Threads.emplace_back(&UsbController::readBulkPackets, this,
                           m_UsbDevice->getEndpointInAddress());
}

UsbController::~UsbController() {
    m_StopThreads = true;
    m_Threads.back().join();
}

bool UsbController::sendClientPacket(Bytes &data) {
    std::cout << "OUT";
    data.print();
    return m_UsbDevice->bulkWrite(m_UsbDevice->getEndpointOutAddress(), data);
}

void UsbController::handleBulkData(const Bytes &data) {
    std::cout << "IN";

    data.print();
    m_Controller->handlePacket(data);
}

void UsbController::controlWrite(UsbDevice::ControlPacket packet, bool write) {
    Log::debug("SENT CONTROL PACKET");
    return m_UsbDevice->controlTransfer(packet, write);
}

void UsbController::readBulkPackets(uint8_t endpoint) {
    FixedBytes<USB_MAX_BULK_TRANSFER_SIZE> buffer;

    while (!m_StopThreads) {
        int transferred = m_UsbDevice->bulkRead(m_UsbDevice->getEndpointInAddress(), buffer);

        // Bulk read failed
        if (transferred < 0) {
            Log::error("OOPS, Bulk Read Failed");
            break;
        }

        if (transferred > 0) {
            Bytes data = buffer.toBytes(transferred);
            handleBulkData(data);
        }
    }
}
