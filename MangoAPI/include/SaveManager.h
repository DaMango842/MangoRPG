#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <optional>
#include <nlohmann/json.hpp>
#include "SafeTypes.hpp"
#include "Player.h"
#include "SlotInfo.h"

#include <MangoPtr.hpp>

using json = nlohmann::json;

const int MAX_SLOTS = 30;

class SaveManager {
public:
    static SaveManager& getInstance();

    bool saveToSlot(int slotIndex, SafePtr<Player> player);
    bool loadFromSlot(int slotIndex, SafePtr<Player> player);

    MangoPtr<Player> loadFromSlot(int slotIndex);

    bool renameSaveSlot(int slotIndex, const std::string& newName); // 源代码不改动
    bool deleteSlot(int slotIndex);

    bool hasDataAtSlot(int slot) const;

    std::string getSlotPath(int slotIndex) const;
    std::vector<int> getUsedSlots() const;
    std::vector<SlotInfo> getAllSlotInfos() const;

private:
    SaveManager();

    std::string getSavesDirectory() const;
    std::string currentTimeString() const;

    static constexpr const char* SAVE_DIR = "saves";
};
