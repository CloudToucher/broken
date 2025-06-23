#include "Weapon.h"
#include "Entity.h"
#include "SoundManager.h"
#include <algorithm>
#include <random>
#include <iostream>
#include <sstream>

// 静态成员初始化
SpecialEffectManager* SpecialEffectManager::instance = nullptr;

// Weapon类实现
Weapon::Weapon(const std::string& name) : Item(name) {
    // 初始化默认值
    weaponType = WeaponType::MELEE;
    primaryAttackMethod = AttackMethod::MELEE_SLASH;
    availableAttackMethods.push_back(AttackMethod::MELEE_SLASH);
    
    baseDamage = 10.0f;
    range = 50.0f;
    attackSpeed = 1.0f;
    criticalChance = 0.05f;
    criticalMultiplier = 2.0f;
    
    accuracy = 0.95f;
    penetration = 0.0f;
    
    maxDurability = 100;
    currentDurability = 100;
    durabilityLossPerUse = 1.0f;
    
    supportsCombo = false;
    maxComboCount = 1;
    comboWindow = 2.0f;
    comboDamageBonus = 0.15f;
    
    animationSpeed = 1.0f;
    
    requiredStrength = 0;
    requiredDexterity = 0;
    requiredIntelligence = 0;
    
    // 添加武器标志
    addFlag(ItemFlag::WEAPON);
}

Item* Weapon::clone() const {
    Weapon* newWeapon = new Weapon(getName());
    
    // 复制基本Item属性
    newWeapon->setWeight(getWeight());
    newWeapon->setVolume(getVolume());
    newWeapon->setLength(getLength());
    newWeapon->setValue(getValue());
    newWeapon->setDescription(getDescription());
    
    // 复制装备槽位
    for (const auto& slot : getEquipSlots()) {
        newWeapon->addEquipSlot(slot);
    }
    
    // 复制标签 - 遍历所有可能的ItemFlag枚举值
    // 从WEARABLE到FLASHLIGHT，确保包含所有标签
    for (int i = static_cast<int>(ItemFlag::WEARABLE); i <= static_cast<int>(ItemFlag::FLASHLIGHT); ++i) {
        ItemFlag flag = static_cast<ItemFlag>(i);
        if (hasFlag(flag)) {
            newWeapon->addFlag(flag);
            std::cout << "复制标签到新武器 " << newWeapon->getName() << ": " << GetItemFlagName(flag) << std::endl;
        }
    }
    
    // 复制武器特有属性
    newWeapon->weaponType = weaponType;
    newWeapon->primaryAttackMethod = primaryAttackMethod;
    newWeapon->availableAttackMethods = availableAttackMethods;
    
    newWeapon->baseDamage = baseDamage;
    newWeapon->range = range;
    newWeapon->attackSpeed = attackSpeed;
    newWeapon->criticalChance = criticalChance;
    newWeapon->criticalMultiplier = criticalMultiplier;
    
    newWeapon->accuracy = accuracy;
    newWeapon->penetration = penetration;
    
    newWeapon->maxDurability = maxDurability;
    newWeapon->currentDurability = currentDurability;
    newWeapon->durabilityLossPerUse = durabilityLossPerUse;
    
    newWeapon->specialEffects = specialEffects;
    
    newWeapon->supportsCombo = supportsCombo;
    newWeapon->maxComboCount = maxComboCount;
    newWeapon->comboWindow = comboWindow;
    newWeapon->comboDamageBonus = comboDamageBonus;
    
    newWeapon->attackSound = attackSound;
    newWeapon->hitSound = hitSound;
    newWeapon->criticalSound = criticalSound;
    newWeapon->comboSound = comboSound;
    
    newWeapon->animationSpeed = animationSpeed;
    newWeapon->animationName = animationName;
    
    newWeapon->requiredStrength = requiredStrength;
    newWeapon->requiredDexterity = requiredDexterity;
    newWeapon->requiredIntelligence = requiredIntelligence;
    
    return newWeapon;
}

