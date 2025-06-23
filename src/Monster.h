//#pragma once
//#ifndef MONSTER_H
//#define MONSTER_H
//
//#include "Entity.h"
//#include "MonsterState.h"
//#include "Storage.h"
//#include <SDL3/SDL.h>
//#include <memory> // For std::unique_ptr
//#include <functional> // For std::function
//
//// Forward declaration
//class Gun;
//
//class Monster : public Entity {
//private:
//    MonsterState currentState;  // 当前状态
//    Entity* target;             // 目标实体
//    float detectionRange;       // 检测范围
//    float chaseDistance;        // 追逐距离
//    float attackRange;          // 攻击范围
//    // int attackDamage;        // 攻击伤害 (将由武器决定)
//    // 攻击冷却已迁移到Entity类中
//    std::unique_ptr<Gun> equippedWeapon; // 怪物装备的武器（手持武器）
//    std::unique_ptr<Storage> backpack;   // 怪物的背包存储空间
//    bool itemsDropped;          // 标记物品是否已掉落
//    bool colliderRemoved;       // 标记碰撞体积是否已移除
//
//public:
//    Monster(int startX, int startY, Entity* targetEntity = nullptr);
//    
//    // 状态机相关方法
//    void setState(MonsterState newState);
//    MonsterState getState() const { return currentState; }
//    
//    // 设置目标
//    void setTarget(Entity* targetEntity) { target = targetEntity; }
//    
//    // 重写更新方法
//    void update(float deltaTime = 1.0f / 60.0f) override;
//    
//    // 重写渲染方法
//    void render(SDL_Renderer* renderer, int cameraX, int cameraY) override;
//    
//    // 重写装备物品方法，区分手持武器和穿戴物品
//    float equipItem(std::unique_ptr<Item> item);
//    
//    // 获取当前手持武器
//    Gun* getEquippedWeapon() const { return equippedWeapon.get(); }
//    
//    // 状态处理方法
//    void handleIdleState();
//    void handleChaseState();
//    void handleAttackState();
//    void handleDeadState();
//    
//    // 计算与目标的距离
//    float distanceToTarget() const;
//    
//    // 计算与目标的距离平方（新增方法）
//    float distanceToTargetSquared() const;
//    
//    // 检查是否能看到目标（新增方法）
//    bool canSeeTarget() const;
//    
//    // 射线检测（新增方法）
//    bool raycast(float startX, float startY, float endX, float endY, const std::vector<Collider>& obstacles) const;
//    // 移除旧的 attackDamage，伤害将来自由武器
//    // int getAttackDamage() const { return attackDamage; }
//    
//    // Storage相关方法
//    Storage* getBackpack() const { return backpack.get(); }
//    bool addItemToBackpack(std::unique_ptr<Item> item);
//    std::unique_ptr<Item> removeItemFromBackpack(int index);
//    bool transferItem(Item* item, Storage* sourceStorage, Storage* targetStorage, std::function<void(bool)> callback = nullptr);
//    
//    // 碰撞体积相关方法
//    void removeCollider(); // 移除碰撞体积
//    bool isColliderRemoved() const { return colliderRemoved; }
//    bool isDead() const { return currentState == MonsterState::DEAD; }
//};
//
//#endif // MONSTER_H