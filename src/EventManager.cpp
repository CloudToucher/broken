#include "EventManager.h"
#include <iostream>
#include <algorithm>
#include <sstream>

// 静态成员初始化
EventManager* EventManager::instance = nullptr;

// =============================================================================
// EventManager 构造和析构
// =============================================================================

EventManager::EventManager() 
    : totalEventsProcessed(0), totalEventsCancelled(0), debugMode(false), maxQueueSize(1000) {
    printf("事件管理器已初始化\n");
}

EventManager::~EventManager() {
    clearEvents();
    printf("事件管理器已销毁\n");
}

EventManager& EventManager::getInstance() {
    if (!instance) {
        instance = new EventManager();
    }
    return *instance;
}

void EventManager::destroyInstance() {
    if (instance) {
        delete instance;
        instance = nullptr;
    }
}

// =============================================================================
// 事件注册与分发
// =============================================================================

void EventManager::registerEvent(std::shared_ptr<Event> event) {
    if (!event) {
        if (debugMode) {
            printf("警告: 尝试注册空事件\n");
        }
        return;
    }
    
    // 检查队列大小限制
    if (eventQueue.size() >= maxQueueSize) {
        if (debugMode) {
            printf("警告: 事件队列已满，丢弃事件: %s\n", event->getEventInfo().c_str());
        }
        return;
    }
    
    // 验证事件
    if (!event->validate()) {
        if (debugMode) {
            printf("警告: 事件验证失败: %s\n", event->getEventInfo().c_str());
        }
        return;
    }
    
    eventQueue.push(event);
    
    if (debugMode) {
        printf("事件已注册: %s\n", event->getEventInfo().c_str());
    }
}

void EventManager::registerEvent(Event* event) {
    if (!event) {
        if (debugMode) {
            printf("警告: 尝试注册空事件指针\n");
        }
        return;
    }
    
    registerEvent(std::shared_ptr<Event>(event));
}

void EventManager::queueEvent(std::shared_ptr<Event> event) {
    registerEvent(event);
}

bool EventManager::registerEventHandler(EventType type, EventHandler handler) {
    if (!handler) {
        if (debugMode) {
            printf("警告: 尝试注册空事件处理器\n");
        }
        return false;
    }
    
    eventHandlers[type].push_back(handler);
    
    if (debugMode) {
        printf("事件处理器已注册，类型: %d\n", static_cast<int>(type));
    }
    
    return true;
}

bool EventManager::registerGlobalHandler(EventHandler handler) {
    if (!handler) {
        if (debugMode) {
            printf("警告: 尝试注册空全局事件处理器\n");
        }
        return false;
    }
    
    globalHandlers.push_back(handler);
    
    if (debugMode) {
        printf("全局事件处理器已注册\n");
    }
    
    return true;
}

// =============================================================================
// 事件处理
// =============================================================================

void EventManager::processEvents() {
    while (!eventQueue.empty()) {
        auto event = eventQueue.top();
        eventQueue.pop();
        
        processEvent(event);
    }
}

void EventManager::processEvent(std::shared_ptr<Event> event) {
    if (!event) {
        if (debugMode) {
            printf("警告: 尝试处理空事件\n");
        }
        return;
    }
    
    // 检查事件是否已取消
    if (event->isCancelled()) {
        totalEventsCancelled++;
        if (debugMode) {
            printf("事件已取消: %s\n", event->getEventInfo().c_str());
        }
        return;
    }
    
    // 检查事件是否已处理
    if (event->isProcessed()) {
        if (debugMode) {
            printf("事件已处理: %s\n", event->getEventInfo().c_str());
        }
        return;
    }
    
    if (debugMode) {
        printf("处理事件: %s\n", event->getEventInfo().c_str());
    }
    
    // 调用全局处理器
    for (auto& handler : globalHandlers) {
        try {
            handler(event.get());
        } catch (const std::exception& e) {
            printf("全局事件处理器异常: %s\n", e.what());
        }
    }
    
    // 调用特定类型的处理器
    auto it = eventHandlers.find(event->getType());
    if (it != eventHandlers.end()) {
        for (auto& handler : it->second) {
            try {
                handler(event.get());
            } catch (const std::exception& e) {
                printf("事件处理器异常: %s\n", e.what());
            }
        }
    }
    
    // 执行事件本身的逻辑
    try {
        event->execute();
    } catch (const std::exception& e) {
        printf("事件执行异常: %s\n", e.what());
    }
    
    // 更新统计信息
    totalEventsProcessed++;
    eventTypeCount[event->getType()]++;
    
    if (debugMode) {
        printf("事件处理完成: %s\n", event->getEventInfo().c_str());
    }
}

void EventManager::processEventsOfType(EventType type) {
    std::vector<std::shared_ptr<Event>> eventsToProcess;
    std::vector<std::shared_ptr<Event>> otherEvents;
    
    // 分离指定类型的事件
    while (!eventQueue.empty()) {
        auto event = eventQueue.top();
        eventQueue.pop();
        
        if (event->getType() == type) {
            eventsToProcess.push_back(event);
        } else {
            otherEvents.push_back(event);
        }
    }
    
    // 将其他事件放回队列
    for (auto& event : otherEvents) {
        eventQueue.push(event);
    }
    
    // 处理指定类型的事件
    for (auto& event : eventsToProcess) {
        processEvent(event);
    }
}

// =============================================================================
// 事件清理
// =============================================================================

void EventManager::clearEvents() {
    while (!eventQueue.empty()) {
        eventQueue.pop();
    }
    
    if (debugMode) {
        printf("所有事件已清空\n");
    }
}

