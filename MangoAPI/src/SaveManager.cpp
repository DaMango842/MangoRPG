#include "SaveManager.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <ctime>
#include <iomanip>

SaveManager::SaveManager() {
    std::filesystem::create_directories(getSavesDirectory());
}

SaveManager& SaveManager::getInstance() {
    static SaveManager instance;
    return instance;
}

std::string SaveManager::getSavesDirectory() const {
    return SAVE_DIR;
}

std::string SaveManager::getSlotPath(int slotIndex) const {
    return (std::filesystem::path(getSavesDirectory()) / ("slot" + std::to_string(slotIndex) + ".json")).string();
}

std::string SaveManager::currentTimeString() const {
    std::time_t now = std::time(nullptr);
    std::tm timeInfo;
    localtime_s(&timeInfo, &now);
    std::ostringstream oss;
    oss << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

bool SaveManager::saveToSlot(int slot, SafePtr<Player> player) {
    json j;
    j["playerName"] = player->getName().toAnsiString();
    j["position"] = { {"x", player->getPosition().x}, {"y", player->getPosition().y} };
    j["attributes"] = player->getAttributes().toJson();
    j["timestamp"] = currentTimeString();
    j["playerImgPath"] = player->getImagePath();

    std::ofstream file(getSlotPath(slot));
    if (!file) {
        std::cerr << "Failed to write save file.\n";
        return false;
    }
    file << j.dump(4);
    return true;
}

bool SaveManager::loadFromSlot(int slot, SafePtr<Player> player) {
    std::ifstream file(getSlotPath(slot));
    if (!file) {
        std::cerr << "Failed to read save file.\n";
        return false;
    }

    json j;
    try {
        file >> j;
    }
    catch (const json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return false;
    }

    if (j.contains("playerName") && j["playerName"].is_string()) {
        player->setName(sf::String(j["playerName"].get<std::string>()));
    }
    if (j.contains("position")) {
        player->setPosition({ j["position"].value("x", 0.f), j["position"].value("y", 0.f) });
    }
    if (j.contains("attributes")) player->getAttributes().fromJson(j["attributes"]);
    if (j.contains("playerImgPath")) player->setImagePath(j["playerImgPath"]);

    return true;
}

MangoPtr<Player> SaveManager::loadFromSlot(int slotIndex)
{
    std::ifstream file(getSlotPath(slotIndex));
    if (!file) {
        std::cerr << "Failed to read save file.\n";
        return nullptr;
    }

    json j;
    try {
        file >> j;
    }
    catch (const json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return nullptr;
    }

    auto player = MangoPtr<Player>(Player::createDefaultData());

    if (j.contains("playerName") && j["playerName"].is_string()) {
        player->setName(sf::String(j["playerName"].get<std::string>()));
    }
    if (j.contains("position")) {
        player->setPosition({ j["position"].value("x", 0.f), j["position"].value("y", 0.f) });
    }
    if (j.contains("attributes")) player->getAttributes().fromJson(j["attributes"]);
    if (j.contains("playerImgPath")) player->setImagePath(j["playerImgPath"]);

    return MangoPtr<Player>(std::move(player));
}


bool SaveManager::renameSaveSlot(int slotIndex, const std::string& newName) {
    std::string path = getSlotPath(slotIndex);
    if (!std::filesystem::exists(path)) {
        return false;
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    json j;
    try {
        file >> j;
    }
    catch (...) {
        return false;
    }
    file.close();

    j["name"] = newName;

    std::ofstream outFile(path);
    if (!outFile.is_open()) {
        return false;
    }
    outFile << j.dump(4);
    return true;
}

bool SaveManager::deleteSlot(int slotIndex) {
    std::string path = getSlotPath(slotIndex);
    if (std::filesystem::exists(path)) {
        return std::filesystem::remove(path);
    }
    return false;
}

bool SaveManager::hasDataAtSlot(int slot) const {
    if (slot < 0 || slot >= MAX_SLOTS) return false;
    std::string path = getSlotPath(slot);
    std::ifstream inFile(path);
    return inFile.good(); // 文件存在即代表有存档
}

std::vector<int> SaveManager::getUsedSlots() const {
    std::vector<int> slots;
    for (const auto& entry : std::filesystem::directory_iterator(getSavesDirectory())) {
        if (entry.path().extension() == ".json") {
            std::string filename = entry.path().stem().string();
            if (filename.rfind("slot", 0) == 0) {
                try {
                    int index = std::stoi(filename.substr(4));
                    slots.push_back(index);
                }
                catch (...) {}
            }
        }
    }
    return slots;
}

std::vector<SlotInfo> SaveManager::getAllSlotInfos() const {
    std::vector<SlotInfo> result;

    for (int i = 0; i < MAX_SLOTS; ++i) {
        std::string path = getSlotPath(i);
        SlotInfo info;
        info.slotIndex = i;

        if (std::filesystem::exists(path)) {
            std::ifstream file(path);
            if (!file.is_open()) {
                result.push_back(info);
                continue;
            }

            try {
                json j;
                file >> j;
                info.valid = true;

                if (j.contains("name"))
                    info.saveName = j["name"].get<std::string>();
                else if (j.contains("playerName"))
                    info.playerName = j["playerName"].get<std::string>();

                if (j.contains("timestamp")) {
                    // 解析时间戳字符串为 UNIX 时间戳（毫秒）
                    std::istringstream ss(j["timestamp"].get<std::string>());
                    std::tm tm = {};
                    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
                    if (!ss.fail()) {
                        info.timestamp = static_cast<Timestamp>(std::mktime(&tm)) * 1000;
                    }
                }

                if (j.contains("position")) {
                    info.position.x = j["position"].value("x", 0.f);
                    info.position.y = j["position"].value("y", 0.f);
                }

                if (j.contains("attributes")) {
                    const auto& attr = j["attributes"];
                    json preview;
                    if (attr.contains("LV")) preview["LV"] = attr["LV"];
                    if (attr.contains("HP")) preview["HP"] = attr["HP"];
                    if (attr.contains("MONEY")) preview["MONEY"] = attr["MONEY"];
                    info.attributesPreview = preview;
                }

            }
            catch (...) {}
        }

        result.push_back(info);
    }

    return result;
}
