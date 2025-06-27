#include "Map.h"
#include "Game.h"
#include "Constants.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

Map::Map(SDL_Renderer* rendererPtr, int loadDist) 
    : loadDistance(loadDist), 
      playerGridX(0), 
      playerGridY(0),
      mapDir("map"),
      renderer(rendererPtr),
      maxGridsPerFrame(5) { // 每帧最多加载5个网格，提高加载速度
    
    // 创建地图目录（如果不存在）
    if (!fs::exists(mapDir)) {
        fs::create_directory(mapDir);
    }
}

Map::~Map() {
    // 保存所有已加载的网格
    // 暂时注释掉地图保存逻辑，用于测试
    /*
    for (auto it = grids.begin(); it != grids.end(); ++it) {
        archiveGrid(it->first.x, it->first.y, it->second.get());
    }
    */
}

// 在Map.cpp中实现
void Map::updatePlayerPosition(float worldX, float worldY) {
    // 计算玩家所在的网格坐标
    int newGridX, newGridY;
    worldToGridCoord(worldX, worldY, newGridX, newGridY);
    
    // 如果玩家网格坐标没有变化，不需要更新
    if (newGridX == playerGridX && newGridY == playerGridY) {
        return;
    }
    
    // 更新玩家网格坐标
    playerGridX = newGridX;
    playerGridY = newGridY;
    
    // 测试期间完全禁用动态地图加载/卸载系统
    // 只使用初始化时生成的固定地图
    
    /*
    //// std::cout << "===== 玩家位置更新 =====" << std::endl;
    //// std::cout << "玩家世界坐标: (" << worldX << ", " << worldY << ")" << std::endl;
    //// std::cout << "玩家网格坐标: 从 (" << playerGridX << ", " << playerGridY << ") 变为 (" << newGridX << ", " << newGridY << ")" << std::endl;
    
    // 收集需要加载和卸载的网格
    std::vector<GridCoord> innerGridsToLoad; // 玩家附近的网格（优先加载）
    std::vector<GridCoord> outerGridsToLoad; // 外围网格
    std::vector<GridCoord> gridsToUnload;
    
    // 确定需要加载的网格范围，分为内圈和外圈
    // 内圈：玩家所在位置及其周围8个网格（3x3范围）
    for (int y = playerGridY - 1; y <= playerGridY + 1; ++y) {
        for (int x = playerGridX - 1; x <= playerGridX + 1; ++x) {
            GridCoord coord{x, y};
            if (grids.find(coord) == grids.end()) {
                innerGridsToLoad.push_back(coord);
            }
        }
    }
    
    // 外圈：loadDistance范围内的其他网格
    for (int y = playerGridY - loadDistance; y <= playerGridY + loadDistance; ++y) {
        for (int x = playerGridX - loadDistance; x <= playerGridX + loadDistance; ++x) {
            // 跳过已经处理过的内圈网格
            if (abs(x - playerGridX) <= 1 && abs(y - playerGridY) <= 1) {
                continue;
            }
            
            GridCoord coord{x, y};
            if (grids.find(coord) == grids.end()) {
                outerGridsToLoad.push_back(coord);
            }
        }
    }
    
    //// std::cout << "需要加载的内圈网格数量: " << innerGridsToLoad.size() << std::endl;
    //// std::cout << "需要加载的外圈网格数量: " << outerGridsToLoad.size() << std::endl;
    
    // 确定需要卸载的网格
    for (auto it = grids.begin(); it != grids.end(); ++it) {
        const GridCoord& coord = it->first;
        if (!isGridInLoadRange(coord.x, coord.y)) {
            gridsToUnload.push_back(coord);
        }
    }
    
    std::cout << "需要卸载的网格数量: " << gridsToUnload.size() << std::endl;
    
    // 卸载网格前记录当前网格数量
    int gridCountBeforeUnload = grids.size();
    std::cout << "卸载前网格数量: " << gridCountBeforeUnload << std::endl;
    
    int unloadedCount = 0;
    for (const auto& coord : gridsToUnload) {
        // 封存网格到文件
        // 先检查网格是否存在
        if (grids.find(coord) != grids.end() && grids[coord]) {
            // 确保网格被正确封存到文件
            // 暂时注释掉地图保存逻辑，用于测试
            // archiveGrid(coord.x, coord.y, grids[coord].get());
            //std::cout << "正在卸载网格: (" << coord.x << ", " << coord.y << ")" << std::endl;
            
            // 从内存中移除
            grids.erase(coord);
            unloadedCount++;
        }
        else {
            std::cerr << "错误：尝试卸载不存在的网格 (" << coord.x << ", " << coord.y << ")" << std::endl;
        }
    }
    
    // 卸载后记录网格数量
    int gridCountAfterUnload = grids.size();
    std::cout << "成功卸载 " << unloadedCount << " 个网格，卸载后网格数量: " << gridCountAfterUnload << std::endl;
    
    // 确保在卸载网格后更新障碍物列表
    if (unloadedCount > 0) {
        std::cout << "卸载网格后更新障碍物列表" << std::endl;
        updateObstacles();
    }
    
    // 将需要加载的网格添加到异步加载队列，限制队列大小
    {
        std::lock_guard<std::mutex> lock(loadQueueMutex);
        
        // 清空当前队列，确保新的加载请求能够被处理
        // 这是一个激进的方法，但可以确保玩家附近的网格能够被及时加载
        while (!gridsToLoadAsync.empty()) {
            gridsToLoadAsync.pop();
        }
        
        // 优先添加内圈网格（玩家附近的网格）
        for (const auto& coord : innerGridsToLoad) {
            //// std::cout << "将内圈网格添加到异步加载队列: (" << coord.x << ", " << coord.y << ")" << std::endl;
            gridsToLoadAsync.push(coord);
        }
        
        // 然后添加外圈网格，但限制总队列大小
        const size_t MAX_QUEUE_SIZE = 20; // 限制队列大小，避免积压
        for (const auto& coord : outerGridsToLoad) {
            if (gridsToLoadAsync.size() < MAX_QUEUE_SIZE) {
                //// std::cout << "将外圈网格添加到异步加载队列: (" << coord.x << ", " << coord.y << ")" << std::endl;
                gridsToLoadAsync.push(coord);
            } else {
                //// std::cout << "异步加载队列已满，跳过外圈网格: (" << coord.x << ", " << coord.y << ")" << std::endl;
                break; // 队列已满，停止添加
            }
        }
    }
    
    std::cout << "当前已加载的网格总数: " << grids.size() << std::endl;
    std::cout << "异步加载队列中的网格数: " << gridsToLoadAsync.size() << std::endl;
    std::cout << "===== 玩家位置更新完成 =====" << std::endl;
    
    // 更新障碍物列表
    updateObstacles();
    */
}

