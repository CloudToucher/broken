#include "GameUI.h"
#include "Game.h"
#include "Player.h"
#include "Item.h"
#include "Gun.h"
#include "GunMod.h"
#include "MeleeWeapon.h"
#include "Ammo.h"
#include "Magazine.h"
#include "storage.h"
#include "AttackSystem.h"
#include "Damage.h"
#include <iostream>
#include <sstream>
#include <set>
#include <iomanip>
#include <map>
#include "EquipmentSystem.h"
#include "SkillSystem.h"

// 静态常量定义
const float GameUI::TAB_HEIGHT = 50.0f;

// 将浮点数格式化为保留两位小数的字符串
std::string formatFloat(float value) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << value;
    return ss.str();
}

// 生成带标签的物品文本
std::string getItemTextWithTags(Item* item) {
    if (!item) return "<空>";
    
    std::string text = item->getName();
    
    // 如果物品可堆叠且数量大于1，显示数量
    if (item->isStackable() && item->getStackSize() > 1) {
        text += " (x" + std::to_string(item->getStackSize()) + ")";
    }
    
    // 添加标签
    std::vector<std::string> tags;
    
    // 检查物品类别
    if (item->hasFlag(ItemFlag::WEAPON)) tags.push_back("武器");
    if (item->hasFlag(ItemFlag::ARMOR)) tags.push_back("护甲");
    if (item->hasFlag(ItemFlag::AMMO)) tags.push_back("弹药");
    if (item->hasFlag(ItemFlag::MAGAZINE)) {
        // 对于弹匣，显示当前弹药数量和最大容量
        Magazine* magazine = dynamic_cast<Magazine*>(item);
        if (magazine) {
            std::string magazineTag = "弹匣 " + std::to_string(magazine->getCurrentAmmoCount()) + "/" + std::to_string(magazine->getCapacity());
            tags.push_back(magazineTag);
        } else {
            tags.push_back("弹匣");
        }
    }
    if (item->hasFlag(ItemFlag::FOOD)) tags.push_back("食物");
    if (item->hasFlag(ItemFlag::MEDICAL)) tags.push_back("医疗");
    if (item->hasFlag(ItemFlag::TOOL)) tags.push_back("工具");
    if (item->hasFlag(ItemFlag::CONTAINER)) tags.push_back("容器");
    if (item->hasFlag(ItemFlag::GUNMOD)) tags.push_back("枪械配件");
    
    // 如果有标签，添加到文本中
    if (!tags.empty()) {
        text += " [";
        for (size_t i = 0; i < tags.size(); ++i) {
            if (i > 0) text += ", ";
            text += tags[i];
        }
        text += "]";
    }
    
    return text;
}

GameUI::GameUI()
    : isUIVisible(false), originalTimeScale(1.0f), 
      titleFont(nullptr), subtitleFont(nullptr), itemFont(nullptr), tooltipFont(nullptr),
      currentTab(TabType::EQUIPMENT), // 默认选中装备栏
      equipmentWindow(nullptr), healthWindow(nullptr), skillsWindow(nullptr),
      hoveredItem(nullptr), mouseX(0), mouseY(0),
      itemTooltipWindow(nullptr), currentPlayer(nullptr),
      isDragging(false), draggedItem(nullptr), sourceStorage(nullptr),
      dragStartX(0), dragStartY(0), equipmentAreaValid(false), handSlotRectValid(false),
      pendingHeldItemToReplace(nullptr), pendingNewItemToHold(nullptr), pendingNewItemSource(nullptr),
      confirmationWindow(nullptr), isConfirmationVisible(false), 
      confirmationCallback(nullptr), originalTimeScaleBeforeConfirmation(1.0f),
      rightClickMenuWindow(nullptr), isRightClickMenuVisible(false),
      rightClickTargetItem(nullptr), rightClickTargetStorage(nullptr),
      rightClickMenuX(0), rightClickMenuY(0) {
    // 构造函数中不再创建窗口，移到initFonts方法中
}

GameUI::~GameUI() {
    // 释放字体资源
    if (titleFont) {
        TTF_CloseFont(titleFont);
        titleFont = nullptr;
    }
    if (subtitleFont) {
        TTF_CloseFont(subtitleFont);
        subtitleFont = nullptr;
    }
    if (itemFont) {
        TTF_CloseFont(itemFont);
        itemFont = nullptr;
    }
    if (tooltipFont) {
        TTF_CloseFont(tooltipFont);
        tooltipFont = nullptr;
    }
}

bool GameUI::initFonts() {
    // 加载字体 - 使用Windows系统字体
    titleFont = TTF_OpenFont("C:\\Windows\\Fonts\\simhei.ttf", 48); // 一级标题使用较大字号
    subtitleFont = TTF_OpenFont("C:\\Windows\\Fonts\\simhei.ttf", 42); // 二级标题使用中等字号
    itemFont = TTF_OpenFont("C:\\Windows\\Fonts\\simhei.ttf", 36); // 物品名称使用较小字号
    tooltipFont = TTF_OpenFont("C:\\Windows\\Fonts\\simhei.ttf", 32); // 提示框字体
    
    // 检查字体是否加载成功
    if (!titleFont || !subtitleFont || !itemFont || !tooltipFont) {
        std::cerr << "Error loading fonts" << std::endl;
        return false;
    }
    
    // 初始化标签页窗口
    initializeTabWindows();
    
    // 创建物品提示框窗口 - 初始位置和尺寸为0，后续会根据内容动态调整
    itemTooltipWindow = std::make_unique<UIWindow>(0.0f, 0.0f, 0.0f, 0.0f, 
                                                  SDL_Color{200, 200, 200, 255}, // 浅灰色边框
                                                  230); // 半透明度
    // 设置窗口背景色为深灰色
    itemTooltipWindow->setBorderColor({200, 200, 200, 255});
    // 默认隐藏提示框
    itemTooltipWindow->setVisible(false);
    // 设置物品提示框窗口的字体
    itemTooltipWindow->setFonts(titleFont, subtitleFont, tooltipFont);
    itemTooltipWindow->setScrollEnabled(false); // 禁用滚动，使用自动调整大小
    

    
    // 创建确认对话框窗口 - 启用自动调整大小
    confirmationWindow = std::make_unique<UIWindow>(0.0f, 0.0f, 500.0f, 250.0f, 
                                                   SDL_Color{200, 200, 200, 255}, // 浅灰色边框
                                                   220); // 半透明度
    confirmationWindow->setVisible(false);
    confirmationWindow->setElementClickCallback([this](const UIElement& element) {
        this->handleConfirmationClick(element);
    });
    confirmationWindow->setFonts(titleFont, subtitleFont, itemFont);
    confirmationWindow->setScrollEnabled(true); // 启用滚动
    
    // 配置确认框的自动布局参数
    confirmationWindow->setAutoResize(true);
    confirmationWindow->setPadding(25.0f); // 内边距
    // maxContentWidth 将在显示时动态设置为屏幕宽度的1/4
    
    // 创建右键菜单窗口
    rightClickMenuWindow = std::make_unique<UIWindow>(0.0f, 0.0f, 200.0f, 300.0f,
                                                     SDL_Color{180, 180, 180, 255}, // 浅灰色边框
                                                     200); // 半透明度
    rightClickMenuWindow->setVisible(false);
    rightClickMenuWindow->setElementClickCallback([this](const UIElement& element) {
        this->handleRightClickMenuClick(element);
    });
    rightClickMenuWindow->setFonts(titleFont, subtitleFont, itemFont);
    rightClickMenuWindow->setScrollEnabled(false); // 禁用滚动
    rightClickMenuWindow->setAutoResize(true);
    rightClickMenuWindow->setPadding(10.0f); // 内边距
    
    return true;
}

void GameUI::onElementClick(const UIElement& element) {
    // 处理元素点击事件
    // 检查是否是折叠按钮（显示为+或-）
    if (element.getText() == "+" || element.getText() == "-") {
        // 这是折叠按钮，dataPtr指向Storage*
        Storage* storage = static_cast<Storage*>(element.getDataPtr());
        if (storage) {
            // 切换折叠状态
            storage->setIsCollapsed(!storage->getIsCollapsed());
            // 使用当前玩家引用更新UI
            updatePlayerUI();
        }
        return;
    }
    
    // 检查是否是刷新按钮（显示为⟲）
    if (element.getText() == "⟲") {
        // 这是刷新按钮，dataPtr指向Storage*
        Storage* storage = static_cast<Storage*>(element.getDataPtr());
        if (storage) {
            std::cout << "刷新整理存储空间: " << storage->getName() << std::endl;
            // 调用整理合并方法
            storage->consolidateItems();
            // 更新UI显示
            updatePlayerUI();
        }
        return;
    }
    
    // 获取关联的物品指针（如果有）
    void* dataPtr = element.getDataPtr();
    if (dataPtr) {
        Item* item = static_cast<Item*>(dataPtr);
        std::cout << "点击了物品: " << item->getName() << std::endl;
        // 这里可以添加物品点击的处理逻辑
    }
}

void GameUI::openPlayerUI(Game* game, Player* player) {
    if (!isUIVisible && game) {
        // 记录当前游戏倍率
        originalTimeScale = game->getTimeScale();
        // 设置游戏倍率为0.2
        game->setTimeScale(0.2f);
        
        // 更新当前玩家引用
        if (player) {
            currentPlayer = player;
        }
        
        // 显示界面
        isUIVisible = true;
        if (UIWindow* currentWindow = getCurrentTabWindow()) {
            currentWindow->setVisible(true);
        }
        
        // 如果有当前玩家引用，更新对应标签页的UI
        if (currentPlayer) {
            if (currentTab == TabType::EQUIPMENT) {
                updatePlayerUI(currentPlayer);
            } else if (currentTab == TabType::HEALTH) {
                updateHealthUI();
            } else if (currentTab == TabType::SKILLS) {
                updateSkillsUI();
            }
        }
    }
}

void GameUI::closePlayerUI(Game* game) {
    if (isUIVisible && game) {
        // 恢复游戏倍率
        game->setTimeScale(originalTimeScale);
        
        // 隐藏所有标签页窗口
        isUIVisible = false;
        equipmentWindow->setVisible(false);
        healthWindow->setVisible(false);
        skillsWindow->setVisible(false);
    }
}

void GameUI::togglePlayerUI(Game* game, Player* player) {
    if (isUIVisible) {
        closePlayerUI(game);
    } else {
        openPlayerUI(game, player);
    }
}

void GameUI::updatePlayerUI() {
    // 使用当前玩家引用更新UI
    if (currentPlayer) {
        updatePlayerUI(currentPlayer);
    } else {
        // 如果没有当前玩家引用，则无法更新UI
        std::cerr << "Error: Cannot update player UI, no current player reference." << std::endl;
    }
    
    // 清空存储空间坐标映射
    storageCoordinatesMap.clear();
    
    // 清空装备区域坐标映射有效性
    equipmentAreaValid = false;
}

void GameUI::updatePlayerUI(Player* player) {
    if (!player) return;
    
    // 只在装备栏标签页更新装备UI
    if (currentTab != TabType::EQUIPMENT) return;
    
    UIWindow* currentWindow = getCurrentTabWindow();
    if (!currentWindow) return;
    
    // 更新当前玩家引用
    currentPlayer = player;
    
    currentWindow->clearElements();
    
    // 添加标题
    UIElement title("玩家背包", 20.0f, 60.0f, {255, 255, 255, 255}, UIElementType::TITLE);
    currentWindow->addElement(title);
    
    // 添加手持位显示
    UIElement handSlotTitle("手持物品", 20.0f, 45.0f, {255, 215, 0, 255}, UIElementType::SUBTITLE);
    currentWindow->addElement(handSlotTitle);
    
    // 获取手持物品
    Item* heldItem = player->getHeldItem();
    if (heldItem) {
        // 添加手持物品
        SDL_Color itemColor = {255, 255, 255, 255}; // 默认白色
        
        // 根据物品稀有度设置不同的颜色
        switch (heldItem->getRarity()) {
            case ItemRarity::COMMON:    // 常规 - 白色
                itemColor = {255, 255, 255, 255};
                break;
            case ItemRarity::RARE:      // 稀有 - 蓝色
                itemColor = {100, 149, 237, 255};
                break;
            case ItemRarity::EPIC:      // 史诗 - 紫色
                itemColor = {148, 0, 211, 255};
                break;
            case ItemRarity::LEGENDARY: // 传说 - 金色
                itemColor = {255, 215, 0, 255};
                break;
            case ItemRarity::MYTHIC:    // 神话 - 红色
                itemColor = {255, 0, 0, 255};
                break;
        }
        
        UIElement heldItemElement(getItemTextWithTags(heldItem), 40.0f, 32.0f, itemColor, UIElementType::TEXT);
        heldItemElement.setDataPtr(heldItem);
        currentWindow->addElement(heldItemElement);
    } else {
        // 如果没有手持物品，显示空
        UIElement emptyHandElement("<空>", 40.0f, 32.0f, {150, 150, 150, 255}, UIElementType::TEXT);
        currentWindow->addElement(emptyHandElement);
    }
    
    // 添加额外间距
    UIElement spacer1("", 0.0f, 20.0f, {255, 255, 255, 255}, UIElementType::TEXT);
    currentWindow->addElement(spacer1);
    
    // 添加装备信息
    UIElement equipTitle("已装备物品:", 20.0f, 45.0f, {200, 200, 255, 255}, UIElementType::SUBTITLE);
    currentWindow->addElement(equipTitle);
    
    // 获取装备系统
    EquipmentSystem* equipSystem = player->getEquipmentSystem();
    if (equipSystem) {
        // 用于跟踪已显示的物品，避免重复显示同一个物品（当一个物品覆盖多个槽位时）
        std::set<Item*> displayedItems;
        
        // 定义所有需要显示的装备槽位
        std::vector<EquipSlot> allSlots = {
            EquipSlot::HEAD,
            EquipSlot::EYES,
            EquipSlot::CHEST,
            EquipSlot::ABDOMEN,
            EquipSlot::LEFT_LEG,
            EquipSlot::RIGHT_LEG,
            EquipSlot::LEFT_FOOT,
            EquipSlot::RIGHT_FOOT,
            EquipSlot::LEFT_ARM,
            EquipSlot::RIGHT_ARM,
            EquipSlot::LEFT_HAND,
            EquipSlot::RIGHT_HAND,
            EquipSlot::BACK
        };
        
        // 遍历所有装备槽位
        for (auto slot : allSlots) {
            // 获取该槽位的装备物品
            auto equippedItems = equipSystem->getEquippedItems(slot);
            
            // 将槽位枚举转换为可读字符串
            std::string slotName;
            switch (slot) {
                case EquipSlot::HEAD: slotName = "头部"; break;
                case EquipSlot::EYES: slotName = "眼部"; break;
                case EquipSlot::CHEST: slotName = "胸部"; break;
                case EquipSlot::ABDOMEN: slotName = "腹部"; break;
                case EquipSlot::LEFT_LEG: slotName = "左腿"; break;
                case EquipSlot::RIGHT_LEG: slotName = "右腿"; break;
                case EquipSlot::LEFT_FOOT: slotName = "左脚"; break;
                case EquipSlot::RIGHT_FOOT: slotName = "右脚"; break;
                case EquipSlot::LEFT_ARM: slotName = "左臂"; break;
                case EquipSlot::RIGHT_ARM: slotName = "右臂"; break;
                case EquipSlot::LEFT_HAND: slotName = "左手"; break;
                case EquipSlot::RIGHT_HAND: slotName = "右手"; break;
                case EquipSlot::BACK: slotName = "背部"; break;
                default: slotName = "未知"; break;
            }
            
            // 创建槽位标签元素
            UIElement slotElement(slotName + "：", 40.0f, 0.0f, {200, 200, 200, 255});
            currentWindow->addElement(slotElement);
            
            // 计算物品元素的X偏移量，基于槽位标签的长度
            float itemXOffset = 40.0f + slotName.length() * 14.0f; // 估算文本宽度
            
            // 如果该槽位有装备物品，显示它们
            if (!equippedItems.empty()) {
                // 添加该槽位的所有物品，在同一行显示
                for (size_t i = 0; i < equippedItems.size(); ++i) {
                    Item* equippedItem = equippedItems[i];
                    
                    // 检查该物品是否已经显示过（对于覆盖多个槽位的物品）
                    if (displayedItems.find(equippedItem) != displayedItems.end()) {
                        // 如果已显示过，只显示物品名称，不再添加到displayedItems
                        // 设置物品颜色
                        SDL_Color itemColor = {255, 255, 255, 255}; // 默认白色
                        
                        // 根据物品稀有度设置不同的颜色
                        switch (equippedItem->getRarity()) {
                            case ItemRarity::COMMON:    // 常规 - 白色
                                itemColor = {255, 255, 255, 255};
                                break;
                            case ItemRarity::RARE:      // 稀有 - 蓝色
                                itemColor = {100, 149, 237, 255};
                                break;
                            case ItemRarity::EPIC:      // 史诗 - 紫色
                                itemColor = {148, 0, 211, 255};
                                break;
                            case ItemRarity::LEGENDARY: // 传说 - 金色
                                itemColor = {255, 215, 0, 255};
                                break;
                            case ItemRarity::MYTHIC:    // 神话 - 红色
                                itemColor = {255, 0, 0, 255};
                                break;
                        }
                        
                        // 显示物品名称
                        UIElement itemElement(equippedItem->getName(), itemXOffset, 0.0f, itemColor);
                        itemElement.setDataPtr(equippedItem);
                        currentWindow->addElement(itemElement);
                        
                        // 更新下一个物品的X偏移量
                        itemXOffset += equippedItem->getName().length() * 14.0f + 20.0f; // 估算文本宽度并添加间距
                    } else {
                        // 如果是第一次显示，添加到displayedItems
                        displayedItems.insert(equippedItem);
                        
                        // 设置物品颜色
                        SDL_Color itemColor = {255, 255, 255, 255}; // 默认白色
                        
                        // 根据物品稀有度设置不同的颜色
                        switch (equippedItem->getRarity()) {
                            case ItemRarity::COMMON:    // 常规 - 白色
                                itemColor = {255, 255, 255, 255};
                                break;
                            case ItemRarity::RARE:      // 稀有 - 蓝色
                                itemColor = {100, 149, 237, 255};
                                break;
                            case ItemRarity::EPIC:      // 史诗 - 紫色
                                itemColor = {148, 0, 211, 255};
                                break;
                            case ItemRarity::LEGENDARY: // 传说 - 金色
                                itemColor = {255, 215, 0, 255};
                                break;
                            case ItemRarity::MYTHIC:    // 神话 - 红色
                                itemColor = {255, 0, 0, 255};
                                break;
                        }
                        
                        // 显示物品名称
                        UIElement itemElement(equippedItem->getName(), itemXOffset, 0.0f, itemColor);
                        itemElement.setDataPtr(equippedItem);
                        currentWindow->addElement(itemElement);
                        
                        // 更新下一个物品的X偏移量
                        itemXOffset += equippedItem->getName().length() * 14.0f + 20.0f; // 估算文本宽度并添加间距
                    }
                }
            } 
            else {
                // 如果该槽位没有装备物品，显示<空>
                UIElement emptyElement("<空>", itemXOffset, 0.0f, {150, 150, 150, 255});
                currentWindow->addElement(emptyElement);
            }
            
            // 在一行物品后添加Y轴偏移，为下一行做准备
            UIElement spacerElement("", 0.0f, 32.0f, {0, 0, 0, 0});
            currentWindow->addElement(spacerElement);
        }
    }
        
    
    
    // 添加额外间距
    UIElement spacer2("", 0.0f, 35.0f, {255, 255, 255, 255}, UIElementType::TEXT);
    currentWindow->addElement(spacer2);
    
    // 添加背包信息
    UIElement backpackTitle("背包物品:", 20.0f, 45.0f, {200, 255, 200, 255}, UIElementType::SUBTITLE);
    currentWindow->addElement(backpackTitle);
    
    // 获取所有存储空间
    auto storagePairs = player->getAllAvailableStorages();
    
    // 遍历所有存储空间
    for (auto& storagePair : storagePairs) {
        auto storage = storagePair.second;
        if (!storage) continue;
        
        // 添加存储空间信息
        std::string storageInfo = storage->getName() +
            " (" +
            std::to_string(storage->getItemCount()) + " 件物品" +
            (storage->getMaxItems() != -1 ? "/" + std::to_string(storage->getMaxItems()) : "") + ") - " +
            formatFloat(storage->getCurrentWeight()) + "/" +
            formatFloat(storage->getMaxWeight()) + " kg, " +
            formatFloat(storage->getCurrentVolume()) + "/" +
            formatFloat(storage->getMaxVolume()) + " 体积";
        
        // 添加折叠/展开按钮 - 放在存储空间信息前面，这样会显示在同一行
        // 考虑到UIElement的X坐标会被fontSizeRatio(1.3)缩放，需要预先除以缩放比例
        float fontSizeRatio = 1.3f; // TEXT类型的字体大小比例
        float targetButtonX = currentWindow->getWidth() - 40.0f; // 折叠按钮目标位置
        float buttonX = targetButtonX / fontSizeRatio; // 补偿缩放效果
        
        // 添加刷新按钮（在折叠按钮左边）
        float refreshButtonX = (targetButtonX - 40.0f) / fontSizeRatio; // 刷新按钮位置
        UIElement refreshButton("⟲", refreshButtonX, 0.0f, 
                               SDL_Color{150, 200, 255, 255}, // 淡蓝色
                               UIElementType::TEXT);
        refreshButton.setDataPtr(storage);
        currentWindow->addElement(refreshButton);
        
        //SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "折叠按钮位置: 窗体宽度=%.1f, 目标X=%.1f, 补偿后X=%.1f, 比例=%.1f", 
        //           currentWindow->getWidth(), targetButtonX, buttonX, fontSizeRatio);
        UIElement collapseButton(storage->getIsCollapsed() ? "+" : "-", buttonX, 0.0f, 
                                storage->getIsCollapsed() ? SDL_Color{255, 150, 150, 255} : SDL_Color{200, 200, 200, 255},
                                UIElementType::TEXT);
        collapseButton.setDataPtr(storage);
        currentWindow->addElement(collapseButton);
        
        UIElement storageInfoElement(storageInfo, 20.0f, 45.0f, {200, 200, 200, 255}, UIElementType::SUBTITLE);
        currentWindow->addElement(storageInfoElement);
        
        // 只有在非折叠状态下才显示物品
        if (!storage->getIsCollapsed()) {
            // 遍历存储空间中的物品
            for (size_t i = 0; i < storage->getItemCount(); ++i) {
                Item* item = storage->getItem(i);
                if (item) {
                    // 设置物品颜色
                    SDL_Color itemColor = {255, 255, 255, 255}; // 默认白色
                    
                    // 根据物品稀有度设置不同的颜色
                    switch (item->getRarity()) {
                        case ItemRarity::COMMON:    // 常规 - 白色
                            itemColor = {255, 255, 255, 255};
                            break;
                        case ItemRarity::RARE:      // 稀有 - 蓝色
                            itemColor = {100, 149, 237, 255};
                            break;
                        case ItemRarity::EPIC:      // 史诗 - 紫色
                            itemColor = {148, 0, 211, 255};
                            break;
                        case ItemRarity::LEGENDARY: // 传说 - 金色
                            itemColor = {255, 215, 0, 255};
                            break;
                        case ItemRarity::MYTHIC:    // 神话 - 红色
                            itemColor = {255, 0, 0, 255};
                            break;
                    }
                    
                    UIElement itemElement(getItemTextWithTags(item), 40.0f, 32.0f, itemColor, UIElementType::TEXT);
                    itemElement.setDataPtr(item);
                    currentWindow->addElement(itemElement);
                }
            }
        }
        
        // 在每个存储空间后添加一个空白的占位元素，增加存储空间之间的间距
        // 使用透明色（alpha=0）使其不可见，但仍占用空间
        UIElement storageSpacer("", 0.0f, 15.0f, {0, 0, 0, 0}, UIElementType::TEXT);
        currentWindow->addElement(storageSpacer);
    }

    // 启用块显示功能
    currentWindow->setBlocksEnabled(true);

}

