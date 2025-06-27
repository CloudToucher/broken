#include "Zombie.h"
#include "Game.h"
#include "Map.h"
#include "CreatureAttack.h"
#include "Pathfinding.h"
#include "Constants.h"
#include <cmath>
#include <algorithm>
#include <random>
#include <limits>

// 构造函数
Zombie::Zombie(
    float startX, float startY,
    ZombieType type,
    const std::string& zombieSpecies,
    Faction zombieFaction
) : Creature(startX, startY, 16, 192, 100, {128, 128, 128, 255}, CreatureType::UNDEAD, zombieSpecies, zombieFaction),  // 3格/秒 = 192像素/秒
    zombieState(ZombieState::IDLE),
    zombieType(type),
    visualTarget(nullptr),
    scentTarget(nullptr),
    soundTarget(nullptr),
    aggroLevel(0.8f),
    hungerLevel(0.6f),
    awarenessLevel(0.5f),
    wanderTimer(0),
    wanderX(static_cast<int>(startX)),
    wanderY(static_cast<int>(startY)),
    investigateTimer(0),
    stateTimer(0),
    attackCooldown(0),
    pathfindingFailureCooldown(0),
    lastTargetX(-1),
    lastTargetY(-1) {
    
    // 根据丧尸类型设置属性
    switch (zombieType) {
        case ZombieType::NORMAL:
            // 普通丧尸：3格/秒
            maxHealth = 100;
            health = 100;
            speed = 192;  // 3格/秒 = 192像素/秒
            setPhysicalAttributes(65.0f, 8, 6);  // 重量65kg，力量8，敏捷6
            pathfindingIntelligence = 1.2f; // 基础智能
            break;
            
        case ZombieType::RUNNER:
            // 奔跑者：生命值低但速度快 6格/秒
            maxHealth = 70;
            health = 70;
            speed = 384;  // 6格/秒 = 384像素/秒
            setPhysicalAttributes(55.0f, 6, 14);  // 重量55kg，力量6，敏捷14（高敏捷，易推动）
            pathfindingIntelligence = 2.5f; // 中等智能
            break;
            
        case ZombieType::BLOATER:
            // 臃肿者：血厚但慢 2格/秒
            maxHealth = 200;
            health = 200;
            radius = 24;         // 更大的体型
            speed = 128;  // 2格/秒 = 128像素/秒
            setPhysicalAttributes(120.0f, 12, 3);  // 重量120kg，力量12，敏捷3（重坦克型）
            pathfindingIntelligence = 1.5f; // 较低智能
            break;
            
        case ZombieType::SPITTER:
            // 喷吐者：远程攻击，速度一般 2.5格/秒
            maxHealth = 80;
            health = 80;
            speed = 160;  // 2.5格/秒 = 160像素/秒
            setPhysicalAttributes(60.0f, 7, 8);  // 重量60kg，力量7，敏捷8（平衡型）
            pathfindingIntelligence = 2.0f; // 中等智能
            break;
            
        case ZombieType::TANK:
            // 坦克：高血量高伤害但慢 1.5格/秒
            maxHealth = 300;
            health = 300;
            radius = 20;
            speed = 96;   // 1.5格/秒 = 96像素/秒
            setPhysicalAttributes(150.0f, 16, 2);  // 重量150kg，力量16，敏捷2（超级坦克）
            pathfindingIntelligence = 1.8f; // 中等智能
            break;
    }
    
    // 统一设置感知范围（以格为单位，使用常量）
    setVisualRange(80);  // 80格视觉范围
    setHearingRange(10); // 10格听觉范围
    setSmellRange(5);    // 5格嗅觉范围
    
    // 创建个人气味源
    personalScent = std::make_unique<ScentSource>(
        this, x, y, 30, 128.0f, "丧尸", -1
    );
    
    // 设置丧尸特有的标志
    addFlag(EntityFlag::IS_ZOMBIE);
    //addFlag(EntityFlag::undead);
    
    // 添加攻击方式
    addAttackAbilities();
    
    // 设置通用物理属性
    isStatic = false;      // 丧尸是动态物体
}