// 在Map::update方法中添加标志，确保纹理初始化完成后再渲染
void Map::update() {
    // 测试期间完全禁用异步地图加载系统
    // 只在初始化时生成地图，运行时不再动态加载/卸载网格
    
    /*
    // 每帧处理有限数量的网格加载
    int loadedCount = 0;
    
    std::vector<GridCoord> coordsToProcess;
    {
        std::lock_guard<std::mutex> lock(loadQueueMutex);
        // 确保不会一次处理太多网格，避免卡顿
        while (!gridsToLoadAsync.empty() && loadedCount < maxGridsPerFrame) {
            coordsToProcess.push_back(gridsToLoadAsync.front());
            gridsToLoadAsync.pop();
            loadedCount++;
        }
    }
    
    if (!coordsToProcess.empty()) {
        // std::cout << "===== 开始处理异步加载队列中的网格 ====="  << std::endl;
        // std::cout << "需要处理的网格数量: " << coordsToProcess.size() << std::endl;
    }
    
    // 处理这些网格
    int successfullyLoaded = 0;
    for (const auto& coord : coordsToProcess) {
        // std::cout << "处理网格: (" << coord.x << ", " << coord.y << ")" << std::endl;
        
        // 尝试从文件加载或生成新网格
        // 暂时注释掉地图加载逻辑，用于测试
        // std::unique_ptr<Grid> grid = loadGridFromFile(coord.x, coord.y);
        std::unique_ptr<Grid> grid = nullptr; // 暂时直接设为nullptr，强制生成新网格
        
        if (!grid) {
            // std::cout << "从文件加载失败，生成新网格" << std::endl;
            grid = generateNewGrid(coord.x, coord.y);
            
            // 确保新生成的网格纹理被初始化
            if (grid) {
                // std::cout << "初始化新生成网格的纹理" << std::endl;
                grid->initializeTextures(renderer);
            }
        } else {
            // 确保从文件加载的网格纹理也被初始化
            std::cout << "初始化从文件加载的网格纹理" << std::endl;
            grid->initializeTextures(renderer);
        }
        
        // 添加到地图
        if (grid) {
            // std::cout << "将网格添加到地图: (" << coord.x << ", " << coord.y << ")" << std::endl;
            grids[coord] = std::move(grid);
            successfullyLoaded++;
        } else {
            std::cerr << "错误：无法加载或生成网格: (" << coord.x << ", " << coord.y << ")" << std::endl;
        }
    }
    
    // 只有在处理了网格后才更新障碍物列表
    if (!coordsToProcess.empty()) {
        std::cout << "成功加载并添加到地图的网格数量: " << successfullyLoaded << "/" << coordsToProcess.size() << std::endl;
        std::cout << "更新障碍物列表" << std::endl;
        updateObstacles();
        std::cout << "===== 异步加载队列处理完成 ===== "  << std::endl;
    }
    */
}

