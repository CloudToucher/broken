#pragma once
#ifndef ENTITY_STATE_EFFECT_H
#define ENTITY_STATE_EFFECT_H

#include <string>
#include <functional>

// 实体状态效果类，用于管理实体状态及其持续时间
class EntityStateEffect {
public:
    enum class Type {
        MOVING,         // 移动中
        SHOOTING,       // 射击中
        RELOADING,      // 换弹中
        INTERACTING,    // 交互中
        STUNNED,        // 眩晕
        HEALING,        // 治疗中
        BUFFED,         // 增益
        DEBUFFED,       // 减益
        ATTACKING,      // 攻击中
        CUSTOM          // 自定义状态
    };

private:
    Type type;                     // 状态类型
    std::string name;              // 状态名称
    int duration;                  // 持续时间（毫秒，-1表示永久）
    int remainingTime;             // 剩余时间（毫秒）
    bool isActive;                 // 是否激活
    int priority;                  // 优先级（高优先级状态可以覆盖低优先级状态）
    
    // 回调函数
    std::function<void()> onStart;     // 状态开始时的回调
    std::function<void()> onUpdate;    // 状态更新时的回调
    std::function<void()> onEnd;       // 状态结束时的回调
    
    // 状态数据（可以存储任意数据）
    void* userData;
    
    // 网络同步ID
    int networkId;

public:
    // 构造函数
    EntityStateEffect(
        Type stateType,
        const std::string& stateName,
        int stateDuration = -1,
        int statePriority = 0
    );
    
    // 析构函数
    ~EntityStateEffect();
    
    // 更新状态（返回false表示状态已结束）
    bool update(int deltaTimeMs);
    
    // 开始状态
    void start();
    
    // 结束状态
    void end();
    
    // 设置回调函数
    void setOnStart(std::function<void()> callback) { onStart = callback; }
    void setOnUpdate(std::function<void()> callback) { onUpdate = callback; }
    void setOnEnd(std::function<void()> callback) { onEnd = callback; }
    
    // 获取状态信息
    Type getType() const { return type; }
    const std::string& getName() const { return name; }
    int getDuration() const { return duration; }
    int getRemainingTime() const { return remainingTime; }
    bool getIsActive() const { return isActive; }
    int getPriority() const { return priority; }
    
    // 设置用户数据
    template<typename T>
    void setUserData(T* data) { userData = static_cast<void*>(data); }
    
    // 获取用户数据
    template<typename T>
    T* getUserData() const { return static_cast<T*>(userData); }
    
    // 网络同步相关
    void setNetworkId(int id) { networkId = id; }
    int getNetworkId() const { return networkId; }
    
    // 序列化状态（用于网络传输）
    std::string serialize() const;
    
    // 反序列化状态（用于网络接收）
    static EntityStateEffect deserialize(const std::string& data);
};

#endif // ENTITY_STATE_EFFECT_H 