#pragma once
#ifndef GUNMOD_H
#define GUNMOD_H

#include "Item.h"

class GunMod : public Item {
protected:
    // 枪械配件属性（对枪械的影响值）
    float modSoundLevel = 0.0f;            // 声音级别影响（分贝）
    float modFireRate = 0.0f;              // 射速影响（发/分钟）
    float modAccuracyMOA = 0.0f;           // 精度影响（角分）
    float modRecoil = 0.0f;                // 后坐力影响（百分比）
    float modErgonomics = 0.0f;            // 人体工程学影响（点数）
    float modBreathStability = 0.0f;       // 呼吸稳定性影响（百分比）
    
    // 枪械配件对子弹属性的影响值
    float modDamageBonus = 0.0f;           // 伤害加成（百分比）
    float modRangeBonus = 0.0f;            // 射程影响（米）
    float modBulletSpeedBonus = 0.0f;      // 子弹速度影响（米/秒）
    float modPenetrationBonus = 0.0f;      // 穿透力加成（百分比）
    
    // 新增：槽位容量影响
    std::map<std::string, int> slotCapacityModifiers;  // 对其他槽位容量的修改
    
    // 新增：弹药类型影响
    std::vector<std::string> addedAmmoTypes;           // 新增支持的弹药类型
    std::vector<std::string> removedAmmoTypes;         // 移除支持的弹药类型
    
    // 新增：兼容的槽位类型（通过flag确定）
    std::vector<std::string> compatibleSlots;         // 可以安装到的槽位类型

public:
    // 构造函数
    GunMod(const std::string& itemName);

    // 枪械配件影响值的getter
    float getModSoundLevel() const { return modSoundLevel; }
    float getModFireRate() const { return modFireRate; }
    float getModAccuracyMOA() const { return modAccuracyMOA; }
    float getModRecoil() const { return modRecoil; }
    float getModErgonomics() const { return modErgonomics; }
    float getModBreathStability() const { return modBreathStability; }
    
    // 枪械配件对子弹属性影响值的getter
    float getModDamageBonus() const { return modDamageBonus; }
    float getModRangeBonus() const { return modRangeBonus; }
    float getModBulletSpeedBonus() const { return modBulletSpeedBonus; }
    float getModPenetrationBonus() const { return modPenetrationBonus; }
    
    // 枪械配件影响值的setter
    void setModSoundLevel(float value) { modSoundLevel = value; }
    void setModFireRate(float value) { modFireRate = value; }
    void setModAccuracyMOA(float value) { modAccuracyMOA = value; }
    void setModRecoil(float value) { modRecoil = value; }
    void setModErgonomics(float value) { modErgonomics = value; }
    void setModBreathStability(float value) { modBreathStability = value; }
    
    // 枪械配件对子弹属性影响值的setter
    void setModDamageBonus(float value) { modDamageBonus = value; }
    void setModRangeBonus(float value) { modRangeBonus = value; }
    void setModBulletSpeedBonus(float value) { modBulletSpeedBonus = value; }
    void setModPenetrationBonus(float value) { modPenetrationBonus = value; }
    
    // 设置枪械配件属性
    void setModAttributes(float soundLevel = 0.0f, float fireRate = 0.0f, 
                         float damageBonus = 0.0f, float accuracyMOA = 0.0f, 
                         float recoil = 0.0f, float ergonomics = 0.0f, 
                         float breathStability = 0.0f, float range = 0.0f,
                         float bulletSpeed = 0.0f, float penetrationBonus = 0.0f);
    
    // 新增：槽位容量修改
    void addSlotCapacityModifier(const std::string& slotType, int modifier);
    void removeSlotCapacityModifier(const std::string& slotType);
    const std::map<std::string, int>& getSlotCapacityModifiers() const;
    void clearSlotCapacityModifiers();
    
    // 新增：弹药类型修改
    void addAmmoTypeSupport(const std::string& ammoType);
    void removeAmmoTypeSupport(const std::string& ammoType);
    void addAmmoTypeRestriction(const std::string& ammoType);
    void removeAmmoTypeRestriction(const std::string& ammoType);
    const std::vector<std::string>& getAddedAmmoTypes() const;
    const std::vector<std::string>& getRemovedAmmoTypes() const;
    void clearAmmoTypeChanges();
    
    // 新增：槽位兼容性
    void addCompatibleSlot(const std::string& slotType);
    void removeCompatibleSlot(const std::string& slotType);
    const std::vector<std::string>& getCompatibleSlots() const;
    bool canAttachToSlot(const std::string& slotType) const;
    void clearCompatibleSlots();
    
    // 新增：从Flag自动确定兼容槽位
    void updateCompatibleSlotsFromFlags();
    
    // 拷贝构造函数
    GunMod(const GunMod& other);
    
    // 拷贝赋值运算符
    GunMod& operator=(const GunMod& other);
    
    // 重写克隆方法，用于创建GunMod的深拷贝
    Item* clone() const override {
        return new GunMod(*this);
    }
};

#endif // GUNMOD_H