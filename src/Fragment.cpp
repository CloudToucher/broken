#include "Fragment.h"
#include "Entity.h"
#include "Game.h"
#include "Map.h"
#include <cmath>
#include <random>
#include <algorithm>

// 定义数学常量
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// =============================================================================
// Fragment 类实现
// =============================================================================

Fragment::Fragment(float startX, float startY, float dirX, float dirY, 
                 float speed, float range, int damageValue, Entity* fragmentOwner)
    : x(startX), y(startY), startX(startX), startY(startY), 
      dirX(dirX), dirY(dirY), speed(speed), maxRange(range),
      traveledDistance(0.0f), active(true), owner(fragmentOwner),
      size(2.0f), lifetime(0.0f), maxLifetime(5.0f),
      gravity(98.0f), velocityY(0.0f), drag(0.1f) {
    
    // 设置弹片伤害
    damage.addDamage(DamageType::PIERCE, damageValue, 2); // 刺击伤害，2点穿透
    damage.setSource(fragmentOwner);
    
    // 设置随机颜色（橙红色系）
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> colorDist(200, 255);
    
    color.r = colorDist(gen);
    color.g = colorDist(gen) / 2; // 偏红色
    color.b = 0;
    color.a = 255;
    
    // 归一化方向向量
    float length = std::sqrt(dirX * dirX + dirY * dirY);
    if (length > 0) {
        this->dirX = dirX / length;
        this->dirY = dirY / length;
    }
}

void Fragment::update(float deltaTime) {
    if (!active) return;
    
    // 更新生存时间
    lifetime += deltaTime;
    if (lifetime >= maxLifetime) {
        active = false;
        return;
    }
    
    // 应用物理效果（只应用阻力，不应用重力）
    applyPhysics(deltaTime);
    
    // 俯瞰视角：按初始方向匀速直线运动
    float newX = x + dirX * speed * deltaTime;
    float newY = y + dirY * speed * deltaTime;
    
    // 更新已飞行距离
    float dx = newX - x;
    float dy = newY - y;
    float frameDistance = std::sqrt(dx * dx + dy * dy);
    traveledDistance += frameDistance;
    
    // 检查是否超出最大射程
    if (traveledDistance >= maxRange) {
        active = false;
        return;
    }
    
    // 更新位置
    x = newX;
    y = newY;
}

void Fragment::applyPhysics(float deltaTime) {
    // 俯瞰视角：不应用重力，只应用轻微的空气阻力
    speed *= (1.0f - drag * deltaTime * 0.1f); // 减少阻力影响
    if (speed < 50.0f) { // 提高最低速度阈值
        active = false;
        return;
    }
}

void Fragment::render(SDL_Renderer* renderer, int cameraX, int cameraY) {
    if (!active) return;
    
    // 计算屏幕坐标
    int screenX = static_cast<int>(x - cameraX);
    int screenY = static_cast<int>(y - cameraY);
    
    // 根据生存时间调整透明度
    float alpha = 1.0f - (lifetime / maxLifetime);
    alpha = std::max(0.0f, std::min(1.0f, alpha));
    
    SDL_Color renderColor = color;
    renderColor.a = static_cast<Uint8>(255 * alpha);
    
    // 设置渲染颜色
    SDL_SetRenderDrawColor(renderer, renderColor.r, renderColor.g, renderColor.b, renderColor.a);
    
    // 渲染弹片（小圆点）
    int radius = static_cast<int>(size);
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w; 
            int dy = radius - h; 
            if ((dx*dx + dy*dy) <= (radius * radius)) {
                SDL_RenderPoint(renderer, screenX + dx, screenY + dy);
            }
        }
    }
    
    // 可选：绘制拖尾效果
    if (speed > 50.0f) {
        float trailLength = std::min(20.0f, speed * 0.1f);
        int trailX = static_cast<int>(x - dirX * trailLength - cameraX);
        int trailY = static_cast<int>(y - dirY * trailLength - cameraY);
        
        SDL_SetRenderDrawColor(renderer, renderColor.r / 2, renderColor.g / 2, renderColor.b / 2, renderColor.a / 2);
        SDL_RenderLine(renderer, screenX, screenY, trailX, trailY);
    }
}

