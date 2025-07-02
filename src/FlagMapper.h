#pragma once
#ifndef FLAGMAPPER_H
#define FLAGMAPPER_H

#include "ItemFlag.h"
#include "Item.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

class FlagMapper {
private:
    static std::unordered_map<std::string, ItemFlag> stringToFlag;
    static std::unordered_map<ItemFlag, std::string> flagToString;
    static bool initialized;

public:
    // 初始化映射表
    static void initializeMappings();
    
    // 基础转换方法
    static ItemFlag stringToItemFlag(const std::string& str);
    static std::string itemFlagToString(ItemFlag flag);
    
    // 数组转换方法
    static std::vector<ItemFlag> stringArrayToFlags(const std::vector<std::string>& strings);
    static std::vector<std::string> flagsToStringArray(const std::vector<ItemFlag>& flags);
    
    // JSON辅助方法
    static void addFlagsFromJson(Item* item, const nlohmann::json& flagArray);
    static nlohmann::json flagsToJson(const Item* item);
    
    // 验证方法
    static bool isValidFlagString(const std::string& str);
    static std::vector<std::string> getInvalidFlags(const std::vector<std::string>& flags);
    
    // 获取所有有效的flag字符串（用于验证和调试）
    static std::vector<std::string> getAllValidFlagStrings();
};

#endif // FLAGMAPPER_H 