#include "GameUI.h"
#include "Game.h"
#include "Player.h"
#include "Item.h"
#include "Gun.h"
#include "GunMod.h"
#include "storage.h"
#include <iostream>
#include <sstream>
#include <set>
#include <iomanip>
#include "EquipmentSystem.h"

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
    
    // 添加标签
    std::vector<std::string> tags;
    
    // 检查物品类别
    if (item->hasFlag(ItemFlag::WEAPON)) tags.push_back("武器");
    if (item->hasFlag(ItemFlag::ARMOR)) tags.push_back("护甲");
    if (item->hasFlag(ItemFlag::AMMO)) tags.push_back("弹药");
    if (item->hasFlag(ItemFlag::MAGAZINE)) tags.push_back("弹匣");
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
      playerWindow(nullptr), hoveredItem(nullptr), mouseX(0), mouseY(0),
      itemTooltipWindow(nullptr), currentPlayer(nullptr),
      isDragging(false), draggedItem(nullptr), sourceStorage(nullptr),
      dragStartX(0), dragStartY(0), handSlotRectValid(false),
  
      pendingHeldItemToReplace(nullptr), pendingNewItemToHold(nullptr), pendingNewItemSource(nullptr),
      confirmationWindow(nullptr), isConfirmationVisible(false), 
      confirmationCallback(nullptr), originalTimeScaleBeforeConfirmation(1.0f) {
    // 构造函数中不再创建itemTooltipWindow，移到initFonts方法中
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
    
    // 创建玩家UI窗口
    playerWindow = std::make_unique<UIWindow>(30.0f, 30.0f, 0.0f, 0.0f, SDL_Color{100, 100, 255, 255}, 180);
    playerWindow->setVisible(false);
    playerWindow->setElementClickCallback([this](const UIElement& element) {
        this->onElementClick(element);
    });
    // 设置玩家UI窗口的字体
    playerWindow->setFonts(titleFont, subtitleFont, itemFont);
    
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
    

    
    // 创建确认对话框窗口 - 启用自动调整大小
    confirmationWindow = std::make_unique<UIWindow>(0.0f, 0.0f, 500.0f, 250.0f, 
                                                   SDL_Color{200, 200, 200, 255}, // 浅灰色边框
                                                   220); // 半透明度
    confirmationWindow->setVisible(false);
    confirmationWindow->setElementClickCallback([this](const UIElement& element) {
        this->handleConfirmationClick(element);
    });
    confirmationWindow->setFonts(titleFont, subtitleFont, itemFont);
    
    // 配置确认框的自动布局参数
    confirmationWindow->setAutoResize(true);
    confirmationWindow->setPadding(25.0f); // 内边距
    // maxContentWidth 将在显示时动态设置为屏幕宽度的1/4
    
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
        playerWindow->setVisible(true);
        
        // 如果有当前玩家引用，更新UI
        if (currentPlayer) {
            updatePlayerUI(currentPlayer);
        }
    }
}

