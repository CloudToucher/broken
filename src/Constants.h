#pragma once
#ifndef CONSTANTS_H
#define CONSTANTS_H

// 游戏核心常量
namespace GameConstants {
    // 瓦片/格子相关常量
    constexpr int TILE_SIZE = 64;                    // 方块大小（像素）
    constexpr int DEFAULT_GRID_SIZE = 16;            // 默认网格大小（方块数）
    constexpr int MAP_GRID_SIZE = 24;                // 地图网格大小（方块数）
    
    // 计算得出的常量
    constexpr int TOTAL_GRID_SIZE = DEFAULT_GRID_SIZE * TILE_SIZE;  // 网格总大小（像素）
    constexpr int MAP_TOTAL_GRID_SIZE = MAP_GRID_SIZE * TILE_SIZE;  // 地图网格总大小（像素）
    
    // 渲染相关常量
    constexpr float TILE_CENTER_OFFSET = TILE_SIZE / 2.0f;          // 瓦片中心偏移
    
    // 感知系统常量（以格为单位）
    constexpr float PIXELS_PER_GRID = TILE_SIZE;                    // 每格像素数
    
    // 距离计算辅助函数
    inline float gridsToPixels(float grids) {
        return grids * TILE_SIZE;
    }
    
    inline float pixelsToGrids(float pixels) {
        return pixels / TILE_SIZE;
    }
    
    inline int worldToTileCoord(float worldCoord) {
        return static_cast<int>(worldCoord / TILE_SIZE);
    }
    
    inline float tileCoordToWorld(int tileCoord) {
        return tileCoord * TILE_SIZE;
    }
}

#endif // CONSTANTS_H 