#pragma once
#ifndef ITEM_LOADER_H
#define ITEM_LOADER_H

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include "nlohmann/json.hpp"
#include "Item.h"
#include "Gun.h"
#include "GunMod.h"
#include "Magazine.h"
#include "Ammo.h"
#include "Weapon.h"


// 使用nlohmann/json库的命名空间
using json = nlohmann::json;

// 物品加载器类 - 负责从JSON文件加载物品模板
class ItemLoader {
private:
    // 存储已加载的物品模板
    std::unordered_map<std::string, std::unique_ptr<Item>> itemTemplates;
    std::unordered_map<std::string, std::unique_ptr<Gun>> gunTemplates;
    std::unordered_map<std::string, std::unique_ptr<GunMod>> gunModTemplates;
    std::unordered_map<std::string, std::unique_ptr<Magazine>> magazineTemplates;
    std::unordered_map<std::string, std::unique_ptr<Ammo>> ammoTemplates;
    std::unordered_map<std::string, std::unique_ptr<Weapon>> weaponTemplates;
    
    // 单例实例
    static ItemLoader* instance;
    
    // 私有构造函数（单例模式）
    ItemLoader() = default;
    
public:
    // 禁止复制和移动
    ItemLoader(const ItemLoader&) = delete;
    ItemLoader& operator=(const ItemLoader&) = delete;
    ItemLoader(ItemLoader&&) = delete;
    ItemLoader& operator=(ItemLoader&&) = delete;
    
    // 获取单例实例
    static ItemLoader* getInstance();
    
    // 从JSON文件加载所有物品
    bool loadItemsFromFile(const std::string& filePath);
    
    // 从JSON字符串加载物品
    bool loadItemsFromJson(const std::string& jsonString);
    
    // 从JSON对象加载物品
    bool loadItemsFromJson(const json& jsonData);
    
    // 创建物品实例（基于模板的深拷贝）
    std::unique_ptr<Item> createItem(const std::string& itemName);
    std::unique_ptr<Gun> createGun(const std::string& gunName);
    std::unique_ptr<GunMod> createGunMod(const std::string& modName);
    std::unique_ptr<Magazine> createMagazine(const std::string& magazineName);
    std::unique_ptr<Ammo> createAmmo(const std::string& ammoName);
    std::unique_ptr<Weapon> createWeapon(const std::string& weaponName);
    
    // 检查是否存在指定名称的物品模板
    bool hasItemTemplate(const std::string& itemName) const;
    bool hasGunTemplate(const std::string& gunName) const;
    bool hasGunModTemplate(const std::string& modName) const;
    bool hasMagazineTemplate(const std::string& magazineName) const;
    bool hasAmmoTemplate(const std::string& ammoName) const;
    bool hasWeaponTemplate(const std::string& weaponName) const;
    
private:
    // 从JSON对象加载基本物品
    std::unique_ptr<Item> loadItemFromJson(const json& itemJson);
    
    // 从JSON对象加载枪械
    std::unique_ptr<Gun> loadGunFromJson(const json& gunJson);
    
    // 从JSON对象加载枪械配件
    std::unique_ptr<GunMod> loadGunModFromJson(const json& modJson);
    
    // 从JSON对象加载弹匣
    std::unique_ptr<Magazine> loadMagazineFromJson(const json& magazineJson);
    
    // 从JSON对象加载弹药
    std::unique_ptr<Ammo> loadAmmoFromJson(const json& ammoJson);
    
    // 从JSON对象加载武器
    std::unique_ptr<Weapon> loadWeaponFromJson(const json& weaponJson);
    
    // 辅助方法：从JSON加载物品标签
    void loadItemFlags(Item* item, const json& flagsJson);
    
    // 辅助方法：从JSON加载装备槽位
    void loadEquipSlots(Item* item, const json& slotsJson);
    
    // 辅助方法：解析攻击方式
    AttackMethod parseAttackMethod(const std::string& methodStr);
    
    // 辅助方法：从JSON加载特殊效果
    SpecialEffect loadSpecialEffectFromJson(const json& effectJson);
    
    // 注意：物品类别现在通过ItemFlag处理，不再使用ItemCategory
};

#endif // ITEM_LOADER_H