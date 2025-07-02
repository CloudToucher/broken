#include "FlagMapper.h"
#include <iostream>
#include <algorithm>

// 静态成员变量定义
std::unordered_map<std::string, ItemFlag> FlagMapper::stringToFlag;
std::unordered_map<ItemFlag, std::string> FlagMapper::flagToString;
bool FlagMapper::initialized = false;

void FlagMapper::initializeMappings() {
    if (initialized) return;
    
    // 基本属性标签
    stringToFlag["WEARABLE"] = ItemFlag::WEARABLE;
    stringToFlag["STACKABLE"] = ItemFlag::STACKABLE;
    stringToFlag["CONSUMABLE"] = ItemFlag::CONSUMABLE;
    stringToFlag["CONTAINER"] = ItemFlag::CONTAINER;
    stringToFlag["SINGLE_SLOT"] = ItemFlag::SINGLE_SLOT;
    stringToFlag["EXPANDS_WITH_CONTENTS"] = ItemFlag::EXPANDS_WITH_CONTENTS;
    
    // 类别标签
    stringToFlag["ARMOR"] = ItemFlag::ARMOR;
    stringToFlag["FOOD"] = ItemFlag::FOOD;
    stringToFlag["MEDICAL"] = ItemFlag::MEDICAL;
    stringToFlag["TOOL"] = ItemFlag::TOOL;
    stringToFlag["MISC"] = ItemFlag::MISC;
    
    // 新增标志
    stringToFlag["ONLY_ARMOR_PLATE"] = ItemFlag::ONLY_ARMOR_PLATE;
    stringToFlag["USES_POWER"] = ItemFlag::USES_POWER;
    stringToFlag["ARMOR_PLATE"] = ItemFlag::ARMOR_PLATE;
    stringToFlag["STRENGTH_BOOST"] = ItemFlag::STRENGTH_BOOST;
    stringToFlag["HEAVY"] = ItemFlag::HEAVY;
    
    // 稀有度标签
    stringToFlag["COMMON"] = ItemFlag::COMMON;
    stringToFlag["RARE"] = ItemFlag::RARE;
    stringToFlag["EPIC"] = ItemFlag::EPIC;
    stringToFlag["LEGENDARY"] = ItemFlag::LEGENDARY;
    stringToFlag["MYTHIC"] = ItemFlag::MYTHIC;
    
    // 身体部位标签
    stringToFlag["SLOT_HEAD"] = ItemFlag::SLOT_HEAD;
    stringToFlag["SLOT_CHEST"] = ItemFlag::SLOT_CHEST;
    stringToFlag["SLOT_ABDOMEN"] = ItemFlag::SLOT_ABDOMEN;
    stringToFlag["SLOT_LEFT_LEG"] = ItemFlag::SLOT_LEFT_LEG;
    stringToFlag["SLOT_RIGHT_LEG"] = ItemFlag::SLOT_RIGHT_LEG;
    stringToFlag["SLOT_LEFT_FOOT"] = ItemFlag::SLOT_LEFT_FOOT;
    stringToFlag["SLOT_RIGHT_FOOT"] = ItemFlag::SLOT_RIGHT_FOOT;
    stringToFlag["SLOT_LEFT_ARM"] = ItemFlag::SLOT_LEFT_ARM;
    stringToFlag["SLOT_RIGHT_ARM"] = ItemFlag::SLOT_RIGHT_ARM;
    stringToFlag["SLOT_LEFT_HAND"] = ItemFlag::SLOT_LEFT_HAND;
    stringToFlag["SLOT_RIGHT_HAND"] = ItemFlag::SLOT_RIGHT_HAND;
    stringToFlag["SLOT_BACK"] = ItemFlag::SLOT_BACK;
    
    // 武器类型标签
    stringToFlag["WEAPON"] = ItemFlag::WEAPON;
    stringToFlag["GUN"] = ItemFlag::GUN;
    stringToFlag["MELEE"] = ItemFlag::MELEE;
    stringToFlag["THROWABLE"] = ItemFlag::THROWABLE;
    stringToFlag["GUNMOD"] = ItemFlag::GUNMOD;
    
    // 近战武器子类型标签
    stringToFlag["SWORD"] = ItemFlag::SWORD;
    stringToFlag["AXE"] = ItemFlag::AXE;
    stringToFlag["HAMMER"] = ItemFlag::HAMMER;
    stringToFlag["SPEAR"] = ItemFlag::SPEAR;
    stringToFlag["DAGGER"] = ItemFlag::DAGGER;
    
    // 枪械类型标签
    stringToFlag["PISTOL"] = ItemFlag::PISTOL;
    stringToFlag["REVOLVER"] = ItemFlag::REVOLVER;
    stringToFlag["SHOTGUN"] = ItemFlag::SHOTGUN;
    stringToFlag["SMG"] = ItemFlag::SMG;
    stringToFlag["RIFLE"] = ItemFlag::RIFLE;
    stringToFlag["DMR"] = ItemFlag::DMR;
    stringToFlag["SNIPER_RIFLE"] = ItemFlag::SNIPER_RIFLE;
    stringToFlag["MACHINE_GUN"] = ItemFlag::MACHINE_GUN;
    stringToFlag["GRENADE_LAUNCHER"] = ItemFlag::GRENADE_LAUNCHER;
    
    // 弹药相关标签
    stringToFlag["MAGAZINE"] = ItemFlag::MAGAZINE;
    stringToFlag["AMMO"] = ItemFlag::AMMO;
    
    // 射击模式标签
    stringToFlag["SEMI_AUTO"] = ItemFlag::SEMI_AUTO;
    stringToFlag["FULL_AUTO"] = ItemFlag::FULL_AUTO;
    stringToFlag["BOLT_ACTION"] = ItemFlag::BOLT_ACTION;
    stringToFlag["BURST"] = ItemFlag::BURST;
    
    // 枪械配件槽位标签
    stringToFlag["GUN_MOD"] = ItemFlag::GUN_MOD;
    stringToFlag["MOD_STOCK"] = ItemFlag::MOD_STOCK;
    stringToFlag["MOD_BARREL"] = ItemFlag::MOD_BARREL;
    stringToFlag["MOD_UNDER_BARREL"] = ItemFlag::MOD_UNDER_BARREL;
    stringToFlag["MOD_GRIP"] = ItemFlag::MOD_GRIP;
    stringToFlag["MOD_OPTIC"] = ItemFlag::MOD_OPTIC;
    stringToFlag["MOD_SIDE_MOUNT"] = ItemFlag::MOD_SIDE_MOUNT;
    stringToFlag["MOD_MUZZLE"] = ItemFlag::MOD_MUZZLE;
    stringToFlag["MOD_MAGAZINE_WELL"] = ItemFlag::MOD_MAGAZINE_WELL;
    stringToFlag["MOD_RAIL"] = ItemFlag::MOD_RAIL;
    stringToFlag["MOD_LASER"] = ItemFlag::MOD_LASER;
    stringToFlag["MOD_FLASHLIGHT"] = ItemFlag::MOD_FLASHLIGHT;
    
    // 其他特性标签
    stringToFlag["SILENCED"] = ItemFlag::SILENCED;
    stringToFlag["SCOPE"] = ItemFlag::SCOPE;
    stringToFlag["LASER"] = ItemFlag::LASER;
    stringToFlag["FLASHLIGHT"] = ItemFlag::FLASHLIGHT;
    
    // 新增：配件槽位类型标识
    stringToFlag["SLOT_STOCK"] = ItemFlag::SLOT_STOCK;
    stringToFlag["SLOT_BARREL"] = ItemFlag::SLOT_BARREL;
    stringToFlag["SLOT_UNDER_BARREL"] = ItemFlag::SLOT_UNDER_BARREL;
    stringToFlag["SLOT_GRIP"] = ItemFlag::SLOT_GRIP;
    stringToFlag["SLOT_OPTIC"] = ItemFlag::SLOT_OPTIC;
    stringToFlag["SLOT_SIDE_MOUNT"] = ItemFlag::SLOT_SIDE_MOUNT;
    stringToFlag["SLOT_MUZZLE"] = ItemFlag::SLOT_MUZZLE;
    stringToFlag["SLOT_MAGAZINE_WELL"] = ItemFlag::SLOT_MAGAZINE_WELL;
    stringToFlag["SLOT_RAIL"] = ItemFlag::SLOT_RAIL;
    stringToFlag["SLOT_SPECIAL"] = ItemFlag::SLOT_SPECIAL;
    
    // 新增：弹药口径标识
    stringToFlag["CALIBER_5_56"] = ItemFlag::CALIBER_5_56;
    stringToFlag["CALIBER_7_62"] = ItemFlag::CALIBER_7_62;
    stringToFlag["CALIBER_9MM"] = ItemFlag::CALIBER_9MM;
    stringToFlag["CALIBER_45ACP"] = ItemFlag::CALIBER_45ACP;
    stringToFlag["CALIBER_12GA"] = ItemFlag::CALIBER_12GA;
    stringToFlag["CALIBER_308"] = ItemFlag::CALIBER_308;
    stringToFlag["CALIBER_22LR"] = ItemFlag::CALIBER_22LR;
    stringToFlag["CALIBER_50BMG"] = ItemFlag::CALIBER_50BMG;
    
    // 新增：兼容性标识
    stringToFlag["ACCEPTS_5_56"] = ItemFlag::ACCEPTS_5_56;
    stringToFlag["ACCEPTS_7_62"] = ItemFlag::ACCEPTS_7_62;
    stringToFlag["ACCEPTS_9MM"] = ItemFlag::ACCEPTS_9MM;
    stringToFlag["ACCEPTS_45ACP"] = ItemFlag::ACCEPTS_45ACP;
    stringToFlag["ACCEPTS_12GA"] = ItemFlag::ACCEPTS_12GA;
    stringToFlag["ACCEPTS_308"] = ItemFlag::ACCEPTS_308;
    stringToFlag["ACCEPTS_22LR"] = ItemFlag::ACCEPTS_22LR;
    stringToFlag["ACCEPTS_50BMG"] = ItemFlag::ACCEPTS_50BMG;
    
    // 新增：特殊功能标识
    stringToFlag["ADDS_RAIL_SLOTS"] = ItemFlag::ADDS_RAIL_SLOTS;
    stringToFlag["CHANGES_CALIBER"] = ItemFlag::CHANGES_CALIBER;
    stringToFlag["BIPOD"] = ItemFlag::BIPOD;
    stringToFlag["SUPPRESSER"] = ItemFlag::SUPPRESSER;
    stringToFlag["COMPENSATOR"] = ItemFlag::COMPENSATOR;
    stringToFlag["FLASH_HIDER"] = ItemFlag::FLASH_HIDER;
    
    // 创建反向映射
    for (const auto& pair : stringToFlag) {
        flagToString[pair.second] = pair.first;
    }
    
    initialized = true;
}

