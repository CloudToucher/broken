#include "Event.h"
#include "Entity.h"
#include "Fragment.h"
#include "Game.h"
#include "Constants.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <random>
#include <memory>

// 定义M_PI常量（如果未定义）
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// =============================================================================
// Event 基类实现
// =============================================================================

Event::Event(EventType eventType, const EventSource& eventSource, 
             EventPriority eventPriority, const std::string& desc, float eventDuration)
    : type(eventType), priority(eventPriority), source(eventSource), description(desc),
      status(EventStatus::PENDING), duration(eventDuration), elapsedTime(0.0f),
      isPersistent(eventDuration != 0.0f), updateInterval(0.1f), lastUpdateTime(0.0f) {
    timestamp = std::chrono::steady_clock::now();
}

bool Event::needsUpdate() const {
    if (!isPersistent || status != EventStatus::ACTIVE) return false;
    return (elapsedTime - lastUpdateTime) >= updateInterval;
}

void Event::update(float deltaTime) {
    // 默认实现：更新时间，调用回调
    if (isPersistent && status == EventStatus::ACTIVE) {
        updateTime(deltaTime);
        
        if (needsUpdate()) {
            if (onUpdate) {
                onUpdate(deltaTime);
            }
            lastUpdateTime = elapsedTime;
        }
        
        // 检查是否过期
        if (isExpired()) {
            finish();
        }
    }
}

void Event::finish() {
    if (onEnd) {
        onEnd();
    }
    status = EventStatus::COMPLETED;
}

void Event::updateTime(float deltaTime) {
    elapsedTime += deltaTime;
}

std::string Event::getEventInfo() const {
    std::stringstream ss;
    ss << "Event[Type=" << static_cast<int>(type) 
       << ", Priority=" << static_cast<int>(priority)
       << ", Status=" << static_cast<int>(status)
       << ", Source=" << (source.isEntity() ? "Entity" : (source.isEnvironment() ? "Environment" : "System"))
       << ", Persistent=" << (isPersistent ? "Yes" : "No");
    
    if (isPersistent) {
        ss << ", Duration=" << duration
           << ", Elapsed=" << std::fixed << std::setprecision(1) << elapsedTime;
    }
    
    ss << ", Description='" << description << "']";
    return ss.str();
}

// =============================================================================
// CoordinateEvent 坐标事件实现
// =============================================================================

