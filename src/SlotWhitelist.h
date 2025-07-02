#pragma once
#ifndef SLOTWHITELIST_H
#define SLOTWHITELIST_H

#include "ItemFlag.h"
#include "Item.h"
#include <set>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

class SlotWhitelist {
private:
    std::set<std::string> allowedItems;      // 允许的物品名称
    std::set<ItemFlag> requiredFlags;        // 必须具有的标签
    std::set<ItemFlag> forbiddenFlags;       // 禁止具有的标签
    bool allowAll;                           // 是否允许所有物品

public:
    // 构造函数
    SlotWhitelist();
    explicit SlotWhitelist(bool allowAllItems);
    
    // 物品名称管理
    void addAllowedItem(const std::string& itemName);
    void removeAllowedItem(const std::string& itemName);
    bool isItemAllowed(const std::string& itemName) const;
    const std::set<std::string>& getAllowedItems() const;
    void clearAllowedItems();
    
    // Flag规则管理
    void addRequiredFlag(ItemFlag flag);
    void addForbiddenFlag(ItemFlag flag);
    void removeRequiredFlag(ItemFlag flag);
    void removeForbiddenFlag(ItemFlag flag);
    const std::set<ItemFlag>& getRequiredFlags() const;
    const std::set<ItemFlag>& getForbiddenFlags() const;
    void clearRequiredFlags();
    void clearForbiddenFlags();
    
    // 检查方法
    bool isAllowed(const Item* item) const;
    bool checkFlags(const Item* item) const;
    bool checkItemName(const Item* item) const;
    
    // 全局配置方法
    void setAllowAll(bool allow);
    bool getAllowAll() const;
    
    // 清除所有规则
    void clear();
    
    // 检查是否为空（没有任何限制）
    bool isEmpty() const;
    
    // JSON序列化
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& json);
    
    // 调试和日志
    std::string toString() const;
    std::vector<std::string> getViolationReasons(const Item* item) const;
};

#endif // SLOTWHITELIST_H 