bool Fragment::checkTerrainCollision(const std::vector<Collider>& terrainColliders) {
    if (!active) return false;
    
    for (const auto& collider : terrainColliders) {
        if (collider.getPurpose() == ColliderPurpose::TERRAIN && 
            collider.getIsActive() &&
            checkPointCollision(x, y, collider)) {
            printf("弹片命中地形碰撞箱\n");
            handleCollision();
            return true;
        }
    }
    return false;
}

bool Fragment::checkEntityCollision(const std::vector<Entity*>& entities) {
    if (!active) return false;
    
    for (Entity* entity : entities) {
        if (!entity) continue;
        
        // 弹片伤害所有实体，包括拥有者自己
        const Collider& entityCollider = entity->getCollider();
        if (checkPointCollision(x, y, entityCollider)) {
            // 造成伤害
            entity->takeDamage(damage);
            printf("弹片命中实体，造成%d点伤害\n", damage.getTotalDamage());
            handleCollision();
            return true;
        }
    }
    return false;
}

bool Fragment::checkPointCollision(float px, float py, const Collider& collider) const {
    if (!collider.getIsActive()) return false;
    
    if (collider.getType() == ColliderType::CIRCLE) {
        float dx = px - collider.getCircleX();
        float dy = py - collider.getCircleY();
        return (dx * dx + dy * dy) <= (collider.getRadius() * collider.getRadius());
    } else if (collider.getType() == ColliderType::BOX) {
        const SDL_FRect& box = collider.getBoxCollider();
        return (px >= box.x && px <= box.x + box.w && 
                py >= box.y && py <= box.y + box.h);
    }
    return false;
}

void Fragment::handleCollision() {
    active = false;
    
    // 可以在这里添加碰撞效果，比如火花、声音等
    printf("弹片在位置(%.1f, %.1f)碰撞并消失，飞行距离%.1f/%.1f\n", x, y, traveledDistance, maxRange);
}

float Fragment::getDistanceToTarget(float targetX, float targetY) const {
    float dx = x - targetX;
    float dy = y - targetY;
    return std::sqrt(dx * dx + dy * dy);
}

bool Fragment::isInRange(float targetX, float targetY, float range) const {
    return getDistanceToTarget(targetX, targetY) <= range;
}

// =============================================================================
// FragmentManager 类实现
// =============================================================================

FragmentManager* FragmentManager::instance = nullptr;

FragmentManager& FragmentManager::getInstance() {
    if (!instance) {
        instance = new FragmentManager();
    }
    return *instance;
}

void FragmentManager::destroyInstance() {
    if (instance) {
        delete instance;
        instance = nullptr;
    }
}

void FragmentManager::addFragment(std::unique_ptr<Fragment> fragment) {
    if (fragment) {
        fragments.push_back(std::move(fragment));
    }
}

void FragmentManager::update(float deltaTime) {
    updateFragments(deltaTime);
    
    // 获取游戏实例进行碰撞检测
    Game* game = Game::getInstance();
    if (game) {
        // 获取地形碰撞器
        std::vector<Collider> terrainColliders;
        if (game->getMap()) {
            terrainColliders = game->getMap()->getObstacles();
        }
        
        // 获取所有实体
        std::vector<Entity*> allEntities;
        
        // 添加玩家
        if (game->getPlayer()) {
            allEntities.push_back(game->getPlayer());
        }
        
        // 添加所有丧尸
        const auto& zombies = game->getZombies();
        for (const auto& zombie : zombies) {
            if (zombie && zombie->getHealth() > 0) {
                allEntities.push_back(zombie.get());
            }
        }
        
        // 添加所有生物
        const auto& creatures = game->getCreatures();
        for (const auto& creature : creatures) {
            if (creature && creature->getHealth() > 0) {
                allEntities.push_back(creature.get());
            }
        }
        
        // 执行碰撞检测
        checkFragmentCollisions(terrainColliders, allEntities);
    }
    
    clearInactiveFragments();
}

