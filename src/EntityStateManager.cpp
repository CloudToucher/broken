#include "EntityStateManager.h"
#include <algorithm>
#include <sstream>

EntityStateManager::EntityStateManager() {
    // 初始化状态类型映射
    stateTypeMap["moving"] = EntityStateEffect::Type::MOVING;
    stateTypeMap["shooting"] = EntityStateEffect::Type::SHOOTING;
    stateTypeMap["reloading"] = EntityStateEffect::Type::RELOADING;
    stateTypeMap["interacting"] = EntityStateEffect::Type::INTERACTING;
    stateTypeMap["stunned"] = EntityStateEffect::Type::STUNNED;
    stateTypeMap["healing"] = EntityStateEffect::Type::HEALING;
    stateTypeMap["buffed"] = EntityStateEffect::Type::BUFFED;
    stateTypeMap["debuffed"] = EntityStateEffect::Type::DEBUFFED;
    stateTypeMap["attacking"] = EntityStateEffect::Type::ATTACKING;
}

EntityStateManager::~EntityStateManager() {
    clearStates();
}

void EntityStateManager::update(int deltaTimeMs) {
    // 使用迭代器遍历，以便在循环中安全移除元素
    auto it = activeStates.begin();
    while (it != activeStates.end()) {
        EntityStateEffect* state = it->get();
        
        // 更新状态，如果返回false表示状态已结束
        if (!state->update(deltaTimeMs)) {
            // 触发状态移除回调
            if (onStateRemoved) {
                onStateRemoved(state);
            }
            
            // 移除状态
            it = activeStates.erase(it);
        } else {
            // 触发状态更新回调
            if (onStateUpdated) {
                onStateUpdated(state);
            }
            
            ++it;
        }
    }
}

EntityStateEffect* EntityStateManager::addState(
    EntityStateEffect::Type type,
    const std::string& name,
    int duration,
    int priority
) {
    // 检查是否已存在同名状态
    for (auto& state : activeStates) {
        if (state->getName() == name) {
            // 如果新状态优先级更高，移除旧状态
            if (priority > state->getPriority()) {
                removeState(name);
                break;
            } else {
                // 否则不添加新状态
                return nullptr;
            }
        }
    }
    
    // 创建新状态
    auto newState = std::make_unique<EntityStateEffect>(type, name, duration, priority);
    EntityStateEffect* statePtr = newState.get();
    
    // 激活状态
    statePtr->start();
    
    // 添加到状态列表
    activeStates.push_back(std::move(newState));
    
    // 触发状态添加回调
    if (onStateAdded) {
        onStateAdded(statePtr);
    }
    
    return statePtr;
}

bool EntityStateManager::removeState(const std::string& name) {
    auto it = std::find_if(activeStates.begin(), activeStates.end(),
        [&name](const auto& state) { return state->getName() == name; });
    
    if (it != activeStates.end()) {
        // 触发状态移除回调
        if (onStateRemoved) {
            onStateRemoved(it->get());
        }
        
        // 结束状态
        (*it)->end();
        
        // 移除状态
        activeStates.erase(it);
        return true;
    }
    
    return false;
}

bool EntityStateManager::removeState(EntityStateEffect::Type type) {
    auto it = std::find_if(activeStates.begin(), activeStates.end(),
        [type](const auto& state) { return state->getType() == type; });
    
    if (it != activeStates.end()) {
        // 触发状态移除回调
        if (onStateRemoved) {
            onStateRemoved(it->get());
        }
        
        // 结束状态
        (*it)->end();
        
        // 移除状态
        activeStates.erase(it);
        return true;
    }
    
    return false;
}

bool EntityStateManager::hasState(const std::string& name) const {
    return std::any_of(activeStates.begin(), activeStates.end(),
        [&name](const auto& state) { return state->getName() == name; });
}

bool EntityStateManager::hasState(EntityStateEffect::Type type) const {
    return std::any_of(activeStates.begin(), activeStates.end(),
        [type](const auto& state) { return state->getType() == type; });
}

EntityStateEffect* EntityStateManager::getState(const std::string& name) {
    auto it = std::find_if(activeStates.begin(), activeStates.end(),
        [&name](const auto& state) { return state->getName() == name; });
    
    return (it != activeStates.end()) ? it->get() : nullptr;
}

EntityStateEffect* EntityStateManager::getState(EntityStateEffect::Type type) {
    auto it = std::find_if(activeStates.begin(), activeStates.end(),
        [type](const auto& state) { return state->getType() == type; });
    
    return (it != activeStates.end()) ? it->get() : nullptr;
}

void EntityStateManager::clearStates() {
    // 结束所有状态
    for (auto& state : activeStates) {
        state->end();
        
        // 触发状态移除回调
        if (onStateRemoved) {
            onStateRemoved(state.get());
        }
    }
    
    // 清空状态列表
    activeStates.clear();
}

std::string EntityStateManager::serializeStates() const {
    std::stringstream ss;
    
    // 序列化状态数量
    ss << activeStates.size() << "|";
    
    // 序列化每个状态
    for (const auto& state : activeStates) {
        ss << state->serialize() << "#";
    }
    
    return ss.str();
}

void EntityStateManager::deserializeStates(const std::string& data) {
    // 清空当前状态
    clearStates();
    
    std::stringstream ss(data);
    std::string token;
    
    // 解析状态数量
    std::getline(ss, token, '|');
    int stateCount = std::stoi(token);
    
    // 解析每个状态
    std::string remainingData;
    std::getline(ss, remainingData);
    
    std::stringstream stateStream(remainingData);
    for (int i = 0; i < stateCount; ++i) {
        std::string stateData;
        std::getline(stateStream, stateData, '#');
        
        // 反序列化状态
        EntityStateEffect state = EntityStateEffect::deserialize(stateData);
        
        // 添加状态
        addState(state.getType(), state.getName(), state.getDuration(), state.getPriority());
    }
} 