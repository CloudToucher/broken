#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <vector>
#include <string>
#include <memory>
#include "UIWindow.h"

class Game;
class Player;
class Item;
class Storage;

// 使用UIWindow替代原来的ItemTooltip结构

// 存储空间坐标范围结构体
struct StorageCoordinates {
    float topLeftX;
    float topLeftY;
    float bottomRightX;
    float bottomRightY;
    Storage* storage;
};

class GameUI {
private:
    bool isUIVisible;                // 玩家UI是否可见
    float originalTimeScale;       // 原始游戏时间倍率
    
    // 字体
    TTF_Font* titleFont;           // 标题字体
    TTF_Font* subtitleFont;        // 副标题字体
    TTF_Font* itemFont;            // 物品字体
    TTF_Font* tooltipFont;         // 提示框字体
    
    // UI窗口
    std::unique_ptr<UIWindow> playerWindow;  // 玩家UI窗口
    
    // 鼠标悬停物品
    Item* hoveredItem;             // 当前鼠标悬停的物品
    int mouseX, mouseY;            // 鼠标位置
    
    // 物品提示框
    std::unique_ptr<UIWindow> itemTooltipWindow;  // 物品提示框窗口
    
    // 当前玩家引用
    Player* currentPlayer;         // 当前玩家对象的引用
    
    // 拖拽相关变量
    bool isDragging;               // 是否正在拖拽
    Item* draggedItem;             // 当前拖拽的物品
    Storage* sourceStorage;        // 拖拽物品的源存储空间
    int dragStartX, dragStartY;    // 拖拽开始位置
    
    // 存储空间坐标映射
    std::vector<StorageCoordinates> storageCoordinatesMap; // 存储空间坐标范围映射
    
    // 处理元素点击事件
    void onElementClick(const UIElement& element);
    
    // 更新物品提示框
    void updateItemTooltip();
    
    // 渲染物品提示框
    void renderItemTooltip(SDL_Renderer* renderer, float windowWidth, float windowHeight);
    
    // 获取物品详细信息文本
    std::vector<std::string> getItemDetails(Item* item) const;
    
    // 更新存储空间坐标映射
    void updateStorageCoordinatesMap();
    
public:
    GameUI();
    ~GameUI();
    
    // 初始化字体
    bool initFonts();
    
    // 打开/关闭/切换玩家UI
    void openPlayerUI(Game* game, Player* player = nullptr);
    void closePlayerUI(Game* game);
    void togglePlayerUI(Game* game, Player* player = nullptr);
    
    // 更新玩家UI内容
    void updatePlayerUI(Player* player);
    
    // 使用当前玩家更新UI
    void updatePlayerUI();
    
    // 渲染界面
    void render(SDL_Renderer* renderer, float windowWidth, float windowHeight);
    
    // 更新鼠标悬停物品
    void updateHoveredItem(int mouseX, int mouseY);
    
    // 处理点击事件
    bool handleClick(int mouseX, int mouseY, Player* player, float windowWidth, float windowHeight);
    
    // 处理存储点击事件
    bool handleStorageClick(int mouseX, int mouseY, Player* player, Storage* monsterBackpack, float windowWidth, float windowHeight);
    
    // 处理鼠标移动事件
    bool handleMouseMotion(int mouseX, int mouseY, float windowWidth, float windowHeight);
    
    // 处理鼠标释放事件
    bool handleMouseRelease(int mouseX, int mouseY, Player* player, float windowWidth, float windowHeight);
    
    // 检查UI是否可见
    bool isPlayerUIOpen() const { return isUIVisible; }
    bool isAnyUIOpen() const { return isUIVisible; }
    
    // 根据坐标查找对应元素所属的Storage
    Storage* findStorageByCoordinates(int x, int y);

};

// 辅助函数
std::string formatFloat(float value);
std::string getItemTextWithTags(Item* item);