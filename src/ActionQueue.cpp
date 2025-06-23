#include "ActionQueue.h"
#include "Entity.h"

ActionQueue::ActionQueue(Entity* entity)
    : owner(entity), paused(false), elapsedTime(0.0f) {
}

ActionQueue::~ActionQueue() {
    // 安全清理：先中断当前行为，再清空队列
    if (currentAction) {
        currentAction->interrupt();
        currentAction.reset();
    }
    
    // 清空队列中的所有行为
    while (!actions.empty()) {
        actions.pop();
    }
    
    owner = nullptr;  // 清空所有者指针
}

void ActionQueue::addAction(std::unique_ptr<Action> action) {
    if (action && owner) {  // 确保所有者还存在
        actions.push(std::move(action));
    }
}

void ActionQueue::clearActions() {
    // 中断当前行为
    if (currentAction) {
        currentAction->interrupt();
        currentAction.reset();
    }
    
    // 清空队列
    while (!actions.empty()) {
        actions.pop();
    }
}

void ActionQueue::interrupt() {
    clearActions();
}

void ActionQueue::update(float deltaTime) {
    // 检查所有者是否还存在
    if (!owner || paused) {
        return;
    }
    
    // 如果当前没有行为，从队列中取出一个
    if (!currentAction && !actions.empty()) {
        currentAction = std::move(actions.front());
        actions.pop();
        
        // 开始新行为并重置计时器
        if (currentAction) {
            elapsedTime = 0.0f; // 重置计时器
            currentAction->start();
        }
    }
    
    // 累加经过的时间（只有当有活动行为时才累加）
    if (currentAction && currentAction->isActionStarted() && !currentAction->isActionCompleted()) {
        elapsedTime += deltaTime;
    }
    
    // 更新当前行为
    if (currentAction) {
        currentAction->update(deltaTime);
        
        // 如果行为已完成，移除它并准备下一个行为
        if (currentAction->isActionCompleted()) {
            // 重置计时器，确保进度条立即消失
            elapsedTime = 0.0f;
            
            currentAction.reset();
            
            // 如果队列中还有行为，立即开始下一个行为
            if (!actions.empty()) {
                currentAction = std::move(actions.front());
                actions.pop();
                
                if (currentAction) {
                    elapsedTime = 0.0f; // 重置计时器
                    currentAction->start();
                }
            }
        }
    }
}
