#pragma once
#ifndef ATTACK_SYSTEM_H
#define ATTACK_SYSTEM_H

#include "Item.h"
#include "Damage.h"
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <SDL3/SDL.h>

// 前向声明
class Entity;

// 武器攻击类型枚举（用于区分左键右键等）
enum class WeaponAttackType {
    PRIMARY,        // 主攻击（左键）
    SECONDARY,      // 副攻击（右键）
    SPECIAL         // 特殊攻击
};

// 攻击方式枚举
enum class AttackMethod {
    MELEE_SLASH,        // 近战砍击
    MELEE_STAB,         // 近战刺击
    MELEE_CRUSH,        // 近战重击
    MELEE_QUICK,        // 近战快击
    RANGED_SHOOT,       // 远程射击
    RANGED_THROW,       // 远程投掷
    SPECIAL_ABILITY     // 特殊能力
};

// 攻击形状枚举
enum class AttackShape {
    CIRCLE,             // 圆形（默认）
    SECTOR,             // 扇形
    RECTANGLE,          // 长条形
    LINE,               // 直线
    LARGE_SECTOR        // 超大范围扇形
};

// 攻击参数结构体
struct AttackParams {
    int baseDamage;                 // 基础伤害
    float range;                    // 攻击范围
    float speed;                    // 攻击速度（次/秒）
    int cooldownMs;                 // 冷却时间（毫秒）
    float criticalChance;           // 暴击率
    float criticalMultiplier;       // 暴击倍率
    std::string damageType;         // 伤害类型
    int armorPenetration;           // 护甲穿透
    
    // 攻击形状相关参数
    AttackShape shape;              // 攻击形状
    float width;                    // 宽度（用于长条形攻击）
    float angle;                    // 角度（用于扇形攻击，弧度）
    float direction;                // 攻击方向（弧度，相对于攻击者朝向）
    
    // 特殊效果
    bool canBleed;                  // 能否造成流血
    bool canStun;                   // 能否造成眩晕
    bool canPoison;                 // 能否造成中毒
    bool canKnockback;              // 能否造成击退
    float bleedChance;              // 流血概率
    float stunChance;               // 眩晕概率
    float poisonChance;             // 中毒概率
    float knockbackChance;          // 击退概率
    int stunDuration;               // 眩晕持续时间（毫秒）
    int poisonDuration;             // 中毒持续时间（毫秒）
    float poisonDamage;             // 中毒每秒伤害
    float knockbackForce;           // 击退力度
    
    // 音效和动画
    std::string soundFile;          // 攻击音效
    std::string animationName;      // 攻击动画
    int animationDuration;          // 动画持续时间（毫秒）
    
    // 构造函数
    AttackParams() : 
        baseDamage(10), range(50.0f), speed(1.0f), cooldownMs(1000),
        criticalChance(0.05f), criticalMultiplier(2.0f),
        damageType("blunt"), armorPenetration(0),
        shape(AttackShape::CIRCLE), width(50.0f), angle(1.047f), direction(0.0f), // 默认60度扇形
        canBleed(false), canStun(false), canPoison(false), canKnockback(false),
        bleedChance(0.0f), stunChance(0.0f), poisonChance(0.0f), knockbackChance(0.0f),
        stunDuration(1000), poisonDuration(5000), poisonDamage(2.0f), knockbackForce(10.0f),
        soundFile(""), animationName(""), animationDuration(500) {}
};

// 攻击结果结构体
struct AttackResult {
    bool hit;                       // 是否命中
    bool critical;                  // 是否暴击
    int totalDamage;                // 总伤害
    bool causedBleeding;            // 是否造成流血
    bool causedStun;                // 是否造成眩晕
    bool causedPoison;              // 是否造成中毒
    bool causedKnockback;           // 是否造成击退
    Entity* target;                 // 攻击目标
    
    AttackResult() : hit(false), critical(false), totalDamage(0), 
                    causedBleeding(false), causedStun(false), causedPoison(false), 
                    causedKnockback(false), target(nullptr) {}
};

// 通用攻击系统类
class AttackSystem {
private:
    Entity* owner;                  // 攻击者
    int currentCooldown;            // 当前冷却时间
    
    // 攻击回调函数
    std::function<void(const AttackResult&)> onAttackComplete;
    std::function<Entity*(float)> findTargetFunction;  // 寻找目标的函数

public:
    // 构造函数
    AttackSystem(Entity* attacker);
    
