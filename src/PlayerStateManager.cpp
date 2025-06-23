#include "PlayerStateManager.h"
#include "Player.h"
#include <sstream>

PlayerStateManager::PlayerStateManager(Player* player)
    : player(player) {
    // 初始化状态
    state = std::make_unique<PlayerState>();
    
    // 初始化状态管理器
    stateManager = std::make_unique<EntityStateManager>();
    
    // 设置状态变化回调
    stateManager->setOnStateAdded([this](EntityStateEffect* effect) {
        if (onStateAdded) {
            onStateAdded(effect);
        }
        
        // 同步状态到玩家
        syncStateToPlayer();
    });
    
    stateManager->setOnStateRemoved([this](EntityStateEffect* effect) {
        if (onStateRemoved) {
            onStateRemoved(effect);
        }
        
        // 同步状态到玩家
        syncStateToPlayer();
    });
    
    stateManager->setOnStateUpdated([this](EntityStateEffect* effect) {
        if (onStateUpdated) {
            onStateUpdated(effect);
        }
    });
    
    // 初始同步
    syncPlayerToState();
}

PlayerStateManager::~PlayerStateManager() {
    // 清理资源
}

void PlayerStateManager::update(float deltaTime) {
    // 更新状态管理器
    if (stateManager) {
        stateManager->update(static_cast<int>(deltaTime * 1000)); // 转换为毫秒
    }
    
    // 同步玩家到状态
    syncPlayerToState();
    
    // 同步状态到玩家
    syncStateToPlayer();
}

void PlayerStateManager::setState(const PlayerState& newState) {
    // 更新状态
    *state = newState;
    
    // 同步状态到玩家
    syncStateToPlayer();
    
    // 触发状态变化回调
    if (onStateChanged) {
        onStateChanged(*state);
    }
}

EntityStateEffect* PlayerStateManager::addState(
    EntityStateEffect::Type type,
    const std::string& name,
    int duration,
    int priority
) {
    if (!stateManager) return nullptr;
    return stateManager->addState(type, name, duration, priority);
}

bool PlayerStateManager::removeState(const std::string& name) {
    if (!stateManager) return false;
    return stateManager->removeState(name);
}

bool PlayerStateManager::removeState(EntityStateEffect::Type type) {
    if (!stateManager) return false;
    return stateManager->removeState(type);
}

bool PlayerStateManager::hasState(const std::string& name) const {
    if (!stateManager) return false;
    return stateManager->hasState(name);
}

bool PlayerStateManager::hasState(EntityStateEffect::Type type) const {
    if (!stateManager) return false;
    return stateManager->hasState(type);
}

EntityStateEffect* PlayerStateManager::getState(const std::string& name) {
    if (!stateManager) return nullptr;
    return stateManager->getState(name);
}

EntityStateEffect* PlayerStateManager::getState(EntityStateEffect::Type type) {
    if (!stateManager) return nullptr;
    return stateManager->getState(type);
}

void PlayerStateManager::clearStates() {
    if (stateManager) {
        stateManager->clearStates();
    }
}

void PlayerStateManager::syncStateToPlayer() {
    if (!player || !state) return;
    
    // 更新玩家位置
    player->setPosition(state->x, state->y);
    
    // 更新玩家状态标志
    // 根据状态效果更新玩家状态
    state->isShooting = hasState(EntityStateEffect::Type::SHOOTING);
    state->isReloading = hasState(EntityStateEffect::Type::RELOADING);
    state->isMoving = hasState(EntityStateEffect::Type::MOVING);
    
    // 更新玩家ID和名称
    player->setPlayerId(state->playerId);
    player->setPlayerName(state->playerName);
}

void PlayerStateManager::syncPlayerToState() {
    if (!player || !state) return;
    
    // 更新状态位置
    state->x = player->getX();
    state->y = player->getY();
    
    // 更新状态方向
    // 假设Player类有获取方向的方法
    // state->directionX = player->getDirectionX();
    // state->directionY = player->getDirectionY();
    
    // 更新状态健康值
    state->health = player->getHealth();
    
    // 更新玩家ID和名称
    state->playerId = player->getPlayerId();
    state->playerName = player->getPlayerName();
    
    // 更新手持物品ID
    Item* heldItem = player->getHeldItem();
    state->heldItemId = heldItem ? heldItem->getUniqueId() : "";
}

std::string PlayerStateManager::serializeState() const {
    if (!state) return "";
    
    std::stringstream ss;
    
    // 序列化基本信息
    ss << state->x << "|";
    ss << state->y << "|";
    ss << state->directionX << "|";
    ss << state->directionY << "|";
    ss << state->health << "|";
    ss << state->maxHealth << "|";
    ss << (state->isShooting ? "1" : "0") << "|";
    ss << (state->isReloading ? "1" : "0") << "|";
    ss << (state->isMoving ? "1" : "0") << "|";
    ss << state->heldItemId << "|";
    ss << state->playerId << "|";
    ss << state->playerName << "|";
    
    // 序列化状态效果
    if (stateManager) {
        ss << stateManager->serializeStates();
    }
    
    return ss.str();
}

void PlayerStateManager::deserializeState(const std::string& data) {
    if (!state) return;
    
    std::stringstream ss(data);
    std::string token;
    
    // 解析基本信息
    std::getline(ss, token, '|'); state->x = std::stoi(token);
    std::getline(ss, token, '|'); state->y = std::stoi(token);
    std::getline(ss, token, '|'); state->directionX = std::stof(token);
    std::getline(ss, token, '|'); state->directionY = std::stof(token);
    std::getline(ss, token, '|'); state->health = std::stoi(token);
    std::getline(ss, token, '|'); state->maxHealth = std::stoi(token);
    std::getline(ss, token, '|'); state->isShooting = (token == "1");
    std::getline(ss, token, '|'); state->isReloading = (token == "1");
    std::getline(ss, token, '|'); state->isMoving = (token == "1");
    std::getline(ss, token, '|'); state->heldItemId = token;
    std::getline(ss, token, '|'); state->playerId = std::stoi(token);
    std::getline(ss, token, '|'); state->playerName = token;
    
    // 解析状态效果
    std::string remainingData;
    std::getline(ss, remainingData);
    
    if (stateManager && !remainingData.empty()) {
        stateManager->deserializeStates(remainingData);
    }
    
    // 同步状态到玩家
    syncStateToPlayer();
    
    // 触发状态变化回调
    if (onStateChanged) {
        onStateChanged(*state);
    }
}