void Map::render(SDL_Renderer* renderer, float cameraX, float cameraY) {
    // 获取窗口尺寸
    int windowWidth, windowHeight;
    SDL_GetRenderOutputSize(renderer, &windowWidth, &windowHeight);
    
    // 获取当前缩放级别
    float zoomLevel = Game::getInstance()->getZoomLevel();
    
    // 计算可见区域（考虑缩放因素）
    int startX = static_cast<int>(cameraX);
    int startY = static_cast<int>(cameraY);
    int endX = static_cast<int>(cameraX + windowWidth / zoomLevel);
    int endY = static_cast<int>(cameraY + windowHeight / zoomLevel);
    
    // 渲染所有网格
    for (auto it = grids.begin(); it != grids.end(); ++it) {
        Grid* grid = it->second.get();
        // 检查网格是否在可见区域内
        int gridX = grid->getX();
        int gridY = grid->getY();
        int gridSize = grid->getTotalSize();
        
        if (gridX + gridSize >= startX && gridX <= endX && 
            gridY + gridSize >= startY && gridY <= endY) {
            grid->render(renderer, cameraX, cameraY);
        }
    }
    
    // 渲染障碍物
    for (const auto& obstacle : obstacles) {
        obstacle.render(renderer, cameraX, cameraY);
    }
}

void Map::addGrid(std::unique_ptr<Grid> grid, int gridX, int gridY) {
    // 添加网格到地图
    GridCoord coord{gridX, gridY};
    grids[coord] = std::move(grid);
    
    // 更新障碍物列表
    updateObstacles();
}

// 更新障碍物列表（从所有网格中收集碰撞箱）
void Map::updateObstacles() {
    obstacles.clear();
    
    // 收集所有网格中的障碍物
    int obstacleCount = 0;
    int gridCount = 0;
    for (auto it = grids.begin(); it != grids.end(); ++it) {
        gridCount++;
        std::vector<Collider> gridColliders = it->second->getColliders();
        obstacles.insert(obstacles.end(), gridColliders.begin(), gridColliders.end());
        obstacleCount += gridColliders.size();
    }
    
    // std::cout << "更新障碍物列表，从 " << gridCount << " 个网格中收集了 " << obstacleCount << " 个障碍物" << std::endl;
}

Grid* Map::getGridAt(float worldX, float worldY) const {
    // 将世界坐标转换为网格坐标
    int gridX, gridY;
    worldToGridCoord(worldX, worldY, gridX, gridY);
    
    return getGridAtCoord(gridX, gridY);
}

Grid* Map::getGridAtCoord(int gridX, int gridY) const {
    // 查找指定网格坐标的网格
    GridCoord coord{gridX, gridY};
    auto it = grids.find(coord);
    if (it != grids.end()) {
        return it->second.get();
    }
    
    return nullptr;
}

Tile* Map::getTileAt(float worldX, float worldY) const {
    // 先找到包含指定位置的网格
    Grid* grid = getGridAt(worldX, worldY);
    if (!grid) {
        return nullptr;
    }
    
    // 计算在网格内的相对坐标
    int relX = static_cast<int>(worldX) - grid->getX();
    int relY = static_cast<int>(worldY) - grid->getY();
    
    // 计算方块的网格坐标
    int tileSize = grid->getTileSize();
    int tileX = relX / tileSize;
    int tileY = relY / tileSize;
    
    // 获取方块
    return grid->getTile(tileX, tileY);
}

