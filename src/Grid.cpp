#include "Grid.h"
#include <iostream>

Grid::Grid(const std::string& gridName, int posX, int posY, int gSize, int tSize)
    : name(gridName),
      x(posX),
      y(posY),
      gridSize(gSize),
      tileSize(tSize) {
    // 初始化方块数组 - 修复unique_ptr拷贝构造函数错误
    tiles.resize(gridSize);
    for (auto& row : tiles) {
        row.resize(gridSize);
    }
}

void Grid::addTile(std::unique_ptr<Tile> tile, int gridX, int gridY) {
    // 检查坐标是否有效
    if (gridX < 0 || gridX >= gridSize || gridY < 0 || gridY >= gridSize) {
        std::cerr << "无效的网格坐标: (" << gridX << ", " << gridY << ")" << std::endl;
        return;
    }
    
    // 计算方块的世界坐标
    int worldX = x + gridX * tileSize;
    int worldY = y + gridY * tileSize;
    
    // 设置方块位置
    tile->setPosition(worldX, worldY);
    
    // 添加方块到网格
    tiles[gridY][gridX] = std::move(tile);
}

Tile* Grid::getTile(int gridX, int gridY) const {
    // 检查坐标是否有效
    if (gridX < 0 || gridX >= gridSize || gridY < 0 || gridY >= gridSize) {
        return nullptr;
    }
    
    return tiles[gridY][gridX].get();
}

void Grid::initializeTextures(SDL_Renderer* renderer) {
    // std::cout << "初始化网格纹理: " << name << " 位置(" << x << ", " << y << ")" << std::endl;
    
    // 统计初始化的方块数量
    int initializedCount = 0;
    int totalTiles = 0;
    
    // 单线程初始化所有方块的纹理
    for (int y = 0; y < gridSize; ++y) {
        for (int x = 0; x < gridSize; ++x) {
            if (tiles[y][x]) {
                totalTiles++;
                // 确保方块的纹理被初始化
                if (tiles[y][x]->initializeTexture(renderer)) {
                    initializedCount++;
                }
            }
        }
    }
    
    // std::cout << "网格纹理初始化完成: " << name << "，成功初始化 " << initializedCount << "/" << totalTiles << " 个方块纹理" << std::endl;
}

void Grid::render(SDL_Renderer* renderer, int cameraX, int cameraY) {
    // 渲染所有方块
    for (int y = 0; y < gridSize; ++y) {
        for (int x = 0; x < gridSize; ++x) {
            if (tiles[y][x]) {
                tiles[y][x]->render(renderer, cameraX, cameraY);
            }
        }
    }
}

std::vector<Collider> Grid::getColliders() const {
    std::vector<Collider> colliders;
    
    // 收集所有有碰撞的方块的碰撞箱
    for (int y = 0; y < gridSize; ++y) {
        for (int x = 0; x < gridSize; ++x) {
            if (tiles[y][x] && tiles[y][x]->getHasCollision()) {
                // 使用新的碰撞箱系统获取地形碰撞箱
                std::vector<Collider*> terrainColliders = tiles[y][x]->getCollidersByPurpose(ColliderPurpose::TERRAIN);
                for (const auto& collider : terrainColliders) {
                    if (collider) {
                        colliders.push_back(*collider);
                    }
                }
            }
        }
    }
    
    return colliders;
}

void Grid::setPosition(int posX, int posY) {
    // 计算偏移量
    int offsetX = posX - x;
    int offsetY = posY - y;
    
    // 更新网格位置
    x = posX;
    y = posY;
    
    // 更新所有方块的位置
    for (int y = 0; y < gridSize; ++y) {
        for (int x = 0; x < gridSize; ++x) {
            if (tiles[y][x]) {
                int tileX = tiles[y][x]->getX() + offsetX;
                int tileY = tiles[y][x]->getY() + offsetY;
                tiles[y][x]->setPosition(tileX, tileY);
            }
        }
    }
}

std::unique_ptr<Grid> Grid::createGrasslandGrid(int posX, int posY, int gSize, int tSize) {
    std::cout << "创建草地网格: 位置(" << posX << ", " << posY << "), 大小: " << gSize << "x" << gSize << ", 方块大小: " << tSize << std::endl;
    
    // 创建一个新的网格
    auto grid = std::make_unique<Grid>("GrasslandGrid", posX, posY, gSize, tSize);
    
    // 填充草地方块
    for (int y = 0; y < gSize; ++y) {
        for (int x = 0; x < gSize; ++x) {
            // 创建草地方块
            // 参数：名称、贴图路径、碰撞、透视、可破坏、位置X、位置Y、大小
            auto tile = std::make_unique<Tile>(
                "Grassland",
                "assets/tiles/grassland.bmp",
                false,  // 没有碰撞箱
                true,   // 完全可透视
                false,  // 不可破坏
                0, 0,   // 位置会在addTile中设置
                tSize
            );
            
            // 添加到网格
            grid->addTile(std::move(tile), x, y);
        }
    }
    
    std::cout << "草地网格创建完成，共 " << gSize * gSize << " 个方块" << std::endl;
    return grid;
}