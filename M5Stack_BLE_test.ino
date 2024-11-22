#include <M5Stack.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEClient.h>
#include <BLEUtils.h>

// UUID Definitions
#define UUID_SERVICE "6E400001-B5A3-F393-E0A9-E50E24DCCA9D"      // 手表服务 UUID
#define UUID_WRITE_CHAR "6E400002-B5A3-F393-E0A9-E50E24DCCA9D"  // 写入特征值 UUID
#define UUID_NOTIFY_CHAR "6E400003-B5A3-F393-E0A9-E50E24DCCA9D" // 通知特征值 UUID

// Global Variables
BLEClient* pBLEClient = nullptr;
BLERemoteCharacteristic* writeChar = nullptr;
BLERemoteCharacteristic* notifyChar = nullptr;
bool isDeviceConnected = false; // 连接状态

std::vector<BLEAdvertisedDevice> devicesList; // 存储发现的设备列表
int currentPage = 0; // 当前显示的设备页索引
int selectedDeviceIndex = -1; // 当前选择的设备索引

// 蓝牙通知回调
void notifyCallback(BLERemoteCharacteristic* characteristic, uint8_t* data, size_t length, bool isNotify) {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println("Raw data received:");

    for (size_t i = 0; i < length; i++) {
        M5.Lcd.printf("0x%02X ", data[i]);
    }
    M5.Lcd.println();

    // 示例解析：假设心率数据在第三个字节
    if (length >= 3 && data[0] == 0x16) { // 数据类型 0x16 表示心率
        int heartRate = data[2];
        if (heartRate >= 30 && heartRate <= 200) {
            M5.Lcd.printf("Heart Rate: %d bpm\n", heartRate);
        } else {
            M5.Lcd.println("Invalid heart rate data.");
        }
    } else {
        M5.Lcd.println("Unexpected data format.");
    }
}

// 启用心率功能
void enableHeartRate() {
    if (writeChar) {
        uint8_t command[] = {0x70, 0x01}; // 示例指令：请求心率数据
        writeChar->writeValue(command, sizeof(command), true);
        M5.Lcd.println("Heart rate monitoring enabled.");
    } else {
        M5.Lcd.println("Write characteristic not available.");
    }
}

// 蓝牙连接状态更新回调
void updateConnectionState(bool connected) {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    if (connected) {
        M5.Lcd.println("Device connected!");
        enableHeartRate();
    } else {
        M5.Lcd.println("Device disconnected!");
    }
    isDeviceConnected = connected;
}

// 连接设备
void connectToDevice(BLEAdvertisedDevice* advertisedDevice) {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println("Connecting to device...");

    pBLEClient = BLEDevice::createClient();
    if (pBLEClient->connect(advertisedDevice)) {
        M5.Lcd.println("Connected!");

        BLERemoteService* service = pBLEClient->getService(UUID_SERVICE);
        if (service) {
            writeChar = service->getCharacteristic(UUID_WRITE_CHAR);
            notifyChar = service->getCharacteristic(UUID_NOTIFY_CHAR);

            if (notifyChar && notifyChar->canNotify()) {
                notifyChar->registerForNotify(notifyCallback);
                M5.Lcd.println("Notifications enabled.");
            }
            updateConnectionState(true); // 更新连接状态
        } else {
            M5.Lcd.println("Service not found.");
        }
    } else {
        M5.Lcd.println("Connection failed.");
        updateConnectionState(false); // 更新连接状态
    }
}

// 其余功能与按键逻辑保持不变