void GameUI::render(SDL_Renderer* renderer, float windowWidth, float windowHeight) {
    if (!renderer) return;
    
    // 渲染标签页系统（如果可见）
    if (isUIVisible) {
        // 渲染标签栏
        renderTabBar(renderer, windowWidth, windowHeight);
        
        // 渲染当前标签页窗口
        UIWindow* currentWindow = getCurrentTabWindow();
        if (currentWindow) {
            // 设置窗口尺寸（占据左半屏幕，高度减去标签栏）
            currentWindow->setWidth(windowWidth / 2 - 45.0f);
            currentWindow->setHeight(windowHeight - 60.0f - TAB_HEIGHT);
            
            // 根据当前标签页进行实时更新
            if (currentTab == TabType::EQUIPMENT) {
                // 装备栏：更新存储空间相关功能
                updateStorageCoordinatesMap();
            } else if (currentTab == TabType::HEALTH) {
                // 血量界面：实时更新血量显示
                updateHealthUI();
            } else if (currentTab == TabType::SKILLS) {
                // 技能界面：实时更新技能显示
                updateSkillsUI();
            }
            
            currentWindow->render(renderer, windowWidth, windowHeight);
        
        // 调试模式：渲染元素边框
            // currentWindow->renderElementBorders(renderer);
        
            // 装备栏标签页的额外更新逻辑
            if (currentTab == TabType::EQUIPMENT) {
                
                // 更新装备区域坐标映射
                updateEquipmentAreaCoordinatesMap();
                
                // 更新手持位坐标
                updateHandSlotRect();
        
        // 如果正在拖拽物品，显示可以容纳该物品的存储空间的绿色边框
        if (isDragging && draggedItem) {            
            // 遍历所有存储空间
            int validStorageCount = 0;
            
            for (const auto& coords : storageCoordinatesMap) {
                // 检查存储空间是否能容纳被拖拽的物品
                bool canFit = coords.storage && coords.storage != sourceStorage && coords.storage->canFitItem(draggedItem);
                
                if (canFit) {
                    validStorageCount++;
                    
                    // 设置淡绿色边框，增加透明度使其更明显
                    SDL_SetRenderDrawColor(renderer, 100, 255, 100, 255); // 淡绿色，完全不透明
                    
                    // 绘制边框（左右两侧）
                    SDL_FRect leftBorder = {
                        coords.topLeftX,
                        coords.topLeftY,
                        3.0f, // 更细的边框宽度
                        coords.bottomRightY - coords.topLeftY
                    };
                    
                    SDL_FRect rightBorder = {
                        coords.bottomRightX - 3.0f, // 调整位置以适应新宽度
                        coords.topLeftY,
                        3.0f, // 更细的边框宽度
                        coords.bottomRightY - coords.topLeftY
                    };
                    
                    // 绘制边框
                    SDL_RenderFillRect(renderer, &leftBorder);
                    SDL_RenderFillRect(renderer, &rightBorder);
                    
                    // 绘制顶部和底部边框
                    SDL_FRect topBorder = {
                        coords.topLeftX,
                        coords.topLeftY,
                        coords.bottomRightX - coords.topLeftX,
                        3.0f // 更细的边框高度
                    };
                    
                    SDL_FRect bottomBorder = {
                        coords.topLeftX,
                        coords.bottomRightY - 3.0f,
                        coords.bottomRightX - coords.topLeftX,
                        3.0f // 更细的边框高度
                    };
                    
                    // 绘制顶部和底部边框
                    SDL_RenderFillRect(renderer, &topBorder);
                    SDL_RenderFillRect(renderer, &bottomBorder);
                }
            }
                    
                    // 绘制手持位边框（只有在没有手持物品且正在拖拽来自存储空间的物品时才显示）
                    // 使用当前渲染的player而不是currentPlayer，确保状态一致性
                    Player* renderPlayer = Game::getInstance()->getPlayer();
                    if (sourceStorage && renderPlayer && !renderPlayer->getHeldItem() && handSlotRectValid) {
                        // 使用动态计算的手持位区域
                        // 设置橙色边框表示手持位
                        SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255); // 橙色
                        
                        // 绘制手持位边框（铺满整个窗口宽度）
                        float windowX = currentWindow->getX();
                        float windowWidth = currentWindow->getWidth();
                        
                        SDL_FRect leftBorder = {
                            windowX + 10.0f, // 从窗口左边距开始
                            handSlotRect.y,
                            3.0f,
                            handSlotRect.height
                        };
                        
                        SDL_FRect rightBorder = {
                            windowX + windowWidth - 13.0f, // 到窗口右边距结束
                            handSlotRect.y,
                            3.0f,
                            handSlotRect.height
                        };
                        
                        SDL_FRect topBorder = {
                            windowX + 10.0f, // 从窗口左边距开始
                            handSlotRect.y,
                            windowWidth - 20.0f, // 铺满整个窗口宽度（减去边距）
                            3.0f
                        };
                        
                        SDL_FRect bottomBorder = {
                            windowX + 10.0f, // 从窗口左边距开始
                            handSlotRect.y + handSlotRect.height - 3.0f,
                            windowWidth - 20.0f, // 铺满整个窗口宽度（减去边距）
                            3.0f
                        };
                        
                        // 绘制边框
                        SDL_RenderFillRect(renderer, &leftBorder);
                        SDL_RenderFillRect(renderer, &rightBorder);
                        SDL_RenderFillRect(renderer, &topBorder);
                        SDL_RenderFillRect(renderer, &bottomBorder);
                    }
                    
                    // 绘制装备区域边框（只有在拖拽可穿戴物品时才显示）
                    if (sourceStorage && draggedItem->isWearable() && equipmentAreaValid) {
                        // 设置紫色边框表示装备区域
                        SDL_SetRenderDrawColor(renderer, 147, 112, 219, 255); // 紫色
                        
                        SDL_FRect leftBorder = {
                            equipmentAreaCoordinates.topLeftX + 10.0f,
                            equipmentAreaCoordinates.topLeftY,
                            3.0f,
                            equipmentAreaCoordinates.bottomRightY - equipmentAreaCoordinates.topLeftY
                        };
                        
                        SDL_FRect rightBorder = {
                            equipmentAreaCoordinates.bottomRightX - 13.0f,
                            equipmentAreaCoordinates.topLeftY,
                            3.0f,
                            equipmentAreaCoordinates.bottomRightY - equipmentAreaCoordinates.topLeftY
                        };
                        
                        SDL_FRect topBorder = {
                            equipmentAreaCoordinates.topLeftX + 10.0f,
                            equipmentAreaCoordinates.topLeftY,
                            equipmentAreaCoordinates.bottomRightX - equipmentAreaCoordinates.topLeftX - 20.0f,
                            3.0f
                        };
                        
                        SDL_FRect bottomBorder = {
                            equipmentAreaCoordinates.topLeftX + 10.0f,
                            equipmentAreaCoordinates.bottomRightY - 3.0f,
                            equipmentAreaCoordinates.bottomRightX - equipmentAreaCoordinates.topLeftX - 20.0f,
                            3.0f
                        };
                        
                        // 绘制边框
                        SDL_RenderFillRect(renderer, &leftBorder);
                        SDL_RenderFillRect(renderer, &rightBorder);
                        SDL_RenderFillRect(renderer, &topBorder);
                        SDL_RenderFillRect(renderer, &bottomBorder);
                    }
            
            // 显示有效存储空间数量
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_FRect debugRect2 = {10, 35, 300, 20};
            SDL_RenderFillRect(renderer, &debugRect2);
            
            std::string validText = "可容纳的存储空间数量: " + std::to_string(validStorageCount);
            if (itemFont) {
                SDL_Surface* textSurface = TTF_RenderText_Solid(itemFont, validText.c_str(), 0, {0, 0, 0, 255});
                if (textSurface) {
                    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                    if (textTexture) {
                        SDL_FRect textRect = {10, 35, static_cast<float>(textSurface->w), static_cast<float>(textSurface->h)};
                        SDL_RenderTexture(renderer, textTexture, NULL, &textRect);
                        SDL_DestroyTexture(textTexture);
                    }
                    SDL_DestroySurface(textSurface);
                        }
                    }
                }
            }
        }
    }
    
    // 渲染物品提示框（无论玩家UI是否可见，物品提示框都可以显示）
    renderItemTooltip(renderer, windowWidth, windowHeight);
    

    
    // 渲染确认对话框（如果可见，优先级最高）
    if (isConfirmationVisible && confirmationWindow) {
        confirmationWindow->renderWithWrapping(renderer, windowWidth, windowHeight);
    }
    
    // 渲染右键菜单（在最顶层）
    if (isRightClickMenuVisible && rightClickMenuWindow) {
        rightClickMenuWindow->renderWithWrapping(renderer, windowWidth, windowHeight);
    }
    
    // 如果正在拖拽物品，在鼠标位置绘制物品名称
    if (isDragging && draggedItem) {
        // 创建一个临时的文本表面
        SDL_Color textColor = {255, 255, 255, 200};
        SDL_Surface* textSurface = TTF_RenderText_Blended(itemFont, draggedItem->getName().c_str(), 0, textColor);
        if (textSurface) {
            // 创建纹理
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            if (textTexture) {
                // 设置目标矩形
                SDL_FRect destRect = {
                    static_cast<float>(mouseX), 
                    static_cast<float>(mouseY), 
                    static_cast<float>(textSurface->w), 
                    static_cast<float>(textSurface->h)
                };
                
                // 渲染文本
                SDL_RenderTexture(renderer, textTexture, nullptr, &destRect);
                
                // 释放纹理
                SDL_DestroyTexture(textTexture);
            }
            
            // 释放表面
            SDL_DestroySurface(textSurface);
        }
    }
}

void GameUI::updateHoveredItem(int mouseX, int mouseY) {
    // 保存鼠标位置
    this->mouseX = mouseX;
    this->mouseY = mouseY;

    // 如果确认对话框或右键菜单可见，不更新悬停物品
    if (isConfirmationVisible || isRightClickMenuVisible) {
        hoveredItem = nullptr;
        if (itemTooltipWindow) {
            itemTooltipWindow->setVisible(false);
        }
        return;
    }

    // 重置悬停物品指针
    hoveredItem = nullptr;

    // 检查玩家背包UI（只在装备栏标签页检查悬停物品）
    if (isUIVisible && currentTab == TabType::EQUIPMENT) {
        UIWindow* currentWindow = getCurrentTabWindow();
        if (currentWindow) {
            int elementIndex = currentWindow->getElementAtPosition(mouseX, mouseY);
        if (elementIndex >= 0) {
                const auto& elements = currentWindow->getElements();
            if (elementIndex < static_cast<int>(elements.size())) {
                // 检查元素文本，如果是折叠/展开按钮（+或-）或刷新按钮（⟲），则不将其视为物品
                const std::string& text = elements[elementIndex].getText();
                if (text != "+" && text != "-" && text != "⟲") {
                    // 只有非功能按钮的元素才可能是物品
                    hoveredItem = static_cast<Item*>(elements[elementIndex].getDataPtr());
                    }
                }
            }
        }
    }

    // 更新物品提示框
    updateItemTooltip();
}

