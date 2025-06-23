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