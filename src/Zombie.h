#pragma once
#ifndef ZOMBIE_H
#define ZOMBIE_H

#include "Creature.h"
#include "ScentSource.h"
#include "SoundSource.h"
#include <memory>
#include <vector>

// 丧尸状态枚举
enum class ZombieState {
    IDLE,           // 空闲状态
    WANDERING,      // 徘徊状态
    INVESTIGATING,  // 调查状态（听到声音或闻到气味）
    CHASING,        // 追逐状态（看到目标）
    ATTACKING,      // 攻击状态
    FEEDING,        // 进食状态
    STUNNED,        // 眩晕状态
    DEAD            // 死亡状态
};

// 丧尸类型枚举
enum class ZombieType {
    NORMAL,         // 普通丧尸
    RUNNER,         // 奔跑者（快速）
    BLOATER,        // 臃肿者（爆炸）
    SPITTER,        // 喷吐者（远程攻击）
    TANK            // 坦克（高血量高伤害）
};

class Zombie : public Creature {
private:
    ZombieState zombieState;         // 丧尸状态
    ZombieType zombieType;           // 丧尸类型
    
    // 感知相关
    std::unique_ptr<ScentSource> personalScent;  // 丧尸自身气味
    Entity* visualTarget;            // 视觉目标
    ScentSource* scentTarget;        // 气味目标
    SoundSource* soundTarget;        // 声音目标
    
    // 行为参数
    float aggroLevel;                // 攻击性（0-1）
    float hungerLevel;               // 饥饿程度（0-1）
    float awarenessLevel;            // 警觉程度（0-1）
    
    // 移动相关
    int wanderTimer;                 // 徘徊计时器
    int wanderX;                     // 徘徊目标X
    int wanderY;                     // 徘徊目标Y
    int investigateTimer;            // 调查计时器
    
    // 状态计时器
    int stateTimer;                  // 状态持续时间
    
    // 攻击相关
    int attackCooldown;              // 攻击冷却
    
    // 寻路相关
    int pathfindingFailureCooldown;  // 寻路失败冷却时间（毫秒）
    float lastTargetX, lastTargetY;  // 上次寻路失败的目标位置
    
    // 目标选择
    Entity* selectBestVisualTarget(const std::vector<Entity*>& visibleEntities);
    ScentSource* selectBestScentTarget(const std::vector<ScentSource*>& scentSources);
    SoundSource* selectBestSoundTarget(const std::vector<SoundSource*>& soundSources);
    
    // 状态处理方法
    void handleIdleState(float deltaTime);
    void handleWanderingState(float deltaTime);
    void handleInvestigatingState(float deltaTime);
    void handleChasingState(float deltaTime);
    void handleAttackingState(float deltaTime);
    void handleFeedingState(float deltaTime);
    void handleStunnedState(float deltaTime);
    void handleDeadState(float deltaTime);
    
    // 计算到目标的距离
    float distanceToTarget(float targetX, float targetY) const;
    
    // 移动到指定位置（智能寻路）
    void moveToPosition(float targetX, float targetY, float deltaTime);
    
    // 新增：使用寻路系统移动到目标
    void moveToPositionWithPathfinding(float targetX, float targetY, float deltaTime);
    
    // 新增：直接移动到目标（不寻路）
    void moveDirectlyToPosition(float targetX, float targetY, float deltaTime);
    
    // 更新个人气味
    void updatePersonalScent();

public:
    // 构造函数
    Zombie(
        float startX, float startY,
        ZombieType type = ZombieType::NORMAL,
        const std::string& zombieSpecies = "普通丧尸",
        Faction zombieFaction = Faction::ENEMY
    );
    
    // 析构函数
    virtual ~Zombie() override = default;
    
    // 重写的基类方法
    virtual void update(float deltaTime = 1.0f / 60.0f) override;
    virtual void render(SDL_Renderer* renderer, float cameraX, float cameraY) override;
    
    // 设置丧尸状态
    void setZombieState(ZombieState newState);
    ZombieState getZombieState() const { return zombieState; }
    
    // 获取丧尸类型
    ZombieType getZombieType() const { return zombieType; }
    
    // 感知相关方法
    bool checkVisualTargets(const std::vector<Entity*>& entities);
    bool checkScentSources(const std::vector<ScentSource*>& scentSources);
    bool checkSoundSources(const std::vector<SoundSource*>& soundSources);
    
    // 攻击相关方法
    bool tryAttack(Entity* target);
    
    // 获取个人气味源
    ScentSource* getPersonalScent() const { return personalScent.get(); }
    
    // 获取当前目标
    Entity* getCurrentVisualTarget() const { return visualTarget; }
    ScentSource* getCurrentScentTarget() const { return scentTarget; }
    SoundSource* getCurrentSoundTarget() const { return soundTarget; }
    
private:
    // 添加攻击能力
    void addAttackAbilities();
};

#endif // ZOMBIE_H 