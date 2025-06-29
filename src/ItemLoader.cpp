#include "ItemLoader.h"
#include "Storage.h"
#include "MeleeWeapon.h"
#include <fstream>
#include <iostream>
#include <stdexcept>

// 初始化静态实例指针
ItemLoader* ItemLoader::instance = nullptr;

// 获取单例实例
ItemLoader* ItemLoader::getInstance() {
    if (instance == nullptr) {
        instance = new ItemLoader();
    }
    return instance;
}

// 从JSON文件加载所有物品
bool ItemLoader::loadItemsFromFile(const std::string& filePath) {
    try {
        // 打开文件
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "无法打开物品数据文件: " << filePath << std::endl;
            return false;
        }
        
        // 解析JSON
        json jsonData;
        file >> jsonData;
        file.close();
        
        // 加载物品
        return loadItemsFromJson(jsonData);
    } catch (const std::exception& e) {
        std::cerr << "加载物品数据文件时出错: " << e.what() << std::endl;
        return false;
    }
}

// 从JSON字符串加载物品
bool ItemLoader::loadItemsFromJson(const std::string& jsonString) {
    try {
        json jsonData = json::parse(jsonString);
        return loadItemsFromJson(jsonData);
    } catch (const std::exception& e) {
        std::cerr << "解析JSON字符串时出错: " << e.what() << std::endl;
        return false;
    }
}

