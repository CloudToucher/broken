#include "MeleeWeapon.h"
#include <algorithm>
#include <cmath>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 构造函数
MeleeWeapon::MeleeWeapon(const std::string& name) 
    : Item(name),
      currentCooldown(0.0f),
      comboCount(0),
      timeSinceLastAttack(0.0f) {
    
    // 添加基础标志
    addFlag(ItemFlag::WEAPON);
    addFlag(ItemFlag::MELEE);
}

// 重写use方法
void MeleeWeapon::use() {
    // 近战武器的使用逻辑（比如修理、保养等）
    std::cout << "使用了 " << getName() << std::endl;
}

// 重写克隆方法
MeleeWeapon* MeleeWeapon::clone() const {
    MeleeWeapon* newWeapon = new MeleeWeapon(getName());
    
    // 复制所有Item属性
    newWeapon->setWeight(getWeight());
    newWeapon->setVolume(getVolume());
    newWeapon->setLength(getLength());
    newWeapon->setValue(getValue());
    newWeapon->setDescription(getDescription());
    newWeapon->setRarity(getRarity());
    
    // 复制伤害属性
    newWeapon->setPiercingDamage(getPiercingDamage());
    newWeapon->setBluntDamage(getBluntDamage());
    newWeapon->setSlashingDamage(getSlashingDamage());
    newWeapon->setAttackTime(getAttackTime());
    newWeapon->setStaminaCost(getStaminaCost());
    
    // 复制所有标志
    for (int i = 0; i < static_cast<int>(ItemFlag::FLAG_COUNT); ++i) {
        ItemFlag flag = static_cast<ItemFlag>(i);
        if (hasFlag(flag)) {
            newWeapon->addFlag(flag);
        }
    }
    
    // 复制装备槽位
    for (const auto& slot : getEquipSlots()) {
        newWeapon->addEquipSlot(slot);
    }
    
    // 复制攻击模式配置
    for (const auto& [mode, config] : attackModes) {
        newWeapon->setAttackMode(mode, config);
    }
    
    return newWeapon;
}

// 获取攻击方法
AttackMethod MeleeWeapon::getAttackMethod(WeaponAttackType type) const {
    // 根据flag判断攻击方法
    if (hasFlag(ItemFlag::SWORD)) {
        return type == WeaponAttackType::PRIMARY ? AttackMethod::MELEE_SLASH : AttackMethod::MELEE_STAB;
    } else if (hasFlag(ItemFlag::AXE)) {
        return type == WeaponAttackType::PRIMARY ? AttackMethod::MELEE_SLASH : AttackMethod::MELEE_CRUSH;
    } else if (hasFlag(ItemFlag::HAMMER)) {
        return type == WeaponAttackType::PRIMARY ? AttackMethod::MELEE_CRUSH : AttackMethod::MELEE_CRUSH;
    } else if (hasFlag(ItemFlag::SPEAR)) {
        return type == WeaponAttackType::PRIMARY ? AttackMethod::MELEE_STAB : AttackMethod::MELEE_SLASH;
    } else if (hasFlag(ItemFlag::DAGGER)) {
        return type == WeaponAttackType::PRIMARY ? AttackMethod::MELEE_QUICK : AttackMethod::MELEE_STAB;
    }
    
    // 默认：砍击和刺击
    return type == WeaponAttackType::PRIMARY ? AttackMethod::MELEE_SLASH : AttackMethod::MELEE_STAB;
}

// 获取攻击参数
AttackParams MeleeWeapon::getAttackParams(WeaponAttackType type) const {
    AttackParams params;
    
    // 从Item属性获取基础数据
    float baseDamage = getWeaponDamage();
    params.speed = getWeaponSpeed();
    params.criticalChance = getWeaponCriticalChance();
    params.criticalMultiplier = getWeaponCriticalMultiplier();
    params.armorPenetration = getWeaponPenetration();
    
    // 冷却时间基于攻击速度
    params.cooldownMs = static_cast<int>(1000.0f / params.speed);
    
    // 确定攻击模式
    std::string modeKey = (type == WeaponAttackType::PRIMARY) ? "primary" : "secondary";
    
    if (hasAttackMode(modeKey)) {
        // 从配置中获取攻击参数
        const auto& config = getAttackMode(modeKey);
        
        // 设置形状和几何参数
        params.shape = stringToAttackShape(config.shape);
        params.angle = config.angle * (M_PI / 180.0f); // 度转弧度
        params.range = config.range;
        params.width = config.width;
        
        // 应用伤害倍数
        params.baseDamage = static_cast<int>(baseDamage * config.damageMultiplier);
        
        // 应用特殊效果
        applyAttackEffects(params, config.effects);
        
    } else {
        // 使用默认配置（向后兼容）
        params.baseDamage = static_cast<int>(baseDamage);
        params.range = getWeaponRange();
        params.shape = (type == WeaponAttackType::PRIMARY) ? getPrimaryAttackShape() : getSecondaryAttackShape();
        params.angle = 1.047f; // 60度默认
        params.width = params.range * 0.6f;
        
        if (type == WeaponAttackType::SECONDARY) {
            params.range *= 1.2f;
            params.baseDamage = static_cast<int>(params.baseDamage * 1.15f);
        }
    }
    
    // 连击加成
    if (comboCount > 0) {
        float comboMultiplier = 1.0f + (comboCount * 0.15f); // 每连击增加15%伤害
        params.baseDamage = static_cast<int>(params.baseDamage * comboMultiplier);
        params.speed *= (1.0f + comboCount * 0.2f); // 每连击增加20%速度
        params.cooldownMs = static_cast<int>(1000.0f / params.speed);
        params.criticalChance += comboCount * 0.05f; // 每连击增加5%暴击率
        
        // 连击对特殊效果的影响
        if (params.canBleed) {
            params.bleedChance += comboCount * 0.1f; // 连击增加流血概率
        }
        if (params.canPoison) {
            params.poisonChance += comboCount * 0.05f; // 连击增加中毒概率
        }
    }
    
    // 设置伤害类型
    if (hasFlag(ItemFlag::SWORD) || hasFlag(ItemFlag::AXE)) {
        params.damageType = "slash";
    } else if (hasFlag(ItemFlag::SPEAR) || hasFlag(ItemFlag::DAGGER)) {
        params.damageType = "pierce";
    } else if (hasFlag(ItemFlag::HAMMER)) {
        params.damageType = "blunt";
    } else {
        params.damageType = "slash"; // 默认
    }
    
    return params;
}

