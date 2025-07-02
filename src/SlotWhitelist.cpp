#include "SlotWhitelist.h"
#include "FlagMapper.h"
#include <sstream>
#include <algorithm>

SlotWhitelist::SlotWhitelist() : allowAll(false) {
}

SlotWhitelist::SlotWhitelist(bool allowAllItems) : allowAll(allowAllItems) {
}

// 物品名称管理
void SlotWhitelist::addAllowedItem(const std::string& itemName) {
    allowedItems.insert(itemName);
}

void SlotWhitelist::removeAllowedItem(const std::string& itemName) {
    allowedItems.erase(itemName);
}

bool SlotWhitelist::isItemAllowed(const std::string& itemName) const {
    return allowedItems.find(itemName) != allowedItems.end();
}

const std::set<std::string>& SlotWhitelist::getAllowedItems() const {
    return allowedItems;
}

void SlotWhitelist::clearAllowedItems() {
    allowedItems.clear();
}

// Flag规则管理
void SlotWhitelist::addRequiredFlag(ItemFlag flag) {
    requiredFlags.insert(flag);
}

void SlotWhitelist::addForbiddenFlag(ItemFlag flag) {
    forbiddenFlags.insert(flag);
}

void SlotWhitelist::removeRequiredFlag(ItemFlag flag) {
    requiredFlags.erase(flag);
}

void SlotWhitelist::removeForbiddenFlag(ItemFlag flag) {
    forbiddenFlags.erase(flag);
}

const std::set<ItemFlag>& SlotWhitelist::getRequiredFlags() const {
    return requiredFlags;
}

const std::set<ItemFlag>& SlotWhitelist::getForbiddenFlags() const {
    return forbiddenFlags;
}

void SlotWhitelist::clearRequiredFlags() {
    requiredFlags.clear();
}

void SlotWhitelist::clearForbiddenFlags() {
    forbiddenFlags.clear();
}

// 检查方法
bool SlotWhitelist::isAllowed(const Item* item) const {
    if (!item) {
        return false;
    }
    
    // 如果设置为允许所有，直接返回true
    if (allowAll) {
        return true;
    }
    
    // 检查是否有任何规则，如果没有规则则拒绝
    if (isEmpty()) {
        return false;
    }
    
    // 检查flag规则
    if (!checkFlags(item)) {
        return false;
    }
    
    // 检查物品名称（如果有物品名称白名单的话）
    if (!allowedItems.empty() && !checkItemName(item)) {
        return false;
    }
    
    return true;
}

bool SlotWhitelist::checkFlags(const Item* item) const {
    if (!item) {
        return false;
    }
    
    // 检查必须具有的标签
    for (ItemFlag requiredFlag : requiredFlags) {
        if (!item->hasFlag(requiredFlag)) {
            return false;
        }
    }
    
    // 检查禁止具有的标签
    for (ItemFlag forbiddenFlag : forbiddenFlags) {
        if (item->hasFlag(forbiddenFlag)) {
            return false;
        }
    }
    
    return true;
}

bool SlotWhitelist::checkItemName(const Item* item) const {
    if (!item) {
        return false;
    }
    
    // 如果没有物品名称限制，则通过
    if (allowedItems.empty()) {
        return true;
    }
    
    return isItemAllowed(item->getName());
}

// 全局配置方法
void SlotWhitelist::setAllowAll(bool allow) {
    allowAll = allow;
}

bool SlotWhitelist::getAllowAll() const {
    return allowAll;
}

// 清除所有规则
void SlotWhitelist::clear() {
    allowedItems.clear();
    requiredFlags.clear();
    forbiddenFlags.clear();
    allowAll = false;
}

// 检查是否为空（没有任何限制）
bool SlotWhitelist::isEmpty() const {
    return !allowAll && 
           allowedItems.empty() && 
           requiredFlags.empty() && 
           forbiddenFlags.empty();
}

