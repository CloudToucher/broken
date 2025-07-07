#include "Creature.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <memory>
#include "SoundManager.h"
#include "Game.h"
#include "Pathfinding.h"
#include "Map.h"
#include "Tile.h"
#include "Constants.h"

// 构造函数
Creature::Creature(
    float startX, float startY,
    int entityRadius,
    int entitySpeed,
    int entityHealth,
    SDL_Color entityColor,
    CreatureType creatureType,
    const std::string& creatureSpecies,
    Faction entityFaction
) : Entity(startX, startY, entityRadius, entitySpeed, entityHealth, entityColor, entityFaction),
    type(creatureType),
    state(CreatureState::IDLE),
    species(creatureSpecies),
    age(0),
    maxHealth(entityHealth),
    energy(100),
    maxEnergy(100),
    stamina(100),
    maxStamina(100),
    aggressionLevel(0.5f),
    fearLevel(0.3f),
    intelligenceLevel(0.5f),
    packInstinct(0.5f),
    detectionRange(300),
    pathfindingIntelligence(2.0f), // 默认寻路智能程度
    visualRange(5),     // 默认视觉范围5格
    hearingRange(8),    // 默认听觉范围8格
    smellRange(3),      // 默认嗅觉范围3格
    currentAttackIndex(0),
    currentTarget(nullptr),
    targetUpdateTimer(0),
    targetX(0.0f), targetY(0.0f),
    hasPathTarget(false),
    isFollowingPath(false),
    moveSpeedModifier(1.0f),
    meleeHitDifficulty(0),      // 默认近战命中难度为0
    rangedHitDifficulty(0) {    // 默认远程命中难度为0
    
    // 根据生物类型设置默认标志
    switch (type) {
        case CreatureType::HUMANOID:
            addFlag(EntityFlag::HAS_VISION);
            addFlag(EntityFlag::HAS_HEARING);
            addFlag(EntityFlag::HAS_SMELL);
            addFlag(EntityFlag::HAS_INTELLIGENCE);
            addFlag(EntityFlag::CAN_USE_WEAPONS);
            addFlag(EntityFlag::CAN_USE_GUNS);
            break;
            
        case CreatureType::ANIMAL:
            addFlag(EntityFlag::HAS_VISION);
            addFlag(EntityFlag::HAS_HEARING);
            addFlag(EntityFlag::HAS_SMELL);
            break;
            
        case CreatureType::INSECT:
            addFlag(EntityFlag::HAS_VISION);
            break;
            
        case CreatureType::UNDEAD:
            addFlag(EntityFlag::HAS_VISION);
            addFlag(EntityFlag::IS_ZOMBIE);
            break;
            
        case CreatureType::MUTANT:
            addFlag(EntityFlag::HAS_VISION);
            addFlag(EntityFlag::HAS_HEARING);
            addFlag(EntityFlag::HAS_SMELL);
            addFlag(EntityFlag::CAN_RUSH);
            break;
            
        case CreatureType::ROBOT:
            addFlag(EntityFlag::HAS_VISION);
            addFlag(EntityFlag::HAS_HEARING);
            addFlag(EntityFlag::HAS_INFRARED_VISION);
            addFlag(EntityFlag::CAN_USE_WEAPONS);
            break;
            
        case CreatureType::ALIEN:
            addFlag(EntityFlag::HAS_VISION);
            addFlag(EntityFlag::HAS_HEARING);
            addFlag(EntityFlag::HAS_INFRARED_VISION);
            addFlag(EntityFlag::HAS_INTELLIGENCE);
            break;
            
        case CreatureType::SPIRIT:
            addFlag(EntityFlag::HAS_VISION);
            addFlag(EntityFlag::CAN_FLY);
            break;
            
        case CreatureType::MYTHICAL:
            addFlag(EntityFlag::HAS_VISION);
            addFlag(EntityFlag::HAS_HEARING);
            addFlag(EntityFlag::HAS_SMELL);
            addFlag(EntityFlag::HAS_INTELLIGENCE);
            addFlag(EntityFlag::CAN_FLY);
            break;
    }
}

// 析构函数
Creature::~Creature() {
    // 清理资源
}