void GameUI::closePlayerUI(Game* game) {
    if (isUIVisible && game) {
        // 恢复游戏倍率
        game->setTimeScale(originalTimeScale);
        
        // 隐藏界面
        isUIVisible = false;
        playerWindow->setVisible(false);
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
}

void GameUI::updatePlayerUI(Player* player) {
    if (!player || !playerWindow) return;
    
    // 更新当前玩家引用
    currentPlayer = player;
    
    playerWindow->clearElements();
    
    // 添加标题
    UIElement title("玩家背包", 20.0f, 60.0f, {255, 255, 255, 255}, UIElementType::TITLE);
    playerWindow->addElement(title);
    
    // 添加手持位显示
    UIElement handSlotTitle("手持物品", 20.0f, 45.0f, {255, 215, 0, 255}, UIElementType::SUBTITLE);
    playerWindow->addElement(handSlotTitle);
    
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
        playerWindow->addElement(heldItemElement);
    } else {
        // 如果没有手持物品，显示空
        UIElement emptyHandElement("<空>", 40.0f, 32.0f, {150, 150, 150, 255}, UIElementType::TEXT);
        playerWindow->addElement(emptyHandElement);
    }
    
    // 添加额外间距
    UIElement spacer1("", 0.0f, 20.0f, {255, 255, 255, 255}, UIElementType::TEXT);
    playerWindow->addElement(spacer1);
    
    // 添加装备信息
    UIElement equipTitle("已装备物品:", 20.0f, 45.0f, {200, 200, 255, 255}, UIElementType::SUBTITLE);
    playerWindow->addElement(equipTitle);
    
    // 获取装备系统
    EquipmentSystem* equipSystem = player->getEquipmentSystem();
    if (equipSystem) {
        // 用于跟踪已显示的物品，避免重复显示同一个物品（当一个物品覆盖多个槽位时）
        std::set<Item*> displayedItems;
        
        // 定义所有需要显示的装备槽位
        std::vector<EquipSlot> allSlots = {
            EquipSlot::HEAD,
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
            playerWindow->addElement(slotElement);
            
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
                        playerWindow->addElement(itemElement);
                        
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
                        playerWindow->addElement(itemElement);
                        
                        // 更新下一个物品的X偏移量
                        itemXOffset += equippedItem->getName().length() * 14.0f + 20.0f; // 估算文本宽度并添加间距
                    }
                }
            } 
            else {
                // 如果该槽位没有装备物品，显示<空>
                UIElement emptyElement("<空>", itemXOffset, 0.0f, {150, 150, 150, 255});
                playerWindow->addElement(emptyElement);
            }
            
            // 在一行物品后添加Y轴偏移，为下一行做准备
            UIElement spacerElement("", 0.0f, 32.0f, {0, 0, 0, 0});
            playerWindow->addElement(spacerElement);
        }
    }
        
    
    
    // 添加额外间距
    UIElement spacer2("", 0.0f, 35.0f, {255, 255, 255, 255}, UIElementType::TEXT);
    playerWindow->addElement(spacer2);
    
    // 添加背包信息
    UIElement backpackTitle("背包物品:", 20.0f, 45.0f, {200, 255, 200, 255}, UIElementType::SUBTITLE);
    playerWindow->addElement(backpackTitle);
    
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
        float targetButtonX = playerWindow->getWidth() - 40.0f; // 目标位置
        float buttonX = targetButtonX / fontSizeRatio; // 补偿缩放效果
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "折叠按钮位置: 窗体宽度=%.1f, 目标X=%.1f, 补偿后X=%.1f, 比例=%.1f", 
                   playerWindow->getWidth(), targetButtonX, buttonX, fontSizeRatio);
        UIElement collapseButton(storage->getIsCollapsed() ? "+" : "-", buttonX, 0.0f, 
                                storage->getIsCollapsed() ? SDL_Color{255, 150, 150, 255} : SDL_Color{200, 200, 200, 255},
                                UIElementType::TEXT);
        collapseButton.setDataPtr(storage);
        playerWindow->addElement(collapseButton);
        
        UIElement storageInfoElement(storageInfo, 20.0f, 45.0f, {200, 200, 200, 255}, UIElementType::SUBTITLE);
        playerWindow->addElement(storageInfoElement);
        
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
                    playerWindow->addElement(itemElement);
                }
            }
        }
        
        // 在每个存储空间后添加一个空白的占位元素，增加存储空间之间的间距
        // 使用透明色（alpha=0）使其不可见，但仍占用空间
        UIElement storageSpacer("", 0.0f, 15.0f, {0, 0, 0, 0}, UIElementType::TEXT);
        playerWindow->addElement(storageSpacer);
    }

}