    // 析构函数
    ~AttackSystem() = default;
    
    // 执行攻击
    AttackResult executeAttack(AttackMethod method, const AttackParams& params);
    AttackResult executeAttack(WeaponAttackType type = WeaponAttackType::PRIMARY);
    
    // 更新冷却时间
    void updateCooldown(int deltaTimeMs);
    
    // 检查是否可以攻击
    bool canAttack() const;
    
    // 设置寻找目标的函数
    void setFindTargetFunction(std::function<Entity*(float)> func) { findTargetFunction = func; }
    
    // 设置攻击完成回调
    void setOnAttackComplete(std::function<void(const AttackResult&)> callback) { onAttackComplete = callback; }
    
    // 获取当前冷却时间
    int getCurrentCooldown() const { return currentCooldown; }
    
    // 设置冷却时间
    void setCooldown(int cooldown) { currentCooldown = cooldown; }
    
    // 获取攻击范围内的所有目标（用于可视化）
    std::vector<Entity*> getTargetsInRange(const AttackParams& params) const;
    
    // 渲染攻击范围
    void renderAttackRange(SDL_Renderer* renderer, const AttackParams& params, float cameraX, float cameraY) const;
    
    // 渲染动画攻击范围（新增）
    void renderAnimatedAttackRange(SDL_Renderer* renderer, const AttackParams& params, float cameraX, float cameraY, float animationPhase = 0.0f) const;

private:
    // 内部方法
    AttackResult performMeleeAttack(AttackMethod method, const AttackParams& params);
    AttackResult performRangedAttack(AttackMethod method, const AttackParams& params);
    AttackResult performSpecialAttack(AttackMethod method, const AttackParams& params);
    
    // 计算伤害
    Damage calculateDamage(const AttackParams& params, bool& isCritical);
    
    // 应用特殊效果
    void applyEffects(Entity* target, const AttackParams& params, AttackResult& result);
    
    // 寻找攻击目标
    Entity* findTarget(float range);
    
    // 形状检测方法
    Entity* findTargetInShape(const AttackParams& params) const;
    bool isTargetInCircle(Entity* target, float range) const;
    bool isTargetInSector(Entity* target, const AttackParams& params) const;
    bool isTargetInRectangle(Entity* target, const AttackParams& params) const;
    bool isTargetInLine(Entity* target, const AttackParams& params) const;
    
    // 辅助方法
    float getAngleToTarget(Entity* target) const;
    float normalizeAngle(float angle) const;
    float getDistanceToTarget(Entity* target) const;
    
    // 渲染辅助方法
    void renderCircleRange(SDL_Renderer* renderer, float x, float y, float radius) const;
    void renderSectorRange(SDL_Renderer* renderer, float x, float y, const AttackParams& params) const;
    void renderRectangleRange(SDL_Renderer* renderer, float x, float y, const AttackParams& params) const;
    void renderLineRange(SDL_Renderer* renderer, float x, float y, const AttackParams& params) const;
    
    // 动画渲染辅助方法（新增）
    void renderAnimatedSectorRange(SDL_Renderer* renderer, float x, float y, const AttackParams& params, float animationPhase) const;
    void renderAnimatedRectangleRange(SDL_Renderer* renderer, float x, float y, const AttackParams& params, float animationPhase) const;
    void drawGradientLine(SDL_Renderer* renderer, int x1, int y1, int x2, int y2, Uint8 alpha1, Uint8 alpha2) const;
    void fillGradientSector(SDL_Renderer* renderer, float cx, float cy, float radius, float startAngle, float endAngle, float animationPhase) const;
};

// 武器攻击接口 - 所有武器都可以实现这个接口
class IWeaponAttack {
public:
    virtual ~IWeaponAttack() = default;
    
    // 获取攻击方式
    virtual AttackMethod getAttackMethod(WeaponAttackType type = WeaponAttackType::PRIMARY) const = 0;
    
    // 获取攻击参数
    virtual AttackParams getAttackParams(WeaponAttackType type = WeaponAttackType::PRIMARY) const = 0;
    
    // 检查是否可以攻击
    virtual bool canPerformAttack(WeaponAttackType type = WeaponAttackType::PRIMARY) const = 0;
    
    // 攻击后的处理（如耐久度损失等）
    virtual void onAttackPerformed(WeaponAttackType type = WeaponAttackType::PRIMARY) = 0;
};

#endif // ATTACK_SYSTEM_H 