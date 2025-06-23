#include "PlayerController.h"
#include "Game.h"
#include <iostream>
#include <cmath>

PlayerController::PlayerController(Player* player, bool isLocal)
    : controlledPlayer(player), isLocalController(isLocal),
      keyUp(false), keyDown(false), keyLeft(false), keyRight(false),
      keyReload(false), keyInteract(false), mouseLeftDown(false),
      mouseX(0), mouseY(0) {
    
    // 初始化玩家状态
    if (player) {
        updateStateFromPlayer();
    }
}

void PlayerController::update(float deltaTime) {
    if (!controlledPlayer || !isLocalController) return;
    
    // 获取键盘状态
    const bool* keyState = SDL_GetKeyboardState(nullptr);
    processKeyboardInput(keyState);
    
    // 处理鼠标输入
    processMouseInput();
    
    // 应用输入到玩家
    applyInputToPlayer(deltaTime);
    
    // 更新状态
    updateStateFromPlayer();
}

void PlayerController::handleEvent(const SDL_Event& event, int cameraX, int cameraY) {
    if (!controlledPlayer || !isLocalController) return;
    
    switch (event.type) {
        case SDL_EVENT_MOUSE_MOTION:
            mouseX = event.motion.x;
            mouseY = event.motion.y;
            controlledPlayer->handleMouseMotion(mouseX, mouseY, cameraX, cameraY);
            break;
            
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if (event.button.button == SDL_BUTTON_LEFT) {
                mouseLeftDown = true;
                controlledPlayer->handleMouseClick(SDL_BUTTON_LEFT);
            }
            break;
            
        case SDL_EVENT_MOUSE_BUTTON_UP:
            if (event.button.button == SDL_BUTTON_LEFT) {
                mouseLeftDown = false;
                controlledPlayer->handleMouseRelease(SDL_BUTTON_LEFT);
            }
            break;
    }
}

void PlayerController::processKeyboardInput(const bool* keyState) {
    keyUp = keyState[SDL_SCANCODE_W] || keyState[SDL_SCANCODE_UP];
    keyDown = keyState[SDL_SCANCODE_S] || keyState[SDL_SCANCODE_DOWN];
    keyLeft = keyState[SDL_SCANCODE_A] || keyState[SDL_SCANCODE_LEFT];
    keyRight = keyState[SDL_SCANCODE_D] || keyState[SDL_SCANCODE_RIGHT];
    keyReload = keyState[SDL_SCANCODE_R];
    keyInteract = keyState[SDL_SCANCODE_E];
}

void PlayerController::processMouseInput() {
    // 目前在handleEvent中处理鼠标事件
    // 这里可以添加额外的鼠标处理逻辑
}

void PlayerController::applyInputToPlayer(float deltaTime) {
    // 创建一个布尔数组来模拟当前的键盘状态
    bool keyState[SDL_SCANCODE_MODE] = {false};
    keyState[SDL_SCANCODE_W] = keyUp;
    keyState[SDL_SCANCODE_UP] = keyUp;
    keyState[SDL_SCANCODE_S] = keyDown;
    keyState[SDL_SCANCODE_DOWN] = keyDown;
    keyState[SDL_SCANCODE_A] = keyLeft;
    keyState[SDL_SCANCODE_LEFT] = keyLeft;
    keyState[SDL_SCANCODE_D] = keyRight;
    keyState[SDL_SCANCODE_RIGHT] = keyRight;
    keyState[SDL_SCANCODE_R] = keyReload;
    keyState[SDL_SCANCODE_E] = keyInteract;
    
    // 应用键盘输入
    controlledPlayer->handleInput(keyState, deltaTime);
    
    // 处理特殊按键
    if (keyReload) {
        controlledPlayer->attemptReload();
    }
}

void PlayerController::updateStateFromPlayer() {
    if (!controlledPlayer) return;
    
    // 更新位置和方向
    currentState.x = controlledPlayer->getX();
    currentState.y = controlledPlayer->getY();
    
    // 计算方向向量（从玩家到鼠标）
    float dx = mouseX - currentState.x;
    float dy = mouseY - currentState.y;
    float length = std::sqrt(dx * dx + dy * dy);
    if (length > 0) {
        currentState.directionX = dx / length;
        currentState.directionY = dy / length;
    }
    
    // 更新健康状态
    currentState.health = controlledPlayer->getHealth();
    
    // 更新动作状态
    currentState.isMoving = keyUp || keyDown || keyLeft || keyRight;
    currentState.isShooting = mouseLeftDown;
    currentState.isReloading = keyReload;
    
    // 更新装备状态
    Item* heldItem = controlledPlayer->getHeldItem();
    if (heldItem) {
        currentState.heldItemId = heldItem->getName(); // 简化，实际应使用唯一ID
    } else {
        currentState.heldItemId = "";
    }
}

void PlayerController::setState(const PlayerState& state) {
    currentState = state;
    
    // 如果是远程控制器，则应用状态到玩家
    if (!isLocalController && controlledPlayer) {
        applyStateToPlayer();
    }
}

void PlayerController::applyStateToPlayer() {
    if (!controlledPlayer) return;
    
    // 应用位置（简化版，实际应考虑插值）
    // 注意：这里不直接设置位置，而是让物理系统处理
    // 可以添加一个目标位置，然后让物理系统平滑移动
    
    // 应用健康状态
    // 注意：实际应用中需要更复杂的逻辑处理
    
    // 应用动作状态
    if (currentState.isShooting) {
        controlledPlayer->attemptShoot();
    }
    
    if (currentState.isReloading) {
        controlledPlayer->attemptReload();
    }
    
    // 应用装备状态（简化版）
    // 实际应用中需要更复杂的逻辑处理
} 