void GameUI::render(SDL_Renderer* renderer, float windowWidth, float windowHeight) {
    if (!renderer) return;
    
    // 渲染玩家UI窗口（如果可见）
    if (isUIVisible && playerWindow) {
        // 设置窗口尺寸（占据左半屏幕）
        playerWindow->setWidth(windowWidth / 2 - 45.0f);
        playerWindow->setHeight(windowHeight - 60.0f);
        playerWindow->render(renderer, windowWidth, windowHeight);
        
        // 调试模式：渲染元素边框
        // playerWindow->renderElementBorders(renderer);
        
        // 更新存储空间坐标映射
        updateStorageCoordinatesMap();
        
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
                float windowX = playerWindow->getX();
                float windowWidth = playerWindow->getWidth();
                
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
    
    // 渲染物品提示框（无论玩家UI是否可见，物品提示框都可以显示）
    renderItemTooltip(renderer, windowWidth, windowHeight);
    

    
    // 渲染确认对话框（如果可见，优先级最高）
    if (isConfirmationVisible && confirmationWindow) {
        confirmationWindow->renderWithWrapping(renderer, windowWidth, windowHeight);
    }
    
    // 如果正在拖拽物品，在鼠标位置绘制物品名称
    if (isDragging && draggedItem) {
        // 创建一个临时的文本表面
        SDL_Color textColor = {255, 255, 255, 200};
        SDL_Surface* textSurface = TTF_RenderText_Blended(itemFont, draggedItem->getName().c_str(), draggedItem->getName().length(), textColor);
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

    // 如果确认对话框可见，不更新悬停物品
    if (isConfirmationVisible) {
        hoveredItem = nullptr;
        if (itemTooltipWindow) {
            itemTooltipWindow->setVisible(false);
        }
        return;
    }

    // 重置悬停物品指针
    hoveredItem = nullptr;

    // 检查玩家背包UI
    if (isUIVisible && playerWindow) {
        int elementIndex = playerWindow->getElementAtPosition(mouseX, mouseY);
        if (elementIndex >= 0) {
            const auto& elements = playerWindow->getElements();
            if (elementIndex < static_cast<int>(elements.size())) {
                // 检查元素文本，如果是折叠/展开按钮（+或-），则不将其视为物品
                const std::string& text = elements[elementIndex].getText();
                if (text != "+" && text != "-") {
                    // 只有非折叠按钮的元素才可能是物品
                    hoveredItem = static_cast<Item*>(elements[elementIndex].getDataPtr());
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
    
    // 添加物品类别信息
    std::string categoryStr = "类别: ";
    const auto& flagNames = item->getFlagNames();
    for (size_t i = 0; i < flagNames.size(); ++i) {
        if (i > 0) categoryStr += ", ";
        categoryStr += flagNames[i];
    }
    details.push_back(categoryStr);
    
    // 添加物品基本属性
    details.push_back("重量: " + formatFloat(item->getWeight()) + " kg");
    details.push_back("体积: " + formatFloat(item->getVolume()));
    details.push_back("价值: " + std::to_string(item->getValue()));
    
    //// 添加物品描述
    //if (!item->getDescription().empty()) {
    //    details.push_back(""); // 空行
    //    details.push_back(item->getDescription());
    //}
    
    // 根据物品类型添加特定信息
    if (item->hasFlag(ItemFlag::WEAPON)) {
        details.push_back(""); // 空行
        details.push_back("武器属性:");
        
        // 如果是枪械，添加枪械特定属性
        if (item->hasFlag(ItemFlag::GUN)) {
            Gun* gun = dynamic_cast<Gun*>(item);
            if (gun) {
                details.push_back("伤害加成: " + std::to_string(gun->getDamageBonus()));
                details.push_back("射程加成: " + std::to_string(gun->getRangeBonus()) );
                details.push_back("精度: " + formatFloat(gun->getAccuracyMOA()) + " MOA");
                details.push_back("射速加成: " + std::to_string(gun->getFireRate()) + " RPM");
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
                for (int slot = static_cast<int>(AttachmentSlot::STOCK); 
                     slot <= static_cast<int>(AttachmentSlot::SPECIAL); ++slot) {
                    AttachmentSlot currentSlot = static_cast<AttachmentSlot>(slot);
                    auto slotAttachments = gun->getAllAttachments(currentSlot);
                    
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
        // 可以添加其他武器类型的特定属性
    }
    else if (item->hasFlag(ItemFlag::MAGAZINE)) {
        Magazine* mag = dynamic_cast<Magazine*>(item);
        details.push_back(""); // 空行
        details.push_back("弹匣属性:");
        if (mag) {
            details.push_back("容量: " + std::to_string(mag->getCapacity()));
            details.push_back("当前弹药: " + std::to_string(mag->getCurrentAmmoCount()));
            // 将vector<string>转换为格式化的字符串
            std::string ammoTypes;
            const auto& types = mag->getCompatibleAmmoTypes();
            for (size_t i = 0; i < types.size(); ++i) {
                ammoTypes += types[i];
                if (i < types.size() - 1) {
                    ammoTypes += ", ";
                }
            }
            details.push_back("兼容口径: " + ammoTypes);
        } else {
            details.push_back("警告: 物品标记为弹匣但无法转换为Magazine类型");
        }
    }
    else if (item->hasFlag(ItemFlag::AMMO)) {
        Ammo* ammo = dynamic_cast<Ammo*>(item);
        details.push_back(""); // 空行
        details.push_back("弹药属性:");
        if (ammo) {
            details.push_back("口径: " + ammo->getAmmoType());
            details.push_back("伤害修正: " + formatFloat(ammo->getBaseDamage() * 100) + "%");
            details.push_back("穿透修正: " + formatFloat(ammo->getBasePenetration() * 100) + "%");
        } else {
            details.push_back("警告: 物品标记为弹药但无法转换为Ammo类型");
        }
    }
    else if (item->hasFlag(ItemFlag::GUNMOD)) {
        GunMod* mod = dynamic_cast<GunMod*>(item);
        details.push_back(""); // 空行
        details.push_back("配件属性:");
        if (mod) {
            if (mod->getModDamageBonus() != 0) details.push_back("伤害修正: " + formatFloat(mod->getModDamageBonus()));
            if (mod->getModRangeBonus() != 0) details.push_back("射程修正: " + formatFloat(mod->getModRangeBonus()));
            if (mod->getModAccuracyMOA() != 0) details.push_back("精度修正: " + formatFloat(mod->getModAccuracyMOA()) + " MOA");
            if (mod->getModFireRate() != 0) details.push_back("射速修正: " + std::to_string(mod->getModFireRate()) + " RPM");
            if (mod->getModPenetrationBonus() != 0) details.push_back("穿透修正: " + formatFloat(mod->getModPenetrationBonus()));
            if (mod->getModRecoil() != 0) details.push_back("后坐力修正: " + formatFloat(mod->getModRecoil() * 100) + "%");
            if (mod->getModSoundLevel() != 0) details.push_back("消音效果: " + formatFloat(mod->getModSoundLevel() * 100) + "%");
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
    
    // 最优先检查确认对话框（模态对话框）
    if (isConfirmationVisible && confirmationWindow) {
        if (confirmationWindow->handleClick(mouseX, mouseY, windowWidth, windowHeight)) {
            return true; // 点击被确认对话框处理了
        }
        // 如果确认对话框可见但点击不在其范围内，阻止其他UI交互
        return true;
    }
    

    
    // 检查玩家背包UI
    if (isUIVisible && playerWindow) {
        // 检查是否点击了物品元素
        int elementIndex = playerWindow->getElementAtPosition(mouseX, mouseY);
        if (elementIndex >= 0) {
            const auto& elements = playerWindow->getElements();
            if (elementIndex < static_cast<int>(elements.size())) {
                // 检查元素文本，如果是折叠/展开按钮（+或-），则不将其视为物品
                const std::string& text = elements[elementIndex].getText();
                if (text != "+" && text != "-") {
                    // 只有非折叠按钮的元素才可能是物品
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
        
        return playerWindow->handleClick(mouseX, mouseY, windowWidth, windowHeight);
    }
    
    return false;
}

bool GameUI::handleStorageClick(int mouseX, int mouseY, Player* player, Storage* monsterBackpack, float windowWidth, float windowHeight) {
    // 更新当前玩家引用
    if (player) {
        currentPlayer = player;
    }
    
    // 检查玩家背包UI中的Storage折叠/展开按钮
    if (isUIVisible && playerWindow) {
        int elementIndex = playerWindow->getElementAtPosition(mouseX, mouseY);
        if (elementIndex >= 0) {
            const auto& elements = playerWindow->getElements();
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
    
    return false; // 点击未处理
}

Storage* GameUI::findStorageByCoordinates(int x, int y) {
    // 检查玩家UI是否可见
    if (!isUIVisible || !playerWindow || !currentPlayer) {
        return nullptr;
    }
    
    // 首先检查点击是否在窗口区域内
    if (x < playerWindow->getX() || x > playerWindow->getX() + playerWindow->getWidth() ||
        y < playerWindow->getY() || y > playerWindow->getY() + playerWindow->getHeight()) {
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
        int elementIndex = playerWindow->getElementAtPosition(x, y);
        if (elementIndex < 0) {
            return nullptr; // 没有找到元素
        }
        
        // 获取元素列表
        const auto& elements = playerWindow->getElements();
        if (elementIndex >= static_cast<int>(elements.size())) {
            return nullptr; // 索引超出范围
        }
        
        // 获取元素的数据指针
        void* dataPtr = elements[elementIndex].getDataPtr();
        if (!dataPtr) {
            return nullptr; // 元素没有关联数据
        }
        
        // 首先检查数据指针是否直接指向Storage
        // 这种情况通常出现在折叠/展开按钮上
        if (elements[elementIndex].getText() == "+" || elements[elementIndex].getText() == "-") {
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
    
    return isDragging; // 如果正在拖拽，返回true
}

void GameUI::updateStorageCoordinatesMap() {
    // 清空现有映射
    storageCoordinatesMap.clear();
    
    // 检查玩家窗口是否存在
    if (!playerWindow || !currentPlayer) {
        return;
    }
    
    // 获取所有UI元素
    const auto& elements = playerWindow->getElements();
    
    // 当前处理的存储空间
    Storage* currentStorage = nullptr;
    float storageStartY = 0.0f;
    float storageEndY = 0.0f;
    
    // 遍历所有元素
    for (size_t i = 0; i < elements.size(); ++i) {
        const auto& element = elements[i];
        
        // 获取元素的渲染区域
        ElementRenderRect rect;
        if (!playerWindow->getElementRect(i, rect)) {
            continue;
        }
        
        // 检查元素是否为存储空间信息（通过检查文本内容包含"件物品"来判断）
        if (element.getText().find("件物品") != std::string::npos) {
            // 如果之前有处理中的存储空间，完成其坐标范围计算
            if (currentStorage) {
                StorageCoordinates coords;
                coords.topLeftX = playerWindow->getX();
                coords.topLeftY = storageStartY;
                coords.bottomRightX = playerWindow->getX() + playerWindow->getWidth();
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
        coords.topLeftX = playerWindow->getX();
        coords.topLeftY = storageStartY;
        coords.bottomRightX = playerWindow->getX() + playerWindow->getWidth();
        coords.bottomRightY = playerWindow->getY() + playerWindow->getHeight(); // 窗口底部作为最后一个存储空间的底部
        coords.storage = currentStorage;
        
        storageCoordinatesMap.push_back(coords);
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
    if (handSlotRectValid && playerWindow) {
        float windowX = playerWindow->getX();
        float windowWidth = playerWindow->getWidth();
        
        // 使用与橙色框相同的检测范围：铺满整个窗口宽度
        float detectStartX = windowX + 10.0f;
        float detectEndX = windowX + windowWidth - 10.0f;
        
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "手持位检测: 鼠标(%d,%d), 手持区域(%.1f-%.1f,%.1f,%.1f)", 
                   mouseX, mouseY, 
                   detectStartX, detectEndX, handSlotRect.y, handSlotRect.height);
        
        if (mouseX >= detectStartX && mouseX <= detectEndX &&
            mouseY >= handSlotRect.y && mouseY <= handSlotRect.y + handSlotRect.height) {
            droppedOnHeldItemSlot = true;
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "检测到拖拽到手持位置（铺满窗口宽度）");
        }
    }
        
        // TODO: 装备槽位检测需要实现动态计算，暂时注释掉硬编码的检测逻辑
        // 检查是否拖拽到装备位置，并确定具体的装备槽位
        // （装备槽位的动态计算比手持位更复杂，将在后续版本实现）
    
    // 如果拖拽到装备位置
    if (droppedOnEquipmentSlot && !isEquippedItem && !isHeldItem && sourceStorage) {
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
    
    // 普通确认框处理
    void* dataPtr = element.getDataPtr();
    
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
    
    // 检查玩家窗口是否存在
    if (!playerWindow || !currentPlayer) {
        return;
    }
    
    // 获取所有UI元素
    const auto& elements = playerWindow->getElements();
    
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
            if (playerWindow->getElementRect(handSlotElementIndex, handSlotRect)) {
                handSlotRectValid = true;
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "手持位区域更新: (%.1f,%.1f,%.1f,%.1f), 内容='%s'", 
                           handSlotRect.x, handSlotRect.y, handSlotRect.width, handSlotRect.height, text.c_str());
            }
                 } else {
             SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "索引2的element不是手持位element: '%s'", text.c_str());
         }
     } else {
         SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "UI元素数量不足，无法找到手持位element");
     }
}