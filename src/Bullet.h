#pragma once
#ifndef BULLET_H
#define BULLET_H

#include "Collider.h"
#include "Damage.h"
#include <SDL3/SDL.h>
#include <vector>

class Entity; // 前向声明

class Bullet {
private:
    float x, y;           // 当前位置
    float prevX, prevY;   // 上一帧位置
    float dirX, dirY;     // 方向
    float speed;          // 速度
    bool active;          // 是否活跃
    Entity* owner;        // 子弹所有者
    float angle;          // 子弹飞行角度（弧度）
    float maxRange;       // 最大射程
    float traveledDistance; // 已飞行距离
    Damage damage;        // 子弹造成的伤害

    // 线段检测相关方法
    bool checkLineCircleCollision(float x1, float y1, float x2, float y2,
                                float cx, float cy, float r, float& outT) const;
    bool checkLineRectCollision(float x1, float y1, float x2, float y2,
                              float rx, float ry, float rw, float rh, float& outT) const;
    void handleCollision(float collisionT);

public:
    Bullet(float startX, float startY, float dx, float dy, float s, 
           Entity* bulletOwner, int damageValue, const std::string& damageType = "shooting", 
           int penetration = -1, float range = 1000.0f);
    ~Bullet() = default;
    
    void update(float deltaTime = 1.0f / 60.0f);
    void render(SDL_Renderer* renderer, int cameraX, int cameraY);
    
    // 检查与障碍物的碰撞
    bool checkObstacleCollisions(const std::vector<Collider>& obstacles);
    bool checkEntityCollisions(const std::vector<Entity*>& entities);
    
    // 获取器
    float getX() const { return x; }
    float getY() const { return y; }
    float getDirX() const { return dirX; }
    float getDirY() const { return dirY; }
    bool isActive() const { return active; }
    const Damage& getDamage() const { return damage; }
    Entity* getOwner() const { return owner; }
    
    // 设置器
    void setActive(bool state) { active = state; }
    
    // 添加伤害
    void addDamage(const std::string& type, int amount, int penetration = -1);
    void addDamage(DamageType type, int amount, int penetration = -1);
};

#endif // BULLET_H