void FragmentManager::render(SDL_Renderer* renderer, int cameraX, int cameraY) {
    renderFragments(renderer, cameraX, cameraY);
}

void FragmentManager::updateFragments(float deltaTime) {
    for (auto& fragment : fragments) {
        if (fragment) {
            fragment->update(deltaTime);
        }
    }
}

void FragmentManager::renderFragments(SDL_Renderer* renderer, int cameraX, int cameraY) {
    for (const auto& fragment : fragments) {
        if (fragment && fragment->isActive()) {
            fragment->render(renderer, cameraX, cameraY);
        }
    }
}

void FragmentManager::clearInactiveFragments() {
    fragments.erase(
        std::remove_if(fragments.begin(), fragments.end(),
                      [](const std::unique_ptr<Fragment>& fragment) {
                          return !fragment || !fragment->isActive();
                      }),
        fragments.end()
    );
}

void FragmentManager::clearAllFragments() {
    fragments.clear();
}

void FragmentManager::checkFragmentCollisions(const std::vector<Collider>& terrainColliders, 
                                            const std::vector<Entity*>& entities) {
    for (auto& fragment : fragments) {
        if (fragment && fragment->isActive()) {
            // 先检查地形碰撞
            if (fragment->checkTerrainCollision(terrainColliders)) {
                continue; // 如果撞到地形，就不再检查实体碰撞
            }
            
            // 检查实体碰撞
            fragment->checkEntityCollision(entities);
        }
    }
}

size_t FragmentManager::getActiveFragmentCount() const {
    return std::count_if(fragments.begin(), fragments.end(),
                        [](const std::unique_ptr<Fragment>& fragment) {
                            return fragment && fragment->isActive();
                        });
}

bool FragmentManager::hasActiveFragments() const {
    return getActiveFragmentCount() > 0;
}

void FragmentManager::createExplosionFragments(float centerX, float centerY, int fragmentCount, 
                                             float minSpeed, float maxSpeed, float range, 
                                             int damagePerFragment, Entity* owner) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * M_PI);
    std::uniform_real_distribution<float> speedDist(minSpeed, maxSpeed);
    
    for (int i = 0; i < fragmentCount; i++) {
        // 随机方向
        float angle = angleDist(gen);
        float dirX = std::cos(angle);
        float dirY = std::sin(angle);
        
        // 随机速度
        float fragmentSpeed = speedDist(gen);
        
        // 创建弹片
        auto fragment = std::make_unique<Fragment>(
            centerX, centerY, dirX, dirY, fragmentSpeed, range, damagePerFragment, owner
        );
        
        // 添加一些随机变化
        fragment->setSize(1.0f + (std::rand() % 3)); // 1-3像素大小
        fragment->setGravity(0.0f); // 俯瞰视角不需要重力
        fragment->setDrag(0.02f + (std::rand() % 5) * 0.01f); // 0.02-0.06阻力
        
        addFragment(std::move(fragment));
    }
    
    printf("创建了%d个爆炸弹片，中心位置(%.1f, %.1f)\n", fragmentCount, centerX, centerY);
}

void FragmentManager::debugPrintFragmentInfo() const {
    printf("=== 弹片管理器状态 ===\n");
    printf("总弹片数: %zu\n", fragments.size());
    printf("活跃弹片数: %zu\n", getActiveFragmentCount());
    
    int count = 0;
    for (const auto& fragment : fragments) {
        if (fragment && fragment->isActive() && count < 5) { // 只显示前5个
            printf("  弹片%d: 位置(%.1f,%.1f), 距离%.1f/%.1f\n", 
                   count, fragment->getX(), fragment->getY(),
                   fragment->getTraveledDistance(), fragment->getMaxRange());
            count++;
        }
    }
    
    if (getActiveFragmentCount() > 5) {
        printf("  ... 还有%zu个活跃弹片\n", getActiveFragmentCount() - 5);
    }
    
    printf("=====================\n");
} 