AttackParams Weapon::getAttackParams(WeaponAttackType type) const {
    AttackParams params;
    
    // 设置基础攻击参数
    params.baseDamage = static_cast<int>(getEffectiveDamage());
    params.range = range;
    params.speed = getEffectiveAttackSpeed();
    params.cooldownMs = static_cast<int>(1000.0f / attackSpeed);
    params.criticalChance = criticalChance;
    params.criticalMultiplier = criticalMultiplier;
    params.armorPenetration = static_cast<int>(penetration);
    
    // 设置音效和动画
    params.soundFile = attackSound;
    params.animationName = animationName;
    params.animationDuration = static_cast<int>(1000.0f / animationSpeed);
    
    // 设置特殊效果
    for (const auto& effect : specialEffects) {
        if (effect.type == SpecialEffectType::KNOCKBACK) {
            // 这里可以设置击退相关的参数
        } else if (effect.type == SpecialEffectType::STUN) {
            params.canStun = true;
            params.stunChance = effect.chance;
            params.stunDuration = static_cast<int>(effect.duration * 1000);
        }
        // 可以继续添加其他效果的映射
    }
    
    return params;
}

bool Weapon::canPerformAttack(WeaponAttackType type) const {
    if (isBroken()) return false;
    
    // 检查属性需求（这里需要Entity有获取属性的方法）
    // 暂时简化处理
    return true;
}

void Weapon::onAttackPerformed(WeaponAttackType type) {
    // 攻击后减少耐久度
    reduceDurability(durabilityLossPerUse);
    
    // 播放攻击音效
    if (!attackSound.empty()) {
        SoundManager::getInstance()->playSound(attackSound);
    }
}

void Weapon::addAttackMethod(AttackMethod method) {
    if (std::find(availableAttackMethods.begin(), availableAttackMethods.end(), method) == availableAttackMethods.end()) {
        availableAttackMethods.push_back(method);
    }
}

void Weapon::removeAttackMethod(AttackMethod method) {
    availableAttackMethods.erase(
        std::remove(availableAttackMethods.begin(), availableAttackMethods.end(), method),
        availableAttackMethods.end()
    );
}

bool Weapon::hasAttackMethod(AttackMethod method) const {
    return std::find(availableAttackMethods.begin(), availableAttackMethods.end(), method) != availableAttackMethods.end();
}

float Weapon::getDurabilityPercentage() const {
    if (maxDurability <= 0) return 1.0f;
    return static_cast<float>(currentDurability) / static_cast<float>(maxDurability);
}

void Weapon::reduceDurability(float amount) {
    currentDurability = std::max(0, currentDurability - static_cast<int>(amount));
}

void Weapon::repairWeapon(int amount) {
    currentDurability = std::min(maxDurability, currentDurability + amount);
}

void Weapon::addSpecialEffect(const SpecialEffect& effect) {
    // 检查是否已存在相同类型的效果
    for (auto& existing : specialEffects) {
        if (existing.type == effect.type) {
            existing = effect; // 覆盖现有效果
            return;
        }
    }
    specialEffects.push_back(effect);
}

void Weapon::removeSpecialEffect(SpecialEffectType type) {
    specialEffects.erase(
        std::remove_if(specialEffects.begin(), specialEffects.end(),
            [type](const SpecialEffect& effect) { return effect.type == type; }),
        specialEffects.end()
    );
}

bool Weapon::hasSpecialEffect(SpecialEffectType type) const {
    return std::any_of(specialEffects.begin(), specialEffects.end(),
        [type](const SpecialEffect& effect) { return effect.type == type; });
}

float Weapon::getEffectiveDamage() const {
    return baseDamage * calculateDurabilityModifier();
}

float Weapon::getEffectiveAccuracy() const {
    return accuracy * calculateDurabilityModifier();
}

float Weapon::getEffectiveAttackSpeed() const {
    return attackSpeed * calculateDurabilityModifier();
}

bool Weapon::canBeUsedBy(Entity* entity) const {
    if (!entity) return false;
    
    // 检查属性需求（这里需要Entity有获取属性的方法）
    // 暂时简化处理
    return true;
}

std::string Weapon::getWeaponTypeString() const {
    switch (weaponType) {
        case WeaponType::MELEE: return "近战武器";
        case WeaponType::RANGED: return "远程武器";
        case WeaponType::THROWN: return "投掷武器";
        case WeaponType::SPECIAL: return "特殊武器";
        default: return "未知武器";
    }
}

