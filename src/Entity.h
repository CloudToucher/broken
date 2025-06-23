#pragma once
#ifndef ENTITY_H
#define ENTITY_H

#include <vector>
#include <unordered_set>
#include "Collider.h"
#include "Damage.h"
#include <SDL3/SDL.h>
#include <memory> // For std::unique_ptr
#include "EntityState.h" // 旧的状态枚举
#include "EntityStateManager.h" // 新的状态管理器
#include "ActionQueue.h" // 添加行为队列头文件
#include "EquipmentSystem.h" // 添加装备系统头文件
#include "EntityFlag.h" // 添加实体标志头文件

// 前向声明
class Bullet;
class Game;
class Gun;
class Magazine;

// 实体阵营枚举
enum class Faction {
    PLAYER,
    ENEMY,
    NEUTRAL,
    ENVIRONMENT,
    HOSTILE
};

class Entity {
protected:
    float x, y;                // 实体位置（浮点数精度）
    float prevX, prevY;        // 上一帧位置（用于碰撞检测）
    int radius;                // 实体半径
    int speed;                 // 移动速度
    int health;                // 生命值
    SDL_Color color;           // 实体颜色
    Collider collider;         // 碰撞体
    Faction faction;           // 实体阵营

    // 新增属性
    std::unordered_set<EntityFlag> flags;  // 实体标志列表
    float volume;              // 体积大小（L）
    float weight;              // 重量（kg）
    float height;              // 身高（m）
    int smellIntensity;        // 体味大小（0-100）
    int soundIntensity;        // 行动声音大小（0-100）
    std::string soundFile;     // 行动声音文件名
    
    // 属性值
    int strength;              // 力量
    int dexterity;             // 敏捷
    int perception;            // 感知
    int intelligence;          // 智力

    // 旧的状态相关变量（将逐步替换为新的状态系统）
    EntityState currentState;  // 当前状态
    float stateTimer;          // 状态持续时间计时器
    float speedModifier;       // 速度修正系数

    // 新的状态管理器
    std::unique_ptr<EntityStateManager> stateManager;

    // 添加装备系统成员
    std::unique_ptr<EquipmentSystem> equipmentSystem;

    // 添加行为队列成员
    std::unique_ptr<ActionQueue> actionQueue;

    // 射击冷却相关变量
    int shootCooldown;         // 射击冷却计时器

    // 物理引擎相关属性
    float velocityX, velocityY;           // 当前速度
    float desiredVelocityX, desiredVelocityY; // 期望速度
    float mass;                           // 质量（用于碰撞分离）
    bool isStatic;                        // 是否为静态物体

public:
    // 碰撞响应相关结构体（移到public以便访问）
    struct CollisionInfo {
        Entity* other;
        float penetrationDepth;
        float normalX, normalY;
        float contactX, contactY;
    };

protected:
    std::vector<CollisionInfo> collisions;

public:
    Entity(float startX, float startY, int entityRadius, int entitySpeed, int entityHealth, SDL_Color entityColor, Faction entityFaction = Faction::NEUTRAL);
    virtual ~Entity();

    // 基本更新方法
    virtual void update(float deltaTime = 1.0f / 60.0f);

    // 渲染方法
    virtual void render(SDL_Renderer* renderer, float cameraX, float cameraY);

    // 旧的碰撞检测方法已删除，使用新的物理系统

    // 处理与其他实体的碰撞
    virtual void resolveCollision(Entity* other);

    // 受伤方法
    virtual bool takeDamage(const Damage& damage);

    // 获取器
    float getX() const { return x; }
    float getY() const { return y; }
    int getIntX() const { return static_cast<int>(round(x)); }  // 整数坐标获取器（向后兼容）
    int getIntY() const { return static_cast<int>(round(y)); }  // 整数坐标获取器（向后兼容）
    int getRadius() const { return radius; }  // 添加getRadius方法
    int getHealth() const { return health; }
    Faction getFaction() const { return faction; }
    const Collider& getCollider() const { return collider; }

    // 实体标志相关方法
    void addFlag(EntityFlag flag) { flags.insert(flag); }
    void removeFlag(EntityFlag flag) { flags.erase(flag); }
    bool hasFlag(EntityFlag flag) const { return flags.find(flag) != flags.end(); }
    const std::unordered_set<EntityFlag>& getFlags() const { return flags; }
    
