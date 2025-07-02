#pragma once
#ifndef GUN_H
#define GUN_H

#include "Item.h"
#include "Magazine.h" 
#include "Ammo.h"     
#include "GunMod.h"
#include "SlotWhitelist.h"
#include "FlagMapper.h"
#include <string>
#include <vector>
#include <map>
#include <memory>

// 移除硬编码枚举，改用基于Flag的字符串系统
// 枪械类型、射击模式、槽位类型现在通过ItemFlag和字符串确定

class Gun : public Item {
private:
    // 枪膛内的一发子弹
    std::unique_ptr<Ammo> chamberedRound; 
    
    // 移除硬编码枚举，改用字符串系统
    std::string currentFiringMode;                  // 当前射击模式
    std::vector<std::string> availableFiringModes;  // 可用射击模式列表
    
    // 枪械固有的基础属性值
    float baseSoundLevel;        // 基础枪声大小
    float baseFireRate;          // 基础射速
    float baseAccuracyMOA;       // 基础精度
    float baseRecoil;            // 基础后坐力
    float baseErgonomics;        // 基础人体工程学
    float baseBreathStability;   // 基础呼吸稳定性
    
    // 枪械对子弹的基础加成值
    float baseDamageBonus;       // 基础伤害加成
    float baseRangeBonus;        // 基础射程加成
    float baseBulletSpeedBonus;  // 基础子弹速度加成
    float basePenetrationBonus;  // 基础穿透力加成
    
    // 计算后的最终属性值（考虑配件影响后的值）
    float soundLevel;            // 最终枪声大小
    float fireRate;              // 最终射速
    float accuracyMOA;           // 最终精度
    float recoil;                // 最终后坐力
    float ergonomics;            // 最终人体工程学
    float breathStability;       // 最终呼吸稳定性
    float damageBonus;           // 最终伤害加成
    float rangeBonus;            // 最终射程加成
    float bulletSpeedBonus;      // 最终子弹速度加成
    float penetrationBonus;      // 最终穿透力加成
    float reloadTime;            // 换弹时间（秒）
    
    // 新增：字符串化槽位系统
    std::map<std::string, int> baseSlotCapacity;    // 基础槽位容量
    std::map<std::string, int> currentSlotCapacity; // 当前槽位容量（考虑配件影响）
    std::map<std::string, int> slotUsage;           // 槽位使用状态
    std::map<std::string, std::vector<std::unique_ptr<GunMod>>> attachmentSlots; // 配件存储
    std::map<std::string, SlotWhitelist> slotWhitelists; // 槽位白名单
    
    // 新增：弹药类型系统重构
    std::vector<std::string> baseAcceptedAmmoTypes;    // 基础接受的弹药类型
    std::vector<std::string> currentAcceptedAmmoTypes; // 当前接受的弹药类型（考虑配件影响）
    
    std::unique_ptr<Magazine> currentMagazine; 
    
    // 可装填弹匣名称列表
    std::vector<std::string> acceptedMagazineNames;

public:
    // 简化构造函数
    Gun(const std::string& itemName);

    // 槽位管理（重构为字符串版本）
    void initAttachmentSlots();
    void setupDefaultSlotWhitelists(); // 设置默认白名单规则
    int getSlotCapacity(const std::string& slotType) const;
    int getEffectiveSlotCapacity(const std::string& slotType) const; // 考虑配件影响后的容量
    void setSlotCapacity(const std::string& slotType, int capacity);
    int getSlotUsage(const std::string& slotType) const;
    bool isSlotFull(const std::string& slotType) const;
    
    // 新增：白名单管理
    void setSlotWhitelist(const std::string& slotType, const SlotWhitelist& whitelist);
    SlotWhitelist& getSlotWhitelist(const std::string& slotType);
    bool canAttachToSlot(const std::string& slotType, const GunMod* mod) const;

    // 获取可接受的弹匣名称列表
    const std::vector<std::string>& getAcceptedMagazineNames() const { return acceptedMagazineNames; }
    
    // 设置可接受的弹匣名称列表
    void setAcceptedMagazineNames(const std::vector<std::string>& magazineNames) { acceptedMagazineNames = magazineNames; }
    
    // 添加可接受的弹匣名称
    void addAcceptedMagazineName(const std::string& magazineName) { acceptedMagazineNames.push_back(magazineName); }
    
    // 射击方法 - 返回发射的子弹，同时尝试自动上膛
    std::unique_ptr<Ammo> shoot();
    
    // 手动上膛方法 - 从弹匣取子弹上膛
    bool chamberManually();
    
