#pragma once
#ifndef MACHETE_H
#define MACHETE_H

#include "Item.h"
#include "AttackSystem.h"

class Machete : public Item, public IWeaponAttack {
private:
    float durability;           // 当前耐久度
    float maxDurability;        // 最大耐久度
    float durabilityLossPerHit; // 每次攻击的耐久度损失
    int currentCooldown;        // 当前冷却时间
    
    // 连击系统
    int comboCount;             // 当前连击数
    int maxCombo;               // 最大连击数
    int comboResetTime;         // 连击重置时间（毫秒）
    int timeSinceLastAttack;    // 距离上次攻击的时间

public:
    // 构造函数
    Machete();
    
    // 析构函数
    virtual ~Machete() = default;
    
    // 重写Item的方法
    virtual void use() override;
    virtual Machete* clone() const override;
    
    // 实现IWeaponAttack接口
    virtual AttackMethod getAttackMethod(WeaponAttackType type = WeaponAttackType::PRIMARY) const override;
    virtual AttackParams getAttackParams(WeaponAttackType type = WeaponAttackType::PRIMARY) const override;
    virtual bool canPerformAttack(WeaponAttackType type = WeaponAttackType::PRIMARY) const override;
    virtual void onAttackPerformed(WeaponAttackType type = WeaponAttackType::PRIMARY) override;
    
    // 砍刀特有方法
    void sharpen();  // 磨刀，恢复部分耐久度和攻击力
    bool isSharp() const; // 检查是否锋利
    bool isBroken() const; // 检查是否损坏
    
    // 更新冷却时间
    void updateCooldown(int deltaTimeMs);
    
    // 获取器
    float getDurability() const { return durability; }
    float getMaxDurability() const { return maxDurability; }
    float getDurabilityPercentage() const;
    int getCurrentCooldown() const { return currentCooldown; }
    int getComboCount() const { return comboCount; }
    int getMaxCombo() const { return maxCombo; }
    
    // 设置器
    void setDurability(float dur);
    void setCurrentCooldown(int cooldown) { currentCooldown = cooldown; }
    
    // 连击相关方法
    void resetCombo();
    bool canCombo() const;
};

#endif // MACHETE_H 