#pragma once
#ifndef ENTITY_STATE_MANAGER_H
#define ENTITY_STATE_MANAGER_H

#include "EntityStateEffect.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>

// 实体状态管理器类，用于管理实体的状态列表
class EntityStateManager {
private:
    std::vector<std::unique_ptr<EntityStateEffect>> activeStates;  // 激活的状态列表
    std::unordered_map<std::string, EntityStateEffect::Type> stateTypeMap;  // 状态名称到类型的映射
    
    // 状态变化回调
    std::function<void(EntityStateEffect*)> onStateAdded;
    std::function<void(EntityStateEffect*)> onStateRemoved;
    std::function<void(EntityStateEffect*)> onStateUpdated;

public:
    // 构造函数
    EntityStateManager();
    
    // 析构函数
    ~EntityStateManager();
    
    // 更新所有状态
    void update(int deltaTimeMs);
    
    // 添加状态
    EntityStateEffect* addState(
        EntityStateEffect::Type type,
        const std::string& name,
        int duration = -1,
        int priority = 0
    );
    
    // 移除状态
    bool removeState(const std::string& name);
    bool removeState(EntityStateEffect::Type type);
    
    // 检查状态是否存在
    bool hasState(const std::string& name) const;
    bool hasState(EntityStateEffect::Type type) const;
    
    // 获取状态
    EntityStateEffect* getState(const std::string& name);
    EntityStateEffect* getState(EntityStateEffect::Type type);
    
    // 获取所有状态
    const std::vector<std::unique_ptr<EntityStateEffect>>& getAllStates() const { return activeStates; }
    
    // 清除所有状态
    void clearStates();
    
    // 设置回调
    void setOnStateAdded(std::function<void(EntityStateEffect*)> callback) { onStateAdded = callback; }
    void setOnStateRemoved(std::function<void(EntityStateEffect*)> callback) { onStateRemoved = callback; }
    void setOnStateUpdated(std::function<void(EntityStateEffect*)> callback) { onStateUpdated = callback; }
    
    // 序列化所有状态（用于网络传输）
    std::string serializeStates() const;
    
    // 反序列化状态（用于网络接收）
    void deserializeStates(const std::string& data);
};

#endif // ENTITY_STATE_MANAGER_H 