    // 检查是否可以射击（有子弹在膛内）
    bool canShoot() const { return chamberedRound != nullptr; }
    
    // 检查弹匣是否为空
    bool isMagazineEmpty() const { return !currentMagazine || currentMagazine->isEmpty(); }

    // 获取枪械属性（计算后的最终值）
    float getSoundLevel() const { return soundLevel; }
    float getFireRate() const;
    float getAccuracyMOA() const { return accuracyMOA; }
    float getRecoil() const { return recoil; }
    float getErgonomics() const { return ergonomics; }
    float getBreathStability() const { return breathStability; }
    float getReloadTime() const { return reloadTime; }
    
    // 获取枪械对子弹属性的加成（计算后的最终值）
    float getDamageBonus() const { return damageBonus; }
    float getRangeBonus() const { return rangeBonus; }
    float getBulletSpeedBonus() const { return bulletSpeedBonus; }
    float getPenetrationBonus() const { return penetrationBonus; }
    
    // 获取枪械基础属性值
    float getBaseSoundLevel() const { return baseSoundLevel; }
    float getBaseFireRate() const { return baseFireRate; }
    float getBaseAccuracyMOA() const { return baseAccuracyMOA; }
    float getBaseRecoil() const { return baseRecoil; }
    float getBaseErgonomics() const { return baseErgonomics; }
    float getBaseBreathStability() const { return baseBreathStability; }
    
    // 获取枪械对子弹的基础加成值
    float getBaseDamageBonus() const { return baseDamageBonus; }
    float getBaseRangeBonus() const { return baseRangeBonus; }
    float getBaseBulletSpeedBonus() const { return baseBulletSpeedBonus; }
    float getBasePenetrationBonus() const { return basePenetrationBonus; }
    
    // 获取枪械基础属性值的声明（实现在 cpp 文件中）
    // 设置方法在下方声明

    Ammo* getChamberedRound() const { return chamberedRound.get(); }

    bool canAcceptMagazine(const Magazine* mag) const;
    void loadMagazine(std::unique_ptr<Magazine> mag);
    std::unique_ptr<Magazine> unloadMagazine();
    Magazine* getCurrentMagazine() const;
    
    // 配件管理（重构为字符串版本）
    bool attach(const std::string& slotType, std::unique_ptr<GunMod> attachment);
    std::unique_ptr<GunMod> detach(const std::string& slotType, int index = 0);
    GunMod* getAttachment(const std::string& slotType, int index = 0) const;
    const std::vector<GunMod*> getAllAttachments(const std::string& slotType) const;
    int getTotalAttachmentCount() const;
    
    // 弹药类型管理
    void setBaseAcceptedAmmoTypes(const std::vector<std::string>& types);
    const std::vector<std::string>& getBaseAcceptedAmmoTypes() const;
    const std::vector<std::string>& getEffectiveAmmoTypes() const;
    bool canAcceptAmmoType(const std::string& ammoType) const;
    
    void use() override;
    
    // 射击模式管理（重构为字符串版本）
    void setAvailableFiringModes(const std::vector<std::string>& modes);
    const std::vector<std::string>& getAvailableFiringModes() const;
    const std::string& getCurrentFiringMode() const;
    void setCurrentFiringMode(const std::string& mode);
    void toggleFiringMode();
    
    // 类型检查（基于flag）
    bool isGunType(const std::string& gunType) const;
    bool hasFiringMode(const std::string& mode) const;
    std::vector<std::string> getGunTypes() const;
    
    // 属性聚合（重构）
    float getTotalWeight() const;
    float getTotalValue() const;
    
    // 重新计算所有属性
    void recalculateAllStats();
    void recalculateSlotCapacities();
    void recalculateAmmoTypes();
    void updateGunStats();
    void updateFiringModeFlags(); // 更新射击模式标签
    
    // 设置基础属性值
    void setBaseSoundLevel(float level);
    void setBaseFireRate(float rate);
    void setBaseAccuracyMOA(float accuracy);
    void setBaseRecoil(float recoil);
    void setBaseErgonomics(float ergonomics);
    void setBaseBreathStability(float stability);
    void setBaseDamageBonus(float bonus);
    void setBaseRangeBonus(float bonus);
    void setBaseBulletSpeedBonus(float bonus);
    void setBasePenetrationBonus(float bonus);
    void setReloadTime(float time);
    
    // 重写克隆方法，用于创建Gun的深拷贝
    Item* clone() const override {
        return new Gun(*this);
    }
    
    // 拷贝构造函数
    Gun(const Gun& other);
    
    // 拷贝赋值运算符
    Gun& operator=(const Gun& other);
};

#endif // GUN_H