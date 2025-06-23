#pragma once
#ifndef ACTION_QUEUE_H
#define ACTION_QUEUE_H

#include <queue>
#include <memory>
#include "Action.h"

class Entity;

// 行为队列类
class ActionQueue {
private:
    Entity* owner;                                // 队列所属的实体
    std::queue<std::unique_ptr<Action>> actions;  // 行为队列
    std::unique_ptr<Action> currentAction;        // 当前执行的行为
    bool paused;                                // 队列是否暂停
    float elapsedTime;                          // 累计经过的时间（秒）

public:
    ActionQueue(Entity* entity);
    ~ActionQueue();  // 添加显式析构函数声明

    // 添加行为到队列
    void addAction(std::unique_ptr<Action> action);
    
    // 清空队列
    void clearActions();
    
    // 暂停/恢复队列
    void pause() { paused = true; }
    void resume() { paused = false; }
    bool isPaused() const { return paused; }
    
    // 中断当前行为并清空队列
    void interrupt();
    
    // 更新队列
    void update(float deltaTime);
    
    // 检查队列是否为空
    bool isEmpty() const { return actions.empty() && !currentAction; }
    
    // 获取当前行为
    Action* getCurrentAction() const { return currentAction.get(); }
    
    // 获取累计经过的时间
    float getElapsedTime() const { return elapsedTime; }
};

#endif // ACTION_QUEUE_H