#pragma once
#ifndef EQUIPMENT_SYSTEM_H
#define EQUIPMENT_SYSTEM_H

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <set>
#include "Item.h"

class EquipmentSystem {
private:
    // 双向映射：槽位到物品的映射
    std::unordered_map<EquipSlot, std::set<Item*>> slotToItems;
    
    // 双向映射：物品到槽位的映射
    std::unordered_map<Item*, std::set<EquipSlot>> itemToSlots;
    
    // 物品所有权管理
    std::vector<std::unique_ptr<Item>> ownedItems;

public:
    EquipmentSystem() = default;
    ~EquipmentSystem() = default;
    
    // 穿戴物品，返回操作耗时
    float equipItem(std::unique_ptr<Item> item);
    
    // 卸下物品，返回操作耗时和卸下的物品
    std::pair<float, std::unique_ptr<Item>> unequipItem(EquipSlot slot);
    
    // 卸下特定物品（从所有槽位），返回操作耗时和卸下的物品
    std::pair<float, std::unique_ptr<Item>> unequipItem(Item* item);
    
    // 检查槽位是否已装备物品
    bool isSlotEquipped(EquipSlot slot) const;
    
    // 获取特定槽位的所有装备
    std::vector<Item*> getEquippedItems(EquipSlot slot) const;
    
    // 获取特定槽位的第一个装备（向后兼容）
    Item* getEquippedItem(EquipSlot slot) const;
    
    // 获取所有已装备物品的槽位
    std::vector<EquipSlot> getEquippedSlots() const;
    
    // 获取物品所在的所有槽位
    std::set<EquipSlot> getItemSlots(Item* item) const;

    // 计算穿戴/卸下物品所需的时间
    float calculateEquipTime(const Item* item) const;
    float calculateUnequipTime(const Item* item) const;
    
    // 计算所有装备的总重量
    float getTotalEquipmentWeight() const;
    
    // 获取特定槽位装备的存储空间
    Storage* getEquippedItemStorage(EquipSlot slot, size_t storageIndex) const;
    
    // 获取所有已装备物品的所有存储空间列表
    std::vector<std::pair<EquipSlot, Storage*>> getAllStorages() const;
    
    // 检查物品是否已装备
    bool isItemEquipped(Item* item) const;
    
    // 获取所有已装备的物品
    std::vector<Item*> getAllEquippedItems() const;
};

#endif // EQUIPMENT_SYSTEM_H