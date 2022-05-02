#pragma once
#include <NimBLEDevice.h>

namespace BleScanner {


class Subscriber {
  public:
    virtual void onResult(NimBLEAdvertisedDevice* advertisedDevice) = 0;
};

class Publisher {
  public:
    virtual void subscribe(Subscriber* subscriber) = 0;
    virtual void unsubscribe(Subscriber* subscriber) = 0;
};

} // namespace BleScanner
