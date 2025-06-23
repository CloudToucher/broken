#pragma once
#ifndef PLAYER_STATE_H
#define PLAYER_STATE_H

#include <string>

// 玩家状态结构，用于网络同步
struct PlayerState {
    // 基本位置和方向
    int x;
    int y;
    float directionX;
    float directionY;
    
    // 健康状态
    int health;
    int maxHealth;
    
    // 动作状态
    bool isShooting;
    bool isReloading;
    bool isMoving;
    
    // 装备状态
    std::string heldItemId; // 手持物品的唯一标识符
    
    // 网络相关
    int playerId;          // 玩家唯一ID
    std::string playerName;// 玩家名称
    
    // 构造函数
    PlayerState() : 
        x(0), y(0), directionX(0), directionY(0),
        health(100), maxHealth(100),
        isShooting(false), isReloading(false), isMoving(false),
        heldItemId(""), playerId(-1), playerName("") {}
};

#endif // PLAYER_STATE_H 