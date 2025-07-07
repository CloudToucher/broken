#pragma once
#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include "Event.h"
#include <queue>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <list>

// 事件处理器类型定义
using EventHandler = std::function<void(const Event*)>;

// 事件比较器，用于优先级队列
struct EventComparator {
    bool operator()(const std::shared_ptr<Event>& a, const std::shared_ptr<Event>& b) const {
        // 优先级高的先处理，如果优先级相同则按时间先后处理
        if (a->getPriority() != b->getPriority()) {
            return static_cast<int>(a->getPriority()) < static_cast<int>(b->getPriority());
        }
        return a->getTimestamp() > b->getTimestamp();
    }
};

// 事件管理器类
class EventManager {
private:
    // 即时事件队列（优先级队列）
    std::priority_queue<std::shared_ptr<Event>, std::vector<std::shared_ptr<Event>>, EventComparator> instantEventQueue;
    
    // 持续事件列表（使用list便于中途删除）
    std::list<std::shared_ptr<Event>> persistentEvents;
    
    // 事件处理器映射 (事件类型 -> 处理器列表)
    std::map<EventType, std::vector<EventHandler>> eventHandlers;
    
    // 全局事件处理器（处理所有事件）
    std::vector<EventHandler> globalHandlers;
    
    // 事件统计
    int totalEventsProcessed;
    int totalEventsCancelled;
    int totalEventsExpired;
    std::map<EventType, int> eventTypeCount;
    
    // 是否启用调试模式
    bool debugMode;
    
    // 最大队列大小（防止内存溢出）
    size_t maxQueueSize;
    size_t maxPersistentEvents;
    
    // 单例模式
    static EventManager* instance;
    EventManager();
    
public:
    // 单例访问
    static EventManager& getInstance();
    static void destroyInstance();
    
    // 禁用拷贝构造和赋值
    EventManager(const EventManager&) = delete;
    EventManager& operator=(const EventManager&) = delete;
    
    ~EventManager();
    
    // 事件注册与分发
    void registerEvent(std::shared_ptr<Event> event);              // 注册事件到队列
    void registerEvent(Event* event);                              // 注册事件到队列（原始指针版本）
    void queueEvent(std::shared_ptr<Event> event);                 // 队列事件（registerEvent的别名）
    bool registerEventHandler(EventType type, EventHandler handler); // 注册事件处理器
    bool registerGlobalHandler(EventHandler handler);              // 注册全局事件处理器
    
    // 事件处理
    void processEvents(float deltaTime = 1.0f / 60.0f);           // 处理所有待处理事件（包括持续事件更新）
    void processInstantEvents();                                   // 仅处理即时事件
    void updatePersistentEvents(float deltaTime);                  // 更新持续事件
    void processEvent(std::shared_ptr<Event> event);              // 处理单个事件
    void processEventsOfType(EventType type);                     // 处理特定类型的事件
    
    // 事件清理
    void clearEvents();                                            // 清空所有事件
    void clearInstantEvents();                                     // 清空即时事件
    void clearPersistentEvents();                                 // 清空持续事件
    void clearEventsOfType(EventType type);                       // 清空特定类型的事件
    void clearCompletedEvents();                                  // 清空已完成的事件
    void clearCancelledEvents();                                  // 清空已取消的事件
    void clearExpiredEvents();                                    // 清空已过期的事件
    
    // 持续事件管理
    void addPersistentEvent(std::shared_ptr<Event> event);        // 添加持续事件
    void removePersistentEvent(std::shared_ptr<Event> event);     // 移除持续事件
    void pausePersistentEvent(std::shared_ptr<Event> event);      // 暂停持续事件
    void resumePersistentEvent(std::shared_ptr<Event> event);     // 恢复持续事件
    
