#pragma once
#ifndef AMMO_H
#define AMMO_H

#include "Item.h"
#include <string>
#include <vector>

class Ammo : public Item {
private:
    // 子弹基础属性
    int baseDamage;              // 基础伤害（点数）
    float basePenetration;       // 基础穿透力（毫米钢板）
    float baseRange;             // 基础射程（米）
    float baseSpeed;             // 基础子弹速度（米/秒）
    
    // 子弹对枪械属性的修正值
    float modRecoil = 0.0f;      // 后坐力修正（百分比）
    float modAccuracyMOA = 0.0f; // 精度修正（角分）
    float modErgonomics = 0.0f;  // 人体工程学修正（点数）
    
    std::string ammoType;        // 弹药类型（例如 "9mm", "5.56x45mm"）

public:
    // 简化构造函数
    Ammo(const std::string& itemName);

    // 获取子弹基础属性
    int getBaseDamage() const { return baseDamage; }
    float getBasePenetration() const { return basePenetration; }
    float getBaseRange() const { return baseRange; }
    float getBaseSpeed() const { return baseSpeed; }
    
    // 获取子弹对枪械属性的修正值
    float getModRecoil() const { return modRecoil; }
    float getModAccuracyMOA() const { return modAccuracyMOA; }
    float getModErgonomics() const { return modErgonomics; }
    
    // 获取子弹类型
    const std::string& getAmmoType() const { return ammoType; }
    
    // 设置子弹基础属性
    void setBaseDamage(int damage) { baseDamage = damage; }
    void setBasePenetration(float penetration) { basePenetration = penetration; }
    void setBaseRange(float range) { baseRange = range; }
    void setBaseSpeed(float speed) { baseSpeed = speed; }
    
    // 设置子弹对枪械属性的修正值
    void setModRecoil(float recoilMod) { modRecoil = recoilMod; }
    void setModAccuracyMOA(float accuracyMod) { modAccuracyMOA = accuracyMod; }
    void setModErgonomics(float ergoMod) { modErgonomics = ergoMod; }
    
    // 设置子弹类型
    void setAmmoType(const std::string& type) { ammoType = type; }

    void use() override;
    
    // 拷贝构造函数
    Ammo(const Ammo& other);


    // 拷贝赋值运算符
    Ammo& operator=(const Ammo& other);
    
    // 重写克隆方法，用于创建Ammo的深拷贝
    Item* clone() const override {
        return new Ammo(*this);
    }

};

#endif // AMMO_H