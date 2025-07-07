#pragma once
#ifndef EVENT_H
#define EVENT_H

#include <memory>
#include <string>
#include <functional>
#include <chrono>
#include <vector>
#include <random>
#include <cmath>
#include <SDL3/SDL.h>
#include "Damage.h"
#include "Collider.h"

// 前置声明
class Entity;

// 事件来源类型
enum class EventSourceType {
    ENTITY,       // 来自实体
    ENVIRONMENT,  // 来自环境
    SYSTEM        // 来自系统
};

// 事件来源结构体
struct EventSource {
    EventSourceType type;
    Entity* entity;  // 如果来源是实体，指向该实体；否则为nullptr
    std::string description; // 来源描述（如"地雷爆炸"、"天气系统"等）
    
    EventSource(Entity* sourceEntity, const std::string& desc = "")
        : type(EventSourceType::ENTITY), entity(sourceEntity), description(desc) {}
    
    EventSource(EventSourceType sourceType, const std::string& desc = "")
        : type(sourceType), entity(nullptr), description(desc) {}
    
    static EventSource FromEntity(Entity* entity, const std::string& desc = "") {
        return EventSource(entity, desc);
    }
    
    static EventSource FromEnvironment(const std::string& desc = "环境") {
        return EventSource(EventSourceType::ENVIRONMENT, desc);
    }
    
    static EventSource FromSystem(const std::string& desc = "系统") {
        return EventSource(EventSourceType::SYSTEM, desc);
    }
    
    bool isEntity() const { return type == EventSourceType::ENTITY && entity != nullptr; }
    bool isEnvironment() const { return type == EventSourceType::ENVIRONMENT; }
    bool isSystem() const { return type == EventSourceType::SYSTEM; }
};

// 事件类型枚举 - 重新设计为更具扩展性
enum class EventType {
    UNKNOWN = 0,
    
    // === 即时事件类型 ===
    // 爆炸类事件
    EXPLOSION,           // 爆炸事件
    SHOCKWAVE,          // 冲击波
    
    // 伤害治疗事件
    ENTITY_DAMAGE,       // 实体伤害
    ENTITY_HEAL,         // 实体治疗
    AREA_DAMAGE,         // 区域伤害
    
    // 实体状态变化事件
    ENTITY_DEATH,        // 实体死亡
    ENTITY_SPAWN,        // 实体生成
    ENTITY_TELEPORT,     // 实体传送
    
    // 物理事件
    FORCE_IMPULSE,       // 力冲量
    GRAVITY_CHANGE,      // 重力变化
    
    // 音效视觉事件
    SOUND_EFFECT,        // 声音效果
    PARTICLE_EFFECT,     // 粒子效果
    SCREEN_SHAKE,        // 屏幕震动
    
    // 系统事件
    GAME_STATE_CHANGE,   // 游戏状态变化
    UI_UPDATE,           // UI更新
    SAVE_GAME,           // 保存游戏
    
    // === 持续事件类型 ===
    // 环境效果事件
    SMOKE_CLOUD,         // 烟雾云
    FIRE_AREA,           // 燃烧区域
    TOXIC_CLOUD,         // 毒气云
    RADIATION_FIELD,     // 辐射场
    ELECTROMAGNETIC_FIELD, // 电磁场
    
    // 天气事件
    RAIN,                // 下雨
    WIND,                // 风
    FOG,                 // 雾
    
    // 魔法/科幻效果
    MAGIC_FIELD,         // 魔法场
    ENERGY_BARRIER,      // 能量屏障
    TEMPORAL_DISTORTION, // 时间扭曲
    
    // 陷阱效果
    SPIKE_TRAP,          // 尖刺陷阱
    SLOWING_FIELD,       // 减速场
    HEALING_AURA,        // 治疗光环
    
    // 传送相关
    TELEPORT_GATE,       // 传送门
    TELEPORT_BEACON,     // 传送信标
    
    // 光照效果
    LIGHT_SOURCE,        // 光源
    DARKNESS_FIELD       // 黑暗场域
};

// 事件优先级
enum class EventPriority {
    LOW = 0,
    NORMAL = 1,
    HIGH = 2,
    CRITICAL = 3
};

// 事件状态
enum class EventStatus {
    PENDING,      // 等待处理
    ACTIVE,       // 正在进行中
    COMPLETED,    // 已完成
    CANCELLED,    // 已取消
    EXPIRED       // 已过期
};

// 事件基类
class Event {
protected:
    EventType type;                                    // 事件类型
    EventPriority priority;                            // 事件优先级
    EventSource source;                                // 事件来源
    std::chrono::steady_clock::time_point timestamp;   // 事件创建时间戳
    EventStatus status;                                // 事件状态
    std::string description;                           // 事件描述
    