// 更新方法
void Creature::update(float deltaTime) {
    // 首先调用基类的更新方法
    Entity::update(deltaTime);
    
    // 更新攻击冷却时间
    updateAttackCooldowns(static_cast<int>(deltaTime * 1000));
    
    // 更新目标计时器
    targetUpdateTimer -= static_cast<int>(deltaTime * 1000);
    if (targetUpdateTimer <= 0) {
        targetUpdateTimer = 1000; // 每秒更新一次目标
        selectTarget();
    }
    
    // 根据当前状态执行相应的行为
    updateAI(deltaTime);
    
    // 资源自然恢复
    if (state != CreatureState::DEAD) {
        regenerateEnergy(static_cast<int>(deltaTime * 2)); // 每秒恢复2点能量
        regenerateStamina(static_cast<int>(deltaTime * 3)); // 每秒恢复3点体力
        
        // 如果生命值低于最大值的一半，缓慢恢复生命值
        if (health < maxHealth / 2) {
            regenerateHealth(static_cast<int>(deltaTime * 0.5f)); // 每秒恢复0.5点生命值
        }
    }
}

// 渲染方法
void Creature::render(SDL_Renderer* renderer, float cameraX, float cameraY) {
    // 首先调用基类的渲染方法
    Entity::render(renderer, cameraX, cameraY);
    
    // 在这里可以添加生物特有的渲染逻辑
    // 例如，渲染生命值条、状态图标等
    
    // 渲染生命值条
    int screenX = static_cast<int>(x - cameraX);
    int screenY = static_cast<int>(y - cameraY);
    
    // 生命值条背景
    SDL_SetRenderDrawColor(renderer, 64, 64, 64, 255);
    SDL_FRect healthBarBg = {static_cast<float>(screenX - 20), static_cast<float>(screenY - radius - 10), 40.0f, 5.0f};
    SDL_RenderFillRect(renderer, &healthBarBg);
    
    // 生命值条
    float healthPercent = static_cast<float>(health) / maxHealth;
    SDL_SetRenderDrawColor(renderer, 255 * (1.0f - healthPercent), 255 * healthPercent, 0, 255);
    SDL_FRect healthBar = {static_cast<float>(screenX - 20), static_cast<float>(screenY - radius - 10), 40.0f * healthPercent, 5.0f};
    SDL_RenderFillRect(renderer, &healthBar);
    
    // 如果有目标，绘制一条线连接到目标
    if (currentTarget && state == CreatureState::HUNTING) {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 128);
        SDL_RenderLine(renderer, screenX, screenY, 
                      static_cast<int>(currentTarget->getX() - cameraX), 
                      static_cast<int>(currentTarget->getY() - cameraY));
    }
}

// 受伤方法
bool Creature::takeDamage(const Damage& damage) {
    // 调用基类的受伤方法
    bool result = Entity::takeDamage(damage);
    
    // 如果受到伤害，播放受伤音效
    if (result && health > 0) {
        if (!hurtSound.empty()) {
            //SoundManager::getInstance().playSound(hurtSound);
        }
        
        // 根据伤害程度增加恐惧值
        int totalDamage = 0;
        for (const auto& damageInfo : damage.getDamageList()) {
            totalDamage += std::get<1>(damageInfo);
        }
        
        // 伤害越高，恐惧值增加越多
        float fearIncrease = static_cast<float>(totalDamage) / maxHealth * 0.5f;
        setFearLevel(fearLevel + fearIncrease);
        
        // 如果生命值低，增加逃跑概率
        if (health < maxHealth * 0.3f && fearLevel > 0.7f) {
            setState(CreatureState::FLEEING);
        }
    } else if (health <= 0) {
        //// 如果死亡，播放死亡音效
        //if (!deathSound.empty()) {
        //    SoundManager::getInstance().playSound(deathSound);
        //}
        
        // 设置死亡状态
        setState(CreatureState::DEAD);
    }
    
    return result;
}

// 攻击系统方法

// 添加攻击方式
void Creature::addAttack(std::unique_ptr<CreatureAttack> attack) {
    attacks.push_back(std::move(attack));
}