CoordinateEvent::CoordinateEvent(EventType eventType, const EventSource& eventSource, 
                                float eventX, float eventY, float eventRadius,
                                EventPriority priority, const std::string& desc, float eventDuration)
    : Event(eventType, eventSource, priority, desc, eventDuration), 
      x(eventX), y(eventY), radius(eventRadius) {
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

EntityEvent::EntityEvent(EventType eventType, const EventSource& eventSource, Entity* target,
                        EventPriority priority, const std::string& desc, float eventDuration)
    : Event(eventType, eventSource, priority, desc, eventDuration), targetEntity(target) {
}

bool EntityEvent::validate() const {
    // 目标实体必须有效
    return targetEntity != nullptr;
}

std::string EntityEvent::getEventInfo() const {
    std::stringstream ss;
    ss << "EntityEvent[" << Event::getEventInfo() 
       << ", Target=" << (targetEntity ? "Valid" : "Null") << "]";
    return ss.str();
}

// =============================================================================
// ExplosionEvent 爆炸事件实现
// =============================================================================

// 完整构造函数
ExplosionEvent::ExplosionEvent(float explosionX, float explosionY, float explosionRadiusInGrids,
                              const std::vector<std::pair<DamageType, int>>& damages,
                              int fragments, int fragDamage, float fragRangeInGrids,
                              const EventSource& eventSource,
                              const std::string& type)
    : CoordinateEvent(EventType::EXPLOSION, eventSource, explosionX, explosionY, 
                     GameConstants::gridsToPixels(explosionRadiusInGrids), 
                     EventPriority::HIGH, "Explosion at (" + std::to_string(explosionX) + "," + std::to_string(explosionY) + ")"),
      explosionDamages(damages), fragmentCount(fragments), fragmentDamage(fragDamage),
      fragmentRange(GameConstants::gridsToPixels(fragRangeInGrids)), fragmentMinSpeed(400.0f), fragmentMaxSpeed(800.0f),
      explosionType(type) {
}

// 便捷构造函数（兼容旧接口）
ExplosionEvent::ExplosionEvent(float explosionX, float explosionY, float explosionRadiusInGrids,
                              float totalDamage, int fragments, const EventSource& eventSource,
                              const std::string& type)
    : CoordinateEvent(EventType::EXPLOSION, eventSource, explosionX, explosionY, 
                     GameConstants::gridsToPixels(explosionRadiusInGrids), 
                     EventPriority::HIGH, "Explosion at (" + std::to_string(explosionX) + "," + std::to_string(explosionY) + ")"),
      fragmentCount(fragments), fragmentDamage(20), fragmentRange(GameConstants::gridsToPixels(7.0f)),
      fragmentMinSpeed(400.0f), fragmentMaxSpeed(800.0f), explosionType(type) {
    
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
    Damage result(source.isEntity() ? source.entity : nullptr);
    
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
    printf("执行爆炸事件: 位置(%.1f,%.1f), 半径%.1f格, 来源=%s\n", 
           x, y, radius / 64.0f, source.description.c_str());
    
    markActive();
    
    if (onStart) {
        onStart();
    }
    
    Game* game = Game::getInstance();
    if (!game) {
        printf("警告: 无法获取游戏实例，爆炸事件无法执行\n");
        markCompleted();
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
            fragmentDamage, source.isEntity() ? source.entity : nullptr
        );
        
        printf("  生成了%d个弹片，每个造成%d点刺击伤害\n", fragmentCount, fragmentDamage);
    }
    
    // 4. 添加视觉效果（TODO: 实现爆炸视觉效果）
    // TODO: 添加爆炸粒子效果
    // TODO: 添加爆炸声音效果
    // TODO: 添加屏幕震动效果
    
    // 标记为已完成（爆炸是即时事件）
    markCompleted();
    
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

// =============================================================================
// 新的持续事件类实现
// =============================================================================

// SmokeCloudEvent 烟雾云事件
SmokeCloudEvent::SmokeCloudEvent(float x, float y, float radius, float smokeDuration, 
                                const EventSource& source, float smokeIntensity, float smokeDensity)
    : CoordinateEvent(EventType::SMOKE_CLOUD, source, x, y, radius, 
                     EventPriority::NORMAL, "Smoke cloud", smokeDuration),
      density(smokeDensity), visibilityReduction(smokeDensity * 0.8f), 
      dissipationRate(1.0f / smokeDuration), intensity(smokeIntensity),
      particlesGenerated(false) {
    setUpdateInterval(0.1f); // 每0.1秒更新一次，更频繁的更新
}

void SmokeCloudEvent::execute() {
    printf("烟雾弹爆炸: 位置(%.1f,%.1f), 半径%.1f, 强度%.1f, 持续%.1f秒\n", 
           x, y, radius, intensity, duration);
    markActive();
    
    // 生成烟雾颗粒
    generateParticles();
    
    if (onStart) {
        onStart();
    }
}

void SmokeCloudEvent::update(float deltaTime) {
    Event::update(deltaTime);
    
    if (status != EventStatus::ACTIVE) return;
    
    // 烟雾逐渐消散
    float dissipationAmount = dissipationRate * deltaTime;
    density = std::max(0.0f, density - dissipationAmount);
    visibilityReduction = density * 0.8f;
    
    // 更新所有烟雾颗粒
    for (auto& particle : particles) {
        particle.update(deltaTime);
    }
    
    // 清理死亡的颗粒
    cleanupDeadParticles();
    
    // 输出状态信息（减少频率）
    static float debugTimer = 0.0f;
    debugTimer += deltaTime;
    if (debugTimer >= 1.0f) {
        printf("烟雾云状态: 密度=%.2f, 颗粒数=%zu, 视野影响=%.2f\n", 
               density, particles.size(), visibilityReduction);
        debugTimer = 0.0f;
    }
}

void SmokeCloudEvent::finish() {
    printf("烟雾云消散完成，清理%zu个颗粒\n", particles.size());
    
    // 清理所有颗粒
    particles.clear();
    
    Event::finish();
}

std::string SmokeCloudEvent::getEventInfo() const {
    std::stringstream ss;
    ss << "SmokeCloudEvent[" << CoordinateEvent::getEventInfo()
       << ", Density=" << std::fixed << std::setprecision(2) << density
       << ", Intensity=" << intensity
       << ", Particles=" << particles.size()
       << ", VisibilityReduction=" << visibilityReduction << "]";
    return ss.str();
}

// === SmokeCloudEvent 新增方法实现 ===

void SmokeCloudEvent::generateParticles() {
    if (particlesGenerated) return;
    
    // 基于强度和半径计算颗粒数量
    // 基础公式：颗粒密度 = 强度 * 面积 / 颗粒大小的平方
    const float PARTICLE_SIZE = 6.4f; // 0.1格 = 6.4像素（64像素每格）
    const float PARTICLE_DENSITY_MULTIPLIER = 0.3f; // 调整密度
    
    float area = M_PI * radius * radius;
    int baseParticleCount = (int)(area * intensity * PARTICLE_DENSITY_MULTIPLIER / (PARTICLE_SIZE * PARTICLE_SIZE));
    
    // 确保有最小数量的颗粒
    int particleCount = std::max(20, baseParticleCount);
    
    printf("生成烟雾颗粒: 半径%.1f, 强度%.1f, 颗粒数%d\n", radius, intensity, particleCount);
    
    // 随机数生成器
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * M_PI);
    std::uniform_real_distribution<float> lifespanDist(duration * 0.7f, duration * 1.3f);
    
    // 生成颗粒，使用圆形分布，边缘密度较低
    for (int i = 0; i < particleCount; ++i) {
        float angle = angleDist(gen);
        
        // 使用平方根分布，让颗粒在中心更密集，边缘更稀疏
        std::uniform_real_distribution<float> radiusDist(0.0f, 1.0f);
        float normalizedRadius = std::sqrt(radiusDist(gen)); // 平方根分布
        float particleRadius = normalizedRadius * radius;
        
        float particleX = x + particleRadius * std::cos(angle);
        float particleY = y + particleRadius * std::sin(angle);
        float particleLifespan = lifespanDist(gen);
        
        particles.emplace_back(particleX, particleY, PARTICLE_SIZE, particleLifespan);
    }
    
    particlesGenerated = true;
}