void GameUI::updateItemTooltip() {
    // 如果没有悬停物品，隐藏提示框
    if (!hoveredItem || !itemTooltipWindow) {
        itemTooltipWindow->setVisible(false);
        return;
    }
    
    // 获取物品详细信息
    std::vector<std::string> itemDetails = getItemDetails(hoveredItem);
    
    // 如果没有详细信息，隐藏提示框
    if (itemDetails.empty()) {
        itemTooltipWindow->setVisible(false);
        return;
    }
    
    // 清除旧元素
    itemTooltipWindow->clearElements();
    
    // 添加物品详细信息作为UI元素
    float yOffset = 10.0f; // 初始Y轴偏移
    float maxWidth = 0.0f;
    
    // 第一行作为标题（物品名称）
    if (!itemDetails.empty()) {
        SDL_Color titleColor = {255, 255, 255, 255}; // 白色
        
        // 根据物品稀有度设置不同的颜色
        switch (hoveredItem->getRarity()) {
            case ItemRarity::COMMON:    // 常规 - 白色
                titleColor = {255, 255, 255, 255};
                break;
            case ItemRarity::RARE:      // 稀有 - 蓝色
                titleColor = {100, 149, 237, 255};
                break;
            case ItemRarity::EPIC:      // 史诗 - 紫色
                titleColor = {186, 85, 211, 255};
                break;
            case ItemRarity::LEGENDARY: // 传说 - 橙色
                titleColor = {255, 165, 0, 255};
                break;
            case ItemRarity::MYTHIC:    // 神话 - 红色
                titleColor = {255, 69, 0, 255};
                break;
            default:
                titleColor = {255, 255, 255, 255};
                break;
        }
        
        // 添加标题
        UIElement titleElement(itemDetails[0], 10.0f, 30.0f, titleColor, UIElementType::SUBTITLE);
        itemTooltipWindow->addElement(titleElement);
        yOffset += 30.0f;
        
        // 在物品名称下方添加空白区域
        UIElement spacerElement("", 10.0f, 15.0f, {255, 255, 255, 255}, UIElementType::TEXT);
        itemTooltipWindow->addElement(spacerElement);
        yOffset += 15.0f;
        
        // 计算最大宽度
        float charWidth = 20.0f; // 与后面计算保持一致
        maxWidth = std::max(maxWidth, static_cast<float>(itemDetails[0].length() * charWidth));
        
        // 添加其余详细信息
        for (size_t i = 1; i < itemDetails.size(); ++i) {
            UIElement detailElement(itemDetails[i], 10.0f, 25.0f, {255, 255, 255, 255}, UIElementType::TEXT);
            itemTooltipWindow->addElement(detailElement);
            
            // 计算最大宽度
            float charWidth = 20.0f; // 与后面计算保持一致
            maxWidth = std::max(maxWidth, static_cast<float>(itemDetails[i].length() * charWidth));
        }
    }
    
    // 计算所有元素的Y轴高度总和和最大宽度
    float totalYOffset = 0.0f;
    float calculatedMaxWidth = 0.0f;
    
    for (const auto& element : itemTooltipWindow->getElements()) {
        // 根据元素类型获取字体大小比例
        float yOffsetRatio = 1.0f;
        float fontSizeMultiplier = 1.0f;
        
        switch (element.getType()) {
            case UIElementType::TITLE:
                yOffsetRatio = 1.8f;
                fontSizeMultiplier = 2.0f; // 增大标题字体大小乘数
                break;
            case UIElementType::SUBTITLE:
                yOffsetRatio = 1.5f;
                fontSizeMultiplier = 1.7f; // 增大副标题字体大小乘数
                break;
            case UIElementType::TEXT:
            default:
                yOffsetRatio = 1.3f;
                fontSizeMultiplier = 1.3f; // 增大普通文本字体大小乘数
                break;
        }
        
        // 累加Y轴偏移量
        totalYOffset += element.getYOffset() * yOffsetRatio;
        
        // 计算元素宽度（根据字体大小和文本长度）
        // 增大每个字符的宽度估计值，中文字符通常更宽
        float charWidth = 20.0f; // 增大字符宽度估计值
        float elementWidth = element.getText().length() * charWidth * fontSizeMultiplier;
        calculatedMaxWidth = std::max(calculatedMaxWidth, elementWidth);
    }
    
    // 使用计算出的最大宽度和原始估计的最大宽度中的较大值
    float tooltipWidth = std::max(maxWidth, calculatedMaxWidth) + 60.0f; // 增大边距
    
    // 设置窗口高度为所有元素的Y轴高度总和加上固定值（顶部和底部边距）
    float tooltipHeight = totalYOffset + 60.0f; // 增大顶部和底部边距
    
    itemTooltipWindow->setX(static_cast<float>(mouseX) + 20.0f); // 鼠标右侧
    itemTooltipWindow->setY(static_cast<float>(mouseY) - 10.0f); // 鼠标上方
    itemTooltipWindow->setWidth(tooltipWidth);
    itemTooltipWindow->setHeight(tooltipHeight);
    
    // 显示提示框
    itemTooltipWindow->setVisible(true);
}

void GameUI::renderItemTooltip(SDL_Renderer* renderer, float windowWidth, float windowHeight) {
    if (!itemTooltipWindow || !renderer) return;
    
    // 检查提示框是否可见
    if (!itemTooltipWindow->getVisible()) return;
    
    // 调整提示框位置，确保不超出窗口边界
    float tooltipX = itemTooltipWindow->getX();
    float tooltipY = itemTooltipWindow->getY();
    float tooltipWidth = itemTooltipWindow->getWidth();
    float tooltipHeight = itemTooltipWindow->getHeight();
    
    if (tooltipX + tooltipWidth > windowWidth) {
        itemTooltipWindow->setX(windowWidth - tooltipWidth - 10.0f);
    }
    
    if (tooltipY + tooltipHeight > windowHeight) {
        itemTooltipWindow->setY(windowHeight - tooltipHeight - 10.0f);
    }
    
    // 使用UIWindow的render方法渲染提示框
    itemTooltipWindow->render(renderer, windowWidth, windowHeight);
}

// 获取物品详细信息文本
std::vector<std::string> GameUI::getItemDetails(Item* item) const {
    std::vector<std::string> details;
    
    if (!item) return details;
    
    // 添加物品名称（根据稀有度可能会有不同的前缀或标记）
    std::string namePrefix;
    switch (item->getRarity()) {
        case ItemRarity::COMMON:    namePrefix = ""; break;
        case ItemRarity::RARE:      namePrefix = "[稀有] "; break;
        case ItemRarity::EPIC:      namePrefix = "[史诗] "; break;
        case ItemRarity::LEGENDARY: namePrefix = "[传说] "; break;
        case ItemRarity::MYTHIC:    namePrefix = "[神话] "; break;
        default: namePrefix = ""; break;
    }
    details.push_back(namePrefix + item->getName());
    
    // 添加物品类别信息（只显示重要的类别标签）
    std::vector<std::string> importantCategories;
    if (item->hasFlag(ItemFlag::WEAPON)) importantCategories.push_back("武器");
    if (item->hasFlag(ItemFlag::ARMOR)) importantCategories.push_back("护甲");
    if (item->hasFlag(ItemFlag::AMMO)) importantCategories.push_back("弹药");
    if (item->hasFlag(ItemFlag::CONTAINER)) importantCategories.push_back("容器");
    if (item->hasFlag(ItemFlag::MEDICAL)) importantCategories.push_back("医疗");
    if (item->hasFlag(ItemFlag::FOOD)) importantCategories.push_back("食物");
    if (item->hasFlag(ItemFlag::TOOL)) importantCategories.push_back("工具");
    if (item->hasFlag(ItemFlag::MISC)) importantCategories.push_back("杂项");
    
    if (!importantCategories.empty()) {
    std::string categoryStr = "类别: ";
        for (size_t i = 0; i < importantCategories.size(); ++i) {
        if (i > 0) categoryStr += ", ";
            categoryStr += importantCategories[i];
    }
    details.push_back(categoryStr);
    }
    
    // 添加物品基本属性
    details.push_back("重量: " + formatFloat(item->getWeight()) + " kg");
    details.push_back("体积: " + formatFloat(item->getVolume()) + " L");
    details.push_back("价值: " + std::to_string(item->getValue()) + " $");
    
    // 如果是可穿戴物品，显示覆盖部位和覆盖率
    if (item->hasFlag(ItemFlag::WEARABLE)) {
        details.push_back(""); // 空行
        details.push_back("可穿戴属性:");
        
        // 显示覆盖率信息
        auto coverageSlots = item->getCoverageSlots();
        if (!coverageSlots.empty()) {
            // 将所有覆盖部位信息合并到一行或几行
            std::string coverageLine = "覆盖部位: ";
            bool first = true;
            
            for (const auto& coverage : coverageSlots) {
                std::string slotName;
                switch (coverage.slot) {
                    case EquipSlot::HEAD: slotName = "头部"; break;
                    case EquipSlot::EYES: slotName = "眼部"; break;
                    case EquipSlot::CHEST: slotName = "胸部"; break;
                    case EquipSlot::ABDOMEN: slotName = "腹部"; break;
                    case EquipSlot::LEFT_LEG: slotName = "左腿"; break;
                    case EquipSlot::RIGHT_LEG: slotName = "右腿"; break;
                    case EquipSlot::LEFT_FOOT: slotName = "左脚"; break;
                    case EquipSlot::RIGHT_FOOT: slotName = "右脚"; break;
                    case EquipSlot::LEFT_ARM: slotName = "左臂"; break;
                    case EquipSlot::RIGHT_ARM: slotName = "右臂"; break;
                    case EquipSlot::LEFT_HAND: slotName = "左手"; break;
                    case EquipSlot::RIGHT_HAND: slotName = "右手"; break;
                    case EquipSlot::BACK: slotName = "背部"; break;
                    default: slotName = "未知"; break;
                }
                
                if (!first) {
                    coverageLine += ", ";
                }
                
                coverageLine += slotName + ":" + std::to_string(coverage.coverage) + "%";
                if (coverage.burden > 0) {
                    coverageLine += "(累赘:" + std::to_string(coverage.burden) + ")";
                }
                first = false;
            }
            
            details.push_back(coverageLine);
        } else {
            // 如果没有覆盖率信息，显示传统的装备槽位
            const auto& equipSlots = item->getEquipSlots();
            if (!equipSlots.empty()) {
                std::string slotsLine = "装备槽位: ";
                bool first = true;
                
                for (const auto& slot : equipSlots) {
                    std::string slotName;
                    switch (slot) {
                        case EquipSlot::HEAD: slotName = "头部"; break;
                        case EquipSlot::EYES: slotName = "眼部"; break;
                        case EquipSlot::CHEST: slotName = "胸部"; break;
                        case EquipSlot::ABDOMEN: slotName = "腹部"; break;
                        case EquipSlot::LEFT_LEG: slotName = "左腿"; break;
                        case EquipSlot::RIGHT_LEG: slotName = "右腿"; break;
                        case EquipSlot::LEFT_FOOT: slotName = "左脚"; break;
                        case EquipSlot::RIGHT_FOOT: slotName = "右脚"; break;
                        case EquipSlot::LEFT_ARM: slotName = "左臂"; break;
                        case EquipSlot::RIGHT_ARM: slotName = "右臂"; break;
                        case EquipSlot::LEFT_HAND: slotName = "左手"; break;
                        case EquipSlot::RIGHT_HAND: slotName = "右手"; break;
                        case EquipSlot::BACK: slotName = "背部"; break;
                        default: slotName = "未知"; break;
                    }
                    
                    if (!first) {
                        slotsLine += ", ";
                    }
                    slotsLine += slotName;
                    first = false;
                }
                
                details.push_back(slotsLine);
            }
        }
        
        // 显示防护信息
        const auto& protectionData = item->getProtectionData();
        if (!protectionData.empty()) {
            details.push_back(""); // 空行
            details.push_back("防护等级:");
            
            for (const auto& protection : protectionData) {
                std::string partName;
                switch (protection.bodyPart) {
                    case EquipSlot::HEAD: partName = "头部"; break;
                    case EquipSlot::EYES: partName = "眼部"; break;
                    case EquipSlot::CHEST: partName = "胸部"; break;
                    case EquipSlot::ABDOMEN: partName = "腹部"; break;
                    case EquipSlot::LEFT_LEG: partName = "左腿"; break;
                    case EquipSlot::RIGHT_LEG: partName = "右腿"; break;
                    case EquipSlot::LEFT_FOOT: partName = "左脚"; break;
                    case EquipSlot::RIGHT_FOOT: partName = "右脚"; break;
                    case EquipSlot::LEFT_ARM: partName = "左臂"; break;
                    case EquipSlot::RIGHT_ARM: partName = "右臂"; break;
                    case EquipSlot::LEFT_HAND: partName = "左手"; break;
                    case EquipSlot::RIGHT_HAND: partName = "右手"; break;
                    case EquipSlot::BACK: partName = "背部"; break;
                    default: partName = "未知"; break;
                }
                
                // 收集该部位的所有防护值并在同一行显示
                std::vector<DamageType> mainProtectionTypes = {
                    DamageType::BLUNT, DamageType::SLASH, DamageType::PIERCE,
                    DamageType::ELECTRIC, DamageType::BURN, DamageType::HEAT,
                    DamageType::COLD, DamageType::EXPLOSION, DamageType::SHOOTING
                };
                
                std::string protectionLine = "- " + partName + ": ";
                bool hasProtection = false;
                
                for (DamageType damageType : mainProtectionTypes) {
                    int protectionValue = protection.getProtection(damageType);
                    
                    if (protectionValue > 0) {
                        if (hasProtection) {
                            protectionLine += ", ";
                        }
                        std::string damageTypeName = damageTypeToString(damageType);
                        protectionLine += damageTypeName + ":" + std::to_string(protectionValue);
                        hasProtection = true;
                    }
                }
                
                if (hasProtection) {
                    details.push_back(protectionLine);
                }
            }
        }
    }
    
    // 如果有存储空间，显示存储信息
    if (item->hasFlag(ItemFlag::CONTAINER) && item->getStorageCount() > 0) {
        details.push_back(""); // 空行
        details.push_back("存储空间:");
        
        for (size_t i = 0; i < item->getStorageCount(); ++i) {
            Storage* storage = item->getStorage(i);
            if (storage) {
                std::string storageInfo = "- " + storage->getName() + ":";
                details.push_back(storageInfo);
                
                // 重量信息
                details.push_back("  重量: " + formatFloat(storage->getCurrentWeight()) + "/" + 
                                formatFloat(storage->getMaxWeight()) + " kg");
                
                // 体积信息
                details.push_back("  体积: " + formatFloat(storage->getCurrentVolume()) + "/" + 
                                formatFloat(storage->getMaxVolume()) + " L");
                
                // 长度信息
                details.push_back("  最大长度: " + formatFloat(storage->getMaxLength()) + " cm");
                
                // 物品数量信息
                if (storage->getMaxItems() != -1) {
                    details.push_back("  物品数量: " + std::to_string(storage->getItemCount()) + "/" + 
                                    std::to_string(storage->getMaxItems()) + " 件");
                } else {
                    details.push_back("  物品数量: " + std::to_string(storage->getItemCount()) + " 件");
                }
                
                // 存取时间信息
                details.push_back("  存取时间: " + formatFloat(storage->getAccessTime()) + " 秒");
            }
        }
    }
    
    // 如果是弹药，显示弹药属性
    if (item->hasFlag(ItemFlag::AMMO)) {
        Ammo* ammo = dynamic_cast<Ammo*>(item);
        details.push_back(""); // 空行
        details.push_back("弹药属性:");
        if (ammo) {
            details.push_back("口径: " + ammo->getAmmoType());
            details.push_back("伤害: " + std::to_string(ammo->getBaseDamage()));
            details.push_back("穿透力: " + formatFloat(ammo->getBasePenetration()));
            details.push_back("射程: " + formatFloat(ammo->getBaseRange()) + " m");
            details.push_back("速度: " + formatFloat(ammo->getBaseSpeed()) + " m/s");
            
            // 显示特殊影响属性
            if (ammo->getModRecoil() != 0.0f) {
                details.push_back("后坐力修正: " + formatFloat(ammo->getModRecoil() * 100) + "%");
            }
            if (ammo->getModAccuracyMOA() != 0.0f) {
                details.push_back("精度修正: " + formatFloat(ammo->getModAccuracyMOA()) + " MOA");
            }
            if (ammo->getModErgonomics() != 0.0f) {
                details.push_back("人体工程学修正: " + formatFloat(ammo->getModErgonomics()));
            }
        } else {
            details.push_back("警告: 物品标记为弹药但无法转换为Ammo类型");
        }
    }
    
    // 如果是武器，显示武器属性
    if (item->hasFlag(ItemFlag::WEAPON)) {
        details.push_back(""); // 空行
        details.push_back("武器属性:");
        
        // 如果是枪械，添加枪械特定属性
        if (item->hasFlag(ItemFlag::GUN)) {
            Gun* gun = dynamic_cast<Gun*>(item);
            if (gun) {
                details.push_back("伤害加成: " + std::to_string(gun->getDamageBonus()));
                details.push_back("射程加成: " + std::to_string(gun->getRangeBonus()) + " cm");
                details.push_back("精度: " + formatFloat(gun->getAccuracyMOA()) + " MOA");
                details.push_back("射速: " + std::to_string(gun->getFireRate()) + " RPM");
                details.push_back("穿透加成: " + formatFloat(gun->getPenetrationBonus()));
                
                // 添加弹匣信息
                Magazine* mag = gun->getCurrentMagazine();
                if (mag) {
                    details.push_back("已装弹匣: " + mag->getName() + " (" + 
                                    std::to_string(mag->getCurrentAmmoCount()) + "/" + 
                                    std::to_string(mag->getCapacity()) + ")");
                } else {
                    details.push_back("已装弹匣: 无");
                }
                
                // 添加配件信息
                bool hasAttachments = false;
                details.push_back("已安装配件:");
                
                // 遍历所有可能的配件槽位
                std::vector<std::string> allSlotTypes = {
                    "STOCK", "BARREL", "UNDER_BARREL", "GRIP", "OPTIC", 
                    "SIDE_MOUNT", "MUZZLE", "MAGAZINE_WELL", "RAIL", "SPECIAL"
                };
                
                for (const std::string& slotType : allSlotTypes) {
                    auto slotAttachments = gun->getAllAttachments(slotType);
                    
                    if (!slotAttachments.empty()) {
                        hasAttachments = true;
                        for (auto mod : slotAttachments) {
                            details.push_back("- " + mod->getName());
                        }
                    }
                }
                
                // 如果没有配件，显示"无"
                if (!hasAttachments) {
                    details.push_back("- 无");
                }
            } else {
                details.push_back("警告: 物品标记为枪械但无法转换为Gun类型");
            }
        }
        // 如果是近战武器，添加近战武器特定属性
        else if (item->hasFlag(ItemFlag::MELEE)) {
            MeleeWeapon* melee = dynamic_cast<MeleeWeapon*>(item);
            if (melee) {
                // 获取主攻击和副攻击的参数
                AttackParams primaryAttack = melee->getAttackParams(WeaponAttackType::PRIMARY);
                AttackParams secondaryAttack = melee->getAttackParams(WeaponAttackType::SECONDARY);
                
                details.push_back("基础伤害: " + std::to_string(primaryAttack.baseDamage));
                details.push_back("攻击范围: " + formatFloat(primaryAttack.range) + " 像素");
                details.push_back("攻击速度: " + formatFloat(primaryAttack.speed) + " 次/秒");
                details.push_back("暴击率: " + formatFloat(primaryAttack.criticalChance * 100) + "%");
                details.push_back("暴击倍数: " + formatFloat(primaryAttack.criticalMultiplier) + "x");
                details.push_back("护甲穿透: " + std::to_string(primaryAttack.armorPenetration));
                
                // 显示攻击方式
                details.push_back("攻击方式:");
                
                // 主攻击方式
                std::string primaryDesc = "- 主攻击: ";
                if (primaryAttack.shape == AttackShape::SECTOR) {
                    primaryDesc += "扇形攻击";
                } else if (primaryAttack.shape == AttackShape::RECTANGLE) {
                    primaryDesc += "直线攻击";
                } else if (primaryAttack.shape == AttackShape::CIRCLE) {
                    primaryDesc += "圆形攻击";
                }
                primaryDesc += " (" + primaryAttack.damageType + "伤害)";
                details.push_back(primaryDesc);
                
                // 副攻击方式（如果有）
                if (secondaryAttack.baseDamage > 0) {
                    std::string secondaryDesc = "- 副攻击: ";
                    if (secondaryAttack.shape == AttackShape::SECTOR) {
                        secondaryDesc += "扇形攻击";
                    } else if (secondaryAttack.shape == AttackShape::RECTANGLE) {
                        secondaryDesc += "直线攻击";
                    } else if (secondaryAttack.shape == AttackShape::CIRCLE) {
                        secondaryDesc += "圆形攻击";
                    }
                    secondaryDesc += " (" + secondaryAttack.damageType + "伤害)";
                    if (secondaryAttack.baseDamage != primaryAttack.baseDamage) {
                        secondaryDesc += " [" + std::to_string(secondaryAttack.baseDamage) + "伤害]";
                    }
                    details.push_back(secondaryDesc);
                }
                
                // 显示特殊效果
                bool hasEffects = false;
                details.push_back("特殊效果:");
                
                if (primaryAttack.canBleed && primaryAttack.bleedChance > 0) {
                    details.push_back("- 流血概率: " + formatFloat(primaryAttack.bleedChance * 100) + "%");
                    hasEffects = true;
                }
                if (primaryAttack.canStun && primaryAttack.stunChance > 0) {
                    details.push_back("- 眩晕概率: " + formatFloat(primaryAttack.stunChance * 100) + "%");
                    hasEffects = true;
                }
                if (primaryAttack.canPoison && primaryAttack.poisonChance > 0) {
                    details.push_back("- 中毒概率: " + formatFloat(primaryAttack.poisonChance * 100) + "%");
                    hasEffects = true;
                }
                if (primaryAttack.canKnockback && primaryAttack.knockbackChance > 0) {
                    details.push_back("- 击退概率: " + formatFloat(primaryAttack.knockbackChance * 100) + "%");
                    hasEffects = true;
                }
                
                if (!hasEffects) {
                    details.push_back("- 无");
                }
                
                // 显示当前连击层数
                if (melee->getComboCount() > 0) {
                    details.push_back("当前连击: " + std::to_string(melee->getComboCount()) + " 层");
                }
        } else {
                details.push_back("警告: 物品标记为近战武器但无法转换为MeleeWeapon类型");
        }
    }
    }
    
    // 如果是弹匣，显示弹匣属性
    if (item->hasFlag(ItemFlag::MAGAZINE)) {
        Magazine* magazine = dynamic_cast<Magazine*>(item);
        details.push_back(""); // 空行
        details.push_back("弹匣属性:");
        if (magazine) {
            details.push_back("容量: " + std::to_string(magazine->getCurrentAmmoCount()) + "/" + 
                            std::to_string(magazine->getCapacity()) + " 发");
            
            // 显示兼容口径
            const auto& compatibleTypes = magazine->getCompatibleAmmoTypes();
            if (!compatibleTypes.empty()) {
                details.push_back("兼容口径:");
                for (const auto& type : compatibleTypes) {
                    details.push_back("- " + type);
                }
            }
            
            details.push_back("装填时间: " + formatFloat(magazine->getReloadTime()) + " 秒");
            details.push_back("卸载时间: " + formatFloat(magazine->getUnloadTime()) + " 秒");
        } else {
            details.push_back("警告: 物品标记为弹匣但无法转换为Magazine类型");
        }
    }
    
    // 如果是枪械配件，显示配件属性
    if (item->hasFlag(ItemFlag::GUNMOD)) {
        GunMod* gunMod = dynamic_cast<GunMod*>(item);
        details.push_back(""); // 空行
        details.push_back("配件属性:");
        if (gunMod) {
            // 显示各种修正值（只显示非零的）
            if (gunMod->getModDamageBonus() != 0) {
                details.push_back("伤害修正: " + std::to_string(gunMod->getModDamageBonus()));
            }
            if (gunMod->getModRangeBonus() != 0) {
                details.push_back("射程修正: " + std::to_string(gunMod->getModRangeBonus()) + " cm");
            }
            if (gunMod->getModAccuracyMOA() != 0) {
                details.push_back("精度修正: " + formatFloat(gunMod->getModAccuracyMOA()) + " MOA");
            }
            if (gunMod->getModFireRate() != 0) {
                details.push_back("射速修正: " + formatFloat(gunMod->getModFireRate()) + " RPM");
            }
            if (gunMod->getModRecoil() != 0) {
                details.push_back("后坐力修正: " + formatFloat(gunMod->getModRecoil()));
            }
            if (gunMod->getModErgonomics() != 0) {
                details.push_back("人体工程学修正: " + formatFloat(gunMod->getModErgonomics()));
            }
            if (gunMod->getModBreathStability() != 0) {
                details.push_back("呼吸稳定性修正: " + formatFloat(gunMod->getModBreathStability()));
            }
            if (gunMod->getModBulletSpeedBonus() != 0) {
                details.push_back("子弹速度修正: " + formatFloat(gunMod->getModBulletSpeedBonus()) + " m/s");
            }
            if (gunMod->getModPenetrationBonus() != 0) {
                details.push_back("穿透力修正: " + formatFloat(gunMod->getModPenetrationBonus()));
            }
            if (gunMod->getModSoundLevel() != 0) {
                details.push_back("声音级别修正: " + formatFloat(gunMod->getModSoundLevel()) + " dB");
            }
        } else {
            details.push_back("警告: 物品标记为枪械配件但无法转换为GunMod类型");
        }
    }
    
    return details;
}