// 更新方法
void Zombie::update(float deltaTime) {
    // 先调用基类更新
    Creature::update(deltaTime);
    
    // 更新个人气味位置
    updatePersonalScent();
    
    // 更新状态计时器
    int deltaTimeMs = static_cast<int>(deltaTime * 1000);
    stateTimer -= deltaTimeMs;
    
    // 更新攻击冷却
    if (attackCooldown > 0) {
        attackCooldown -= deltaTimeMs;
    }
    
    // 更新寻路失败冷却
    if (pathfindingFailureCooldown > 0) {
        pathfindingFailureCooldown -= deltaTimeMs;
    }
    
    // 更新攻击冷却（新系统）
    updateAttackCooldowns(deltaTimeMs);
    
    // 丧尸的核心感知和行为逻辑
    if (zombieState != ZombieState::DEAD) {
        // 获取游戏中的所有实体
        Game* game = Game::getInstance();
        if (game) {
            // 获取所有可能的目标（玩家和其他生物）
            std::vector<Entity*> potentialTargets;
            
            // 添加玩家
            if (game->getPlayer()) {
                potentialTargets.push_back(game->getPlayer());
            }
            
            // 添加其他生物（非丧尸）
            for (const auto& creature : game->getCreatures()) {
                if (creature.get() != this && !creature->hasFlag(EntityFlag::IS_ZOMBIE)) {
                    potentialTargets.push_back(creature.get());
                }
            }
            
            // 检查视觉感知
            std::vector<Entity*> visibleTargets = getVisibleEntities(potentialTargets);
            
            if (!visibleTargets.empty()) {
                // 如果看到任何非丧尸生物，追逐最近的那个
                Entity* closestTarget = nullptr;
                float closestDistance = std::numeric_limits<float>::max();
                
                for (Entity* target : visibleTargets) {
                    float distance = distanceToTarget(target->getX(), target->getY());
                    if (distance < closestDistance) {
                        closestDistance = distance;
                        closestTarget = target;
                    }
                }
                
                if (closestTarget) {
                    visualTarget = closestTarget;
                    // 只有在非攻击状态时才设置为追逐状态
                    if (zombieState != ZombieState::ATTACKING) {
                        setZombieState(ZombieState::CHASING);
                    }
                }
            } else {
                // 没有看到目标，检查其他感知
                // TODO: 嗅觉感知（等待重构）
                
                // 检查听觉感知
                // 这里需要从游戏中获取声音源列表
                // 暂时使用简单的实现
                if (zombieState == ZombieState::IDLE || zombieState == ZombieState::WANDERING) {
                    // TODO: 实现声音检测逻辑
                    // 需要Game类提供声音源列表的接口
                }
            }
        }
        
        // 根据当前状态执行行为
        // std::cout << "当前僵尸状态: " << static_cast<int>(zombieState) << std::endl;
        switch (zombieState) {
            case ZombieState::IDLE:
                handleIdleState(deltaTime);
                break;
            case ZombieState::WANDERING:
                handleWanderingState(deltaTime);
                break;
            case ZombieState::INVESTIGATING:
                handleInvestigatingState(deltaTime);
                break;
            case ZombieState::CHASING:
                handleChasingState(deltaTime);
                break;
            case ZombieState::ATTACKING:
                handleAttackingState(deltaTime);
                break;
            case ZombieState::FEEDING:
                handleFeedingState(deltaTime);
                break;
            case ZombieState::STUNNED:
                handleStunnedState(deltaTime);
                break;
            case ZombieState::DEAD:
                handleDeadState(deltaTime);
                break;
        }
    }
    
    // 更新丧尸AI
    updateAI(deltaTime);
}

// 渲染方法
void Zombie::render(SDL_Renderer* renderer, float cameraX, float cameraY) {
    // 调用基类渲染
    Creature::render(renderer, cameraX, cameraY);
    
    // 根据丧尸类型设置不同的颜色
    SDL_Color zombieColor = color;
    switch (zombieType) {
        case ZombieType::NORMAL:
            zombieColor = {100, 100, 100, 255}; // 灰色
            break;
        case ZombieType::RUNNER:
            zombieColor = {120, 80, 80, 255};   // 红灰色
            break;
        case ZombieType::BLOATER:
            zombieColor = {80, 120, 80, 255};   // 绿灰色
            break;
        case ZombieType::SPITTER:
            zombieColor = {120, 120, 80, 255};  // 黄灰色
            break;
        case ZombieType::TANK:
            zombieColor = {60, 60, 60, 255};    // 深灰色
            break;
    }
    
    // 重新绘制丧尸本体
    int screenX = static_cast<int>(x - cameraX);
    int screenY = static_cast<int>(y - cameraY);
    
    SDL_SetRenderDrawColor(renderer, zombieColor.r, zombieColor.g, zombieColor.b, zombieColor.a);
    SDL_FRect zombieRect = {
        static_cast<float>(screenX - radius), 
        static_cast<float>(screenY - radius), 
        static_cast<float>(radius * 2), 
        static_cast<float>(radius * 2)
    };
    SDL_RenderFillRect(renderer, &zombieRect);
}

