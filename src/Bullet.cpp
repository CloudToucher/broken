#include "Bullet.h"
#include "Entity.h"
#include "Constants.h"
#include <cmath>
#include <algorithm>

// 修改 Bullet 构造函数
Bullet::Bullet(float startX, float startY, float dx, float dy, float s, 
    Entity* bulletOwner, int damageValue, const std::string& damageType, int penetration, float range)
: x(startX), y(startY), prevX(startX), prevY(startY),
  dirX(dx), dirY(dy), speed(s), active(true), 
  owner(bulletOwner),
  maxRange(range * GameConstants::TILE_SIZE),
  traveledDistance(0.0f),
  damage(bulletOwner) {
    // 添加伤害
    damage.addDamage(damageType, damageValue, penetration);
    
    // 计算子弹飞行角度
    angle = std::atan2(dy, dx);
}

// 添加伤害方法（字符串类型）
void Bullet::addDamage(const std::string& type, int amount, int penetration) {
    damage.addDamage(type, amount, penetration);
}

// 添加伤害方法（枚举类型）
void Bullet::addDamage(DamageType type, int amount, int penetration) {
    damage.addDamage(type, amount, penetration);
}

void Bullet::update(float deltaTime) {
    if (!active) return;
    
    // 保存上一帧位置
    prevX = x;
    prevY = y;
    
    // 更新位置
    float moveX = dirX * speed * deltaTime * 60;
    float moveY = dirY * speed * deltaTime * 60;
    x += moveX;
    y += moveY;
    
    // 计算本次移动距离并累加到总飞行距离
    float distanceMoved = std::sqrt(moveX * moveX + moveY * moveY);
    traveledDistance += distanceMoved;
    
    // 检查是否超过最大射程
    if (traveledDistance >= maxRange) {
        active = false;
        return;
    }
}

void Bullet::render(SDL_Renderer* renderer, int cameraX, int cameraY) {
    if (!active) return;
    
    // 计算屏幕坐标
    float screenX = x - cameraX;
    float screenY = y - cameraY;
    
    // 设置线段宽度为3像素
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // 黄色子弹
    
    // 绘制一条较短的线段（长度为50像素）
    float endX = screenX + dirX * 50;
    float endY = screenY + dirY * 50;
    
    // 绘制主线段
    SDL_RenderLine(renderer, 
                  static_cast<int>(screenX), static_cast<int>(screenY),
                  static_cast<int>(endX), static_cast<int>(endY));
    
    // 绘制额外的线段来增加宽度
    SDL_RenderLine(renderer, 
                  static_cast<int>(screenX - 1), static_cast<int>(screenY - 1),
                  static_cast<int>(endX - 1), static_cast<int>(endY - 1));
    SDL_RenderLine(renderer, 
                  static_cast<int>(screenX + 1), static_cast<int>(screenY + 1),
                  static_cast<int>(endX + 1), static_cast<int>(endY + 1));
}

bool Bullet::checkLineCircleCollision(float x1, float y1, float x2, float y2,
                                    float cx, float cy, float r, float& outT) const {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float fx = x1 - cx;
    float fy = y1 - cy;

    float a = dx*dx + dy*dy;
    float b = 2 * (fx*dx + fy*dy);
    float c = (fx*fx + fy*fy) - r*r;

    float discriminant = b*b - 4*a*c;
    if (discriminant < 0) return false;

    discriminant = std::sqrt(discriminant);
    float t1 = (-b - discriminant) / (2*a);
    float t2 = (-b + discriminant) / (2*a);

    bool hit = false;
    outT = 1.1f; // 超出线段范围
    if (t1 >= 0 && t1 <= 1) { outT = t1; hit = true; }
    if (t2 >= 0 && t2 <= 1 && t2 < outT) { outT = t2; hit = true; }
    return hit;
}

bool Bullet::checkLineRectCollision(float x1, float y1, float x2, float y2,
                                  float rx, float ry, float rw, float rh, float& outT) const {
    float tmin = 0.0f, tmax = 1.0f;
    float dx = x2 - x1, dy = y2 - y1;

    // X轴
    if (std::abs(dx) < 1e-8) {
        if (x1 < rx || x1 > rx + rw) return false;
    } else {
        float ood = 1.0f / dx;
        float t1 = (rx - x1) * ood;
        float t2 = (rx + rw - x1) * ood;
        if (t1 > t2) std::swap(t1, t2);
        tmin = std::max(tmin, t1);
        tmax = std::min(tmax, t2);
        if (tmin > tmax) return false;
    }

    // Y轴
    if (std::abs(dy) < 1e-8) {
        if (y1 < ry || y1 > ry + rh) return false;
    } else {
        float ood = 1.0f / dy;
        float t1 = (ry - y1) * ood;
        float t2 = (ry + rh - y1) * ood;
        if (t1 > t2) std::swap(t1, t2);
        tmin = std::max(tmin, t1);
        tmax = std::min(tmax, t2);
        if (tmin > tmax) return false;
    }

    if (tmin >= 0.0f && tmin <= 1.0f) {
        outT = tmin;
        return true;
    }
    return false;
}

void Bullet::handleCollision(float collisionT) {
    // 将子弹位置调整到碰撞点
    x = prevX + (x - prevX) * collisionT;
    y = prevY + (y - prevY) * collisionT;
    active = false;
}

bool Bullet::checkObstacleCollisions(const std::vector<Collider>& obstacles) {
    if (!active) return false;
    
    float minT = 1.1f;
    bool hasCollision = false;
    
    for (const auto& obstacle : obstacles) {
        float t;
        bool hit = false;
        
        if (obstacle.getType() == ColliderType::CIRCLE) {
            hit = checkLineCircleCollision(prevX, prevY, x, y,
                                         obstacle.getCircleX(), obstacle.getCircleY(),
                                         obstacle.getRadius(), t);
        } else {
            const SDL_FRect& box = obstacle.getBoxCollider();
            hit = checkLineRectCollision(prevX, prevY, x, y,
                                       box.x, box.y, box.w, box.h, t);
        }
        
        if (hit && t < minT) {
            minT = t;
            hasCollision = true;
        }
    }
    
    if (hasCollision) {
        handleCollision(minT);
        return true;
    }
    return false;
}

bool Bullet::checkEntityCollisions(const std::vector<Entity*>& entities) {
    if (!active) return false;
    
    float minT = 1.1f;
    Entity* hitEntity = nullptr;
    
    for (auto* entity : entities) {
        // 不检测与自己的碰撞
        if (entity == owner) continue;
        
        // 不检测与同阵营实体的碰撞（除非是中立阵营）
        if (owner && entity->getFaction() == owner->getFaction() && 
            owner->getFaction() != Faction::NEUTRAL) {
            continue;
        }
        
        const Collider& entityCollider = entity->getCollider();
        float t;
        bool hit = false;
        
        if (entityCollider.getType() == ColliderType::CIRCLE) {
            hit = checkLineCircleCollision(prevX, prevY, x, y,
                                         entityCollider.getCircleX(), entityCollider.getCircleY(),
                                         entityCollider.getRadius(), t);
        } else {
            const SDL_FRect& box = entityCollider.getBoxCollider();
            hit = checkLineRectCollision(prevX, prevY, x, y,
                                       box.x, box.y, box.w, box.h, t);
        }
        
        if (hit && t < minT) {
            minT = t;
            hitEntity = entity;
        }
    }
    
    if (hitEntity) {
        handleCollision(minT);
        hitEntity->takeDamage(damage);
        return true;
    }
    return false;
}