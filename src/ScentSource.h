#pragma once
#ifndef SCENT_SOURCE_H
#define SCENT_SOURCE_H

#include <string>

// 前向声明
class Entity;

// 气味源结构体
struct ScentSource {
    Entity* owner;          // 气味源所有者
    int x;                  // 气味源位置X
    int y;                  // 气味源位置Y
    int intensity;          // 气味强度（0-100）
    float radius;           // 气味扩散半径（像素）
    std::string scentType;  // 气味类型（如"血液"、"腐烂"等）
    int duration;           // 持续时间（毫秒，-1表示永久）
    int age;                // 已存在时间（毫秒）
    bool isActive;          // 是否活跃

    // 构造函数
    ScentSource(
        Entity* sourceOwner,
        int sourceX,
        int sourceY,
        int sourceIntensity,
        float sourceRadius,
        const std::string& type,
        int sourceDuration = -1
    ) : owner(sourceOwner),
        x(sourceX),
        y(sourceY),
        intensity(sourceIntensity),
        radius(sourceRadius),
        scentType(type),
        duration(sourceDuration),
        age(0),
        isActive(true) {}

    // 更新方法
    void update(int deltaTimeMs) {
        // 更新年龄
        age += deltaTimeMs;
        
        // 如果有持续时间限制，检查是否已过期
        if (duration > 0 && age >= duration) {
            isActive = false;
            return;
        }
        
        // 随着时间推移，气味强度逐渐减弱
        // 每10秒减弱10%的强度
        float decayRate = deltaTimeMs / 10000.0f * 0.1f;
        intensity = std::max(0, static_cast<int>(intensity * (1.0f - decayRate)));
        
        // 如果强度为0，标记为非活跃
        if (intensity <= 0) {
            isActive = false;
        }
    }
    
    // 更新位置方法（如果气味源移动）
    void updatePosition(int newX, int newY) {
        x = newX;
        y = newY;
    }
};

#endif // SCENT_SOURCE_H 