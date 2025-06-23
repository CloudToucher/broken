#pragma once
#ifndef ENTITY_FLAG_H
#define ENTITY_FLAG_H

#include <string>

// 实体标志枚举
enum class EntityFlag {
    HAS_VISION,           // 有视觉
    HAS_HEARING,          // 有听觉
    HAS_SMELL,            // 有味觉
    HAS_INFRARED_VISION,  // 有红外视觉
    HAS_INTELLIGENCE,     // 有智慧
    IS_ZOMBIE,            // 丧尸化生物
    CAN_RUSH,             // 可以冲刺突袭的
    CAN_FLY,              // 可以飞行的
    CAN_USE_WEAPONS,      // 可以使用武器攻击的
    CAN_USE_GUNS          // 可以使用枪械射击的
};

// 辅助函数：将EntityFlag转换为字符串
inline std::string EntityFlagToString(EntityFlag flag) {
    switch (flag) {
        case EntityFlag::HAS_VISION: return "HAS_VISION";
        case EntityFlag::HAS_HEARING: return "HAS_HEARING";
        case EntityFlag::HAS_SMELL: return "HAS_SMELL";
        case EntityFlag::HAS_INFRARED_VISION: return "HAS_INFRARED_VISION";
        case EntityFlag::HAS_INTELLIGENCE: return "HAS_INTELLIGENCE";
        case EntityFlag::IS_ZOMBIE: return "IS_ZOMBIE";
        case EntityFlag::CAN_RUSH: return "CAN_RUSH";
        case EntityFlag::CAN_FLY: return "CAN_FLY";
        case EntityFlag::CAN_USE_WEAPONS: return "CAN_USE_WEAPONS";
        case EntityFlag::CAN_USE_GUNS: return "CAN_USE_GUNS";
        default: return "UNKNOWN_FLAG";
    }
}

#endif // ENTITY_FLAG_H 