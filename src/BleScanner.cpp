/**
 * @file BleScanner.cpp
 *
 * Created: 2022
 * License: GNU GENERAL PUBLIC LICENSE (see LICENSE)
 *
 * This library provides a BLE scanner to be used by other libraries to
 * receive advertisements from BLE devices
 *
 */

#include "BleScanner.h"
#include <NimBLEUtils.h>
#include <NimBLEScan.h>
#include <NimBLEAdvertisedDevice.h>

namespace BleScanner {

Scanner::Scanner(int reservedSubscribers) {
  subscribers.reserve(reservedSubscribers);
}

void Scanner::initialize(const std::string& deviceName, const bool wantDuplicates, const uint16_t interval, const uint16_t window) {
  if (!BLEDevice::getInitialized()) {
    if (wantDuplicates) {
      // reduce memory footprint, cache is not used anyway
      NimBLEDevice::setScanDuplicateCacheSize(10);
    }
    BLEDevice::init(deviceName);
  }
  bleScan = BLEDevice::getScan();
  bleScan->setAdvertisedDeviceCallbacks(this, wantDuplicates);
  bleScan->setInterval(interval);
  bleScan->setWindow(window);
}

void Scanner::update() {
  if (!scanningEnabled || bleScan->isScanning()) {
    return;
  }

  if (scanDuration == 0) {
    // Avoid unbridled growth of results vector
    bleScan->setMaxResults(0);
  }

  bool result = bleScan->start(scanDuration, nullptr, false);
  if (!result) {
    scanErrors++;
    if (scanErrors % 100 == 0) {
      log_w("BLE Scan error (100x)");
    }
  }
}

void Scanner::enableScanning(bool enable) {
  scanningEnabled = enable;
  if (!enable) {
    bleScan->stop();
  }
}

void Scanner::setScanDuration(const uint32_t value) {
  scanDuration = value;
}

void Scanner::addAddressToWhitelist(std::string bleAddress) {
  NimBLEAddress address(bleAddress);
  addAddressToWhitelist(address);
}

void Scanner::addAddressToWhitelist(BLEAddress bleAddress) {
  if (whitelistEnabled && NimBLEDevice::whiteListAdd(bleAddress)) {
    #ifdef DEBUG_BLE_SCANNER
    log_d("adding %s to whitelist", bleAddress.toString().c_str());
    log_d("Adresses in whitelist: %d", NimBLEDevice::getWhiteListCount());
    #endif
    bleScan->setFilterPolicy(1);
  } else {
    log_w("BLE Whitelist not enabled or add failed");
  }
}

void Scanner::removeAddressFromWhitelist(BLEAddress bleAddress) {
  NimBLEDevice::whiteListRemove(bleAddress);
  #ifdef DEBUG_BLE_SCANNER
  log_d("removing %s from whitelist", bleAddress.toString().c_str());
  log_d("Adresses in whitelist: %d", NimBLEDevice::getWhiteListCount());
  #endif
  if (whitelistEnabled && NimBLEDevice::getWhiteListCount() == 0 ) {
    bleScan->setFilterPolicy(0);
  } else {
    log_w("BLE Whitelist not enabled or remove failed");
  }
}

void Scanner::enableWhitelist(bool enable) {
  if (enable) {
    whitelistEnabled = true;
    bleScan->setFilterPolicy(1);
    #ifdef DEBUG_BLE_SCANNER
    log_d("BLE whitelist enabled");
    #endif
  } else {
    whitelistEnabled = false;
    bleScan->setFilterPolicy(0);
    #ifdef DEBUG_BLE_SCANNER
    log_d("BLE whitelist disabled");
    #endif
  }
}

void Scanner::subscribe(Subscriber* subscriber) {
  if (std::find(subscribers.begin(), subscribers.end(), subscriber) != subscribers.end()) {
    return;
  }

  subscribers.push_back(subscriber);
}

void Scanner::unsubscribe(Subscriber* subscriber) {
  auto it = std::find(subscribers.begin(), subscribers.end(), subscriber);
  if (it != subscribers.end()) {
    subscribers.erase(it);
  }
}

void Scanner::onResult(NimBLEAdvertisedDevice* advertisedDevice) {
  #ifdef DEBUG_BLE_SCANNER
  log_d("result %s", advertisedDevice->getAddress().toString().c_str());
  #endif
  for (const auto& subscriber : subscribers) {
    subscriber->onResult(advertisedDevice);
  }
}

} // namespace BleScanner
