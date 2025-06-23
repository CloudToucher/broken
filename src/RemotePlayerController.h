#pragma once
#ifndef REMOTE_PLAYER_CONTROLLER_H
#define REMOTE_PLAYER_CONTROLLER_H

#include "PlayerController.h"
#include "Player.h"
#include <cmath>

// 远程玩家控制器，用于模拟网络玩家
class RemotePlayerController : public PlayerController {
private:
    float time;               // 累计时间
    float radius;             // 圆形路径半径
    float speed;              // 移动速度
    float centerX, centerY;   // 圆形路径中心点

    // 更新位置
    void updatePosition(float deltaTime);

public:
    // 构造函数
    RemotePlayerController(Player* player, float pathRadius = 200.0f, float moveSpeed = 5.0f);  // 5格/秒对应的弧度速度
    
    // 更新控制器
    void update(float deltaTime) override;
};

#endif // REMOTE_PLAYER_CONTROLLER_H 