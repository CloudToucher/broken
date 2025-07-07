#include "EventManager.h"
#include <iostream>

// 示例事件处理器
void explosionHandler(const Event* event) {
    const ExplosionEvent* explosion = dynamic_cast<const ExplosionEvent*>(event);
    if (explosion) {
        printf("爆炸事件处理器: 在位置(%.1f, %.1f)发生%s爆炸，伤害%d，破片数%d，来源：%s\n",
               explosion->getX(), explosion->getY(), 
               explosion->getExplosionType().c_str(),
               explosion->getTotalDamage(), explosion->getFragmentCount(),
               explosion->getSource().description.c_str());
    }
}

void smokeHandler(const Event* event) {
    const SmokeCloudEvent* smoke = dynamic_cast<const SmokeCloudEvent*>(event);
    if (smoke) {
        printf("烟雾事件处理器: 在位置(%.1f, %.1f)生成烟雾云，密度%.2f，持续%.1fs\n",
               smoke->getX(), smoke->getY(), smoke->getDensity(), smoke->getDuration());
    }
}

void fireHandler(const Event* event) {
    const FireAreaEvent* fire = dynamic_cast<const FireAreaEvent*>(event);
    if (fire) {
        printf("燃烧事件处理器: 在位置(%.1f, %.1f)生成燃烧区域，DPS=%d，持续%.1fs\n",
               fire->getX(), fire->getY(), fire->getDamagePerSecond(), fire->getDuration());
    }
}

void teleportHandler(const Event* event) {
    const TeleportGateEvent* teleport = dynamic_cast<const TeleportGateEvent*>(event);
    if (teleport) {
        printf("传送门事件处理器: 传送门位置(%.1f, %.1f) -> 目标(%.1f, %.1f)，持续%.1fs\n",
               teleport->getX(), teleport->getY(), 
               teleport->getTargetX(), teleport->getTargetY(), teleport->getDuration());
    }
}

void globalHandler(const Event* event) {
    printf("全局事件处理器: 处理事件类型 %d，状态 %d，来源类型 %d\n", 
           static_cast<int>(event->getType()), 
           static_cast<int>(event->getStatus()),
           static_cast<int>(event->getSource().type));
}

