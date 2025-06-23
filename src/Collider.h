#pragma once
#ifndef COLLIDER_H
#define COLLIDER_H

#include <SDL3/SDL.h>
#include <string>

enum class ColliderType {
    BOX,
    CIRCLE
};

// 新增：碰撞箱用途类型
enum class ColliderPurpose {
    ENTITY,         // 实体碰撞箱 - 由实体诞生，用于实体间碰撞检测
    TERRAIN,        // 地形碰撞箱 - 由tile控制，用于地形阻挡
    VISION          // 视线碰撞箱 - 由tile控制，用于视线检测和事件（如烟雾）
};

class Collider {
private:
    ColliderType type;
    ColliderPurpose purpose;      // 新增：碰撞箱用途
    SDL_FRect boxCollider;        // 矩形碰撞箱
    float circleX, circleY;       // 圆形碰撞体中心
    float radius;                 // 圆形碰撞体半径
    std::string tag;              // 碰撞体标签（如"player", "enemy", "bullet"等）
    bool isActive;                // 新增：是否激活
    int layer;                    // 新增：碰撞层级（用于分层检测）
    
public:
    // 创建矩形碰撞体
    Collider(float x, float y, float width, float height, 
             const std::string& colliderTag, 
             ColliderPurpose colliderPurpose = ColliderPurpose::ENTITY,
             int colliderLayer = 0);
    
    // 创建圆形碰撞体
    Collider(float x, float y, float r, 
             const std::string& colliderTag,
             ColliderPurpose colliderPurpose = ColliderPurpose::ENTITY,
             int colliderLayer = 0);
    
    // 更新碰撞体位置
    void updatePosition(float x, float y);
    
    // 检测与另一个碰撞体的碰撞
    bool checkCollision(const Collider& other) const;
    
    // 新增：检测与指定用途的碰撞体的碰撞
    bool checkCollisionWithPurpose(const Collider& other, ColliderPurpose targetPurpose) const;
    
    // 获取碰撞体标签
    const std::string& getTag() const { return tag; }
    
    // 获取碰撞体类型
    ColliderType getType() const { return type; }
    
    // 新增：获取碰撞体用途
    ColliderPurpose getPurpose() const { return purpose; }
    void setPurpose(ColliderPurpose newPurpose) { purpose = newPurpose; }
    
    // 新增：获取和设置激活状态
    bool getIsActive() const { return isActive; }
    void setIsActive(bool active) { isActive = active; }
    
    // 新增：获取和设置层级
    int getLayer() const { return layer; }
    void setLayer(int newLayer) { layer = newLayer; }
    
    // 获取矩形碰撞体数据
    const SDL_FRect& getBoxCollider() const { return boxCollider; }
    
    // 获取圆形碰撞体数据
    float getCircleX() const { return circleX; }
    float getCircleY() const { return circleY; }
    float getRadius() const { return radius; }
    
    // 新增：获取碰撞体位置（统一接口）
    float getX() const { 
        return (type == ColliderType::CIRCLE) ? circleX : boxCollider.x + boxCollider.w / 2; 
    }
    float getY() const { 
        return (type == ColliderType::CIRCLE) ? circleY : boxCollider.y + boxCollider.h / 2; 
    }
    
    // 新增：获取碰撞箱尺寸（统一接口）
    float getWidth() const;
    float getHeight() const;
    
    // 新增：检查是否与另一个碰撞箱相交（intersects方法）
    bool intersects(const Collider& other) const;
    
    // 调试用：渲染碰撞体（不同用途用不同颜色）
    void render(SDL_Renderer* renderer, float cameraX, float cameraY) const;
    
    // 检查点是否在碰撞体内部
    bool contains(int x, int y) const;
    bool contains(float x, float y) const;
    
    // 射线检测方法
    float raycast(float startX, float startY, float dirX, float dirY) const;
    
    // 新增：射线检测（只检测指定用途的碰撞体）
    float raycastWithPurpose(float startX, float startY, float dirX, float dirY, ColliderPurpose targetPurpose) const;
};

#endif // COLLIDER_H