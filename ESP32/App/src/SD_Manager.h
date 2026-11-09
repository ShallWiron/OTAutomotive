#ifndef SD_MANAGER_H
#define SD_MANAGER_H

#include <Arduino.h>
#include <SD.h>
#include <vector>

struct SdEntry {
    String name;
    size_t size;
};

class SD_Manager {
public:
    explicit SD_Manager(uint8_t csPin = 15);

    bool begin();
    bool isReady();
    String getCardType() const;
    std::vector<SdEntry> listBinFiles(const String& dirPath = "/");
    String getStatusJson();

    File openRead(const String& path);
    File openWrite(const String& path);
    bool remove(const String& path);
    bool exists(const String& path);

private:
    uint8_t _csPin;
    bool _isInitialized;

    void scanDirectory(File dir, std::vector<SdEntry>& list);
};

#endif // SD_MANAGER_H