bool GameUI::handleClick(int mouseX, int mouseY, Player* player, float windowWidth, float windowHeight) {
    // 更新当前玩家引用
    if (player) {
        currentPlayer = player;
    }
    
    // 处理右键菜单点击（如果可见）
    if (isRightClickMenuVisible && rightClickMenuWindow) {
        // 检查点击是否在右键菜单内
        bool clickInsideMenu = (mouseX >= rightClickMenuWindow->getX() && 
                               mouseX <= rightClickMenuWindow->getX() + rightClickMenuWindow->getWidth() &&
                               mouseY >= rightClickMenuWindow->getY() && 
                               mouseY <= rightClickMenuWindow->getY() + rightClickMenuWindow->getHeight());
        
        if (clickInsideMenu) {
            // 获取被点击的元素
            int elementIndex = rightClickMenuWindow->getElementAtPosition(mouseX, mouseY);
            if (elementIndex >= 0) {
                const auto& elements = rightClickMenuWindow->getElements();
                if (elementIndex < static_cast<int>(elements.size())) {
                    // 处理右键菜单项点击
                    handleRightClickMenuClick(elements[elementIndex]);
                    return true;
                }
            }
        }
        // 点击在菜单外部，隐藏菜单
        hideRightClickMenu();
    }
    
    // 最优先检查确认对话框（模态对话框）
    if (isConfirmationVisible && confirmationWindow) {
        if (confirmationWindow->handleClick(mouseX, mouseY, windowWidth, windowHeight)) {
            return true; // 点击被确认对话框处理了
        }
        // 如果确认对话框可见但点击不在其范围内，阻止其他UI交互
        return true;
    }
    
    // 检查标签栏点击
    if (handleTabBarClick(mouseX, mouseY, windowWidth, windowHeight)) {
        return true; // 点击被标签栏处理了
    }
    
    // 检查玩家背包UI（只在装备栏标签页处理物品点击）
    if (isUIVisible && currentTab == TabType::EQUIPMENT) {
        UIWindow* currentWindow = getCurrentTabWindow();
        if (currentWindow) {
        // 检查是否点击了物品元素
            int elementIndex = currentWindow->getElementAtPosition(mouseX, mouseY);
        if (elementIndex >= 0) {
                const auto& elements = currentWindow->getElements();
            if (elementIndex < static_cast<int>(elements.size())) {
                // 检查元素文本，如果是折叠/展开按钮（+或-）或刷新按钮（⟲），则不将其视为物品
                const std::string& text = elements[elementIndex].getText();
                if (text != "+" && text != "-" && text != "⟲") {
                    // 只有非功能按钮的元素才可能是物品
                    Item* clickedItem = static_cast<Item*>(elements[elementIndex].getDataPtr());
                    if (clickedItem) {
                        // 检查是否点击的是已装备的物品
                        bool isEquippedItem = false;
                        EquipmentSystem* equipSystem = player->getEquipmentSystem();
                        EquipSlot clickedItemSlot = EquipSlot::NONE;
                        
                        if (equipSystem) {
                            // 获取已装备的槽位
                            auto equippedSlots = equipSystem->getEquippedSlots();
                            
                            // 遍历已装备的槽位，检查点击的物品是否是已装备的物品
                            for (auto slot : equippedSlots) {
                                auto equippedItems = equipSystem->getEquippedItems(slot);
                                for (auto* equippedItem : equippedItems) {
                                    if (equippedItem == clickedItem) {
                                        isEquippedItem = true;
                                        clickedItemSlot = slot;
                                        break;
                                    }
                                }
                                if (isEquippedItem) break; // 如果找到了，就不再继续查找
                            }
                        }
                        
                        // 删除已装备物品左键点击就卸下的逻辑，改为只支持拖拽操作
                        // 开始拖拽
                        isDragging = true;
                        draggedItem = clickedItem;
                        dragStartX = mouseX;
                        dragStartY = mouseY;
                        
                        // 查找源存储空间
                        sourceStorage = findStorageByCoordinates(mouseX, mouseY);
                        
                        // 更新存储空间坐标映射
                        updateStorageCoordinatesMap();
                    }
                }
            }
        }
        
            return currentWindow->handleClick(mouseX, mouseY, windowWidth, windowHeight);
        }
    }
    
    return false;
}

bool GameUI::handleStorageClick(int mouseX, int mouseY, Player* player, Storage* monsterBackpack, float windowWidth, float windowHeight) {
    // 更新当前玩家引用
    if (player) {
        currentPlayer = player;
    }
    
    // 检查玩家背包UI中的Storage折叠/展开按钮（只在装备栏标签页）
    if (isUIVisible && currentTab == TabType::EQUIPMENT) {
        UIWindow* currentWindow = getCurrentTabWindow();
        if (currentWindow) {
            int elementIndex = currentWindow->getElementAtPosition(mouseX, mouseY);
        if (elementIndex >= 0) {
                const auto& elements = currentWindow->getElements();
            if (elementIndex < static_cast<int>(elements.size())) {
                // 检查是否点击了Storage元素
                void* dataPtr = elements[elementIndex].getDataPtr();
                if (dataPtr) {
                    // 尝试将数据指针转换为Storage
                    Storage* storage = static_cast<Storage*>(dataPtr);
                    
                    // 检查是否为折叠/展开按钮（通过文本判断）
                    if (elements[elementIndex].getText() == "+" || elements[elementIndex].getText() == "-") {
                        // 切换折叠状态
                        storage->setIsCollapsed(!storage->getIsCollapsed());
                        
                        // 更新UI
                        if (player) {
                            updatePlayerUI(player);
                        }
                        
                        return true; // 点击已处理
                        }
                    }
                }
            }
        }
    }
    
    return false; // 点击未处理
}

Storage* GameUI::findStorageByCoordinates(int x, int y) {
    // 检查玩家UI是否可见且在装备栏标签页
    if (!isUIVisible || currentTab != TabType::EQUIPMENT || !currentPlayer) {
        return nullptr;
    }
    
    UIWindow* currentWindow = getCurrentTabWindow();
    if (!currentWindow) {
        return nullptr;
    }
    
    // 首先检查点击是否在窗口区域内
    if (x < currentWindow->getX() || x > currentWindow->getX() + currentWindow->getWidth() ||
        y < currentWindow->getY() || y > currentWindow->getY() + currentWindow->getHeight()) {
        return nullptr;
    }
    
    // 直接根据坐标范围查找存储空间
    for (const auto& coords : storageCoordinatesMap) {
        if (x >= coords.topLeftX && x <= coords.bottomRightX &&
            y >= coords.topLeftY && y <= coords.bottomRightY) {
            return coords.storage;
        }
    }
    
    // 如果坐标映射为空或未找到匹配的存储空间，回退到旧方法
    if (storageCoordinatesMap.empty()) {
        // 获取指定坐标的UI元素索引
        int elementIndex = currentWindow->getElementAtPosition(x, y);
        if (elementIndex < 0) {
            return nullptr; // 没有找到元素
        }
        
        // 获取元素列表
        const auto& elements = currentWindow->getElements();
        if (elementIndex >= static_cast<int>(elements.size())) {
            return nullptr; // 索引超出范围
        }
        
        // 获取元素的数据指针
        void* dataPtr = elements[elementIndex].getDataPtr();
        if (!dataPtr) {
            return nullptr; // 元素没有关联数据
        }
        
        // 首先检查数据指针是否直接指向Storage
        // 这种情况通常出现在折叠/展开按钮和刷新按钮上
        if (elements[elementIndex].getText() == "+" || elements[elementIndex].getText() == "-" || elements[elementIndex].getText() == "⟲") {
            return static_cast<Storage*>(dataPtr);
        }
        
        // 否则，假设数据指针指向Item
        Item* item = static_cast<Item*>(dataPtr);
        
        // 获取所有可用的存储空间
        auto storagePairs = currentPlayer->getAllAvailableStorages();
        
        // 遍历所有存储空间，查找包含该物品的存储空间
        for (auto& storagePair : storagePairs) {
            Storage* storage = storagePair.second;
            if (!storage) continue;
            
            // 遍历存储空间中的所有物品
            for (size_t i = 0; i < storage->getItemCount(); ++i) {
                if (storage->getItem(i) == item) {
                    return storage; // 找到了包含该物品的存储空间
                }
            }
        }
    }
    
    return nullptr; // 没有找到匹配的存储空间
}

bool GameUI::handleMouseMotion(int mouseX, int mouseY, float windowWidth, float windowHeight) {
    // 如果任何模态对话框可见，阻止鼠标移动事件处理
    if (isConfirmationVisible) {
        return true; // 事件被处理，阻止进一步传播
    }
    
    // 更新鼠标位置
    this->mouseX = mouseX;
    this->mouseY = mouseY;
    
    // 如果正在拖拽，可以在这里添加拖拽效果
    // 例如，可以在鼠标位置绘制被拖拽物品的图标
    
    // 更新悬停物品
    updateHoveredItem(mouseX, mouseY);
    
    // 更新UIWindow中的悬停元素
    UIWindow* currentWindow = getCurrentTabWindow();
    if (currentWindow && currentWindow->getVisible()) {
        int elementIndex = currentWindow->getElementAtPosition(mouseX, mouseY);
        currentWindow->setHoveredElement(elementIndex);
    }
    
    return isDragging; // 如果正在拖拽，返回true
}

void GameUI::updateStorageCoordinatesMap() {
    // 清空存储空间坐标映射
    storageCoordinatesMap.clear();
    
    // 只在装备栏标签页进行坐标映射
    if (currentTab != TabType::EQUIPMENT) return;
    
    UIWindow* currentWindow = getCurrentTabWindow();
    if (!currentWindow) return;
    
    const auto& elements = currentWindow->getElements();
    
    Storage* currentStorage = nullptr;
    float storageStartY = 0.0f;
    
    for (size_t i = 0; i < elements.size(); ++i) {
        const auto& element = elements[i];
        
        // 获取元素的渲染区域
        ElementRenderRect rect;
        if (!currentWindow->getElementRect(i, rect)) {
            continue;
        }
        
        // 检查元素是否为存储空间信息（通过检查文本内容包含"件物品"来判断）
        if (element.getText().find("件物品") != std::string::npos) {
            // 如果之前有处理中的存储空间，完成其坐标范围计算
            if (currentStorage) {
                StorageCoordinates coords;
                coords.topLeftX = currentWindow->getX();
                coords.topLeftY = storageStartY;
                coords.bottomRightX = currentWindow->getX() + currentWindow->getWidth();
                coords.bottomRightY = rect.y; // 当前存储空间信息元素的顶部作为上一个存储空间的底部
                coords.storage = currentStorage;
                
                storageCoordinatesMap.push_back(coords);
            }
            
            // 获取当前元素关联的存储空间
            void* dataPtr = elements[i > 0 ? i - 1 : i].getDataPtr(); // 折叠按钮通常在存储空间信息前面
            if (dataPtr && (elements[i > 0 ? i - 1 : i].getText() == "+" || elements[i > 0 ? i - 1 : i].getText() == "-")) {
                currentStorage = static_cast<Storage*>(dataPtr);
                storageStartY = rect.y;
            } else {
                currentStorage = nullptr;
            }
        }
    }
    
    // 处理最后一个存储空间
    if (currentStorage) {
        StorageCoordinates coords;
        coords.topLeftX = currentWindow->getX();
        coords.topLeftY = storageStartY;
        coords.bottomRightX = currentWindow->getX() + currentWindow->getWidth();
        coords.bottomRightY = currentWindow->getY() + currentWindow->getHeight(); // 窗口底部作为最后一个存储空间的底部
        coords.storage = currentStorage;
        
        storageCoordinatesMap.push_back(coords);
    }
}

