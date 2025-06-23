#pragma once
#ifndef ITEM_H
#define ITEM_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_set>
#include <SDL3/SDL.h> // 用于可能的渲染或图标
#include "ItemFlag.h" // 添加物品标签头文件

// 前向声明
class Storage;

// 物品类别通过ItemFlag实现，不再使用枚举

// 物品稀有度枚举
enum class ItemRarity {
    COMMON,     // 常规
    RARE,       // 稀有
    EPIC,       // 史诗
    LEGENDARY,  // 传说
    MYTHIC      // 神话
};

// 装备覆盖部位枚举
enum class EquipSlot {
    NONE,           // 不可穿戴
    HEAD,           // 头部
    CHEST,          // 胸部
    ABDOMEN,        // 腹部
    LEFT_LEG,       // 左腿
    RIGHT_LEG,      // 右腿
    LEFT_FOOT,      // 左脚
    RIGHT_FOOT,     // 右脚
    LEFT_ARM,       // 左臂
    RIGHT_ARM,      // 右臂
    LEFT_HAND,      // 左手
    RIGHT_HAND,     // 右手
    BACK,           // 背部
};

class Item {
protected:
    std::string name;                      // 物品名称
    float weight;                          // 重量（千克）
    float volume;                          // 体积（立方分米）
    float length;                          // 长度（厘米）
    int value;                             // 物品价值（金币）
    bool wearable;                         // 是否可穿戴
    std::vector<EquipSlot> equipSlots;     // 物品可以覆盖的槽位（可以有多个）
    ItemRarity rarity;                     // 物品稀有度
    
    // 物品标签集合（用于替代物品类别）
    std::unordered_set<ItemFlag> flags;    // 物品标签
    
    // 存储空间列表
    std::vector<std::unique_ptr<Storage>> storages;
    
    // 新增属性
    std::string description;               // 物品描述
    int piercingDamage;                    // 穿刺伤害
    int bluntDamage;                       // 钝击伤害
    int slashingDamage;                    // 斩击伤害
    float attackTime;                      // 攻击耗时（秒）
    int staminaCost;                       // 攻击耐力消耗
    int activationCost;                    // 激活消耗
    
    float piercingDefense;                // 对穿刺防御
    float bluntDefense;                    // 对钝击防御
    float slashingDefense;                 // 对斩击防御
    float bulletDefense;                   // 对射击防御
    float usesRemaining;                   // 可使用次数

    // 唯一标识符
    std::string uniqueId;                  // 物品唯一标识符

public:
    // 简化的构造函数，只需要基本属性
    Item(const std::string& itemName, 
         float itemWeight = 1.0f, 
         float itemVolume = 1.0f, 
         float itemLength = 1.0f, 
         int itemValue = 0);

    virtual ~Item();
    
    // 拷贝构造函数
    Item(const Item& other);
    
    // 拷贝赋值运算符
    Item& operator=(const Item& other);

    // 添加标签方法
    void addFlag(ItemFlag flag);
    void addFlags(const std::vector<ItemFlag>& flagList);
    bool hasFlag(ItemFlag flag) const;
    void removeFlag(ItemFlag flag);
    
    // 根据标签完善物品属性
    void processFlags();
    
    // 获取器
    const std::string& getName() const { return name; }
    float getWeight() const { return weight; }
    float getVolume() const { return volume; }
    float getLength() const { return length; }
    int getValue() const { return value; }
    bool isWearable() const { return wearable; }
    const std::vector<EquipSlot>& getEquipSlots() const { return equipSlots; }
    EquipSlot getPrimaryEquipSlot() const { return equipSlots.empty() ? EquipSlot::NONE : equipSlots[0]; }
    ItemRarity getRarity() const { return rarity; }
    
    // 新增属性获取器
    const std::string& getDescription() const { return description; }
    int getPiercingDamage() const { return piercingDamage; }
    int getBluntDamage() const { return bluntDamage; }
    int getSlashingDamage() const { return slashingDamage; }
    float getAttackTime() const { return attackTime; }
    int getStaminaCost() const { return staminaCost; }
    int getActivationCost() const { return activationCost; }
    float getPiercingDefense() const { return piercingDefense; }
    float getBluntDefense() const { return bluntDefense; }
    float getSlashingDefense() const { return slashingDefense; }
    float getBulletDefense() const { return bulletDefense; }
    float getUsesRemaining() const { return usesRemaining; }
    
    // 获取唯一标识符
    const std::string& getUniqueId() const { return uniqueId; }
    void setUniqueId(const std::string& id) { uniqueId = id; }
    void generateUniqueId(); // 生成唯一标识符
    
    // 物品类别判断方法（通过ItemFlag实现）
    bool isWeapon() const { return hasFlag(ItemFlag::WEAPON); }
    bool isGun() const { return hasFlag(ItemFlag::GUN); }
    bool isMagazine() const { return hasFlag(ItemFlag::MAGAZINE); }
    bool isAmmo() const { return hasFlag(ItemFlag::AMMO); }
    bool isConsumable() const { return hasFlag(ItemFlag::CONSUMABLE); }
    bool isContainer() const { return hasFlag(ItemFlag::CONTAINER); }
    
    // 设置器
    void setName(const std::string& newName) { name = newName; }
    void setWeight(float newWeight) { weight = newWeight; }
    void setVolume(float newVolume) { volume = newVolume; }
    void setLength(float newLength) { length = newLength; }
    void setValue(int newValue) { value = newValue; }
    void setRarity(ItemRarity newRarity) { rarity = newRarity; }
    void setDescription(const std::string& newDescription) { description = newDescription; }
    void setPiercingDamage(int damage) { piercingDamage = damage; }
    void setBluntDamage(int damage) { bluntDamage = damage; }
    void setSlashingDamage(int damage) { slashingDamage = damage; }
    void setAttackTime(float time) { attackTime = time; }
    void setStaminaCost(int cost) { staminaCost = cost; }
    void setActivationCost(int cost) { activationCost = cost; }
    void setPiercingDefense(float defense) { piercingDefense = defense; }
    void setBluntDefense(float defense) { bluntDefense = defense; }
    void setSlashingDefense(float defense) { slashingDefense = defense; }
    void setBulletDefense(float defense) { bulletDefense = defense; }
    void setUsesRemaining(float uses) { usesRemaining = uses; }
    
    // 类别相关方法已通过ItemFlag实现
    
    // 槽位相关方法
    void addEquipSlot(EquipSlot slot);
    bool canEquipToSlot(EquipSlot slot) const;
    void removeEquipSlot(EquipSlot slot);
    
    // 存储空间相关方法
    void addStorage(std::unique_ptr<Storage> storage);
    size_t getStorageCount() const;
    Storage* getStorage(size_t index) const;
    
    // 计算总重量（包括存储空间中的物品）
    float getTotalWeight() const;
    
    // 物品的特有行为，例如使用、装备等，可以在子类中重写
    virtual void use() {};
    
    // 克隆方法，用于创建物品的深拷贝
    virtual Item* clone() const {
        return new Item(*this);
    }
    
    // 获取物品的所有标签名称
    std::vector<std::string> getFlagNames() const;
    
    // 枪械配件相关方法
    bool isGunMod() const { return hasFlag(ItemFlag::GUNMOD); }
};

#endif // ITEM_H