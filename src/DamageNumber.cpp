#include "DamageNumber.h"
#include <random>
#include <cmath>

// 静态成员变量初始化
TTF_Font* DamageNumber::font = nullptr;
bool DamageNumber::fontInitialized = false;

// 构造函数（兼容旧版本）
DamageNumber::DamageNumber(float startX, float startY, int damageValue, bool critical)
    : x(startX), y(startY), damage(damageValue), 
      type(critical ? DamageNumberType::CRITICAL : DamageNumberType::DAMAGE),
      maxLifeTime(2.0f), lifeTime(maxLifeTime), alpha(255.0f) {
    
    // 设置飘字移动速度 - 改为短暂向上，然后快速停止
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> horizontalDis(-20.0f, 20.0f);
    std::uniform_real_distribution<float> verticalDis(-60.0f, -30.0f);
    
    velocityX = horizontalDis(gen);
    velocityY = verticalDis(gen);
    
    // 设置颜色和文本
    setupColorAndText();
}

// 新构造函数：支持miss显示
DamageNumber::DamageNumber(float startX, float startY, DamageNumberType numberType, int damageValue)
    : x(startX), y(startY), damage(damageValue), type(numberType),
      maxLifeTime(2.0f), lifeTime(maxLifeTime), alpha(255.0f) {
    
    // 设置飘字移动速度
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> horizontalDis(-20.0f, 20.0f);
    std::uniform_real_distribution<float> verticalDis(-60.0f, -30.0f);
    
    velocityX = horizontalDis(gen);
    velocityY = verticalDis(gen);
    
    // 设置颜色和文本
    setupColorAndText();
}

// 设置颜色和文本的辅助方法
void DamageNumber::setupColorAndText() {
    switch (type) {
        case DamageNumberType::CRITICAL:
            // 暴击：红色 + 大一号
            color = {255, 50, 50, 255};
            maxLifeTime = 3.0f; // 暴击持续时间更长
            lifeTime = maxLifeTime;
            velocityY *= 0.8f; // 暴击移动更慢，更醒目
            text = std::to_string(damage) + "!"; // 暴击加感叹号
            break;
            
        case DamageNumberType::DAMAGE:
            // 普通伤害：金黄色
            color = {255, 215, 0, 255};
            maxLifeTime = 2.0f;
            lifeTime = maxLifeTime;
            text = std::to_string(damage);
            break;
            
        case DamageNumberType::MISS:
            // 未命中：醒目的白色
            color = {255, 255, 255, 255};
            maxLifeTime = 2.0f; // miss显示时间延长
            lifeTime = maxLifeTime;
            text = "MISS";
            // 增大初始速度，让miss更明显
            velocityY *= 1.2f;
            velocityX *= 1.2f;
            break;
    }
}

// 静态方法：初始化字体
bool DamageNumber::initFont(TTF_Font* gameFont) {
    // 优先使用指定的像素字体
    font = TTF_OpenFont("assets/outline_pixel-7_solid.ttf", 22);
    if (!font) {
        // 如果加载失败，回退到游戏字体
        if (gameFont) {
            font = gameFont;
        } else {
            return false;
        }
    }
    fontInitialized = true;
    return true;
}

// 静态方法：清理字体资源
void DamageNumber::cleanupFont() {
    font = nullptr;
    fontInitialized = false;
}