std::string Weapon::getDetailedInfo() const {
    std::stringstream ss;
    ss << getName() << " (" << getWeaponTypeString() << ")\n";
    ss << "伤害: " << getEffectiveDamage() << "\n";
    ss << "范围: " << range << "\n";
    ss << "攻击速度: " << getEffectiveAttackSpeed() << "\n";
    ss << "暴击率: " << (criticalChance * 100) << "%\n";
    ss << "耐久度: " << currentDurability << "/" << maxDurability << "\n";
    
    if (!specialEffects.empty()) {
        ss << "特殊效果:\n";
        for (const auto& effect : specialEffects) {
            ss << "- " << SpecialEffectManager::effectTypeToString(effect.type) 
               << " (" << (effect.chance * 100) << "%)\n";
        }
    }
    
    return ss.str();
}

void Weapon::applySpecialEffects(Entity* target) {
    if (!target) return;
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    
    for (const auto& effect : specialEffects) {
        if (dis(gen) <= effect.chance) {
            SpecialEffectManager::getInstance()->applyEffect(nullptr, target, effect);
        }
    }
}

float Weapon::calculateDurabilityModifier() const {
    float durabilityRatio = getDurabilityPercentage();
    
    // 耐久度低于50%时开始影响性能
    if (durabilityRatio < 0.5f) {
        return 0.5f + durabilityRatio; // 最低50%效果
    }
    
    return 1.0f;
}

// SpecialEffectManager实现
SpecialEffectManager* SpecialEffectManager::getInstance() {
    if (instance == nullptr) {
        instance = new SpecialEffectManager();
    }
    return instance;
}

void SpecialEffectManager::registerCustomEffect(const std::string& name, 
    std::function<void(Entity*, Entity*, const SpecialEffect&)> effect) {
    customEffects[name] = effect;
}

void SpecialEffectManager::applyEffect(Entity* attacker, Entity* target, const SpecialEffect& effect) {
    if (!target) return;
    
    switch (effect.type) {
        case SpecialEffectType::POISON:
            applyPoisonEffect(target, effect);
            break;
        case SpecialEffectType::FIRE:
            applyFireEffect(target, effect);
            break;
        case SpecialEffectType::FREEZE:
            applyFreezeEffect(target, effect);
            break;
        case SpecialEffectType::ELECTRIC:
            applyElectricEffect(target, effect);
            break;
        case SpecialEffectType::VAMPIRE:
            applyVampireEffect(attacker, target, effect);
            break;
        case SpecialEffectType::KNOCKBACK:
            applyKnockbackEffect(attacker, target, effect);
            break;
        case SpecialEffectType::STUN:
            applyStunEffect(target, effect);
            break;
        case SpecialEffectType::ARMOR_PIERCE:
            applyArmorPierceEffect(target, effect);
            break;
        case SpecialEffectType::EXPLOSIVE:
            applyExplosiveEffect(attacker, target, effect);
            break;
        case SpecialEffectType::CUSTOM:
            if (!effect.customName.empty() && customEffects.find(effect.customName) != customEffects.end()) {
                customEffects[effect.customName](attacker, target, effect);
            }
            break;
        default:
            break;
    }
}

SpecialEffectType SpecialEffectManager::parseEffectType(const std::string& typeStr) {
    if (typeStr == "POISON") return SpecialEffectType::POISON;
    if (typeStr == "FIRE") return SpecialEffectType::FIRE;
    if (typeStr == "FREEZE") return SpecialEffectType::FREEZE;
    if (typeStr == "ELECTRIC") return SpecialEffectType::ELECTRIC;
    if (typeStr == "VAMPIRE") return SpecialEffectType::VAMPIRE;
    if (typeStr == "KNOCKBACK") return SpecialEffectType::KNOCKBACK;
    if (typeStr == "STUN") return SpecialEffectType::STUN;
    if (typeStr == "ARMOR_PIERCE") return SpecialEffectType::ARMOR_PIERCE;
    if (typeStr == "MULTI_HIT") return SpecialEffectType::MULTI_HIT;
    if (typeStr == "CHAIN_ATTACK") return SpecialEffectType::CHAIN_ATTACK;
    if (typeStr == "EXPLOSIVE") return SpecialEffectType::EXPLOSIVE;
    if (typeStr == "CUSTOM") return SpecialEffectType::CUSTOM;
    
    return SpecialEffectType::NONE;
}

