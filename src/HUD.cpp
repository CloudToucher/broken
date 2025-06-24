#include "HUD.h"
#include "Game.h"
#include "Player.h"
#include "Map.h"
#include "Action.h"
#include "Entity.h"
#include "MeleeWeapon.h"
#include <string>
#include <vector>

HUD::HUD() : ammoFont(nullptr), coordFont(nullptr) {
    bgColor = { 0, 0, 0, 128 }; // 半透明黑色
    textColor = { 255, 255, 255, 255 }; // 白色
    // 退出按钮将在render中根据窗口大小动态设置
}

HUD::~HUD() {
    if (ammoFont) {
        TTF_CloseFont(ammoFont);
        ammoFont = nullptr;
    }
    if (coordFont) {
        TTF_CloseFont(coordFont);
        coordFont = nullptr;
    }
}

bool HUD::initFont() {
    // 加载弹药显示字体
    ammoFont = TTF_OpenFont("assets/outline_pixel-7_solid.ttf", 24);
    // 加载坐标显示字体
    coordFont = TTF_OpenFont("assets/outline_pixel-7_solid.ttf", 18);
    return ammoFont != nullptr && coordFont != nullptr;
}

void HUD::render(SDL_Renderer* renderer, int health, int currentAmmo, int maxAmmo) {
    // 获取窗口尺寸
    int windowWidth = Game::getInstance()->getWindowWidth();
    int windowHeight = Game::getInstance()->getWindowHeight();

    // 绘制生命值背景
    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_FRect healthBg = { static_cast<float>(windowWidth - 110), static_cast<float>(windowHeight - 40), 100.0f, 30.0f };
    SDL_RenderFillRect(renderer, &healthBg);

    // 绘制生命值条
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // 红色生命条
    SDL_FRect healthBar = { static_cast<float>(windowWidth - 105), static_cast<float>(windowHeight - 35), static_cast<float>(health * 90 / 100), 20.0f };
    SDL_RenderFillRect(renderer, &healthBar);
    
    // 绘制退出按钮（右上角）
    exitButton = { static_cast<float>(windowWidth - 50), 10.0f, 40.0f, 40.0f };
    SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255); // 红色按钮背景
    SDL_RenderFillRect(renderer, &exitButton);
    
    // 绘制X形状
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // 白色X
    // 绘制"\"线
    SDL_RenderLine(renderer, 
        static_cast<int>(exitButton.x + 10), static_cast<int>(exitButton.y + 10),
        static_cast<int>(exitButton.x + exitButton.w - 10), static_cast<int>(exitButton.y + exitButton.h - 10));
    // 绘制"/"线
    SDL_RenderLine(renderer, 
        static_cast<int>(exitButton.x + 10), static_cast<int>(exitButton.y + exitButton.h - 10),
        static_cast<int>(exitButton.x + exitButton.w - 10), static_cast<int>(exitButton.y + 10));
    
    // 绘制弹药信息（右下角）
    if (ammoFont && (maxAmmo > 0 || currentAmmo > 0)) {
        // 创建弹药文本
        std::string ammoText = std::to_string(currentAmmo) + "/" + std::to_string(maxAmmo);
        
        // 设置文本颜色（橘黄色）
        SDL_Color ammoColor = { 255, 165, 0, 255 }; // 橘黄色
        
        // 创建文本表面 - 使用SDL3的API
        SDL_Surface* textSurface = TTF_RenderText_Solid(ammoFont, ammoText.c_str(), 0, ammoColor);
        if (textSurface) {
            // 创建纹理
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            if (textTexture) {
                // 设置渲染位置（右下角）
                SDL_FRect textRect = { 
                    static_cast<float>(windowWidth - 110), 
                    static_cast<float>(windowHeight - 80), 
                    static_cast<float>(textSurface->w), 
                    static_cast<float>(textSurface->h) 
                };
                
                // 渲染文本
                SDL_RenderTexture(renderer, textTexture, nullptr, &textRect);
                
                // 释放纹理
                SDL_DestroyTexture(textTexture);
            }
            
            // 释放表面
            SDL_DestroySurface(textSurface);
        }
    }
    
    // 渲染游戏倍率信息
    renderTimeScale(renderer, Game::getInstance()->getTimeScale());
    
    // 获取玩家位置并渲染坐标信息
    if (Game::getInstance()->getPlayer()) {
        int playerX = Game::getInstance()->getPlayer()->getX();
        int playerY = Game::getInstance()->getPlayer()->getY();
        renderCoordinates(renderer, playerX, playerY);
        
        // 渲染碰撞系统调试信息
        renderCollisionDebugInfo(renderer, Game::getInstance()->getPlayer());
        
        // 渲染连击信息
        renderComboInfo(renderer, Game::getInstance()->getPlayer());
    }
}

