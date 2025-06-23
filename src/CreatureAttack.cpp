#include "CreatureAttack.h"
#include "Creature.h" // 将在后面创建
#include <random>
#include <cmath>

// 构造函数
CreatureAttack::CreatureAttack(
    const std::string& attackName,
    AttackType attackType,
    int damage,
    int attackRange,
    int attackCooldown,
    float attackAccuracy,
    AttackEffect attackEffects
) : name(attackName),
    type(attackType),
    effects(attackEffects),
    baseDamage(damage),
    range(attackRange),
    cooldown(attackCooldown),
    currentCooldown(0),
    accuracy(attackAccuracy),
    critChance(0.05f),
    critMultiplier(2.0f),
    energyCost(0),
    staminaCost(0),
    soundFile(""),
    animationDuration(0),
    animationName("") {
}

// 执行攻击
bool CreatureAttack::execute(Creature* attacker, Entity* target) {
    // 检查是否可以攻击
    if (!canAttack()) {
        return false;
    }
    
    // 重置冷却时间
    currentCooldown = cooldown;
    
    // 调用开始攻击回调
    if (onStartCallback) {
        onStartCallback(attacker);
    }
    
    // 检查目标是否在范围内
    int dx = target->getX() - attacker->getX();
    int dy = target->getY() - attacker->getY();
    int distanceSquared = dx * dx + dy * dy;
    
    if (distanceSquared > range * range) {
        // 目标超出范围
        if (onMissCallback) {
            onMissCallback(attacker);
        }
        return false;
    }
    
    // 生成随机数，判断是否命中
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    float hitRoll = dis(gen);
    
    if (hitRoll > accuracy) {
        // 未命中
        if (onMissCallback) {
            onMissCallback(attacker);
        }
        return false;
    }
    
    // 计算伤害
    Damage damage = calculateDamage(attacker);
    
    // 造成伤害
    bool damageDealt = target->takeDamage(damage);
    
    // 应用攻击效果
    if (damageDealt) {
        applyEffects(attacker, target);
        
        // 调用命中回调
        if (onHitCallback) {
            onHitCallback(attacker, target);
        }
    }
    
    // 调用攻击结束回调
    if (onFinishCallback) {
        onFinishCallback(attacker);
    }
    
    return damageDealt;
}

// 更新冷却时间
void CreatureAttack::updateCooldown(int deltaTimeMs) {
    if (currentCooldown > 0) {
        currentCooldown = std::max(0, currentCooldown - deltaTimeMs);
    }
}

// 检查是否可以攻击
bool CreatureAttack::canAttack() const {
    return currentCooldown <= 0;
}

// 计算实际伤害
Damage CreatureAttack::calculateDamage(Creature* attacker) const {
    // 基础伤害
    int finalDamage = baseDamage;
    
    // 根据攻击者的属性调整伤害
    switch (type) {
        case AttackType::MELEE:
        case AttackType::SLAM:
            // 近战攻击基于力量
            finalDamage += attacker->getStrength() / 2;
            break;
            
        case AttackType::CLAW:
        case AttackType::BITE:
            // 爪击和咬击基于力量和敏捷的组合
            finalDamage += (attacker->getStrength() + attacker->getDexterity()) / 3;
            break;
            
        case AttackType::RANGED:
            // 远程攻击基于敏捷和感知
            finalDamage += (attacker->getDexterity() + attacker->getPerception()) / 3;
            break;
            
        case AttackType::GRAB:
            // 抓取攻击基于力量
            finalDamage += attacker->getStrength() / 3;
            break;
            
        case AttackType::SPECIAL:
            // 特殊攻击基于智力
            finalDamage += attacker->getIntelligence() / 2;
            break;
    }
    
    // 检查暴击
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    float critRoll = dis(gen);
    
    bool isCrit = critRoll < critChance;
    if (isCrit) {
        finalDamage = static_cast<int>(finalDamage * critMultiplier);
    }
    
    // 创建伤害对象
    Damage damageObj;
    
    // 根据攻击类型添加不同类型的伤害
    switch (type) {
        case AttackType::MELEE:
            damageObj.addDamage("blunt", finalDamage, 0);
            break;
            
        case AttackType::SLAM:
            damageObj.addDamage("blunt", finalDamage, 2);
            break;
            
        case AttackType::CLAW:
            damageObj.addDamage("slash", finalDamage, 1);
            break;
            
        case AttackType::BITE:
            damageObj.addDamage("pierce", finalDamage / 2, 3);
            damageObj.addDamage("slash", finalDamage / 2, 1);
            break;
            
        case AttackType::RANGED:
            damageObj.addDamage("pierce", finalDamage, 2);
            break;
            
        case AttackType::GRAB:
            damageObj.addDamage("blunt", finalDamage, 0);
            break;
            
        case AttackType::SPECIAL:
            damageObj.addDamage("pure", finalDamage, 5);
            break;
    }
    
    return damageObj;
}

// 应用攻击效果
void CreatureAttack::applyEffects(Creature* attacker, Entity* target) const {
    // 根据攻击效果添加状态效果
    
    // 流血效果
    if (static_cast<int>(effects & AttackEffect::BLEEDING) > 0) {
        // 添加流血状态，持续10秒，每秒造成基础伤害的10%
        target->addState(EntityStateEffect::Type::DEBUFFED, "Bleeding", 10000, 1);
        // 具体的流血效果逻辑将在后续实现
    }
    
    // 中毒效果
    if (static_cast<int>(effects & AttackEffect::POISON) > 0) {
        // 添加中毒状态，持续15秒
        target->addState(EntityStateEffect::Type::DEBUFFED, "Poisoned", 15000, 1);
        // 具体的中毒效果逻辑将在后续实现
    }
    
    // 眩晕效果
    if (static_cast<int>(effects & AttackEffect::STUN) > 0) {
        // 添加眩晕状态，持续2秒
        target->addState(EntityStateEffect::Type::STUNNED, "Stunned", 2000, 3);
    }
    
    // 击退效果
    if (static_cast<int>(effects & AttackEffect::KNOCKBACK) > 0) {
        // 击退效果逻辑将在后续实现
    }
    
    // 感染效果
    if (static_cast<int>(effects & AttackEffect::INFECTION) > 0) {
        // 添加感染状态，持续30秒
        target->addState(EntityStateEffect::Type::DEBUFFED, "Infected", 30000, 1);
        // 具体的感染效果逻辑将在后续实现
    }
    
    // 定身效果
    if (static_cast<int>(effects & AttackEffect::IMMOBILIZE) > 0) {
        // 添加定身状态，持续3秒
        target->addState(EntityStateEffect::Type::DEBUFFED, "Immobilized", 3000, 2);
        // 具体的定身效果逻辑将在后续实现
    }
    
    // 虚弱效果
    if (static_cast<int>(effects & AttackEffect::WEAKEN) > 0) {
        // 添加虚弱状态，持续8秒
        target->addState(EntityStateEffect::Type::DEBUFFED, "Weakened", 8000, 1);
        // 具体的虚弱效果逻辑将在后续实现
    }
    
    // 恐惧效果
    if (static_cast<int>(effects & AttackEffect::FEAR) > 0) {
        // 添加恐惧状态，持续5秒
        target->addState(EntityStateEffect::Type::DEBUFFED, "Feared", 5000, 2);
        // 具体的恐惧效果逻辑将在后续实现
    }
} 