// 获取攻击方式
CreatureAttack* Creature::getAttack(int index) {
    if (index >= 0 && index < attacks.size()) {
        return attacks[index].get();
    }
    return nullptr;
}

// 获取当前攻击方式
CreatureAttack* Creature::getCurrentAttack() {
    return getAttack(currentAttackIndex);
}

// 设置当前攻击方式
void Creature::setCurrentAttack(int index) {
    if (index >= 0 && index < attacks.size()) {
        currentAttackIndex = index;
    }
}

// 更新攻击冷却时间
void Creature::updateAttackCooldowns(int deltaTimeMs) {
    for (auto& attack : attacks) {
        attack->updateCooldown(deltaTimeMs);
    }
}

// 执行攻击
bool Creature::executeAttack(Entity* target) {
    if (!canAttack() || !target) {
        return false;
    }
    
    CreatureAttack* attack = getCurrentAttack();
    if (!attack) {
        return false;
    }
    
    //// 播放攻击音效
    //if (!attackSound.empty()) {
    //    SoundManager::getInstance().playSound(attackSound);
    //}
    
    // 消耗资源
    consumeEnergy(attack->getEnergyCost());
    consumeStamina(attack->getStaminaCost());
    
    // 执行攻击
    return attack->execute(this, target);
}

// 检查是否可以攻击
bool Creature::canAttack() const {
    if (state == CreatureState::DEAD) {
        return false;
    }
    
    if (attacks.empty()) {
        return false;
    }
    
    CreatureAttack* attack = attacks[currentAttackIndex].get();
    if (!attack || !attack->canAttack()) {
        return false;
    }
    
    // 检查资源是否足够
    if (energy < attack->getEnergyCost() || stamina < attack->getStaminaCost()) {
        return false;
    }
    
    return true;
}

// 状态管理

// 设置生物状态
void Creature::setState(CreatureState newState) {
    state = newState;
    
    // 根据状态播放相应的音效
    switch (state) {
        //case CreatureState::IDLE:
        //    if (!idleSound.empty()) {
        //        SoundManager::getInstance().playSound(idleSound);
        //    }
        //    break;
        //    
        //case CreatureState::ATTACKING:
        //    if (!attackSound.empty()) {
        //        SoundManager::getInstance().playSound(attackSound);
        //    }
        //    break;
        //    
        //case CreatureState::DEAD:
        //    if (!deathSound.empty()) {
        //        SoundManager::getInstance().playSound(deathSound);
        //    }
        //    break;
            
        default:
            break;
    }
}

// 获取生物状态
CreatureState Creature::getState() const {
    return state;
}

// AI行为

// 更新AI行为
void Creature::updateAI(float deltaTime) {
    // 如果死亡，不执行任何行为
    if (state == CreatureState::DEAD) {
        return;
    }
    
    // 根据当前状态执行相应的行为
    switch (state) {
        case CreatureState::IDLE:
            // 随机决定是否开始漫游
            if (rand() % 100 < 5) { // 5%的概率
                setState(CreatureState::WANDERING);
            }
            break;
            
        case CreatureState::WANDERING:
            // 漫游行为
            // TODO: 实现随机移动
            
            // 如果发现目标，切换到狩猎状态
            if (currentTarget) {
                setState(CreatureState::HUNTING);
            }
            break;
            
        case CreatureState::HUNTING:
            // 如果有目标，向目标移动
            if (currentTarget) {
                moveToTarget(deltaTime);
                
                // 如果目标在攻击范围内，切换到攻击状态
                CreatureAttack* attack = getCurrentAttack();
                if (attack && isTargetInRange(currentTarget, attack->getRange())) {
                    setState(CreatureState::ATTACKING);
                }
            } else {
                // 如果没有目标，回到空闲状态
                setState(CreatureState::IDLE);
            }
            break;
            
        case CreatureState::ATTACKING:
            // 如果有目标，执行攻击
            if (currentTarget) {
                // 如果目标不在攻击范围内，切换回狩猎状态
                CreatureAttack* attack = getCurrentAttack();
                if (!attack || !isTargetInRange(currentTarget, attack->getRange())) {
                    setState(CreatureState::HUNTING);
                } else {
                    // 执行攻击
                    executeAttack(currentTarget);
                }
            } else {
                // 如果没有目标，回到空闲状态
                setState(CreatureState::IDLE);
            }
            break;
            
        case CreatureState::FLEEING:
            // 如果有目标，远离目标
            if (currentTarget) {
                fleeFromTarget(deltaTime);
                
                // 如果恐惧值降低或者目标远离，回到空闲状态
                if (fearLevel < 0.3f || !isTargetInRange(currentTarget, detectionRange * 2)) {
                    setState(CreatureState::IDLE);
                }
            } else {
                // 如果没有目标，回到空闲状态
                setState(CreatureState::IDLE);
            }
            break;
            
        case CreatureState::EATING:
            // 进食行为
            // TODO: 实现进食逻辑
            break;
            
        case CreatureState::SLEEPING:
            // 睡眠行为
            // TODO: 实现睡眠逻辑
            break;
            
        default:
            break;
    }
}

