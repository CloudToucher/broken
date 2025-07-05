#pragma once
#ifndef FRAGMENT_H
#define FRAGMENT_H

#include "Collider.h"
#include "Damage.h"
#include <SDL3/SDL.h>
#include <memory>
#include <vector>

class Entity; // 前向声明

// 弹片类
class Fragment {
private:
    float x, y;                      // 当前位置
    float startX, startY;            // 起始位置
    float dirX, dirY;                // 飞行方向
    float speed;                     // 飞行速度
    float maxRange;                  // 最大飞行距离
    float traveledDistance;          // 已飞行距离
    bool active;                     // 是否活跃
    
    Damage damage;                   // 弹片造成的伤害
    Entity* owner;                   // 弹片来源（爆炸源或攻击者）
    
    // 渲染属性
    SDL_Color color;                 // 弹片颜色
    float size;                      // 弹片大小
    float lifetime;                  // 生存时间
    float maxLifetime;               // 最大生存时间
    
    // 物理属性
    float gravity;                   // 重力影响（模拟弹片下坠）
    float velocityY;                 // Y轴速度（用于重力计算）
    float drag;                      // 空气阻力
    
public:
    // 构造函数
    Fragment(float startX, float startY, float dirX, float dirY, 
             float speed, float range, int damageValue, Entity* fragmentOwner);
    
    // 析构函数
    ~Fragment() = default;
    
    // 更新弹片状态
    void update(float deltaTime);
    
    // 渲染弹片
    void render(SDL_Renderer* renderer, int cameraX, int cameraY);
    
    // 碰撞检测
    bool checkTerrainCollision(const std::vector<Collider>& terrainColliders);
    bool checkEntityCollision(const std::vector<Entity*>& entities);
    
    // 获取器
    float getX() const { return x; }
    float getY() const { return y; }
    bool isActive() const { return active; }
    const Damage& getDamage() const { return damage; }
    Entity* getOwner() const { return owner; }
    float getTraveledDistance() const { return traveledDistance; }
    float getMaxRange() const { return maxRange; }
    
    // 设置器
    void setColor(SDL_Color newColor) { color = newColor; }
    void setSize(float newSize) { size = newSize; }
    void setGravity(float newGravity) { gravity = newGravity; }
    void setDrag(float newDrag) { drag = newDrag; }
    void deactivate() { active = false; }
    
    // 实用方法
    float getDistanceToTarget(float targetX, float targetY) const;
    bool isInRange(float targetX, float targetY, float range) const;
    
private:
    // 内部方法
    void applyPhysics(float deltaTime);
    bool checkPointCollision(float x, float y, const Collider& collider) const;
    void handleCollision();
};

// 弹片管理器类
class FragmentManager {
private:
    std::vector<std::unique_ptr<Fragment>> fragments;
    static FragmentManager* instance;
    
    FragmentManager() = default;
    
public:
    // 单例访问
    static FragmentManager& getInstance();
    static void destroyInstance();
    
    // 禁用拷贝构造和赋值
    FragmentManager(const FragmentManager&) = delete;
    FragmentManager& operator=(const FragmentManager&) = delete;
    
    ~FragmentManager() = default;
    
    // 弹片管理
    void addFragment(std::unique_ptr<Fragment> fragment);
    void update(float deltaTime);                                  // 更新所有弹片
    void render(SDL_Renderer* renderer, int cameraX, int cameraY); // 渲染所有弹片
    void updateFragments(float deltaTime);
    void renderFragments(SDL_Renderer* renderer, int cameraX, int cameraY);
    void clearInactiveFragments();
    void clearAllFragments();
    
    // 碰撞检测
    void checkFragmentCollisions(const std::vector<Collider>& terrainColliders, 
                               const std::vector<Entity*>& entities);
    
    // 查询方法
    size_t getActiveFragmentCount() const;
    bool hasActiveFragments() const;
    
    // 批量创建弹片
    void createExplosionFragments(float centerX, float centerY, int fragmentCount, 
                                float minSpeed, float maxSpeed, float range, 
                                int damagePerFragment, Entity* owner);
    
    // 调试方法
    void debugPrintFragmentInfo() const;
};

#endif // FRAGMENT_H 