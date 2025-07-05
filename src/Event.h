#pragma once
#ifndef EVENT_H
#define EVENT_H

#include <memory>
#include <string>
#include <functional>
#include <chrono>
#include <vector>
#include "Damage.h"

// 前置声明
class Entity;

// 事件类型枚举
enum class EventType {
    UNKNOWN = 0,
    
    // 坐标事件类型
    EXPLOSION,           // 爆炸事件
    SOUND_EFFECT,        // 声音效果
    PARTICLE_EFFECT,     // 粒子效果
    AREA_DAMAGE,         // 区域伤害
    
    // 实体事件类型
    ENTITY_DAMAGE,       // 实体伤害
    ENTITY_HEAL,         // 实体治疗
    ENTITY_DEATH,        // 实体死亡
    ENTITY_SPAWN,        // 实体生成
    
    // 系统事件类型
    GAME_STATE_CHANGE,   // 游戏状态变化
    UI_UPDATE,           // UI更新
    SAVE_GAME            // 保存游戏
};

// 事件优先级
enum class EventPriority {
    LOW = 0,
    NORMAL = 1,
    HIGH = 2,
    CRITICAL = 3
};

// 事件基类
class Event {
protected:
    EventType type;                                    // 事件类型
    EventPriority priority;                            // 事件优先级
    std::chrono::steady_clock::time_point timestamp;   // 事件时间戳
    bool processed;                                    // 是否已处理
    bool cancelled;                                    // 是否已取消
    std::string description;                           // 事件描述

public:
    Event(EventType eventType, EventPriority eventPriority = EventPriority::NORMAL, const std::string& desc = "");
    virtual ~Event() = default;

    // 获取事件信息
    EventType getType() const { return type; }
    EventPriority getPriority() const { return priority; }
    std::chrono::steady_clock::time_point getTimestamp() const { return timestamp; }
    const std::string& getDescription() const { return description; }
    
    // 状态管理
    bool isProcessed() const { return processed; }
    bool isCancelled() const { return cancelled; }
    void markProcessed() { processed = true; }
    void cancel() { cancelled = true; }
    
    // 虚方法供子类重写
    virtual void execute() = 0;                        // 执行事件
    virtual std::string getEventInfo() const;          // 获取事件信息字符串
    virtual bool validate() const { return true; }     // 验证事件是否有效
};

// 坐标事件基类
class CoordinateEvent : public Event {
protected:
    float x, y;                                        // 事件坐标
    float radius;                                      // 影响半径

public:
    CoordinateEvent(EventType eventType, float eventX, float eventY, float eventRadius = 0.0f,
                   EventPriority priority = EventPriority::NORMAL, const std::string& desc = "");
    
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
    Entity* sourceEntity;                              // 事件源实体（可选）

public:
    EntityEvent(EventType eventType, Entity* target, Entity* source = nullptr,
               EventPriority priority = EventPriority::NORMAL, const std::string& desc = "");
    
    // 实体相关方法
    Entity* getTargetEntity() const { return targetEntity; }
    Entity* getSourceEntity() const { return sourceEntity; }
    void setTargetEntity(Entity* target) { targetEntity = target; }
    void setSourceEntity(Entity* source) { sourceEntity = source; }
    
    bool validate() const override;                    // 验证实体是否有效
    std::string getEventInfo() const override;
};

// 爆炸事件类
class ExplosionEvent : public CoordinateEvent {
private:
    std::vector<std::pair<DamageType, int>> explosionDamages; // 多种伤害类型
    int fragmentCount;                                      // 破片数量
    int fragmentDamage;                                     // 每个弹片的伤害
    float fragmentRange;                                    // 弹片飞行距离（格数）
    float fragmentMinSpeed;                                 // 弹片最小速度
    float fragmentMaxSpeed;                                 // 弹片最大速度
    std::string explosionType;                              // 爆炸类型（手榴弹、炸弹等）
    Entity* source;                                         // 爆炸源实体
    
public:
    // 完整构造函数
    ExplosionEvent(float explosionX, float explosionY, float explosionRadiusInGrids,
                  const std::vector<std::pair<DamageType, int>>& damages,
                  int fragments, int fragDamage, float fragRangeInGrids,
                  const std::string& type = "爆炸", Entity* explosionSource = nullptr);
    
    // 便捷构造函数（兼容旧接口）
    ExplosionEvent(float explosionX, float explosionY, float explosionRadiusInGrids,
                  float totalDamage, int fragments = 0, const std::string& type = "爆炸");
    
    // 爆炸参数访问器
    const std::vector<std::pair<DamageType, int>>& getExplosionDamages() const { return explosionDamages; }
    int getTotalDamage() const;
    int getFragmentCount() const { return fragmentCount; }
    int getFragmentDamage() const { return fragmentDamage; }
    float getFragmentRange() const { return fragmentRange; }
    float getFragmentMinSpeed() const { return fragmentMinSpeed; }
    float getFragmentMaxSpeed() const { return fragmentMaxSpeed; }
    const std::string& getExplosionType() const { return explosionType; }
    Entity* getSource() const { return source; }
    
    // 爆炸参数设置器
    void addDamage(DamageType type, int amount);
    void setFragmentCount(int count) { fragmentCount = count; }
    void setFragmentDamage(int fragDamage) { fragmentDamage = fragDamage; }
    void setFragmentRange(float range) { fragmentRange = range; }
    void setFragmentSpeed(float minSpeed, float maxSpeed) { fragmentMinSpeed = minSpeed; fragmentMaxSpeed = maxSpeed; }
    void setExplosionType(const std::string& type) { explosionType = type; }
    void setSource(Entity* explosionSource) { source = explosionSource; }
    
    // 伤害计算
    Damage calculateDamageAtDistance(float distance) const;    // 根据距离计算完整伤害对象
    float calculateTotalDamageAtDistance(float distance) const; // 根据距离计算总伤害值
    
    void execute() override;                                   // 执行爆炸事件
    std::string getEventInfo() const override;
    bool validate() const override;
};

#endif // EVENT_H 