// 状态处理方法实现
void Zombie::handleIdleState(float deltaTime) {
    // 空闲状态：偶尔切换到徘徊状态
    if (stateTimer <= 0) {
        // 随机决定是否开始徘徊
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 100);
        
        if (dis(gen) < 70) { // 70%概率开始徘徊（提高概率）
            setZombieState(ZombieState::WANDERING);
        } else {
            stateTimer = 1000 + dis(gen) * 20; // 1-3秒后再次检查（缩短时间）
        }
    }
}

void Zombie::handleWanderingState(float deltaTime) {
    // 徘徊状态：随机挪动1-2格
    if (wanderTimer <= 0) {
        // 选择新的徘徊目标（1-2格距离）
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> gridDis(1, 2); // 1-2格
        std::uniform_real_distribution<> angleDis(0.0, 2.0 * 3.14159); // 随机角度
        
        int grids = gridDis(gen);
        double angle = angleDis(gen);
        int distance = grids * GameConstants::TILE_SIZE; // 每格的像素数
        
        wanderX = x + static_cast<int>(distance * cos(angle));
        wanderY = y + static_cast<int>(distance * sin(angle));
        wanderTimer = 1500 + gridDis(gen) * 500; // 1.5-2.5秒的徘徊时间
    }
    
    // 向目标移动
            moveToPosition(static_cast<float>(wanderX), static_cast<float>(wanderY), deltaTime);
    
    wanderTimer -= static_cast<int>(deltaTime * 1000);
    
    // 如果到达目标或时间用完，切换到空闲状态
    if (wanderTimer <= 0 || distanceToTarget(wanderX, wanderY) < 32.0f) {
        setZombieState(ZombieState::IDLE);
    }
}

void Zombie::handleInvestigatingState(float deltaTime) {
    // 调查状态：前往声源或气味源，同时继续检测视觉目标
    
    // 首先检查是否有新的视觉目标
    Game* game = Game::getInstance();
    if (game) {
        std::vector<Entity*> potentialTargets;
        
        // 添加玩家
        if (game->getPlayer()) {
            potentialTargets.push_back(game->getPlayer());
        }
        
        // 添加其他生物（非丧尸）
        for (const auto& creature : game->getCreatures()) {
            if (creature.get() != this && !creature->hasFlag(EntityFlag::IS_ZOMBIE)) {
                potentialTargets.push_back(creature.get());
            }
        }
        
        // 检查视觉感知
        std::vector<Entity*> visibleTargets = getVisibleEntities(potentialTargets);
        
        if (!visibleTargets.empty()) {
            // 找到新目标，立即切换到追逐状态
            Entity* closestTarget = nullptr;
            float closestDistance = std::numeric_limits<float>::max();
            
            for (Entity* target : visibleTargets) {
                float distance = distanceToTarget(target->getX(), target->getY());
                if (distance < closestDistance) {
                    closestDistance = distance;
                    closestTarget = target;
                }
            }
            
            if (closestTarget) {
                visualTarget = closestTarget;
                setZombieState(ZombieState::CHASING);
                return;
            }
        }
    }
    
    // 没有视觉目标，继续调查行为
    if (soundTarget) {
        moveToPosition(static_cast<float>(soundTarget->x), static_cast<float>(soundTarget->y), deltaTime);
        
        // 如果到达声源位置
        if (distanceToTarget(soundTarget->x, soundTarget->y) < GameConstants::TILE_SIZE) {
            soundTarget = nullptr;
            setZombieState(ZombieState::WANDERING);
        }
    } else {
        // 没有声音目标，随机徘徊调查
        if (wanderTimer <= 0) {
            // 在附近随机选择调查点
            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_real_distribution<> angleDis(0.0, 2.0 * 3.14159);
            std::uniform_int_distribution<> distDis(GameConstants::TILE_SIZE, GameConstants::TILE_SIZE * 3); // 1-3格距离
            
            double angle = angleDis(gen);
            int distance = distDis(gen);
            
            wanderX = x + static_cast<int>(distance * cos(angle));
            wanderY = y + static_cast<int>(distance * sin(angle));
            wanderTimer = 1000; // 1秒移动时间
        }
        
        moveToPosition(static_cast<float>(wanderX), static_cast<float>(wanderY), deltaTime);
        wanderTimer -= static_cast<int>(deltaTime * 1000);
    }
    
    investigateTimer -= static_cast<int>(deltaTime * 1000);
    if (investigateTimer <= 0) {
        setZombieState(ZombieState::WANDERING);
    }
}