// 渲染游戏时间倍率信息
void HUD::renderTimeScale(SDL_Renderer* renderer, float timeScale) {
    if (!coordFont) return;
    
    // 获取窗口尺寸
    int windowWidth = Game::getInstance()->getWindowWidth();
    int windowHeight = Game::getInstance()->getWindowHeight();
    
    // 创建时间倍率文本
    std::string timeScaleText = "游戏速度: " + formatFloat(timeScale) + "x";
    
    // 设置文本颜色（黄色）
    SDL_Color timeScaleColor = { 255, 255, 0, 255 };
    
    // 绘制背景
    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_FRect timeScaleBg = { 
        static_cast<float>(windowWidth - 300), 
        static_cast<float>(windowHeight - 150), 
        290.0f, 
        25.0f 
    };
    SDL_RenderFillRect(renderer, &timeScaleBg);
    
    // 渲染时间倍率文本
    SDL_Surface* timeScaleSurface = TTF_RenderText_Solid(coordFont, timeScaleText.c_str(), 0, timeScaleColor);
    if (timeScaleSurface) {
        SDL_Texture* timeScaleTexture = SDL_CreateTextureFromSurface(renderer, timeScaleSurface);
        if (timeScaleTexture) {
            SDL_FRect timeScaleRect = { 
                static_cast<float>(windowWidth - 290), 
                static_cast<float>(windowHeight - 145), 
                static_cast<float>(timeScaleSurface->w), 
                static_cast<float>(timeScaleSurface->h) 
            };
            SDL_RenderTexture(renderer, timeScaleTexture, nullptr, &timeScaleRect);
            SDL_DestroyTexture(timeScaleTexture);
        }
        SDL_DestroySurface(timeScaleSurface);
    }
}

// 渲染坐标信息
void HUD::renderCoordinates(SDL_Renderer* renderer, float worldX, float worldY) {
    if (!coordFont) return;
    
    // 获取窗口尺寸
    int windowWidth = Game::getInstance()->getWindowWidth();
    int windowHeight = Game::getInstance()->getWindowHeight();
    
    // 计算网格坐标
    int gridX, gridY;
    Map::worldToGridCoord(worldX, worldY, gridX, gridY);
    
    // 计算方块坐标
    int gridSize = 16; // 网格大小
    int tileSize = 64; // 方块大小
    int totalGridSize = gridSize * tileSize;
    
    // 计算在网格内的相对坐标
    int relX = worldX - (gridX * totalGridSize);
    int relY = worldY - (gridY * totalGridSize);
    
    // 计算方块的网格坐标
    int tileX = relX / tileSize;
    int tileY = relY / tileSize;
    
    // 创建坐标文本
    std::string worldCoordText = "世界坐标: (" + formatFloat(worldX) + ", " + formatFloat(worldY) + ")";
    std::string gridCoordText = "网格坐标: (" + std::to_string(gridX) + ", " + std::to_string(gridY) + ")";
    std::string tileCoordText = "方块坐标: (" + std::to_string(tileX) + ", " + std::to_string(tileY) + ")";
    
    // 设置文本颜色（浅绿色）
    SDL_Color coordColor = { 100, 255, 100, 255 };
    
    // 绘制背景
    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_FRect coordBg = { 
        static_cast<float>(windowWidth - 300), 
        static_cast<float>(windowHeight - 120), 
        290.0f, 
        80.0f 
    };
    SDL_RenderFillRect(renderer, &coordBg);
    
    // 渲染世界坐标文本
    SDL_Surface* worldSurface = TTF_RenderText_Solid(coordFont, worldCoordText.c_str(), 0, coordColor);
    if (worldSurface) {
        SDL_Texture* worldTexture = SDL_CreateTextureFromSurface(renderer, worldSurface);
        if (worldTexture) {
            SDL_FRect worldRect = { 
                static_cast<float>(windowWidth - 290), 
                static_cast<float>(windowHeight - 110), 
                static_cast<float>(worldSurface->w), 
                static_cast<float>(worldSurface->h) 
            };
            SDL_RenderTexture(renderer, worldTexture, nullptr, &worldRect);
            SDL_DestroyTexture(worldTexture);
        }
        SDL_DestroySurface(worldSurface);
    }
    
    // 渲染网格坐标文本
    SDL_Surface* gridSurface = TTF_RenderText_Solid(coordFont, gridCoordText.c_str(), 0, coordColor);
    if (gridSurface) {
        SDL_Texture* gridTexture = SDL_CreateTextureFromSurface(renderer, gridSurface);
        if (gridTexture) {
            SDL_FRect gridRect = { 
                static_cast<float>(windowWidth - 290), 
                static_cast<float>(windowHeight - 85), 
                static_cast<float>(gridSurface->w), 
                static_cast<float>(gridSurface->h) 
            };
            SDL_RenderTexture(renderer, gridTexture, nullptr, &gridRect);
            SDL_DestroyTexture(gridTexture);
        }
        SDL_DestroySurface(gridSurface);
    }
    
    // 渲染方块坐标文本
    SDL_Surface* tileSurface = TTF_RenderText_Solid(coordFont, tileCoordText.c_str(), 0, coordColor);
    if (tileSurface) {
        SDL_Texture* tileTexture = SDL_CreateTextureFromSurface(renderer, tileSurface);
        if (tileTexture) {
            SDL_FRect tileRect = { 
                static_cast<float>(windowWidth - 290), 
                static_cast<float>(windowHeight - 60), 
                static_cast<float>(tileSurface->w), 
                static_cast<float>(tileSurface->h) 
            };
            SDL_RenderTexture(renderer, tileTexture, nullptr, &tileRect);
            SDL_DestroyTexture(tileTexture);
        }
        SDL_DestroySurface(tileSurface);
    }
}

