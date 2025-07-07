# 事件系统标准化升级总结

## 概述

本次升级对游戏的事件系统进行了全面的标准化和扩展，实现了用户要求的三个主要目标：
1. **标准化事件来源** - 所有事件现在都有明确的创造来源
2. **支持持续事件** - 实现了完整的持续事件生命周期管理
3. **更好的扩展性** - 为将来添加多种新事件类型（烟雾、燃烧、传送等）奠定了基础

## 主要改进

### 1. 事件来源系统 (EventSource)

**新增结构体：**
```cpp
struct EventSource {
    EventSourceType type;    // ENTITY, ENVIRONMENT, SYSTEM
    Entity* entity;          // 如果来自实体，指向该实体
    std::string description; // 来源描述
    
    // 便捷静态方法
    static EventSource FromEntity(Entity* entity, const std::string& desc = "");
    static EventSource FromEnvironment(const std::string& desc = "环境");
    static EventSource FromSystem(const std::string& desc = "系统");
};
```

**使用示例：**
```cpp
// 来自玩家的爆炸
EventSource playerSource = EventSource::FromEntity(player, "玩家手动触发");

// 来自环境的爆炸
EventSource envSource = EventSource::FromEnvironment("地雷爆炸");

// 来自系统的爆炸
EventSource sysSource = EventSource::FromSystem("测试系统");
```

### 2. 持续事件系统

**新增事件状态管理：**
- `EventStatus::PENDING` - 等待处理
- `EventStatus::ACTIVE` - 正在进行中
- `EventStatus::COMPLETED` - 已完成
- `EventStatus::CANCELLED` - 已取消
- `EventStatus::EXPIRED` - 已过期

**生命周期管理：**
```cpp
class Event {
    float duration;           // 持续时间（-1为永久）
    float elapsedTime;        // 已经过时间
    bool isPersistent;        // 是否为持续事件
    float updateInterval;     // 更新间隔
    
    // 生命周期回调
    std::function<void()> onStart;
    std::function<void(float)> onUpdate;
    std::function<void()> onEnd;
    
    virtual void update(float deltaTime);
    virtual void finish();
};
```

### 3. 新的持续事件类型

**SmokeCloudEvent (烟雾云)**
- 持续时间内降低视野
- 密度逐渐消散
- 可配置视野影响程度

**FireAreaEvent (燃烧区域)**
- 持续造成火焰伤害
- 支持燃料消耗机制
- 可配置每秒伤害量

**TeleportGateEvent (传送门)**
- 支持双向传送
- 可设置允许传送的实体列表
- 防止传送门之间反复跳跃的机制

### 4. 重新设计的事件类型系统

**分类更清晰：**
```cpp
enum class EventType {
    // === 即时事件类型 ===
    EXPLOSION, SHOCKWAVE, ENTITY_DAMAGE, ENTITY_HEAL,
    FORCE_IMPULSE, SOUND_EFFECT, PARTICLE_EFFECT,
    
    // === 持续事件类型 ===
    SMOKE_CLOUD, FIRE_AREA, TOXIC_CLOUD, RADIATION_FIELD,
    TELEPORT_GATE, HEALING_AURA, MAGIC_FIELD,
    
    // 预留未来扩展
    RAIN, WIND, FOG, ENERGY_BARRIER, TEMPORAL_DISTORTION
};
```

### 5. 升级的EventManager

**双队列系统：**
- `instantEventQueue` - 即时事件优先级队列
- `persistentEvents` - 持续事件列表

**新的处理流程：**
```cpp
void processEvents(float deltaTime) {
    processInstantEvents();           // 处理即时事件
    updatePersistentEvents(deltaTime); // 更新持续事件
    clearCompletedEvents();           // 清理已完成事件
}
```

**新增管理功能：**
- 持续事件的添加、移除、暂停、恢复
- 按类型查询持续事件
- 详细的事件统计和调试信息

## API 变化

### 构造函数更新

**旧接口：**
```cpp
ExplosionEvent(float x, float y, float radius, float damage, int fragments, const std::string& type);
```