void Zombie::handleChasingState(float deltaTime) {
    // 追逐状态：追逐视觉目标
    if (visualTarget) {
        // 检查目标是否仍然可见
        if (canSeeEntity(visualTarget)) {
            moveToPosition(visualTarget->getX(), visualTarget->getY(), deltaTime);
            
            // 如果接近目标，切换到攻击状态
            float distance = distanceToTarget(visualTarget->getX(), visualTarget->getY());
            if (distance < 48.0f) {
                // std::cout << "僵尸接近目标，距离: " << distance << "，切换到攻击状态" << std::endl;
                setZombieState(ZombieState::ATTACKING);
            }
        } else {
            // 失去目标，先清除视觉目标，然后重新检测
            // std::cout << "僵尸失去视觉目标" << std::endl;
            visualTarget = nullptr;
            
            // 重新检测是否有其他可见目标
            Game* game = Game::getInstance();
            if (game) {
                std::vector<Entity*> potentialTargets;
                
                // 添加玩家
                if (game->getPlayer()) {
                    potentialTargets.push_back(game->getPlayer());
                }
                
                // 添加其他生物（非丧尸）
                for (const auto& creature : game->getCreatures()) {
                    if (creature.get() != this && !creature->hasFlag(EntityFlag::IS_ZOMBIE)) {
                        potentialTargets.push_back(creature.get());
                    }
                }
                
                // 检查视觉感知
                std::vector<Entity*> visibleTargets = getVisibleEntities(potentialTargets);
                
                if (!visibleTargets.empty()) {
                    // 找到新目标，继续追逐
                    Entity* closestTarget = nullptr;
                    float closestDistance = std::numeric_limits<float>::max();
                    
                    for (Entity* target : visibleTargets) {
                        float distance = distanceToTarget(target->getX(), target->getY());
                        if (distance < closestDistance) {
                            closestDistance = distance;
                            closestTarget = target;
                        }
                    }
                    
                    if (closestTarget) {
                        visualTarget = closestTarget;
                        // 继续追逐新目标
                        return;
                    }
                }
            }
            
            // 没有找到新目标，切换到调查状态
            setZombieState(ZombieState::INVESTIGATING);
            investigateTimer = 3000; // 调查3秒
        }
    } else {
        setZombieState(ZombieState::IDLE);
    }
}

void Zombie::handleAttackingState(float deltaTime) {
    // 攻击状态
    // std::cout << "进入攻击状态处理，visualTarget: " << (visualTarget ? "有" : "无") << ", attackCooldown: " << attackCooldown << std::endl;
    
    if (visualTarget && attackCooldown <= 0) {
        // std::cout << "僵尸尝试攻击目标，冷却时间: " << attackCooldown << std::endl;
        if (tryAttack(visualTarget)) {
            attackCooldown = 1500; // 1.5秒攻击冷却
            // std::cout << "攻击成功，设置冷却时间1500ms" << std::endl;
        } else {
            // std::cout << "tryAttack返回false" << std::endl;
        }
    } else if (visualTarget) {
        // std::cout << "僵尸攻击冷却中，剩余: " << attackCooldown << "ms" << std::endl;
    } else {
        // std::cout << "没有visualTarget" << std::endl;
    }
    
    // 检查距离
    if (visualTarget) {
        float distance = distanceToTarget(visualTarget->getX(), visualTarget->getY());
        // std::cout << "当前距离: " << distance << std::endl;
        
        // 如果目标太远，回到追逐状态
        if (distance > 64.0f) {
            // std::cout << "目标太远，切换到追逐状态" << std::endl;
            setZombieState(ZombieState::CHASING);
        }
    } else {
        // 如果没有目标，回到空闲状态
        // std::cout << "没有目标，切换到空闲状态" << std::endl;
        setZombieState(ZombieState::IDLE);
    }
}