void HUD::renderActionProgress(SDL_Renderer* renderer, Action* currentAction, float duration, float elapsed) {
    if (!currentAction) return;
    
    // 获取窗口尺寸
    int windowWidth = Game::getInstance()->getWindowWidth();
    int windowHeight = Game::getInstance()->getWindowHeight();
    
    // 获取行动状态和名称
    EntityState actionState = currentAction->getActionState();
    std::string actionName;
    
    // 根据状态设置行动名称
    switch (actionState) {
        case EntityState::RELOADING:
            actionName = "换弹中";
            break;
        case EntityState::UNLOADING:
            actionName = "卸弹中";
            break;
        case EntityState::CHAMBERING:
            actionName = "上膛中";
            break;
        case EntityState::STUNNED:
            actionName = "眩晕中";
            break;
        case EntityState::HEALING:
            actionName = "治疗中";
            break;
        case EntityState::AIMING:
            actionName = "瞄准中";
            break;
        case EntityState::SPRINTING:
            actionName = "冲刺中";
            break;
        case EntityState::CROUCHING:
            actionName = "蹲伏中";
            break;
        case EntityState::PRONE:
            actionName = "卧倒中";
            break;
        default:
            actionName = "行动中";
            break;
    }
    
    // 计算进度比例（已完成的比例）
    float progress = elapsed / duration;
    
    // 如果Action已完成或进度超过100%，强制设置为100%
    if (currentAction->isActionCompleted() || progress >= 1.0f) {
        progress = 1.0f;
    }
    
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;
    
    // 转换为剩余比例（进度条从右向左减少）
    float remainingProgress = 1.0f - progress;
    
    // 设置进度条位置和大小
    float barWidth = 150.0f;
    float barHeight = 20.0f;
    float barX = windowWidth - barWidth - 20.0f; // 右侧边距20像素
    float barY = windowHeight / 2.0f - barHeight / 2.0f; // 垂直居中
    
    // 绘制进度条背景
    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_FRect barBg = { barX, barY, barWidth, barHeight };
    SDL_RenderFillRect(renderer, &barBg);
    
    // 绘制进度条（从右向左减少）
    SDL_SetRenderDrawColor(renderer, 0, 200, 255, 255); // 蓝色进度条
    SDL_FRect progressBar = { barX + 2, barY + 2, (barWidth - 4) * remainingProgress, barHeight - 4 };
    SDL_RenderFillRect(renderer, &progressBar);
    
    // 如果有字体，渲染行动名称
    if (ammoFont) {
        // 设置文本颜色（白色）
        SDL_Color textColor = { 255, 255, 255, 255 };
        
        // 创建文本表面
        SDL_Surface* textSurface = TTF_RenderText_Solid(ammoFont, actionName.c_str(), 0, textColor);
        if (textSurface) {
            // 创建纹理
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            if (textTexture) {
                // 设置渲染位置（进度条上方）
                SDL_FRect textRect = { 
                    barX, 
                    barY - 30.0f, 
                    static_cast<float>(textSurface->w), 
                    static_cast<float>(textSurface->h) 
                };
                
                // 渲染文本
                SDL_RenderTexture(renderer, textTexture, nullptr, &textRect);
                
                // 释放纹理
                SDL_DestroyTexture(textTexture);
            }
            
            // 释放表面
            SDL_DestroySurface(textSurface);
        }
    }
}

