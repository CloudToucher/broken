#pragma once
#ifndef MAP_H
#define MAP_H

#include <SDL3/SDL.h>
#include <vector>
#include <queue>
#include <memory>
#include <unordered_map>
#include <string>
#include <filesystem>
#include "Collider.h"
#include "Grid.h"
#include "Tile.h"

// 网格坐标结构体，用于哈希表键值
struct GridCoord {
    int x, y;
    
    bool operator==(const GridCoord& other) const {
        return x == other.x && y == other.y;
    }
};

// 为GridCoord提供哈希函数
namespace std {
    template<>
    struct hash<GridCoord> {
        size_t operator()(const GridCoord& coord) const {
            return hash<int>()(coord.x) ^ (hash<int>()(coord.y) << 1);
        }
    };
}

class Map {
private:
    std::vector<Collider> obstacles;                                // 障碍物列表
    std::unordered_map<GridCoord, std::unique_ptr<Grid>> grids;    // 网格哈希表
    int loadDistance;                                              // 加载距离（网格数）
    int playerGridX, playerGridY;                                  // 玩家所在的网格坐标
    std::string mapDir;                                            // 地图文件目录
    SDL_Renderer* renderer;                                        // 渲染器引用
    // 异步加载相关
    std::queue<GridCoord> gridsToLoadAsync;
    std::mutex loadQueueMutex;
    int maxGridsPerFrame; // 每帧最多加载的网格数

    // 获取网格文件路径
    std::string getGridFilePath(int gridX, int gridY) const;
    
    // 检查网格是否需要加载
    bool isGridInLoadRange(int gridX, int gridY) const;
    
    // 生成新网格
    std::unique_ptr<Grid> generateNewGrid(int gridX, int gridY);
    
    // 封存网格到文件
    void archiveGrid(int gridX, int gridY, Grid* grid);
    
    // 从文件加载网格
    std::unique_ptr<Grid> loadGridFromFile(int gridX, int gridY);
    
    // 更新障碍物列表
    void updateObstacles();

public:
    Map(SDL_Renderer* renderer, int loadDist = 4);
    ~Map();

    // 更新方法，每帧调用
    void update();

    void render(SDL_Renderer* renderer, float cameraX, float cameraY);
    
    // 获取障碍物列表
    const std::vector<Collider>& getObstacles() const { return obstacles; }

    // 添加网格到地图
    void addGrid(std::unique_ptr<Grid> grid, int gridX, int gridY);
    
    // 获取指定世界坐标的网格
    Grid* getGridAt(float worldX, float worldY) const;
    
    // 获取指定网格坐标的网格
    Grid* getGridAtCoord(int gridX, int gridY) const;
    
    // 获取指定世界坐标的方块
    Tile* getTileAt(float worldX, float worldY) const;
    
    // 更新玩家位置，触发网格加载/卸载
    void updatePlayerPosition(float worldX, float worldY);
    
    // 初始化地图（生成初始网格）
    void initialize();
    
    // 将世界坐标转换为网格坐标
    static void worldToGridCoord(float worldX, float worldY, int& gridX, int& gridY);
    
    // 将网格坐标转换为世界坐标（网格左下角）
    static void gridCoordToWorld(int gridX, int gridY, int& worldX, int& worldY);
    
    // 新增：强制更新障碍物列表（公有接口）
    void forceUpdateObstacles() { updateObstacles(); }
};

#endif // MAP_H
