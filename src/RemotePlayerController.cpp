#include "RemotePlayerController.h"
#include <iostream>

RemotePlayerController::RemotePlayerController(Player* player, float pathRadius, float moveSpeed)
    : PlayerController(player, false), // 设置为非本地控制器
      time(0.0f), radius(pathRadius), speed(moveSpeed) {
    
    // 获取玩家初始位置作为圆心
    if (player) {
        centerX = player->getX();
        centerY = player->getY();
        
        // 设置玩家名称和ID
        player->setIsLocalPlayer(false);
        player->setPlayerName("RemotePlayer");
        player->setPlayerId(999); // 假设ID为999
        
        std::cout << "Remote player created at position: " << centerX << ", " << centerY << std::endl;
    }
}

void RemotePlayerController::update(float deltaTime) {
    if (!getPlayer()) return;
    
    // 累加时间
    time += deltaTime * speed;
    
    // 更新位置
    updatePosition(deltaTime);
    
    // 更新状态
    updateStateFromPlayer();
}

void RemotePlayerController::updatePosition(float deltaTime) {
    Player* player = getPlayer();
    if (!player) return;
    
    // 计算圆形路径上的新位置
    float x = centerX + radius * std::cos(time);
    float y = centerY + radius * std::sin(time);
    
    // 计算移动方向
    float dirX = -std::sin(time); // 垂直于半径的方向
    float dirY = std::cos(time);
    
    // 直接设置玩家位置（在实际网络中，会通过插值平滑移动）
    player->setPosition(x, y);
    
    // 更新玩家状态
    PlayerState state = getState();
    state.x = x;
    state.y = y;
    state.directionX = dirX;
    state.directionY = dirY;
    state.isMoving = true;
    
    // 每5秒尝试射击一次
    state.isShooting = (std::fmod(time, 5.0f) < 0.1f);
    
    // 设置状态
    setState(state);
    
    // 如果需要射击，调用射击方法
    if (state.isShooting) {
        player->attemptShoot();
    }
} 