void GameUI::updateEquipmentAreaCoordinatesMap() {
    // 重置装备区域有效性
    equipmentAreaValid = false;
    
    // 只在装备栏标签页进行坐标映射
    if (currentTab != TabType::EQUIPMENT) return;
    
    UIWindow* currentWindow = getCurrentTabWindow();
    if (!currentWindow) return;
    
    const auto& elements = currentWindow->getElements();
    
    float equipmentAreaStartY = 0.0f;
    float equipmentAreaEndY = 0.0f;
    bool foundEquipmentStart = false;
    bool foundEquipmentEnd = false;
    
    for (size_t i = 0; i < elements.size(); ++i) {
        const auto& element = elements[i];
        
        // 获取元素的渲染区域
        ElementRenderRect rect;
        if (!currentWindow->getElementRect(i, rect)) {
            continue;
        }
        
        // 查找"已装备物品:"标题
        if (!foundEquipmentStart && element.getText() == "已装备物品:") {
            equipmentAreaStartY = rect.y;
            foundEquipmentStart = true;
            continue;
        }
        
        // 查找"背包物品:"标题作为装备区域的结束
        if (foundEquipmentStart && !foundEquipmentEnd && element.getText() == "背包物品:") {
            equipmentAreaEndY = rect.y;
            foundEquipmentEnd = true;
            break;
        }
    }
    
    // 如果找到了装备区域的开始和结束，设置坐标
    if (foundEquipmentStart) {
        equipmentAreaCoordinates.topLeftX = currentWindow->getX();
        equipmentAreaCoordinates.topLeftY = equipmentAreaStartY;
        equipmentAreaCoordinates.bottomRightX = currentWindow->getX() + currentWindow->getWidth();
        
        // 如果找到了结束位置，使用它；否则使用窗口底部
        if (foundEquipmentEnd) {
            equipmentAreaCoordinates.bottomRightY = equipmentAreaEndY;
        } else {
            equipmentAreaCoordinates.bottomRightY = currentWindow->getY() + currentWindow->getHeight();
        }
        
        equipmentAreaValid = true;
    }
}

bool GameUI::handleMouseRelease(int mouseX, int mouseY, Player* player, float windowWidth, float windowHeight) {
    // 如果任何模态对话框可见，阻止拖拽操作
    if (isConfirmationVisible) {
        // 如果有拖拽状态，取消拖拽
        isDragging = false;
        draggedItem = nullptr;
        sourceStorage = nullptr;
        return true; // 事件被处理
    }
    
    // 如果没有正在拖拽，直接返回
    if (!isDragging || !draggedItem || !player) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "拖拽状态无效: isDragging=%d, draggedItem=%p, player=%p",
            isDragging ? 1 : 0, draggedItem, player);
        isDragging = false;
        draggedItem = nullptr;
        sourceStorage = nullptr;
        return false;
    }

    // 更新当前玩家引用
    if (player) {
        currentPlayer = player;
    }

    // 查找鼠标释放位置对应的存储空间
    Storage* targetStorage = findStorageByCoordinates(mouseX, mouseY);

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "鼠标释放位置: (%d, %d), 目标存储空间=%s",
        mouseX, mouseY, targetStorage ? targetStorage->getName().c_str() : "未知");

    // 检查是否是已装备的物品
    bool isEquippedItem = false;
    EquipSlot equippedSlot = EquipSlot::NONE;
    EquipmentSystem* equipSystem = player->getEquipmentSystem();

    if (equipSystem) {
        // 获取已装备的槽位
        auto equippedSlots = equipSystem->getEquippedSlots();

        // 遍历已装备的槽位，检查拖拽的物品是否是已装备的物品
        for (auto slot : equippedSlots) {
            auto equippedItems = equipSystem->getEquippedItems(slot);
            for (auto* equippedItem : equippedItems) {
                if (equippedItem == draggedItem) {
                    isEquippedItem = true;
                    equippedSlot = slot;
                    break;
                }
            }
            if (isEquippedItem) break; // 如果找到了，就不再继续查找
        }
    }

    // 检查是否是手持物品
    bool isHeldItem = (player->getHeldItem() == draggedItem);
    
    // 检查是否拖拽到装备位置
    bool droppedOnEquipmentSlot = false;
    EquipSlot targetEquipSlot = EquipSlot::NONE;
    
    // 检查是否拖拽到手持位置
    bool droppedOnHeldItemSlot = false;
    
    // 更新手持位坐标
    updateHandSlotRect();
    
    // 检查是否拖拽到手持位置（使用铺满窗口宽度的范围，与橙色框一致）
    if (handSlotRectValid && currentTab == TabType::EQUIPMENT) {
        UIWindow* currentWindow = getCurrentTabWindow();
        if (currentWindow) {
            float windowX = currentWindow->getX();
            float windowWidth = currentWindow->getWidth();
        
            // 使用与橙色框相同的检测范围：铺满整个窗口宽度
            float detectStartX = windowX + 10.0f;
            float detectEndX = windowX + windowWidth - 10.0f;
            
            //SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "手持位检测: 鼠标(%d,%d), 手持区域(%.1f-%.1f,%.1f,%.1f)", 
            //           mouseX, mouseY, 
            //           detectStartX, detectEndX, handSlotRect.y, handSlotRect.height);
            
            if (mouseX >= detectStartX && mouseX <= detectEndX &&
                mouseY >= handSlotRect.y && mouseY <= handSlotRect.y + handSlotRect.height) {
            droppedOnHeldItemSlot = true;
                //SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "检测到拖拽到手持位置（铺满窗口宽度）");
        }
        }
    }
        
        // TODO: 装备槽位检测需要实现动态计算，暂时注释掉硬编码的检测逻辑
        // 检查是否拖拽到装备位置，并确定具体的装备槽位
        // （装备槽位的动态计算比手持位更复杂，将在后续版本实现）
    
    // 检查是否拖拽到装备区域
    bool droppedOnEquipmentArea = false;
    
    // 更新装备区域坐标
    updateEquipmentAreaCoordinatesMap();
    
    // 检查是否拖拽到装备区域（只有在装备栏标签页才进行检测）
    if (equipmentAreaValid && currentTab == TabType::EQUIPMENT && !isEquippedItem && !isHeldItem && sourceStorage) {
        // 检查鼠标位置是否在装备区域内
        if (mouseX >= equipmentAreaCoordinates.topLeftX + 10.0f && mouseX <= equipmentAreaCoordinates.bottomRightX - 10.0f &&
            mouseY >= equipmentAreaCoordinates.topLeftY && mouseY <= equipmentAreaCoordinates.bottomRightY) {
            droppedOnEquipmentArea = true;
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "检测到拖拽到装备区域");
        }
    }
    
    // 如果拖拽到装备区域
    if (droppedOnEquipmentArea && draggedItem->isWearable()) {
        // 先从源存储空间取出物品，然后装备
        player->takeItemWithAction(draggedItem, sourceStorage, [player](std::unique_ptr<Item> takenItem) {
            if (takenItem) {
                // 取出成功，尝试装备
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "成功取出物品: %s，开始装备", takenItem->getName().c_str());
                player->equipItemWithAction(std::move(takenItem));
            } else {
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "取出物品失败");
            }
        });
        
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "尝试将可穿戴物品装备: %s", draggedItem->getName().c_str());
    }
    // 如果拖拽到装备位置
    else if (droppedOnEquipmentSlot && !isEquippedItem && !isHeldItem && sourceStorage) {
        // 先检查物品是否可以装备到目标槽位
        bool canEquip = false;
        if (draggedItem->isWearable()) {
            // 检查物品是否可以装备到目标槽位
            canEquip = draggedItem->canEquipToSlot(targetEquipSlot);
        }
        
        if (canEquip) {
            // 物品可以装备到目标槽位，使用Entity的equipItemWithAction方法尝试装备物品
            player->takeItemWithAction(draggedItem, sourceStorage, [player](std::unique_ptr<Item> takenItem) {
                if (takenItem) {
                    // 取出成功，尝试装备
                    player->equipItemWithAction(std::move(takenItem));
                }
            });
            
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "尝试将物品装备到槽位: %d", static_cast<int>(targetEquipSlot));
        } else {
            // 物品不能装备到目标槽位，显示提示信息
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "物品不能装备到槽位: %d", static_cast<int>(targetEquipSlot));
        }
    }
    // 如果拖拽到手持位置
    else if (droppedOnHeldItemSlot && !isEquippedItem && !isHeldItem && sourceStorage) {
        // 检查玩家是否已经手持物品
        Item* currentHeldItem = player->getHeldItem();
        if (currentHeldItem) {
            // 如果已有手持物品，显示存储选择确认框
            showStorageSelectionConfirmationDialog(currentHeldItem, draggedItem, sourceStorage);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "已有手持物品，显示存储选择确认框");
        } else {
            // 如果没有手持物品，直接手持新物品
        player->holdItemFromStorage(draggedItem, sourceStorage);
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "尝试将物品设置为手持物品");
        }
    }
    // 如果找到了目标存储空间
    else if (targetStorage) {
        // 检查目标存储空间是否可以容纳该物品
        bool canFit = targetStorage->canFitItem(draggedItem);

        // 检查目标存储空间是否属于玩家
        bool isPlayerStorage = false;
        auto playerStorages = player->getAllAvailableStorages();
        for (const auto& [slot, storage] : playerStorages) {
            if (storage == targetStorage) {
                isPlayerStorage = true;
                break;
            }
        }

        // 如果是已装备或手持物品，并且目标存储空间可以容纳该物品
        if ((isEquippedItem || isHeldItem) && canFit) {
            // 如果是已装备物品
            if (isEquippedItem) {
                // 使用按物品卸下的方法，将物品作为一个整体卸下（支持一个物品覆盖多个槽位的情况）
                player->unequipItemWithAction(draggedItem, [this, player, targetStorage](std::unique_ptr<Item> unequippedItem) {
                    if (unequippedItem) {
                        // 卸下成功，将物品放入目标存储空间
                        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "成功卸下装备: %s，正在放入目标存储空间", unequippedItem->getName().c_str());

                        // 将物品放入目标存储空间
                        if (targetStorage->addItem(std::move(unequippedItem))) {
                            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "成功将物品放入目标存储空间");
                        }
                        else {
                            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "无法将物品放入目标存储空间");
                        }

                        // 更新UI
                        updatePlayerUI(player);
                    }
                    else {
                        // 卸下失败
                        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "装备卸下失败");
                    }
                    });
            }
            // 如果是手持物品
            else if (isHeldItem) {
                // 使用行动队列：先卸下手持物品，然后存储到目标存储空间
                player->unequipItem(EquipSlot::RIGHT_HAND, [this, player, targetStorage](std::unique_ptr<Item> unequippedItem) {
                            if (unequippedItem) {
                        // 卸下成功，使用行动队列将物品存储到目标存储空间
                        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "成功卸下手持物品: %s，通过行动队列放入目标存储空间", unequippedItem->getName().c_str());

                        // 创建存储行为并添加到行动队列
                        player->storeItemWithAction(std::move(unequippedItem), targetStorage);
                        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "已将手持物品存储行为添加到行动队列");

                                // 更新UI
                                updatePlayerUI(player);
                            }
                            else {
                                // 卸下失败
                                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "手持物品卸下失败");
                            }
                            });
            }
        }
        // 如果是普通物品转移（源存储空间存在且与目标存储空间不同）
        else if (sourceStorage && targetStorage != sourceStorage) {
            // 使用Player的transferItem方法，通过行动队列实现物品转移
            bool transferResult = player->transferItem(draggedItem, sourceStorage, targetStorage, [this, player](bool success) {
                if (success) {
                    // 转移成功，更新UI
                    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "成功将物品移动到目标存储空间，更新UI");
                    updatePlayerUI(player);
                }
                else {
                    // 转移失败
                    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "物品转移失败");
                }
                });

            if (!transferResult) {
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "物品转移操作未能添加到行为队列");
            }
        }
        else {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "目标存储空间无效或与源存储空间相同，或者物品不符合转移条件");
        }
    }
    else {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "未找到有效的目标存储空间");
    }

    // 重置拖拽状态
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "结束拖拽物品: isDragging=%d, 目标存储空间=%s",
        isDragging ? 1 : 0,
        targetStorage ? targetStorage->getName().c_str() : "未知");

    isDragging = false;
    draggedItem = nullptr;
    sourceStorage = nullptr;

    return true;
}



// 显示确认对话框
void GameUI::showConfirmationDialog(const std::string& title, const std::string& message, 
                                   const std::string& confirmText, const std::string& cancelText,
                                   std::function<void(bool)> callback) {
    if (!confirmationWindow) return;
    
    // 保存回调函数
    confirmationCallback = callback;
    
    // 暂停游戏
    if (Game::getInstance()) {
        originalTimeScaleBeforeConfirmation = Game::getInstance()->getTimeScale();
        Game::getInstance()->setTimeScale(0.0f);
    }
    
    // 获取实际窗口尺寸
    float screenWidth = 1920.0f; 
    float screenHeight = 1080.0f;
    if (Game::getInstance()) {
        screenWidth = static_cast<float>(Game::getInstance()->getWindowWidth());
        screenHeight = static_cast<float>(Game::getInstance()->getWindowHeight());
    }
    
    // 设置确认框宽度为屏幕宽度的1/4
    float confirmationWidth = screenWidth / 4.0f;
    confirmationWindow->setMaxContentWidth(confirmationWidth - 50.0f); // 减去边距
    
    // 更新窗口内容
    updateConfirmationDialog(title, message, confirmText, cancelText);
    
    // 手动调整大小并居中（确保正确的顺序）
    confirmationWindow->autoSizeToContent();
    confirmationWindow->centerOnScreen(screenWidth, screenHeight);
    
    // 显示窗口
    isConfirmationVisible = true;
    confirmationWindow->setVisible(true);
}

// 隐藏确认对话框
void GameUI::hideConfirmationDialog() {
    if (!confirmationWindow) return;
    
    isConfirmationVisible = false;
    confirmationWindow->setVisible(false);
    
    // 恢复游戏倍率
    if (Game::getInstance()) {
        Game::getInstance()->setTimeScale(originalTimeScaleBeforeConfirmation);
    }
    
    // 清理回调函数
    confirmationCallback = nullptr;
}

// 更新确认对话框内容
void GameUI::updateConfirmationDialog(const std::string& title, const std::string& message, 
                                     const std::string& confirmText, const std::string& cancelText) {
    if (!confirmationWindow) return;
    
    confirmationWindow->clearElements();
    
    // 添加标题
    UIElement titleElement(title, 0.0f, 15.0f, {255, 255, 255, 255}, UIElementType::TITLE);
    confirmationWindow->addElement(titleElement);
    
    // 添加消息文本（会自动换行）
    UIElement messageElement(message, 0.0f, 25.0f, {220, 220, 220, 255}, UIElementType::TEXT);
    confirmationWindow->addElement(messageElement);
    
    // 添加按钮间距
    UIElement spacer("", 0.0f, 30.0f, {0, 0, 0, 0}, UIElementType::TEXT);
    confirmationWindow->addElement(spacer);
    
    // 添加确认按钮 - 使用相对较近的位置让按钮在同一行
    UIElement confirmButton(confirmText, 50.0f, 45.0f, {100, 255, 100, 255}, UIElementType::TEXT);
    confirmButton.setDataPtr(reinterpret_cast<void*>(1)); // 使用非零值表示确认
    confirmationWindow->addElement(confirmButton);
    
    // 添加取消按钮 - 确认按钮文本宽度大约为120像素，间距40像素
    float cancelButtonX = 50.0f + 120.0f + 40.0f; // 确认按钮X + 预估宽度 + 间距
    UIElement cancelButton(cancelText, cancelButtonX, 45.0f, {255, 100, 100, 255}, UIElementType::TEXT);
    cancelButton.setDataPtr(nullptr); // 使用nullptr表示取消
    confirmationWindow->addElement(cancelButton);
}

// 处理确认对话框的点击事件
void GameUI::handleConfirmationClick(const UIElement& element) {
    // 检查是否为存储选择确认框
    if (pendingHeldItemToReplace) {
        handleStorageSelectionConfirmationClick(element);
        return;
    }
    
    // 检查是否为单发装填/卸载Action的确认框
    void* dataPtr = element.getDataPtr();
    if (dataPtr) {
        std::string* actionData = static_cast<std::string*>(dataPtr);
        if (actionData && (actionData->find("load_single_ammo:") == 0 || actionData->find("unload_single_ammo:") == 0)) {
            std::cout << "确认框点击 - 处理弹药Action: " << *actionData << std::endl;
            handleAmmoActionConfirmationClick(*actionData);
            delete actionData; // 释放分配的内存
            hideConfirmationDialog();
            return;
        } else {
            std::cout << "确认框点击 - dataPtr非空但不是弹药Action" << std::endl;
        }
    } else {
        std::cout << "确认框点击 - dataPtr为空" << std::endl;
    }
    
    // 检查取消按钮
    if (element.getText() == "取消" || element.getText().find("cancel_") == 0) {
        hideConfirmationDialog();
        return;
    }
    
    // 普通确认框处理
    // 根据dataPtr判断是确认还是取消
    bool confirmed = (dataPtr != nullptr);
    
    // 调用回调函数
    if (confirmationCallback) {
        confirmationCallback(confirmed);
    }
    
    // 隐藏对话框
    hideConfirmationDialog();
}

// 显示存储选择确认框
void GameUI::showStorageSelectionConfirmationDialog(Item* currentHeldItem, Item* newItem, Storage* newItemSource) {
    if (!confirmationWindow || !currentPlayer || !currentHeldItem) return;
    
    // 保存待处理的物品信息
    pendingHeldItemToReplace = currentHeldItem;
    pendingNewItemToHold = newItem;
    pendingNewItemSource = newItemSource;
    
    // 构建确认框内容
    std::string title = "选择存储位置";
    std::string message = "当前手持：" + currentHeldItem->getName();
    
    if (newItem) {
        message += "\n新手持：" + newItem->getName();
    }
    
    message += "\n\n请选择将当前手持物品放入哪个存储空间：\n";
    
    // 获取所有可用的存储空间
    auto storagePairs = currentPlayer->getAllAvailableStorages();
    
    // 为每个可以容纳物品的存储空间添加选项
    for (const auto& [slot, storage] : storagePairs) {
        if (storage && storage->canFitItem(currentHeldItem)) {
            // 计算存储时间
            float storeTime = storage->getAccessTime();
            float takeTime = newItemSource ? newItemSource->getAccessTime() : 0.0f;
            float totalTime = storeTime + takeTime;
            
            message += "\n• " + storage->getName() + 
                      " (" + std::to_string((int)(totalTime * 10) / 10.0f) + "秒)" +
                      " [" + std::to_string(storage->getItemCount()) + "/" + 
                      std::to_string(storage->getMaxItems()) + "]";
        }
    }
    
    // 更新确认框内容
    updateStorageSelectionConfirmationDialog();
    
    // 暂停游戏并显示确认框
    if (Game::getInstance()) {
        originalTimeScaleBeforeConfirmation = Game::getInstance()->getTimeScale();
        Game::getInstance()->setTimeScale(0.0f);
    }
    
    // 获取实际窗口尺寸
    float screenWidth = 1920.0f;
    float screenHeight = 1080.0f;
    if (Game::getInstance()) {
        screenWidth = static_cast<float>(Game::getInstance()->getWindowWidth());
        screenHeight = static_cast<float>(Game::getInstance()->getWindowHeight());
    }
    
    // 设置确认框宽度为屏幕宽度的1/3（稍大一些以容纳存储选项）
    float confirmationWidth = screenWidth / 3.0f;
    confirmationWindow->setMaxContentWidth(confirmationWidth - 50.0f);
    
    // 调整大小并居中
    confirmationWindow->autoSizeToContent();
    confirmationWindow->centerOnScreen(screenWidth, screenHeight);
    
    // 显示窗口
    isConfirmationVisible = true;
    confirmationWindow->setVisible(true);
}

