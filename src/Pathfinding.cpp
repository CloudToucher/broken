#include "Pathfinding.h"
#include "Map.h"
#include "Tile.h"
#include "Game.h"
#include "Collider.h"
#include "Constants.h"
#include <cmath>
#include <algorithm>
#include <iostream>

// AStar类实现

float AStar::calculateHeuristic(int x1, int y1, int x2, int y2) const {
    // 使用欧几里得距离作为启发式函数
    int dx = x2 - x1;
    int dy = y2 - y1;
    return std::sqrt(dx * dx + dy * dy);
}

std::vector<std::pair<int, int>> AStar::getNeighbors(int x, int y) const {
    std::vector<std::pair<int, int>> neighbors;
    
    // 8方向移动（包括对角线）
    const int directions[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1},  // 左上、左、左下
        { 0, -1},          { 0, 1},  // 上、下
        { 1, -1}, { 1, 0}, { 1, 1}   // 右上、右、右下
    };
    
    for (int i = 0; i < 8; ++i) {
        int newX = x + directions[i][0];
        int newY = y + directions[i][1];
        
        if (isWalkable(newX, newY)) {
            neighbors.push_back({newX, newY});
        }
    }
    
    return neighbors;
}

bool AStar::isWalkable(int x, int y) const {
    if (!map) return false;
    
    // 将网格坐标转换为世界坐标
    int worldX = x * GameConstants::TILE_SIZE;
    int worldY = y * GameConstants::TILE_SIZE;
    
    // 获取该位置的tile
    Tile* tile = map->getTileAt(worldX, worldY);
    if (!tile) return true; // 如果没有tile，认为可通行
    
    // 检查是否有地形碰撞箱
    return !tile->hasColliderWithPurpose(ColliderPurpose::TERRAIN);
}

float AStar::getMoveCost(int fromX, int fromY, int toX, int toY) const {
    if (!map) return 1.0f;
    
    // 将网格坐标转换为世界坐标
    int worldX = toX * GameConstants::TILE_SIZE;
    int worldY = toY * GameConstants::TILE_SIZE;
    
    // 获取目标tile的移动耗时倍数
    Tile* tile = map->getTileAt(worldX, worldY);
    float baseCost = tile ? tile->getMoveCost() : 100.0f; // 默认100
    
    // 标准化基础耗时（以100为基准）
    float normalizedCost = baseCost / 100.0f;
    
    // 检查是否为对角线移动
    int dx = abs(toX - fromX);
    int dy = abs(toY - fromY);
    
    if (dx == 1 && dy == 1) {
        // 对角线移动，距离为√2
        return normalizedCost * 1.414f;
    } else {
        // 直线移动，距离为1
        return normalizedCost;
    }
}

std::vector<PathPoint> AStar::reconstructPath(PathNode* endNode) const {
    std::vector<PathPoint> path;
    PathNode* current = endNode;
    
    while (current != nullptr) {
        // 将网格坐标转换为世界坐标（网格中心）
        float worldX = current->x * GameConstants::TILE_SIZE + GameConstants::TILE_CENTER_OFFSET;
        float worldY = current->y * GameConstants::TILE_SIZE + GameConstants::TILE_CENTER_OFFSET;
        
        // 获取移动耗时倍数
        Tile* tile = map->getTileAt(current->x * GameConstants::TILE_SIZE, current->y * GameConstants::TILE_SIZE);
        float moveCost = tile ? tile->getMoveCost() / 100.0f : 1.0f;
        
        path.emplace_back(worldX, worldY, moveCost);
        current = current->parent;
    }
    
    // 反转路径（从起点到终点）
    std::reverse(path.begin(), path.end());
    return path;
}

