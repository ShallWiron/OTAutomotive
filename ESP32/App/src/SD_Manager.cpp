#include "SD_Manager.h"

SD_Manager::SD_Manager(uint8_t csPin)
    : _csPin(csPin), _isInitialized(false) {}

bool SD_Manager::begin() {
    _isInitialized = SD.begin(_csPin);
    return _isInitialized;
}

bool SD_Manager::isReady() {
    if (!_isInitialized || SD.cardType() == CARD_NONE) {
        SD.end();
        _isInitialized = SD.begin(_csPin);
    }
    return _isInitialized && (SD.cardType() != CARD_NONE);
}

String SD_Manager::getCardType() const {
    if (!const_cast<SD_Manager*>(this)->isReady()) return "NONE";
    uint8_t t = SD.cardType();
    if (t == CARD_MMC)  return "MMC";
    if (t == CARD_SD)   return "SDSC";
    if (t == CARD_SDHC) return "SDHC";
    return "UNKNOWN";
}

void SD_Manager::scanDirectory(File dir, std::vector<SdEntry>& list) {
    if (!dir || !dir.isDirectory()) return;

    File file = dir.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            String name = String(file.name());
            if (!name.startsWith("/")) name = "/" + name;

            String lower = name;
            lower.toLowerCase();
            if (lower.endsWith(".bin")) {
                list.push_back({ name, file.size() });
            }
        }
        file = dir.openNextFile();
    }
}

std::vector<SdEntry> SD_Manager::listBinFiles(const String& dirPath) {
    std::vector<SdEntry> list;
    if (!isReady()) return list;

    File root = SD.open(dirPath);
    scanDirectory(root, list);
    root.close();
    return list;
}

String SD_Manager::getStatusJson() {
    if (!isReady()) {
        return "{\"mounted\":false,\"card_type\":\"NONE\",\"files\":[]}";
    }

    std::vector<SdEntry> files = listBinFiles("/");
    String json = "{\"mounted\":true,\"card_type\":\"" + getCardType() + "\",\"files\":[";

    for (size_t i = 0; i < files.size(); i++) {
        if (i > 0) json += ",";
        json += "{\"name\":\"" + files[i].name + "\",\"size\":" + String(files[i].size) + "}";
    }
    json += "]}";
    return json;
}

File SD_Manager::openRead(const String& path) {
    if (!isReady()) return File();
    String p = path.startsWith("/") ? path : ("/" + path);
    return SD.open(p, FILE_READ);
}

File SD_Manager::openWrite(const String& path) {
    if (!isReady()) return File();
    String p = path.startsWith("/") ? path : ("/" + path);
    if (SD.exists(p)) {
        SD.remove(p);
    }
    return SD.open(p, FILE_WRITE);
}

bool SD_Manager::remove(const String& path) {
    if (!isReady()) return false;
    String p = path.startsWith("/") ? path : ("/" + path);
    return SD.remove(p);
}

bool SD_Manager::exists(const String& path) {
    if (!isReady()) return false;
    String p = path.startsWith("/") ? path : ("/" + path);
    return SD.exists(p);
}
