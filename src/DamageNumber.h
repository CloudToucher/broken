#pragma once
#ifndef DAMAGE_NUMBER_H
#define DAMAGE_NUMBER_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>

// 飘字类型枚举
enum class DamageNumberType {
    DAMAGE,     // 普通伤害（红色）
    CRITICAL,   // 暴击伤害（黄色/橙色）
    MISS        // 未命中（蓝色）
};

// 伤害数字飘字类
class DamageNumber {
private:
    float x, y;                 // 飘字位置
    float velocityX, velocityY; // 飘字速度
    int damage;                 // 伤害数值
    DamageNumberType type;      // 飘字类型
    float lifeTime;             // 生存时间（秒）
    float maxLifeTime;          // 最大生存时间（秒）
    float alpha;                // 透明度
    SDL_Color color;            // 颜色
    std::string text;           // 显示文本
    
    static TTF_Font* font;      // 共享字体
    static bool fontInitialized; // 字体是否已初始化
    
    // 辅助方法
    void setupColorAndText();   // 设置颜色和文本

public:
    // 构造函数
    DamageNumber(float startX, float startY, int damageValue, bool critical = false);
    
    // 新构造函数：支持miss显示
    DamageNumber(float startX, float startY, DamageNumberType numberType, int damageValue = 0);
    
    // 析构函数
    ~DamageNumber() = default;
    
    // 静态方法：初始化字体
    static bool initFont(TTF_Font* gameFont);
    
    // 静态方法：清理字体资源
    static void cleanupFont();
    
    // 更新飘字状态
    void update(float deltaTime);
    
    // 渲染飘字
    void render(SDL_Renderer* renderer, float cameraX, float cameraY);
    
    // 检查是否应该被销毁
    bool shouldDestroy() const { return lifeTime <= 0.0f; }
    
    // 获取位置
    float getX() const { return x; }
    float getY() const { return y; }
    
    // 获取伤害值
    int getDamage() const { return damage; }
    
    // 获取飘字类型
    DamageNumberType getType() const { return type; }
};

#endif // DAMAGE_NUMBER_H 