// 更新存储选择确认框内容
void GameUI::updateStorageSelectionConfirmationDialog() {
    if (!confirmationWindow || !currentPlayer || !pendingHeldItemToReplace) return;
    
    confirmationWindow->clearElements();
    
    // 添加标题
    UIElement title("选择存储位置", 20.0f, 50.0f, {255, 255, 255, 255}, UIElementType::TITLE);
    confirmationWindow->addElement(title);
    
    // 添加当前手持物品信息
    std::string currentHeldText = "当前手持：" + pendingHeldItemToReplace->getName();
    UIElement currentHeld(currentHeldText, 20.0f, 40.0f, {200, 200, 200, 255}, UIElementType::TEXT);
    confirmationWindow->addElement(currentHeld);
    
    // 添加新手持物品信息（如果有）
    if (pendingNewItemToHold) {
        std::string newHeldText = "新手持：" + pendingNewItemToHold->getName();
        UIElement newHeld(newHeldText, 20.0f, 35.0f, {200, 200, 200, 255}, UIElementType::TEXT);
        confirmationWindow->addElement(newHeld);
    }
    
    // 添加说明
    UIElement instruction("请选择将当前手持物品放入哪个存储空间：", 20.0f, 40.0f, {255, 255, 0, 255}, UIElementType::TEXT);
    confirmationWindow->addElement(instruction);
    
    // 添加间距
    UIElement spacer1("", 0.0f, 20.0f, {0, 0, 0, 0}, UIElementType::TEXT);
    confirmationWindow->addElement(spacer1);
    
    // 获取所有可用的存储空间
    auto storagePairs = currentPlayer->getAllAvailableStorages();
    
    // 遍历所有存储空间，显示可以容纳当前手持物品的存储空间
    for (const auto& [slot, storage] : storagePairs) {
        if (storage && storage->canFitItem(pendingHeldItemToReplace)) {
            // 计算存储时间
            float storeTime = storage->getAccessTime();
            float takeTime = pendingNewItemSource ? pendingNewItemSource->getAccessTime() : 0.0f;
            float totalTime = storeTime + takeTime;
            
            // 创建存储选项文本
            std::string optionText = storage->getName() + 
                                   " (" + std::to_string((int)(totalTime * 10) / 10.0f) + "秒)" +
                                   " [" + std::to_string(storage->getItemCount()) + "/" + 
                                   std::to_string(storage->getMaxItems()) + "]";
            
            // 创建UI元素
            UIElement storageOption(optionText, 40.0f, 35.0f, {100, 255, 100, 255}, UIElementType::TEXT);
            storageOption.setDataPtr(storage); // 将存储空间指针绑定到元素
            confirmationWindow->addElement(storageOption);
        }
    }
    
    // 添加取消按钮
    UIElement spacer2("", 0.0f, 20.0f, {0, 0, 0, 0}, UIElementType::TEXT);
    confirmationWindow->addElement(spacer2);
    
    UIElement cancelButton("取消", 40.0f, 35.0f, {255, 100, 100, 255}, UIElementType::TEXT);
    cancelButton.setDataPtr(nullptr); // 取消按钮没有关联数据
    confirmationWindow->addElement(cancelButton);
}

// 处理存储选择确认框的点击事件
void GameUI::handleStorageSelectionConfirmationClick(const UIElement& element) {
    if (!currentPlayer || !pendingHeldItemToReplace) return;
    
    void* dataPtr = element.getDataPtr();
    
    // 检查是否点击了取消按钮
    if (element.getText() == "取消" || dataPtr == nullptr) {
        hideConfirmationDialog();
        // 清理待处理的物品信息
        pendingHeldItemToReplace = nullptr;
        pendingNewItemToHold = nullptr;
        pendingNewItemSource = nullptr;
        return;
    }
    
    // 检查是否选择了存储空间
    Storage* selectedStorage = static_cast<Storage*>(dataPtr);
    if (selectedStorage && selectedStorage->canFitItem(pendingHeldItemToReplace)) {
        // 创建行为序列：1. 放下当前手持物品，2. 拿起新物品
        
        // 先卸下当前手持物品到选定的存储空间
        currentPlayer->unequipItem(EquipSlot::RIGHT_HAND, [this, selectedStorage](std::unique_ptr<Item> unequippedItem) {
            if (unequippedItem) {
                // 将卸下的物品放入选定的存储空间
                bool addResult = selectedStorage->addItem(std::move(unequippedItem));
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "将手持物品放入存储空间: %s", addResult ? "成功" : "失败");
                
                // 更新UI
                if (currentPlayer) {
                    updatePlayerUI(currentPlayer);
                }
            }
        });
        
        // 然后从源存储空间中取出新物品并手持
        if (pendingNewItemToHold && pendingNewItemSource) {
            currentPlayer->holdItemFromStorage(pendingNewItemToHold, pendingNewItemSource);
        }
        
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "装备替换序列已添加到行为队列");
        
        // 隐藏确认框
        hideConfirmationDialog();
        
        // 清理待处理的物品信息
        pendingHeldItemToReplace = nullptr;
        pendingNewItemToHold = nullptr;
        pendingNewItemSource = nullptr;
    }
}

// 测试存储选择确认框
void GameUI::testStorageSelectionDialog() {
    if (!currentPlayer) return;
    
    // 创建测试物品
    auto testCurrentItem = std::make_unique<Item>("测试当前手持物品", 1.0f, 1.0f, 1.0f, 100.0f);
    auto testNewItem = std::make_unique<Item>("测试新物品", 1.5f, 1.2f, 1.1f, 150.0f);
    
    // 获取第一个可用的存储空间作为源
    auto storagePairs = currentPlayer->getAllAvailableStorages();
    Storage* testSource = nullptr;
    if (!storagePairs.empty()) {
        testSource = storagePairs.begin()->second;
    }
    
    // 显示存储选择确认框
    showStorageSelectionConfirmationDialog(testCurrentItem.get(), testNewItem.get(), testSource);
    
    // 注意：这里我们不释放物品指针，因为它们只是用于测试显示
    testCurrentItem.release();
    testNewItem.release();
}

void GameUI::updateHandSlotRect() {
    // 重置手持位坐标有效性
    handSlotRectValid = false;
    
    // 检查玩家窗口是否存在且在装备栏标签页
    if (!isUIVisible || currentTab != TabType::EQUIPMENT || !currentPlayer) {
        return;
    }
    
    UIWindow* currentWindow = getCurrentTabWindow();
    if (!currentWindow) {
        return;
    }
    
    // 获取所有UI元素
    const auto& elements = currentWindow->getElements();
    
    // 精确定位手持物品element
    // 布局顺序：
    // 0: title ("玩家背包")
    // 1: handSlotTitle ("手持物品")  
    // 2: heldItem/emptyHandElement (<空> 或实际手持物品)
    // 3: spacer1
    // 4: equipTitle...
    
    int handSlotElementIndex = 2; // 手持物品element固定在索引2
    
    // 验证索引有效性和element内容
    if (handSlotElementIndex < static_cast<int>(elements.size())) {
        const auto& element = elements[handSlotElementIndex];
        const std::string& text = element.getText();
        
        // 验证这确实是手持位element（显示<空>或有手持物品数据）
        bool isHandSlotElement = (text == "<空>" || 
                                 (element.getDataPtr() != nullptr && 
                                  currentPlayer->getHeldItem() == static_cast<Item*>(element.getDataPtr())));
        
        if (isHandSlotElement) {
            // 获取element的渲染区域
            if (currentWindow->getElementRect(handSlotElementIndex, handSlotRect)) {
                handSlotRectValid = true;
                //SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "手持位区域更新: (%.1f,%.1f,%.1f,%.1f), 内容='%s'", 
                //           handSlotRect.x, handSlotRect.y, handSlotRect.width, handSlotRect.height, text.c_str());
            }
        } else {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "索引2的element不是手持位element: '%s'", text.c_str());
        }
    } else {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "UI元素数量不足，无法找到手持位element");
    }
}

// 标签页系统实现

// 初始化标签页窗口
void GameUI::initializeTabWindows() {
    // 计算窗口位置和尺寸（为标题栏留出空间）
    float windowX = 30.0f;
    float windowY = 30.0f + TAB_HEIGHT; // 标题栏下方
    float windowWidth = 0.0f; // 自动调整
    float windowHeight = 0.0f; // 自动调整
    
    // 创建装备栏窗口（原来的playerWindow）
    equipmentWindow = std::make_unique<UIWindow>(windowX, windowY, windowWidth, windowHeight, 
                                               SDL_Color{100, 100, 255, 255}, 180);
    equipmentWindow->setVisible(false);
    equipmentWindow->setElementClickCallback([this](const UIElement& element) {
        this->onElementClick(element);
    });
    equipmentWindow->setFonts(titleFont, subtitleFont, itemFont);
    equipmentWindow->setScrollEnabled(true); // 启用滚动
    
    // 创建血量情况窗口
    healthWindow = std::make_unique<UIWindow>(windowX, windowY, windowWidth, windowHeight, 
                                            SDL_Color{255, 100, 100, 255}, 180);
    healthWindow->setVisible(false);
    healthWindow->setElementClickCallback([this](const UIElement& element) {
        // 血量窗口的点击处理（暂时空实现）
    });
    healthWindow->setFonts(titleFont, subtitleFont, itemFont);
    healthWindow->setScrollEnabled(true); // 启用滚动
    
    // 创建技能等级窗口
    skillsWindow = std::make_unique<UIWindow>(windowX, windowY, windowWidth, windowHeight, 
                                            SDL_Color{100, 255, 100, 255}, 180);
    skillsWindow->setVisible(false);
    skillsWindow->setElementClickCallback([this](const UIElement& element) {
        // 技能窗口的点击处理（暂时空实现）
    });
    skillsWindow->setFonts(titleFont, subtitleFont, itemFont);
    skillsWindow->setScrollEnabled(true); // 启用滚动
}

// 切换到指定标签页
void GameUI::switchToTab(TabType tab) {
    // 隐藏所有窗口
    equipmentWindow->setVisible(false);
    healthWindow->setVisible(false);
    skillsWindow->setVisible(false);
    
    // 更新当前标签页
    currentTab = tab;
    
    // 如果UI可见，显示对应的窗口
    if (isUIVisible) {
        UIWindow* targetWindow = getCurrentTabWindow();
        if (targetWindow) {
            targetWindow->setVisible(true);
            
            // 更新窗口内容
            if (currentPlayer) {
                if (tab == TabType::EQUIPMENT) {
                    updatePlayerUI(currentPlayer);
                } else if (tab == TabType::HEALTH) {
                    updateHealthUI();
                } else if (tab == TabType::SKILLS) {
                    updateSkillsUI();
                }
            }
        }
    }
}

// 渲染标签栏
void GameUI::renderTabBar(SDL_Renderer* renderer, float windowWidth, float windowHeight) {
    if (!isUIVisible) return;
    
    float tabWidth = 200.0f; // 增加标签宽度，避免文字重叠
    float tabStartX = 30.0f;
    float tabY = 30.0f;
    
    for (int i = 0; i < TAB_COUNT; i++) {
        TabType tab = static_cast<TabType>(i);
        float tabX = tabStartX + i * tabWidth;
        
        // 设置标签颜色（选中的标签更亮）
        SDL_Color tabColor;
        if (tab == currentTab) {
            tabColor = {200, 200, 255, 255}; // 选中标签：亮蓝色
        } else {
            tabColor = {100, 100, 150, 255}; // 未选中标签：深蓝色
        }
        
        // 绘制标签背景
        SDL_FRect tabRect = {tabX, tabY, tabWidth, TAB_HEIGHT};
        SDL_SetRenderDrawColor(renderer, tabColor.r, tabColor.g, tabColor.b, tabColor.a);
        SDL_RenderFillRect(renderer, &tabRect);
        
        // 绘制标签边框
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderRect(renderer, &tabRect);
        
        // 绘制标签文字
        const char* tabName = getTabName(tab);
        if (tabName && subtitleFont) {
            SDL_Surface* textSurface = TTF_RenderText_Blended(subtitleFont, tabName, 0, {255, 255, 255, 255});
            if (textSurface) {
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                if (textTexture) {
                    // 计算文字居中位置
                    float textX = tabX + (tabWidth - textSurface->w) / 2.0f;
                    float textY = tabY + (TAB_HEIGHT - textSurface->h) / 2.0f;
                    
                    SDL_FRect textRect = {textX, textY, (float)textSurface->w, (float)textSurface->h};
                    SDL_RenderTexture(renderer, textTexture, nullptr, &textRect);
                    
                    SDL_DestroyTexture(textTexture);
                }
                SDL_DestroySurface(textSurface);
            }
        }
    }
}

// 处理标签栏点击事件
bool GameUI::handleTabBarClick(int mouseX, int mouseY, float windowWidth, float windowHeight) {
    if (!isUIVisible) return false;
    
    float tabWidth = 200.0f; // 与renderTabBar保持一致
    float tabStartX = 30.0f;
    float tabY = 30.0f;
    
    // 检查点击是否在标签栏区域内
    if (mouseY >= tabY && mouseY <= tabY + TAB_HEIGHT) {
        for (int i = 0; i < TAB_COUNT; i++) {
            float tabX = tabStartX + i * tabWidth;
            if (mouseX >= tabX && mouseX <= tabX + tabWidth) {
                // 点击了标签页i
                TabType clickedTab = static_cast<TabType>(i);
                if (clickedTab != currentTab) {
                    switchToTab(clickedTab);
                }
                return true; // 消费掉点击事件
            }
        }
    }
    
    return false;
}

// 获取当前标签页对应的窗口
UIWindow* GameUI::getCurrentTabWindow() {
    switch (currentTab) {
        case TabType::EQUIPMENT:
            return equipmentWindow.get();
        case TabType::HEALTH:
            return healthWindow.get();
        case TabType::SKILLS:
            return skillsWindow.get();
        default:
            return equipmentWindow.get();
    }
}

// 获取标签页名称
const char* GameUI::getTabName(TabType tab) const {
    switch (tab) {
        case TabType::EQUIPMENT:
            return "装备栏";
        case TabType::HEALTH:
            return "角色状态";
        case TabType::SKILLS:
            return "技能等级";
        default:
            return "未知";
    }
}