// 检查是否可以攻击
bool MeleeWeapon::canPerformAttack(WeaponAttackType type) const {
    return currentCooldown <= 0;
}

// 攻击执行后处理
void MeleeWeapon::onAttackPerformed(WeaponAttackType type) {
    // 设置冷却时间
    currentCooldown = 1000.0f / getWeaponSpeed();
    
    // 连击处理
    if (comboCount < getMaxCombo()) {
        comboCount++;
    }
    
    // 重置距离上次攻击的时间
    timeSinceLastAttack = 0.0f;
}

// 更新冷却时间
void MeleeWeapon::updateCooldown(float deltaTimeMs) {
    if (currentCooldown > 0) {
        currentCooldown = std::max(0.0f, currentCooldown - deltaTimeMs);
    }
    
    // 更新距离上次攻击的时间
    timeSinceLastAttack += deltaTimeMs;
    
    // 检查连击重置
    if (timeSinceLastAttack >= getComboWindow() * 1000.0f) {
        resetCombo();
    }
}

// 从Item属性获取武器数据
float MeleeWeapon::getWeaponDamage() const {
    // 优先使用对应类型的伤害
    if (hasFlag(ItemFlag::SWORD) || hasFlag(ItemFlag::AXE)) {
        return getSlashingDamage() > 0 ? getSlashingDamage() : 35.0f;
    } else if (hasFlag(ItemFlag::SPEAR) || hasFlag(ItemFlag::DAGGER)) {
        return getPiercingDamage() > 0 ? getPiercingDamage() : 30.0f;
    } else if (hasFlag(ItemFlag::HAMMER)) {
        return getBluntDamage() > 0 ? getBluntDamage() : 45.0f;
    }
    
    // 使用最高的伤害值
    float maxDamage = std::max({getSlashingDamage(), getPiercingDamage(), getBluntDamage()});
    return maxDamage > 0 ? maxDamage : 25.0f; // 默认值
}

float MeleeWeapon::getWeaponRange() const {
    // 从length属性计算范围，单位转换为像素
    float baseRange = getLength() * 200.0f; // 0.4m -> 80px
    
    // 根据武器类型调整
    if (hasFlag(ItemFlag::SPEAR)) {
        baseRange *= 1.5f; // 长柄武器范围更远
    } else if (hasFlag(ItemFlag::DAGGER)) {
        baseRange *= 0.7f; // 匕首范围更近
    }
    
    return baseRange > 0 ? baseRange : 80.0f; // 默认80像素
}

float MeleeWeapon::getWeaponSpeed() const {
    // 从攻击时间计算速度
    float attackTime = getAttackTime();
    if (attackTime > 0) {
        return 1.0f / attackTime; // 攻击时间越短，速度越快
    }
    
    // 根据重量计算速度
    float weight = getWeight();
    float baseSpeed = 2.0f / (1.0f + weight); // 重量越大，速度越慢
    
    // 根据武器类型调整
    if (hasFlag(ItemFlag::DAGGER)) {
        baseSpeed *= 1.5f; // 匕首更快
    } else if (hasFlag(ItemFlag::HAMMER)) {
        baseSpeed *= 0.7f; // 锤子更慢
    }
    
    return std::max(0.5f, baseSpeed); // 最小0.5/秒
}