    // 查询接口
    size_t getInstantEventCount() const;                           // 获取待处理即时事件数量
    size_t getPersistentEventCount() const;                       // 获取持续事件数量
    size_t getTotalEventCount() const;                            // 获取总事件数量
    size_t getEventCountOfType(EventType type) const;             // 获取特定类型的事件数量
    bool hasInstantEvents() const;                                // 是否有待处理即时事件
    bool hasPersistentEvents() const;                             // 是否有持续事件
    bool hasEventsOfType(EventType type) const;                   // 是否有特定类型的事件
    
    // 获取持续事件列表
    std::vector<std::shared_ptr<Event>> getPersistentEventsOfType(EventType type) const;
    std::vector<std::shared_ptr<Event>> getAllPersistentEvents() const;
    
    // 统计信息
    int getTotalEventsProcessed() const { return totalEventsProcessed; }
    int getTotalEventsCancelled() const { return totalEventsCancelled; }
    int getTotalEventsExpired() const { return totalEventsExpired; }
    int getEventTypeCount(EventType type) const;
    void printStatistics() const;                                  // 打印统计信息
    
    // 配置
    void setDebugMode(bool enabled) { debugMode = enabled; }
    bool isDebugMode() const { return debugMode; }
    void setMaxQueueSize(size_t size) { maxQueueSize = size; }
    size_t getMaxQueueSize() const { return maxQueueSize; }
    void setMaxPersistentEvents(size_t size) { maxPersistentEvents = size; }
    size_t getMaxPersistentEvents() const { return maxPersistentEvents; }
    
    // 便捷方法 - 更新以使用新的事件系统
    void triggerExplosion(float x, float y, float radius, float damage, int fragments = 0, 
                         const EventSource& source = EventSource::FromEnvironment(), 
                         const std::string& type = "generic");
    void triggerEntityDamage(Entity* target, float damage, const EventSource& source = EventSource::FromEnvironment());
    void triggerEntityHeal(Entity* target, float healAmount, const EventSource& source = EventSource::FromEnvironment());
    
    // 新的持续事件便捷方法
    void triggerSmokeCloud(float x, float y, float radius, float duration, 
                          const EventSource& source = EventSource::FromEnvironment(), 
                          float intensity = 1.0f, float density = 0.8f);
    void triggerFireArea(float x, float y, float radius, float duration, 
                        const EventSource& source = EventSource::FromEnvironment(), 
                        int damagePerSecond = 5);
    void triggerTeleportGate(float gateX, float gateY, float gateRadius, 
                            float destX, float destY, float duration,
                            const EventSource& source = EventSource::FromEnvironment(), 
                            bool bidirectional = false);
    
    // 调试方法
    void debugPrintEventQueue() const;                             // 打印事件队列内容
    void debugPrintPersistentEvents() const;                      // 打印持续事件内容
    std::string getEventManagerStatus() const;                     // 获取事件管理器状态字符串
};

// 全局便捷函数 - 更新以使用新的事件系统
namespace EventSystem {
    void TriggerExplosion(float x, float y, float radius, float damage, int fragments = 0, 
                         const EventSource& source = EventSource::FromEnvironment(), 
                         const std::string& type = "generic");
    void TriggerSmokeCloud(float x, float y, float radius, float duration, 
                          const EventSource& source = EventSource::FromEnvironment(), 
                          float intensity = 1.0f, float density = 0.8f);
    void TriggerFireArea(float x, float y, float radius, float duration, 
                        const EventSource& source = EventSource::FromEnvironment(), 
                        int damagePerSecond = 5);
    void TriggerTeleportGate(float gateX, float gateY, float gateRadius, 
                            float destX, float destY, float duration,
                            const EventSource& source = EventSource::FromEnvironment(), 
                            bool bidirectional = false);
    void RegisterEventHandler(EventType type, EventHandler handler);
    void RegisterGlobalHandler(EventHandler handler);
    void ProcessEvents(float deltaTime = 1.0f / 60.0f);
    void SetDebugMode(bool enabled);
}

#endif // EVENT_MANAGER_H 