    // 持续事件相关
    float duration;                                    // 持续时间（秒，-1表示永久）
    float elapsedTime;                                 // 已经过时间
    bool isPersistent;                                 // 是否为持续事件
    float updateInterval;                              // 更新间隔（秒）
    float lastUpdateTime;                              // 上次更新时间
    
    // 生命周期回调
    std::function<void()> onStart;                     // 开始时回调
    std::function<void(float)> onUpdate;               // 更新时回调
    std::function<void()> onEnd;                       // 结束时回调

public:
    Event(EventType eventType, const EventSource& eventSource, 
          EventPriority eventPriority = EventPriority::NORMAL, 
          const std::string& desc = "", float eventDuration = 0.0f);
    virtual ~Event() = default;

    // 获取事件信息
    EventType getType() const { return type; }
    EventPriority getPriority() const { return priority; }
    const EventSource& getSource() const { return source; }
    std::chrono::steady_clock::time_point getTimestamp() const { return timestamp; }
    const std::string& getDescription() const { return description; }
    EventStatus getStatus() const { return status; }
    
    // 持续事件相关
    float getDuration() const { return duration; }
    float getElapsedTime() const { return elapsedTime; }
    float getRemainingTime() const { return duration < 0 ? -1 : duration - elapsedTime; }
    bool getIsPersistent() const { return isPersistent; }
    float getUpdateInterval() const { return updateInterval; }
    bool isExpired() const { return duration >= 0 && elapsedTime >= duration; }
    bool needsUpdate() const;
    
    // 状态管理
    bool isPending() const { return status == EventStatus::PENDING; }
    bool isActive() const { return status == EventStatus::ACTIVE; }
    bool isCompleted() const { return status == EventStatus::COMPLETED; }
    bool isCancelled() const { return status == EventStatus::CANCELLED; }
    void markActive() { status = EventStatus::ACTIVE; }
    void markCompleted() { status = EventStatus::COMPLETED; }
    void cancel() { status = EventStatus::CANCELLED; }
    
    // 持续事件控制
    void setDuration(float newDuration) { duration = newDuration; isPersistent = (newDuration != 0.0f); }
    void setUpdateInterval(float interval) { updateInterval = interval; }
    void setLifecycleCallbacks(std::function<void()> start, std::function<void(float)> update, std::function<void()> end) {
        onStart = start; onUpdate = update; onEnd = end;
    }
    
    // 虚方法供子类重写
    virtual void execute() = 0;                        // 执行事件（即时事件）或开始事件（持续事件）
    virtual void update(float deltaTime);              // 更新持续事件
    virtual void finish();                             // 结束事件
    virtual std::string getEventInfo() const;          // 获取事件信息字符串
    virtual bool validate() const { return true; }     // 验证事件是否有效
    
    // 时间管理
    void updateTime(float deltaTime);
};

// 坐标事件基类
class CoordinateEvent : public Event {
protected:
    float x, y;                                        // 事件坐标
    float radius;                                      // 影响半径

public:
    CoordinateEvent(EventType eventType, const EventSource& eventSource, 
                   float eventX, float eventY, float eventRadius = 0.0f,
                   EventPriority priority = EventPriority::NORMAL, 
                   const std::string& desc = "", float eventDuration = 0.0f);
    
    // 坐标相关方法
    float getX() const { return x; }
    float getY() const { return y; }
    float getRadius() const { return radius; }
    void setPosition(float newX, float newY) { x = newX; y = newY; }
    void setRadius(float newRadius) { radius = newRadius; }
    
    // 距离计算
    float getDistanceTo(float targetX, float targetY) const;
    float getDistanceTo(const Entity* entity) const;
    bool isInRange(float targetX, float targetY) const;
    bool isInRange(const Entity* entity) const;
    
    std::string getEventInfo() const override;
};

// 实体事件基类
class EntityEvent : public Event {
protected:
    Entity* targetEntity;                              // 目标实体

public:
    EntityEvent(EventType eventType, const EventSource& eventSource, Entity* target,
               EventPriority priority = EventPriority::NORMAL, 
               const std::string& desc = "", float eventDuration = 0.0f);
    
    // 实体相关方法
    Entity* getTargetEntity() const { return targetEntity; }
    void setTargetEntity(Entity* target) { targetEntity = target; }
    
    bool validate() const override;                    // 验证实体是否有效
    std::string getEventInfo() const override;
};

