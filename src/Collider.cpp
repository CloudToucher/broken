#include "Collider.h"
#include <algorithm>
#include <cmath>
#include <limits>  // 添加limits头文件以使用std::numeric_limits

#define M_PI 3.1415
// 创建矩形碰撞体
Collider::Collider(float x, float y, float width, float height, 
                   const std::string& colliderTag,
                   ColliderPurpose colliderPurpose,
                   int colliderLayer)
    : type(ColliderType::BOX), purpose(colliderPurpose), tag(colliderTag), 
      isActive(true), layer(colliderLayer) {
    boxCollider.x = x;
    boxCollider.y = y;
    boxCollider.w = width;
    boxCollider.h = height;
    circleX = circleY = radius = 0; // 矩形碰撞体不使用圆形参数
}

// 创建圆形碰撞体
Collider::Collider(float x, float y, float r, 
                   const std::string& colliderTag,
                   ColliderPurpose colliderPurpose,
                   int colliderLayer)
    : type(ColliderType::CIRCLE), purpose(colliderPurpose), tag(colliderTag),
      isActive(true), layer(colliderLayer), circleX(x), circleY(y), radius(r) {
    // 矩形数据设为空
    boxCollider = { 0, 0, 0, 0 };
}

// 更新碰撞体位置
void Collider::updatePosition(float x, float y) {
    if (type == ColliderType::BOX) {
        boxCollider.x = x;
        boxCollider.y = y;
    } else if (type == ColliderType::CIRCLE) {
        circleX = x;
        circleY = y;
    }
}

// 检测与另一个碰撞体的碰撞
bool Collider::checkCollision(const Collider& other) const {
    // 如果任一碰撞体未激活，返回false
    if (!isActive || !other.isActive) {
        return false;
    }

    if (type == ColliderType::BOX && other.type == ColliderType::BOX) {
        // 矩形与矩形碰撞检测
        return (boxCollider.x < other.boxCollider.x + other.boxCollider.w &&
                boxCollider.x + boxCollider.w > other.boxCollider.x &&
                boxCollider.y < other.boxCollider.y + other.boxCollider.h &&
                boxCollider.y + boxCollider.h > other.boxCollider.y);
    } else if (type == ColliderType::CIRCLE && other.type == ColliderType::CIRCLE) {
        // 圆形与圆形碰撞检测
        float dx = circleX - other.circleX;
        float dy = circleY - other.circleY;
        float distance = std::sqrt(dx * dx + dy * dy);
        return distance < (radius + other.radius);
    } else {
        // 矩形与圆形碰撞检测
        const Collider* box = (type == ColliderType::BOX) ? this : &other;
        const Collider* circle = (type == ColliderType::CIRCLE) ? this : &other;

        // 找到矩形上距离圆心最近的点
        float closestX = std::max(box->boxCollider.x, 
                                  std::min(circle->circleX, box->boxCollider.x + box->boxCollider.w));
        float closestY = std::max(box->boxCollider.y, 
                                  std::min(circle->circleY, box->boxCollider.y + box->boxCollider.h));

        // 计算距离
        float dx = circle->circleX - closestX;
        float dy = circle->circleY - closestY;
        float distance = std::sqrt(dx * dx + dy * dy);

        return distance < circle->radius;
    }
}

// 新增：检测与指定用途的碰撞体的碰撞
bool Collider::checkCollisionWithPurpose(const Collider& other, ColliderPurpose targetPurpose) const {
    // 检查对方碰撞体是否为指定用途
    if (other.purpose != targetPurpose) {
        return false;
    }
    
    // 如果用途匹配，执行正常碰撞检测
    return checkCollision(other);
}

// 检查点是否在碰撞体内部
bool Collider::contains(int x, int y) const {
    return contains(static_cast<float>(x), static_cast<float>(y));
}

bool Collider::contains(float x, float y) const {
    if (!isActive) {
        return false;
    }

    if (type == ColliderType::BOX) {
        return (x >= boxCollider.x && x <= boxCollider.x + boxCollider.w &&
                y >= boxCollider.y && y <= boxCollider.y + boxCollider.h);
    } else if (type == ColliderType::CIRCLE) {
        float dx = x - circleX;
        float dy = y - circleY;
        return (dx * dx + dy * dy) <= (radius * radius);
    }
    return false;
}