void SmokeCloudEvent::cleanupDeadParticles() {
    particles.erase(
        std::remove_if(particles.begin(), particles.end(), 
                      [](const SmokeParticle& p) { return p.isDead(); }),
        particles.end()
    );
}

std::vector<Collider*> SmokeCloudEvent::getActiveVisionColliders() const {
    std::vector<Collider*> activeColliders;
    
    for (const auto& particle : particles) {
        if (!particle.isDead() && particle.visionCollider && particle.visionCollider->getIsActive()) {
            activeColliders.push_back(particle.visionCollider.get());
        }
    }
    
    return activeColliders;
}

bool SmokeCloudEvent::isPointInSmoke(float px, float py) const {
    // 首先检查是否在大致范围内
    float distanceToCenter = std::sqrt((px - x) * (px - x) + (py - y) * (py - y));
    if (distanceToCenter > radius) {
        return false;
    }
    
    // 检查是否有颗粒覆盖这个点
    for (const auto& particle : particles) {
        if (particle.isDead()) continue;
        
        float particleDistance = std::sqrt((px - particle.x) * (px - particle.x) + (py - particle.y) * (py - particle.y));
        if (particleDistance <= particle.size / 2.0f && particle.opacity > 0.3f) {
            return true;
        }
    }
    
    return false;
}

void SmokeCloudEvent::renderSmoke(SDL_Renderer* renderer, float cameraX, float cameraY) const {
    if (status != EventStatus::ACTIVE) return;
    
    // 渲染所有烟雾颗粒
    for (const auto& particle : particles) {
        particle.render(renderer, cameraX, cameraY);
    }
}

// FireAreaEvent 燃烧区域事件
FireAreaEvent::FireAreaEvent(float x, float y, float radius, float fireDuration,
                            const EventSource& source, int dps)
    : CoordinateEvent(EventType::FIRE_AREA, source, x, y, radius,
                     EventPriority::HIGH, "Fire area", fireDuration),
      damagePerSecond(dps), spreadRate(0.1f), fuelRemaining(fireDuration * 10.0f) {
    setUpdateInterval(1.0f); // 每秒更新一次，造成伤害
}

void FireAreaEvent::execute() {
    printf("燃烧区域开始: 位置(%.1f,%.1f), 半径%.1f, 持续%.1f秒, DPS=%d\n", 
           x, y, radius, duration, damagePerSecond);
    markActive();
    
    if (onStart) {
        onStart();
    }
}

