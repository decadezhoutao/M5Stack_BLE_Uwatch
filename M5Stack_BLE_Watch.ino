#include <M5Stack.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

// BLE Constants
#define SCAN_TIME 5          // BLE scan duration in seconds
#define ITEMS_PER_PAGE 2     // Devices displayed per page

// Global Variables
BLEScan* pBLEScan;               // BLE scan instance
BLEScanResults lastScanResults;  // Store results of the last scan
int currentPage = 0;             // Current page for pagination
bool isScanning = false;         // Indicates if scanning is ongoing
bool scanComplete = false;       // Indicates if a scan has completed
bool isStaticMode = false;       // Static mode toggle

// BLE Device Scan Callback (optional)
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        // Optional: Handle each device when found
    }
};

// Display devices for the current page
void displayDevicesForPage() {
    M5.Lcd.fillScreen(BLACK); // Clear the screen
    M5.Lcd.setCursor(0, 0);

    int deviceCount = lastScanResults.getCount(); // Total devices
    int totalPages = (deviceCount + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE; // Calculate total pages

    // Show current page information
    M5.Lcd.printf("Devices: %d (Page %d/%d)\n", deviceCount, currentPage + 1, totalPages);

    // If no devices found, display a message
    if (deviceCount == 0) {
        M5.Lcd.println("No devices found.");
        return;
    }

    // Get the range of devices to display on this page
    int startIdx = currentPage * ITEMS_PER_PAGE;
    int endIdx = min(startIdx + ITEMS_PER_PAGE, deviceCount);

    for (int i = startIdx; i < endIdx; ++i) {
        BLEAdvertisedDevice device = lastScanResults.getDevice(i);
        M5.Lcd.printf("Device %d:\n", i + 1);
        M5.Lcd.println(device.toString().c_str());
    }

    // Show navigation instructions
    M5.Lcd.setCursor(0, 200);
    M5.Lcd.println("A: Prev | B: Next | C: Rescan");
}

// Handle button inputs
void handleButtons() {
    M5.update(); // Update button states

    // Button A: Go to the previous page
    if (M5.BtnA.wasPressed() && currentPage > 0) {
        currentPage--;
        displayDevicesForPage();
    }

    // Button B: Go to the next page
    if (M5.BtnB.wasPressed()) {
        int deviceCount = lastScanResults.getCount();
        int totalPages = (deviceCount + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;
        if (currentPage < totalPages - 1) {
            currentPage++;
            displayDevicesForPage();
        }
    }

    // Button C: Rescan
    if (M5.BtnC.wasPressed()) {
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.println("Starting new scan...");
        isStaticMode = false;
        isScanning = true;
        scanComplete = false;
    }
}

// Start BLE scanning and display results
void startScan() {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println("Scanning for BLE devices...");

    // Perform BLE scan
    lastScanResults = pBLEScan->start(SCAN_TIME, false);
    int deviceCount = lastScanResults.getCount();
    M5.Lcd.printf("Devices found: %d\n", deviceCount);

    // Reset to the first page and display results
    currentPage = 0;
    displayDevicesForPage();

    // Mark the scan as complete
    scanComplete = true;
    isScanning = false;
}

void setup() {
    // Initialize M5Stack
    M5.begin();
    M5.Lcd.setTextSize(2);
    M5.Lcd.println("Initializing BLE...");

    // Initialize BLE
    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true); // Enable active scan for more data
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);

    // Indicate ready for scanning
    scanComplete = false;
    isScanning = false;
}

void loop() {
    // Handle button inputs
    handleButtons();

    // If static mode is active, skip scanning
    if (isStaticMode || scanComplete) {
        delay(100);
        return;
    }

    // Start scanning if requested
    if (isScanning) {
        startScan();
    }

    delay(100); // Reduce power consumption
}