void Zombie::handleFeedingState(float deltaTime) {
    // 进食状态（暂时简单实现）
    stateTimer -= static_cast<int>(deltaTime * 1000);
    if (stateTimer <= 0) {
        setZombieState(ZombieState::IDLE);
    }
}

void Zombie::handleStunnedState(float deltaTime) {
    // 眩晕状态：无法行动
    stateTimer -= static_cast<int>(deltaTime * 1000);
    if (stateTimer <= 0) {
        setZombieState(ZombieState::IDLE);
    }
}

void Zombie::handleDeadState(float deltaTime) {
    // 死亡状态：什么都不做
}

// 设置丧尸状态
void Zombie::setZombieState(ZombieState newState) {
    if (zombieState != newState) {
        // std::cout << "僵尸状态改变: " << static_cast<int>(zombieState) << " -> " << static_cast<int>(newState) << std::endl;
        zombieState = newState;
        
        // 根据新状态设置计时器
        switch (newState) {
            case ZombieState::IDLE:
                stateTimer = 2000; // 2秒
                break;
            case ZombieState::WANDERING:
                stateTimer = 5000; // 5秒
                break;
            case ZombieState::INVESTIGATING:
                stateTimer = 8000; // 8秒
                break;
            case ZombieState::ATTACKING:
                stateTimer = -1; // 无限制
                break;
            case ZombieState::FEEDING:
                stateTimer = 3000; // 3秒
                break;
            case ZombieState::STUNNED:
                stateTimer = 2000; // 2秒
                break;
            default:
                stateTimer = -1;
                break;
        }
    }
}

// 尝试攻击目标
bool Zombie::tryAttack(Entity* target) {
    if (!target || attackCooldown > 0) {
        return false;
    }
    
    // 检查攻击距离
    float distance = distanceToTarget(target->getX(), target->getY());
    if (distance > 48.0f) {
        return false;
    }
    
    // 添加调试输出
    // std::cout << "僵尸正在攻击！距离: " << distance << std::endl;
    
    // 创建伤害
    Damage damage(this);
    
    // 根据丧尸类型设置不同的伤害
    switch (zombieType) {
        case ZombieType::NORMAL:
            damage.addDamage(DamageType::PIERCE, 20);
            // std::cout << "普通僵尸攻击，造成20点刺击伤害" << std::endl;
            break;
        case ZombieType::RUNNER:
            damage.addDamage(DamageType::PIERCE, 15);
            // std::cout << "奔跑者攻击，造成15点刺击伤害" << std::endl;
            break;
        case ZombieType::BLOATER:
            damage.addDamage(DamageType::BLUNT, 30);
            // std::cout << "臃肿者攻击，造成30点钝击伤害" << std::endl;
            break;
        case ZombieType::SPITTER:
            damage.addDamage(DamageType::TOXIC, 25);
            // std::cout << "喷吐者攻击，造成25点毒素伤害" << std::endl;
            break;
        case ZombieType::TANK:
            damage.addDamage(DamageType::BLUNT, 40);
            // std::cout << "坦克攻击，造成40点钝击伤害" << std::endl;
            break;
    }
    
    // 造成伤害
    bool damageResult = target->takeDamage(damage);
    // std::cout << "伤害结果: " << (damageResult ? "成功" : "失败") << std::endl;
    // std::cout << "目标血量: " << target->getHealth() << std::endl;
    
    return true;
}

// 计算到目标的距离
float Zombie::distanceToTarget(float targetX, float targetY) const {
    float dx = targetX - x;
    float dy = targetY - y;
    return sqrt(dx * dx + dy * dy);
}

