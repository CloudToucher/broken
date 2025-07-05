# 事件系统使用说明

## 概述

事件系统是一个统一的事件处理框架，支持两种主要的事件类型：

1. **坐标事件** - 基于位置的事件（如爆炸、音效等）
2. **实体事件** - 针对特定实体的事件（如伤害、治疗等）

## 系统架构

```
Event (基类)
├── CoordinateEvent (坐标事件基类)
│   └── ExplosionEvent (爆炸事件)
└── EntityEvent (实体事件基类)
    ├── EntityDamageEvent (实体伤害事件) - 待实现
    └── EntityHealEvent (实体治疗事件) - 待实现
```

## 核心组件

### 1. 事件类型 (EventType)

```cpp
enum class EventType {
    EXPLOSION,          // 爆炸事件
    SOUND_EFFECT,       // 声音效果
    PARTICLE_EFFECT,    // 粒子效果
    AREA_DAMAGE,        // 区域伤害
    ENTITY_DAMAGE,      // 实体伤害
    ENTITY_HEAL,        // 实体治疗
    // ... 更多类型
};
```

### 2. 事件优先级 (EventPriority)

```cpp
enum class EventPriority {
    LOW = 0,
    NORMAL = 1,
    HIGH = 2,
    CRITICAL = 3
};
```

### 3. 事件管理器 (EventManager)

采用单例模式，负责事件的注册、分发和处理。

## 使用方法

### 1. 基本使用

```cpp
#include "EventManager.h"

// 获取事件管理器实例
EventManager& eventManager = EventManager::getInstance();

// 启用调试模式（可选）
eventManager.setDebugMode(true);
```

### 2. 创建和注册爆炸事件

```cpp
// 方法1：直接创建事件对象
auto explosion = std::make_shared<ExplosionEvent>(
    100.0f,     // X坐标
    200.0f,     // Y坐标
    50.0f,      // 爆炸半径
    75.0f,      // 爆炸伤害
    10,         // 破片数量
    "手榴弹"    // 爆炸类型
);
eventManager.registerEvent(explosion);

// 方法2：使用便捷方法
eventManager.triggerExplosion(300.0f, 400.0f, 100.0f, 120.0f, 20, "炸弹");

// 方法3：使用全局便捷函数
EventSystem::TriggerExplosion(500.0f, 600.0f, 25.0f, 40.0f, 5, "地雷");
```

### 3. 注册事件处理器

```cpp
// 注册爆炸事件处理器
eventManager.registerEventHandler(EventType::EXPLOSION, [](const Event* event) {
    const ExplosionEvent* explosion = dynamic_cast<const ExplosionEvent*>(event);
    if (explosion) {
        printf("爆炸发生在位置(%.1f, %.1f)，伤害%.1f\n",
               explosion->getX(), explosion->getY(), explosion->getDamage());
    }
});

// 注册全局事件处理器（处理所有事件）
eventManager.registerGlobalHandler([](const Event* event) {
    printf("处理事件类型: %d\n", static_cast<int>(event->getType()));
});
```

### 4. 处理事件

```cpp
// 处理所有待处理事件
eventManager.processEvents();

// 或使用全局便捷函数
EventSystem::ProcessEvents();

// 处理特定类型的事件
eventManager.processEventsOfType(EventType::EXPLOSION);
```

## 爆炸事件详细参数

### ExplosionEvent 构造函数参数

```cpp
ExplosionEvent(
    float explosionX,           // 爆炸X坐标
    float explosionY,           // 爆炸Y坐标
    float explosionRadius,      // 爆炸半径
    float explosionDamage,      // 爆炸伤害
    int fragments = 0,          // 破片数量（可选）
    const std::string& type = "generic"  // 爆炸类型（可选）
);
```

### 伤害计算

```cpp
// 根据距离计算伤害
float damage = explosion->calculateDamageAtDistance(distance);

// 根据距离计算破片伤害
float fragmentDamage = explosion->calculateFragmentDamageAtDistance(distance);
```