// 渲染碰撞系统调试信息
void HUD::renderCollisionDebugInfo(SDL_Renderer* renderer, Entity* entity) {
    if (!coordFont || !entity) return;
    
    // 获取窗口尺寸
    int windowWidth = Game::getInstance()->getWindowWidth();
    
    // 计算碰撞相关数据
    float pushPower = entity->calculatePushPower();
    float resistance = entity->calculatePushResistance();
    float weight = entity->getWeight();
    int strength = entity->getStrength();
    int dexterity = entity->getDexterity();
    
    // 创建调试文本
    std::string pushText = "推开能力: " + formatFloat(pushPower);
    std::string resistText = "抗推能力: " + formatFloat(resistance);
    std::string weightText = "重量: " + formatFloat(weight) + "kg";
    std::string strText = "力量: " + std::to_string(strength);
    std::string dexText = "敏捷: " + std::to_string(dexterity);
    
    // 设置文本颜色（青色）
    SDL_Color debugColor = { 0, 255, 255, 255 };
    
    // 绘制背景
    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_FRect debugBg = { 10.0f, 10.0f, 250.0f, 120.0f };
    SDL_RenderFillRect(renderer, &debugBg);
    
    // 渲染各行调试信息
    std::vector<std::string> debugLines = {pushText, resistText, weightText, strText, dexText};
    for (size_t i = 0; i < debugLines.size(); ++i) {
        SDL_Surface* debugSurface = TTF_RenderText_Solid(coordFont, debugLines[i].c_str(), 0, debugColor);
        if (debugSurface) {
            SDL_Texture* debugTexture = SDL_CreateTextureFromSurface(renderer, debugSurface);
            if (debugTexture) {
                SDL_FRect debugRect = { 
                    15.0f, 
                    15.0f + i * 20.0f, 
                    static_cast<float>(debugSurface->w), 
                    static_cast<float>(debugSurface->h) 
                };
                SDL_RenderTexture(renderer, debugTexture, nullptr, &debugRect);
                SDL_DestroyTexture(debugTexture);
            }
            SDL_DestroySurface(debugSurface);
        }
    }
}

bool HUD::isExitButtonClicked(int mouseX, int mouseY) {
    return (mouseX >= exitButton.x && mouseX <= exitButton.x + exitButton.w &&
            mouseY >= exitButton.y && mouseY <= exitButton.y + exitButton.h);
}

// 渲染连击信息
void HUD::renderComboInfo(SDL_Renderer* renderer, Player* player) {
    if (!coordFont || !player) return;
    
    // 检查玩家是否手持砍刀
    Item* heldItem = player->getHeldItem();
    if (!heldItem || !heldItem->hasFlag(ItemFlag::MELEE)) {
        return;
    }
    
    MeleeWeapon* meleeWeapon = dynamic_cast<MeleeWeapon*>(heldItem);
    if (!meleeWeapon) {
        return;
    }
    
    int comboCount = meleeWeapon->getComboCount();
    if (comboCount <= 0) {
        return; // 没有连击时不显示
    }
    
    // 获取窗口尺寸
    int windowWidth = Game::getInstance()->getWindowWidth();
    
    // 创建连击文本
    std::string comboText = std::to_string(comboCount) + " 连击!";
    
    // 根据连击数设置颜色
    SDL_Color comboColor;
    if (comboCount == 1) {
        comboColor = { 255, 255, 100, 255 }; // 浅黄色
    } else if (comboCount == 2) {
        comboColor = { 255, 150, 0, 255 };   // 橙色
    } else {
        comboColor = { 255, 50, 50, 255 };   // 红色
    }
    
    // 绘制连击背景
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_FRect comboBg = { 
        static_cast<float>(windowWidth / 2 - 80), 
        150.0f, 
        160.0f, 
        40.0f 
    };
    SDL_RenderFillRect(renderer, &comboBg);
    
    // 绘制边框
    SDL_SetRenderDrawColor(renderer, comboColor.r, comboColor.g, comboColor.b, 255);
    SDL_RenderRect(renderer, &comboBg);
    
    // 渲染连击文本
    SDL_Surface* comboSurface = TTF_RenderText_Solid(coordFont, comboText.c_str(), 0, comboColor);
    if (comboSurface) {
        SDL_Texture* comboTexture = SDL_CreateTextureFromSurface(renderer, comboSurface);
        if (comboTexture) {
            SDL_FRect comboRect = { 
                static_cast<float>(windowWidth / 2 - comboSurface->w / 2), 
                160.0f, 
                static_cast<float>(comboSurface->w), 
                static_cast<float>(comboSurface->h) 
            };
            SDL_RenderTexture(renderer, comboTexture, nullptr, &comboRect);
            SDL_DestroyTexture(comboTexture);
        }
        SDL_DestroySurface(comboSurface);
    }
}