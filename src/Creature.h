#pragma once
#ifndef CREATURE_H
#define CREATURE_H

#include "Entity.h"
#include "CreatureAttack.h"
#include "SoundSource.h"
#include "ScentSource.h"
#include "Map.h"
#include "Collider.h"
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

// 前向声明
class Map;
class Tile;
class CreaturePathfinder;
struct PathPoint;
enum class PathfindingResult;

// 生物类型枚举
enum class CreatureType {
    HUMANOID,       // 人形生物
    ANIMAL,         // 动物
    INSECT,         // 昆虫
    UNDEAD,         // 不死生物
    MUTANT,         // 变异生物
    ROBOT,          // 机器人
    ALIEN,          // 外星生物
    SPIRIT,         // 灵体
    MYTHICAL        // 神话生物
};

// 生物状态枚举（补充Entity状态）
enum class CreatureState {
    IDLE,           // 空闲
    WANDERING,      // 漫游
    HUNTING,        // 狩猎
    FLEEING,        // 逃跑
    ATTACKING,      // 攻击中
    EATING,         // 进食
    SLEEPING,       // 睡眠
    DEAD            // 死亡
};

// 生物类，派生自Entity
class Creature : public Entity {
protected:
    // 生物基本属性
    CreatureType type;                   // 生物类型
    CreatureState state;                 // 生物状态
    std::string species;                 // 物种名称
    int age;                             // 年龄
    int maxHealth;                       // 最大生命值
    
    // 生物资源
    int energy;                          // 能量值
    int maxEnergy;                       // 最大能量值
    int stamina;                         // 体力值
    int maxStamina;                      // 最大体力值
    
    // 生物行为参数
    float aggressionLevel;               // 攻击性（0-1）
    float fearLevel;                     // 恐惧程度（0-1）
    float intelligenceLevel;             // 智能程度（0-1）
    float packInstinct;                  // 群居本能（0-1）
    int detectionRange;                  // 探测范围
    
    // 新增：寻路智能程度
    float pathfindingIntelligence;       // 寻路智能程度（1.2-8.0），决定A*搜索深度
    
    // 感官范围（以格为单位，1格=64像素）
    int visualRange;                     // 视觉半径
    int hearingRange;                    // 听觉半径
    int smellRange;                      // 嗅觉半径
    
    // 攻击系统
    std::vector<std::unique_ptr<CreatureAttack>> attacks;  // 攻击方式列表
    int currentAttackIndex;                               // 当前选择的攻击索引
    
    // 生物声音
    std::string idleSound;               // 空闲声音
    std::string attackSound;             // 攻击声音
    std::string hurtSound;               // 受伤声音
    std::string deathSound;              // 死亡声音
    
    // 生物AI相关
    Entity* currentTarget;               // 当前目标
    int targetUpdateTimer;               // 目标更新计时器
    std::vector<Entity*> knownEntities;  // 已知实体列表
    
    // 新增：寻路相关
    float targetX, targetY;              // 寻路目标位置
    bool hasPathTarget;                  // 是否有寻路目标
    bool isFollowingPath;                // 是否正在跟随路径
    float moveSpeedModifier;             // 移动速度修正（基于地形）
    
    // 生物特殊能力
    std::unordered_map<std::string, int> specialAbilities;  // 特殊能力及其等级

    // 命中难度
    int meleeHitDifficulty;              // 近战命中难度
    int rangedHitDifficulty;             // 远程命中难度

public:
    // 构造函数
    Creature(
        float startX, float startY,
        int entityRadius,
        int entitySpeed,
        int entityHealth,
        SDL_Color entityColor,
        CreatureType creatureType,
        const std::string& creatureSpecies,
        Faction entityFaction = Faction::NEUTRAL
    );
    
    // 析构函数
    virtual ~Creature() override;
    
    // 重写的基类方法
    virtual void update(float deltaTime = 1.0f / 60.0f) override;
    virtual void render(SDL_Renderer* renderer, float cameraX, float cameraY) override;
    virtual bool takeDamage(const Damage& damage) override;
    
    // 生物特有方法
    
    // 攻击系统
    void addAttack(std::unique_ptr<CreatureAttack> attack);
    CreatureAttack* getAttack(int index);
    CreatureAttack* getCurrentAttack();
    void setCurrentAttack(int index);
    void updateAttackCooldowns(int deltaTimeMs);
    bool executeAttack(Entity* target);
    bool canAttack() const;
    
    // 状态管理
    void setState(CreatureState newState);
    CreatureState getState() const;
    
    // AI行为
    void updateAI(float deltaTime);
    void selectTarget();
    void moveToTarget(float deltaTime);
    void fleeFromTarget(float deltaTime);
    bool isTargetInRange(Entity* target, int range) const;
    