ItemFlag FlagMapper::stringToItemFlag(const std::string& str) {
    if (!initialized) {
        initializeMappings();
    }
    
    auto it = stringToFlag.find(str);
    if (it != stringToFlag.end()) {
        return it->second;
    }
    
    // 如果找不到，返回一个默认值，这里使用FLAG_COUNT表示无效
    std::cerr << "警告: 无效的ItemFlag字符串: " << str << std::endl;
    return ItemFlag::FLAG_COUNT;
}

std::string FlagMapper::itemFlagToString(ItemFlag flag) {
    if (!initialized) {
        initializeMappings();
    }
    
    auto it = flagToString.find(flag);
    if (it != flagToString.end()) {
        return it->second;
    }
    
    std::cerr << "警告: 无效的ItemFlag枚举值: " << static_cast<int>(flag) << std::endl;
    return "UNKNOWN";
}

std::vector<ItemFlag> FlagMapper::stringArrayToFlags(const std::vector<std::string>& strings) {
    std::vector<ItemFlag> flags;
    flags.reserve(strings.size());
    
    for (const auto& str : strings) {
        ItemFlag flag = stringToItemFlag(str);
        if (flag != ItemFlag::FLAG_COUNT) {
            flags.push_back(flag);
        }
    }
    
    return flags;
}