// 爆炸事件类（重构以使用新的事件系统）
class ExplosionEvent : public CoordinateEvent {
private:
    std::vector<std::pair<DamageType, int>> explosionDamages; // 多种伤害类型
    int fragmentCount;                                      // 破片数量
    int fragmentDamage;                                     // 每个弹片的伤害
    float fragmentRange;                                    // 弹片飞行距离（格数）
    float fragmentMinSpeed;                                 // 弹片最小速度
    float fragmentMaxSpeed;                                 // 弹片最大速度
    std::string explosionType;                              // 爆炸类型（手榴弹、炸弹等）
    
public:
    // 完整构造函数
    ExplosionEvent(float explosionX, float explosionY, float explosionRadiusInGrids,
                  const std::vector<std::pair<DamageType, int>>& damages,
                  int fragments, int fragDamage, float fragRangeInGrids,
                  const EventSource& eventSource,
                  const std::string& type = "爆炸");
    
    // 便捷构造函数（兼容旧接口）
    ExplosionEvent(float explosionX, float explosionY, float explosionRadiusInGrids,
                  float totalDamage, int fragments, const EventSource& eventSource,
                  const std::string& type = "爆炸");
    
    // 爆炸参数访问器
    const std::vector<std::pair<DamageType, int>>& getExplosionDamages() const { return explosionDamages; }
    int getTotalDamage() const;
    int getFragmentCount() const { return fragmentCount; }
    int getFragmentDamage() const { return fragmentDamage; }
    float getFragmentRange() const { return fragmentRange; }
    float getFragmentMinSpeed() const { return fragmentMinSpeed; }
    float getFragmentMaxSpeed() const { return fragmentMaxSpeed; }
    const std::string& getExplosionType() const { return explosionType; }
    
    // 爆炸参数设置器
    void addDamage(DamageType type, int amount);
    void setFragmentCount(int count) { fragmentCount = count; }
    void setFragmentDamage(int fragDamage) { fragmentDamage = fragDamage; }
    void setFragmentRange(float range) { fragmentRange = range; }
    void setFragmentSpeed(float minSpeed, float maxSpeed) { fragmentMinSpeed = minSpeed; fragmentMaxSpeed = maxSpeed; }
    void setExplosionType(const std::string& type) { explosionType = type; }
    
    // 伤害计算
    Damage calculateDamageAtDistance(float distance) const;    // 根据距离计算完整伤害对象
    float calculateTotalDamageAtDistance(float distance) const; // 根据距离计算总伤害值
    
    void execute() override;                                   // 执行爆炸事件
    std::string getEventInfo() const override;
    bool validate() const override;
};

// === 新的持续事件类示例 ===

// 烟雾云事件
class SmokeCloudEvent : public CoordinateEvent {
private:
    float density;           // 烟雾密度 (0.0-1.0)
    float visibilityReduction; // 视野降低程度
    float dissipationRate;   // 消散速率
    float intensity;         // 烟雾强度，影响生成的颗粒数量
    
    // 烟雾颗粒系统
    struct SmokeParticle {
        float x, y;              // 位置
        float vx, vy;            // 速度向量
        float size;              // 大小（半径）
        float opacity;           // 不透明度 (0.0-1.0)
        float lifespan;          // 生命周期
        float maxLifespan;       // 最大生命周期
        float distanceFromCenter; // 距离烟雾中心的距离（用于边缘淡化）
        std::unique_ptr<Collider> visionCollider; // 视觉碰撞箱
        
        SmokeParticle(float px, float py, float pSize, float maxLife, float centerX, float centerY)
            : x(px), y(py), vx(0.0f), vy(0.0f), size(pSize), opacity(1.0f), lifespan(0.0f), maxLifespan(maxLife) {
            // 计算距离中心的距离
            distanceFromCenter = std::sqrt((px - centerX) * (px - centerX) + (py - centerY) * (py - centerY));
            
            // 添加随机速度（模拟烟雾飘动）
            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_real_distribution<float> speedDist(-10.0f, 10.0f);
            vx = speedDist(gen);
            vy = speedDist(gen);
            
            // 创建视觉碰撞箱
            visionCollider = std::make_unique<Collider>(
                x - size/2, y - size/2, size, size,
                "smoke_particle", ColliderPurpose::VISION, 10
            );
        }
        