void Map::initialize() {
    // std::cout << "===== 开始初始化地图 =====" << std::endl;
    // 生成初始网格（玩家位置为(0,0)，位于(0,0)网格的左下角）
    // 只加载玩家周围的网格，避免一次性加载过多
    int totalGridsGenerated = 0;
    for (int y = -loadDistance; y <= loadDistance; ++y) {
        for (int x = -loadDistance; x <= loadDistance; ++x) {
            auto grid = generateNewGrid(x, y);
            GridCoord coord{x, y};
            grids[coord] = std::move(grid);
            totalGridsGenerated++;
        }
    }
    
    // std::cout << "总共生成网格数量: " << totalGridsGenerated << std::endl;
    
    // 将网格分为两组：立即初始化的核心网格和延迟初始化的外围网格
    std::vector<Grid*> coreGrids; // 玩家所在及相邻的9个网格
    std::vector<Grid*> outerGrids; // 其他网格
    
    // 遍历所有网格，分类
    for (auto it = grids.begin(); it != grids.end(); ++it) {
        const GridCoord& coord = it->first;
        // 玩家所在位置(0,0)及其周围8个网格立即初始化
        if (abs(coord.x) <= 1 && abs(coord.y) <= 1) {
            coreGrids.push_back(it->second.get());
        } else {
            outerGrids.push_back(it->second.get());
        }
    }
    
    // std::cout << "立即初始化核心网格数量: " << coreGrids.size() << std::endl;
    // 立即初始化核心网格（玩家所在及周围的网格）
    int coreGridsInitialized = 0;
    for (auto* grid : coreGrids) {
        grid->initializeTextures(renderer);
        coreGridsInitialized++;
    }
    // std::cout << "已初始化核心网格数量: " << coreGridsInitialized << std::endl;
    
    // std::cout << "立即初始化外围网格数量: " << outerGrids.size() << std::endl;
    // 立即初始化外围网格，而不是添加到异步队列
    // 这样可以确保所有初始网格都有纹理
    int outerGridsInitialized = 0;
    for (auto* grid : outerGrids) {
        grid->initializeTextures(renderer);
        outerGridsInitialized++;
    }
    // std::cout << "已初始化外围网格数量: " << outerGridsInitialized << std::endl;
    
    // 更新障碍物列表
    updateObstacles();
    
    // std::cout << "地图初始化完成，共加载 " << grids.size() << " 个网格，总共初始化纹理的网格数: " << (coreGridsInitialized + outerGridsInitialized) << std::endl;
    // std::cout << "===== 地图初始化完成 =====" << std::endl;
}

std::string Map::getGridFilePath(int gridX, int gridY) const {
    // 创建文件名：gridX_gridY.txt
    std::stringstream ss;
    ss << mapDir << "/" << gridX << "_" << gridY << ".txt";
    return ss.str();
}

bool Map::isGridInLoadRange(int gridX, int gridY) const {
    return abs(gridX - playerGridX) <= loadDistance && 
           abs(gridY - playerGridY) <= loadDistance;
}

std::unique_ptr<Grid> Map::generateNewGrid(int gridX, int gridY) {
    //// std::cout << "生成新网格: (" << gridX << ", " << gridY << ")" << std::endl;
    
    // 计算网格的世界坐标
    int worldX, worldY;
    gridCoordToWorld(gridX, gridY, worldX, worldY);
    //// std::cout << "  网格世界坐标: (" << worldX << ", " << worldY << ")" << std::endl;
    
    // 网格基本参数
    int gridSize = GameConstants::MAP_GRID_SIZE; // 每个网格的方块数
    int tileSize = GameConstants::TILE_SIZE; // 每个方块的像素数
    
    // 创建一个新的网格
    std::stringstream gridName;
    gridName << "Grid_" << gridX << "_" << gridY;
    
    // 直接使用Grid::createGrasslandGrid创建草地网格
    auto grid = Grid::createGrasslandGrid(worldX, worldY, gridSize, tileSize);
   // // std::cout << "  创建草地网格成功: " << gridName.str() << std::endl;
    return grid;
}