// JSON序列化
nlohmann::json SlotWhitelist::toJson() const {
    nlohmann::json json;
    
    json["allow_all"] = allowAll;
    
    // 序列化允许的物品列表
    if (!allowedItems.empty()) {
        json["allowed_items"] = nlohmann::json::array();
        for (const std::string& item : allowedItems) {
            json["allowed_items"].push_back(item);
        }
    }
    
    // 序列化必须的标签
    if (!requiredFlags.empty()) {
        json["required_flags"] = nlohmann::json::array();
        for (ItemFlag flag : requiredFlags) {
            std::string flagStr = FlagMapper::itemFlagToString(flag);
            if (flagStr != "UNKNOWN") {
                json["required_flags"].push_back(flagStr);
            }
        }
    }
    
    // 序列化禁止的标签
    if (!forbiddenFlags.empty()) {
        json["forbidden_flags"] = nlohmann::json::array();
        for (ItemFlag flag : forbiddenFlags) {
            std::string flagStr = FlagMapper::itemFlagToString(flag);
            if (flagStr != "UNKNOWN") {
                json["forbidden_flags"].push_back(flagStr);
            }
        }
    }
    
    return json;
}

void SlotWhitelist::fromJson(const nlohmann::json& json) {
    clear(); // 清除现有规则
    
    // 读取allow_all设置
    if (json.contains("allow_all") && json["allow_all"].is_boolean()) {
        allowAll = json["allow_all"].get<bool>();
    }
    
    // 读取允许的物品列表
    if (json.contains("allowed_items") && json["allowed_items"].is_array()) {
        for (const auto& item : json["allowed_items"]) {
            if (item.is_string()) {
                allowedItems.insert(item.get<std::string>());
            }
        }
    }
    
    // 读取必须的标签
    if (json.contains("required_flags") && json["required_flags"].is_array()) {
        for (const auto& flagElement : json["required_flags"]) {
            if (flagElement.is_string()) {
                std::string flagStr = flagElement.get<std::string>();
                ItemFlag flag = FlagMapper::stringToItemFlag(flagStr);
                if (flag != ItemFlag::FLAG_COUNT) {
                    requiredFlags.insert(flag);
                }
            }
        }
    }
    
    // 读取禁止的标签
    if (json.contains("forbidden_flags") && json["forbidden_flags"].is_array()) {
        for (const auto& flagElement : json["forbidden_flags"]) {
            if (flagElement.is_string()) {
                std::string flagStr = flagElement.get<std::string>();
                ItemFlag flag = FlagMapper::stringToItemFlag(flagStr);
                if (flag != ItemFlag::FLAG_COUNT) {
                    forbiddenFlags.insert(flag);
                }
            }
        }
    }
}

// 调试和日志
std::string SlotWhitelist::toString() const {
    std::stringstream ss;
    
    ss << "SlotWhitelist{";
    ss << "allowAll=" << (allowAll ? "true" : "false");
    
    if (!allowedItems.empty()) {
        ss << ", allowedItems=[";
        bool first = true;
        for (const std::string& item : allowedItems) {
            if (!first) ss << ", ";
            ss << item;
            first = false;
        }
        ss << "]";
    }
    
    if (!requiredFlags.empty()) {
        ss << ", requiredFlags=[";
        bool first = true;
        for (ItemFlag flag : requiredFlags) {
            if (!first) ss << ", ";
            ss << FlagMapper::itemFlagToString(flag);
            first = false;
        }
        ss << "]";
    }
    
    if (!forbiddenFlags.empty()) {
        ss << ", forbiddenFlags=[";
        bool first = true;
        for (ItemFlag flag : forbiddenFlags) {
            if (!first) ss << ", ";
            ss << FlagMapper::itemFlagToString(flag);
            first = false;
        }
        ss << "]";
    }
    
    ss << "}";
    return ss.str();
}

std::vector<std::string> SlotWhitelist::getViolationReasons(const Item* item) const {
    std::vector<std::string> reasons;
    
    if (!item) {
        reasons.push_back("物品为空");
        return reasons;
    }
    
    if (allowAll) {
        return reasons; // 允许所有，没有违规原因
    }
    
    if (isEmpty()) {
        reasons.push_back("白名单为空，不允许任何物品");
        return reasons;
    }
    
    // 检查必须的标签
    for (ItemFlag requiredFlag : requiredFlags) {
        if (!item->hasFlag(requiredFlag)) {
            reasons.push_back("缺少必需标签: " + FlagMapper::itemFlagToString(requiredFlag));
        }
    }
    
    // 检查禁止的标签
    for (ItemFlag forbiddenFlag : forbiddenFlags) {
        if (item->hasFlag(forbiddenFlag)) {
            reasons.push_back("具有禁止标签: " + FlagMapper::itemFlagToString(forbiddenFlag));
        }
    }
    
    // 检查物品名称
    if (!allowedItems.empty() && !checkItemName(item)) {
        reasons.push_back("物品名称不在允许列表中: " + item->getName());
    }
    
    return reasons;
} 