std::vector<std::string> FlagMapper::flagsToStringArray(const std::vector<ItemFlag>& flags) {
    std::vector<std::string> strings;
    strings.reserve(flags.size());
    
    for (ItemFlag flag : flags) {
        std::string str = itemFlagToString(flag);
        if (str != "UNKNOWN") {
            strings.push_back(str);
        }
    }
    
    return strings;
}

void FlagMapper::addFlagsFromJson(Item* item, const nlohmann::json& flagArray) {
    if (!item || !flagArray.is_array()) {
        return;
    }
    
    for (const auto& flagElement : flagArray) {
        if (flagElement.is_string()) {
            std::string flagStr = flagElement.get<std::string>();
            ItemFlag flag = stringToItemFlag(flagStr);
            if (flag != ItemFlag::FLAG_COUNT) {
                item->addFlag(flag);
            }
        }
    }
}

nlohmann::json FlagMapper::flagsToJson(const Item* item) {
    if (!item) {
        return nlohmann::json::array();
    }
    
    nlohmann::json flagArray = nlohmann::json::array();
    
    // 遍历所有可能的flag，检查item是否具有该flag
    for (int i = 0; i < static_cast<int>(ItemFlag::FLAG_COUNT); ++i) {
        ItemFlag flag = static_cast<ItemFlag>(i);
        if (item->hasFlag(flag)) {
            std::string flagStr = itemFlagToString(flag);
            if (flagStr != "UNKNOWN") {
                flagArray.push_back(flagStr);
            }
        }
    }
    
    return flagArray;
}

bool FlagMapper::isValidFlagString(const std::string& str) {
    if (!initialized) {
        initializeMappings();
    }
    
    return stringToFlag.find(str) != stringToFlag.end();
}

std::vector<std::string> FlagMapper::getInvalidFlags(const std::vector<std::string>& flags) {
    std::vector<std::string> invalidFlags;
    
    for (const std::string& flag : flags) {
        if (!isValidFlagString(flag)) {
            invalidFlags.push_back(flag);
        }
    }
    
    return invalidFlags;
}

std::vector<std::string> FlagMapper::getAllValidFlagStrings() {
    if (!initialized) {
        initializeMappings();
    }
    
    std::vector<std::string> validFlags;
    validFlags.reserve(stringToFlag.size());
    
    for (const auto& pair : stringToFlag) {
        validFlags.push_back(pair.first);
    }
    
    // 排序以便于查看和调试
    std::sort(validFlags.begin(), validFlags.end());
    
    return validFlags;
} 