#pragma once
#ifndef MELEE_WEAPON_H
#define MELEE_WEAPON_H

#include "Item.h"
#include "AttackSystem.h"
#include <map>
#include <vector>

// 攻击效果配置
struct AttackEffectConfig {
    std::string type;           // 效果类型：BLEEDING, POISON, STUN, KNOCKBACK
    float chance;               // 触发概率
    float duration;             // 持续时间（秒）
    float magnitude;            // 效果强度
    
    AttackEffectConfig() : type(""), chance(0.0f), duration(0.0f), magnitude(0.0f) {}
    AttackEffectConfig(const std::string& t, float c, float d, float m) 
        : type(t), chance(c), duration(d), magnitude(m) {}
};

// 攻击模式配置
struct AttackModeConfig {
    std::string shape;              // 攻击形状：SECTOR, RECTANGLE, LARGE_SECTOR
    float angle;                    // 角度（度）
    float range;                    // 范围
    float width;                    // 宽度
    float damageMultiplier;         // 伤害倍数
    std::vector<AttackEffectConfig> effects;  // 特殊效果列表
    
    AttackModeConfig() : shape("SECTOR"), angle(60.0f), range(80.0f), width(50.0f), damageMultiplier(1.0f) {}
};

// 通用近战武器类 - 支持刀剑、钝器、长柄武器等
class MeleeWeapon : public Item, public IWeaponAttack {
private:
    // 动态属性（从Item基础属性派生）
    float currentCooldown;        // 当前冷却时间
    int comboCount;              // 当前连击数
    float timeSinceLastAttack;   // 距离上次攻击的时间
    
    // 攻击模式配置
    std::map<std::string, AttackModeConfig> attackModes; // primary, secondary

public:
    // 构造函数
    MeleeWeapon(const std::string& name = "近战武器");
    virtual ~MeleeWeapon() = default;
    
    // 重写Item的方法
    virtual void use() override;
    virtual MeleeWeapon* clone() const override;
    
    // 实现IWeaponAttack接口
    virtual AttackMethod getAttackMethod(WeaponAttackType type = WeaponAttackType::PRIMARY) const override;
    virtual AttackParams getAttackParams(WeaponAttackType type = WeaponAttackType::PRIMARY) const override;
    virtual bool canPerformAttack(WeaponAttackType type = WeaponAttackType::PRIMARY) const override;
    virtual void onAttackPerformed(WeaponAttackType type = WeaponAttackType::PRIMARY) override;
    
    // 更新系统
    void updateCooldown(float deltaTimeMs);
    
    // 获取器
    float getCurrentCooldown() const { return currentCooldown; }
    int getComboCount() const { return comboCount; }
    
    // 攻击模式配置
    void setAttackMode(const std::string& mode, const AttackModeConfig& config);
    const AttackModeConfig& getAttackMode(const std::string& mode) const;
    bool hasAttackMode(const std::string& mode) const;
    
private:
    // 从Item属性获取武器数据
    float getWeaponDamage() const;
    float getWeaponRange() const;
    float getWeaponSpeed() const;
    float getWeaponCriticalChance() const;
    float getWeaponCriticalMultiplier() const;
    int getWeaponPenetration() const;
    
    // 从flag判断武器类型和特效
    AttackShape getPrimaryAttackShape() const;
    AttackShape getSecondaryAttackShape() const;
    bool hasSpecialEffect(const std::string& effectName) const;
    
    // 辅助方法
    AttackShape stringToAttackShape(const std::string& shapeStr) const;
    void applyAttackEffects(AttackParams& params, const std::vector<AttackEffectConfig>& effects) const;
    
    // 连击系统
    int getMaxCombo() const;
    float getComboWindow() const;
    void resetCombo();
};

#endif // MELEE_WEAPON_H 