    // 新增属性的获取器和设置器
    float getVolume() const { return volume; }
    void setVolume(float vol) { volume = vol; }
    
    float getWeight() const { return weight; }
    void setWeight(float w) { weight = w; }
    
    float getHeight() const { return height; }
    void setHeight(float h) { height = h; }
    
    int getSmellIntensity() const { return smellIntensity; }
    void setSmellIntensity(int intensity) { smellIntensity = std::min(100, std::max(0, intensity)); }
    
    int getSoundIntensity() const { return soundIntensity; }
    void setSoundIntensity(int intensity) { soundIntensity = std::min(100, std::max(0, intensity)); }
    
    const std::string& getSoundFile() const { return soundFile; }
    void setSoundFile(const std::string& file) { soundFile = file; }
    
    // 属性值的获取器和设置器
    int getStrength() const { return strength; }
    void setStrength(int str) { strength = str; }
    
    int getDexterity() const { return dexterity; }
    void setDexterity(int dex) { dexterity = dex; }
    
    int getPerception() const { return perception; }
    void setPerception(int per) { perception = per; }
    
    int getIntelligence() const { return intelligence; }
    void setIntelligence(int intel) { intelligence = intel; }

    // 旧的状态管理方法（将逐步替换）
    EntityState getCurrentState() const { return currentState; }
    float getStateTimer() const { return stateTimer; }
    bool isInState(EntityState state) const { return currentState == state; }
    bool canPerformAction() const; // 检查是否可以执行手上操作
    bool canMove() const; // 检查是否可以移动
    float getSpeedModifier() const { return speedModifier; } // 获取速度修正系数

    // 旧的状态设置方法（将逐步替换）
    void setState(EntityState newState, float duration);
    void updateStateTimer(float deltaTime); // 更新状态计时器

    // 新的状态管理方法
    EntityStateManager* getStateManager() const { return stateManager.get(); }
    
    // 添加状态
    EntityStateEffect* addState(
        EntityStateEffect::Type type,
        const std::string& name,
        int duration = -1,
        int priority = 0
    );
    
    // 移除状态
    bool removeState(const std::string& name);
    bool removeState(EntityStateEffect::Type type);
    
    // 检查状态是否存在
    bool hasState(const std::string& name) const;
    bool hasState(EntityStateEffect::Type type) const;
    
    // 获取状态
    EntityStateEffect* getState(const std::string& name);
    EntityStateEffect* getState(EntityStateEffect::Type type);
    
    // 获取所有状态
    const std::vector<std::unique_ptr<EntityStateEffect>>& getAllStates() const;
    
    // 清除所有状态
    void clearStates();
    
    // 根据状态更新实体属性
    void updateStateEffects();

    // 武器操作行为封装

        // 射击相关接口 - 修改现有方法
        // 向指定方向射击，返回创建的子弹（如果成功）
    Bullet* shootInDirection(Gun* weapon, float dirX, float dirY);

    // 自动选择存储空间中最合适的弹匣进行换弹
    float reloadWeaponAuto(Gun* weapon);

    // 使用行为队列系统进行换弹操作
    void reloadWeaponWithActions(Gun* weapon);

    // 删除：shoot方法，由shootInDirection替代
    // Bullet* shoot(Gun* weapon, float dirX, float dirY); // 删除此方法

// 存储空间相关方法
// 存储相关方法
    bool addItem(std::unique_ptr<Item> item);
    std::unique_ptr<Item> takeItem(Item* item);

    // 将物品存储到最大容量的存储空间中
    bool storeItemInLargestStorage(std::unique_ptr<Item> item);
    
    // 使用Action存储物品
    void storeItemWithAction(std::unique_ptr<Item> item, Storage* storage);
    
    // 使用Action取出物品
    void takeItemWithAction(Item* item, Storage* storage, std::function<void(std::unique_ptr<Item>)> callback = nullptr);
    // 取物品函数
    std::unique_ptr<Item> removeItem(EquipSlot slot, Storage* storage, int index);
    size_t getTotalItemCount() const;
    std::vector<std::tuple<EquipSlot, Storage*, int, Item*>> findItemsByCategory(ItemFlag flag) const;
    std::vector<std::tuple<EquipSlot, Storage*, int, Item*>> findItemsByName(const std::string& name) const;
    Magazine* findFullestMagazine(const std::vector<std::string>& compatibleTypes) const;
    std::unique_ptr<Magazine> removeMagazine(Magazine* magazine);