// 射线检测方法
float Collider::raycast(float startX, float startY, float dirX, float dirY) const {
    if (!isActive) {
        return -1.0f; // 未激活的碰撞体不参与射线检测
    }

    if (type == ColliderType::BOX) {
        // 对矩形进行射线检测
        float tMin = 0.0f;
        float tMax = std::numeric_limits<float>::max();

        // 检测X轴
        if (std::abs(dirX) < 1e-6f) {
            // 射线与X轴平行
            if (startX < boxCollider.x || startX > boxCollider.x + boxCollider.w) {
                return -1.0f; // 不相交
            }
        } else {
            float t1 = (boxCollider.x - startX) / dirX;
            float t2 = (boxCollider.x + boxCollider.w - startX) / dirX;
            if (t1 > t2) std::swap(t1, t2);
            tMin = std::max(tMin, t1);
            tMax = std::min(tMax, t2);
            if (tMin > tMax) return -1.0f;
        }

        // 检测Y轴
        if (std::abs(dirY) < 1e-6f) {
            // 射线与Y轴平行
            if (startY < boxCollider.y || startY > boxCollider.y + boxCollider.h) {
                return -1.0f; // 不相交
            }
        } else {
            float t1 = (boxCollider.y - startY) / dirY;
            float t2 = (boxCollider.y + boxCollider.h - startY) / dirY;
            if (t1 > t2) std::swap(t1, t2);
            tMin = std::max(tMin, t1);
            tMax = std::min(tMax, t2);
            if (tMin > tMax) return -1.0f;
        }

        return (tMin >= 0) ? tMin : tMax;
    } else if (type == ColliderType::CIRCLE) {
        // 对圆形进行射线检测
        float dx = startX - circleX;
        float dy = startY - circleY;
        
        float a = dirX * dirX + dirY * dirY;
        float b = 2 * (dx * dirX + dy * dirY);
        float c = dx * dx + dy * dy - radius * radius;
        
        float discriminant = b * b - 4 * a * c;
        if (discriminant < 0) {
            return -1.0f; // 不相交
        }
        
        float t1 = (-b - std::sqrt(discriminant)) / (2 * a);
        float t2 = (-b + std::sqrt(discriminant)) / (2 * a);
        
        if (t1 >= 0) return t1;
        if (t2 >= 0) return t2;
        return -1.0f;
    }
    
    return -1.0f;
}

// 新增：射线检测（只检测指定用途的碰撞体）
float Collider::raycastWithPurpose(float startX, float startY, float dirX, float dirY, ColliderPurpose targetPurpose) const {
    // 检查碰撞体用途是否匹配
    if (purpose != targetPurpose) {
        return -1.0f;
    }
    
    // 如果用途匹配，执行正常射线检测
    return raycast(startX, startY, dirX, dirY);
}

// 新增：获取碰撞箱宽度（统一接口）
float Collider::getWidth() const {
    if (type == ColliderType::BOX) {
        return boxCollider.w;
    } else if (type == ColliderType::CIRCLE) {
        return radius * 2.0f; // 圆形的宽度为直径
    }
    return 0.0f;
}

// 新增：获取碰撞箱高度（统一接口）
float Collider::getHeight() const {
    if (type == ColliderType::BOX) {
        return boxCollider.h;
    } else if (type == ColliderType::CIRCLE) {
        return radius * 2.0f; // 圆形的高度为直径
    }
    return 0.0f;
}

// 新增：检查是否与另一个碰撞箱相交（intersects方法）
bool Collider::intersects(const Collider& other) const {
    // intersects方法就是checkCollision方法的别名
    return checkCollision(other);
}

// 调试用：渲染碰撞体（不同用途用不同颜色）
void Collider::render(SDL_Renderer* renderer, float cameraX, float cameraY) const {
    if (!isActive) {
        return; // 未激活的碰撞体不渲染
    }

    // 根据用途设置不同颜色
    SDL_Color renderColor;
    switch (purpose) {
        case ColliderPurpose::ENTITY:
            renderColor = { 255, 0, 0, 128 }; // 红色 - 实体碰撞箱
            break;
        case ColliderPurpose::TERRAIN:
            renderColor = { 0, 255, 0, 128 }; // 绿色 - 地形碰撞箱
            break;
        case ColliderPurpose::VISION:
            renderColor = { 0, 0, 255, 128 }; // 蓝色 - 视线碰撞箱
            break;
        default:
            renderColor = { 255, 255, 255, 128 }; // 白色 - 默认
            break;
    }

    SDL_SetRenderDrawColor(renderer, renderColor.r, renderColor.g, renderColor.b, renderColor.a);

    if (type == ColliderType::BOX) {
        SDL_FRect screenRect = {
            boxCollider.x - cameraX,
            boxCollider.y - cameraY,
            boxCollider.w,
            boxCollider.h
        };
        SDL_RenderFillRect(renderer, &screenRect);
    } else if (type == ColliderType::CIRCLE) {
        // 简单的圆形渲染（使用多边形近似）
        const int segments = 16;
        for (int i = 0; i < segments; ++i) {
            float angle1 = (float)i / segments * 2 * M_PI;
            float angle2 = (float)(i + 1) / segments * 2 * M_PI;
            
            float x1 = circleX + radius * std::cos(angle1) - cameraX;
            float y1 = circleY + radius * std::sin(angle1) - cameraY;
            float x2 = circleX + radius * std::cos(angle2) - cameraX;
            float y2 = circleY + radius * std::sin(angle2) - cameraY;
            
            SDL_RenderLine(renderer, (int)x1, (int)y1, (int)x2, (int)y2);
        }
    }
}