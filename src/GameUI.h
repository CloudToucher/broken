#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include "UIWindow.h"

class Game;
class Player;
class Item;
class Storage;
class Magazine;

// 使用UIWindow替代原来的ItemTooltip结构

// 存储空间坐标范围结构体
struct StorageCoordinates {
    float topLeftX;
    float topLeftY;
    float bottomRightX;
    float bottomRightY;
    Storage* storage;
};

// 装备区域坐标范围结构体
struct EquipmentAreaCoordinates {
    float topLeftX;
    float topLeftY;
    float bottomRightX;
    float bottomRightY;
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
    
    // 标签页系统
    enum class TabType {
        EQUIPMENT = 0,  // 装备栏（当前已实现的）
        HEALTH = 1,     // 血量情况
        SKILLS = 2      // 技能等级列表
    };
    
    TabType currentTab;              // 当前选中的标签页
    static const int TAB_COUNT = 3;  // 标签页总数
    static const float TAB_HEIGHT;   // 标签页标题栏高度
    
    // UI窗口（每个标签页对应一个窗口）
    std::unique_ptr<UIWindow> equipmentWindow;   // 装备栏窗口
    std::unique_ptr<UIWindow> healthWindow;      // 血量情况窗口
    std::unique_ptr<UIWindow> skillsWindow;      // 技能等级窗口
    
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
    
    // 装备区域坐标映射
    EquipmentAreaCoordinates equipmentAreaCoordinates; // 装备区域坐标范围
    bool equipmentAreaValid; // 装备区域坐标是否有效
    
    // 手持位坐标（动态计算）
    ElementRenderRect handSlotRect; // 手持位元素的渲染区域
    bool handSlotRectValid; // 手持位坐标是否有效
    
    // 待处理的物品信息（用于存储选择确认框）
    Item* pendingHeldItemToReplace;                    // 待替换的手持物品
    Item* pendingNewItemToHold;                        // 待手持的新物品
    Storage* pendingNewItemSource;                     // 新物品的源存储空间
    
    // 确认对话框相关
    std::unique_ptr<UIWindow> confirmationWindow;      // 确认对话框窗口
    bool isConfirmationVisible;                        // 确认对话框是否可见
    std::function<void(bool)> confirmationCallback;    // 确认对话框回调函数
    float originalTimeScaleBeforeConfirmation;         // 确认对话框显示前的游戏倍率
    
    // 右键菜单相关
    std::unique_ptr<UIWindow> rightClickMenuWindow;    // 右键菜单窗口
    bool isRightClickMenuVisible;                      // 右键菜单是否可见
    Item* rightClickTargetItem;                        // 右键菜单目标物品
    Storage* rightClickTargetStorage;                  // 右键菜单目标物品所在的存储空间
    int rightClickMenuX, rightClickMenuY;             // 右键菜单显示位置
    
    // 标签页相关方法
    void initializeTabWindows();
    void switchToTab(TabType tab);
    void renderTabBar(SDL_Renderer* renderer, float windowWidth, float windowHeight);
    bool handleTabBarClick(int mouseX, int mouseY, float windowWidth, float windowHeight);
    UIWindow* getCurrentTabWindow();
    const char* getTabName(TabType tab) const;
    void updateHealthUI();
    void updateSkillsUI();
    
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
    
    // 更新装备区域坐标映射
    void updateEquipmentAreaCoordinatesMap();
    
    // 更新手持位坐标
    void updateHandSlotRect();
    
    // 确认对话框相关方法
    void showConfirmationDialog(const std::string& title, const std::string& message, 
                               const std::string& confirmText = "确认", const std::string& cancelText = "取消",
                               std::function<void(bool)> callback = nullptr);
    void hideConfirmationDialog();
    void updateConfirmationDialog(const std::string& title, const std::string& message, 
                                 const std::string& confirmText, const std::string& cancelText);
    void handleConfirmationClick(const UIElement& element);
    
    // 存储选择确认框方法
    void showStorageSelectionConfirmationDialog(Item* currentHeldItem, Item* newItem, Storage* newItemSource);
    void updateStorageSelectionConfirmationDialog();
    void handleStorageSelectionConfirmationClick(const UIElement& element);
    
    // 右键菜单相关方法
    void showRightClickMenu(int mouseX, int mouseY, Item* item, Storage* storage);
    void hideRightClickMenu();
    void updateRightClickMenu();
    void handleRightClickMenuClick(const UIElement& element);
    void performItemAction(const std::string& action, Item* item, Storage* storage);
    
    // 子弹装填/卸载相关方法
    void showAmmoSelectionDialog(Magazine* magazine, Storage* magazineStorage);
    void showStorageSelectionForUnloadAmmo(Magazine* magazine);
    void handleAmmoActionConfirmationClick(const std::string& actionData);
    
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
    
    // 处理右键点击事件
    bool handleRightClick(int mouseX, int mouseY, Player* player, float windowWidth, float windowHeight);
    
    // 处理存储点击事件
    bool handleStorageClick(int mouseX, int mouseY, Player* player, Storage* monsterBackpack, float windowWidth, float windowHeight);
    
    // 处理鼠标移动事件
    bool handleMouseMotion(int mouseX, int mouseY, float windowWidth, float windowHeight);
    
    // 处理鼠标释放事件
    bool handleMouseRelease(int mouseX, int mouseY, Player* player, float windowWidth, float windowHeight);
    
    // 处理滚轮事件
    bool handleScroll(int mouseX, int mouseY, float scrollDelta);
    
    // 检查UI是否可见
    bool isPlayerUIOpen() const { return isUIVisible; }
    bool isAnyUIOpen() const { return isUIVisible || isConfirmationVisible || isRightClickMenuVisible; }
    
    // 根据坐标查找对应元素所属的Storage
    Storage* findStorageByCoordinates(int x, int y);
    
    // 测试方法（可以通过按键触发）
    void testConfirmationDialog() {
        showConfirmationDialog(
            "确认框测试", 
            "这是测试消息，检查是否正确居中显示并设置为四分之一屏幕宽度。",
            "确定", 
            "取消",
            [](bool confirmed) {
                if (confirmed) {
                    std::cout << "用户点击了确定按钮" << std::endl;
                } else {
                    std::cout << "用户点击了取消按钮" << std::endl;
                }
            }
        );
    }

    // 测试存储选择确认框
    void testStorageSelectionDialog();

};

// 辅助函数
std::string formatFloat(float value);
std::string getItemTextWithTags(Item* item);