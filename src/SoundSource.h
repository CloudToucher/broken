#pragma once
#ifndef SOUND_SOURCE_H
#define SOUND_SOURCE_H

#include <string>

// 前向声明
class Entity;

// 声音类型枚举
enum class SoundType {
    GUNSHOT,        // 枪声
    FOOTSTEP,       // 脚步声
    VOICE,          // 人声
    IMPACT,         // 撞击声
    EXPLOSION,      // 爆炸声
    CREATURE,       // 生物声音
    ENVIRONMENT,    // 环境声音
    MECHANICAL      // 机械声音
};

// 声源结构体
struct SoundSource {
    Entity* owner;          // 声音源所有者（可以为nullptr）
    int x;                  // 声音源位置X
    int y;                  // 声音源位置Y
    int intensity;          // 声音强度（0-100）
    float radius;           // 声音传播半径（像素）
    SoundType soundType;    // 声音类型
    std::string soundFile;  // 声音文件路径（可选）
    int duration;           // 持续时间（毫秒，0表示瞬时声音）
    int age;                // 已存在时间（毫秒）
    bool isActive;          // 是否活跃

    // 构造函数
    SoundSource(
        Entity* sourceOwner,
        int sourceX,
        int sourceY,
        int sourceIntensity,
        float sourceRadius,
        SoundType type,
        const std::string& file = "",
        int sourceDuration = 0
    ) : owner(sourceOwner),
        x(sourceX),
        y(sourceY),
        intensity(sourceIntensity),
        radius(sourceRadius),
        soundType(type),
        soundFile(file),
        duration(sourceDuration),
        age(0),
        isActive(true) {}

    // 更新方法
    void update(int deltaTimeMs) {
        // 更新年龄
        age += deltaTimeMs;
        
        // 如果是瞬时声音或者已经超过持续时间，标记为非活跃
        if (duration == 0 || (duration > 0 && age >= duration)) {
            isActive = false;
            return;
        }
        
        // 对于持续性声音，随时间衰减
        if (duration > 0) {
            float decayRate = deltaTimeMs / static_cast<float>(duration);
            intensity = std::max(0, static_cast<int>(intensity * (1.0f - decayRate)));
            
            // 如果强度为0，标记为非活跃
            if (intensity <= 0) {
                isActive = false;
            }
        }
    }
    
    // 更新位置方法（如果声源移动）
    void updatePosition(int newX, int newY) {
        x = newX;
        y = newY;
    }
};

#endif // SOUND_SOURCE_H 