**新接口：**
```cpp
ExplosionEvent(float x, float y, float radius, 
               const std::vector<std::pair<DamageType, int>>& damages,
               int fragments, int fragDamage, float fragRange,
               const EventSource& source, const std::string& type);
```

### 便捷方法扩展

**新增持续事件便捷方法：**
```cpp
// EventManager便捷方法
void triggerSmokeCloud(float x, float y, float radius, float duration, 
                      const EventSource& source, float density = 0.8f);
void triggerFireArea(float x, float y, float radius, float duration, 
                    const EventSource& source, int damagePerSecond = 5);
void triggerTeleportGate(float gateX, float gateY, float gateRadius, 
                        float destX, float destY, float duration,
                        const EventSource& source, bool bidirectional = false);

// 全局便捷函数
namespace EventSystem {
    void TriggerSmokeCloud(...);
    void TriggerFireArea(...);
    void TriggerTeleportGate(...);
}
```

## 兼容性

### 向后兼容

为保持兼容性，我们保留了便捷构造函数：
```cpp
// 仍然支持简单的爆炸创建
ExplosionEvent(float x, float y, float radius, float totalDamage, 
               int fragments, const EventSource& source, const std::string& type);
```

### 代码迁移

**需要更新的地方：**
1. `Game::triggerExplosionAtMouse()` - 已更新使用新的EventSource
2. `EventManager::processEvents()` - 已更新支持deltaTime参数
3. `EventTest.cpp` - 已更新测试新功能

## 扩展示例

### 添加新的持续事件

```cpp
// 新建毒气云事件
class ToxicCloudEvent : public CoordinateEvent {
private:
    int poisonDamagePerSecond;
    float toxicity;
    
public:
    ToxicCloudEvent(float x, float y, float radius, float duration,
                   const EventSource& source, int dps = 3, float tox = 0.5f)
        : CoordinateEvent(EventType::TOXIC_CLOUD, source, x, y, radius,
                         EventPriority::HIGH, "Toxic cloud", duration),
          poisonDamagePerSecond(dps), toxicity(tox) {
        setUpdateInterval(1.0f);
    }
    
    void execute() override { /* 激活毒气云 */ }
    void update(float deltaTime) override { /* 造成中毒伤害 */ }
    void finish() override { /* 毒气云消散 */ }
};
```

### 便捷方法集成

```cpp
// EventManager中添加
void triggerToxicCloud(float x, float y, float radius, float duration,
                      const EventSource& source, int dps = 3, float toxicity = 0.5f) {
    auto toxicCloud = std::make_shared<ToxicCloudEvent>(x, y, radius, duration, source, dps, toxicity);
    registerEvent(toxicCloud);
}

// 全局便捷函数
namespace EventSystem {
    void TriggerToxicCloud(float x, float y, float radius, float duration,
                          const EventSource& source, int dps = 3, float toxicity = 0.5f) {
        EventManager::getInstance().triggerToxicCloud(x, y, radius, duration, source, dps, toxicity);
    }
}
```

## 性能优化

1. **分离队列** - 即时事件和持续事件分别管理，提高处理效率
2. **智能清理** - 自动清理已完成和过期的事件
3. **按需更新** - 持续事件只在需要时更新
4. **内存管理** - 使用shared_ptr避免内存泄漏

## 调试功能

```cpp
// 新增调试方法
eventManager.debugPrintPersistentEvents();  // 打印持续事件状态
eventManager.getEventManagerStatus();       // 获取管理器状态字符串

// 查询功能
auto smokeEvents = eventManager.getPersistentEventsOfType(EventType::SMOKE_CLOUD);
size_t activeEvents = eventManager.getPersistentEventCount();
```

## 测试验证

EventTest.cpp 包含完整的测试案例：
- 即时事件处理测试
- 持续事件生命周期测试  
- 事件来源系统测试
- 便捷方法测试
- 查询功能测试
- 5-10秒的实时模拟测试

## 总结

这次升级实现了一个更加标准化、可扩展和功能完整的事件系统，为将来添加各种新的游戏机制（天气系统、魔法效果、陷阱系统等）提供了坚实的基础。所有现有代码保持兼容，新功能可以逐步集成到游戏中。 