std::pair<PathfindingResult, std::vector<PathPoint>> AStar::findPath(const PathfindingRequest& request) {
    // 检查起点和终点是否可通行
    if (!isWalkable(request.startX, request.startY)) {
        return {PathfindingResult::START_BLOCKED, {}};
    }
    if (!isWalkable(request.targetX, request.targetY)) {
        return {PathfindingResult::TARGET_BLOCKED, {}};
    }
    
    // 如果起点就是终点
    if (request.startX == request.targetX && request.startY == request.targetY) {
        return {PathfindingResult::SUCCESS, {}};
    }
    
    // 计算智能程度限制
    float startToEndDistance = calculateHeuristic(request.startX, request.startY, request.targetX, request.targetY);
    float intelligenceLimit = request.intelligence * startToEndDistance + (request.intelligence - 1.0f) * 8.0f;
    
    // 初始化数据结构
    std::priority_queue<PathNode*, std::vector<PathNode*>, std::greater<PathNode*>> openList;
    std::unordered_set<std::pair<int, int>, NodeHash> closedList;
    std::unordered_map<std::pair<int, int>, std::unique_ptr<PathNode>, NodeHash> allNodes;
    
    // 创建起始节点
    auto startNode = std::make_unique<PathNode>(request.startX, request.startY);
    startNode->gCost = 0;
    startNode->hCost = startToEndDistance;
    startNode->fCost = startNode->hCost;
    
    PathNode* startPtr = startNode.get();
    allNodes[{request.startX, request.startY}] = std::move(startNode);
    openList.push(startPtr);
    
    int iterations = 0;
    
    while (!openList.empty() && iterations < 10000) { // 设置最大迭代保护
        iterations++;
        
        // 获取f值最小的节点
        PathNode* current = openList.top();
        openList.pop();
        
        // 如果已经在关闭列表中，跳过
        if (closedList.find({current->x, current->y}) != closedList.end()) {
            continue;
        }
        
        // 添加到关闭列表
        closedList.insert({current->x, current->y});
        
        // 计算当前节点到终点的距离
        float currentToEndDistance = calculateHeuristic(current->x, current->y, request.targetX, request.targetY);
        
        // 检查是否到达目标
        if (current->x == request.targetX && current->y == request.targetY) {
            std::vector<PathPoint> path = reconstructPath(current);
            return {PathfindingResult::SUCCESS, path};
        }
        
        // 检查智能程度限制：如果当前节点到终点距离超过智能限制，停止搜索
        if (currentToEndDistance >= intelligenceLimit) {
            break; // 智能不足，停止搜索
        }
        
        // 检查所有邻居
        std::vector<std::pair<int, int>> neighbors = getNeighbors(current->x, current->y);
        for (const auto& neighbor : neighbors) {
            int nx = neighbor.first;
            int ny = neighbor.second;
            
            // 如果邻居在关闭列表中，跳过
            if (closedList.find({nx, ny}) != closedList.end()) {
                continue;
            }
            
            // 计算到邻居的代价
            float tentativeGCost = current->gCost + getMoveCost(current->x, current->y, nx, ny);
            
            // 查找或创建邻居节点
            PathNode* neighborNode = nullptr;
            auto it = allNodes.find({nx, ny});
            if (it == allNodes.end()) {
                auto newNode = std::make_unique<PathNode>(nx, ny);
                neighborNode = newNode.get();
                allNodes[{nx, ny}] = std::move(newNode);
            } else {
                neighborNode = it->second.get();
            }
            
            // 如果这条路径更好，或者是第一次访问这个节点
            if (neighborNode->parent == nullptr || tentativeGCost < neighborNode->gCost) {
                neighborNode->parent = current;
                neighborNode->gCost = tentativeGCost;
                neighborNode->hCost = calculateHeuristic(nx, ny, request.targetX, request.targetY);
                neighborNode->fCost = neighborNode->gCost + neighborNode->hCost;
                
                openList.push(neighborNode);
            }
        }
    }
    
    // 找不到路径，返回NO_PATH让系统进行直线移动
    return {PathfindingResult::NO_PATH, {}};
}

std::vector<PathPoint> AStar::smoothPath(const std::vector<PathPoint>& path) const {
    if (path.size() <= 2) return path;
    
    std::vector<PathPoint> smoothed;
    smoothed.push_back(path[0]); // 添加起点
    
    size_t current = 0;
    while (current < path.size() - 1) {
        size_t farthest = current + 1;
        
        // 找到最远的可直达点
        for (size_t i = current + 2; i < path.size(); ++i) {
            int x1 = static_cast<int>(path[current].x / 64);
            int y1 = static_cast<int>(path[current].y / 64);
            int x2 = static_cast<int>(path[i].x / 64);
            int y2 = static_cast<int>(path[i].y / 64);
            
            if (hasDirectPath(x1, y1, x2, y2)) {
                farthest = i;
            } else {
                break;
            }
        }
        
        current = farthest;
        smoothed.push_back(path[current]);
    }
    
    return smoothed;
}

bool AStar::hasDirectPath(int x1, int y1, int x2, int y2) const {
    // 使用Bresenham直线算法检查路径上是否有障碍物
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int x = x1;
    int y = y1;
    int n = 1 + dx + dy;
    int x_inc = (x2 > x1) ? 1 : -1;
    int y_inc = (y2 > y1) ? 1 : -1;
    int error = dx - dy;
    
    dx *= 2;
    dy *= 2;
    
    for (; n > 0; --n) {
        // 检查当前点是否可通行
        if (!isWalkable(x, y)) {
            return false;
        }
        
        if (error > 0) {
            x += x_inc;
            error -= dy;
        } else {
            y += y_inc;
            error += dx;
        }
    }
    
    return true;
}

// CreaturePathfinder类实现