// 移动到指定位置（智能寻路）
void Zombie::moveToPosition(float targetX, float targetY, float deltaTime) {
    // 检查是否在寻路失败冷却期间，且目标位置相同
    if (pathfindingFailureCooldown > 0) {
        float distanceToLastFailedTarget = std::sqrt(
            (targetX - lastTargetX) * (targetX - lastTargetX) + 
            (targetY - lastTargetY) * (targetY - lastTargetY)
        );
        
        // 如果目标位置与上次寻路失败的位置很接近（一个瓦片内），则使用直线移动
        if (distanceToLastFailedTarget < GameConstants::TILE_SIZE) {
            // 寻路失败冷却期间，使用直线移动
            moveDirectlyToPosition(targetX, targetY, deltaTime);
            return;
        }
    }
    
    // 设置寻路目标
    if (!hasValidPathTarget() || 
        abs(static_cast<int>(this->targetX) - static_cast<int>(targetX)) > 32 ||
        abs(static_cast<int>(this->targetY) - static_cast<int>(targetY)) > 32) {
        setPathTarget(targetX, targetY);
    }
    
    // 使用寻路系统
    Game* game = Game::getInstance();
    if (game && game->getPathfinder()) {
        updatePathfinding(deltaTime, game->getPathfinder());
        
        // 检查寻路是否失败
        if (!hasValidPathTarget()) {
            // 检查寻路器中是否有有效路径
            const std::vector<PathPoint>* path = game->getPathfinder()->getCreaturePath(this);
            bool hasValidPath = (path != nullptr && !path->empty());
            
            if (!hasValidPath) {
                // 寻路失败，记录失败信息并设置冷却时间
                lastTargetX = targetX;
                lastTargetY = targetY;
                pathfindingFailureCooldown = 2000; // 2秒冷却时间
                
                // 按照用户方案：寻路失败时使用直线移动
                moveDirectlyToPosition(targetX, targetY, deltaTime);
                return;
            }
        }
    } else {
        // 如果寻路系统不可用，使用直线移动
        moveDirectlyToPosition(targetX, targetY, deltaTime);
    }
}

// 新增：使用寻路系统移动到目标
void Zombie::moveToPositionWithPathfinding(float targetX, float targetY, float deltaTime) {
    // 设置寻路目标
    setPathTarget(targetX, targetY);
    
    // 使用寻路系统
    Game* game = Game::getInstance();
    if (game && game->getPathfinder()) {
        updatePathfinding(deltaTime, game->getPathfinder());
    }
}

// 新增：直接移动到目标（不寻路）
void Zombie::moveDirectlyToPosition(float targetX, float targetY, float deltaTime) {
    float dx = targetX - x;
    float dy = targetY - y;
    float distance = sqrt(dx * dx + dy * dy);
    
    if (distance > 1.0f) {
        // 归一化方向向量
        dx /= distance;
        dy /= distance;
        
        // 计算移动速度
        float actualSpeed = speed * deltaTime;
        
        // 计算目标位置
        float newX = x + dx * actualSpeed;
        float newY = y + dy * actualSpeed;
        
        // 预检测碰撞 - 使用和Creature::moveDirectlyToTarget相同的逻辑
        bool canMove = true;
        
        Game* game = Game::getInstance();
        if (game && game->getMap()) {
            // 根据生物碰撞箱类型创建临时碰撞箱
            std::unique_ptr<Collider> tempCollider;
            if (collider.getType() == ColliderType::BOX) {
                tempCollider = std::make_unique<Collider>(
                    newX, newY, collider.getWidth(), collider.getHeight(), 
                    "temp_zombie", ColliderPurpose::ENTITY, 0);
            } else if (collider.getType() == ColliderType::CIRCLE) {
                tempCollider = std::make_unique<Collider>(
                    newX, newY, collider.getRadius(), 
                    "temp_zombie", ColliderPurpose::ENTITY, 0);
            } else {
                // 如果类型未知，不移动
                return;
            }
            
            // 获取生物可能影响的tile范围
            int minTileX = GameConstants::worldToTileCoord(newX - collider.getWidth()/2);
            int maxTileX = GameConstants::worldToTileCoord(newX + collider.getWidth()/2);
            int minTileY = GameConstants::worldToTileCoord(newY - collider.getHeight()/2);
            int maxTileY = GameConstants::worldToTileCoord(newY + collider.getHeight()/2);
            
            // 检查与地形的碰撞
            for (int tileX = minTileX; tileX <= maxTileX && canMove; tileX++) {
                for (int tileY = minTileY; tileY <= maxTileY && canMove; tileY++) {
                    Tile* tile = game->getMap()->getTileAt(GameConstants::tileCoordToWorld(tileX), GameConstants::tileCoordToWorld(tileY));
                    if (tile && tile->hasColliderWithPurpose(ColliderPurpose::TERRAIN)) {
                        auto terrainColliders = tile->getCollidersByPurpose(ColliderPurpose::TERRAIN);
                        for (Collider* terrainCollider : terrainColliders) {
                            if (tempCollider->intersects(*terrainCollider)) {
                                canMove = false;
                                break;
                            }
                        }
                    }
                }
            }
        }
        
        if (canMove) {
            // 没有碰撞，安全移动
            x = newX;
            y = newY;
            collider.updatePosition(x, y);
        }
        // 如果有碰撞，就不移动（不恢复位置，因为我们是预检测）
    }
}