// 事件系统测试函数
void testEventSystem() {
    printf("=== 改进的事件系统测试开始 ===\n");
    
    // 获取事件管理器实例
    EventManager& eventManager = EventManager::getInstance();
    
    // 启用调试模式
    eventManager.setDebugMode(true);
    
    // 注册事件处理器
    eventManager.registerEventHandler(EventType::EXPLOSION, explosionHandler);
    eventManager.registerEventHandler(EventType::SMOKE_CLOUD, smokeHandler);
    eventManager.registerEventHandler(EventType::FIRE_AREA, fireHandler);
    eventManager.registerEventHandler(EventType::TELEPORT_GATE, teleportHandler);
    eventManager.registerGlobalHandler(globalHandler);
    
    printf("\n=== 测试1: 即时事件（爆炸） ===\n");
    
    // 创建不同来源的爆炸事件
    EventSource envSource = EventSource::FromEnvironment("地雷爆炸");
    EventSource sysSource = EventSource::FromSystem("测试系统");
    
    auto explosion1 = std::make_shared<ExplosionEvent>(100.0f, 200.0f, 3.0f, 75.0f, 10, envSource, "手榴弹");
    auto explosion2 = std::make_shared<ExplosionEvent>(300.0f, 400.0f, 5.0f, 120.0f, 20, sysSource, "炸弹");
    
    // 注册即时事件
    eventManager.queueEvent(explosion1);
    eventManager.queueEvent(explosion2);
    
    printf("\n当前即时事件队列状态:\n");
    eventManager.debugPrintEventQueue();
    
    printf("\n=== 测试2: 持续事件 ===\n");
    
    // 创建持续事件
    EventSource playerSource = EventSource::FromEntity(nullptr, "玩家测试"); // 这里用nullptr模拟玩家
    
    auto smokeCloud = std::make_shared<SmokeCloudEvent>(500.0f, 600.0f, 100.0f, 10.0f, playerSource, 0.8f);
    auto fireArea = std::make_shared<FireAreaEvent>(700.0f, 800.0f, 150.0f, 15.0f, envSource, 5);
    auto teleportGate = std::make_shared<TeleportGateEvent>(900.0f, 1000.0f, 50.0f, 100.0f, 200.0f, 20.0f, sysSource);
    
    // 注册持续事件
    eventManager.queueEvent(smokeCloud);
    eventManager.queueEvent(fireArea);
    eventManager.queueEvent(teleportGate);
    
    printf("\n当前持续事件列表状态:\n");
    eventManager.debugPrintPersistentEvents();
    
    printf("\n=== 测试3: 事件信息和伤害计算 ===\n");
    printf("爆炸1信息: %s\n", explosion1->getEventInfo().c_str());
    printf("烟雾云信息: %s\n", smokeCloud->getEventInfo().c_str());
    printf("燃烧区域信息: %s\n", fireArea->getEventInfo().c_str());
    
    printf("\n爆炸1在不同距离的伤害:\n");
    printf("  距离0处: %.1f\n", explosion1->calculateTotalDamageAtDistance(0.0f));
    printf("  距离50处: %.1f\n", explosion1->calculateTotalDamageAtDistance(50.0f));
    printf("  距离100处: %.1f\n", explosion1->calculateTotalDamageAtDistance(100.0f));
    printf("  距离200处: %.1f\n", explosion1->calculateTotalDamageAtDistance(200.0f));
    
    printf("\n=== 测试4: 处理事件（5秒模拟） ===\n");
    float deltaTime = 1.0f; // 1秒步长
    for (int i = 0; i < 5; i++) {
        printf("\n--- 第%d秒 ---\n", i + 1);
        eventManager.processEvents(deltaTime);
        
        printf("状态统计:\n");
        printf("  即时事件数: %zu\n", eventManager.getInstantEventCount());
        printf("  持续事件数: %zu\n", eventManager.getPersistentEventCount());
        printf("  已处理事件: %d\n", eventManager.getTotalEventsProcessed());
        printf("  已过期事件: %d\n", eventManager.getTotalEventsExpired());
    }
    
    printf("\n=== 测试5: 便捷方法 ===\n");
    
    // 测试便捷方法
    eventManager.triggerExplosion(1100.0f, 1200.0f, 4.0f, 90.0f, 15, envSource, "火箭弹");
    eventManager.triggerSmokeCloud(1300.0f, 1400.0f, 120.0f, 8.0f, playerSource, 0.6f);
    eventManager.triggerFireArea(1500.0f, 1600.0f, 80.0f, 12.0f, sysSource, 8);
    eventManager.triggerTeleportGate(1700.0f, 1800.0f, 60.0f, 200.0f, 300.0f, 25.0f, envSource, true);
    
    printf("\n=== 测试6: 全局便捷函数 ===\n");
    
    EventSystem::TriggerExplosion(1900.0f, 2000.0f, 6.0f, 100.0f, 25, playerSource, "迫击炮");
    EventSystem::TriggerSmokeCloud(2100.0f, 2200.0f, 200.0f, 20.0f, envSource, 0.9f);
    EventSystem::TriggerFireArea(2300.0f, 2400.0f, 100.0f, 30.0f, sysSource, 10);
    
    printf("\n=== 测试7: 处理新事件（10秒模拟） ===\n");
    for (int i = 0; i < 10; i++) {
        printf("\n--- 第%d秒 ---\n", i + 1);
        EventSystem::ProcessEvents(deltaTime);
        
        if (i == 4) {
            printf("  在第5秒添加额外烟雾云...\n");
            EventSystem::TriggerSmokeCloud(2500.0f, 2600.0f, 150.0f, 5.0f, playerSource, 0.7f);
        }
    }
    
    printf("\n=== 测试8: 查询功能 ===\n");
    
    printf("特定类型事件数量:\n");
    printf("  爆炸事件: %zu\n", eventManager.getEventCountOfType(EventType::EXPLOSION));
    printf("  烟雾事件: %zu\n", eventManager.getEventCountOfType(EventType::SMOKE_CLOUD));
    printf("  燃烧事件: %zu\n", eventManager.getEventCountOfType(EventType::FIRE_AREA));
    printf("  传送门事件: %zu\n", eventManager.getEventCountOfType(EventType::TELEPORT_GATE));
    
    auto smokeEvents = eventManager.getPersistentEventsOfType(EventType::SMOKE_CLOUD);
    printf("\n当前活跃烟雾云数量: %zu\n", smokeEvents.size());
    for (size_t i = 0; i < smokeEvents.size(); i++) {
        auto smoke = std::dynamic_pointer_cast<SmokeCloudEvent>(smokeEvents[i]);
        if (smoke) {
            printf("  烟雾云%zu: 密度%.2f, 剩余时间%.1fs\n", 
                   i + 1, smoke->getDensity(), smoke->getRemainingTime());
        }
    }
    
    printf("\n=== 测试9: 最终统计 ===\n");
    eventManager.printStatistics();
    
    printf("\n事件管理器状态: %s\n", eventManager.getEventManagerStatus().c_str());
    
    printf("\n=== 改进的事件系统测试完成 ===\n");
}

// 如果需要单独运行测试，可以使用这个main函数
#ifdef EVENT_TEST_STANDALONE
int main() {
    testEventSystem();
    EventManager::destroyInstance();
    return 0;
}
#endif 