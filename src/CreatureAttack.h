#pragma once
#ifndef CREATURE_ATTACK_H
#define CREATURE_ATTACK_H

#include <string>
#include <functional>
#include "Damage.h"

// 前向声明
class Creature;
class Entity;

// 攻击类型枚举
enum class AttackType {
    MELEE,          // 近战攻击
    RANGED,         // 远程攻击
    GRAB,           // 抓取攻击
    BITE,           // 咬击攻击
    SLAM,           // 猛击攻击
    CLAW,           // 爪击攻击
    SPECIAL         // 特殊攻击
};

// 攻击效果枚举（可组合）
enum class AttackEffect {
    NONE = 0,               // 无特殊效果
    BLEEDING = 1,           // 流血效果
    POISON = 2,             // 中毒效果
    STUN = 4,               // 眩晕效果
    KNOCKBACK = 8,          // 击退效果
    INFECTION = 16,         // 感染效果
    IMMOBILIZE = 32,        // 定身效果
    WEAKEN = 64,            // 虚弱效果
    FEAR = 128              // 恐惧效果
};

// 允许AttackEffect进行位运算组合
inline AttackEffect operator|(AttackEffect a, AttackEffect b) {
    return static_cast<AttackEffect>(static_cast<int>(a) | static_cast<int>(b));
}

inline bool operator&(AttackEffect a, AttackEffect b) {
    return (static_cast<int>(a) & static_cast<int>(b)) != 0;
}

// 生物攻击类
class CreatureAttack {
private:
    std::string name;           // 攻击名称
    AttackType type;            // 攻击类型
    AttackEffect effects;       // 攻击效果（可组合）
    int baseDamage;             // 基础伤害值
    int range;                  // 攻击范围（像素）
    int cooldown;               // 冷却时间（毫秒）
    int currentCooldown;        // 当前冷却计时器
    float accuracy;             // 命中率（0.0-1.0）
    float critChance;           // 暴击率（0.0-1.0）
    float critMultiplier;       // 暴击伤害倍率
    int energyCost;             // 能量消耗
    int staminaCost;            // 体力消耗
    
    // 攻击音效
    std::string soundFile;      // 攻击音效文件
    
    // 攻击动画相关
    int animationDuration;      // 动画持续时间（毫秒）
    std::string animationName;  // 动画名称
    
    // 攻击回调函数
    std::function<void(Creature*, Entity*)> onHitCallback;  // 命中目标时的回调
    std::function<void(Creature*)> onMissCallback;          // 未命中目标时的回调
    std::function<void(Creature*)> onStartCallback;         // 开始攻击时的回调
    std::function<void(Creature*)> onFinishCallback;        // 攻击结束时的回调

public:
    // 构造函数
    CreatureAttack(
        const std::string& attackName,
        AttackType attackType,
        int damage,
        int attackRange,
        int attackCooldown,
        float attackAccuracy = 0.8f,
        AttackEffect attackEffects = AttackEffect::NONE
    );
    
    // 析构函数
    ~CreatureAttack() = default;
    
    // 执行攻击
    bool execute(Creature* attacker, Entity* target);
    
    // 更新冷却时间
    void updateCooldown(int deltaTimeMs);
    
    // 检查是否可以攻击
    bool canAttack() const;
    
    // 计算实际伤害
    Damage calculateDamage(Creature* attacker) const;
    
    // 应用攻击效果
    void applyEffects(Creature* attacker, Entity* target) const;
    
    // 获取器和设置器
    const std::string& getName() const { return name; }
    AttackType getType() const { return type; }
    AttackEffect getEffects() const { return effects; }
    int getBaseDamage() const { return baseDamage; }
    int getRange() const { return range; }
    int getCooldown() const { return cooldown; }
    int getCurrentCooldown() const { return currentCooldown; }
    float getAccuracy() const { return accuracy; }
    float getCritChance() const { return critChance; }
    float getCritMultiplier() const { return critMultiplier; }
    int getEnergyCost() const { return energyCost; }
    int getStaminaCost() const { return staminaCost; }
    
    void setName(const std::string& attackName) { name = attackName; }
    void setType(AttackType attackType) { type = attackType; }
    void setEffects(AttackEffect attackEffects) { effects = attackEffects; }
    void setBaseDamage(int damage) { baseDamage = damage; }
    void setRange(int attackRange) { range = attackRange; }
    void setCooldown(int attackCooldown) { cooldown = attackCooldown; }
    void setCurrentCooldown(int current) { currentCooldown = current; }
    void setAccuracy(float attackAccuracy) { accuracy = attackAccuracy; }
    void setCritChance(float chance) { critChance = chance; }
    void setCritMultiplier(float multiplier) { critMultiplier = multiplier; }
    void setEnergyCost(int cost) { energyCost = cost; }
    void setStaminaCost(int cost) { staminaCost = cost; }
    
    // 设置回调函数
    void setOnHitCallback(std::function<void(Creature*, Entity*)> callback) { onHitCallback = callback; }
    void setOnMissCallback(std::function<void(Creature*)> callback) { onMissCallback = callback; }
    void setOnStartCallback(std::function<void(Creature*)> callback) { onStartCallback = callback; }
    void setOnFinishCallback(std::function<void(Creature*)> callback) { onFinishCallback = callback; }
    
    // 攻击音效相关
    const std::string& getSoundFile() const { return soundFile; }
    void setSoundFile(const std::string& file) { soundFile = file; }
    
    // 攻击动画相关
    int getAnimationDuration() const { return animationDuration; }
    void setAnimationDuration(int duration) { animationDuration = duration; }
    const std::string& getAnimationName() const { return animationName; }
    void setAnimationName(const std::string& name) { animationName = name; }
};

#endif // CREATURE_ATTACK_H 