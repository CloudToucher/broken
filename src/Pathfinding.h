#pragma once
#ifndef PATHFINDING_H
#define PATHFINDING_H

#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <functional>

// 前向声明
class Map;
class Tile;

// 路径节点结构
struct PathNode {
    int x, y;                    // 网格坐标
    float gCost;                 // 从起点到当前节点的实际代价
    float hCost;                 // 从当前节点到终点的启发式代价
    float fCost;                 // 总代价 (gCost + hCost)
    PathNode* parent;            // 父节点指针
    
    PathNode(int posX, int posY) : x(posX), y(posY), gCost(0), hCost(0), fCost(0), parent(nullptr) {}
    
    // 用于优先队列的比较
    bool operator>(const PathNode& other) const {
        return fCost > other.fCost;
    }
};

// 路径点结构（世界坐标）
struct PathPoint {
    float x, y;                  // 世界坐标
    float moveCost;              // 移动到此点的耗时倍数
    
    PathPoint(float posX, float posY, float cost = 1.0f) : x(posX), y(posY), moveCost(cost) {}
};

// 寻路结果枚举
enum class PathfindingResult {
    SUCCESS,                     // 成功找到路径
    NO_PATH,                     // 无法找到路径（智能不足）
    TARGET_UNREACHABLE,          // 目标不可达
    START_BLOCKED,               // 起点被阻挡
    TARGET_BLOCKED               // 终点被阻挡
};

// 寻路请求结构
struct PathfindingRequest {
    int startX, startY;          // 起点网格坐标
    int targetX, targetY;        // 终点网格坐标
    float intelligence;          // 寻路智能程度 (1.2-8.0)
    
    PathfindingRequest(int sX, int sY, int tX, int tY, float intel) 
        : startX(sX), startY(sY), targetX(tX), targetY(tY), intelligence(intel) {
        // 现在使用基于距离的智能限制，不再需要maxIterations
    }
};

// A*寻路算法类
class AStar {
private:
    Map* map;                    // 地图引用
    
    // 节点哈希函数
    struct NodeHash {
        size_t operator()(const std::pair<int, int>& pos) const {
            return std::hash<int>()(pos.first) ^ (std::hash<int>()(pos.second) << 1);
        }
    };
    
    // 计算启发式距离（欧几里得距离）
    float calculateHeuristic(int x1, int y1, int x2, int y2) const;
    
    // 获取邻居节点（8方向）
    std::vector<std::pair<int, int>> getNeighbors(int x, int y) const;
    
    // 检查节点是否可通行
    bool isWalkable(int x, int y) const;
    
    // 获取移动代价（考虑地形耗时倍数和对角线移动）
    float getMoveCost(int fromX, int fromY, int toX, int toY) const;
    
    // 重建路径
    std::vector<PathPoint> reconstructPath(PathNode* endNode) const;

public:
    AStar(Map* gameMap) : map(gameMap) {}
    
    // 执行A*寻路
    std::pair<PathfindingResult, std::vector<PathPoint>> findPath(const PathfindingRequest& request);
    
    // 简化路径（移除不必要的中间点）
    std::vector<PathPoint> smoothPath(const std::vector<PathPoint>& path) const;
    
    // 检查两点间是否有直线路径
    bool hasDirectPath(int x1, int y1, int x2, int y2) const;
};

// 生物寻路管理器
class CreaturePathfinder {
private:
    AStar astar;
    
    // 寻路冷却管理
    struct PathfindingCooldown {
        float timer;                 // 冷却计时器
        float interval;              // 冷却间隔 (0.1秒 + 随机0.0-0.05秒)
        bool needsUpdate;            // 是否需要更新路径
        
        PathfindingCooldown() : timer(0.0f), interval(0.1f), needsUpdate(true) {
            // 添加随机时间避免同帧卡顿 - 减少间隔提高频率
            interval += (rand() % 50) / 1000.0f; // 0.0-0.049秒
        }
    };
    
    // 生物寻路数据
    struct CreaturePathData {
        std::vector<PathPoint> currentPath;     // 当前路径
        int currentWaypoint;                    // 当前路径点索引
        PathfindingCooldown cooldown;           // 寻路冷却
        PathfindingResult lastResult;           // 上次寻路结果
        int lastTargetX, lastTargetY;           // 上次目标位置
        
        CreaturePathData() : currentWaypoint(0), lastResult(PathfindingResult::NO_PATH), 
                            lastTargetX(-1), lastTargetY(-1) {}
    };
    
    // 生物寻路数据映射（使用生物指针作为键）
    std::unordered_map<void*, std::unique_ptr<CreaturePathData>> pathDataMap;

public:
    CreaturePathfinder(Map* gameMap) : astar(gameMap) {}
    
    // 为生物请求寻路
    PathfindingResult requestPath(void* creature, int startX, int startY, int targetX, int targetY, float intelligence);
    
    // 更新生物寻路状态
    void updateCreature(void* creature, float deltaTime);
    
    // 获取生物下一个移动目标
    std::pair<bool, PathPoint> getNextWaypoint(void* creature, float currentX, float currentY);
    
    // 检查生物是否需要直线移动（当寻路失败时）
    bool shouldMoveDirectly(void* creature) const;
    
    // 获取直线移动方向
    std::pair<float, float> getDirectMoveDirection(void* creature, int startX, int startY, int targetX, int targetY) const;
    
    // 清理生物寻路数据
    void removeCreature(void* creature);
    
    // 强制重新计算路径
    void forcePathUpdate(void* creature);
    
    // 调试：获取生物当前路径
    const std::vector<PathPoint>* getCreaturePath(void* creature) const;
};

#endif // PATHFINDING_H 