void EventManager::clearEventsOfType(EventType type) {
    std::vector<std::shared_ptr<Event>> remainingEvents;
    
    while (!eventQueue.empty()) {
        auto event = eventQueue.top();
        eventQueue.pop();
        
        if (event->getType() != type) {
            remainingEvents.push_back(event);
        }
    }
    
    // 将剩余事件放回队列
    for (auto& event : remainingEvents) {
        eventQueue.push(event);
    }
    
    if (debugMode) {
        printf("类型 %d 的事件已清空\n", static_cast<int>(type));
    }
}

void EventManager::clearCancelledEvents() {
    std::vector<std::shared_ptr<Event>> validEvents;
    
    while (!eventQueue.empty()) {
        auto event = eventQueue.top();
        eventQueue.pop();
        
        if (!event->isCancelled()) {
            validEvents.push_back(event);
        }
    }
    
    // 将有效事件放回队列
    for (auto& event : validEvents) {
        eventQueue.push(event);
    }
    
    if (debugMode) {
        printf("已取消的事件已清空\n");
    }
}

// =============================================================================
// 查询接口
// =============================================================================

size_t EventManager::getEventCount() const {
    return eventQueue.size();
}

size_t EventManager::getEventCountOfType(EventType type) const {
    // 这个方法效率不高，因为需要遍历整个队列
    // 在实际应用中，可以考虑维护一个类型计数器
    size_t count = 0;
    auto tempQueue = eventQueue;
    
    while (!tempQueue.empty()) {
        auto event = tempQueue.top();
        tempQueue.pop();
        
        if (event->getType() == type) {
            count++;
        }
    }
    
    return count;
}

bool EventManager::hasEvents() const {
    return !eventQueue.empty();
}

bool EventManager::hasEventsOfType(EventType type) const {
    return getEventCountOfType(type) > 0;
}

// =============================================================================
// 统计信息
// =============================================================================

int EventManager::getEventTypeCount(EventType type) const {
    auto it = eventTypeCount.find(type);
    return (it != eventTypeCount.end()) ? it->second : 0;
}

void EventManager::printStatistics() const {
    printf("\n=== 事件管理器统计信息 ===\n");
    printf("总处理事件数: %d\n", totalEventsProcessed);
    printf("总取消事件数: %d\n", totalEventsCancelled);
    printf("当前队列大小: %zu\n", eventQueue.size());
    printf("最大队列大小: %zu\n", maxQueueSize);
    printf("调试模式: %s\n", debugMode ? "开启" : "关闭");
    
    if (!eventTypeCount.empty()) {
        printf("\n按类型统计:\n");
        for (const auto& pair : eventTypeCount) {
            printf("  类型 %d: %d 个事件\n", static_cast<int>(pair.first), pair.second);
        }
    }
    
    printf("========================\n\n");
}

// =============================================================================
// 便捷方法
// =============================================================================

void EventManager::triggerExplosion(float x, float y, float radius, float damage, int fragments, const std::string& type) {
    auto explosion = std::make_shared<ExplosionEvent>(x, y, radius, damage, fragments, type);
    registerEvent(explosion);
}

void EventManager::triggerEntityDamage(Entity* target, float damage, Entity* source) {
    // TODO: 实现EntityDamageEvent类
    if (debugMode) {
        printf("触发实体伤害事件: 目标=%p, 伤害=%.1f, 来源=%p\n", target, damage, source);
    }
}

void EventManager::triggerEntityHeal(Entity* target, float healAmount, Entity* source) {
    // TODO: 实现EntityHealEvent类
    if (debugMode) {
        printf("触发实体治疗事件: 目标=%p, 治疗=%.1f, 来源=%p\n", target, healAmount, source);
    }
}

// =============================================================================
// 调试方法
// =============================================================================

void EventManager::debugPrintEventQueue() const {
    printf("\n=== 事件队列内容 ===\n");
    printf("队列大小: %zu\n", eventQueue.size());
    
    if (eventQueue.empty()) {
        printf("队列为空\n");
    } else {
        auto tempQueue = eventQueue;
        int index = 0;
        
        while (!tempQueue.empty() && index < 10) { // 最多显示10个事件
            auto event = tempQueue.top();
            tempQueue.pop();
            
            printf("  [%d] %s\n", index, event->getEventInfo().c_str());
            index++;
        }
        
        if (tempQueue.size() > 0) {
            printf("  ... 还有 %zu 个事件\n", tempQueue.size());
        }
    }
    
    printf("==================\n\n");
}

std::string EventManager::getEventManagerStatus() const {
    std::stringstream ss;
    ss << "EventManager[Events=" << eventQueue.size() 
       << ", Processed=" << totalEventsProcessed
       << ", Cancelled=" << totalEventsCancelled
       << ", Debug=" << (debugMode ? "ON" : "OFF")
       << ", MaxQueue=" << maxQueueSize << "]";
    return ss.str();
}

// =============================================================================
// 全局便捷函数
// =============================================================================

namespace EventSystem {
    void TriggerExplosion(float x, float y, float radius, float damage, int fragments, const std::string& type) {
        EventManager::getInstance().triggerExplosion(x, y, radius, damage, fragments, type);
    }
    
    void RegisterEventHandler(EventType type, EventHandler handler) {
        EventManager::getInstance().registerEventHandler(type, handler);
    }
    
    void RegisterGlobalHandler(EventHandler handler) {
        EventManager::getInstance().registerGlobalHandler(handler);
    }
    
    void ProcessEvents() {
        EventManager::getInstance().processEvents();
    }
    
    void SetDebugMode(bool enabled) {
        EventManager::getInstance().setDebugMode(enabled);
    }
} 