float MeleeWeapon::getWeaponCriticalChance() const {
    // 根据武器类型设置暴击率
    if (hasFlag(ItemFlag::DAGGER)) {
        return 0.20f; // 匕首暴击率高
    } else if (hasFlag(ItemFlag::SWORD)) {
        return 0.15f; // 剑类中等暴击率
    } else if (hasFlag(ItemFlag::HAMMER)) {
        return 0.08f; // 锤子暴击率低但倍数高
    }
    
    return 0.12f; // 默认暴击率
}

float MeleeWeapon::getWeaponCriticalMultiplier() const {
    // 根据武器类型设置暴击倍数
    if (hasFlag(ItemFlag::HAMMER)) {
        return 3.0f; // 锤子暴击倍数最高
    } else if (hasFlag(ItemFlag::AXE)) {
        return 2.5f; // 斧头暴击倍数高
    } else if (hasFlag(ItemFlag::DAGGER)) {
        return 1.8f; // 匕首暴击倍数低但频率高
    }
    
    return 2.2f; // 默认暴击倍数
}

int MeleeWeapon::getWeaponPenetration() const {
    // 根据武器类型设置穿甲
    if (hasFlag(ItemFlag::SPEAR)) {
        return 10; // 长矛穿甲高
    } else if (hasFlag(ItemFlag::DAGGER)) {
        return 8; // 匕首穿甲中等
    } else if (hasFlag(ItemFlag::HAMMER)) {
        return 15; // 锤子穿甲很高
    }
    
    return 5; // 默认穿甲
}

// 获取攻击形状
AttackShape MeleeWeapon::getPrimaryAttackShape() const {
    // 大部分近战武器主攻击都是扇形
    return AttackShape::SECTOR;
}

AttackShape MeleeWeapon::getSecondaryAttackShape() const {
    // 副攻击通常是直线攻击
    return AttackShape::RECTANGLE;
}

// 检查特殊效果
bool MeleeWeapon::hasSpecialEffect(const std::string& effectName) const {
    // 通过flag检查特殊效果
    if (effectName == "bleeding") {
        return hasFlag(ItemFlag::SWORD) || hasFlag(ItemFlag::AXE) || hasFlag(ItemFlag::DAGGER);
    } else if (effectName == "stunning") {
        return hasFlag(ItemFlag::HAMMER);
    } else if (effectName == "knockback") {
        return hasFlag(ItemFlag::HAMMER) || hasFlag(ItemFlag::AXE);
    }
    
    return false;
}

// 连击系统
int MeleeWeapon::getMaxCombo() const {
    // 根据武器类型设置最大连击数
    if (hasFlag(ItemFlag::DAGGER)) {
        return 5; // 匕首连击最多
    } else if (hasFlag(ItemFlag::SWORD)) {
        return 3; // 剑类中等连击
    } else if (hasFlag(ItemFlag::HAMMER)) {
        return 1; // 锤子不支持连击
    }
    
    return 3; // 默认连击数
}

float MeleeWeapon::getComboWindow() const {
    // 连击窗口时间（秒）
    return 2.0f;
}

void MeleeWeapon::resetCombo() {
    comboCount = 0;
    timeSinceLastAttack = 0.0f;
}

// 攻击模式配置方法
void MeleeWeapon::setAttackMode(const std::string& mode, const AttackModeConfig& config) {
    attackModes[mode] = config;
}

const AttackModeConfig& MeleeWeapon::getAttackMode(const std::string& mode) const {
    static AttackModeConfig defaultConfig;
    auto it = attackModes.find(mode);
    if (it != attackModes.end()) {
        return it->second;
    }
    return defaultConfig;
}

bool MeleeWeapon::hasAttackMode(const std::string& mode) const {
    return attackModes.find(mode) != attackModes.end();
}

// 辅助方法
AttackShape MeleeWeapon::stringToAttackShape(const std::string& shapeStr) const {
    if (shapeStr == "SECTOR") return AttackShape::SECTOR;
    if (shapeStr == "RECTANGLE") return AttackShape::RECTANGLE;
    if (shapeStr == "LARGE_SECTOR") return AttackShape::LARGE_SECTOR;
    if (shapeStr == "CIRCLE") return AttackShape::CIRCLE;
    if (shapeStr == "LINE") return AttackShape::LINE;
    return AttackShape::SECTOR; // 默认
}

void MeleeWeapon::applyAttackEffects(AttackParams& params, const std::vector<AttackEffectConfig>& effects) const {
    for (const auto& effect : effects) {
        if (effect.type == "BLEEDING") {
            params.canBleed = true;
            params.bleedChance = effect.chance;
        } else if (effect.type == "POISON") {
            params.canPoison = true;
            params.poisonChance = effect.chance;
            params.poisonDuration = static_cast<int>(effect.duration * 1000); // 转换为毫秒
            params.poisonDamage = effect.magnitude;
        } else if (effect.type == "STUN") {
            params.canStun = true;
            params.stunChance = effect.chance;
            params.stunDuration = static_cast<int>(effect.duration * 1000); // 转换为毫秒
        } else if (effect.type == "KNOCKBACK") {
            params.canKnockback = true;
            params.knockbackChance = effect.chance;
            params.knockbackForce = effect.magnitude;
        }
    }
} 