    // 装备系统相关方法
    EquipmentSystem* getEquipmentSystem() const { return equipmentSystem.get(); }

    // 穿戴物品，返回操作耗时（已弃用，请使用equipItemWithAction）
    // 注意：此方法只处理可穿戴物品，手持物品应由子类处理
    [[deprecated("已弃用，请使用equipItemWithAction")]]
    float equipItem(std::unique_ptr<Item> item);

    // 使用Action穿戴物品
    // 注意：此方法只处理可穿戴物品，手持物品应由子类处理
    void equipItemWithAction(std::unique_ptr<Item> item);

    // 卸下物品，返回操作耗时和卸下的物品
    std::pair<float, std::unique_ptr<Item>> unequipItem(EquipSlot slot);
    
    // 按物品卸下装备，返回操作耗时和卸下的物品
    std::pair<float, std::unique_ptr<Item>> unequipItem(Item* item);
    
    // 使用Action卸下装备
    void unequipItemWithAction(EquipSlot slot, std::function<void(std::unique_ptr<Item>)> callback = nullptr);
    
    // 使用Action按物品卸下装备
    void unequipItemWithAction(Item* item, std::function<void(std::unique_ptr<Item>)> callback = nullptr);

    // 获取所有已装备物品的存储空间列表
    std::vector<std::pair<EquipSlot, Storage*>> getAllAvailableStorages() const;

    // 获取所有存储空间的总重量
    float getTotalStorageWeight() const;

    bool canShoot(Gun* weapon) const; // 检查是否可以射击
    bool needsReload(Gun* weapon) const; // 检查是否需要换弹

    // 自动武器维护方法
    void maintainWeapon(Gun* weapon); // 自动维护武器状态（上膛、换弹等）

    // 获取武器状态信息
    std::string getWeaponStatus(Gun* weapon) const; // 获取武器状态描述

    // 射击冷却相关方法
    int getShootCooldown() const { return shootCooldown; }
    void setShootCooldown(int cooldown) { shootCooldown = cooldown; }
    bool canShootByCooldown() const { return shootCooldown <= 0; }
    void updateShootCooldown(); // 更新射击冷却计时器

    // 行为队列相关方法
    ActionQueue* getActionQueue() const { return actionQueue.get(); }
    void addAction(std::unique_ptr<Action> action); // 添加行为到队列
    void clearActions(); // 清空行为队列
    bool hasActiveActions() const; // 检查是否有活动的行为

    // 使用Action在两个存储空间之间转移物品
    void transferItemWithAction(Item* item, Storage* sourceStorage, Storage* targetStorage, std::function<void(bool)> callback = nullptr);

    // 物理引擎相关方法
    void setVelocity(float vx, float vy) { velocityX = vx; velocityY = vy; }
    void addVelocity(float vx, float vy) { velocityX += vx; velocityY += vy; }
    void setDesiredVelocity(float vx, float vy) { desiredVelocityX = vx; desiredVelocityY = vy; }
    
    float getVelocityX() const { return velocityX; }
    float getVelocityY() const { return velocityY; }
    float getMass() const { return mass; }
    void setMass(float m) { mass = m; }
    
    bool getIsStatic() const { return isStatic; }
    void setIsStatic(bool static_) { isStatic = static_; }
    
    // 物理更新方法
    void updatePhysics(float deltaTime);
    void applyForce(float forceX, float forceY);
    
    // 碰撞检测和响应
    bool checkCollisionWith(Entity* other, CollisionInfo& info);
    void separateFromEntity(Entity* other, const CollisionInfo& info);
    void separateFromTerrain(float normalX, float normalY, float penetration);
    void checkTerrainCollisionAtPosition(float& newX, float& newY);
    void resolveTerrainCollision(float& newX, float& newY, const Collider& terrainCollider);
    
    // 推开能力计算
    float calculatePushPower() const;      // 计算推开他人的能力
    float calculatePushResistance() const; // 计算抗推能力（被推开的难度）
    
    // 便捷的属性配置方法
    void setPhysicalAttributes(float entityWeight, int entityStrength, int entityDexterity) {
        weight = entityWeight;
        strength = entityStrength;
        dexterity = entityDexterity;
    }
};

#endif // ENTITY_H