// 更新血量情况UI
void GameUI::updateHealthUI() {
    if (!healthWindow || !currentPlayer) return;
    
    healthWindow->clearElements();
    
    // 添加标题
    UIElement title("角色状态", 20.0f, 60.0f, {255, 255, 255, 255}, UIElementType::TITLE);
    healthWindow->addElement(title);
    
    // 添加间距
    UIElement spacer1("", 0.0f, 25.0f, {255, 255, 255, 255}, UIElementType::TEXT);
    healthWindow->addElement(spacer1);
    
    // 获取玩家血量数据
    int headHealth = currentPlayer->getHeadHealth();
    int torsoHealth = currentPlayer->getTorsoHealth();
    int leftLegHealth = currentPlayer->getLeftLegHealth();
    int rightLegHealth = currentPlayer->getRightLegHealth();
    int leftArmHealth = currentPlayer->getLeftArmHealth();
    int rightArmHealth = currentPlayer->getRightArmHealth();
    
    int maxHeadHealth = Player::getMaxHealthForBodyPart(Player::BodyPart::HEAD);
    int maxTorsoHealth = Player::getMaxHealthForBodyPart(Player::BodyPart::TORSO);
    int maxLegHealth = Player::getMaxHealthForBodyPart(Player::BodyPart::LEFT_LEG);
    int maxArmHealth = Player::getMaxHealthForBodyPart(Player::BodyPart::LEFT_ARM);
    
    // 计算总血量
    int currentTotalHealth = headHealth + torsoHealth + leftLegHealth + rightLegHealth + leftArmHealth + rightArmHealth;
    int maxTotalHealth = maxHeadHealth + maxTorsoHealth + maxLegHealth * 2 + maxArmHealth * 2;
    
    // 显示总体血量概览
    UIElement totalHealthTitle("总体血量:", 20.0f, 45.0f, {255, 215, 0, 255}, UIElementType::SUBTITLE);
    healthWindow->addElement(totalHealthTitle);
    
    // 总血量数值和百分比
    float healthPercentage = (float)currentTotalHealth / maxTotalHealth * 100.0f;
    std::string totalHealthText = std::to_string(currentTotalHealth) + "/" + std::to_string(maxTotalHealth) + 
                                 " (" + formatFloat(healthPercentage) + "%)";
    
    // 根据血量百分比设置颜色
    SDL_Color totalHealthColor;
    if (healthPercentage >= 80.0f) {
        totalHealthColor = {50, 255, 50, 255};   // 绿色 - 健康
    } else if (healthPercentage >= 50.0f) {
        totalHealthColor = {255, 255, 50, 255};  // 黄色 - 轻伤
    } else if (healthPercentage >= 25.0f) {
        totalHealthColor = {255, 165, 0, 255};   // 橙色 - 重伤
    } else {
        totalHealthColor = {255, 50, 50, 255};   // 红色 - 危险
    }
    
    UIElement totalHealthValue(totalHealthText, 40.0f, 32.0f, totalHealthColor, UIElementType::TEXT);
    healthWindow->addElement(totalHealthValue);
    
    // 添加分割线
    UIElement spacer2("", 0.0f, 35.0f, {255, 255, 255, 255}, UIElementType::TEXT);
    healthWindow->addElement(spacer2);
    
    // 显示各部位详细血量
    UIElement detailTitle("身体部位详情:", 20.0f, 45.0f, {200, 200, 255, 255}, UIElementType::SUBTITLE);
    healthWindow->addElement(detailTitle);
    
    // 头部血量
    std::string headText = "头部: " + std::to_string(headHealth) + "/" + std::to_string(maxHeadHealth);
    float headPercentage = (float)headHealth / maxHeadHealth * 100.0f;
    SDL_Color headColor = (headPercentage >= 70.0f) ? SDL_Color{50, 255, 50, 255} :
                         (headPercentage >= 40.0f) ? SDL_Color{255, 255, 50, 255} :
                         (headPercentage >= 15.0f) ? SDL_Color{255, 165, 0, 255} :
                                                    SDL_Color{255, 50, 50, 255};
    UIElement headElement(headText, 40.0f, 32.0f, headColor, UIElementType::TEXT);
    healthWindow->addElement(headElement);
    
    // 躯干血量
    std::string torsoText = "躯干: " + std::to_string(torsoHealth) + "/" + std::to_string(maxTorsoHealth);
    float torsoPercentage = (float)torsoHealth / maxTorsoHealth * 100.0f;
    SDL_Color torsoColor = (torsoPercentage >= 70.0f) ? SDL_Color{50, 255, 50, 255} :
                          (torsoPercentage >= 40.0f) ? SDL_Color{255, 255, 50, 255} :
                          (torsoPercentage >= 15.0f) ? SDL_Color{255, 165, 0, 255} :
                                                     SDL_Color{255, 50, 50, 255};
    UIElement torsoElement(torsoText, 40.0f, 32.0f, torsoColor, UIElementType::TEXT);
    healthWindow->addElement(torsoElement);
    
    // 左臂血量
    std::string leftArmText = "左臂: " + std::to_string(leftArmHealth) + "/" + std::to_string(maxArmHealth);
    float leftArmPercentage = (float)leftArmHealth / maxArmHealth * 100.0f;
    SDL_Color leftArmColor = (leftArmPercentage >= 70.0f) ? SDL_Color{50, 255, 50, 255} :
                            (leftArmPercentage >= 40.0f) ? SDL_Color{255, 255, 50, 255} :
                            (leftArmPercentage >= 15.0f) ? SDL_Color{255, 165, 0, 255} :
                                                          SDL_Color{255, 50, 50, 255};
    UIElement leftArmElement(leftArmText, 40.0f, 32.0f, leftArmColor, UIElementType::TEXT);
    healthWindow->addElement(leftArmElement);
    
    // 右臂血量
    std::string rightArmText = "右臂: " + std::to_string(rightArmHealth) + "/" + std::to_string(maxArmHealth);
    float rightArmPercentage = (float)rightArmHealth / maxArmHealth * 100.0f;
    SDL_Color rightArmColor = (rightArmPercentage >= 70.0f) ? SDL_Color{50, 255, 50, 255} :
                             (rightArmPercentage >= 40.0f) ? SDL_Color{255, 255, 50, 255} :
                             (rightArmPercentage >= 15.0f) ? SDL_Color{255, 165, 0, 255} :
                                                           SDL_Color{255, 50, 50, 255};
    UIElement rightArmElement(rightArmText, 40.0f, 32.0f, rightArmColor, UIElementType::TEXT);
    healthWindow->addElement(rightArmElement);
    
    // 左腿血量
    std::string leftLegText = "左腿: " + std::to_string(leftLegHealth) + "/" + std::to_string(maxLegHealth);
    float leftLegPercentage = (float)leftLegHealth / maxLegHealth * 100.0f;
    SDL_Color leftLegColor = (leftLegPercentage >= 70.0f) ? SDL_Color{50, 255, 50, 255} :
                            (leftLegPercentage >= 40.0f) ? SDL_Color{255, 255, 50, 255} :
                            (leftLegPercentage >= 15.0f) ? SDL_Color{255, 165, 0, 255} :
                                                          SDL_Color{255, 50, 50, 255};
    UIElement leftLegElement(leftLegText, 40.0f, 32.0f, leftLegColor, UIElementType::TEXT);
    healthWindow->addElement(leftLegElement);
    
    // 右腿血量
    std::string rightLegText = "右腿: " + std::to_string(rightLegHealth) + "/" + std::to_string(maxLegHealth);
    float rightLegPercentage = (float)rightLegHealth / maxLegHealth * 100.0f;
    SDL_Color rightLegColor = (rightLegPercentage >= 70.0f) ? SDL_Color{50, 255, 50, 255} :
                             (rightLegPercentage >= 40.0f) ? SDL_Color{255, 255, 50, 255} :
                             (rightLegPercentage >= 15.0f) ? SDL_Color{255, 165, 0, 255} :
                                                           SDL_Color{255, 50, 50, 255};
    UIElement rightLegElement(rightLegText, 40.0f, 32.0f, rightLegColor, UIElementType::TEXT);
    healthWindow->addElement(rightLegElement);
    
    // 添加分割线
    UIElement spacer3("", 0.0f, 35.0f, {255, 255, 255, 255}, UIElementType::TEXT);
    healthWindow->addElement(spacer3);
    
    // 显示健康状态说明
    UIElement statusTitle("健康状态:", 20.0f, 45.0f, {200, 200, 255, 255}, UIElementType::SUBTITLE);
    healthWindow->addElement(statusTitle);
    
    // 根据关键部位血量给出状态描述
    std::string healthStatus;
    SDL_Color statusColor;
    
    if (headHealth <= 0 || torsoHealth <= 0) {
        healthStatus = "危急！关键部位受损严重";
        statusColor = {255, 50, 50, 255};
    } else if (headHealth < maxHeadHealth * 0.3f || torsoHealth < maxTorsoHealth * 0.3f) {
        healthStatus = "重伤！需要立即治疗";
        statusColor = {255, 100, 50, 255};
    } else if (currentTotalHealth < maxTotalHealth * 0.5f) {
        healthStatus = "中度受伤，建议休息治疗";
        statusColor = {255, 200, 50, 255};
    } else if (currentTotalHealth < maxTotalHealth * 0.8f) {
        healthStatus = "轻度受伤，状态良好";
        statusColor = {255, 255, 100, 255};
    } else {
        healthStatus = "身体健康，状态良好";
        statusColor = {100, 255, 100, 255};
    }
    
    UIElement statusElement(healthStatus, 40.0f, 32.0f, statusColor, UIElementType::TEXT);
    healthWindow->addElement(statusElement);
    
    // 添加分割线
    UIElement spacer4("", 0.0f, 35.0f, {255, 255, 255, 255}, UIElementType::TEXT);
    healthWindow->addElement(spacer4);
    
    // 显示防护等级信息
    UIElement protectionTitle("防护等级:", 20.0f, 45.0f, {255, 200, 100, 255}, UIElementType::SUBTITLE);
    healthWindow->addElement(protectionTitle);
    
    // 获取装备系统
    EquipmentSystem* equipmentSystem = currentPlayer->getEquipmentSystem();
    if (equipmentSystem) {
        // 创建一个映射来汇总每个身体部位的防护值
        std::map<EquipSlot, std::map<DamageType, int>> totalProtection;
        
        // 获取所有已装备的物品
        std::vector<Item*> equippedItems = equipmentSystem->getAllEquippedItems();
        
        // 遍历所有已装备物品，汇总防护值
        for (Item* item : equippedItems) {
            if (item && item->hasFlag(ItemFlag::WEARABLE)) {
                const auto& protectionData = item->getProtectionData();
                
                for (const auto& protection : protectionData) {
                    EquipSlot bodyPart = protection.bodyPart;
                    
                    // 累加各种伤害类型的防护值
                    std::vector<DamageType> mainProtectionTypes = {
                        DamageType::BLUNT, DamageType::SLASH, DamageType::PIERCE,
                        DamageType::ELECTRIC, DamageType::BURN, DamageType::HEAT,
                        DamageType::COLD, DamageType::EXPLOSION, DamageType::SHOOTING
                    };
                    
                    for (DamageType damageType : mainProtectionTypes) {
                        int protectionValue = protection.getProtection(damageType);
                        if (protectionValue > 0) {
                            totalProtection[bodyPart][damageType] += protectionValue;
                        }
                    }
                }
            }
        }
        
        // 显示各个身体部位的防护信息
        std::vector<EquipSlot> bodyParts = {
            EquipSlot::HEAD, EquipSlot::CHEST, EquipSlot::ABDOMEN,
            EquipSlot::LEFT_ARM, EquipSlot::RIGHT_ARM,
            EquipSlot::LEFT_LEG, EquipSlot::RIGHT_LEG
        };
        
        bool hasAnyProtection = false;
        for (EquipSlot bodyPart : bodyParts) {
            if (totalProtection.find(bodyPart) != totalProtection.end() && !totalProtection[bodyPart].empty()) {
                hasAnyProtection = true;
                
                // 身体部位名称
                std::string partName;
                switch (bodyPart) {
                    case EquipSlot::HEAD: partName = "头部"; break;
                    case EquipSlot::CHEST: partName = "胸部"; break;
                    case EquipSlot::ABDOMEN: partName = "腹部"; break;
                    case EquipSlot::LEFT_ARM: partName = "左臂"; break;
                    case EquipSlot::RIGHT_ARM: partName = "右臂"; break;
                    case EquipSlot::LEFT_LEG: partName = "左腿"; break;
                    case EquipSlot::RIGHT_LEG: partName = "右腿"; break;
                    default: partName = "未知"; break;
                }
                
                UIElement partTitle(partName + "防护:", 40.0f, 35.0f, {200, 255, 200, 255}, UIElementType::TEXT);
                healthWindow->addElement(partTitle);
                
                // 显示该部位的防护值（按伤害类型分类）
                for (const auto& protectionPair : totalProtection[bodyPart]) {
                    DamageType damageType = protectionPair.first;
                    int protectionValue = protectionPair.second;
                    
                    if (protectionValue > 0) {
                        std::string damageTypeName = damageTypeToString(damageType);
                        std::string protectionText = "  " + damageTypeName + ": " + std::to_string(protectionValue);
                        
                        // 根据防护值设置颜色
                        SDL_Color protectionColor;
                        if (protectionValue >= 40) {
                            protectionColor = {100, 255, 100, 255};  // 绿色 - 高防护
                        } else if (protectionValue >= 20) {
                            protectionColor = {255, 255, 100, 255};  // 黄色 - 中等防护
                        } else {
                            protectionColor = {255, 200, 100, 255};  // 橙色 - 低防护
                        }
                        
                        UIElement protectionElement(protectionText, 60.0f, 30.0f, protectionColor, UIElementType::TEXT);
                        healthWindow->addElement(protectionElement);
                    }
                }
                
                // 部位间间距
                UIElement partSpacer("", 0.0f, 10.0f, {255, 255, 255, 255}, UIElementType::TEXT);
                healthWindow->addElement(partSpacer);
            }
        }
        
        // 如果没有任何防护装备
        if (!hasAnyProtection) {
            UIElement noProtectionElement("未装备任何防护装备", 40.0f, 32.0f, {150, 150, 150, 255}, UIElementType::TEXT);
            healthWindow->addElement(noProtectionElement);
        }
    } else {
        UIElement errorElement("无法获取装备信息", 40.0f, 32.0f, {255, 100, 100, 255}, UIElementType::TEXT);
        healthWindow->addElement(errorElement);
    }
}

// 更新技能等级UI
void GameUI::updateSkillsUI() {
    if (!skillsWindow || !currentPlayer) return;
    
    skillsWindow->clearElements();
    
    // 添加标题
    UIElement title("技能等级", 20.0f, 50.0f, {255, 255, 255, 255}, UIElementType::TITLE);
    skillsWindow->addElement(title);
    
    // 添加间距
    UIElement spacer1("", 0.0f, 20.0f, {255, 255, 255, 255}, UIElementType::TEXT);
    skillsWindow->addElement(spacer1);
    
    // 获取技能系统
    SkillSystem* skillSystem = currentPlayer->getSkillSystem();
    if (!skillSystem) return;
    
    // 获取所有技能数据
    const auto& allSkills = skillSystem->getAllSkills();
    
    // 按分类显示技能
    std::vector<std::string> categories = {"火器技能", "近战技能", "生活技能"};
    
    for (const std::string& category : categories) {
        // 添加分类标题
        UIElement categoryTitle(category, 20.0f, 45.0f, {255, 255, 100, 255}, UIElementType::SUBTITLE);
        skillsWindow->addElement(categoryTitle);
        
        // 添加该分类下的技能
        for (const auto& skillPair : allSkills) {
            SkillType skillType = skillPair.first;
            const Skill& skill = skillPair.second;
            
            // 检查技能是否属于当前分类
            std::string skillCategory = SkillSystem::getSkillCategoryName(skillType);
            if (skillCategory != category) continue;
            
            // 技能名称 - 使用固定宽度对齐
            std::string skillName = SkillSystem::skillTypeToString(skillType);
            UIElement nameElement(skillName, 40.0f, 0.0f, {255, 255, 255, 255}, UIElementType::TEXT);
            skillsWindow->addElement(nameElement);
            
            // 计算进度条起始位置（使用固定宽度200像素确保对齐）
            float progressBarStartX = 40.0f + 200.0f;
            
            // 绘制进度条（20个格子，每格代表1级）
            float gridSize = 20.0f; // 每格的大小
            float gridSpacing = 2.0f; // 格子间距
            
            for (int level = 0; level < 20; level++) {
                float gridX = progressBarStartX + level * (gridSize + gridSpacing);
                
                // 确定格子颜色
                SDL_Color gridColor;
                if (level < skill.level) {
                    // 已升级的等级 - 白色边框，填充
                    gridColor = {255, 255, 255, 255};
                } else if (level == skill.level && skill.level < 20) {
                    // 正在升级的等级 - 白色边框，部分填充
                    gridColor = {255, 255, 255, 255};
                } else {
                    // 未达到的等级 - 灰色边框
                    gridColor = {100, 100, 100, 255};
                }
                
                // 创建格子元素（使用特殊字符表示方块）
                std::string gridChar = (level < skill.level) ? "■" : "□";
                UIElement gridElement(gridChar, gridX, 0.0f, gridColor, UIElementType::TEXT);
                skillsWindow->addElement(gridElement);
            }
            
            // 等级信息（如 2/20）
            float levelInfoX = progressBarStartX + 20 * (gridSize + gridSpacing) + 20.0f;
            std::string levelInfo = std::to_string(skill.level) + "/20";
            UIElement levelElement(levelInfo, levelInfoX, 0.0f, {200, 200, 255, 255}, UIElementType::TEXT);
            skillsWindow->addElement(levelElement);
            
            // 经验值百分比
            float expPercentX = levelInfoX + levelInfo.length() * 14.0f + 20.0f;
            std::string expPercent;
            if (skill.level >= 20) {
                expPercent = "MAX";
            } else {
                int percentage = (skill.currentLevelExp * 100) / 100; // 当前等级经验百分比
                expPercent = std::to_string(percentage) + "%";
            }
            UIElement expElement(expPercent, expPercentX, 0.0f, {100, 255, 100, 255}, UIElementType::TEXT);
            skillsWindow->addElement(expElement);
            
            // 添加行间距
            UIElement rowSpacer("", 0.0f, 32.0f, {0, 0, 0, 0}, UIElementType::TEXT);
            skillsWindow->addElement(rowSpacer);
        }
        
        // 分类间距
        UIElement categorySpacer("", 0.0f, 20.0f, {0, 0, 0, 0}, UIElementType::TEXT);
        skillsWindow->addElement(categorySpacer);
    }
}

// 处理滚轮事件
bool GameUI::handleScroll(int mouseX, int mouseY, float scrollDelta) {
    // 如果UI不可见，不处理滚动事件
    if (!isUIVisible) return false;
    
    // 首先检查物品提示框是否处理了滚动事件
    if (itemTooltipWindow && itemTooltipWindow->getVisible()) {
        if (itemTooltipWindow->handleScroll(mouseX, mouseY, scrollDelta)) {
            return true;
        }
    }
    
    // 检查确认对话框是否处理了滚动事件
    if (confirmationWindow && confirmationWindow->getVisible()) {
        if (confirmationWindow->handleScroll(mouseX, mouseY, scrollDelta)) {
            return true;
        }
    }
    
    // 检查当前标签页窗口是否处理了滚动事件
    UIWindow* currentWindow = getCurrentTabWindow();
    if (currentWindow && currentWindow->getVisible()) {
        if (currentWindow->handleScroll(mouseX, mouseY, scrollDelta)) {
            return true;
        }
    }
    
    return false; // 如果没有窗口处理滚动事件，返回false
}

// 右键菜单相关方法实现

// 显示右键菜单
void GameUI::showRightClickMenu(int mouseX, int mouseY, Item* item, Storage* storage) {
    if (!rightClickMenuWindow || !item) {
        return;
    }
    
    // 隐藏其他可能显示的菜单
    hideRightClickMenu();
    
    // 保存右键菜单相关信息
    rightClickTargetItem = item;
    rightClickTargetStorage = storage;
    rightClickMenuX = mouseX;
    rightClickMenuY = mouseY;
    
    // 更新菜单内容
    updateRightClickMenu();
    
    // 设置菜单位置（确保不超出屏幕边界）
    rightClickMenuWindow->setX(static_cast<float>(mouseX));
    rightClickMenuWindow->setY(static_cast<float>(mouseY));
    
    // 显示菜单
    rightClickMenuWindow->setVisible(true);
    isRightClickMenuVisible = true;
    
    std::cout << "显示右键菜单: " << item->getName() << std::endl;
}

// 隐藏右键菜单
void GameUI::hideRightClickMenu() {
    if (rightClickMenuWindow) {
        rightClickMenuWindow->setVisible(false);
    }
    isRightClickMenuVisible = false;
    rightClickTargetItem = nullptr;
    rightClickTargetStorage = nullptr;
}

