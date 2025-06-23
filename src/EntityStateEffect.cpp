#include "EntityStateEffect.h"
#include <sstream>

EntityStateEffect::EntityStateEffect(
    Type stateType,
    const std::string& stateName,
    int stateDuration,
    int statePriority
) : type(stateType),
    name(stateName),
    duration(stateDuration),
    remainingTime(stateDuration),
    isActive(false),
    priority(statePriority),
    userData(nullptr),
    networkId(-1) {
}

EntityStateEffect::~EntityStateEffect() {
    // 确保状态结束时清理资源
    if (isActive) {
        end();
    }
}

bool EntityStateEffect::update(int deltaTimeMs) {
    if (!isActive) return false;
    
    // 如果是永久状态，直接返回true
    if (duration == -1) {
        if (onUpdate) onUpdate();
        return true;
    }
    
    // 更新剩余时间
    remainingTime -= deltaTimeMs;
    
    // 如果状态已结束
    if (remainingTime <= 0) {
        end();
        return false;
    }
    
    // 调用更新回调
    if (onUpdate) onUpdate();
    
    return true;
}

void EntityStateEffect::start() {
    if (isActive) return;
    
    isActive = true;
    
    // 调用开始回调
    if (onStart) onStart();
}

void EntityStateEffect::end() {
    if (!isActive) return;
    
    isActive = false;
    
    // 调用结束回调
    if (onEnd) onEnd();
}

std::string EntityStateEffect::serialize() const {
    std::stringstream ss;
    
    // 序列化基本信息
    ss << static_cast<int>(type) << "|";
    ss << name << "|";
    ss << duration << "|";
    ss << remainingTime << "|";
    ss << (isActive ? "1" : "0") << "|";
    ss << priority << "|";
    ss << networkId;
    
    return ss.str();
}

EntityStateEffect EntityStateEffect::deserialize(const std::string& data) {
    std::stringstream ss(data);
    std::string token;
    
    // 解析类型
    std::getline(ss, token, '|');
    Type type = static_cast<Type>(std::stoi(token));
    
    // 解析名称
    std::getline(ss, token, '|');
    std::string name = token;
    
    // 解析持续时间
    std::getline(ss, token, '|');
    int duration = std::stoi(token);
    
    // 解析剩余时间
    std::getline(ss, token, '|');
    int remainingTime = std::stoi(token);
    
    // 解析是否激活
    std::getline(ss, token, '|');
    bool isActive = (token == "1");
    
    // 解析优先级
    std::getline(ss, token, '|');
    int priority = std::stoi(token);
    
    // 解析网络ID
    std::getline(ss, token, '|');
    int networkId = std::stoi(token);
    
    // 创建状态效果
    EntityStateEffect effect(type, name, duration, priority);
    effect.remainingTime = remainingTime;
    effect.isActive = isActive;
    effect.networkId = networkId;
    
    return effect;
} 