#include "Tile.h"
#include <iostream>
#include <algorithm>
#include "Game.h"

std::unordered_map<std::string, SDL_Texture*> Tile::textureCache;
std::mutex Tile::textureCacheMutex;

Tile::Tile(const std::string& tileName, const std::string& texPath, bool collision, 
         bool transparent, bool destructible, int posX, int posY, int tileSize, float tileMoveCost) 
    : name(tileName), texturePath(texPath), texture(nullptr), 
      hasCollision(collision), isTransparent(transparent), isDestructible(destructible),
      rotation(TileRotation::ROTATION_0), moveCost(tileMoveCost), x(posX), y(posY), size(tileSize), textureFromCache(false) {
    
    // 根据属性自动添加适当的碰撞箱
    if (hasCollision) {
        addTerrainCollider(); // 添加地形碰撞箱（填满整个tile）
    }
    
    if (!isTransparent) {
        addVisionCollider(); // 如果不透明，添加视线碰撞箱（填满整个tile）
    }
    // 注意：transparent=true意味着不阻挡视线，所以不添加视线碰撞箱
}

Tile::~Tile() {
    // 释放纹理资源
    if (texture && !textureFromCache) {
        // 只销毁不是从缓存中获取的纹理
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
    
    // colliders会自动清理（使用unique_ptr）
}

// 新增：碰撞箱管理方法
void Tile::addCollider(std::unique_ptr<Collider> collider) {
    if (collider) {
        colliders.push_back(std::move(collider));
    }
}

void Tile::removeCollider(size_t index) {
    if (index < colliders.size()) {
        colliders.erase(colliders.begin() + index);
    }
}

void Tile::clearColliders() {
    colliders.clear();
}

// 按用途获取碰撞箱
std::vector<Collider*> Tile::getCollidersByPurpose(ColliderPurpose purpose) const {
    std::vector<Collider*> result;
    for (const auto& collider : colliders) {
        if (collider->getPurpose() == purpose) {
            result.push_back(collider.get());
        }
    }
    return result;
}

// 检查是否有指定用途的碰撞箱
bool Tile::hasColliderWithPurpose(ColliderPurpose purpose) const {
    for (const auto& collider : colliders) {
        if (collider->getPurpose() == purpose) {
            return true;
        }
    }
    return false;
}

// 便捷方法：添加地形碰撞箱
void Tile::addTerrainCollider() {
    auto terrainCollider = std::make_unique<Collider>(
        static_cast<float>(x), static_cast<float>(y), 
        static_cast<float>(size), static_cast<float>(size), 
        "terrain_" + name, ColliderPurpose::TERRAIN, 1
    );
    addCollider(std::move(terrainCollider));
}

// 便捷方法：添加视线碰撞箱（如果不透明）
void Tile::addVisionCollider() {
    auto visionCollider = std::make_unique<Collider>(
        static_cast<float>(x), static_cast<float>(y), 
        static_cast<float>(size), static_cast<float>(size), 
        "vision_" + name, ColliderPurpose::VISION, 2
    );
    addCollider(std::move(visionCollider));
}

// 便捷方法：移除指定用途的所有碰撞箱
void Tile::removeCollidersByPurpose(ColliderPurpose purpose) {
    colliders.erase(
        std::remove_if(colliders.begin(), colliders.end(),
            [purpose](const std::unique_ptr<Collider>& collider) {
                return collider->getPurpose() == purpose;
            }),
        colliders.end()
    );
}

bool Tile::initializeTexture(SDL_Renderer* renderer) {
    // 如果纹理已经初始化，则不需要再次初始化
    if (texture) {
        return true;
    }
    
    // 检查纹理缓存
    {
        std::lock_guard<std::mutex> lock(textureCacheMutex);
        auto it = textureCache.find(texturePath);
        if (it != textureCache.end()) {
            texture = it->second;
            textureFromCache = true; // 标记为从缓存中获取的纹理
            return true;
        }
    }
    
    // 加载贴图
    SDL_Surface* surface = nullptr;
    bool loadFailed = false;
    
    // 尝试加载原始纹理
    surface = SDL_LoadBMP(texturePath.c_str());
    if (!surface) {
        std::cerr << "无法加载贴图: " << texturePath << " - " << SDL_GetError() << std::endl;
        loadFailed = true;
        
        // 尝试加载备用纹理
        std::cout << "  尝试加载备用纹理: assets/tiles/default.bmp" << std::endl;
        surface = SDL_LoadBMP("assets/tiles/default.bmp");
        if (!surface) {
            std::cout << "  备用纹理加载失败，创建1x1像素纯色表面" << std::endl;
            // 如果备用纹理也加载失败，创建一个1x1像素的纯色表面作为最后的备用
            surface = SDL_CreateSurface(1, 1, SDL_PIXELFORMAT_RGBA8888);
            if (!surface) {
                std::cerr << "  无法创建备用表面: " << SDL_GetError() << std::endl;
                return false; // 完全失败
            }
        } else {
            std::cout << "  备用纹理加载成功" << std::endl;
        }
    }
    
    // 创建贴图
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    
    if (!texture) {
        std::cerr << "  无法创建纹理: " << SDL_GetError() << std::endl;
        return false; // 纹理创建失败
    }
    
    // 只有成功加载原始纹理时才将其添加到缓存
    if (!loadFailed) {
        std::lock_guard<std::mutex> lock(textureCacheMutex);
        textureCache[texturePath] = texture;
        textureFromCache = true; // 标记为已添加到缓存
    }
    
    return true; // 初始化成功
}

void Tile::render(SDL_Renderer* renderer, int cameraX, int cameraY) {
    // 计算屏幕坐标
    int screenX = x - cameraX;
    int screenY = y - cameraY;
    
    // 检查是否在屏幕范围内
    int windowWidth, windowHeight;
    SDL_GetRenderOutputSize(renderer, &windowWidth, &windowHeight);
    
    // 获取当前缩放级别
    float zoomLevel = Game::getInstance()->getZoomLevel();
    
    // 考虑缩放因素进行可见性检查
    if (screenX + size < 0 || screenX > windowWidth / zoomLevel || 
        screenY + size < 0 || screenY > windowHeight / zoomLevel) {
        return; // 不在屏幕范围内，不需要渲染
    }
    
    // 设置渲染区域
    SDL_FRect dstRect = {
        static_cast<float>(screenX),
        static_cast<float>(screenY),
        static_cast<float>(size),
        static_cast<float>(size)
    };
    
    // 如果贴图未初始化，尝试初始化
    bool initSuccess = true;
    if (!texture) {
        initSuccess = initializeTexture(renderer);
        if (!initSuccess) {
            // 记录初始化失败的信息
            static bool loggedError = false;
            if (!loggedError) {
                std::cerr << "方块纹理初始化失败: " << name << " 在位置 (" << x << ", " << y << ")" << std::endl;
                loggedError = true; // 只记录一次，避免日志过多
            }
        }
    }
    // 如果贴图已初始化，则渲染贴图
    if (texture) {
        double angle = static_cast<double>(static_cast<int>(rotation));
        SDL_RenderTextureRotated(renderer, texture, nullptr, &dstRect, angle, nullptr, SDL_FLIP_NONE);
    } else {
        // 如果仍然没有贴图，渲染一个彩色矩形作为占位符
        if (initSuccess) {
            // 黄色，表示纹理未加载但初始化过程没有报错
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        } else {
            // 红色，表示初始化失败
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        }
        SDL_RenderFillRect(renderer, &dstRect);
    }
}

void Tile::setRotation(TileRotation newRotation) {
    rotation = newRotation;
}

void Tile::setPosition(int posX, int posY) {
    x = posX;
    y = posY;
    
    // 更新所有碰撞箱的位置
    for (auto& collider : colliders) {
        collider->updatePosition(static_cast<float>(posX), static_cast<float>(posY));
    }
}

// 调试用：渲染所有碰撞箱
void Tile::renderColliders(SDL_Renderer* renderer, float cameraX, float cameraY) const {
    for (const auto& collider : colliders) {
        if (collider) {
            collider->render(renderer, cameraX, cameraY);
        }
    }
}

void Tile::clearTextureCache() {
    std::lock_guard<std::mutex> lock(textureCacheMutex);
    // 在清除缓存前记录当前缓存大小
    size_t cacheSize = textureCache.size();
    if (cacheSize > 0) {
        std::cout << "清除纹理缓存，当前缓存大小: " << cacheSize << std::endl;
        
        // 销毁所有缓存的纹理
        for (auto& pair : textureCache) {
            SDL_DestroyTexture(pair.second);
        }
        textureCache.clear();
        
        std::cout << "纹理缓存已清空" << std::endl;
    }
}