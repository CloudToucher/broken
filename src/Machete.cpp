#include "Machete.h"
#include <algorithm>

// 构造函数
Machete::Machete() 
    : Item("砍刀", 1.5f, 2.0f, 80.0f, 0),
      durability(100.0f),
      maxDurability(100.0f),
      durabilityLossPerHit(0.5f),
      currentCooldown(0),
      comboCount(0),
      maxCombo(3),
      comboResetTime(2000),
      timeSinceLastAttack(0) {
    
    // 设置砍刀特有属性
    setDescription("一把锋利的砍刀，适合砍击和清理植被。在近战中非常有效。");
    
    // 添加标志
    addFlag(ItemFlag::WEAPON);
    addFlag(ItemFlag::MELEE);
    addFlag(ItemFlag::TOOL); // 砍刀也是工具
    
    // 武器只有在手持时才发挥作用，不设置装备槽位
    
    // 设置伤害类型
    setSlashingDamage(35);              // 砍击伤害
    setPiercingDamage(5);               // 少量穿刺伤害
    
    // 设置攻击时间
    setAttackTime(0.8f);                // 攻击动作耗时0.8秒
    setStaminaCost(15);                 // 每次攻击消耗15点体力
}

// 重写use方法
void Machete::use() {
    // 砍刀的使用逻辑（比如磨刀）
    if (durability < maxDurability) {
        sharpen();
    }
}

// 重写克隆方法
Machete* Machete::clone() const {
    Machete* newMachete = new Machete();
    
    // 复制当前状态
    newMachete->durability = this->durability;
    newMachete->currentCooldown = this->currentCooldown;
    
    return newMachete;
}

// 实现IWeaponAttack接口
AttackMethod Machete::getAttackMethod(WeaponAttackType type) const {
    switch (type) {
        case WeaponAttackType::PRIMARY:
            return AttackMethod::MELEE_SLASH;  // 左键：砍击
        case WeaponAttackType::SECONDARY:
            return AttackMethod::MELEE_STAB;   // 右键：刺击
        default:
            return AttackMethod::MELEE_SLASH;
    }
}

AttackParams Machete::getAttackParams(WeaponAttackType type) const {
    AttackParams params;
    
    // 基础属性
    params.baseDamage = 35;
    params.range = 80.0f;
    params.speed = 1.2f;
    params.cooldownMs = static_cast<int>(1000.0f / params.speed);
    
    // 根据攻击类型设置不同的形状和参数
    switch (type) {
        case WeaponAttackType::PRIMARY: {
            // 左键：扇形攻击
            params.shape = AttackShape::SECTOR;
            params.angle = 1.047f; // 60度扇形（弧度）
            params.width = 60.0f;  // 不用于扇形，但保留
            params.damageType = "slash";
            params.canBleed = true;
            params.bleedChance = 0.4f + (comboCount * 0.1f);
            break;
        }
        case WeaponAttackType::SECONDARY: {
            // 右键：长条形攻击
            params.shape = AttackShape::RECTANGLE;
            params.angle = 0.0f;   // 不用于长条形
            params.width = 40.0f;  // 长条宽度
            params.range = 100.0f; // 更长的攻击距离
            params.baseDamage = 45; // 更高的基础伤害
            params.speed = 0.8f;   // 更慢的攻击速度
            params.cooldownMs = static_cast<int>(1000.0f / params.speed);
            params.damageType = "pierce";
            params.armorPenetration = 5; // 更高的穿甲
            params.canStun = true;
            params.stunChance = 0.3f;
            params.stunDuration = 1500;
            break;
        }
        default:
            params.shape = AttackShape::SECTOR;
            params.angle = 1.047f;
            params.damageType = "slash";
            break;
    }
    
    // 连击加成
    float comboMultiplier = 1.0f + (comboCount * 0.15f); // 每连击增加15%伤害
    params.baseDamage = static_cast<int>(params.baseDamage * comboMultiplier);
    
    // 连击速度加成
    if (comboCount > 0) {
        params.speed *= (1.0f + comboCount * 0.2f); // 每连击增加20%攻击速度
        params.cooldownMs = static_cast<int>(1000.0f / params.speed);
    }
    
    // 暴击属性（连击增加暴击率）
    params.criticalChance = 0.12f + (comboCount * 0.05f); // 每连击增加5%暴击率
    params.criticalMultiplier = 2.2f;
    
    // 连击增加穿甲
    params.armorPenetration += comboCount;
    
    // 音效和动画（根据攻击类型和连击数变化）
    if (type == WeaponAttackType::PRIMARY) {
        // 左键砍击音效
        if (comboCount == 0) {
            params.soundFile = "melee_slash1.wav";
            params.animationName = "machete_slash1";
        } else if (comboCount == 1) {
            params.soundFile = "melee_slash2.wav";
            params.animationName = "machete_slash2";
        } else {
            params.soundFile = "melee_slash3.wav";
            params.animationName = "machete_combo";
        }
    } else {
        // 右键刺击音效
        params.soundFile = "melee_stab.wav";
        params.animationName = "machete_stab";
    }
    params.animationDuration = 800 - (comboCount * 100); // 连击越多动画越快
    
    // 根据耐久度调整参数
    float durabilityModifier = getDurabilityPercentage();
    params.baseDamage = static_cast<int>(params.baseDamage * durabilityModifier);
    
    if (isBroken()) {
        params.baseDamage /= 4; // 损坏的武器伤害大幅降低
        params.canBleed = false;
        params.canStun = false;
    }
    
    return params;
}

bool Machete::canPerformAttack(WeaponAttackType type) const {
    return currentCooldown <= 0 && !isBroken();
}

void Machete::onAttackPerformed(WeaponAttackType type) {
    // 减少耐久度
    durability = std::max(0.0f, durability - durabilityLossPerHit);
    
    // 增加连击数
    if (comboCount < maxCombo) {
        comboCount++;
    }
    
    // 重置距离上次攻击的时间
    timeSinceLastAttack = 0;
    
    // 设置冷却时间（连击时冷却更短）
    float baseSpeed = 1.2f * (1.0f + comboCount * 0.2f);
    currentCooldown = static_cast<int>(1000.0f / baseSpeed);
}

// 更新冷却时间
void Machete::updateCooldown(int deltaTimeMs) {
    if (currentCooldown > 0) {
        currentCooldown = std::max(0, currentCooldown - deltaTimeMs);
    }
    
    // 更新距离上次攻击的时间
    timeSinceLastAttack += deltaTimeMs;
    
    // 如果太久没有攻击，重置连击
    if (timeSinceLastAttack >= comboResetTime) {
        resetCombo();
    }
}

// 磨刀方法
void Machete::sharpen() {
    // 恢复30%的耐久度
    float newDurability = std::min(maxDurability, durability + maxDurability * 0.3f);
    setDurability(newDurability);
}

// 检查是否锋利
bool Machete::isSharp() const {
    return getDurabilityPercentage() > 0.7f; // 耐久度超过70%算锋利
}

// 检查是否损坏
bool Machete::isBroken() const {
    return durability <= 0.0f;
}

// 获取耐久度百分比
float Machete::getDurabilityPercentage() const {
    return maxDurability > 0 ? durability / maxDurability : 0.0f;
}

// 设置耐久度
void Machete::setDurability(float dur) {
    durability = std::max(0.0f, std::min(maxDurability, dur));
}

// 重置连击
void Machete::resetCombo() {
    comboCount = 0;
    timeSinceLastAttack = 0;
}

// 检查是否可以连击
bool Machete::canCombo() const {
    return comboCount < maxCombo && timeSinceLastAttack < comboResetTime;
} 