// 更新右键菜单内容
void GameUI::updateRightClickMenu() {
    if (!rightClickMenuWindow || !rightClickTargetItem) {
        return;
    }
    
    rightClickMenuWindow->clearElements();
    
    // 添加标题（物品名称）
    UIElement title(rightClickTargetItem->getName(), 10.0f, 30.0f, 
                   {255, 255, 200, 255}, UIElementType::SUBTITLE);
    rightClickMenuWindow->addElement(title);
    
    // 添加分隔线
    UIElement separator("─────────────", 10.0f, 20.0f, 
                       {150, 150, 150, 255}, UIElementType::TEXT);
    rightClickMenuWindow->addElement(separator);
    
    // 通用操作：手持
    UIElement holdOption("手持", 15.0f, 35.0f, {255, 255, 255, 255}, UIElementType::TEXT, (void*)"hold");
    rightClickMenuWindow->addElement(holdOption);
    
    // 如果是可穿戴的，添加穿戴选项
    if (rightClickTargetItem->isWearable()) {
        UIElement wearOption("穿戴", 15.0f, 35.0f, {255, 255, 255, 255}, UIElementType::TEXT, (void*)"wear");
        rightClickMenuWindow->addElement(wearOption);
    }
    
    // 如果是枪械，添加改造选项（先不实现）
    if (rightClickTargetItem->hasFlag(ItemFlag::GUN)) {
        UIElement modifyOption("改造（暂未实现）", 15.0f, 35.0f, {150, 150, 150, 255}, UIElementType::TEXT, (void*)"modify");
        rightClickMenuWindow->addElement(modifyOption);
    }
    
    // 如果是弹匣，添加弹匣相关选项
    if (rightClickTargetItem->hasFlag(ItemFlag::MAGAZINE)) {
        Magazine* magazine = static_cast<Magazine*>(rightClickTargetItem);
        
        // 装填子弹选项（只有当弹匣不满时）
        if (!magazine->isFull()) {
            UIElement loadAmmoOption("装填子弹", 15.0f, 35.0f, {255, 255, 255, 255}, UIElementType::TEXT, (void*)"load_ammo");
            rightClickMenuWindow->addElement(loadAmmoOption);
        }
        
        // 卸除子弹选项（只有当弹匣有子弹时）
        if (!magazine->isEmpty()) {
            UIElement unloadAmmoOption("卸除子弹", 15.0f, 35.0f, {255, 255, 255, 255}, UIElementType::TEXT, (void*)"unload_ammo");
            rightClickMenuWindow->addElement(unloadAmmoOption);
        }
    }
    
    // 自动调整窗口大小以适应内容
    rightClickMenuWindow->autoSizeToContent();
}

// 处理右键菜单点击事件
void GameUI::handleRightClickMenuClick(const UIElement& element) {
    if (!rightClickTargetItem) {
        return;
    }
    
    // 获取操作类型
    void* actionData = element.getDataPtr();
    if (!actionData) {
        return;
    }
    
    std::string action = static_cast<const char*>(actionData);
    
    // 执行对应的操作
    performItemAction(action, rightClickTargetItem, rightClickTargetStorage);
    
    // 隐藏菜单
    hideRightClickMenu();
}

// 执行物品操作
void GameUI::performItemAction(const std::string& action, Item* item, Storage* storage) {
    if (!item || !currentPlayer) {
        return;
    }
    
    std::cout << "执行操作: " << action << " 对物品: " << item->getName() << std::endl;
    
    if (action == "hold") {
        // 手持物品 - 需要检查当前手持位是否已有物品
        if (storage) {
            Item* currentHeldItem = currentPlayer->getHeldItem();
            if (currentHeldItem) {
                // 手持位已有物品，显示存储选择确认框
                showStorageSelectionConfirmationDialog(currentHeldItem, item, storage);
            } else {
                // 手持位为空，直接手持
                currentPlayer->holdItemFromStorage(item, storage);
                updatePlayerUI();
            }
        }
    }
    else if (action == "wear") {
        // 穿戴物品
        if (storage && item->isWearable()) {
            // 从存储空间取出物品并穿戴
            currentPlayer->takeItemWithAction(item, storage, [this](std::unique_ptr<Item> takenItem) {
                if (takenItem && this->currentPlayer) {
                    this->currentPlayer->equipItemWithAction(std::move(takenItem));
                    this->updatePlayerUI();
                }
            });
        }
    }
    else if (action == "modify") {
        // 改造枪械（暂未实现）
        std::cout << "枪械改造功能尚未实现" << std::endl;
    }
    else if (action == "load_ammo") {
        // 装填子弹到弹匣
        if (item->hasFlag(ItemFlag::MAGAZINE)) {
            showAmmoSelectionDialog(static_cast<Magazine*>(item), storage);
        }
    }
    else if (action == "unload_ammo") {
        // 卸除子弹从弹匣
        if (item->hasFlag(ItemFlag::MAGAZINE)) {
            showStorageSelectionForUnloadAmmo(static_cast<Magazine*>(item));
        }
    }
}

// 显示子弹选择对话框（用于装填弹匣）
void GameUI::showAmmoSelectionDialog(Magazine* magazine, Storage* magazineStorage) {
    if (!magazine || !currentPlayer) {
        return;
    }
    
    // 获取兼容的弹药类型
    const auto& compatibleTypes = magazine->getCompatibleAmmoTypes();
    
    // 查找所有兼容的子弹
    std::vector<std::tuple<Storage*, int, Ammo*>> compatibleAmmo;
    
    // 遍历所有存储空间寻找兼容子弹
    auto storages = currentPlayer->getAllAvailableStorages();
    for (const auto& storagePair : storages) {
        Storage* storage = storagePair.second;
        for (size_t i = 0; i < storage->getItemCount(); ++i) {
            Item* item = storage->getItem(i);
            if (item && item->hasFlag(ItemFlag::AMMO)) {
                Ammo* ammo = static_cast<Ammo*>(item);
                // 检查弹药类型是否兼容
                for (const auto& compatibleType : compatibleTypes) {
                    if (ammo->getAmmoType() == compatibleType) {
                        compatibleAmmo.push_back(std::make_tuple(storage, static_cast<int>(i), ammo));
                        break;
                    }
                }
            }
        }
    }
    
    if (compatibleAmmo.empty()) {
        showConfirmationDialog("无兼容子弹", "未找到与该弹匣兼容的子弹。", "确定", "", nullptr);
        return;
    }
    
    // 创建子弹选择确认框
    confirmationWindow->clearElements();
    
    // 标题
    UIElement title("选择要装填的子弹（批量）", 20.0f, 40.0f, {255, 255, 255, 255}, UIElementType::TITLE);
    confirmationWindow->addElement(title);
    
    // 弹匣信息
    std::string magazineInfo = "弹匣: " + magazine->getName() + " (" + 
                              std::to_string(magazine->getCurrentAmmoCount()) + "/" + 
                              std::to_string(magazine->getCapacity()) + ")";
    UIElement magazineInfoElement(magazineInfo, 20.0f, 35.0f, {200, 200, 255, 255}, UIElementType::TEXT);
    confirmationWindow->addElement(magazineInfoElement);
    
    // 分隔线
    UIElement separator("─────────────────────", 20.0f, 25.0f, {150, 150, 150, 255}, UIElementType::TEXT);
    confirmationWindow->addElement(separator);
    
    // 添加每个兼容子弹的选项
    for (size_t i = 0; i < compatibleAmmo.size() && i < 10; ++i) { // 最多显示10个选项
        auto [storage, index, ammo] = compatibleAmmo[i];
        
        std::string ammoText = ammo->getName();
        if (ammo->isStackable() && ammo->getStackSize() > 1) {
            ammoText += " (x" + std::to_string(ammo->getStackSize()) + ")";
        }
        ammoText += " [" + storage->getName() + "]";
        
        // 将选择信息编码到dataPtr中
        std::string* actionData = new std::string("load_single_ammo:" + std::to_string(reinterpret_cast<uintptr_t>(magazine)) + 
                                                 ":" + std::to_string(reinterpret_cast<uintptr_t>(ammo)) + 
                                                 ":" + std::to_string(reinterpret_cast<uintptr_t>(storage)));
        
        UIElement ammoOption(ammoText, 25.0f, 35.0f, {255, 255, 255, 255}, UIElementType::TEXT, actionData);
        confirmationWindow->addElement(ammoOption);
    }
    
    // 取消按钮
    UIElement cancelButton("取消", 20.0f, 40.0f, {255, 100, 100, 255}, UIElementType::TEXT, (void*)"cancel_ammo_selection");
    confirmationWindow->addElement(cancelButton);
    
    // 暂停游戏时间
    if (Game::getInstance()) {
        originalTimeScaleBeforeConfirmation = Game::getInstance()->getTimeScale();
        Game::getInstance()->setTimeScale(0.0f);
    }
    
    // 获取实际屏幕尺寸
    float screenWidth = 1920.0f;
    float screenHeight = 1080.0f;
    if (Game::getInstance()) {
        screenWidth = static_cast<float>(Game::getInstance()->getWindowWidth());
        screenHeight = static_cast<float>(Game::getInstance()->getWindowHeight());
    }
    
    // 显示确认框
    confirmationWindow->autoSizeToContent();
    confirmationWindow->centerOnScreen(screenWidth, screenHeight);
    confirmationWindow->setVisible(true);
    isConfirmationVisible = true;
}

// 显示存储选择对话框（用于卸除子弹）
void GameUI::showStorageSelectionForUnloadAmmo(Magazine* magazine) {
    if (!magazine || !currentPlayer) {
        return;
    }
    
    // 获取所有存储空间
    auto availableStorages = currentPlayer->getAllAvailableStorages();
    
    if (availableStorages.empty()) {
        showConfirmationDialog("无存储空间", "未找到可用的存储空间。", "确定", "", nullptr);
        return;
    }
    
    // 创建存储选择确认框
    confirmationWindow->clearElements();
    
    // 标题
    UIElement title("选择卸除子弹到（全部）", 20.0f, 40.0f, {255, 255, 255, 255}, UIElementType::TITLE);
    confirmationWindow->addElement(title);
    
    // 弹匣信息
    std::string magazineInfo = "弹匣: " + magazine->getName() + " (" + 
                              std::to_string(magazine->getCurrentAmmoCount()) + "发)";
    UIElement magazineInfoElement(magazineInfo, 20.0f, 35.0f, {200, 200, 255, 255}, UIElementType::TEXT);
    confirmationWindow->addElement(magazineInfoElement);
    
    // 分隔线
    UIElement separator("─────────────────────", 20.0f, 25.0f, {150, 150, 150, 255}, UIElementType::TEXT);
    confirmationWindow->addElement(separator);
    
    // 添加每个存储空间的选项
    for (const auto& storagePair : availableStorages) {
        Storage* storage = storagePair.second;
        std::string storageText = storage->getName();
        
        // 显示存储空间的可用容量信息
        float availableWeight = storage->getMaxWeight() - storage->getCurrentWeight();
        float availableVolume = storage->getMaxVolume() - storage->getCurrentVolume();
        storageText += " (重量: " + formatFloat(availableWeight) + "/" + formatFloat(storage->getMaxWeight()) + 
                      " 体积: " + formatFloat(availableVolume) + "/" + formatFloat(storage->getMaxVolume()) + ")";
        
        // 将选择信息编码到dataPtr中
        std::string* actionData = new std::string("unload_single_ammo:" + std::to_string(reinterpret_cast<uintptr_t>(magazine)) + 
                                                 ":" + std::to_string(reinterpret_cast<uintptr_t>(storage)));
        
        UIElement storageOption(storageText, 25.0f, 35.0f, {255, 255, 255, 255}, UIElementType::TEXT, actionData);
        confirmationWindow->addElement(storageOption);
    }
    
    // 取消按钮
    UIElement cancelButton("取消", 20.0f, 40.0f, {255, 100, 100, 255}, UIElementType::TEXT, (void*)"cancel_storage_selection");
    confirmationWindow->addElement(cancelButton);
    
    // 暂停游戏时间
    if (Game::getInstance()) {
        originalTimeScaleBeforeConfirmation = Game::getInstance()->getTimeScale();
        Game::getInstance()->setTimeScale(0.0f);
    }
    
    // 获取实际屏幕尺寸
    float screenWidth = 1920.0f;
    float screenHeight = 1080.0f;
    if (Game::getInstance()) {
        screenWidth = static_cast<float>(Game::getInstance()->getWindowWidth());
        screenHeight = static_cast<float>(Game::getInstance()->getWindowHeight());
    }
    
    // 显示确认框
    confirmationWindow->autoSizeToContent();
    confirmationWindow->centerOnScreen(screenWidth, screenHeight);
    confirmationWindow->setVisible(true);
    isConfirmationVisible = true;
}

// 处理右键点击事件
bool GameUI::handleRightClick(int mouseX, int mouseY, Player* player, float windowWidth, float windowHeight) {
    if (!isUIVisible || !player) {
        return false;
    }
    
    // 首先隐藏任何现有的右键菜单
    hideRightClickMenu();
    
    // 更新当前玩家引用
    currentPlayer = player;
    
    // 检查是否点击了物品
    Item* clickedItem = nullptr;
    Storage* itemStorage = nullptr;
    
    // 遍历所有窗口查找被点击的物品
    UIWindow* currentWindow = getCurrentTabWindow();
    if (currentWindow && currentWindow->getVisible()) {
        int elementIndex = currentWindow->getElementAtPosition(mouseX, mouseY);
        if (elementIndex >= 0) {
            const auto& elements = currentWindow->getElements();
            if (elementIndex < static_cast<int>(elements.size())) {
                const UIElement& element = elements[elementIndex];
                void* dataPtr = element.getDataPtr();
                
                if (dataPtr) {
                    // 尝试将dataPtr转换为Item*
                    clickedItem = static_cast<Item*>(dataPtr);
                    
                    // 查找该物品所在的存储空间
                    itemStorage = findStorageByCoordinates(mouseX, mouseY);
                    
                    if (clickedItem && itemStorage) {
                        // 显示右键菜单
                        showRightClickMenu(mouseX, mouseY, clickedItem, itemStorage);
                        return true;
                    }
                }
            }
        }
    }
    
    return false;
}

// 处理弹药Action确认框的点击事件
void GameUI::handleAmmoActionConfirmationClick(const std::string& actionData) {
    std::cout << "进入handleAmmoActionConfirmationClick，actionData: " << actionData << std::endl;
    
    if (!currentPlayer) {
        std::cout << "错误：currentPlayer为空" << std::endl;
        return;
    }
    
    std::cout << "currentPlayer可用，开始解析actionData" << std::endl;
    
    // 解析actionData字符串
    std::string action;
    Magazine* magazine = nullptr;
    Ammo* ammo = nullptr;
    Storage* storage = nullptr;
    
    // 分割字符串：action:magazine_ptr:ammo_ptr:storage_ptr 或 action:magazine_ptr:storage_ptr
    size_t pos1 = actionData.find(':');
    if (pos1 == std::string::npos) return;
    
    action = actionData.substr(0, pos1);
    
    size_t pos2 = actionData.find(':', pos1 + 1);
    if (pos2 == std::string::npos) return;
    
    // 解析magazine指针
    uintptr_t magazinePtr = std::stoull(actionData.substr(pos1 + 1, pos2 - pos1 - 1));
    magazine = reinterpret_cast<Magazine*>(magazinePtr);
    
    if (action == "load_single_ammo") {
        // 装填子弹：load_single_ammo:magazine_ptr:ammo_ptr:storage_ptr
        size_t pos3 = actionData.find(':', pos2 + 1);
        if (pos3 == std::string::npos) {
            std::cout << "错误：无法找到第3个冒号" << std::endl;
            return;
        }
        
        // 解析ammo和storage指针
        uintptr_t ammoPtr = std::stoull(actionData.substr(pos2 + 1, pos3 - pos2 - 1));
        uintptr_t storagePtr = std::stoull(actionData.substr(pos3 + 1)); // 从pos3+1到字符串末尾
        
        ammo = reinterpret_cast<Ammo*>(ammoPtr);
        storage = reinterpret_cast<Storage*>(storagePtr);
        
        if (magazine && ammo && storage) {
            std::cout << "解析成功 - Magazine: " << magazine->getName() << ", Ammo: " << ammo->getName() << ", Storage: " << storage->getName() << std::endl;
            
            // 计算要装填的子弹数量：子弹组数量与剩余容量取最小值
            int ammoCount = ammo->isStackable() ? ammo->getStackSize() : 1;
            int remainingCapacity = magazine->getCapacity() - magazine->getCurrentAmmoCount();
            int loadCount = std::min(ammoCount, remainingCapacity);
            
            std::cout << "计算结果 - 子弹数量: " << ammoCount << ", 剩余容量: " << remainingCapacity << ", 装填数量: " << loadCount << std::endl;
            
            // 批量添加LoadSingleAmmoAction到队列
            for (int i = 0; i < loadCount; ++i) {
                auto loadAction = std::make_unique<LoadSingleAmmoAction>(
                    currentPlayer, magazine, ammo, storage,
                    [this, i, loadCount](bool success) {
                        if (success) {
                            std::cout << "第 " << (i + 1) << " 发子弹装填成功" << std::endl;
                        } else {
                            std::cout << "第 " << (i + 1) << " 发子弹装填失败" << std::endl;
                        }
                        // 在最后一发完成时更新UI
                        if (i == loadCount - 1) {
                            this->updatePlayerUI();
                        }
                    }
                );
                
                currentPlayer->getActionQueue()->addAction(std::move(loadAction));
            }
            
            std::cout << "已添加 " << loadCount << " 个装填Action到队列" << std::endl;
        }
    }
    else if (action == "unload_single_ammo") {
        // 卸除子弹：unload_single_ammo:magazine_ptr:storage_ptr
        uintptr_t storagePtr = std::stoull(actionData.substr(pos2 + 1));
        storage = reinterpret_cast<Storage*>(storagePtr);
        
        if (magazine && storage) {
            // 计算要卸除的子弹数量：弹匣内子弹数量
            int unloadCount = magazine->getCurrentAmmoCount();
            
            std::cout << "准备卸除 " << unloadCount << " 发子弹从弹匣" << std::endl;
            
            // 批量添加UnloadSingleAmmoAction到队列
            for (int i = 0; i < unloadCount; ++i) {
                auto unloadAction = std::make_unique<UnloadSingleAmmoAction>(
                    currentPlayer, magazine, storage,
                    [this, i, unloadCount](std::unique_ptr<Ammo> unloadedAmmo) {
                        std::cout << "第 " << (i + 1) << " 发子弹卸除成功" << std::endl;
                        // 在最后一发完成时更新UI
                        if (i == unloadCount - 1) {
                            this->updatePlayerUI();
                        }
                    }
                );
                
                currentPlayer->getActionQueue()->addAction(std::move(unloadAction));
            }
            
            std::cout << "已添加 " << unloadCount << " 个卸除Action到队列" << std::endl;
        }
    }
}