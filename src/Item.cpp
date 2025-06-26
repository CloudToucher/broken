#include "Item.h"
#include "Storage.h"
#include <algorithm>
#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>

// 更新构造函数，简化参数列表
Item::Item(const std::string& itemName, 
           float itemWeight, float itemVolume, float itemLength, 
           int itemValue)
    : name(itemName), 
      weight(itemWeight), 
      volume(itemVolume), 
      length(itemLength), 
      value(itemValue),
      wearable(false),
      rarity(ItemRarity::COMMON),
      description(""),
      piercingDamage(0),
      bluntDamage(0),
      slashingDamage(0),
      attackTime(1.0f),
      staminaCost(0),
      activationCost(0),
      piercingDefense(0.0f),
      bluntDefense(0.0f),
      slashingDefense(0.0f),
      bulletDefense(0.0f),
      usesRemaining(1.0f),
      uniqueId("") {
    // 不再默认添加NONE槽位，让物品初始没有装备槽位
    // 装备槽位将通过addEquipSlot方法添加
    
    // 生成唯一ID
    generateUniqueId();
}

// 实现析构函数
Item::~Item() {
    // 存储空间会在unique_ptr被销毁时自动清理
}

// 实现拷贝构造函数
Item::Item(const Item& other)
    : name(other.name),
      weight(other.weight),
      volume(other.volume),
      length(other.length),
      value(other.value),
      wearable(other.wearable),
      equipSlots(other.equipSlots),
      coverageSlots(other.coverageSlots),
      rarity(other.rarity),
      flags(other.flags),
      description(other.description),
      piercingDamage(other.piercingDamage),
      bluntDamage(other.bluntDamage),
      slashingDamage(other.slashingDamage),
      attackTime(other.attackTime),
      staminaCost(other.staminaCost),
      activationCost(other.activationCost),
      piercingDefense(other.piercingDefense),
      bluntDefense(other.bluntDefense),
      slashingDefense(other.slashingDefense),
      bulletDefense(other.bulletDefense),
      usesRemaining(other.usesRemaining) {
    // 深拷贝存储空间
    for (const auto& storage : other.storages) {
        if (storage) {
            // 创建Storage的深拷贝
            Storage* storageCopy = new Storage(*storage);
            storages.push_back(std::unique_ptr<Storage>(storageCopy));
        }
    }
    
    // 生成新的唯一ID，而不是复制原物品的ID
    generateUniqueId();
}

// 实现拷贝赋值运算符
Item& Item::operator=(const Item& other) {
    if (this != &other) {
        name = other.name;
        weight = other.weight;
        volume = other.volume;
        length = other.length;
        value = other.value;
        wearable = other.wearable;
        equipSlots = other.equipSlots;
        coverageSlots = other.coverageSlots;
        rarity = other.rarity;
        flags = other.flags;
        description = other.description;
        piercingDamage = other.piercingDamage;
        bluntDamage = other.bluntDamage;
        slashingDamage = other.slashingDamage;
        attackTime = other.attackTime;
        staminaCost = other.staminaCost;
        activationCost = other.activationCost;
        piercingDefense = other.piercingDefense;
        bluntDefense = other.bluntDefense;
        slashingDefense = other.slashingDefense;
        bulletDefense = other.bulletDefense;
        usesRemaining = other.usesRemaining;
        
        // 清空当前存储空间
        storages.clear();
        
        // 深拷贝存储空间
        for (const auto& storage : other.storages) {
            if (storage) {
                // 创建Storage的深拷贝
                Storage* storageCopy = new Storage(*storage);
                storages.push_back(std::unique_ptr<Storage>(storageCopy));
            }
        }
        
        // 生成新的唯一ID，而不是复制原物品的ID
        generateUniqueId();
    }
    return *this;
}

// 实现生成唯一ID的方法
void Item::generateUniqueId() {
    // 如果已经有ID，不重新生成
    if (!uniqueId.empty()) {
        return;
    }
    
    // 使用当前时间和随机数生成唯一ID
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto epoch = now_ms.time_since_epoch();
    auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count();
    
    // 创建随机数生成器
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 0xFFFF);
    
    // 生成ID格式：时间戳-随机数-物品名称哈希
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << value;
    ss << "-";
    ss << std::hex << std::setw(4) << std::setfill('0') << dis(gen);
    ss << "-";
    
    // 使用物品名称的哈希值作为ID的一部分
    std::hash<std::string> hasher;
    size_t nameHash = hasher(name);
    ss << std::hex << std::setw(8) << std::setfill('0') << nameHash;
    
    uniqueId = ss.str();
}

// 标签相关方法实现
void Item::addFlag(ItemFlag flag) {
    flags.insert(flag);
    // 添加标签后立即处理其效果
    processFlags();
}

void Item::addFlags(const std::vector<ItemFlag>& flagList) {
    for (const auto& flag : flagList) {
        flags.insert(flag);
    }
    // 批量添加标签后处理效果
    processFlags();
}

