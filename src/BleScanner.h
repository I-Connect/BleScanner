#pragma once

#include "Arduino.h"
#include <string>
#include <NimBLEDevice.h>
#include "BleInterfaces.h"

namespace BleScanner {

class Scanner : public Publisher, BLEAdvertisedDeviceCallbacks {
  public:
    Scanner(int reservedSubscribers = 10);
    ~Scanner() = default;

    void initialize(const std::string& deviceName = "blescanner", const bool wantDuplicates = false, const uint16_t interval = 23, const uint16_t window = 23);
    void update();
    void setScanDuration(const uint32_t value);

    void subscribe(Subscriber* subscriber) override;
    void unsubscribe(Subscriber* subscriber) override;

    void onResult(NimBLEAdvertisedDevice* advertisedDevice) override;

  private:
    uint32_t scanDuration = 3;
    BLEScan* bleScan = nullptr;
    std::vector<Subscriber*> subscribers;
};

} // namespace BleScanner

