#include "Event.h"
#include "Entity.h"
#include "Fragment.h"
#include "Game.h"
#include "Constants.h"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>

// =============================================================================
// Event 基类实现
// =============================================================================

Event::Event(EventType eventType, EventPriority eventPriority, const std::string& desc)
    : type(eventType), priority(eventPriority), description(desc), processed(false), cancelled(false) {
    timestamp = std::chrono::steady_clock::now();
}

std::string Event::getEventInfo() const {
    std::stringstream ss;
    ss << "Event[Type=" << static_cast<int>(type) 
       << ", Priority=" << static_cast<int>(priority)
       << ", Processed=" << (processed ? "Yes" : "No")
       << ", Cancelled=" << (cancelled ? "Yes" : "No")
       << ", Description='" << description << "']";
    return ss.str();
}

// =============================================================================
// CoordinateEvent 坐标事件实现
// =============================================================================

CoordinateEvent::CoordinateEvent(EventType eventType, float eventX, float eventY, float eventRadius,
                                EventPriority priority, const std::string& desc)
    : Event(eventType, priority, desc), x(eventX), y(eventY), radius(eventRadius) {
}

float CoordinateEvent::getDistanceTo(float targetX, float targetY) const {
    float dx = x - targetX;
    float dy = y - targetY;
    return std::sqrt(dx * dx + dy * dy);
}

float CoordinateEvent::getDistanceTo(const Entity* entity) const {
    if (!entity) return -1.0f;
    
    // 假设Entity有getX()和getY()方法
    return getDistanceTo(entity->getX(), entity->getY());
}

bool CoordinateEvent::isInRange(float targetX, float targetY) const {
    if (radius <= 0.0f) return false;
    return getDistanceTo(targetX, targetY) <= radius;
}

bool CoordinateEvent::isInRange(const Entity* entity) const {
    if (!entity || radius <= 0.0f) return false;
    return getDistanceTo(entity) <= radius;
}

std::string CoordinateEvent::getEventInfo() const {
    std::stringstream ss;
    ss << "CoordinateEvent[" << Event::getEventInfo() 
       << ", Position=(" << std::fixed << std::setprecision(1) << x << "," << y << ")"
       << ", Radius=" << radius << "]";
    return ss.str();
}

// =============================================================================
// EntityEvent 实体事件实现
// =============================================================================

EntityEvent::EntityEvent(EventType eventType, Entity* target, Entity* source,
                        EventPriority priority, const std::string& desc)
    : Event(eventType, priority, desc), targetEntity(target), sourceEntity(source) {
}

bool EntityEvent::validate() const {
    // 目标实体必须有效
    return targetEntity != nullptr;
}

std::string EntityEvent::getEventInfo() const {
    std::stringstream ss;
    ss << "EntityEvent[" << Event::getEventInfo() 
       << ", Target=" << (targetEntity ? "Valid" : "Null")
       << ", Source=" << (sourceEntity ? "Valid" : "Null") << "]";
    return ss.str();
}

// =============================================================================
// ExplosionEvent 爆炸事件实现
// =============================================================================

// 完整构造函数
ExplosionEvent::ExplosionEvent(float explosionX, float explosionY, float explosionRadiusInGrids,
                              const std::vector<std::pair<DamageType, int>>& damages,
                              int fragments, int fragDamage, float fragRangeInGrids,
                              const std::string& type, Entity* explosionSource)
    : CoordinateEvent(EventType::EXPLOSION, explosionX, explosionY, GameConstants::gridsToPixels(explosionRadiusInGrids), 
                     EventPriority::HIGH, "Explosion at (" + std::to_string(explosionX) + "," + std::to_string(explosionY) + ")"),
      explosionDamages(damages), fragmentCount(fragments), fragmentDamage(fragDamage),
      fragmentRange(GameConstants::gridsToPixels(fragRangeInGrids)), fragmentMinSpeed(400.0f), fragmentMaxSpeed(800.0f),
      explosionType(type), source(explosionSource) {
}

// 便捷构造函数（兼容旧接口）
ExplosionEvent::ExplosionEvent(float explosionX, float explosionY, float explosionRadiusInGrids,
                              float totalDamage, int fragments, const std::string& type)
    : CoordinateEvent(EventType::EXPLOSION, explosionX, explosionY, GameConstants::gridsToPixels(explosionRadiusInGrids), 
                     EventPriority::HIGH, "Explosion at (" + std::to_string(explosionX) + "," + std::to_string(explosionY) + ")"),
      fragmentCount(fragments), fragmentDamage(20), fragmentRange(GameConstants::gridsToPixels(7.0f)),
      fragmentMinSpeed(400.0f), fragmentMaxSpeed(800.0f), explosionType(type), source(nullptr) {
    
    // 将总伤害分解为高温伤害和钝击伤害
    explosionDamages.push_back({DamageType::HEAT, static_cast<int>(totalDamage * 0.5f)});
    explosionDamages.push_back({DamageType::BLUNT, static_cast<int>(totalDamage * 0.5f)});
}