// 选择目标
void Creature::selectTarget() {
    // 如果已经死亡，不选择目标
    if (state == CreatureState::DEAD) {
        currentTarget = nullptr;
        return;
    }
    
    // TODO: 从游戏中获取附近的实体，选择合适的目标
    // 这里需要Game类提供获取附近实体的方法
    
    // 临时实现：保持当前目标
}

// 向目标移动
void Creature::moveToTarget(float deltaTime) {
    if (!currentTarget) {
        // 没有目标，停止移动
        setDesiredVelocity(0.0f, 0.0f);
        return;
    }
    
    // 计算方向向量
    float dx = currentTarget->getX() - x;
    float dy = currentTarget->getY() - y;
    float distance = std::sqrt(dx*dx + dy*dy);
    
    // 如果已经足够接近，停止移动
    if (distance <= radius + currentTarget->getCollider().getRadius()) {
        setDesiredVelocity(0.0f, 0.0f);
        return;
    }
    
    // 归一化方向向量
    float dirX = dx / distance;
    float dirY = dy / distance;
    
    // 计算期望速度
    float desiredSpeed = speed * getSpeedModifier();
    float desiredVelX = dirX * desiredSpeed;
    float desiredVelY = dirY * desiredSpeed;
    
    // 设置期望速度，让物理系统处理实际移动
    setDesiredVelocity(desiredVelX, desiredVelY);
}

// 远离目标
void Creature::fleeFromTarget(float deltaTime) {
    if (!currentTarget) {
        // 没有目标，停止移动
        setDesiredVelocity(0.0f, 0.0f);
        return;
    }
    
    // 计算方向向量（与移动到目标相反）
    float dx = x - currentTarget->getX();
    float dy = y - currentTarget->getY();
    float distance = std::sqrt(dx*dx + dy*dy);
    
    // 如果已经足够远，停止移动
    if (distance >= detectionRange * 2) {
        setDesiredVelocity(0.0f, 0.0f);
        return;
    }
    
    // 归一化方向向量
    float dirX = dx / distance;
    float dirY = dy / distance;
    
    // 计算期望速度（逃跑时速度提高）
    float desiredSpeed = speed * getSpeedModifier() * 1.5f;
    float desiredVelX = dirX * desiredSpeed;
    float desiredVelY = dirY * desiredSpeed;
    
    // 设置期望速度，让物理系统处理实际移动
    setDesiredVelocity(desiredVelX, desiredVelY);
}

// 检查目标是否在范围内
bool Creature::isTargetInRange(Entity* target, int range) const {
    if (!target) {
        return false;
    }
    
    int dx = target->getX() - x;
    int dy = target->getY() - y;
    int distanceSquared = dx * dx + dy * dy;
    
    return distanceSquared <= range * range;
}

// 资源管理

// 消耗能量
void Creature::consumeEnergy(int amount) {
    energy = std::max(0, energy - amount);
}

// 消耗体力
void Creature::consumeStamina(int amount) {
    stamina = std::max(0, stamina - amount);
}

// 恢复能量
void Creature::regenerateEnergy(int amount) {
    energy = std::min(maxEnergy, energy + amount);
}

// 恢复体力
void Creature::regenerateStamina(int amount) {
    stamina = std::min(maxStamina, stamina + amount);
}

