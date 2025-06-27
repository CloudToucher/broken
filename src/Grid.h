#pragma once
#ifndef GRID_H
#define GRID_H

#include <SDL3/SDL.h>
#include <string>
#include <vector>
#include <memory>
#include "Tile.h"
#include "Constants.h"

class Grid {
private:
    std::string name;                              // 网格名称
    std::vector<std::vector<std::unique_ptr<Tile>>> tiles; // 16x16的方块数组
    int x, y;                                      // 网格位置（世界坐标）
    int gridSize;                                  // 网格大小（默认16）
    int tileSize;                                  // 方块大小（默认64像素）

public:
    // 构造函数
    Grid(const std::string& gridName, int posX, int posY, int gSize = GameConstants::DEFAULT_GRID_SIZE, int tSize = GameConstants::TILE_SIZE);
    
    // 添加方块到网格
    void addTile(std::unique_ptr<Tile> tile, int gridX, int gridY);
    
    // 获取网格中的方块
    Tile* getTile(int gridX, int gridY) const;
    
    // 获取网格中所有方块的二维数组（用于保存和加载）
    const std::vector<std::vector<std::unique_ptr<Tile>>>& getTiles() const { return tiles; }
    
    // 初始化所有方块的贴图
    void initializeTextures(SDL_Renderer* renderer);
    
    // 渲染网格
    void render(SDL_Renderer* renderer, int cameraX, int cameraY);
    
    // 获取网格属性
    const std::string& getName() const { return name; }
    int getX() const { return x; }
    int getY() const { return y; }
    int getGridSize() const { return gridSize; }
    int getTileSize() const { return tileSize; }
    int getTotalSize() const { return gridSize * tileSize; } // 网格总大小（像素）
    
    // 获取网格中所有方块的碰撞箱
    std::vector<Collider> getColliders() const;
    
    // 设置网格位置
    void setPosition(int posX, int posY);
    
    // 创建一个全是草地的网格
    static std::unique_ptr<Grid> createGrasslandGrid(int posX, int posY, int gSize = GameConstants::DEFAULT_GRID_SIZE, int tSize = GameConstants::TILE_SIZE);
};

#endif // GRID_H