int ExplosionEvent::getTotalDamage() const {
    int total = 0;
    for (const auto& damage : explosionDamages) {
        total += damage.second;
    }
    return total;
}

void ExplosionEvent::addDamage(DamageType type, int amount) {
    if (amount <= 0) return;
    
    // 查找是否已存在相同类型的伤害
    for (auto& damage : explosionDamages) {
        if (damage.first == type) {
            damage.second += amount;
            return;
        }
    }
    
    // 不存在，添加新的伤害类型
    explosionDamages.push_back({type, amount});
}

Damage ExplosionEvent::calculateDamageAtDistance(float distance) const {
    Damage result(source);
    
    if (distance >= radius || explosionDamages.empty()) {
        return result; // 空伤害
    }
    
    // 计算距离衰减系数
    float damageRatio = 1.0f;
    if (distance > 0.0f) {
        // 线性衰减：中心点满伤害，边缘处无伤害
        damageRatio = 1.0f - (distance / radius);
        damageRatio = std::max(0.0f, damageRatio);
    }
    
    // 应用衰减到所有伤害类型
    for (const auto& damageInfo : explosionDamages) {
        int adjustedDamage = static_cast<int>(damageInfo.second * damageRatio);
        if (adjustedDamage > 0) {
            result.addDamage(damageInfo.first, adjustedDamage);
        }
    }
    
    return result;
}

float ExplosionEvent::calculateTotalDamageAtDistance(float distance) const {
    Damage damage = calculateDamageAtDistance(distance);
    return static_cast<float>(damage.getTotalDamage());
}

void ExplosionEvent::execute() {
    printf("执行爆炸事件: 位置(%.1f,%.1f), 半径%.1f格\n", x, y, radius);
    
    Game* game = Game::getInstance();
    if (!game) {
        printf("警告: 无法获取游戏实例，爆炸事件无法执行\n");
        markProcessed();
        return;
    }
    
    // 1. 获取所有可能受影响的实体
    std::vector<Entity*> allEntities;
    
    // 添加玩家
    if (game->getPlayer()) {
        allEntities.push_back(game->getPlayer());
    }
    
    // 添加所有丧尸
    const auto& zombies = game->getZombies();
    for (const auto& zombie : zombies) {
        if (zombie && zombie->getHealth() > 0) {
            allEntities.push_back(zombie.get());
        }
    }
    
    // 添加所有生物
    const auto& creatures = game->getCreatures();
    for (const auto& creature : creatures) {
        if (creature && creature->getHealth() > 0) {
            allEntities.push_back(creature.get());
        }
    }
    
    // 2. 对范围内的实体造成爆炸伤害
    int entitiesHit = 0;
    for (Entity* entity : allEntities) {
        if (!entity) continue;
        
        float distance = getDistanceTo(entity);
        if (distance <= radius) {
            // 计算该距离的伤害
            Damage explosionDamage = calculateDamageAtDistance(distance);
            
            if (!explosionDamage.isEmpty()) {
                entity->takeDamage(explosionDamage);
                entitiesHit++;
                
                printf("  实体在距离%.1f处受到%d点爆炸伤害\n", 
                       distance, explosionDamage.getTotalDamage());
            }
        }
    }
    
    // 3. 生成弹片
    if (fragmentCount > 0) {
        FragmentManager& fragmentManager = FragmentManager::getInstance();
        
        fragmentManager.createExplosionFragments(
            x, y, fragmentCount, 
            fragmentMinSpeed, fragmentMaxSpeed, 
            fragmentRange, // 已经是像素值了
            fragmentDamage, source
        );
        
        printf("  生成了%d个弹片，每个造成%d点刺击伤害\n", fragmentCount, fragmentDamage);
    }
    
    // 4. 添加视觉效果（TODO: 实现爆炸视觉效果）
    // TODO: 添加爆炸粒子效果
    // TODO: 添加爆炸声音效果
    // TODO: 添加屏幕震动效果
    
    // 标记为已处理
    markProcessed();
    
    printf("爆炸执行完成: 命中%d个实体, 生成%d个弹片\n", entitiesHit, fragmentCount);
}

std::string ExplosionEvent::getEventInfo() const {
    std::stringstream ss;
    ss << "ExplosionEvent[" << CoordinateEvent::getEventInfo()
       << ", TotalDamage=" << getTotalDamage()
       << ", DamageTypes=" << explosionDamages.size()
       << ", Fragments=" << fragmentCount
       << ", FragmentDamage=" << fragmentDamage
       << ", FragmentRange=" << std::fixed << std::setprecision(1) << fragmentRange
       << ", Type='" << explosionType << "']";
    return ss.str();
}

bool ExplosionEvent::validate() const {
    // 验证爆炸事件参数是否有效
    return radius > 0.0f && fragmentCount >= 0 && fragmentDamage >= 0 && fragmentRange >= 0.0f;
} 