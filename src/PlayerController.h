#pragma once
#ifndef PLAYER_CONTROLLER_H
#define PLAYER_CONTROLLER_H

#include "Player.h"
#include "PlayerState.h"
#include <SDL3/SDL.h>
#include <memory>

// 玩家控制器类，处理输入和控制逻辑
class PlayerController {
private:
    Player* controlledPlayer;  // 被控制的玩家实体
    bool isLocalController;    // 是否为本地控制器
    PlayerState currentState;  // 当前玩家状态
    
    // 输入状态
    bool keyUp;
    bool keyDown;
    bool keyLeft;
    bool keyRight;
    bool keyReload;
    bool keyInteract;
    bool mouseLeftDown;
    int mouseX;
    int mouseY;
    
    // 处理键盘输入
    void processKeyboardInput(const bool* keyState);
    
    // 处理鼠标输入
    void processMouseInput();
    
    // 应用输入到玩家实体
    void applyInputToPlayer(float deltaTime);

public:
    // 构造函数
    PlayerController(Player* player, bool isLocal = true);
    
    // 虚析构函数
    virtual ~PlayerController() = default;
    
    // 更新控制器
    virtual void update(float deltaTime);
    
    // 处理输入事件
    void handleEvent(const SDL_Event& event, int cameraX, int cameraY);
    
    // 获取和设置玩家状态
    const PlayerState& getState() const { return currentState; }
    void setState(const PlayerState& state);
    
    // 从玩家实体更新状态
    void updateStateFromPlayer();
    
    // 将状态应用到玩家实体
    void applyStateToPlayer();
    
    // 获取控制的玩家
    Player* getPlayer() const { return controlledPlayer; }
};

#endif // PLAYER_CONTROLLER_H 