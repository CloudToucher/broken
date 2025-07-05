#include "EventManager.h"
#include <iostream>

// 示例事件处理器
void explosionHandler(const Event* event) {
    const ExplosionEvent* explosion = dynamic_cast<const ExplosionEvent*>(event);
    if (explosion) {
        printf("爆炸事件处理器: 在位置(%.1f, %.1f)发生%s爆炸，伤害%d，破片数%d\n",
               explosion->getX(), explosion->getY(), 
               explosion->getExplosionType().c_str(),
               explosion->getTotalDamage(), explosion->getFragmentCount());
    }
}

void globalHandler(const Event* event) {
    printf("全局事件处理器: 处理事件类型 %d\n", static_cast<int>(event->getType()));
}

// 事件系统测试函数
void testEventSystem() {
    printf("=== 事件系统测试开始 ===\n");
    
    // 获取事件管理器实例
    EventManager& eventManager = EventManager::getInstance();
    
    // 启用调试模式
    eventManager.setDebugMode(true);
    
    // 注册事件处理器
    eventManager.registerEventHandler(EventType::EXPLOSION, explosionHandler);
    eventManager.registerGlobalHandler(globalHandler);
    
    printf("\n1. 创建爆炸事件...\n");
    
    // 创建一些测试爆炸事件
    auto explosion1 = std::make_shared<ExplosionEvent>(100.0f, 200.0f, 50.0f, 75.0f, 10, "手榴弹");
    auto explosion2 = std::make_shared<ExplosionEvent>(300.0f, 400.0f, 100.0f, 120.0f, 20, "炸弹");
    auto explosion3 = std::make_shared<ExplosionEvent>(500.0f, 600.0f, 25.0f, 40.0f, 5, "地雷");
    
    // 注册事件
    eventManager.queueEvent(explosion1);
    eventManager.queueEvent(explosion2);
    eventManager.queueEvent(explosion3);
    
    printf("\n2. 当前事件队列状态:\n");
    eventManager.debugPrintEventQueue();
    
    printf("\n3. 测试爆炸事件信息...\n");
    printf("爆炸1信息: %s\n", explosion1->getEventInfo().c_str());
    printf("爆炸2信息: %s\n", explosion2->getEventInfo().c_str());
    printf("爆炸3信息: %s\n", explosion3->getEventInfo().c_str());
    
    printf("\n4. 测试伤害计算...\n");
    printf("爆炸1在距离0处的伤害: %.1f\n", explosion1->calculateTotalDamageAtDistance(0.0f));
    printf("爆炸1在距离25处的伤害: %.1f\n", explosion1->calculateTotalDamageAtDistance(25.0f));
    printf("爆炸1在距离50处的伤害: %.1f\n", explosion1->calculateTotalDamageAtDistance(50.0f));
    
    printf("\n5. 处理所有事件...\n");
    eventManager.processEvents();
    
    printf("\n6. 处理后的统计信息:\n");
    eventManager.printStatistics();
    
    printf("\n7. 测试便捷方法触发爆炸...\n");
    eventManager.triggerExplosion(700.0f, 800.0f, 75.0f, 90.0f, 15, "火箭弹");
    
    printf("\n8. 测试全局便捷函数...\n");
    EventSystem::TriggerExplosion(900.0f, 1000.0f, 60.0f, 80.0f, 12, "迫击炮");
    
    printf("\n9. 处理新事件...\n");
    EventSystem::ProcessEvents();
    
    printf("\n10. 最终统计信息:\n");
    eventManager.printStatistics();
    
    printf("\n=== 事件系统测试完成 ===\n");
}

// 如果需要单独运行测试，可以使用这个main函数
#ifdef EVENT_TEST_STANDALONE
int main() {
    testEventSystem();
    EventManager::destroyInstance();
    return 0;
}
#endif 