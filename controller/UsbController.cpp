#include "UsbController.h"
#include "../utils/log.h"
#include <memory>

#define USB_MAX_BULK_TRANSFER_SIZE 64

UsbController::UsbController(std::unique_ptr<UsbDevice> usbDevice)
    : m_UsbDevice(std::move(usbDevice)), m_StopThreads(false) {
    GipDevice::SendPacket sendPacket =
        std::bind(&UsbController::sendClientPacket, this, std::placeholders::_1);
    m_Controller = std::make_unique<Controller>(sendPacket);

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