// 暂时不使用的地图保存功能（用于测试）
void Map::archiveGrid(int gridX, int gridY, Grid* grid) {
    // 测试期间注释掉网格保存功能
    return;
    
    /* 原始网格保存代码已注释
    if (!grid) {
        std::cerr << "错误：尝试封存空网格 (" << gridX << ", " << gridY << ")" << std::endl;
        return;
    }
    
    std::string filePath = getGridFilePath(gridX, gridY);
    //std::cout << "封存网格: (" << gridX << ", " << gridY << ") 到文件: " << filePath << std::endl;
    
    // 创建目录（如果不存在）
    std::filesystem::path dirPath = std::filesystem::path(filePath).parent_path();
    if (!std::filesystem::exists(dirPath)) {
        std::filesystem::create_directories(dirPath);
    }
    
    // 打开文件进行写入
    std::ofstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "错误：无法打开文件进行写入: " << filePath << std::endl;
        return;
    }
    
    // 写入网格基本信息
    file << "[GRID_INFO]" << std::endl;
    file << "name=" << grid->getName() << std::endl;
    file << "position=" << grid->getX() << "," << grid->getY() << std::endl;
    file << "size=" << grid->getGridSize() << std::endl;
    file << "tileSize=" << grid->getTileSize() << std::endl;
    file << std::endl;
    
    // 写入方块信息
    file << "[TILES]" << std::endl;
    for (int y = 0; y < grid->getGridSize(); ++y) {
        for (int x = 0; x < grid->getGridSize(); ++x) {
            Tile* tile = grid->getTile(x, y);
            if (tile) {
                file << "tile=" << x << "," << y << "," << tile->getName() << ","
                     << (tile->getHasCollision() ? "1" : "0") << ","
                     << (tile->getIsTransparent() ? "1" : "0") << ","
                     << (tile->getIsDestructible() ? "1" : "0") << ","
                     << static_cast<int>(tile->getRotation()) << std::endl;
            }
        }
    }
    file << std::endl;
    
    // 写入实体信息（未来实现）
    file << "[ENTITIES]" << std::endl;
    // 这里将来可以添加实体的保存逻辑
    // 例如：file << "entity=type,x,y,health,..." << std::endl;
    
    file.close();
    // std::cout << "网格封存完成: " << filePath << std::endl;
    */
}

