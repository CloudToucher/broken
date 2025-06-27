#pragma once
#ifndef TILE_H
#define TILE_H

#include <SDL3/SDL.h>
#include <string>
#include "Collider.h"
#include "Constants.h"
#include <thread>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <memory>

// 方块旋转角度枚举
enum class TileRotation {
    ROTATION_0 = 0,    // 0度
    ROTATION_90 = 90,  // 90度
    ROTATION_180 = 180,// 180度
    ROTATION_270 = 270 // 270度
};

class Tile {
private:
    static std::unordered_map<std::string, SDL_Texture*> textureCache;
    static std::mutex textureCacheMutex;
    std::string name;          // 方块名称
    std::string texturePath;   // 贴图路径
    SDL_Texture* texture;      // 贴图
    bool hasCollision;         // 是否有碰撞箱
    bool isTransparent;        // 是否可视野穿透
    bool isDestructible;       // 是否可破坏
    TileRotation rotation;     // 旋转角度
    float moveCost;            // 新增：移动耗时倍数，默认100（平地）
    
    // 重构：支持多种用途的碰撞箱
    std::vector<std::unique_ptr<Collider>> colliders; // 所有碰撞箱
    
    int x, y;                  // 方块位置（世界坐标）
    int size;                  // 方块大小（默认64像素）
    bool textureFromCache;     // 标记纹理是否来自缓存

public:
    // 构造函数
    Tile(const std::string& tileName, const std::string& texPath, bool collision, 
         bool transparent, bool destructible, int posX, int posY, int tileSize = GameConstants::TILE_SIZE, float tileMoveCost = 100.0f);
    ~Tile();

    static void clearTextureCache();
    
    // 初始化贴图，返回是否成功初始化
    bool initializeTexture(SDL_Renderer* renderer);

    // 渲染方块
    void render(SDL_Renderer* renderer, int cameraX, int cameraY);

    // 设置旋转角度
    void setRotation(TileRotation newRotation);

    // 获取方块属性
    const std::string& getName() const { return name; }
    bool getHasCollision() const { return hasCollision; }
    bool getIsTransparent() const { return isTransparent; }
    bool getIsDestructible() const { return isDestructible; }
    TileRotation getRotation() const { return rotation; }
    
    // 新增：获取和设置移动耗时
    float getMoveCost() const { return moveCost; }
    void setMoveCost(float cost) { moveCost = std::max(1.0f, cost); } // 最小为1
    
    // 重构：碰撞箱管理方法
    void addCollider(std::unique_ptr<Collider> collider);
    void removeCollider(size_t index);
    void clearColliders();
    
    // 获取所有碰撞箱
    const std::vector<std::unique_ptr<Collider>>& getColliders() const { return colliders; }
    
    // 按用途获取碰撞箱
    std::vector<Collider*> getCollidersByPurpose(ColliderPurpose purpose) const;
    
    // 检查是否有指定用途的碰撞箱
    bool hasColliderWithPurpose(ColliderPurpose purpose) const;
    
    // 便捷方法：添加地形碰撞箱
    void addTerrainCollider();
    
    // 便捷方法：添加视线碰撞箱（如果不透明）
    void addVisionCollider();
    
    // 便捷方法：移除指定用途的所有碰撞箱
    void removeCollidersByPurpose(ColliderPurpose purpose);
    
    int getX() const { return x; }
    int getY() const { return y; }
    int getSize() const { return size; }

    // 设置方块位置
    void setPosition(int posX, int posY);
    
    // 调试用：渲染所有碰撞箱
    void renderColliders(SDL_Renderer* renderer, float cameraX, float cameraY) const;
};

#endif // TILE_H