        void update(float deltaTime, float smokeCenterX, float smokeCenterY, float smokeRadius) {
            lifespan += deltaTime;
            
            // 更新位置（动态移动）
            x += vx * deltaTime;
            y += vy * deltaTime;
            
            // 重新计算距离中心的距离
            distanceFromCenter = std::sqrt((x - smokeCenterX) * (x - smokeCenterX) + (y - smokeCenterY) * (y - smokeCenterY));
            
            // 计算基础不透明度（生命周期末期逐渐消散）
            float lifeFraction = lifespan / maxLifespan;
            float baseOpacity = 1.0f - lifeFraction;
            
            // 边缘淡化效果：距离中心越远，不透明度越低
            float edgeFadeFactor = 1.0f;
            if (smokeRadius > 0.0f) {
                float normalizedDistance = distanceFromCenter / smokeRadius;
                if (normalizedDistance > 0.7f) {
                    // 在70%半径之外开始淡化
                    edgeFadeFactor = 1.0f - ((normalizedDistance - 0.7f) / 0.3f);
                    edgeFadeFactor = std::max(0.0f, edgeFadeFactor);
                }
            }
            
            // 组合不透明度
            opacity = baseOpacity * edgeFadeFactor;
            if (opacity < 0.0f) opacity = 0.0f;
            
            // 随时间减缓速度（模拟空气阻力）
            vx *= 0.995f;
            vy *= 0.995f;
            
            // 更新碰撞箱位置
            if (visionCollider) {
                visionCollider->updatePosition(x - size/2, y - size/2);
                visionCollider->setIsActive(opacity > 0.1f); // 只有足够不透明的颗粒才阻挡视线
            }
        }
        
        bool isDead() const {
            return lifespan >= maxLifespan || opacity <= 0.0f;
        }
        
        void render(SDL_Renderer* renderer, float cameraX, float cameraY) const {
            if (opacity <= 0.0f) return;
            
            // 计算屏幕位置
            float screenX = x - cameraX - size/2;
            float screenY = y - cameraY - size/2;
            float screenSize = size;
            
            // 设置烟雾颜色（灰色，根据不透明度调整alpha）
            Uint8 alpha = (Uint8)(opacity * 128); // 半透明
            SDL_SetRenderDrawColor(renderer, 100, 100, 100, alpha);
            
            // 渲染圆形烟雾颗粒（使用简单的方形近似）
            SDL_FRect smokeRect = { screenX, screenY, screenSize, screenSize };
            SDL_RenderFillRect(renderer, &smokeRect);
        }
    };
    
    std::vector<SmokeParticle> particles; // 烟雾颗粒列表
    bool particlesGenerated;              // 是否已生成颗粒
    
    // 生成烟雾颗粒
    void generateParticles();
    
    // 清理死亡颗粒
    void cleanupDeadParticles();
    
public:
    SmokeCloudEvent(float x, float y, float radius, float smokeDuration, 
                   const EventSource& source, float smokeIntensity = 1.0f, float smokeDensity = 0.8f);
    
    float getDensity() const { return density; }
    float getVisibilityReduction() const { return visibilityReduction; }
    float getIntensity() const { return intensity; }
    size_t getParticleCount() const { return particles.size(); }
    
    // 获取所有活跃的视觉碰撞箱
    std::vector<Collider*> getActiveVisionColliders() const;
    
    // 检查指定点是否在烟雾中
    bool isPointInSmoke(float px, float py) const;
    
    // 渲染烟雾效果
    void renderSmoke(SDL_Renderer* renderer, float cameraX, float cameraY) const;
    
    void execute() override;
    void update(float deltaTime) override;
    void finish() override;
    std::string getEventInfo() const override;
};

// 燃烧区域事件
class FireAreaEvent : public CoordinateEvent {
private:
    int damagePerSecond;     // 每秒伤害
    float spreadRate;        // 蔓延速率
    float fuelRemaining;     // 剩余燃料
    
public:
    FireAreaEvent(float x, float y, float radius, float fireDuration,
                 const EventSource& source, int dps = 5);
    
    int getDamagePerSecond() const { return damagePerSecond; }
    float getSpreadRate() const { return spreadRate; }
    float getFuelRemaining() const { return fuelRemaining; }
    
    void execute() override;
    void update(float deltaTime) override;
    void finish() override;
    std::string getEventInfo() const override;
};

// 传送门事件
class TeleportGateEvent : public CoordinateEvent {
private:
    float targetX, targetY;  // 传送目标位置
    bool isBidirectional;    // 是否双向传送
    std::vector<Entity*> allowedEntities; // 允许传送的实体列表（空表示所有）
    
public:
    TeleportGateEvent(float gateX, float gateY, float gateRadius,
                     float destX, float destY, float gateDuration,
                     const EventSource& source, bool bidirectional = false);
    
    float getTargetX() const { return targetX; }
    float getTargetY() const { return targetY; }
    bool getIsBidirectional() const { return isBidirectional; }
    
    void addAllowedEntity(Entity* entity) { allowedEntities.push_back(entity); }
    bool canTeleport(Entity* entity) const;
    
    void execute() override;
    void update(float deltaTime) override;
    void finish() override;
    std::string getEventInfo() const override;
};

#endif // EVENT_H 