bool Item::hasFlag(ItemFlag flag) const {
    return flags.find(flag) != flags.end();
}

void Item::removeFlag(ItemFlag flag) {
    flags.erase(flag);
    // 移除标签后重新处理效果
    processFlags();
}

// 根据标签处理物品属性
void Item::processFlags() {
    // 重置可穿戴属性，但不清空装备槽位
    wearable = false;
    
    // 处理稀有度标签
    if (hasFlag(ItemFlag::COMMON)) rarity = ItemRarity::COMMON;
    if (hasFlag(ItemFlag::RARE)) rarity = ItemRarity::RARE;
    if (hasFlag(ItemFlag::EPIC)) rarity = ItemRarity::EPIC;
    if (hasFlag(ItemFlag::LEGENDARY)) rarity = ItemRarity::LEGENDARY;
    if (hasFlag(ItemFlag::MYTHIC)) rarity = ItemRarity::MYTHIC;
    
    // 处理可穿戴标签
    if (hasFlag(ItemFlag::WEARABLE)) {
        wearable = true;
    }
    
    // 处理身体部位标签 - 只添加不存在的槽位，不清空现有槽位
    if (hasFlag(ItemFlag::SLOT_HEAD) && std::find(equipSlots.begin(), equipSlots.end(), EquipSlot::HEAD) == equipSlots.end()) 
        equipSlots.push_back(EquipSlot::HEAD);
    if (hasFlag(ItemFlag::SLOT_CHEST) && std::find(equipSlots.begin(), equipSlots.end(), EquipSlot::CHEST) == equipSlots.end()) 
        equipSlots.push_back(EquipSlot::CHEST);
    if (hasFlag(ItemFlag::SLOT_ABDOMEN) && std::find(equipSlots.begin(), equipSlots.end(), EquipSlot::ABDOMEN) == equipSlots.end()) 
        equipSlots.push_back(EquipSlot::ABDOMEN);
    if (hasFlag(ItemFlag::SLOT_LEFT_LEG) && std::find(equipSlots.begin(), equipSlots.end(), EquipSlot::LEFT_LEG) == equipSlots.end()) 
        equipSlots.push_back(EquipSlot::LEFT_LEG);
    if (hasFlag(ItemFlag::SLOT_RIGHT_LEG) && std::find(equipSlots.begin(), equipSlots.end(), EquipSlot::RIGHT_LEG) == equipSlots.end()) 
        equipSlots.push_back(EquipSlot::RIGHT_LEG);
    if (hasFlag(ItemFlag::SLOT_LEFT_FOOT) && std::find(equipSlots.begin(), equipSlots.end(), EquipSlot::LEFT_FOOT) == equipSlots.end()) 
        equipSlots.push_back(EquipSlot::LEFT_FOOT);
    if (hasFlag(ItemFlag::SLOT_RIGHT_FOOT) && std::find(equipSlots.begin(), equipSlots.end(), EquipSlot::RIGHT_FOOT) == equipSlots.end()) 
        equipSlots.push_back(EquipSlot::RIGHT_FOOT);
    if (hasFlag(ItemFlag::SLOT_LEFT_ARM) && std::find(equipSlots.begin(), equipSlots.end(), EquipSlot::LEFT_ARM) == equipSlots.end()) 
        equipSlots.push_back(EquipSlot::LEFT_ARM);
    if (hasFlag(ItemFlag::SLOT_RIGHT_ARM) && std::find(equipSlots.begin(), equipSlots.end(), EquipSlot::RIGHT_ARM) == equipSlots.end()) 
        equipSlots.push_back(EquipSlot::RIGHT_ARM);
    if (hasFlag(ItemFlag::SLOT_LEFT_HAND) && std::find(equipSlots.begin(), equipSlots.end(), EquipSlot::LEFT_HAND) == equipSlots.end()) 
        equipSlots.push_back(EquipSlot::LEFT_HAND);
    if (hasFlag(ItemFlag::SLOT_RIGHT_HAND) && std::find(equipSlots.begin(), equipSlots.end(), EquipSlot::RIGHT_HAND) == equipSlots.end()) 
        equipSlots.push_back(EquipSlot::RIGHT_HAND);
    if (hasFlag(ItemFlag::SLOT_BACK) && std::find(equipSlots.begin(), equipSlots.end(), EquipSlot::BACK) == equipSlots.end()) 
        equipSlots.push_back(EquipSlot::BACK);

    
    // 如果有装备槽位但没有可穿戴标签，自动添加可穿戴属性
    if (!equipSlots.empty() && !wearable) {
        wearable = true;
    }
    
    // 处理容器相关标签
    if (hasFlag(ItemFlag::CONTAINER)) {
        // 如果有EXPANDS_WITH_CONTENTS标签，表示容器会显示当前内容物体积（不改变最大容量）
        if (hasFlag(ItemFlag::EXPANDS_WITH_CONTENTS)) {
            // 为所有存储空间设置expandsWithContents属性
            for (auto& storage : storages) {
                if (storage) {
                    storage->setExpandsWithContents(true);
                }
            }
        } else {
            // 确保所有存储空间的expandsWithContents属性为false
            for (auto& storage : storages) {
                if (storage) {
                    storage->setExpandsWithContents(false);
                }
            }
        }
    }
}

