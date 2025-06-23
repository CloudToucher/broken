#pragma once
#ifndef PLAYER_STATE_MANAGER_H
#define PLAYER_STATE_MANAGER_H

#include "PlayerState.h"
#include "EntityStateManager.h"
#include <memory>
#include <functional>

// 前向声明
class Player;

// 玩家状态管理器类，用于管理玩家的状态
class PlayerStateManager {
private:
    Player* player;                         // 关联的玩家（不拥有）
    std::unique_ptr<PlayerState> state;     // 玩家状态
    std::unique_ptr<EntityStateManager> stateManager; // 状态效果管理器
    
    // 状态变化回调
    std::function<void(EntityStateEffect*)> onStateAdded;
    std::function<void(EntityStateEffect*)> onStateRemoved;
    std::function<void(EntityStateEffect*)> onStateUpdated;
    
    // 状态同步回调
    std::function<void(const PlayerState&)> onStateChanged;

public:
    // 构造函数
    PlayerStateManager(Player* player);
    
    // 析构函数
    ~PlayerStateManager();
    
    // 更新状态
    void update(float deltaTime);
    
    // 获取状态
    PlayerState* getState() const { return state.get(); }
    
    // 设置状态（用于网络同步）
    void setState(const PlayerState& newState);
    
    // 添加状态效果
    EntityStateEffect* addState(
        EntityStateEffect::Type type,
        const std::string& name,
        int duration = -1,
        int priority = 0
    );
    
    // 移除状态效果
    bool removeState(const std::string& name);
    bool removeState(EntityStateEffect::Type type);
    
    // 检查状态效果是否存在
    bool hasState(const std::string& name) const;
    bool hasState(EntityStateEffect::Type type) const;
    
    // 获取状态效果
    EntityStateEffect* getState(const std::string& name);
    EntityStateEffect* getState(EntityStateEffect::Type type);
    
    // 清除所有状态效果
    void clearStates();
    
    // 设置回调
    void setOnStateAdded(std::function<void(EntityStateEffect*)> callback) { onStateAdded = callback; }
    void setOnStateRemoved(std::function<void(EntityStateEffect*)> callback) { onStateRemoved = callback; }
    void setOnStateUpdated(std::function<void(EntityStateEffect*)> callback) { onStateUpdated = callback; }
    void setOnStateChanged(std::function<void(const PlayerState&)> callback) { onStateChanged = callback; }
    
    // 同步状态到玩家
    void syncStateToPlayer();
    
    // 同步玩家到状态
    void syncPlayerToState();
    
    // 序列化状态（用于网络传输）
    std::string serializeState() const;
    
    // 反序列化状态（用于网络接收）
    void deserializeState(const std::string& data);
};

#endif // PLAYER_STATE_MANAGER_H