// 恢复生命值
void Creature::regenerateHealth(int amount) {
    health = std::min(maxHealth, health + amount);
}

// 特殊能力管理

// 添加特殊能力
void Creature::addSpecialAbility(const std::string& name, int level) {
    specialAbilities[name] = level;
}

// 移除特殊能力
void Creature::removeSpecialAbility(const std::string& name) {
    specialAbilities.erase(name);
}

// 检查是否有特殊能力
bool Creature::hasSpecialAbility(const std::string& name) const {
    return specialAbilities.find(name) != specialAbilities.end();
}

// 获取特殊能力等级
int Creature::getSpecialAbilityLevel(const std::string& name) const {
    auto it = specialAbilities.find(name);
    if (it != specialAbilities.end()) {
        return it->second;
    }
    return 0;
}

// 提升特殊能力等级
void Creature::upgradeSpecialAbility(const std::string& name, int levels) {
    if (hasSpecialAbility(name)) {
        specialAbilities[name] += levels;
    }
}

// 感知相关方法实现

// 检查是否能看到实体
bool Creature::canSeeEntity(Entity* entity) const {
    if (!entity || entity == this) {
        return false;
    }
    
    // 计算距离
    float dx = entity->getX() - x;
    float dy = entity->getY() - y;
    float distance = sqrt(dx * dx + dy * dy);
    
    // 检查是否在视觉范围内
    float visualRangePixels = GameConstants::gridsToPixels(visualRange);
    if (distance > visualRangePixels) {
        return false;
    }
    
    // 检查视线是否被阻挡
    return raycast(static_cast<float>(x), static_cast<float>(y), 
                  static_cast<float>(entity->getX()), static_cast<float>(entity->getY()));
}

// 检查是否能听到声音
bool Creature::canHearSound(const SoundSource& sound) const {
    if (!sound.isActive) {
        return false;
    }
    
    // 计算生物到声源的距离
    float dx = sound.x - x;
    float dy = sound.y - y;
    float distance = sqrt(dx * dx + dy * dy);
    
    // 听觉半径转换为像素
    float hearingRangePixels = GameConstants::gridsToPixels(hearingRange);
    
    // 检查距离是否在听觉半径和声音传播半径内
    return (distance <= hearingRangePixels && distance <= sound.radius);
}

// 检查是否能嗅到气味（空函数，等待重构）
bool Creature::canSmellScent(const ScentSource& scent) const {
    // TODO: 等待重构，暂时返回false
    return false;
}

// 获取可见的实体列表
std::vector<Entity*> Creature::getVisibleEntities(const std::vector<Entity*>& entities) const {
    std::vector<Entity*> visibleEntities;
    
    for (Entity* entity : entities) {
        if (canSeeEntity(entity)) {
            visibleEntities.push_back(entity);
        }
    }
    
    return visibleEntities;
}

// 获取可听到的声音列表
std::vector<SoundSource*> Creature::getAudibleSounds(const std::vector<SoundSource*>& sounds) const {
    std::vector<SoundSource*> audibleSounds;
    
    for (SoundSource* sound : sounds) {
        if (canHearSound(*sound)) {
            audibleSounds.push_back(sound);
        }
    }
    
    return audibleSounds;
}

// 获取可嗅到的气味列表（空函数，等待重构）
std::vector<ScentSource*> Creature::getSmellableScents(const std::vector<ScentSource*>& scents) const {
    // TODO: 等待重构，暂时返回空列表
    return std::vector<ScentSource*>();
}