// 从JSON对象加载物品
bool ItemLoader::loadItemsFromJson(const json& jsonData) {
    try {
        // 加载基本物品
        if (jsonData.contains("items") && jsonData["items"].is_array()) {
            for (const auto& itemJson : jsonData["items"]) {
                auto item = loadItemFromJson(itemJson);
                if (item) {
                    std::string name = item->getName();
                    itemTemplates[name] = std::move(item);
                }
            }
        }
        
        // 加载枪械
        if (jsonData.contains("guns") && jsonData["guns"].is_array()) {
            for (const auto& gunJson : jsonData["guns"]) {
                auto gun = loadGunFromJson(gunJson);
                if (gun) {
                    std::string name = gun->getName();
                    // 先存储在gunTemplates中
                    gunTemplates[name] = std::move(gun);
                    // 然后创建一个副本用于itemTemplates
                    // 这里直接使用dynamic_cast将Gun*转换为Item*
                    itemTemplates[name] = std::unique_ptr<Item>(gunTemplates[name]->clone());
                }
            }
        }
        
        // 加载枪械配件
        if (jsonData.contains("gunmods") && jsonData["gunmods"].is_array()) {
            for (const auto& modJson : jsonData["gunmods"]) {
                auto mod = loadGunModFromJson(modJson);
                if (mod) {
                    std::string name = mod->getName();
                    // 先存储在gunModTemplates中
                    gunModTemplates[name] = std::move(mod);
                    // 然后创建一个副本用于itemTemplates
                    // 这里直接使用dynamic_cast将GunMod*转换为Item*
                    itemTemplates[name] = std::unique_ptr<Item>(gunModTemplates[name]->clone());
                }
            }
        }
        
        // 加载弹匣
        if (jsonData.contains("magazines") && jsonData["magazines"].is_array()) {
            for (const auto& magazineJson : jsonData["magazines"]) {
                auto magazine = loadMagazineFromJson(magazineJson);
                if (magazine) {
                    std::string name = magazine->getName();
                    // 先存储在magazineTemplates中
                    magazineTemplates[name] = std::move(magazine);
                    // 然后创建一个副本用于itemTemplates
                    // 这里直接使用dynamic_cast将Magazine*转换为Item*
                    itemTemplates[name] = std::unique_ptr<Item>(magazineTemplates[name]->clone());
                }
            }
        }
        
        // 加载弹药
        if (jsonData.contains("ammo") && jsonData["ammo"].is_array()) {
            for (const auto& ammoJson : jsonData["ammo"]) {
                auto ammo = loadAmmoFromJson(ammoJson);
                if (ammo) {
                    std::string name = ammo->getName();
                    // 先存储在ammoTemplates中
                    ammoTemplates[name] = std::move(ammo);
                    // 然后创建一个副本用于itemTemplates
                    // 这里直接使用dynamic_cast将Ammo*转换为Item*
                    itemTemplates[name] = std::unique_ptr<Item>(ammoTemplates[name]->clone());
                }
            }
        }
        
        // 加载武器
        if (jsonData.contains("weapons") && jsonData["weapons"].is_array()) {
            for (const auto& weaponJson : jsonData["weapons"]) {
                // 检查武器类型，决定创建哪种武器对象
                if (weaponJson.contains("weaponType") && weaponJson["weaponType"] == "MELEE") {
                    // 创建近战武器
                    auto meleeWeapon = loadMeleeWeaponFromJson(weaponJson);
                    if (meleeWeapon) {
                        std::string name = meleeWeapon->getName();
                        // 存储为通用武器模板
                        itemTemplates[name] = std::unique_ptr<Item>(meleeWeapon->clone());
                        std::cout << "加载近战武器: " << name << std::endl;
                    }
                } else {
                    // 创建普通武器（远程、投掷等）
                auto weapon = loadWeaponFromJson(weaponJson);
                if (weapon) {
                    std::string name = weapon->getName();
                    // 先存储在weaponTemplates中
                    weaponTemplates[name] = std::move(weapon);
                    // 然后创建一个副本用于itemTemplates
                    itemTemplates[name] = std::unique_ptr<Item>(weaponTemplates[name]->clone());
                    }
                }
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "处理JSON数据时出错: " << e.what() << std::endl;
        return false;
    }
}

// 创建物品实例（基于模板的深拷贝）
std::unique_ptr<Item> ItemLoader::createItem(const std::string& itemName) {
    if (!hasItemTemplate(itemName)) {
        std::cerr << "未找到物品模板: " << itemName << std::endl;
        return nullptr;
    }
    
    // 创建新物品（使用简化的构造函数）
    const Item* templateItem = itemTemplates[itemName].get();
    auto newItem = std::make_unique<Item>(templateItem->getName());
    
    // 设置基本属性
    newItem->setWeight(templateItem->getWeight());
    newItem->setVolume(templateItem->getVolume());
    newItem->setLength(templateItem->getLength());
    newItem->setValue(templateItem->getValue());
    
    // 复制装备槽位
    for (const auto& slot : templateItem->getEquipSlots()) {
        newItem->addEquipSlot(slot);
    }
    
    // 复制标签 - 遍历所有可能的ItemFlag枚举值
    // 从WEARABLE到FLASHLIGHT，确保包含所有标签
    for (int i = static_cast<int>(ItemFlag::WEARABLE); i <= static_cast<int>(ItemFlag::FLASHLIGHT); ++i) {
        ItemFlag flag = static_cast<ItemFlag>(i);
        if (templateItem->hasFlag(flag)) {
            newItem->addFlag(flag);
            std::cout << "复制标签到新物品 " << newItem->getName() << ": " << GetItemFlagName(flag) << std::endl;
        }
    }
    
    // 复制新增属性
    newItem->setDescription(templateItem->getDescription());
    newItem->setPiercingDamage(templateItem->getPiercingDamage());
    newItem->setBluntDamage(templateItem->getBluntDamage());
    newItem->setSlashingDamage(templateItem->getSlashingDamage());
    newItem->setAttackTime(templateItem->getAttackTime());
    newItem->setStaminaCost(templateItem->getStaminaCost());
    newItem->setActivationCost(templateItem->getActivationCost());
    newItem->setPiercingDefense(templateItem->getPiercingDefense());
    newItem->setBluntDefense(templateItem->getBluntDefense());
    newItem->setSlashingDefense(templateItem->getSlashingDefense());
    newItem->setBulletDefense(templateItem->getBulletDefense());
    newItem->setUsesRemaining(templateItem->getUsesRemaining());
    
    // 复制存储空间
    for (size_t i = 0; i < templateItem->getStorageCount(); ++i) {
        Storage* templateStorage = templateItem->getStorage(i);
        if (templateStorage) {
            // 创建存储空间的深拷贝
            auto newStorage = std::make_unique<Storage>(templateStorage->getName());
            
            // 设置存储空间属性
            newStorage->setMaxWeight(templateStorage->getMaxWeight());
            newStorage->setMaxVolume(templateStorage->getMaxVolume());
            newStorage->setMaxLength(templateStorage->getMaxLength());
            newStorage->setAccessTime(templateStorage->getAccessTime());
            newStorage->setMaxItems(templateStorage->getMaxItems());
            newStorage->setStorageTime(templateStorage->getStorageTime());
            
            newItem->addStorage(std::move(newStorage));
        }
    }
    
    return newItem;
}

std::unique_ptr<Gun> ItemLoader::createGun(const std::string& gunName) {
    if (!hasItemTemplate(gunName)) {
        std::cerr << "未找到枪械模板: " << gunName << std::endl;
        return nullptr;
    }
    
    // 确保模板是Gun类型
    const Gun* templateGun = dynamic_cast<const Gun*>(itemTemplates[gunName].get());
    if (!templateGun) {
        std::cerr << "物品不是枪械类型: " << gunName << std::endl;
        return nullptr;
    }
    
    // 创建新枪械（使用简化的构造函数）
    auto newGun = std::make_unique<Gun>(templateGun->getName());
    
    // 设置基本属性
    newGun->setWeight(templateGun->getWeight());
    newGun->setVolume(templateGun->getVolume());
    newGun->setLength(templateGun->getLength());
    newGun->setValue(templateGun->getValue());
    
    // 设置枪械特有属性
    newGun->setGunType(templateGun->getGunType());
    newGun->setBaseFireRate(templateGun->getBaseFireRate());
    newGun->setBaseAccuracyMOA(templateGun->getBaseAccuracyMOA());
    newGun->setAvailableFiringModes(std::vector<FiringMode>{templateGun->getCurrentFiringMode()}); // 简化处理，只复制当前射击模式
    newGun->setBaseRecoil(templateGun->getBaseRecoil());
    newGun->setBaseErgonomics(templateGun->getBaseErgonomics());
    newGun->setBaseBreathStability(templateGun->getBaseBreathStability());
    newGun->setAcceptedAmmoTypes(std::vector<std::string>{}); // 简化处理，暂不复制弹药类型
    newGun->setBaseSoundLevel(templateGun->getBaseSoundLevel());
    newGun->setBaseDamageBonus(templateGun->getBaseDamageBonus());
    newGun->setBaseRangeBonus(templateGun->getBaseRangeBonus());
    newGun->setBaseBulletSpeedBonus(templateGun->getBaseBulletSpeedBonus());
    newGun->setBasePenetrationBonus(templateGun->getBasePenetrationBonus());
    
    // 复制装备槽位
    for (const auto& slot : templateGun->getEquipSlots()) {
        newGun->addEquipSlot(slot);
    }
    
    // 复制标签
    for (int i = static_cast<int>(ItemFlag::WEARABLE); i <= static_cast<int>(ItemFlag::FLASHLIGHT); ++i) {
        ItemFlag flag = static_cast<ItemFlag>(i);
        if (templateGun->hasFlag(flag)) {
            newGun->addFlag(flag);
        }
    }
    
    // 复制配件槽位容量
    for (const auto& [slot, capacity] : templateGun->getSlotCapacity()) {
        newGun->setSlotCapacity(slot, capacity);
    }
    
    // 复制可接受的弹匣名称列表
    newGun->setAcceptedMagazineNames(templateGun->getAcceptedMagazineNames());
    
    // 注意：这里没有复制枪械的配件、弹匣和枪膛内子弹
    // 这些通常应该在游戏逻辑中单独处理
    
    return newGun;
}

std::unique_ptr<GunMod> ItemLoader::createGunMod(const std::string& modName) {
    if (!hasItemTemplate(modName)) {
        std::cerr << "未找到枪械配件模板: " << modName << std::endl;
        return nullptr;
    }
    
    // 确保模板是GunMod类型
    const GunMod* templateMod = dynamic_cast<const GunMod*>(itemTemplates[modName].get());
    if (!templateMod) {
        std::cerr << "物品不是枪械配件类型: " << modName << std::endl;
        return nullptr;
    }
    
    // 创建新枪械配件（使用简化的构造函数）
    auto newMod = std::make_unique<GunMod>(templateMod->getName());
    
    // 设置基本属性
    newMod->setWeight(templateMod->getWeight());
    newMod->setVolume(templateMod->getVolume());
    newMod->setLength(templateMod->getLength());
    newMod->setValue(templateMod->getValue());
    
    // 复制装备槽位
    for (const auto& slot : templateMod->getEquipSlots()) {
        newMod->addEquipSlot(slot);
    }
    
    // 复制标签
    for (int i = static_cast<int>(ItemFlag::WEARABLE); i <= static_cast<int>(ItemFlag::FLASHLIGHT); ++i) {
        ItemFlag flag = static_cast<ItemFlag>(i);
        if (templateMod->hasFlag(flag)) {
            newMod->addFlag(flag);
        }
    }
    
    // 复制枪械配件特有属性
    newMod->setModAttributes(
        templateMod->getModSoundLevel(),
        templateMod->getModFireRate(),
        templateMod->getModDamageBonus(),
        templateMod->getModAccuracyMOA(),
        templateMod->getModRecoil(),
        templateMod->getModErgonomics(),
        templateMod->getModBreathStability(),
        templateMod->getModRangeBonus(),
        templateMod->getModBulletSpeedBonus(),
        templateMod->getModPenetrationBonus()
    );
    
    return newMod;
}

std::unique_ptr<Magazine> ItemLoader::createMagazine(const std::string& magazineName) {
    if (!hasItemTemplate(magazineName)) {
        std::cerr << "未找到弹匣模板: " << magazineName << std::endl;
        return nullptr;
    }
    
    // 确保模板是Magazine类型
    const Magazine* templateMag = dynamic_cast<const Magazine*>(itemTemplates[magazineName].get());
    if (!templateMag) {
        std::cerr << "物品不是弹匣类型: " << magazineName << std::endl;
        return nullptr;
    }
    
    // 创建新弹匣（使用简化的构造函数）
    auto newMag = std::make_unique<Magazine>(templateMag->getName());
    
    // 设置基本属性
    newMag->setWeight(templateMag->getWeight());
    newMag->setVolume(templateMag->getVolume());
    newMag->setLength(templateMag->getLength());
    newMag->setValue(templateMag->getValue());
    
    // 设置弹匣特有属性
    newMag->setCompatibleAmmoTypes(templateMag->getCompatibleAmmoTypes());
    newMag->setCapacity(templateMag->getCapacity());
    newMag->setUnloadTime(templateMag->getUnloadTime());
    newMag->setReloadTime(templateMag->getReloadTime());
    
    // 复制装备槽位
    for (const auto& slot : templateMag->getEquipSlots()) {
        newMag->addEquipSlot(slot);
    }
    
    // 复制标签
    for (int i = static_cast<int>(ItemFlag::WEARABLE); i <= static_cast<int>(ItemFlag::FLASHLIGHT); ++i) {
        ItemFlag flag = static_cast<ItemFlag>(i);
        if (templateMag->hasFlag(flag)) {
            newMag->addFlag(flag);
        }
    }
    
    // 注意：这里没有复制弹匣中的子弹
    // 这些通常应该在游戏逻辑中单独处理
    
    return newMag;
}

std::unique_ptr<Ammo> ItemLoader::createAmmo(const std::string& ammoName) {
    if (!hasItemTemplate(ammoName)) {
        std::cerr << "未找到弹药模板: " << ammoName << std::endl;
        return nullptr;
    }
    
    // 确保模板是Ammo类型
    const Ammo* templateAmmo = dynamic_cast<const Ammo*>(itemTemplates[ammoName].get());
    if (!templateAmmo) {
        std::cerr << "物品不是弹药类型: " << ammoName << std::endl;
        return nullptr;
    }
    
    // 创建新弹药（使用简化的构造函数）
    auto newAmmo = std::make_unique<Ammo>(templateAmmo->getName());
    
    // 设置基本属性
    newAmmo->setWeight(templateAmmo->getWeight());
    newAmmo->setVolume(templateAmmo->getVolume());
    newAmmo->setLength(templateAmmo->getLength());
    newAmmo->setValue(templateAmmo->getValue());
    
    // 设置弹药特有属性
    newAmmo->setBaseDamage(templateAmmo->getBaseDamage());
    newAmmo->setBasePenetration(templateAmmo->getBasePenetration());
    newAmmo->setBaseRange(templateAmmo->getBaseRange());
    newAmmo->setBaseSpeed(templateAmmo->getBaseSpeed());
    newAmmo->setAmmoType(templateAmmo->getAmmoType());
    newAmmo->setModRecoil(templateAmmo->getModRecoil());
    newAmmo->setModAccuracyMOA(templateAmmo->getModAccuracyMOA());
    newAmmo->setModErgonomics(templateAmmo->getModErgonomics());
    
    // 复制装备槽位
    for (const auto& slot : templateAmmo->getEquipSlots()) {
        newAmmo->addEquipSlot(slot);
    }
    
    // 复制标签
    for (int i = static_cast<int>(ItemFlag::WEARABLE); i <= static_cast<int>(ItemFlag::FLASHLIGHT); ++i) {
        ItemFlag flag = static_cast<ItemFlag>(i);
        if (templateAmmo->hasFlag(flag)) {
            newAmmo->addFlag(flag);
        }
    }
    
    return newAmmo;
}

// 检查是否存在指定名称的物品模板
bool ItemLoader::hasItemTemplate(const std::string& itemName) const {
    return itemTemplates.find(itemName) != itemTemplates.end();
}

bool ItemLoader::hasGunTemplate(const std::string& gunName) const {
    return gunTemplates.find(gunName) != gunTemplates.end();
}

bool ItemLoader::hasGunModTemplate(const std::string& modName) const {
    return gunModTemplates.find(modName) != gunModTemplates.end();
}

bool ItemLoader::hasMagazineTemplate(const std::string& magazineName) const {
    return magazineTemplates.find(magazineName) != magazineTemplates.end();
}

bool ItemLoader::hasAmmoTemplate(const std::string& ammoName) const {
    return ammoTemplates.find(ammoName) != ammoTemplates.end();
}

// 从JSON对象加载基本物品
std::unique_ptr<Item> ItemLoader::loadItemFromJson(const json& itemJson) {
    try {
        // 检查必要字段
        if (!itemJson.contains("name") || !itemJson["name"].is_string()) {
            std::cerr << "物品JSON缺少有效的name字段" << std::endl;
            return nullptr;
        }
        
        // 获取基本属性
        std::string name = itemJson["name"];
        
        // 创建物品（使用简化的构造函数）
        auto item = std::make_unique<Item>(name);
        
        // 设置基本属性
        if (itemJson.contains("weight")) item->setWeight(itemJson["weight"]);
        if (itemJson.contains("volume")) item->setVolume(itemJson["volume"]);
        if (itemJson.contains("length")) item->setLength(itemJson["length"]);
        if (itemJson.contains("value")) item->setValue(itemJson["value"]);
        
        // 先加载装备槽位，再加载标签
        // 加载装备槽位（向后兼容）
        if (itemJson.contains("equipSlots") && itemJson["equipSlots"].is_array()) {
            loadEquipSlots(item.get(), itemJson["equipSlots"]);
        }
        
        // 加载覆盖率信息（新系统）
        if (itemJson.contains("coverage")) {
            loadCoverageSlots(item.get(), itemJson["coverage"]);
        }
        
        // 加载防护数据（新系统）
        if (itemJson.contains("protection")) {
            loadProtectionData(item.get(), itemJson["protection"]);
        }
        
        // 加载标签
        if (itemJson.contains("flags") && itemJson["flags"].is_array()) {
            loadItemFlags(item.get(), itemJson["flags"]);
        }
        
        // 加载新增属性
        if (itemJson.contains("description") && itemJson["description"].is_string()) {
            item->setDescription(itemJson["description"]);
        }
        
        // 加载伤害属性
        if (itemJson.contains("piercingDamage") && itemJson["piercingDamage"].is_number_integer()) {
            item->setPiercingDamage(itemJson["piercingDamage"]);
        }
        
        if (itemJson.contains("bluntDamage") && itemJson["bluntDamage"].is_number_integer()) {
            item->setBluntDamage(itemJson["bluntDamage"]);
        }
        
        if (itemJson.contains("slashingDamage") && itemJson["slashingDamage"].is_number_integer()) {
            item->setSlashingDamage(itemJson["slashingDamage"]);
        }
        
        // 加载攻击属性
        if (itemJson.contains("attackTime") && itemJson["attackTime"].is_number_float()) {
            item->setAttackTime(itemJson["attackTime"]);
        }
        
        if (itemJson.contains("staminaCost") && itemJson["staminaCost"].is_number_integer()) {
            item->setStaminaCost(itemJson["staminaCost"]);
        }
        
        if (itemJson.contains("activationCost") && itemJson["activationCost"].is_number_integer()) {
            item->setActivationCost(itemJson["activationCost"]);
        }
        
        // 加载防御属性
        if (itemJson.contains("piercingDefense") && itemJson["piercingDefense"].is_number_float()) {
            item->setPiercingDefense(itemJson["piercingDefense"]);
        }
        
        if (itemJson.contains("bluntDefense") && itemJson["bluntDefense"].is_number_float()) {
            item->setBluntDefense(itemJson["bluntDefense"]);
        }
        
        if (itemJson.contains("slashingDefense") && itemJson["slashingDefense"].is_number_float()) {
            item->setSlashingDefense(itemJson["slashingDefense"]);
        }
        
        if (itemJson.contains("bulletDefense") && itemJson["bulletDefense"].is_number_float()) {
            item->setBulletDefense(itemJson["bulletDefense"]);
        }
        
        if (itemJson.contains("usesRemaining") && itemJson["usesRemaining"].is_number_float()) {
            item->setUsesRemaining(itemJson["usesRemaining"]);
        }
        
        // 加载存储空间
        if (itemJson.contains("storages") && itemJson["storages"].is_array()) {
            for (const auto& storageJson : itemJson["storages"]) {
                // 获取存储空间属性
                std::string storageName = storageJson.value("name", "存储空间");
                float maxWeight = storageJson.value("maxWeight", 10.0f);
                float maxVolume = storageJson.value("maxVolume", 10.0f);
                float maxLength = storageJson.value("maxLength", 10.0f);
                float accessTime = storageJson.value("accessTime", 1.0f);
                int maxItems = storageJson.value("maxItems", -1);
                float storageTime = storageJson.value("storageTime", 0.0f);
                
                // 创建存储空间并添加到物品中
                auto storage = std::make_unique<Storage>(storageName);
                storage->setMaxWeight(maxWeight);
                storage->setMaxVolume(maxVolume);
                storage->setMaxLength(maxLength);
                storage->setAccessTime(accessTime);
                storage->setMaxItems(maxItems);
                storage->setStorageTime(storageTime);
                item->addStorage(std::move(storage));
            }
        }
        
        // 打印调试信息
        std::cout << "加载物品: " << name << ", 装备槽位数量: " << item->getEquipSlots().size() << std::endl;
        for (const auto& slot : item->getEquipSlots()) {
            std::cout << "  - 槽位: " << static_cast<int>(slot) << std::endl;
        }
        
        return item;
    } catch (const std::exception& e) {
        std::cerr << "加载物品JSON时出错: " << e.what() << std::endl;
        return nullptr;
    }
}

// 从JSON对象加载枪械
std::unique_ptr<Gun> ItemLoader::loadGunFromJson(const json& gunJson) {
    try {
        // 检查必要字段
        if (!gunJson.contains("name") || !gunJson["name"].is_string()) {
            std::cerr << "枪械JSON缺少有效的name字段" << std::endl;
            return nullptr;
        }
        
        // 获取基本属性
        std::string name = gunJson["name"];
        
        // 创建枪械（使用简化的构造函数）
        auto gun = std::make_unique<Gun>(name);
        
        // 设置基本属性
        if (gunJson.contains("weight")) gun->setWeight(gunJson["weight"]);
        if (gunJson.contains("volume")) gun->setVolume(gunJson["volume"]);
        if (gunJson.contains("length")) gun->setLength(gunJson["length"]);
        if (gunJson.contains("value")) gun->setValue(gunJson["value"]);
        
        // 设置枪械类型
        GunType gunType = GunType::PISTOL; // 默认值
        if (gunJson.contains("gunType") && gunJson["gunType"].is_string()) {
            std::string typeStr = gunJson["gunType"];
            if (typeStr == "PISTOL") gunType = GunType::PISTOL;
            else if (typeStr == "REVOLVER") gunType = GunType::REVOLVER;
            else if (typeStr == "SMG") gunType = GunType::SMG;
            else if (typeStr == "SHOTGUN") gunType = GunType::SHOTGUN;
            else if (typeStr == "RIFLE") gunType = GunType::RIFLE;
            else if (typeStr == "SNIPER_RIFLE") gunType = GunType::SNIPER_RIFLE;
            else if (typeStr == "DMR") gunType = GunType::DMR;
            else if (typeStr == "MACHINE_GUN") gunType = GunType::MACHINE_GUN;
            else if (typeStr == "GRENADE_LAUNCHER") gunType = GunType::GRENADE_LAUNCHER;
        }
        gun->setGunType(gunType);
        
        // 设置枪械基础属性
        if (gunJson.contains("fireRate")) gun->setBaseFireRate(gunJson["fireRate"]);
        if (gunJson.contains("accuracy")) gun->setBaseAccuracyMOA(gunJson["accuracy"]);
        if (gunJson.contains("recoil")) gun->setBaseRecoil(gunJson["recoil"]);
        if (gunJson.contains("ergonomics")) gun->setBaseErgonomics(gunJson["ergonomics"]);
        if (gunJson.contains("breathStability")) gun->setBaseBreathStability(gunJson["breathStability"]);
        if (gunJson.contains("soundLevel")) gun->setBaseSoundLevel(gunJson["soundLevel"]);
        if (gunJson.contains("damageBonus")) gun->setBaseDamageBonus(gunJson["damageBonus"]);
        if (gunJson.contains("rangeBonus")) gun->setBaseRangeBonus(gunJson["rangeBonus"]);
        if (gunJson.contains("bulletSpeedBonus")) gun->setBaseBulletSpeedBonus(gunJson["bulletSpeedBonus"]);
        if (gunJson.contains("penetrationBonus")) gun->setBasePenetrationBonus(gunJson["penetrationBonus"]);
        
        // 设置射击模式
        std::vector<FiringMode> firingModes;
        if (gunJson.contains("firingModes") && gunJson["firingModes"].is_array()) {
            for (const auto& modeJson : gunJson["firingModes"]) {
                if (modeJson.is_string()) {
                    std::string modeStr = modeJson;
                    if (modeStr == "SEMI_AUTO") firingModes.push_back(FiringMode::SEMI_AUTO);
                    else if (modeStr == "FULL_AUTO") firingModes.push_back(FiringMode::FULL_AUTO);
                    else if (modeStr == "BOLT_ACTION") firingModes.push_back(FiringMode::BOLT_ACTION);
                    else if (modeStr == "BURST") firingModes.push_back(FiringMode::BURST);
                }
            }
        }
        if (firingModes.empty()) {
            firingModes.push_back(FiringMode::SEMI_AUTO); // 默认半自动
        }
        gun->setAvailableFiringModes(firingModes);
        
        // 设置兼容弹药类型
        std::vector<std::string> ammoTypes;
        if (gunJson.contains("ammoTypes") && gunJson["ammoTypes"].is_array()) {
            for (const auto& typeJson : gunJson["ammoTypes"]) {
                if (typeJson.is_string()) {
                    ammoTypes.push_back(typeJson);
                }
            }
        }
        gun->setAcceptedAmmoTypes(ammoTypes);
        
        // 设置兼容弹匣名称列表
        std::vector<std::string> magazineNames;
        if (gunJson.contains("acceptedMagazines") && gunJson["acceptedMagazines"].is_array()) {
            for (const auto& magJson : gunJson["acceptedMagazines"]) {
                if (magJson.is_string()) {
                    magazineNames.push_back(magJson);
                }
            }
        }
        if (!magazineNames.empty()) {
            gun->setAcceptedMagazineNames(magazineNames);
        }
        
        // 加载标签
        if (gunJson.contains("flags") && gunJson["flags"].is_array()) {
            loadItemFlags(gun.get(), gunJson["flags"]);
        }
        
        // 加载配件槽位容量（如果JSON中有定义）
        if (gunJson.contains("slotCapacity") && gunJson["slotCapacity"].is_object()) {
            // 先初始化默认槽位容量
            gun->initAttachmentSlots();
            
            // 然后根据JSON覆盖特定槽位的容量
            for (auto it = gunJson["slotCapacity"].begin(); it != gunJson["slotCapacity"].end(); ++it) {
                std::string slotName = it.key();
                int capacity = it.value();
                
                // 将字符串转换为AttachmentSlot枚举
                AttachmentSlot slot;
                if (slotName == "STOCK") slot = AttachmentSlot::STOCK;
                else if (slotName == "BARREL") slot = AttachmentSlot::BARREL;
                else if (slotName == "UNDER_BARREL") slot = AttachmentSlot::UNDER_BARREL;
                else if (slotName == "GRIP") slot = AttachmentSlot::GRIP;
                else if (slotName == "OPTIC") slot = AttachmentSlot::OPTIC;
                else if (slotName == "RAIL") slot = AttachmentSlot::RAIL;
                else if (slotName == "MUZZLE") slot = AttachmentSlot::MUZZLE;
                else if (slotName == "MAGAZINE_WELL") slot = AttachmentSlot::MAGAZINE_WELL;
                else if (slotName == "SPECIAL") slot = AttachmentSlot::SPECIAL;
                else {
                    std::cerr << "未知配件槽位类型: " << slotName << std::endl;
                    continue;
                }
                
                // 设置槽位容量
                gun->setSlotCapacity(slot, capacity);
            }
        }
        
        return gun;
    } catch (const std::exception& e) {
        std::cerr << "加载枪械时出错: " << e.what() << std::endl;
        return nullptr;
    }
}

// 从JSON对象加载枪械配件
std::unique_ptr<GunMod> ItemLoader::loadGunModFromJson(const json& modJson) {
    try {
        // 检查必要字段
        if (!modJson.contains("name") || !modJson["name"].is_string()) {
            std::cerr << "枪械配件JSON缺少有效的name字段" << std::endl;
            return nullptr;
        }
        
        // 获取基本属性
        std::string name = modJson["name"];
        
        // 创建配件（使用简化的构造函数）
        auto mod = std::make_unique<GunMod>(name);
        
        // 设置基本属性
        if (modJson.contains("weight")) mod->setWeight(modJson["weight"]);
        if (modJson.contains("volume")) mod->setVolume(modJson["volume"]);
        if (modJson.contains("length")) mod->setLength(modJson["length"]);
        if (modJson.contains("value")) mod->setValue(modJson["value"]);
        
        // 获取配件特有属性
        float soundLevel = modJson.value("modSoundLevel", 0.0f);
        float fireRate = modJson.value("modFireRate", 0.0f);
        float damageBonus = modJson.value("modDamageBonus", 0.0f);
        float accuracy = modJson.value("modAccuracyMOA", 0.0f);
        float recoil = modJson.value("modRecoil", 0.0f);
        float ergonomics = modJson.value("modErgonomics", 0.0f);
        float breathStability = modJson.value("modBreathStability", 0.0f);
        float range = modJson.value("modRange", 0.0f);
        float bulletSpeed = modJson.value("modBulletSpeed", 0.0f);
        float penetrationBonus = modJson.value("modPenetrationBonus", 0.0f);
        
        // 设置配件属性
        mod->setModAttributes(
            soundLevel, fireRate, damageBonus, accuracy, recoil, ergonomics,
            breathStability, range, bulletSpeed, penetrationBonus
        );
        
        // 加载标签
        if (modJson.contains("flags") && modJson["flags"].is_array()) {
            loadItemFlags(mod.get(), modJson["flags"]);
        }
        
        // 确保配件至少有一个槽位类型标签
        bool hasSlotTypeFlag = false;
        for (int i = static_cast<int>(ItemFlag::MOD_STOCK); i <= static_cast<int>(ItemFlag::MOD_FLASHLIGHT); ++i) {
            ItemFlag flag = static_cast<ItemFlag>(i);
            if (mod->hasFlag(flag)) {
                hasSlotTypeFlag = true;
                break;
            }
        }
        
        // 如果JSON中指定了槽位类型，但没有通过标签设置，则手动添加
        if (!hasSlotTypeFlag && modJson.contains("slotType") && modJson["slotType"].is_string()) {
            std::string slotType = modJson["slotType"];

            if (slotType == "STOCK") mod->addFlag(ItemFlag::MOD_STOCK);
            else if (slotType == "BARREL") mod->addFlag(ItemFlag::MOD_BARREL);
            else if (slotType == "UNDER_BARREL") mod->addFlag(ItemFlag::MOD_UNDER_BARREL);
            else if (slotType == "GRIP") mod->addFlag(ItemFlag::MOD_GRIP);
            else if (slotType == "OPTIC") mod->addFlag(ItemFlag::MOD_OPTIC);
            else if (slotType == "SIDE_MOUNT") mod->addFlag(ItemFlag::MOD_SIDE_MOUNT);
            else if (slotType == "RAIL") mod->addFlag(ItemFlag::MOD_RAIL);
            else if (slotType == "MUZZLE") mod->addFlag(ItemFlag::MOD_MUZZLE);
            else if (slotType == "MAGAZINE_WELL") mod->addFlag(ItemFlag::MOD_MAGAZINE_WELL);
            else if (slotType == "SPECIAL") mod->addFlag(ItemFlag::MOD_FLASHLIGHT);
            else {
                std::cerr << "未知配件槽位类型: " << slotType << std::endl;
            }
        }
        
        return mod;
    } catch (const std::exception& e) {
        std::cerr << "加载枪械配件时出错: " << e.what() << std::endl;
        return nullptr;
    }
}

// 从JSON对象加载弹匣
std::unique_ptr<Magazine> ItemLoader::loadMagazineFromJson(const json& magazineJson) {
    try {
        // 检查必要字段
        if (!magazineJson.contains("name") || !magazineJson["name"].is_string()) {
            std::cerr << "弹匣JSON缺少有效的name字段" << std::endl;
            return nullptr;
        }
        
        // 获取基本属性
        std::string name = magazineJson["name"];
        
        // 创建弹匣（使用简化的构造函数）
        auto magazine = std::make_unique<Magazine>(name);
        
        // 设置基本属性
        if (magazineJson.contains("weight")) magazine->setWeight(magazineJson["weight"]);
        if (magazineJson.contains("volume")) magazine->setVolume(magazineJson["volume"]);
        if (magazineJson.contains("length")) magazine->setLength(magazineJson["length"]);
        if (magazineJson.contains("value")) magazine->setValue(magazineJson["value"]);
        
        // 设置弹匣特有属性
        if (magazineJson.contains("capacity")) magazine->setCapacity(magazineJson["capacity"]);
        if (magazineJson.contains("unloadTime")) magazine->setUnloadTime(magazineJson["unloadTime"]);
        if (magazineJson.contains("reloadTime")) magazine->setReloadTime(magazineJson["reloadTime"]);
        
        // 设置兼容弹药类型
        std::vector<std::string> ammoTypes;
        if (magazineJson.contains("compatibleAmmoTypes") && magazineJson["compatibleAmmoTypes"].is_array()) {
            for (const auto& typeJson : magazineJson["compatibleAmmoTypes"]) {
                if (typeJson.is_string()) {
                    ammoTypes.push_back(typeJson);
                }
            }
        }
        magazine->setCompatibleAmmoTypes(ammoTypes);
        
        // 加载标签
        if (magazineJson.contains("flags") && magazineJson["flags"].is_array()) {
            loadItemFlags(magazine.get(), magazineJson["flags"]);
        }
        
        return magazine;
    } catch (const std::exception& e) {
        std::cerr << "加载弹匣时出错: " << e.what() << std::endl;
        return nullptr;
    }
}

// 从JSON对象加载弹药
std::unique_ptr<Ammo> ItemLoader::loadAmmoFromJson(const json& ammoJson) {
    try {
        // 检查必要字段
        if (!ammoJson.contains("name") || !ammoJson["name"].is_string()) {
            std::cerr << "弹药JSON缺少有效的name字段" << std::endl;
            return nullptr;
        }
        
        // 获取基本属性
        std::string name = ammoJson["name"];
        
        // 创建弹药（使用简化的构造函数）
        auto ammo = std::make_unique<Ammo>(name);
        
        // 设置基本属性
        if (ammoJson.contains("weight")) ammo->setWeight(ammoJson["weight"]);
        if (ammoJson.contains("volume")) ammo->setVolume(ammoJson["volume"]);
        if (ammoJson.contains("length")) ammo->setLength(ammoJson["length"]);
        if (ammoJson.contains("value")) ammo->setValue(ammoJson["value"]);
        
        // 设置弹药特有属性
        if (ammoJson.contains("damage")) ammo->setBaseDamage(ammoJson["damage"]);
        if (ammoJson.contains("penetration")) ammo->setBasePenetration(ammoJson["penetration"]);
        if (ammoJson.contains("range")) ammo->setBaseRange(ammoJson["range"]);
        if (ammoJson.contains("speed")) ammo->setBaseSpeed(ammoJson["speed"]);
        if (ammoJson.contains("ammoType")) ammo->setAmmoType(ammoJson["ammoType"]);
        
        // 设置弹药对枪械的修正值
        if (ammoJson.contains("recoilMod")) ammo->setModRecoil(ammoJson["recoilMod"]);
        if (ammoJson.contains("accuracyMod")) ammo->setModAccuracyMOA(ammoJson["accuracyMod"]);
        if (ammoJson.contains("ergoMod")) ammo->setModErgonomics(ammoJson["ergoMod"]);
        
        // 加载标签
        if (ammoJson.contains("flags") && ammoJson["flags"].is_array()) {
            loadItemFlags(ammo.get(), ammoJson["flags"]);
        }
        
        return ammo;
    } catch (const std::exception& e) {
        std::cerr << "加载弹药时出错: " << e.what() << std::endl;
        return nullptr;
    }
}

// 辅助方法：从JSON加载物品标签
void ItemLoader::loadItemFlags(Item* item, const json& flagsJson) {
    // 使用静态映射表存储字符串到ItemFlag的映射关系
    static const std::unordered_map<std::string, ItemFlag> flagMap = {
        // 基本属性标签
        {"WEARABLE", ItemFlag::WEARABLE},
        {"STACKABLE", ItemFlag::STACKABLE},
        {"CONSUMABLE", ItemFlag::CONSUMABLE},
        {"CONTAINER", ItemFlag::CONTAINER},
        {"SINGLE_SLOT", ItemFlag::SINGLE_SLOT},
        {"EXPANDS_WITH_CONTENTS", ItemFlag::EXPANDS_WITH_CONTENTS},
        
        // 类别标签
        {"ARMOR", ItemFlag::ARMOR},
        {"FOOD", ItemFlag::FOOD},
        {"MEDICAL", ItemFlag::MEDICAL},
        {"TOOL", ItemFlag::TOOL},
        {"MISC", ItemFlag::MISC},
        
        // 新增标志
        {"ONLY_ARMOR_PLATE", ItemFlag::ONLY_ARMOR_PLATE},
        {"USES_POWER", ItemFlag::USES_POWER},
        {"ARMOR_PLATE", ItemFlag::ARMOR_PLATE},
        {"STRENGTH_BOOST", ItemFlag::STRENGTH_BOOST},
        {"HEAVY", ItemFlag::HEAVY},
        
        // 稀有度标签
        {"COMMON", ItemFlag::COMMON},
        {"RARE", ItemFlag::RARE},
        {"EPIC", ItemFlag::EPIC},
        {"LEGENDARY", ItemFlag::LEGENDARY},
        {"MYTHIC", ItemFlag::MYTHIC},
        
        // 装备槽位标签
        {"SLOT_HEAD", ItemFlag::SLOT_HEAD},
        {"SLOT_CHEST", ItemFlag::SLOT_CHEST},
        {"SLOT_ABDOMEN", ItemFlag::SLOT_ABDOMEN},
        {"SLOT_LEFT_LEG", ItemFlag::SLOT_LEFT_LEG},
        {"SLOT_RIGHT_LEG", ItemFlag::SLOT_RIGHT_LEG},
        {"SLOT_LEFT_FOOT", ItemFlag::SLOT_LEFT_FOOT},
        {"SLOT_RIGHT_FOOT", ItemFlag::SLOT_RIGHT_FOOT},
        {"SLOT_LEFT_ARM", ItemFlag::SLOT_LEFT_ARM},
        {"SLOT_RIGHT_ARM", ItemFlag::SLOT_RIGHT_ARM},
        {"SLOT_LEFT_HAND", ItemFlag::SLOT_LEFT_HAND},
        {"SLOT_RIGHT_HAND", ItemFlag::SLOT_RIGHT_HAND},
        {"SLOT_BACK", ItemFlag::SLOT_BACK},
        
        // 武器类型标签
        {"WEAPON", ItemFlag::WEAPON},
        {"GUN", ItemFlag::GUN},
        {"MELEE", ItemFlag::MELEE},
        {"THROWABLE", ItemFlag::THROWABLE},
        {"GUNMOD", ItemFlag::GUNMOD},
        
        // 近战武器子类型标签
        {"SWORD", ItemFlag::SWORD},
        {"AXE", ItemFlag::AXE},
        {"HAMMER", ItemFlag::HAMMER},
        {"SPEAR", ItemFlag::SPEAR},
        {"DAGGER", ItemFlag::DAGGER},
        
        // 枪械类型标签
        {"PISTOL", ItemFlag::PISTOL},
        {"REVOLVER", ItemFlag::REVOLVER},
        {"SHOTGUN", ItemFlag::SHOTGUN},
        {"SMG", ItemFlag::SMG},
        {"RIFLE", ItemFlag::RIFLE},
        {"DMR", ItemFlag::DMR},
        {"SNIPER_RIFLE", ItemFlag::SNIPER_RIFLE},
        {"MACHINE_GUN", ItemFlag::MACHINE_GUN},
        {"GRENADE_LAUNCHER", ItemFlag::GRENADE_LAUNCHER},
        
        // 弹药相关标签
        {"MAGAZINE", ItemFlag::MAGAZINE},
        {"AMMO", ItemFlag::AMMO},
        
        // 射击模式标签
        {"SEMI_AUTO", ItemFlag::SEMI_AUTO},
        {"FULL_AUTO", ItemFlag::FULL_AUTO},
        {"BOLT_ACTION", ItemFlag::BOLT_ACTION},
        {"BURST", ItemFlag::BURST},
        
        // 枪械配件槽位标签
        {"GUN_MOD", ItemFlag::GUN_MOD},
        {"MOD_STOCK", ItemFlag::MOD_STOCK},
        {"MOD_BARREL", ItemFlag::MOD_BARREL},
        {"MOD_UNDER_BARREL", ItemFlag::MOD_UNDER_BARREL},
        {"MOD_GRIP", ItemFlag::MOD_GRIP},
        {"MOD_OPTIC", ItemFlag::MOD_OPTIC},
        {"MOD_SIDE_MOUNT", ItemFlag::MOD_SIDE_MOUNT},
        {"MOD_MUZZLE", ItemFlag::MOD_MUZZLE},
        {"MOD_MAGAZINE_WELL", ItemFlag::MOD_MAGAZINE_WELL},
        {"MOD_RAIL", ItemFlag::MOD_RAIL},
        {"MOD_LASER", ItemFlag::MOD_LASER},
        {"MOD_FLASHLIGHT", ItemFlag::MOD_FLASHLIGHT},
        
        // 其他特性标签
        {"SILENCED", ItemFlag::SILENCED},
        {"SCOPE", ItemFlag::SCOPE},
        {"LASER", ItemFlag::LASER},
        {"FLASHLIGHT", ItemFlag::FLASHLIGHT}
    };
    
    for (const auto& flagJson : flagsJson) {
        if (flagJson.is_string()) {
            std::string flagStr = flagJson;
            
            // 使用映射表查找对应的ItemFlag
            auto it = flagMap.find(flagStr);
            if (it != flagMap.end()) {
                // 找到对应的标签，添加到物品中
                item->addFlag(it->second);
                std::cout << "为物品 " << item->getName() << " 添加标签: " << flagStr << std::endl;
            } else {
                // 记录未知标签，但不影响加载过程
                std::cerr << "未知物品标签: " << flagStr << " (物品: " << item->getName() << ")" << std::endl;
            }
        }
    }
}

// 注意：物品类别现在通过ItemFlag处理，不再使用ItemCategory

// 辅助方法：从JSON加载装备槽位
void ItemLoader::loadEquipSlots(Item* item, const json& slotsJson) {
    // 先清空物品的装备槽位，移除默认的NONE槽位
    // 注意：这里我们不能直接访问item的equipSlots成员，因为它是私有的
    // 所以我们需要先移除所有现有槽位，然后添加新的槽位
    
    // 获取当前所有槽位并创建一个副本，避免在迭代过程中修改集合
    std::vector<EquipSlot> currentSlots = item->getEquipSlots();
    
    // 移除所有现有槽位
    for (const auto& slot : currentSlots) {
        item->removeEquipSlot(slot);
    }
    
    // 添加JSON中指定的槽位
    for (const auto& slotJson : slotsJson) {
        if (slotJson.is_string()) {
            std::string slotStr = slotJson;
            
            // 将字符串转换为EquipSlot枚举
            if (slotStr == "NONE") item->addEquipSlot(EquipSlot::NONE);
            else if (slotStr == "HEAD") item->addEquipSlot(EquipSlot::HEAD);
            else if (slotStr == "EYES") item->addEquipSlot(EquipSlot::EYES);
            else if (slotStr == "CHEST") item->addEquipSlot(EquipSlot::CHEST);
            else if (slotStr == "ABDOMEN") item->addEquipSlot(EquipSlot::ABDOMEN);
            else if (slotStr == "LEFT_LEG") item->addEquipSlot(EquipSlot::LEFT_LEG);
            else if (slotStr == "RIGHT_LEG") item->addEquipSlot(EquipSlot::RIGHT_LEG);
            else if (slotStr == "LEFT_FOOT") item->addEquipSlot(EquipSlot::LEFT_FOOT);
            else if (slotStr == "RIGHT_FOOT") item->addEquipSlot(EquipSlot::RIGHT_FOOT);
            else if (slotStr == "LEFT_ARM") item->addEquipSlot(EquipSlot::LEFT_ARM);
            else if (slotStr == "RIGHT_ARM") item->addEquipSlot(EquipSlot::RIGHT_ARM);
            else if (slotStr == "LEFT_HAND") item->addEquipSlot(EquipSlot::LEFT_HAND);
            else if (slotStr == "RIGHT_HAND") item->addEquipSlot(EquipSlot::RIGHT_HAND);
            else if (slotStr == "BACK") item->addEquipSlot(EquipSlot::BACK);
        }
    }
}

// 辅助方法：从JSON加载覆盖率信息
void ItemLoader::loadCoverageSlots(Item* item, const json& coverageJson) {
    // coverageJson可以是三种格式：
    // 1. 数组格式：["CHEST", "ABDOMEN"] - 默认100%覆盖率，0累赘值
    // 2. 对象格式（简单）：{"CHEST": 80, "ABDOMEN": 90} - 指定覆盖率，0累赘值
    // 3. 对象格式（完整）：{"CHEST": {"coverage": 80, "burden": 5}, "ABDOMEN": {"coverage": 90, "burden": 3}}
    
    if (coverageJson.is_array()) {
        // 数组格式：默认100%覆盖率，0累赘值
        for (const auto& slotJson : coverageJson) {
            if (slotJson.is_string()) {
                std::string slotStr = slotJson;
                EquipSlot slot = stringToEquipSlot(slotStr);
                if (slot != EquipSlot::NONE) {
                    item->addCoverageSlot(slot, 100, 0); // 默认100%覆盖率，0累赘值
                }
            }
        }
    } else if (coverageJson.is_object()) {
        // 对象格式：指定覆盖率和累赘值
        for (auto it = coverageJson.begin(); it != coverageJson.end(); ++it) {
            std::string slotStr = it.key();
            EquipSlot slot = stringToEquipSlot(slotStr);
            
            if (slot != EquipSlot::NONE) {
                if (it.value().is_number()) {
                    // 简单格式：只有覆盖率
                    int coverage = it.value();
                    item->addCoverageSlot(slot, coverage, 0); // 默认累赘值为0
                } else if (it.value().is_object()) {
                    // 完整格式：包含覆盖率和累赘值
                    const json& slotData = it.value();
                    int coverage = 100; // 默认覆盖率
                    int burden = 0;     // 默认累赘值
                    
                    if (slotData.contains("coverage") && slotData["coverage"].is_number()) {
                        coverage = slotData["coverage"];
                    }
                    if (slotData.contains("burden") && slotData["burden"].is_number()) {
                        burden = slotData["burden"];
                    }
                    
                    item->addCoverageSlot(slot, coverage, burden);
                }
            }
        }
    }
}

// 辅助方法：从JSON加载防护数据
void ItemLoader::loadProtectionData(Item* item, const json& protectionJson) {
    // protectionJson格式：
    // {
    //     "CHEST": {
    //         "钝击": 25,
    //         "斩击": 30,
    //         "刺击": 20,
    //         "射击": 40
    //     },
    //     "HEAD": {
    //         "钝击": 35,
    //         "射击": 45
    //     }
    // }
    
    if (!protectionJson.is_object()) {
        return;
    }
    
    for (auto it = protectionJson.begin(); it != protectionJson.end(); ++it) {
        std::string bodyPartStr = it.key();
        EquipSlot bodyPart = stringToEquipSlot(bodyPartStr);
        
        if (bodyPart != EquipSlot::NONE && it.value().is_object()) {
            // 为该身体部位添加防护数据
            item->addProtectionData(bodyPart);
            
            // 加载各种伤害类型的防护值
            const json& damageTypeProtections = it.value();
            for (auto damageIt = damageTypeProtections.begin(); damageIt != damageTypeProtections.end(); ++damageIt) {
                std::string damageTypeStr = damageIt.key();
                
                if (damageIt.value().is_number()) {
                    int protectionValue = damageIt.value();
                    DamageType damageType = stringToDamageType(damageTypeStr);
                    item->setProtection(bodyPart, damageType, protectionValue);
                }
            }
        }
    }
}

// 辅助方法：将字符串转换为EquipSlot枚举
EquipSlot ItemLoader::stringToEquipSlot(const std::string& slotStr) {
    if (slotStr == "NONE") return EquipSlot::NONE;
    else if (slotStr == "HEAD") return EquipSlot::HEAD;
    else if (slotStr == "EYES") return EquipSlot::EYES;
    else if (slotStr == "EYES") return EquipSlot::EYES;
    else if (slotStr == "CHEST") return EquipSlot::CHEST;
    else if (slotStr == "ABDOMEN") return EquipSlot::ABDOMEN;
    else if (slotStr == "LEFT_LEG") return EquipSlot::LEFT_LEG;
    else if (slotStr == "RIGHT_LEG") return EquipSlot::RIGHT_LEG;
    else if (slotStr == "LEFT_FOOT") return EquipSlot::LEFT_FOOT;
    else if (slotStr == "RIGHT_FOOT") return EquipSlot::RIGHT_FOOT;
    else if (slotStr == "LEFT_ARM") return EquipSlot::LEFT_ARM;
    else if (slotStr == "RIGHT_ARM") return EquipSlot::RIGHT_ARM;
    else if (slotStr == "LEFT_HAND") return EquipSlot::LEFT_HAND;
    else if (slotStr == "RIGHT_HAND") return EquipSlot::RIGHT_HAND;
    else if (slotStr == "BACK") return EquipSlot::BACK;
    else return EquipSlot::NONE;
}

// 从JSON对象加载武器
std::unique_ptr<Weapon> ItemLoader::loadWeaponFromJson(const json& weaponJson) {
    try {
        // 检查必要字段
        if (!weaponJson.contains("name") || !weaponJson["name"].is_string()) {
            std::cerr << "武器JSON缺少有效的name字段" << std::endl;
            return nullptr;
        }
        
        // 获取基本属性
        std::string name = weaponJson["name"];
        
        // 创建武器
        auto weapon = std::make_unique<Weapon>(name);
        
        // 设置基本属性
        if (weaponJson.contains("weight")) weapon->setWeight(weaponJson["weight"]);
        if (weaponJson.contains("volume")) weapon->setVolume(weaponJson["volume"]);
        if (weaponJson.contains("length")) weapon->setLength(weaponJson["length"]);
        if (weaponJson.contains("value")) weapon->setValue(weaponJson["value"]);
        if (weaponJson.contains("description")) weapon->setDescription(weaponJson["description"]);
        
        // 设置武器类型
        if (weaponJson.contains("weaponType")) {
            std::string typeStr = weaponJson["weaponType"];
            if (typeStr == "MELEE") weapon->setWeaponType(WeaponType::MELEE);
            else if (typeStr == "RANGED") weapon->setWeaponType(WeaponType::RANGED);
            else if (typeStr == "THROWN") weapon->setWeaponType(WeaponType::THROWN);
            else if (typeStr == "SPECIAL") weapon->setWeaponType(WeaponType::SPECIAL);
        }
        
        // 设置主要攻击方式
        if (weaponJson.contains("primaryAttackMethod")) {
            std::string methodStr = weaponJson["primaryAttackMethod"];
            AttackMethod method = parseAttackMethod(methodStr);
            weapon->setPrimaryAttackMethod(method);
        }
        
        // 设置可用攻击方式
        if (weaponJson.contains("availableAttackMethods") && weaponJson["availableAttackMethods"].is_array()) {
            for (const auto& methodJson : weaponJson["availableAttackMethods"]) {
                if (methodJson.is_string()) {
                    AttackMethod method = parseAttackMethod(methodJson);
                    weapon->addAttackMethod(method);
                }
            }
        }
        
        // 设置武器属性
        if (weaponJson.contains("damage")) weapon->setBaseDamage(weaponJson["damage"]);
        if (weaponJson.contains("range")) weapon->setRange(weaponJson["range"]);
        if (weaponJson.contains("attackSpeed")) weapon->setAttackSpeed(weaponJson["attackSpeed"]);
        if (weaponJson.contains("criticalChance")) weapon->setCriticalChance(weaponJson["criticalChance"]);
        if (weaponJson.contains("criticalMultiplier")) weapon->setCriticalMultiplier(weaponJson["criticalMultiplier"]);
        if (weaponJson.contains("accuracy")) weapon->setAccuracy(weaponJson["accuracy"]);
        if (weaponJson.contains("penetration")) weapon->setPenetration(weaponJson["penetration"]);
        
        // 设置耐久度
        if (weaponJson.contains("maxDurability")) weapon->setMaxDurability(weaponJson["maxDurability"]);
        if (weaponJson.contains("durability")) weapon->setCurrentDurability(weaponJson["durability"]);
        
        // 设置连击系统
        if (weaponJson.contains("supportsCombo")) weapon->setSupportsCombo(weaponJson["supportsCombo"]);
        if (weaponJson.contains("maxComboCount")) weapon->setMaxComboCount(weaponJson["maxComboCount"]);
        if (weaponJson.contains("comboWindow")) weapon->setComboWindow(weaponJson["comboWindow"]);
        if (weaponJson.contains("comboDamageBonus")) weapon->setComboDamageBonus(weaponJson["comboDamageBonus"]);
        
        // 设置音效
        if (weaponJson.contains("attackSound")) weapon->setAttackSound(weaponJson["attackSound"]);
        if (weaponJson.contains("hitSound")) weapon->setHitSound(weaponJson["hitSound"]);
        if (weaponJson.contains("criticalSound")) weapon->setCriticalSound(weaponJson["criticalSound"]);
        if (weaponJson.contains("comboSound")) weapon->setComboSound(weaponJson["comboSound"]);
        
        // 设置动画
        if (weaponJson.contains("animationSpeed")) weapon->setAnimationSpeed(weaponJson["animationSpeed"]);
        if (weaponJson.contains("animationName")) weapon->setAnimationName(weaponJson["animationName"]);
        
        // 设置需求属性
        if (weaponJson.contains("requiredStrength")) weapon->setRequiredStrength(weaponJson["requiredStrength"]);
        if (weaponJson.contains("requiredDexterity")) weapon->setRequiredDexterity(weaponJson["requiredDexterity"]);
        if (weaponJson.contains("requiredIntelligence")) weapon->setRequiredIntelligence(weaponJson["requiredIntelligence"]);
        
        // 加载特殊效果
        if (weaponJson.contains("specialEffects") && weaponJson["specialEffects"].is_array()) {
            for (const auto& effectJson : weaponJson["specialEffects"]) {
                SpecialEffect effect = loadSpecialEffectFromJson(effectJson);
                weapon->addSpecialEffect(effect);
            }
        }
        
        // 设置装备槽位
        if (weaponJson.contains("equipSlots") && weaponJson["equipSlots"].is_array()) {
            loadEquipSlots(weapon.get(), weaponJson["equipSlots"]);
        }
        
        // 加载标签
        if (weaponJson.contains("flags") && weaponJson["flags"].is_array()) {
            loadItemFlags(weapon.get(), weaponJson["flags"]);
        }
        
        return weapon;
    } catch (const std::exception& e) {
        std::cerr << "加载武器时出错: " << e.what() << std::endl;
        return nullptr;
    }
}

// 从JSON对象加载近战武器
std::unique_ptr<MeleeWeapon> ItemLoader::loadMeleeWeaponFromJson(const json& weaponJson) {
    try {
        // 检查必要字段
        if (!weaponJson.contains("name") || !weaponJson["name"].is_string()) {
            std::cerr << "近战武器JSON缺少有效的name字段" << std::endl;
            return nullptr;
        }
        
        // 获取基本属性
        std::string name = weaponJson["name"];
        
        // 创建近战武器
        auto meleeWeapon = std::make_unique<MeleeWeapon>(name);
        
        // 设置基本Item属性
        if (weaponJson.contains("weight")) meleeWeapon->setWeight(weaponJson["weight"]);
        if (weaponJson.contains("volume")) meleeWeapon->setVolume(weaponJson["volume"]);
        if (weaponJson.contains("length")) meleeWeapon->setLength(weaponJson["length"]);
        if (weaponJson.contains("value")) meleeWeapon->setValue(weaponJson["value"]);
        if (weaponJson.contains("description")) meleeWeapon->setDescription(weaponJson["description"]);
        
        // 设置伤害属性（用于MeleeWeapon的getWeaponDamage等方法）
        if (weaponJson.contains("damage")) {
            // 根据武器类型设置对应的伤害属性
            float damage = weaponJson["damage"];
            
            // 默认设置斩击伤害，具体类型将由flag控制
            meleeWeapon->setSlashingDamage(static_cast<int>(damage));
        }
        
        // 设置攻击时间（用于速度计算）
        if (weaponJson.contains("attackSpeed")) {
            float attackSpeed = weaponJson["attackSpeed"];
            meleeWeapon->setAttackTime(1.0f / attackSpeed); // 转换为秒
        }
        
        // 武器只有在手持时才发挥作用，不设置默认装备槽位
        
        // 设置装备槽位
        if (weaponJson.contains("equipSlots") && weaponJson["equipSlots"].is_array()) {
            loadEquipSlots(meleeWeapon.get(), weaponJson["equipSlots"]);
        }
        
        // 加载标签（这是关键，决定武器类型和特效）
        if (weaponJson.contains("flags") && weaponJson["flags"].is_array()) {
            loadItemFlags(meleeWeapon.get(), weaponJson["flags"]);
        }
        
        // 加载攻击模式配置
        if (weaponJson.contains("attackModes") && weaponJson["attackModes"].is_object()) {
            loadAttackModes(meleeWeapon.get(), weaponJson["attackModes"]);
        }
        
        return meleeWeapon;
    } catch (const std::exception& e) {
        std::cerr << "加载近战武器时出错: " << e.what() << std::endl;
        return nullptr;
    }
}

// 创建近战武器实例
std::unique_ptr<MeleeWeapon> ItemLoader::createMeleeWeapon(const std::string& weaponName) {
    if (!hasItemTemplate(weaponName)) {
        std::cerr << "未找到近战武器模板: " << weaponName << std::endl;
        return nullptr;
    }
    
    // 检查模板是否为MeleeWeapon类型
    const MeleeWeapon* templateWeapon = dynamic_cast<const MeleeWeapon*>(itemTemplates[weaponName].get());
    if (!templateWeapon) {
        std::cerr << "物品不是近战武器类型: " << weaponName << std::endl;
        return nullptr;
    }
    
    // 创建新的近战武器（深拷贝）
    return std::unique_ptr<MeleeWeapon>(dynamic_cast<MeleeWeapon*>(templateWeapon->clone()));
}

// 创建武器实例
std::unique_ptr<Weapon> ItemLoader::createWeapon(const std::string& weaponName) {
    if (!hasWeaponTemplate(weaponName)) {
        std::cerr << "未找到武器模板: " << weaponName << std::endl;
        return nullptr;
    }
    
    // 创建新武器（深拷贝）
    const Weapon* templateWeapon = weaponTemplates[weaponName].get();
    return std::unique_ptr<Weapon>(dynamic_cast<Weapon*>(templateWeapon->clone()));
}

// 检查是否存在武器模板
bool ItemLoader::hasWeaponTemplate(const std::string& weaponName) const {
    return weaponTemplates.find(weaponName) != weaponTemplates.end();
}

// 辅助方法：解析攻击方式
AttackMethod ItemLoader::parseAttackMethod(const std::string& methodStr) {
    if (methodStr == "SLASH") return AttackMethod::MELEE_SLASH;
    if (methodStr == "STAB") return AttackMethod::MELEE_STAB;
    if (methodStr == "HEAVY_ATTACK") return AttackMethod::MELEE_CRUSH;
    if (methodStr == "QUICK_ATTACK") return AttackMethod::MELEE_QUICK;
    if (methodStr == "RANGED_ATTACK") return AttackMethod::RANGED_SHOOT;
    if (methodStr == "THROW") return AttackMethod::RANGED_THROW;
    if (methodStr == "SPECIAL_ABILITY") return AttackMethod::SPECIAL_ABILITY;
    
    return AttackMethod::MELEE_SLASH; // 默认值
}

// 辅助方法：从JSON加载特殊效果
SpecialEffect ItemLoader::loadSpecialEffectFromJson(const json& effectJson) {
    SpecialEffect effect;
    
    if (effectJson.contains("type") && effectJson["type"].is_string()) {
        effect.type = SpecialEffectManager::parseEffectType(effectJson["type"]);
    }
    
    if (effectJson.contains("chance")) effect.chance = effectJson["chance"];
    if (effectJson.contains("duration")) effect.duration = effectJson["duration"];
    if (effectJson.contains("magnitude")) effect.magnitude = effectJson["magnitude"];
    
    if (effectJson.contains("customName") && effectJson["customName"].is_string()) {
        effect.customName = effectJson["customName"];
    }
    
    if (effectJson.contains("parameters") && effectJson["parameters"].is_object()) {
        for (auto& [key, value] : effectJson["parameters"].items()) {
            if (value.is_number()) {
                effect.parameters[key] = value;
            }
        }
    }
    
    return effect;
}

// 从JSON加载攻击模式配置
void ItemLoader::loadAttackModes(MeleeWeapon* weapon, const json& attackModesJson) {
    try {
        for (const auto& [modeKey, modeJson] : attackModesJson.items()) {
            AttackModeConfig config;
            
            // 加载基础参数
            if (modeJson.contains("shape") && modeJson["shape"].is_string()) {
                config.shape = modeJson["shape"];
            }
            
            if (modeJson.contains("angle") && modeJson["angle"].is_number()) {
                config.angle = modeJson["angle"];
            }
            
            if (modeJson.contains("range") && modeJson["range"].is_number()) {
                config.range = modeJson["range"];
            }
            
            if (modeJson.contains("width") && modeJson["width"].is_number()) {
                config.width = modeJson["width"];
            }
            
            if (modeJson.contains("damageMultiplier") && modeJson["damageMultiplier"].is_number()) {
                config.damageMultiplier = modeJson["damageMultiplier"];
            }
            
            // 加载特殊效果
            if (modeJson.contains("effects") && modeJson["effects"].is_array()) {
                for (const auto& effectJson : modeJson["effects"]) {
                    AttackEffectConfig effectConfig;
                    
                    if (effectJson.contains("type") && effectJson["type"].is_string()) {
                        effectConfig.type = effectJson["type"];
                    }
                    
                    if (effectJson.contains("chance") && effectJson["chance"].is_number()) {
                        effectConfig.chance = effectJson["chance"];
                    }
                    
                    if (effectJson.contains("duration") && effectJson["duration"].is_number()) {
                        effectConfig.duration = effectJson["duration"];
                    }
                    
                    if (effectJson.contains("magnitude") && effectJson["magnitude"].is_number()) {
                        effectConfig.magnitude = effectJson["magnitude"];
                    }
                    
                    config.effects.push_back(effectConfig);
                }
            }
            
            // 设置攻击模式
            weapon->setAttackMode(modeKey, config);
        }
    } catch (const std::exception& e) {
        std::cerr << "加载攻击模式配置时出错: " << e.what() << std::endl;
    }
}