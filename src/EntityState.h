#pragma once
#ifndef ENTITY_STATE_H
#define ENTITY_STATE_H

// 实体状态枚举
enum class EntityState {
     IDLE,               // 空闲状态，可以自由行动
     RELOADING,          // 换弹中，不可手上操作，移动减速
     UNLOADING,          // 卸下弹匣中，不可手上操作，移动减速
     CHAMBERING,         // 上膛/拉栓中，不可手上操作，移动减速
     STUNNED,            // 被击晕，不可操作，不可移动
     HEALING,            // 治疗中，不可手上操作，移动减速
     AIMING,             // 瞄准中，不可手上操作，移动减速
     SPRINTING,          // 冲刺中，不可手上操作，移动加速
     CROUCHING,          // 蹲伏中，不可手上操作，移动减速
     PRONE,              // 卧倒中，不可手上操作，移动极慢
     EQUIPPING,          // 穿戴物品中，不可手上操作，移动减速
     UNEQUIPPING,        // 卸下物品中，不可手上操作，移动减速
     STORING_ITEM,       // 存储物品中，不可手上操作，移动减速
     TAKING_ITEM,        // 取出物品中，不可手上操作，移动减速
     TRANSFERRING_ITEM,  // 转移物品中，不可手上操作，移动减速
     SLOWED,             // 被减速，移动速度降低
     DEAD
};

#endif // ENTITY_STATE_H