    // 新增：寻路相关方法
    void setPathTarget(float targetX, float targetY);
    void clearPathTarget();
    bool hasValidPathTarget() const { return hasPathTarget; }
    void updatePathfinding(float deltaTime, CreaturePathfinder* pathfinder);
    void moveAlongPath(float deltaTime, CreaturePathfinder* pathfinder);
    void moveDirectlyToTarget(float deltaTime);
    
    // 资源管理
    void consumeEnergy(int amount);
    void consumeStamina(int amount);
    void regenerateEnergy(int amount);
    void regenerateStamina(int amount);
    void regenerateHealth(int amount);
    
    // 获取器和设置器
    CreatureType getCreatureType() const { return type; }
    void setCreatureType(CreatureType creatureType) { type = creatureType; }
    
    const std::string& getSpecies() const { return species; }
    void setSpecies(const std::string& creatureSpecies) { species = creatureSpecies; }
    
    int getAge() const { return age; }
    void setAge(int creatureAge) { age = creatureAge; }
    
    int getMaxHealth() const { return maxHealth; }
    void setMaxHealth(int max) { maxHealth = max; }
    
    int getEnergy() const { return energy; }
    void setEnergy(int value) { energy = std::min(maxEnergy, std::max(0, value)); }
    
    int getMaxEnergy() const { return maxEnergy; }
    void setMaxEnergy(int max) { maxEnergy = max; }
    
    int getStamina() const { return stamina; }
    void setStamina(int value) { stamina = std::min(maxStamina, std::max(0, value)); }
    
    int getMaxStamina() const { return maxStamina; }
    void setMaxStamina(int max) { maxStamina = max; }
    
    float getAggressionLevel() const { return aggressionLevel; }
    void setAggressionLevel(float level) { aggressionLevel = std::min(1.0f, std::max(0.0f, level)); }
    
    float getFearLevel() const { return fearLevel; }
    void setFearLevel(float level) { fearLevel = std::min(1.0f, std::max(0.0f, level)); }
    
    float getIntelligenceLevel() const { return intelligenceLevel; }
    void setIntelligenceLevel(float level) { intelligenceLevel = std::min(1.0f, std::max(0.0f, level)); }
    
    float getPackInstinct() const { return packInstinct; }
    void setPackInstinct(float level) { packInstinct = std::min(1.0f, std::max(0.0f, level)); }
    
    int getDetectionRange() const { return detectionRange; }
    void setDetectionRange(int range) { detectionRange = range; }
    
    // 新增：寻路智能程度
    float getPathfindingIntelligence() const { return pathfindingIntelligence; }
    void setPathfindingIntelligence(float intelligence) { 
        pathfindingIntelligence = std::min(8.0f, std::max(1.2f, intelligence)); 
    }
    
    // 感官范围获取器和设置器
    int getVisualRange() const { return visualRange; }
    void setVisualRange(int range) { visualRange = range; }
    
    int getHearingRange() const { return hearingRange; }
    void setHearingRange(int range) { hearingRange = range; }
    
    int getSmellRange() const { return smellRange; }
    void setSmellRange(int range) { smellRange = range; }
    
    Entity* getCurrentTarget() const { return currentTarget; }
    void setCurrentTarget(Entity* target) { currentTarget = target; }
    
    // 特殊能力管理
    void addSpecialAbility(const std::string& name, int level = 1);
    void removeSpecialAbility(const std::string& name);
    bool hasSpecialAbility(const std::string& name) const;
    int getSpecialAbilityLevel(const std::string& name) const;
    void upgradeSpecialAbility(const std::string& name, int levels = 1);
    
    // 命中难度相关
    int getMeleeHitDifficulty() const { return meleeHitDifficulty; }
    void setMeleeHitDifficulty(int difficulty) { meleeHitDifficulty = difficulty; }
    int getRangedHitDifficulty() const { return rangedHitDifficulty; }
    void setRangedHitDifficulty(int difficulty) { rangedHitDifficulty = difficulty; }
    
    // 感知相关方法
    bool canSeeEntity(Entity* entity) const;
    bool canHearSound(const SoundSource& sound) const;
    bool canSmellScent(const ScentSource& scent) const; // 空函数，等待重构
    
    // 检查多个目标的感知
    std::vector<Entity*> getVisibleEntities(const std::vector<Entity*>& entities) const;
    std::vector<SoundSource*> getAudibleSounds(const std::vector<SoundSource*>& sounds) const;
    std::vector<ScentSource*> getSmellableScents(const std::vector<ScentSource*>& scents) const; // 空函数，等待重构
    
    // 射线检测（用于视线检测）
    bool raycast(float startX, float startY, float endX, float endY) const;
};

#endif // CREATURE_H 