void FireAreaEvent::update(float deltaTime) {
    Event::update(deltaTime);
    
    if (status != EventStatus::ACTIVE) return;
    
    // 消耗燃料
    fuelRemaining -= deltaTime;
    if (fuelRemaining <= 0) {
        finish();
        return;
    }
    
    // 对范围内的实体造成伤害
    Game* game = Game::getInstance();
    if (game) {
        std::vector<Entity*> allEntities;
        
        if (game->getPlayer()) {
            allEntities.push_back(game->getPlayer());
        }
        
        const auto& zombies = game->getZombies();
        for (const auto& zombie : zombies) {
            if (zombie && zombie->getHealth() > 0) {
                allEntities.push_back(zombie.get());
            }
        }
        
        const auto& creatures = game->getCreatures();
        for (const auto& creature : creatures) {
            if (creature && creature->getHealth() > 0) {
                allEntities.push_back(creature.get());
            }
        }
        
        int entitiesHit = 0;
        for (Entity* entity : allEntities) {
            if (!entity) continue;
            
            if (isInRange(entity)) {
                Damage fireDamage(source.isEntity() ? source.entity : nullptr);
                fireDamage.addDamage(DamageType::HEAT, damagePerSecond);
                entity->takeDamage(fireDamage);
                entitiesHit++;
            }
        }
        
        if (entitiesHit > 0) {
            printf("燃烧区域伤害: %d个实体受到%d点火焰伤害\n", entitiesHit, damagePerSecond);
        }
    }
    
    // TODO: 火焰蔓延逻辑
    // TODO: 添加火焰粒子效果
}

void FireAreaEvent::finish() {
    printf("燃烧区域熄灭\n");
    Event::finish();
}

std::string FireAreaEvent::getEventInfo() const {
    std::stringstream ss;
    ss << "FireAreaEvent[" << CoordinateEvent::getEventInfo()
       << ", DPS=" << damagePerSecond
       << ", FuelRemaining=" << std::fixed << std::setprecision(1) << fuelRemaining << "]";
    return ss.str();
}

// TeleportGateEvent 传送门事件
TeleportGateEvent::TeleportGateEvent(float gateX, float gateY, float gateRadius,
                                    float destX, float destY, float gateDuration,
                                    const EventSource& source, bool bidirectional)
    : CoordinateEvent(EventType::TELEPORT_GATE, source, gateX, gateY, gateRadius,
                     EventPriority::NORMAL, "Teleport gate", gateDuration),
      targetX(destX), targetY(destY), isBidirectional(bidirectional) {
    setUpdateInterval(0.1f); // 每0.1秒检查一次传送
}

bool TeleportGateEvent::canTeleport(Entity* entity) const {
    if (!entity) return false;
    
    // 如果允许实体列表为空，则所有实体都可以传送
    if (allowedEntities.empty()) return true;
    
    // 检查实体是否在允许列表中
    return std::find(allowedEntities.begin(), allowedEntities.end(), entity) != allowedEntities.end();
}

void TeleportGateEvent::execute() {
    printf("传送门激活: 位置(%.1f,%.1f) -> (%.1f,%.1f), 半径%.1f, 持续%.1f秒\n", 
           x, y, targetX, targetY, radius, duration);
    markActive();
    
    if (onStart) {
        onStart();
    }
}

void TeleportGateEvent::update(float deltaTime) {
    Event::update(deltaTime);
    
    if (status != EventStatus::ACTIVE) return;
    
    // 检查传送
    Game* game = Game::getInstance();
    if (game) {
        std::vector<Entity*> allEntities;
        
        if (game->getPlayer()) {
            allEntities.push_back(game->getPlayer());
        }
        
        const auto& zombies = game->getZombies();
        for (const auto& zombie : zombies) {
            if (zombie && zombie->getHealth() > 0) {
                allEntities.push_back(zombie.get());
            }
        }
        
        const auto& creatures = game->getCreatures();
        for (const auto& creature : creatures) {
            if (creature && creature->getHealth() > 0) {
                allEntities.push_back(creature.get());
            }
        }
        
        for (Entity* entity : allEntities) {
            if (!entity || !canTeleport(entity)) continue;
            
            if (isInRange(entity)) {
                // 执行传送
                // TODO: 添加传送特效
                printf("实体传送: (%.1f,%.1f) -> (%.1f,%.1f)\n", 
                       entity->getX(), entity->getY(), targetX, targetY);
                
                // 这里需要Entity类支持setPosition方法
                // entity->setPosition(targetX, targetY);
                
                // TODO: 添加传送冷却，防止实体在传送门之间反复跳跃
            }
        }
    }
}

void TeleportGateEvent::finish() {
    printf("传送门关闭\n");
    Event::finish();
}

std::string TeleportGateEvent::getEventInfo() const {
    std::stringstream ss;
    ss << "TeleportGateEvent[" << CoordinateEvent::getEventInfo()
       << ", Target=(" << std::fixed << std::setprecision(1) << targetX << "," << targetY << ")"
       << ", Bidirectional=" << (isBidirectional ? "Yes" : "No")
       << ", AllowedEntities=" << allowedEntities.size() << "]";
    return ss.str();
} 