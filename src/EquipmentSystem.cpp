#include "EquipmentSystem.h"
#include <algorithm>

float EquipmentSystem::calculateEquipTime(const Item* item) const {
    if (!item) return 0.0f;
    
    // 基础穿戴时间，可以根据物品类型、重量等因素调整
    float baseTime = 1.0f;
    
    // 根据重量调整时间
    baseTime += item->getWeight() * 0.05f;
    
    return baseTime;
}

float EquipmentSystem::calculateUnequipTime(const Item* item) const {
    if (!item) return 0.0f;
    
    // 卸下时间通常比穿戴时间短
    float baseTime = 0.8f;
    
    // 根据重量调整时间
    baseTime += item->getWeight() * 0.03f;
    
    return baseTime;
}

float EquipmentSystem::equipItem(std::unique_ptr<Item> item) {
    if (!item || !item->isWearable()) {
        return 0.0f; // 不可穿戴物品
    }
    
    // 获取物品的装备槽位
    const std::vector<EquipSlot>& slots = item->getEquipSlots();
    if (slots.empty() || slots[0] == EquipSlot::NONE) {
        return 0.0f; // 无效槽位
    }
    
    // 计算穿戴时间
    float equipTime = calculateEquipTime(item.get());
    float totalUnequipTime = 0.0f;
    
    // 获取物品指针（用于后续映射）
    Item* itemPtr = item.get();
    
    // 将物品添加到所有权管理容器
    ownedItems.push_back(std::move(item));
    
    // 为物品创建空的槽位集合
    itemToSlots[itemPtr] = std::set<EquipSlot>();
    
    // 遍历物品的所有装备槽位
    for (EquipSlot slot : slots) {
        // 将物品添加到每个槽位的映射中
        slotToItems[slot].insert(itemPtr);
        
        // 将槽位添加到物品的映射中
        itemToSlots[itemPtr].insert(slot);
    }
    
    return equipTime;
}

std::pair<float, std::unique_ptr<Item>> EquipmentSystem::unequipItem(EquipSlot slot) {
    if (!isSlotEquipped(slot)) {
        return {0.0f, nullptr}; // 槽位未装备物品
    }
    
    // 获取槽位中的第一个物品（向后兼容）
    Item* itemPtr = *slotToItems[slot].begin();
    
    // 使用新的按物品卸下方法
    return unequipItem(itemPtr);
}

std::pair<float, std::unique_ptr<Item>> EquipmentSystem::unequipItem(Item* item) {
    if (!item || !isItemEquipped(item)) {
        return {0.0f, nullptr}; // 物品未装备
    }
    
    // 计算卸下时间
    float unequipTime = calculateUnequipTime(item);
    
    // 获取物品所在的所有槽位
    auto slots = itemToSlots[item];
    
    // 从所有槽位中移除该物品
    for (EquipSlot slot : slots) {
        slotToItems[slot].erase(item);
        
        // 如果槽位为空，从映射中移除
        if (slotToItems[slot].empty()) {
            slotToItems.erase(slot);
        }
    }
    
    // 从物品到槽位的映射中移除
    itemToSlots.erase(item);
    
    // 查找并移除物品的所有权
    std::unique_ptr<Item> removedItem = nullptr;
    for (auto it = ownedItems.begin(); it != ownedItems.end(); ++it) {
        if (it->get() == item) {
            removedItem = std::move(*it);
            ownedItems.erase(it);
            break;
        }
    }
    
    return {unequipTime, std::move(removedItem)};
}

bool EquipmentSystem::isSlotEquipped(EquipSlot slot) const {
    return slotToItems.find(slot) != slotToItems.end() && !slotToItems.at(slot).empty();
}

std::vector<Item*> EquipmentSystem::getEquippedItems(EquipSlot slot) const {
    std::vector<Item*> items;
    auto it = slotToItems.find(slot);
    if (it != slotToItems.end()) {
        items.insert(items.end(), it->second.begin(), it->second.end());
    }
    return items;
}

Item* EquipmentSystem::getEquippedItem(EquipSlot slot) const {
    auto items = getEquippedItems(slot);
    return items.empty() ? nullptr : items[0];
}

std::vector<EquipSlot> EquipmentSystem::getEquippedSlots() const {
    std::vector<EquipSlot> slots;
    for (const auto& [slot, items] : slotToItems) {
        if (!items.empty()) {
            slots.push_back(slot);
        }
    }
    return slots;
}

std::set<EquipSlot> EquipmentSystem::getItemSlots(Item* item) const {
    auto it = itemToSlots.find(item);
    if (it != itemToSlots.end()) {
        return it->second;
    }
    return {};
}

bool EquipmentSystem::isItemEquipped(Item* item) const {
    return itemToSlots.find(item) != itemToSlots.end();
}

std::vector<Item*> EquipmentSystem::getAllEquippedItems() const {
    std::vector<Item*> items;
    for (const auto& [item, slots] : itemToSlots) {
        items.push_back(item);
    }
    return items;
}

float EquipmentSystem::getTotalEquipmentWeight() const {
    float totalWeight = 0.0f;
    for (const auto& item : ownedItems) {
        totalWeight += item->getTotalWeight();
    }
    return totalWeight;
}

Storage* EquipmentSystem::getEquippedItemStorage(EquipSlot slot, size_t storageIndex) const {
    Item* item = getEquippedItem(slot);
    if (item) {
        return item->getStorage(storageIndex);
    }
    return nullptr;
}

std::vector<std::pair<EquipSlot, Storage*>> EquipmentSystem::getAllStorages() const {
    std::vector<std::pair<EquipSlot, Storage*>> allStorages;
    
    // 遍历所有已装备的物品
    for (const auto& [slot, items] : slotToItems) {
        for (Item* item : items) {
            // 获取该物品的所有存储空间
            size_t storageCount = item->getStorageCount();
            for (size_t i = 0; i < storageCount; ++i) {
                Storage* storage = item->getStorage(i);
                if (storage) {
                    allStorages.emplace_back(slot, storage);
                }
            }
        }
    }
    
    return allStorages;
}