std::string SpecialEffectManager::effectTypeToString(SpecialEffectType type) {
    switch (type) {
        case SpecialEffectType::POISON: return "中毒";
        case SpecialEffectType::FIRE: return "燃烧";
        case SpecialEffectType::FREEZE: return "冰冻";
        case SpecialEffectType::ELECTRIC: return "电击";
        case SpecialEffectType::VAMPIRE: return "吸血";
        case SpecialEffectType::KNOCKBACK: return "击退";
        case SpecialEffectType::STUN: return "眩晕";
        case SpecialEffectType::ARMOR_PIERCE: return "破甲";
        case SpecialEffectType::MULTI_HIT: return "多重攻击";
        case SpecialEffectType::CHAIN_ATTACK: return "连锁攻击";
        case SpecialEffectType::EXPLOSIVE: return "爆炸";
        case SpecialEffectType::CUSTOM: return "自定义";
        default: return "无";
    }
}

// 内置特殊效果实现
void SpecialEffectManager::applyPoisonEffect(Entity* target, const SpecialEffect& effect) {
    std::cout << "应用中毒效果: 持续时间=" << effect.duration << "s, 强度=" << effect.magnitude << std::endl;
    // 这里应该调用Entity的状态效果系统
    // target->addStatusEffect(StatusEffectType::POISON, effect.duration, effect.magnitude);
}

void SpecialEffectManager::applyFireEffect(Entity* target, const SpecialEffect& effect) {
    std::cout << "应用燃烧效果: 持续时间=" << effect.duration << "s, 强度=" << effect.magnitude << std::endl;
    // target->addStatusEffect(StatusEffectType::BURNING, effect.duration, effect.magnitude);
}

void SpecialEffectManager::applyFreezeEffect(Entity* target, const SpecialEffect& effect) {
    std::cout << "应用冰冻效果: 持续时间=" << effect.duration << "s" << std::endl;
    // target->addStatusEffect(StatusEffectType::FROZEN, effect.duration, 1.0f);
}

void SpecialEffectManager::applyElectricEffect(Entity* target, const SpecialEffect& effect) {
    std::cout << "应用电击效果: 伤害=" << effect.magnitude << std::endl;
    // target->takeDamage(effect.magnitude, DamageType::ELECTRIC);
}

void SpecialEffectManager::applyVampireEffect(Entity* attacker, Entity* target, const SpecialEffect& effect) {
    std::cout << "应用吸血效果: 回复=" << effect.magnitude << std::endl;
    // if (attacker) {
    //     attacker->heal(effect.magnitude);
    // }
}

void SpecialEffectManager::applyKnockbackEffect(Entity* attacker, Entity* target, const SpecialEffect& effect) {
    std::cout << "应用击退效果: 力度=" << effect.magnitude << std::endl;
    // 计算击退方向和距离
    // target->applyKnockback(direction, effect.magnitude);
}

void SpecialEffectManager::applyStunEffect(Entity* target, const SpecialEffect& effect) {
    std::cout << "应用眩晕效果: 持续时间=" << effect.duration << "s" << std::endl;
    // target->addStatusEffect(StatusEffectType::STUNNED, effect.duration, 1.0f);
}

void SpecialEffectManager::applyArmorPierceEffect(Entity* target, const SpecialEffect& effect) {
    std::cout << "应用破甲效果: 穿透=" << effect.magnitude << std::endl;
    // 这个效果通常在伤害计算时处理，而不是作为状态效果
}

void SpecialEffectManager::applyExplosiveEffect(Entity* attacker, Entity* target, const SpecialEffect& effect) {
    std::cout << "应用爆炸效果: 范围=" << effect.magnitude << std::endl;
    // 在目标位置创建爆炸，影响范围内的所有实体
} 