PathfindingResult CreaturePathfinder::requestPath(void* creature, int startX, int startY, int targetX, int targetY, float intelligence) {
    // 获取或创建生物寻路数据
    auto& pathData = pathDataMap[creature];
    if (!pathData) {
        pathData = std::make_unique<CreaturePathData>();
    }
    
    // 检查冷却时间
    if (pathData->cooldown.timer > 0) {
        return pathData->lastResult;
    }
    
    // 将世界坐标转换为网格坐标
    int gridStartX = startX / GRID_SIZE;
    int gridStartY = startY / GRID_SIZE;
    int gridTargetX = targetX / GRID_SIZE;
    int gridTargetY = targetY / GRID_SIZE;
    
    // 检查是否有直接无障碍路径
    if (astar.hasDirectPath(gridStartX, gridStartY, gridTargetX, gridTargetY)) {
        // 如果有直接路径，清空A*路径，让生物直线移动
        pathData->currentPath.clear();
        pathData->lastResult = PathfindingResult::NO_PATH; // 使用NO_PATH表示直线移动
        pathData->lastTargetX = targetX;
        pathData->lastTargetY = targetY;
        
        // 设置短冷却时间（因为不需要复杂计算）- 提高频率
        pathData->cooldown.timer = 0.05f + (rand() % 50) / 1000.0f; // 0.05-0.1秒
        return PathfindingResult::NO_PATH; // 返回NO_PATH让生物直线移动
    }
    
    // 只有在有障碍物时才使用A*寻路
    PathfindingRequest request(gridStartX, gridStartY, gridTargetX, gridTargetY, intelligence);
    
    // 执行寻路
    auto result = astar.findPath(request);
    
    // 更新寻路数据
    pathData->lastResult = result.first;
    pathData->currentPath = result.second;
    pathData->currentWaypoint = 0;
    pathData->lastTargetX = targetX;
    pathData->lastTargetY = targetY;
    
    // 如果成功找到路径，进行路径平滑
    if (result.first == PathfindingResult::SUCCESS && !result.second.empty()) {
        pathData->currentPath = astar.smoothPath(pathData->currentPath);
    }
    
    // 如果寻路失败，清空路径让生物进行直线移动
    if (result.first == PathfindingResult::NO_PATH) {
        pathData->currentPath.clear();
    }
    
    // 重置冷却时间
    pathData->cooldown.timer = pathData->cooldown.interval;
    pathData->cooldown.needsUpdate = false;
    
    return result.first;
}

void CreaturePathfinder::updateCreature(void* creature, float deltaTime) {
    auto it = pathDataMap.find(creature);
    if (it == pathDataMap.end()) return;
    
    auto& pathData = it->second;
    
    // 更新冷却计时器
    if (pathData->cooldown.timer > 0) {
        pathData->cooldown.timer -= deltaTime;
        if (pathData->cooldown.timer <= 0) {
            pathData->cooldown.needsUpdate = true;
        }
    }
}

std::pair<bool, PathPoint> CreaturePathfinder::getNextWaypoint(void* creature, float currentX, float currentY) {
    auto it = pathDataMap.find(creature);
    if (it == pathDataMap.end() || it->second->currentPath.empty()) {
        return {false, PathPoint(0, 0)};
    }
    
    auto& pathData = it->second;
    
    // 检查是否到达当前路径点
    if (pathData->currentWaypoint < pathData->currentPath.size()) {
        const PathPoint& waypoint = pathData->currentPath[pathData->currentWaypoint];
        
        // 计算到路径点的距离
        float dx = waypoint.x - currentX;
        float dy = waypoint.y - currentY;
        float distance = std::sqrt(dx * dx + dy * dy);
        
        // 如果距离足够近，移动到下一个路径点
        if (distance < 32.0f) { // 半个网格的距离
            pathData->currentWaypoint++;
            if (pathData->currentWaypoint >= pathData->currentPath.size()) {
                // 路径完成
                pathData->currentPath.clear();
                pathData->currentWaypoint = 0;
                return {false, PathPoint(0, 0)};
            }
        }
        
        return {true, pathData->currentPath[pathData->currentWaypoint]};
    }
    
    return {false, PathPoint(0, 0)};
}

bool CreaturePathfinder::shouldMoveDirectly(void* creature) const {
    auto it = pathDataMap.find(creature);
    if (it == pathDataMap.end()) return true;
    
    return it->second->lastResult != PathfindingResult::SUCCESS;
}

std::pair<float, float> CreaturePathfinder::getDirectMoveDirection(void* creature, int startX, int startY, int targetX, int targetY) const {
    float dx = targetX - startX;
    float dy = targetY - startY;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    if (distance < 1.0f) {
        return {0.0f, 0.0f};
    }
    
    return {dx / distance, dy / distance};
}

void CreaturePathfinder::removeCreature(void* creature) {
    pathDataMap.erase(creature);
}

void CreaturePathfinder::forcePathUpdate(void* creature) {
    auto it = pathDataMap.find(creature);
    if (it != pathDataMap.end()) {
        it->second->cooldown.timer = 0;
        it->second->cooldown.needsUpdate = true;
        it->second->currentPath.clear();
    }
}

const std::vector<PathPoint>* CreaturePathfinder::getCreaturePath(void* creature) const {
    auto it = pathDataMap.find(creature);
    if (it == pathDataMap.end()) return nullptr;
    
    return &it->second->currentPath;
} 