### 伤害衰减规则

- **最小伤害半径内**：造成满伤害
- **最大伤害半径内**：线性衰减
- **超出最大伤害半径**：指数衰减（最多10%伤害）

## 事件查询和统计

```cpp
// 查询事件数量
size_t eventCount = eventManager.getEventCount();
size_t explosionCount = eventManager.getEventCountOfType(EventType::EXPLOSION);

// 检查是否有事件
bool hasEvents = eventManager.hasEvents();
bool hasExplosions = eventManager.hasEventsOfType(EventType::EXPLOSION);

// 打印统计信息
eventManager.printStatistics();

// 调试打印事件队列
eventManager.debugPrintEventQueue();
```

## 事件清理

```cpp
// 清空所有事件
eventManager.clearEvents();

// 清空特定类型的事件
eventManager.clearEventsOfType(EventType::EXPLOSION);

// 清空已取消的事件
eventManager.clearCancelledEvents();
```

## 配置选项

```cpp
// 设置调试模式
eventManager.setDebugMode(true);

// 设置最大队列大小（防止内存溢出）
eventManager.setMaxQueueSize(1000);
```

## 完整示例

```cpp
#include "EventManager.h"

void setupEventSystem() {
    EventManager& eventManager = EventManager::getInstance();
    
    // 启用调试模式
    eventManager.setDebugMode(true);
    
    // 注册爆炸事件处理器
    eventManager.registerEventHandler(EventType::EXPLOSION, [](const Event* event) {
        const ExplosionEvent* explosion = dynamic_cast<const ExplosionEvent*>(event);
        if (explosion) {
            printf("爆炸！位置(%.1f,%.1f) 伤害%.1f 破片%d\n",
                   explosion->getX(), explosion->getY(),
                   explosion->getDamage(), explosion->getFragmentCount());
        }
    });
    
    // 创建爆炸事件
    eventManager.triggerExplosion(100.0f, 200.0f, 50.0f, 75.0f, 10, "手榴弹");
    
    // 处理所有事件
    eventManager.processEvents();
    
    // 打印统计信息
    eventManager.printStatistics();
}

// 游戏循环中调用
void gameLoop() {
    // 处理事件
    EventSystem::ProcessEvents();
    
    // 其他游戏逻辑...
}

// 程序结束时清理
void cleanup() {
    EventManager::destroyInstance();
}
```

## 扩展指南

### 添加新的事件类型

1. 在 `EventType` 枚举中添加新类型
2. 创建新的事件类，继承自 `Event`、`CoordinateEvent` 或 `EntityEvent`
3. 实现 `execute()` 方法
4. 在 `EventManager` 中添加便捷方法（可选）

### 添加新的坐标事件

```cpp
class SoundEffectEvent : public CoordinateEvent {
private:
    std::string soundFile;
    float volume;
    
public:
    SoundEffectEvent(float x, float y, float radius, const std::string& file, float vol)
        : CoordinateEvent(EventType::SOUND_EFFECT, x, y, radius), 
          soundFile(file), volume(vol) {}
    
    void execute() override {
        // 播放音效的实现
        printf("播放音效: %s 在位置(%.1f,%.1f)\n", soundFile.c_str(), getX(), getY());
        markProcessed();
    }
};
```

## 注意事项

1. **内存管理**：事件使用智能指针管理，自动释放内存
2. **异常处理**：事件处理器和事件执行都有异常捕获
3. **性能考虑**：使用优先级队列确保重要事件优先处理
4. **调试模式**：建议在开发阶段启用调试模式以获取详细日志
5. **队列限制**：设置最大队列大小防止内存溢出

## 测试

运行 `EventTest.cpp` 中的测试函数来验证事件系统的功能：

```cpp
testEventSystem();
```

这将创建多个测试事件，注册处理器，并演示完整的事件生命周期。 