// 更新个人气味
void Zombie::updatePersonalScent() {
    if (personalScent) {
        personalScent->updatePosition(x, y);
        personalScent->update(16); // 假设16ms更新间隔
    }
}

// 添加攻击能力
void Zombie::addAttackAbilities() {
    // 根据僵尸类型添加不同的攻击方式
    switch (zombieType) {
        case ZombieType::NORMAL: {
            // 普通僵尸：抓取 + 咬击
            auto grabAttack = std::make_unique<CreatureAttack>(
                "抓取", AttackType::GRAB, 15, 48, 1000, 0.8f, AttackEffect::NONE
            );
            auto biteAttack = std::make_unique<CreatureAttack>(
                "咬击", AttackType::BITE, 25, 32, 1500, 0.9f, AttackEffect::BLEEDING | AttackEffect::INFECTION
            );
            addAttack(std::move(grabAttack));
            addAttack(std::move(biteAttack));
            break;
        }
        case ZombieType::RUNNER: {
            // 奔跑者：快速爪击 + 冲撞
            auto clawAttack = std::make_unique<CreatureAttack>(
                "爪击", AttackType::CLAW, 20, 40, 800, 0.85f, AttackEffect::BLEEDING
            );
            auto slamAttack = std::make_unique<CreatureAttack>(
                "冲撞", AttackType::SLAM, 30, 48, 2000, 0.7f, AttackEffect::KNOCKBACK | AttackEffect::STUN
            );
            addAttack(std::move(clawAttack));
            addAttack(std::move(slamAttack));
            break;
        }
        case ZombieType::BLOATER: {
            // 臃肿者：重击 + 毒气
            auto slamAttack = std::make_unique<CreatureAttack>(
                "重击", AttackType::SLAM, 40, 56, 2500, 0.7f, AttackEffect::KNOCKBACK
            );
            auto poisonAttack = std::make_unique<CreatureAttack>(
                "毒气", AttackType::SPECIAL, 15, 80, 3000, 0.9f, AttackEffect::POISON | AttackEffect::WEAKEN
            );
            addAttack(std::move(slamAttack));
            addAttack(std::move(poisonAttack));
            break;
        }
        case ZombieType::SPITTER: {
            // 喷吐者：酸液喷射 + 咬击
            auto spitAttack = std::make_unique<CreatureAttack>(
                "酸液喷射", AttackType::RANGED, 30, 128, 2000, 0.8f, AttackEffect::POISON
            );
            auto biteAttack = std::make_unique<CreatureAttack>(
                "咬击", AttackType::BITE, 20, 32, 1200, 0.9f, AttackEffect::POISON | AttackEffect::INFECTION
            );
            addAttack(std::move(spitAttack));
            addAttack(std::move(biteAttack));
            break;
        }
        case ZombieType::TANK: {
            // 坦克：毁灭重击 + 践踏
            auto crushAttack = std::make_unique<CreatureAttack>(
                "毁灭重击", AttackType::SLAM, 60, 64, 3000, 0.8f, AttackEffect::KNOCKBACK | AttackEffect::STUN
            );
            auto trampleAttack = std::make_unique<CreatureAttack>(
                "践踏", AttackType::SPECIAL, 45, 72, 2500, 0.75f, AttackEffect::IMMOBILIZE | AttackEffect::WEAKEN
            );
            addAttack(std::move(crushAttack));
            addAttack(std::move(trampleAttack));
            break;
        }
    }
}