// 更新飘字状态
void DamageNumber::update(float deltaTime) {
    // 减少生存时间
    lifeTime -= deltaTime;
    
    // 改进的运动逻辑：短暂向上，然后快速停止
    float lifeRatio = lifeTime / maxLifeTime;
    
    if (lifeRatio > 0.6f) {
        // 前40%时间：向上移动
        x += velocityX * deltaTime;
        y += velocityY * deltaTime;
        
        // 快速衰减速度
        velocityX *= 0.90f;
        velocityY *= 0.92f;
    } else {
        // 后60%时间：基本停止移动，只是轻微飘动
        velocityX *= 0.95f;
        velocityY *= 0.95f;
        
        x += velocityX * deltaTime * 0.3f; // 大幅减少移动
        y += velocityY * deltaTime * 0.3f;
    }
    
    // 计算透明度（基于剩余生存时间）
    if (lifeRatio > 0.8f) {
        // 开始阶段：完全不透明
        alpha = 255.0f;
    } else if (lifeRatio > 0.4f) {
        // 中间阶段：保持可见
        alpha = 255.0f;
    } else {
        // 结束阶段：逐渐透明
        alpha = 255.0f * (lifeRatio / 0.4f);
    }
    
    // 确保透明度在有效范围内
    alpha = std::max(0.0f, std::min(255.0f, alpha));
}

// 渲染飘字
void DamageNumber::render(SDL_Renderer* renderer, float cameraX, float cameraY) {
    if (!fontInitialized || !font) {
        return;
    }
    
    // 计算屏幕坐标
    int screenX = static_cast<int>(x - cameraX);
    int screenY = static_cast<int>(y - cameraY);
    
    // 设置颜色和透明度
    SDL_Color renderColor = color;
    renderColor.a = static_cast<Uint8>(alpha);
    
    // 创建文本表面
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, text.c_str(), 0, renderColor);
    if (!textSurface) {
        return;
    }
    
    // 创建纹理
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!textTexture) {
        SDL_DestroySurface(textSurface);
        return;
    }
    
    // 设置纹理的透明度
    SDL_SetTextureAlphaMod(textTexture, renderColor.a);
    
    // 获取文本尺寸
    int textWidth = textSurface->w;
    int textHeight = textSurface->h;
    
    // 计算渲染位置（居中）
    SDL_FRect destRect = {
        static_cast<float>(screenX - textWidth / 2),
        static_cast<float>(screenY - textHeight / 2),
        static_cast<float>(textWidth),
        static_cast<float>(textHeight)
    };
    
    // 如果是暴击，放大文字（大一号）
    if (type == DamageNumberType::CRITICAL) {
        float scale = 1.5f; // 固定放大，不要动画
        destRect.w *= scale;
        destRect.h *= scale;
        destRect.x -= (destRect.w - textWidth) / 2;
        destRect.y -= (destRect.h - textHeight) / 2;
    }
    
    // 如果是MISS，也放大文字并添加描边效果
    if (type == DamageNumberType::MISS) {
        float scale = 1.3f; // MISS放大
        destRect.w *= scale;
        destRect.h *= scale;
        destRect.x -= (destRect.w - textWidth) / 2;
        destRect.y -= (destRect.h - textHeight) / 2;
        
        // 渲染黑色描边（创建描边效果）
        SDL_Color blackColor = {0, 0, 0, renderColor.a};
        SDL_Surface* blackSurface = TTF_RenderText_Blended(font, text.c_str(), 0, blackColor);
        if (blackSurface) {
            SDL_Texture* blackTexture = SDL_CreateTextureFromSurface(renderer, blackSurface);
            if (blackTexture) {
                SDL_SetTextureAlphaMod(blackTexture, renderColor.a);
                
                // 渲染8个方向的描边
                for (int dx = -1; dx <= 1; dx++) {
                    for (int dy = -1; dy <= 1; dy++) {
                        if (dx == 0 && dy == 0) continue; // 跳过中心
                        SDL_FRect outlineRect = destRect;
                        outlineRect.x += dx * 2;
                        outlineRect.y += dy * 2;
                        SDL_RenderTexture(renderer, blackTexture, nullptr, &outlineRect);
                    }
                }
                SDL_DestroyTexture(blackTexture);
            }
            SDL_DestroySurface(blackSurface);
        }
    }
    
    // 渲染文本
    SDL_RenderTexture(renderer, textTexture, nullptr, &destRect);
    
    // 清理资源
    SDL_DestroyTexture(textTexture);
    SDL_DestroySurface(textSurface);
} 