// 类别相关方法已通过ItemFlag实现

// 槽位相关方法
void Item::addEquipSlot(EquipSlot slot) {
    if (!canEquipToSlot(slot)) {
        equipSlots.push_back(slot);
        // 添加槽位时自动设置为可穿戴
        wearable = true;
    }
}

bool Item::canEquipToSlot(EquipSlot slot) const {
    if (!wearable) return false;
    return std::find(equipSlots.begin(), equipSlots.end(), slot) != equipSlots.end();
}

void Item::removeEquipSlot(EquipSlot slot) {
    auto it = std::find(equipSlots.begin(), equipSlots.end(), slot);
    if (it != equipSlots.end()) {
        equipSlots.erase(it);
        // 如果没有槽位，设置为不可穿戴
        if (equipSlots.empty()) {
            wearable = false;
        }
    }
}

void Item::addStorage(std::unique_ptr<Storage> storage) {
    if (storage) {
        // 如果物品有EXPANDS_WITH_CONTENTS标志，设置存储空间显示当前内容物体积（不改变最大容量）
        if (hasFlag(ItemFlag::EXPANDS_WITH_CONTENTS)) {
            storage->setExpandsWithContents(true);
        }
        storages.push_back(std::move(storage));
    }
}

size_t Item::getStorageCount() const {
    return storages.size();
}

Storage* Item::getStorage(size_t index) const {
    if (index < storages.size()) {
        return storages[index].get();
    }
    return nullptr;
}

float Item::getTotalWeight() const {
    float total = weight;
    
    // 添加所有存储空间中物品的重量
    for (const auto& storage : storages) {
        total += storage->getCurrentWeight();
    }
    
    return total;
}

// 获取物品的所有标签名称
std::vector<std::string> Item::getFlagNames() const {
    std::vector<std::string> names;
    for (const auto& flag : flags) {
        names.push_back(GetItemFlagName(flag));
    }
    return names;
}

// 新的覆盖率相关方法实现
void Item::addCoverageSlot(EquipSlot slot, int coverage) {
    // 确保覆盖率在0-100范围内
    coverage = std::max(0, std::min(100, coverage));
    
    // 检查是否已存在该槽位
    for (auto& slotCoverage : coverageSlots) {
        if (slotCoverage.slot == slot) {
            slotCoverage.coverage = coverage;
            return;
        }
    }
    
    // 添加新的槽位覆盖
    coverageSlots.emplace_back(slot, coverage);
    
    // 自动设置为可穿戴
    wearable = true;
    
    // 为了向后兼容，也添加到equipSlots中
    if (std::find(equipSlots.begin(), equipSlots.end(), slot) == equipSlots.end()) {
        equipSlots.push_back(slot);
    }
}

void Item::setCoverageSlot(EquipSlot slot, int coverage) {
    // 确保覆盖率在0-100范围内
    coverage = std::max(0, std::min(100, coverage));
    
    for (auto& slotCoverage : coverageSlots) {
        if (slotCoverage.slot == slot) {
            slotCoverage.coverage = coverage;
            return;
        }
    }
    
    // 如果槽位不存在，添加它
    addCoverageSlot(slot, coverage);
}

int Item::getCoverage(EquipSlot slot) const {
    for (const auto& slotCoverage : coverageSlots) {
        if (slotCoverage.slot == slot) {
            return slotCoverage.coverage;
        }
    }
    return 0; // 不覆盖该槽位
}

bool Item::hasSlotCoverage(EquipSlot slot) const {
    for (const auto& slotCoverage : coverageSlots) {
        if (slotCoverage.slot == slot) {
            return true;
        }
    }
    return false;
}

void Item::removeCoverageSlot(EquipSlot slot) {
    // 从覆盖率列表中移除
    auto it = std::remove_if(coverageSlots.begin(), coverageSlots.end(),
        [slot](const EquipSlotCoverage& coverage) {
            return coverage.slot == slot;
        });
    coverageSlots.erase(it, coverageSlots.end());
    
    // 为了向后兼容，也从equipSlots中移除
    auto equipIt = std::find(equipSlots.begin(), equipSlots.end(), slot);
    if (equipIt != equipSlots.end()) {
        equipSlots.erase(equipIt);
    }
    
    // 如果没有覆盖任何槽位，设置为不可穿戴
    if (coverageSlots.empty() && equipSlots.empty()) {
        wearable = false;
    }
}

std::vector<EquipSlot> Item::getAllCoveredSlots() const {
    std::vector<EquipSlot> slots;
    for (const auto& coverage : coverageSlots) {
        slots.push_back(coverage.slot);
    }
    return slots;
}