// 射线检测方法
bool Creature::raycast(float startX, float startY, float endX, float endY) const {
    // 获取游戏实例
    Game* game = Game::getInstance();
    if (!game) {
        return true; // 如果没有游戏实例，假设没有阻拦
    }
    
    // 获取所有视觉碰撞箱（统一接口）
    std::vector<Collider*> visionColliders = game->getAllVisionColliders();
    
    // 简单的射线检测实现
    float dx = endX - startX;
    float dy = endY - startY;
    float distance = sqrt(dx * dx + dy * dy);
    
    if (distance < 1.0f) {
        return true; // 距离太近，认为可见
    }
    
    // 归一化方向向量
    dx /= distance;
    dy /= distance;
    
    // 每隔一定步长检查碰撞
    float stepSize = 8.0f; // 减小步长，提高精度
    float currentDistance = 0.0f;
    
    while (currentDistance < distance) {
        float currentX = startX + dx * currentDistance;
        float currentY = startY + dy * currentDistance;
        
        // 检查当前点是否与任何视觉碰撞箱碰撞
        for (Collider* visionCollider : visionColliders) {
            if (visionCollider && visionCollider->contains(static_cast<int>(currentX), static_cast<int>(currentY))) {
                return false; // 被视觉障碍物阻挡
            }
        }
        
        currentDistance += stepSize;
    }
    
    return true; // 没有被阻挡
}

// 新增：寻路相关方法实现

void Creature::setPathTarget(float targetPosX, float targetPosY) {
    targetX = targetPosX;
    targetY = targetPosY;
    hasPathTarget = true;
    isFollowingPath = false; // 重置路径跟随状态
}

void Creature::clearPathTarget() {
    hasPathTarget = false;
    isFollowingPath = false;
    targetX = 0.0f;
    targetY = 0.0f;
}

void Creature::updatePathfinding(float deltaTime, CreaturePathfinder* pathfinder) {
    if (!pathfinder || !hasPathTarget) return;
    
    // 更新寻路系统中的生物状态
    pathfinder->updateCreature(this, deltaTime);
    
    // 请求寻路（如果冷却时间已过）
    PathfindingResult result = pathfinder->requestPath(
        this, 
        static_cast<int>(x), static_cast<int>(y),
        static_cast<int>(targetX), static_cast<int>(targetY),
        pathfindingIntelligence
    );
    
    // 根据寻路结果决定移动方式
    if (result == PathfindingResult::SUCCESS) {
        // A*成功找到路径，跟随路径
        isFollowingPath = true;
        moveAlongPath(deltaTime, pathfinder);
    } else {
        // NO_PATH表示无障碍直线路径，或者智能不足，直接朝目标移动
        isFollowingPath = false;
        moveDirectlyToTarget(deltaTime);
    }
}

void Creature::moveAlongPath(float deltaTime, CreaturePathfinder* pathfinder) {
    if (!pathfinder) {
        setDesiredVelocity(0.0f, 0.0f);
        return;
    }
    
    // 获取下一个路径点
    auto waypointResult = pathfinder->getNextWaypoint(this, x, y);
    if (!waypointResult.first) {
        // 路径完成或无效
        clearPathTarget();
        setDesiredVelocity(0.0f, 0.0f);
        return;
    }
    
    const PathPoint& waypoint = waypointResult.second;
    
    // 计算移动方向
    float dx = waypoint.x - x;
    float dy = waypoint.y - y;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    if (distance > 1.0f) {
        // 标准化方向向量
        dx /= distance;
        dy /= distance;
        
        // 应用地形移动耗时修正
        float terrainModifier = 1.0f / waypoint.moveCost;
        
        // 计算期望速度
        float desiredSpeed = speed * terrainModifier;
        float desiredVelX = dx * desiredSpeed;
        float desiredVelY = dy * desiredSpeed;
        
        // 设置期望速度，让物理系统处理实际移动
        setDesiredVelocity(desiredVelX, desiredVelY);
    } else {
        setDesiredVelocity(0.0f, 0.0f);
    }
}

void Creature::moveDirectlyToTarget(float deltaTime) {
    if (!hasPathTarget) {
        setDesiredVelocity(0.0f, 0.0f);
        return;
    }
    
    // 计算直线移动方向
    float dx = targetX - x;
    float dy = targetY - y;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    if (distance > 1.0f) {
        // 标准化方向向量
        dx /= distance;
        dy /= distance;
        
        // 计算期望速度
        float desiredSpeed = speed;
        float desiredVelX = dx * desiredSpeed;
        float desiredVelY = dy * desiredSpeed;
        
        // 设置期望速度，让物理系统处理实际移动和碰撞检测
        setDesiredVelocity(desiredVelX, desiredVelY);
    } else {
        // 到达目标
        clearPathTarget();
        setDesiredVelocity(0.0f, 0.0f);
    }
} 