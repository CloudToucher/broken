#pragma once
#ifndef HUD_H
#define HUD_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <iomanip> // 添加用于格式化浮点数
#include <sstream> // 添加用于字符串流
#include "EntityState.h"

// 声明formatFloat函数
std::string formatFloat(float value);

class Action;

class HUD {
private:
    SDL_Color bgColor;    // 背景颜色
    SDL_Color textColor;  // 文本颜色
    SDL_FRect exitButton; // 退出按钮区域
    TTF_Font* ammoFont;   // 弹药显示字体
    TTF_Font* coordFont;  // 坐标显示字体

public:
    HUD();
    ~HUD(); // 添加析构函数释放字体资源
    
    // 初始化字体
    bool initFont();
    
    // 渲染HUD
    void render(SDL_Renderer* renderer, int health, int currentAmmo = 0, int maxAmmo = 0);
    
    // 渲染游戏倍率信息
    void renderTimeScale(SDL_Renderer* renderer, float timeScale);
    
    // 渲染坐标信息
    void renderCoordinates(SDL_Renderer* renderer, float worldX, float worldY);
    
    // 渲染行动进度条
    void renderActionProgress(SDL_Renderer* renderer, Action* currentAction, float duration, float elapsed);
    
    // 渲染碰撞系统调试信息
    void renderCollisionDebugInfo(SDL_Renderer* renderer, class Entity* entity);
    
    // 渲染连击信息
    void renderComboInfo(SDL_Renderer* renderer, class Player* player);
    
    // 检查是否点击了退出按钮
    bool isExitButtonClicked(int mouseX, int mouseY);
};


#endif // HUD_H