// 暂时不使用的地图加载功能（用于测试）
std::unique_ptr<Grid> Map::loadGridFromFile(int gridX, int gridY) {
    // 测试期间注释掉文件加载功能，直接返回nullptr让系统生成新网格
    return nullptr;
    
    /* 原始文件加载代码已注释
    // 检查文件是否存在
    std::string filePath = getGridFilePath(gridX, gridY);
    //std::cout << "尝试从文件加载网格: (" << gridX << ", " << gridY << ")" << std::endl;
    //std::cout << "  文件路径: " << filePath << std::endl;
    
    if (!fs::exists(filePath)) {
        std::cout << "  文件不存在，将生成新网格" << std::endl;
        return nullptr;
    }
    
    try {
        //std::cout << "  文件存在，开始加载" << std::endl;
        
        // 打开文件进行读取
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "  无法打开文件进行读取: " << filePath << std::endl;
            return nullptr;
        }
        
        // 默认网格参数
        std::string gridName = "Grid_" + std::to_string(gridX) + "_" + std::to_string(gridY);
        int worldX, worldY;
        gridCoordToWorld(gridX, gridY, worldX, worldY); // 默认世界坐标
        int gridSize = GameConstants::MAP_GRID_SIZE; // 默认网格大小
        int tileSize = GameConstants::TILE_SIZE; // 默认方块大小
        
        // 读取文件内容
        std::string line;
        std::string section;
        
        while (std::getline(file, line)) {
            // 跳过空行
            if (line.empty()) {
                continue;
            }
            
            // 检查是否是节标记
            if (line[0] == '[' && line[line.length() - 1] == ']') {
                section = line;
                continue;
            }
            
            // 根据当前节处理数据
            if (section == "[GRID_INFO]") {
                // 处理网格信息
                size_t pos = line.find('=');
                if (pos != std::string::npos) {
                    std::string key = line.substr(0, pos);
                    std::string value = line.substr(pos + 1);
                    
                    if (key == "name") {
                        gridName = value;
                    } else if (key == "position") {
                        // 解析位置 "x,y"
                        size_t commaPos = value.find(',');
                        if (commaPos != std::string::npos) {
                            worldX = std::stoi(value.substr(0, commaPos));
                            worldY = std::stoi(value.substr(commaPos + 1));
                        }
                    } else if (key == "size") {
                        gridSize = std::stoi(value);
                    } else if (key == "tileSize") {
                        tileSize = std::stoi(value);
                    }
                }
            }
        }
        
        // 创建网格
        // std::cout << "  创建网格: " << gridName << " 位置(" << worldX << ", " << worldY << ") 大小" << gridSize << "x" << gridSize << " 方块大小" << tileSize << std::endl;
        auto grid = std::make_unique<Grid>(gridName, worldX, worldY, gridSize, tileSize);
        
        // 重置文件指针到开头
        file.clear();
        file.seekg(0, std::ios::beg);
        section = "";
        
        // 再次读取文件，这次处理方块和实体
        int tilesLoaded = 0;
        while (std::getline(file, line)) {
            // 跳过空行
            if (line.empty()) {
                continue;
            }
            
            // 检查是否是节标记
            if (line[0] == '[' && line[line.length() - 1] == ']') {
                section = line;
                continue;
            }
            
            // 处理方块信息
            if (section == "[TILES]") {
                if (line.substr(0, 5) == "tile=") {
                    std::string tileData = line.substr(5);
                    std::vector<std::string> parts;
                    size_t pos = 0;
                    std::string token;
                    
                    // 分割字符串 "x,y,name,hasCollision,isTransparent,isDestructible,rotation"
                    while ((pos = tileData.find(',')) != std::string::npos) {
                        token = tileData.substr(0, pos);
                        parts.push_back(token);
                        tileData.erase(0, pos + 1);
                    }
                    parts.push_back(tileData); // 添加最后一部分
                    
                    if (parts.size() >= 7) {
                        int tileX = std::stoi(parts[0]);
                        int tileY = std::stoi(parts[1]);
                        std::string tileName = parts[2];
                        bool hasCollision = (parts[3] == "1");
                        bool isTransparent = (parts[4] == "1");
                        bool isDestructible = (parts[5] == "1");
                        int rotationValue = std::stoi(parts[6]);
                        
                        // 确定贴图路径（这里可以根据方块名称确定）
                        std::string texturePath = "assets/tiles/grassland.bmp";
                        if (tileName != "Grassland") {
                            texturePath = "assets/tiles/" + tileName + ".bmp";
                        }
                        
                        // 创建方块
                        auto tile = std::make_unique<Tile>(
                            tileName,
                            texturePath,
                            hasCollision,
                            isTransparent,
                            isDestructible,
                            0, 0, // 位置会在addTile中设置
                            tileSize
                        );
                        
                        // 设置旋转角度
                        TileRotation rotation = static_cast<TileRotation>(rotationValue);
                        tile->setRotation(rotation);
                        
                        // 添加到网格
                        grid->addTile(std::move(tile), tileX, tileY);
                        tilesLoaded++;
                    }
                }
            }
            // 处理实体信息（未来实现）
            else if (section == "[ENTITIES]") {
                // 这里将来可以添加实体的加载逻辑
                // 例如：if (line.substr(0, 7) == "entity=") { ... }
            }
        }
        
        file.close();
        // std::cout << "  网格加载完成: " << gridName << "，加载了 " << tilesLoaded << " 个方块" << std::endl;
        
        // 立即初始化网格的纹理，确保渲染时可用
        if (renderer) {
            // std::cout << "  立即初始化网格纹理" << std::endl;
            grid->initializeTextures(renderer);
        } else {
            std::cerr << "  警告：无法初始化网格纹理，渲染器为空" << std::endl;
        }
        
        return grid;
    } catch (const std::exception& e) {
        std::cerr << "加载网格文件失败: " << filePath << " - " << e.what() << std::endl;
        // 加载失败时返回一个新生成的网格作为备用
        // std::cout << "  加载失败，使用generateNewGrid作为备用" << std::endl;
        auto grid = generateNewGrid(gridX, gridY);
        
        // 立即初始化网格的纹理
        if (renderer && grid) {
            // std::cout << "  立即初始化备用网格纹理" << std::endl;
            grid->initializeTextures(renderer);
        }
        
        return grid;
    }
    */
}

void Map::worldToGridCoord(float worldX, float worldY, int& gridX, int& gridY) {
    // 使用常量来定义网格和方块大小
    int gridSize = GameConstants::MAP_GRID_SIZE;
    int tileSize = GameConstants::TILE_SIZE;
    int totalGridSize = gridSize * tileSize;
    
    // 计算网格坐标
    gridX = static_cast<int>(floor(worldX / totalGridSize));
    gridY = static_cast<int>(floor(worldY / totalGridSize));
}

void Map::gridCoordToWorld(int gridX, int gridY, int& worldX, int& worldY) {
    // 使用常量来定义网格和方块大小
    int gridSize = GameConstants::MAP_GRID_SIZE;
    int tileSize = GameConstants::TILE_SIZE;
    int totalGridSize = gridSize * tileSize;
    
    // 计算世界坐标（网格左下角）
    worldX = gridX * totalGridSize;
    worldY = gridY * totalGridSize;
}
