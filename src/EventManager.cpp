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
    : totalEventsProcessed(0), totalEventsCancelled(0), totalEventsExpired(0), 
      debugMode(false), maxQueueSize(1000), maxPersistentEvents(100) {
    printf("事件管理器已初始化（支持持续事件）\n");
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
    
    // 验证事件
    if (!event->validate()) {
        if (debugMode) {
            printf("警告: 事件验证失败: %s\n", event->getEventInfo().c_str());
        }
        return;
    }
    
    // 根据事件类型选择队列
    if (event->getIsPersistent()) {
        // 持续事件
        if (persistentEvents.size() >= maxPersistentEvents) {
            if (debugMode) {
                printf("警告: 持续事件队列已满，丢弃事件: %s\n", event->getEventInfo().c_str());
            }
            return;
        }
        
        addPersistentEvent(event);
    } else {
        // 即时事件
        if (instantEventQueue.size() >= maxQueueSize) {
            if (debugMode) {
                printf("警告: 即时事件队列已满，丢弃事件: %s\n", event->getEventInfo().c_str());
            }
            return;
        }
        
        instantEventQueue.push(event);
    }
    
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

void EventManager::processEvents(float deltaTime) {
    // 处理即时事件
    processInstantEvents();
    
    // 更新持续事件
    updatePersistentEvents(deltaTime);
    
    // 清理已完成的事件
    clearCompletedEvents();
}

void EventManager::processInstantEvents() {
    while (!instantEventQueue.empty()) {
        auto event = instantEventQueue.top();
        instantEventQueue.pop();
        
        processEvent(event);
    }
}

void EventManager::updatePersistentEvents(float deltaTime) {
    auto it = persistentEvents.begin();
    while (it != persistentEvents.end()) {
        auto event = *it;
        
        if (!event) {
            it = persistentEvents.erase(it);
            continue;
        }
        
        // 检查事件是否已取消
        if (event->isCancelled()) {
            totalEventsCancelled++;
            if (debugMode) {
                printf("持续事件已取消: %s\n", event->getEventInfo().c_str());
            }
            it = persistentEvents.erase(it);
            continue;
        }
        
        // 如果事件还未激活，先激活它
        if (event->isPending()) {
            processEvent(event);
            
            // 如果事件在execute后变为完成状态（可能是验证失败等），直接移除
            if (event->isCompleted()) {
                it = persistentEvents.erase(it);
                continue;
            }
        }
        
        // 更新持续事件
        if (event->isActive()) {
            try {
                event->update(deltaTime);
            } catch (const std::exception& e) {
                printf("持续事件更新异常: %s\n", e.what());
                event->cancel();
            }
            
            // 检查是否已过期或完成
            if (event->isExpired()) {
                totalEventsExpired++;
                if (debugMode) {
                    printf("持续事件已过期: %s\n", event->getEventInfo().c_str());
                }
                event->finish();
            }
            
            if (event->isCompleted()) {
                it = persistentEvents.erase(it);
                continue;
            }
        }
        
        ++it;
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
    
    // 检查事件是否已完成
    if (event->isCompleted()) {
        if (debugMode) {
            printf("事件已完成: %s\n", event->getEventInfo().c_str());
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
    // 处理即时事件
    std::vector<std::shared_ptr<Event>> eventsToProcess;
    std::vector<std::shared_ptr<Event>> otherEvents;
    
    // 分离指定类型的即时事件
    while (!instantEventQueue.empty()) {
        auto event = instantEventQueue.top();
        instantEventQueue.pop();
        
        if (event->getType() == type) {
            eventsToProcess.push_back(event);
        } else {
            otherEvents.push_back(event);
        }
    }
    
    // 将其他事件放回队列
    for (auto& event : otherEvents) {
        instantEventQueue.push(event);
    }
    
    // 处理指定类型的即时事件
    for (auto& event : eventsToProcess) {
        processEvent(event);
    }
    
    // 处理持续事件（这里只是触发执行，不更新）
    for (auto& event : persistentEvents) {
        if (event && event->getType() == type && event->isPending()) {
            processEvent(event);
        }
    }
}

// =============================================================================
// 事件清理
// =============================================================================

void EventManager::clearEvents() {
    clearInstantEvents();
    clearPersistentEvents();
    
    if (debugMode) {
        printf("所有事件已清空\n");
    }
}

void EventManager::clearInstantEvents() {
    while (!instantEventQueue.empty()) {
        instantEventQueue.pop();
    }
    
    if (debugMode) {
        printf("即时事件队列已清空\n");
    }
}

void EventManager::clearPersistentEvents() {
    persistentEvents.clear();
    
    if (debugMode) {
        printf("持续事件列表已清空\n");
    }
}

void EventManager::clearEventsOfType(EventType type) {
    // 清理即时事件
    std::vector<std::shared_ptr<Event>> remainingEvents;
    
    while (!instantEventQueue.empty()) {
        auto event = instantEventQueue.top();
        instantEventQueue.pop();
        
        if (event->getType() != type) {
            remainingEvents.push_back(event);
        }
    }
    
    // 将剩余事件放回队列
    for (auto& event : remainingEvents) {
        instantEventQueue.push(event);
    }
    
    // 清理持续事件
    auto it = persistentEvents.begin();
    while (it != persistentEvents.end()) {
        if ((*it)->getType() == type) {
            it = persistentEvents.erase(it);
        } else {
            ++it;
        }
    }
    
    if (debugMode) {
        printf("类型 %d 的事件已清空\n", static_cast<int>(type));
    }
}

void EventManager::clearCompletedEvents() {
    auto it = persistentEvents.begin();
    while (it != persistentEvents.end()) {
        if ((*it)->isCompleted()) {
            it = persistentEvents.erase(it);
        } else {
            ++it;
        }
    }
}

void EventManager::clearCancelledEvents() {
    auto it = persistentEvents.begin();
    while (it != persistentEvents.end()) {
        if ((*it)->isCancelled()) {
            it = persistentEvents.erase(it);
        } else {
            ++it;
        }
    }
}

void EventManager::clearExpiredEvents() {
    auto it = persistentEvents.begin();
    while (it != persistentEvents.end()) {
        if ((*it)->isExpired()) {
            it = persistentEvents.erase(it);
        } else {
            ++it;
        }
    }
}

// =============================================================================
// 持续事件管理
// =============================================================================

void EventManager::addPersistentEvent(std::shared_ptr<Event> event) {
    if (!event || !event->getIsPersistent()) return;
    
    persistentEvents.push_back(event);
    
    if (debugMode) {
        printf("持续事件已添加: %s\n", event->getEventInfo().c_str());
    }
}

void EventManager::removePersistentEvent(std::shared_ptr<Event> event) {
    if (!event) return;
    
    auto it = std::find(persistentEvents.begin(), persistentEvents.end(), event);
    if (it != persistentEvents.end()) {
        persistentEvents.erase(it);
        
        if (debugMode) {
            printf("持续事件已移除: %s\n", event->getEventInfo().c_str());
        }
    }
}

void EventManager::pausePersistentEvent(std::shared_ptr<Event> event) {
    // TODO: 实现事件暂停功能
    // 可以添加EventStatus::PAUSED状态
}

void EventManager::resumePersistentEvent(std::shared_ptr<Event> event) {
    // TODO: 实现事件恢复功能
}

// =============================================================================
// 查询接口
// =============================================================================

size_t EventManager::getInstantEventCount() const {
    return instantEventQueue.size();
}

size_t EventManager::getPersistentEventCount() const {
    return persistentEvents.size();
}

size_t EventManager::getTotalEventCount() const {
    return getInstantEventCount() + getPersistentEventCount();
}

size_t EventManager::getEventCountOfType(EventType type) const {
    size_t count = 0;
    
    // 计算即时事件中的数量
    std::priority_queue<std::shared_ptr<Event>, std::vector<std::shared_ptr<Event>>, EventComparator> tempQueue = instantEventQueue;
    while (!tempQueue.empty()) {
        if (tempQueue.top()->getType() == type) {
            count++;
        }
        tempQueue.pop();
    }
    
    // 计算持续事件中的数量
    for (const auto& event : persistentEvents) {
        if (event && event->getType() == type) {
            count++;
        }
    }
    
    return count;
}

bool EventManager::hasInstantEvents() const {
    return !instantEventQueue.empty();
}

bool EventManager::hasPersistentEvents() const {
    return !persistentEvents.empty();
}

bool EventManager::hasEventsOfType(EventType type) const {
    return getEventCountOfType(type) > 0;
}

std::vector<std::shared_ptr<Event>> EventManager::getPersistentEventsOfType(EventType type) const {
    std::vector<std::shared_ptr<Event>> result;
    
    for (const auto& event : persistentEvents) {
        if (event && event->getType() == type) {
            result.push_back(event);
        }
    }
    
    return result;
}

std::vector<std::shared_ptr<Event>> EventManager::getAllPersistentEvents() const {
    return std::vector<std::shared_ptr<Event>>(persistentEvents.begin(), persistentEvents.end());
}

int EventManager::getEventTypeCount(EventType type) const {
    auto it = eventTypeCount.find(type);
    return (it != eventTypeCount.end()) ? it->second : 0;
}

void EventManager::printStatistics() const {
    printf("=== 事件管理器统计信息 ===\n");
    printf("总处理事件数: %d\n", totalEventsProcessed);
    printf("总取消事件数: %d\n", totalEventsCancelled);
    printf("总过期事件数: %d\n", totalEventsExpired);
    printf("待处理即时事件数: %zu\n", getInstantEventCount());
    printf("活跃持续事件数: %zu\n", getPersistentEventCount());
    printf("最大队列大小: %zu\n", maxQueueSize);
    printf("最大持续事件数: %zu\n", maxPersistentEvents);
    printf("调试模式: %s\n", debugMode ? "开启" : "关闭");
    
    printf("各类型事件处理统计:\n");
    for (const auto& pair : eventTypeCount) {
        printf("  类型 %d: %d 次\n", static_cast<int>(pair.first), pair.second);
    }
    printf("========================\n");
}

// =============================================================================
// 便捷方法
// =============================================================================

void EventManager::triggerExplosion(float x, float y, float radius, float damage, int fragments, 
                                   const EventSource& source, const std::string& type) {
    auto explosion = std::make_shared<ExplosionEvent>(x, y, radius, damage, fragments, source, type);
    registerEvent(explosion);
}

void EventManager::triggerEntityDamage(Entity* target, float damage, const EventSource& source) {
    // TODO: 实现EntityDamageEvent
    if (debugMode) {
        printf("实体伤害事件: 目标=%p, 伤害=%.1f, 来源=%s\n", 
               target, damage, source.description.c_str());
    }
}

void EventManager::triggerEntityHeal(Entity* target, float healAmount, const EventSource& source) {
    // TODO: 实现EntityHealEvent
    if (debugMode) {
        printf("实体治疗事件: 目标=%p, 治疗量=%.1f, 来源=%s\n", 
               target, healAmount, source.description.c_str());
    }
}

void EventManager::triggerSmokeCloud(float x, float y, float radius, float duration, 
                                    const EventSource& source, float intensity, float density) {
    auto smokeCloud = std::make_shared<SmokeCloudEvent>(x, y, radius, duration, source, intensity, density);
    registerEvent(smokeCloud);
}

void EventManager::triggerFireArea(float x, float y, float radius, float duration, 
                                  const EventSource& source, int damagePerSecond) {
    auto fireArea = std::make_shared<FireAreaEvent>(x, y, radius, duration, source, damagePerSecond);
    registerEvent(fireArea);
}

void EventManager::triggerTeleportGate(float gateX, float gateY, float gateRadius, 
                                      float destX, float destY, float duration,
                                      const EventSource& source, bool bidirectional) {
    auto teleportGate = std::make_shared<TeleportGateEvent>(gateX, gateY, gateRadius, 
                                                           destX, destY, duration, source, bidirectional);
    registerEvent(teleportGate);
}

// =============================================================================
// 调试方法
// =============================================================================

void EventManager::debugPrintEventQueue() const {
    printf("=== 即时事件队列 ===\n");
    printf("队列大小: %zu\n", instantEventQueue.size());
    
    std::priority_queue<std::shared_ptr<Event>, std::vector<std::shared_ptr<Event>>, EventComparator> tempQueue = instantEventQueue;
    int index = 0;
    while (!tempQueue.empty() && index < 10) { // 只显示前10个
        auto event = tempQueue.top();
        tempQueue.pop();
        printf("  [%d] %s\n", index++, event->getEventInfo().c_str());
    }
    
    if (tempQueue.size() > 0) {
        printf("  ... 还有 %zu 个事件\n", tempQueue.size());
    }
    printf("==================\n");
}

void EventManager::debugPrintPersistentEvents() const {
    printf("=== 持续事件列表 ===\n");
    printf("列表大小: %zu\n", persistentEvents.size());
    
    int index = 0;
    for (const auto& event : persistentEvents) {
        if (event) {
            printf("  [%d] %s\n", index++, event->getEventInfo().c_str());
        }
        if (index >= 10) break; // 只显示前10个
    }
    
    if (persistentEvents.size() > 10) {
        printf("  ... 还有 %zu 个事件\n", persistentEvents.size() - 10);
    }
    printf("==================\n");
}

std::string EventManager::getEventManagerStatus() const {
    std::stringstream ss;
    ss << "EventManager[InstantEvents=" << getInstantEventCount()
       << ", PersistentEvents=" << getPersistentEventCount()
       << ", Processed=" << totalEventsProcessed
       << ", Cancelled=" << totalEventsCancelled
       << ", Expired=" << totalEventsExpired
       << ", Debug=" << (debugMode ? "On" : "Off") << "]";
    return ss.str();
}

// =============================================================================
// 全局便捷函数
// =============================================================================

namespace EventSystem {
    void TriggerExplosion(float x, float y, float radius, float damage, int fragments, 
                         const EventSource& source, const std::string& type) {
        EventManager::getInstance().triggerExplosion(x, y, radius, damage, fragments, source, type);
    }
    
    void TriggerSmokeCloud(float x, float y, float radius, float duration, 
                          const EventSource& source, float intensity, float density) {
        EventManager::getInstance().triggerSmokeCloud(x, y, radius, duration, source, intensity, density);
    }
    
    void TriggerFireArea(float x, float y, float radius, float duration, 
                        const EventSource& source, int damagePerSecond) {
        EventManager::getInstance().triggerFireArea(x, y, radius, duration, source, damagePerSecond);
    }
    
    void TriggerTeleportGate(float gateX, float gateY, float gateRadius, 
                            float destX, float destY, float duration,
                            const EventSource& source, bool bidirectional) {
        EventManager::getInstance().triggerTeleportGate(gateX, gateY, gateRadius, 
                                                       destX, destY, duration, source, bidirectional);
    }

    void RegisterEventHandler(EventType type, EventHandler handler) {
        EventManager::getInstance().registerEventHandler(type, handler);
    }

    void RegisterGlobalHandler(EventHandler handler) {
        EventManager::getInstance().registerGlobalHandler(handler);
    }

    void ProcessEvents(float deltaTime) {
        EventManager::getInstance().processEvents(deltaTime);
    }

    void SetDebugMode(bool enabled) {
        EventManager::getInstance().setDebugMode(enabled);
    }
} 