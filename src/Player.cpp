#include "Player.h"
#include "Game.h" // 可能需要用于获取渲染器等
#include <iostream> // 用于测试输出和错误输出
#include <SDL3/SDL_scancode.h>
#include <cmath>
#include <algorithm>
#include "Gun.h" // 确保包含了 Gun.h
#include "SoundManager.h" // <--- 添加这一行
#include "Action.h" // 包含Action.h以使用HoldItemAction
#include "EntityStateEffect.h" // 包含EntityStateEffect.h
#include "storage.h" // 包含EntityStateEffect.h
#include "AttackSystem.h" // 包含攻击系统头文件
#include "MeleeWeapon.h" // 包含通用近战武器头文件
#include "Damage.h" // 包含伤害系统头文件
#include "Constants.h" // 包含常量定义
#include <random> // 包含随机数生成器


// 在构造函数中初始化新增变量
Player::Player(float startX, float startY)
    : Creature(startX, startY, 20, 320, 100, { 255, 0, 0, 255 }, CreatureType::HUMANOID, "Player", Faction::PLAYER),  // 5格/秒 = 320像素/秒
      isMouseLeftDown(false), playerTexture(nullptr), textureInitialized(false),
      playerId(-1), playerName("Player"), isLocalPlayer(true), controller(nullptr),
      STR(10), AGI(12), INT(16), PER(14),  // 初始化四个基础属性
      headHealth(MAX_HEAD_HEALTH), torsoHealth(MAX_TORSO_HEALTH), 
      leftLegHealth(MAX_LEG_HEALTH), rightLegHealth(MAX_LEG_HEALTH),
      leftArmHealth(MAX_ARM_HEALTH), rightArmHealth(MAX_ARM_HEALTH),  // 初始化身体部位血量
      dodgeCount(0), lastDodgeResetTime(0) {  // 初始化闪避系统
    
    // 设置物理引擎属性
    setPhysicalAttributes(70.0f, 12, 10);  // 重量70kg，力量12，敏捷10（人类平衡型）
    isStatic = false;      // 玩家是动态物体
    
    // 设置玩家的感知范围（以格为单位，使用常量）
    setVisualRange(80);  // 80格视觉范围
    setHearingRange(10); // 10格听觉范围
    setSmellRange(5);    // 5格嗅觉范围
    
    //// 设置玩家的行为参数
    //setAggressionLevel(0.3f);      // 较低的攻击性
    //setFearLevel(0.4f);            // 适中的恐惧
    //setIntelligenceLevel(0.9f);    // 高智能
    //setPackInstinct(0.7f);         // 较强的群体协作
    
    // 初始化状态管理器
    stateManager = std::make_unique<PlayerStateManager>(this);
    
    // 初始化攻击系统
    attackSystem = std::make_unique<AttackSystem>(this);
    
    // 初始化技能系统
    skillSystem = std::make_unique<SkillSystem>();
    
    // 设置攻击系统的寻找目标函数
    attackSystem->setFindTargetFunction([this](float range) -> Entity* {
        return this->findAttackTarget(range);
    });
    
    // 设置攻击完成回调
    attackSystem->setOnAttackComplete([this](const AttackResult& result) {
        if (result.hit) {
            std::cout << "玩家攻击命中！造成 " << result.totalDamage << " 点伤害";
            if (result.critical) {
                std::cout << " (暴击!)";
            }
            
            // 显示连击信息
            if (heldItem && heldItem->hasFlag(ItemFlag::MELEE)) {
                MeleeWeapon* meleeWeapon = dynamic_cast<MeleeWeapon*>(heldItem.get());
                if (meleeWeapon && meleeWeapon->getComboCount() > 0) {
                    std::cout << " [" << meleeWeapon->getComboCount() << "连击!]";
                }
            }
            
            if (result.causedBleeding) {
                std::cout << " (流血!)";
            }
            if (result.causedStun) {
                std::cout << " (眩晕!)";
            }
            std::cout << std::endl;
            
            // 获得技能经验
            if (heldItem) {
                if (heldItem->hasFlag(ItemFlag::GUN)) {
                    // 枪械攻击获得经验
                    gainWeaponExperience("GUN", result.critical ? 2 : 1);
                } else if (heldItem->hasFlag(ItemFlag::MELEE)) {
                    // 近战攻击获得经验
                    if (heldItem->hasFlag(ItemFlag::SWORD)) {
                        gainMeleeExperience("SWORD", result.critical ? 2 : 1);
                    } else if (heldItem->hasFlag(ItemFlag::DAGGER)) {
                        gainMeleeExperience("DAGGER", result.critical ? 2 : 1);
                    } else if (heldItem->hasFlag(ItemFlag::HAMMER)) {
                        gainMeleeExperience("HAMMER", result.critical ? 2 : 1);
                    } else {
                        gainMeleeExperience("MELEE", result.critical ? 2 : 1);
                    }
                }
            } else {
                // 徒手攻击
                gainMeleeExperience("UNARMED", result.critical ? 2 : 1);
            }
        }
    });
    
    // 设置状态回调
    if (stateManager) {
        stateManager->setOnStateAdded([this](EntityStateEffect* state) {
            // 当状态添加时，根据状态类型执行特定逻辑
            if (state->getType() == EntityStateEffect::Type::SHOOTING) {
                // 射击状态添加时的逻辑
                std::cout << "Player entered shooting state" << std::endl;
            } else if (state->getType() == EntityStateEffect::Type::MOVING) {
                // 移动状态添加时的逻辑
                std::cout << "Player entered moving state" << std::endl;
            }
        });
        
        stateManager->setOnStateRemoved([this](EntityStateEffect* state) {
            // 当状态移除时，根据状态类型执行特定逻辑
            if (state->getType() == EntityStateEffect::Type::SHOOTING) {
                // 射击状态结束时的逻辑
                std::cout << "Player exited shooting state" << std::endl;
            } else if (state->getType() == EntityStateEffect::Type::MOVING) {
                // 移动状态结束时的逻辑
                std::cout << "Player exited moving state" << std::endl;
            }
        });
    }
}

// 析构函数实现
Player::~Player() {
    // 清理纹理资源
    if (playerTexture) {
        SDL_DestroyTexture(playerTexture);
        playerTexture = nullptr;
    }
}

// 修改handleMouseClick方法
// 在 handleMouseClick 方法中
void Player::handleMouseClick(int button) {
    // 如果是左键点击，标记为按下状态
    if (button == SDL_BUTTON_LEFT) {
        isMouseLeftDown = true;
        
        // 检查手持武器类型，选择攻击方式
        if (heldItem && heldItem->hasFlag(ItemFlag::MELEE)) {
            // 近战武器：左键攻击（扇形）
            attemptAttack(WeaponAttackType::PRIMARY);
        } else if (heldItem && heldItem->hasFlag(ItemFlag::GUN)) {
            // 枪械：射击
            addPlayerState(EntityStateEffect::Type::SHOOTING, "single_shot", 100); // 100毫秒的射击状态
            attemptShoot();
        }
    } else if (button == SDL_BUTTON_RIGHT) {
        // 右键点击
        if (heldItem && heldItem->hasFlag(ItemFlag::MELEE)) {
            // 近战武器：右键攻击（长条形）
            attemptAttack(WeaponAttackType::SECONDARY);
        }
    }
}

// 在 update 方法中
void Player::update(float deltaTime) {
    // 调用基类的更新方法，包括状态计时器更新
    Creature::update(deltaTime);
    
    // 更新状态管理器
    if (stateManager) {
        stateManager->update(deltaTime);
    }

    // 更新攻击系统
    if (attackSystem) {
        attackSystem->updateCooldown(static_cast<int>(deltaTime * 1000));
    }
    
    // 更新闪避次数
    updateDodgeCount();
    
    // 更新武器冷却时间
    if (heldItem && heldItem->hasFlag(ItemFlag::MELEE)) {
        // 如果是近战武器，更新其冷却时间
        IWeaponAttack* weaponAttack = dynamic_cast<IWeaponAttack*>(heldItem.get());
        if (weaponAttack) {
            MeleeWeapon* meleeWeapon = dynamic_cast<MeleeWeapon*>(heldItem.get());
            if (meleeWeapon) {
                meleeWeapon->updateCooldown(static_cast<int>(deltaTime * 1000));
            }
        }
    }
    
    // 只有本地玩家才处理鼠标持续攻击逻辑
    if (isLocalPlayer && isMouseLeftDown && canPerformAction()) {
        if (heldItem && heldItem->hasFlag(ItemFlag::GUN) && canShootByCooldown()) {
            // 枪械的持续射击
            if (!hasPlayerState(EntityStateEffect::Type::SHOOTING)) {
                // 添加持续射击状态（持续到松开鼠标）
                addPlayerState(EntityStateEffect::Type::SHOOTING, "continuous_shot", -1); // -1表示持续状态
            }
            attemptShoot();
        } else if (heldItem && heldItem->hasFlag(ItemFlag::MELEE)) {
            // 近战武器的连续攻击（左键持续按下时）
            if (canAttack()) {
                attemptAttack(WeaponAttackType::PRIMARY);
            }
        }
    }
}

// 新增attemptReload方法（替代原reloadCurrentWeapon方法）
void Player::attemptReload() {
    Gun* currentGun = nullptr;
    if (heldItem && heldItem->hasFlag(ItemFlag::GUN)) {
        currentGun = static_cast<Gun*>(heldItem.get());
    }

    if (currentGun) {
        std::cout << "Player attempting to reload " << currentGun->getName() << std::endl;
        
        // 检查当前弹匣状态
        if (currentGun->getCurrentMagazine()) {
            std::cout << "Current magazine: " << currentGun->getCurrentMagazine()->getName() 
                      << " with " << currentGun->getCurrentMagazine()->getCurrentAmmoCount() << " rounds" << std::endl;
        } else {
            std::cout << "No current magazine in gun" << std::endl;
        }
        
        // 检查是否有兼容的备用弹匣
        auto compatibleTypes = currentGun->getEffectiveAmmoTypes();
        std::cout << "Gun compatible ammo types: ";
        for (const auto& type : compatibleTypes) {
            std::cout << type << " ";
        }
        std::cout << std::endl;
        
        // 手动查找弹匣
        auto magazineItems = findItemsByCategory(ItemFlag::MAGAZINE);
        std::cout << "Found " << magazineItems.size() << " magazines in storage" << std::endl;
        
        for (const auto& [slot, storage, index, item] : magazineItems) {
            Magazine* mag = static_cast<Magazine*>(item);
            std::cout << "  Magazine: " << mag->getName() 
                      << " with " << mag->getCurrentAmmoCount() << " rounds" 
                      << " (compatible types: ";
            for (const auto& ammoType : mag->getCompatibleAmmoTypes()) {
                std::cout << ammoType << " ";
            }
            std::cout << ")" << std::endl;
        }
        
        // 添加换弹状态
        addPlayerState(EntityStateEffect::Type::RELOADING, "reloading", static_cast<int>(currentGun->getReloadTime() * 1000));
        
        // 使用Entity的方法进行换弹
        float reloadTime = reloadWeaponAuto(currentGun);
        std::cout << "Reload initiated, expected time: " << reloadTime << " seconds" << std::endl;
    } else {
        std::cout << "Player cannot reload - no gun held or wrong item type" << std::endl;
    }
}

// 添加handleMouseRelease方法
void Player::handleMouseRelease(int button) {
    // 如果是左键释放，取消按下状态
    if (button == SDL_BUTTON_LEFT) {
        isMouseLeftDown = false;
        
        // 移除持续射击状态
        removePlayerState("continuous_shot");
    }
}

void Player::equipItem(std::unique_ptr<Item> item) {
    if (!item) {
        return;
    }

    // 如果是可穿戴物品，使用Action系统进行装备
    if (item->isWearable()) {
        // 如果是可穿戴物品（如背包、护甲等），使用Action系统进行装备
        // 这样可以确保有正确的装备动画和时间
        std::string itemName = item->getName(); // 保存物品名称用于日志
        equipItemWithAction(std::move(item));
        std::cout << "Player started equipping wearable item: " << itemName << std::endl;
    } else {
        // 对于其他所有物品（武器、工具、弹药等），都设置为手持物品（右手）
        heldItem = std::move(item);
        if (heldItem) {
            std::cout << "Player equipped in right hand: " << heldItem->getName() << std::endl;
        }
    }
}

Item* Player::getHeldItem() const {
    return heldItem.get();
}

void Player::useHeldItem() {
    if (heldItem) {
        std::cout << "Player uses: " << heldItem->getName() << std::endl;
        heldItem->use(); // 调用物品的 use 方法
        // 根据物品类型，可能在使用后销毁，例如消耗品
        if (heldItem->hasFlag(ItemFlag::CONSUMABLE)) {
            heldItem.reset(); // 销毁消耗品
        }
    }
}

// 从存储空间中取出物品并手持
bool Player::holdItemFromStorage(Item* item, Storage* storage) {
    if (!item || !storage) {
        return false;
    }
    
    // 检查物品是否在存储空间中
    bool itemFound = false;
    for (size_t i = 0; i < storage->getItemCount(); ++i) {
        if (storage->getItem(i) == item) {
            itemFound = true;
            break;
        }
    }
    
    if (!itemFound) {
        return false;
    }
    
    // 创建手持物品行为，并设置回调函数
    auto holdAction = std::make_unique<HoldItemAction>(this, item, storage, 
        [this](std::unique_ptr<Item> item) {
            // 回调函数：将物品设置为手持物品
            this->equipItem(std::move(item));
        });
    
    // 将行为添加到行为队列
    actionQueue->addAction(std::move(holdAction));
    
    return true;
}

bool Player::initializeTexture(SDL_Renderer* renderer) {
    if (textureInitialized && playerTexture) {
        return true; // 已经初始化过了
    }
    
    // 加载player.bmp文件
    SDL_Surface* surface = SDL_LoadBMP("assets/tiles/player.bmp");
    if (!surface) {
        std::cerr << "无法加载玩家贴图: assets/tiles/player.bmp - " << SDL_GetError() << std::endl;
        return false;
    }
    
    // 设置透明色键（假设使用白色作为透明色）
    SDL_SetSurfaceColorKey(surface, true, SDL_MapSurfaceRGB(surface, 255, 255, 255));
    
    // 创建纹理
    playerTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    
    if (!playerTexture) {
        std::cerr << "无法创建玩家纹理: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // 设置纹理混合模式以支持透明
    SDL_SetTextureBlendMode(playerTexture, SDL_BLENDMODE_BLEND);
    
    textureInitialized = true;
    return true;
}

void Player::render(SDL_Renderer* renderer, float cameraX, float cameraY) {
    // 初始化纹理（如果还没有初始化）
    if (!textureInitialized) {
        initializeTexture(renderer);
    }
    
    // 如果有纹理，使用纹理渲染；否则使用基类的矩形渲染
    if (playerTexture) {
        SDL_FRect playerRect = {
            static_cast<float>(x - radius - cameraX),
            static_cast<float>(y - radius - cameraY),
            static_cast<float>(radius * 2),
            static_cast<float>(radius * 2)
        };
        SDL_RenderTexture(renderer, playerTexture, nullptr, &playerRect);
    } else {
        // 如果纹理加载失败，回退到基类的矩形渲染
        SDL_FRect entityRect = {
            static_cast<float>(x - radius - cameraX),
            static_cast<float>(y - radius - cameraY),
            static_cast<float>(radius * 2),
            static_cast<float>(radius * 2)
        };
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, &entityRect);
    }
    

    //// 渲染手持物品 (如果存在且有渲染逻辑)
    //if (heldItem) {
    //    // 假设物品渲染在玩家旁边或手上
    //    // 这里只是一个示例，具体渲染位置和方式需要根据游戏设计调整
    //    int itemRenderX = x - cameraX + radius + 5; // 玩家右侧
    //    int itemRenderY = y - cameraY;
    //    heldItem->render(renderer, itemRenderX, itemRenderY);
    //}
}

// 在Player.cpp中修改handleInput方法，考虑状态对移动的影响

void Player::handleInput(const bool* keyState, float deltaTime) {
    // 只有本地玩家才处理键盘输入
    if (!isLocalPlayer) {
        return;
    }
    
    // 计算移动方向
    float dx = 0.0f, dy = 0.0f;  // 改为float类型
    
    // 检查WASD键
    if (keyState[SDL_SCANCODE_W]) dy -= 1.0f;
    if (keyState[SDL_SCANCODE_S]) dy += 1.0f;
    if (keyState[SDL_SCANCODE_A]) dx -= 1.0f;
    if (keyState[SDL_SCANCODE_D]) dx += 1.0f;
    
    // 如果有移动输入
    if (dx != 0.0f || dy != 0.0f) {
        // 如果没有移动状态，添加一个
        if (!hasPlayerState(EntityStateEffect::Type::MOVING)) {
            addPlayerState(EntityStateEffect::Type::MOVING, "moving", -1); // -1表示持续状态
        }
        
        // 归一化方向向量（对角线移动不应该更快）
        float length = std::sqrt(dx * dx + dy * dy);
        if (length > 0) {
            dx /= length;
            dy /= length;
        }
        
        // 应用速度修正系数
        float adjustedSpeed = speed * getSpeedModifier();
        
        // 计算期望速度并设置给物理系统
        float desiredVelX = dx * adjustedSpeed;
        float desiredVelY = dy * adjustedSpeed;
        setDesiredVelocity(desiredVelX, desiredVelY);
    } else {
        // 如果没有移动输入，停止移动
        setDesiredVelocity(0.0f, 0.0f);
        removePlayerState(EntityStateEffect::Type::MOVING);
    }
    
    // 检查R键（换弹）- 使用静态变量防止重复触发
    static bool prevRKeyState = false;
    bool currentRKeyState = keyState[SDL_SCANCODE_R];
    if (currentRKeyState && !prevRKeyState) {
        std::cout << "R key pressed - attempting reload with action queue" << std::endl;
        reloadCurrentWeapon(); // 使用原有的换弹方法，它会使用Action队列
    }
    prevRKeyState = currentRKeyState;
    
    // 检查F键（交互）
    if (keyState[SDL_SCANCODE_F]) {
        // 添加交互状态
        if (!hasPlayerState(EntityStateEffect::Type::INTERACTING)) {
            addPlayerState(EntityStateEffect::Type::INTERACTING, "interacting", 500); // 500毫秒的交互状态
            
            // 尝试与附近的实体交互
            Entity* interactingEntity = getInteractingEntity();
            if (interactingEntity) {
                // 处理交互逻辑
                std::cout << "Player is interacting with an entity" << std::endl;
            }
        }
    }
}


void Player::handleMouseMotion(int mouseX, int mouseY, float cameraX, float cameraY) {
    // 获取当前缩放级别
    float zoomLevel = Game::getInstance()->getZoomLevel();
    
    // 根据缩放调整鼠标坐标
    mouseScreenX = mouseX;
    mouseScreenY = mouseY;
    
    // 将屏幕坐标转换为世界坐标，考虑缩放因素
    mouseWorldX = cameraX + static_cast<int>(mouseX / zoomLevel);
    mouseWorldY = cameraY + static_cast<int>(mouseY / zoomLevel);
}

// 移除Player特有的换弹方法，直接使用Entity的
// 实现reloadCurrentWeapon方法
void Player::reloadCurrentWeapon(bool needChamber) {
    Gun* currentGun = nullptr;
    if (heldItem && heldItem->hasFlag(ItemFlag::GUN)) {
        currentGun = static_cast<Gun*>(heldItem.get());
    }
    
    if (currentGun) {
        reloadWeaponAuto(currentGun); // 使用Entity的方法
        
        //// 如果需要上膛且枪膛为空
        //if (needChamber && !currentGun->getChamberedRound()) {
        //    chamberRound(currentGun);
        //}
    }
}



// 实现attemptShoot方法
void Player::attemptShoot() {
    // 检查是否有手持物品，且是枪械
    if (!heldItem) {
        std::cout << "Player cannot shoot - no item held" << std::endl;
        return;
    }
    
    if (!heldItem->hasFlag(ItemFlag::GUN)) {
        std::cout << "Player cannot shoot - held item is not a gun: " << heldItem->getName() << std::endl;
        return;
    }
    
    // 转换为Gun类型
    Gun* gun = static_cast<Gun*>(heldItem.get());
    std::cout << "Player attempting to shoot " << gun->getName() << std::endl;
    
    // 检查枪械状态
    if (gun->getCurrentMagazine()) {
        std::cout << "Current magazine: " << gun->getCurrentMagazine()->getName() 
                  << " with " << gun->getCurrentMagazine()->getCurrentAmmoCount() << " rounds" << std::endl;
    } else {
        std::cout << "No magazine loaded" << std::endl;
    }
    
    if (gun->getChamberedRound()) {
        std::cout << "Round chambered: " << gun->getChamberedRound()->getName() << std::endl;
    } else {
        std::cout << "No round chambered" << std::endl;
    }
    
    // 检查是否可以射击
    if (!canShoot(gun)) {
        std::cout << "Gun cannot shoot - checking if reload needed" << std::endl;
        // 如果需要换弹，自动换弹
        if (needsReload(gun)) {
            std::cout << "Reload needed" << std::endl;
            attemptReload();
        }
        return;
    }
    
    // 计算射击方向
    float dirX = mouseWorldX - x;
    float dirY = mouseWorldY - y;
    
    // 归一化方向向量
    float length = std::sqrt(dirX * dirX + dirY * dirY);
    if (length > 0) {
        dirX /= length;
        dirY /= length;
    }
    
    std::cout << "Firing gun in direction (" << dirX << ", " << dirY << ")" << std::endl;
    
    // 射击
    Bullet* bullet = shootInDirection(gun, dirX, dirY);
    
    // 如果射击成功，添加短暂的射击状态
    if (bullet) {
        std::cout << "Bullet created successfully" << std::endl;
        // 如果不是连续射击状态，添加短暂的射击反馈状态
        if (!hasPlayerState("continuous_shot")) {
            addPlayerState(EntityStateEffect::Type::SHOOTING, "shot_feedback", 100); // 100毫秒的射击反馈状态
        }
        
        // 获得射击技能经验
        if (gun->hasFlag(ItemFlag::RIFLE)) {
            gainWeaponExperience("RIFLE", 1);
        } else if (gun->hasFlag(ItemFlag::PISTOL)) {
            gainWeaponExperience("PISTOL", 1);
        } else {
            gainWeaponExperience("GUN", 1); // 通用枪械经验
        }
    } else {
        std::cout << "Failed to create bullet" << std::endl;
    }
}

// 获取当前交互的实体
Entity* Player::getInteractingEntity() const {
    // TODO: 实现寻找最近的可交互实体的逻辑
    // 可以是死亡的生物、箱子、门等
    return nullptr;
}


// 物品转移方法实现
bool Player::transferItem(Item* item, Storage* sourceStorage, Storage* targetStorage, std::function<void(bool)> callback) {
    // 检查参数有效性
    if (!item || !sourceStorage || !targetStorage) {
        if (callback) {
            callback(false);
        }
        return false;
    }
    
    // 检查物品是否在源存储空间中
    bool itemFound = false;
    for (size_t i = 0; i < sourceStorage->getItemCount(); ++i) {
        if (sourceStorage->getItem(i) == item) {
            itemFound = true;
            break;
        }
    }
    
    if (!itemFound) {
        if (callback) {
            callback(false);
        }
        return false;
    }
    
    // 检查目标存储空间是否有足够空间
    if (!targetStorage->canFitItem(item)) {
        if (callback) {
            callback(false);
        }
        return false;
    }
    
    // 将物品转移操作添加到行为队列中
    transferItemWithAction(item, sourceStorage, targetStorage, callback);
    return true; // 如果执行到这里，说明参数检查都通过了，返回true表示操作已添加到队列
}

// 卸下装备方法实现
void Player::unequipItem(EquipSlot slot, std::function<void(std::unique_ptr<Item>)> callback) {
    // 检查是否是手持物品槽位
    if (slot == EquipSlot::RIGHT_HAND && heldItem) {
        // 如果是手持物品，直接处理而不使用行为队列
        std::unique_ptr<Item> item = std::move(heldItem);
        std::cout << "Player unequipped from right hand: " << item->getName() << std::endl;
        
        // 如果有回调，调用回调函数
        if (callback) {
            callback(std::move(item));
        }
        return;
    }
    
    // 对于其他装备槽位，使用Entity的unequipItemWithAction方法
    unequipItemWithAction(slot, callback);
}

void Player::setPosition(float newX, float newY) {
    x = newX;
    y = newY;
    collider.updatePosition(x, y);
}

// 实现状态管理快捷方法
EntityStateEffect* Player::addPlayerState(
    EntityStateEffect::Type type,
    const std::string& name,
    int duration,
    int priority
) {
    if (!stateManager) return nullptr;
    return stateManager->addState(type, name, duration, priority);
}

bool Player::removePlayerState(const std::string& name) {
    if (!stateManager) return false;
    return stateManager->removeState(name);
}

bool Player::removePlayerState(EntityStateEffect::Type type) {
    if (!stateManager) return false;
    return stateManager->removeState(type);
}

bool Player::hasPlayerState(const std::string& name) const {
    if (!stateManager) return false;
    return stateManager->hasState(name);
}

bool Player::hasPlayerState(EntityStateEffect::Type type) const {
    if (!stateManager) return false;
    return stateManager->hasState(type);
}

// 通用攻击方法（默认主攻击）
void Player::attemptAttack() {
    attemptAttack(WeaponAttackType::PRIMARY);
}

// 带攻击类型的攻击方法
void Player::attemptAttack(WeaponAttackType type) {
    // 检查是否可以攻击
    if (!canAttack()) {
        return;
    }
    
    // 检查是否手持武器
    if (!heldItem) {
        return;
    }
    
    // 检查是否是可攻击的武器
    IWeaponAttack* weaponAttack = dynamic_cast<IWeaponAttack*>(heldItem.get());
    if (!weaponAttack) {
        return;
    }
    
    // 检查武器是否可以攻击
    if (!weaponAttack->canPerformAttack(type)) {
        return;
    }
    
    // 获取攻击参数
    AttackMethod method = weaponAttack->getAttackMethod(type);
    AttackParams params = weaponAttack->getAttackParams(type);
    
    // 设置攻击方向（朝向鼠标）
    float dx = mouseWorldX - x;
    float dy = mouseWorldY - y;
    params.direction = std::atan2(dy, dx);
    
    // 执行攻击
    AttackResult result = attackSystem->executeAttack(method, params);
    
    if (result.hit) {
        // 添加攻击状态
        addPlayerState(EntityStateEffect::Type::ATTACKING, "attack", params.animationDuration);
        
        // 武器攻击后处理
        weaponAttack->onAttackPerformed(type);
        
        std::cout << "玩家攻击成功！目标血量: " << result.target->getHealth() << std::endl;
    } else {
        std::cout << "玩家攻击未命中目标！" << std::endl;
        
        // 如果有目标但未命中，显示miss
        if (result.target) {
            Game* game = Game::getInstance();
            if (game) {
                game->addDamageNumber(result.target->getX(), result.target->getY() - result.target->getRadius(), DamageNumberType::MISS);
            }
        }
    }
}

// 检查是否可以攻击
bool Player::canAttack() const {
    // 检查是否可以执行动作
    if (!canPerformAction()) {
        return false;
    }
    
    // 检查攻击系统是否可以攻击
    if (!attackSystem || !attackSystem->canAttack()) {
        return false;
    }
    
    // 检查是否在攻击状态中
    if (hasPlayerState(EntityStateEffect::Type::ATTACKING)) {
        return false;
    }
    
    return true;
}

// 寻找攻击目标
Entity* Player::findAttackTarget(float range) const {
    Game* game = Game::getInstance();
    if (!game) {
        return nullptr;
    }
    
    Entity* closestTarget = nullptr;
    float closestDistance = range + 1; // 初始化为超出范围的距离
    
    // 检查所有僵尸
    for (const auto& zombie : game->getZombies()) {
        if (!zombie || zombie->getHealth() <= 0) {
            continue;
        }
        
        float dx = zombie->getX() - x;
        float dy = zombie->getY() - y;
        float distance = std::sqrt(dx * dx + dy * dy);
        
        if (distance <= range && distance < closestDistance) {
            closestDistance = distance;
            closestTarget = zombie.get();
        }
    }
    
    // 检查其他生物
    for (const auto& creature : game->getCreatures()) {
        if (!creature || creature.get() == this || creature->getHealth() <= 0) {
            continue;
        }
        
        // 检查是否是敌对阵营
        if (creature->getFaction() == Faction::ENEMY || creature->getFaction() == Faction::HOSTILE) {
            float dx = creature->getX() - x;
            float dy = creature->getY() - y;
            float distance = std::sqrt(dx * dx + dy * dy);
            
            if (distance <= range && distance < closestDistance) {
                closestDistance = distance;
                closestTarget = creature.get();
            }
        }
    }
    
    return closestTarget;
}

// 技能系统相关方法实现
void Player::addSkillExperience(SkillType skillType, int experience) {
    if (skillSystem) {
        skillSystem->addExperience(skillType, experience);
    }
}

int Player::getSkillLevel(SkillType skillType) const {
    if (skillSystem) {
        return skillSystem->getSkillLevel(skillType);
    }
    return 0;
}

int Player::getTotalSkillExperience(SkillType skillType) const {
    if (skillSystem) {
        return skillSystem->getTotalExperience(skillType);
    }
    return 0;
}

int Player::getCurrentLevelSkillExperience(SkillType skillType) const {
    if (skillSystem) {
        return skillSystem->getCurrentLevelExperience(skillType);
    }
    return 0;
}

int Player::getExpToNextSkillLevel(SkillType skillType) const {
    if (skillSystem) {
        return skillSystem->getExpToNextLevel(skillType);
    }
    return 100;
}

// 便捷方法：根据武器类型获得经验
void Player::gainWeaponExperience(const std::string& weaponType, int baseExp) {
    if (!skillSystem) return;
    
    // 所有枪械都获得枪法经验
    if (weaponType == "GUN" || weaponType == "RIFLE" || weaponType == "PISTOL" || 
        weaponType == "SHOTGUN" || weaponType == "SMG" || weaponType == "SNIPER") {
        addSkillExperience(SkillType::MARKSMANSHIP, baseExp);
    }
    
    // 根据具体武器类型获得对应技能经验
    if (weaponType == "PISTOL") {
        addSkillExperience(SkillType::PISTOL, baseExp * 2);
    } else if (weaponType == "RIFLE") {
        addSkillExperience(SkillType::RIFLE, baseExp * 2);
    } else if (weaponType == "SHOTGUN") {
        addSkillExperience(SkillType::SHOTGUN, baseExp * 2);
    } else if (weaponType == "SMG") {
        addSkillExperience(SkillType::SMG, baseExp * 2);
    } else if (weaponType == "SNIPER") {
        addSkillExperience(SkillType::SNIPER, baseExp * 2);
    } else if (weaponType == "HEAVY_WEAPONS") {
        addSkillExperience(SkillType::HEAVY_WEAPONS, baseExp * 2);
    }
}

// 便捷方法：根据近战武器类型获得经验
void Player::gainMeleeExperience(const std::string& meleeType, int baseExp) {
    if (!skillSystem) return;
    
    // 所有近战武器都获得近战经验
    addSkillExperience(SkillType::MELEE, baseExp);
    
    // 根据具体近战武器类型获得对应技能经验
    if (meleeType == "SWORD" || meleeType == "SLASHING") {
        addSkillExperience(SkillType::SLASHING, baseExp * 2);
    } else if (meleeType == "DAGGER" || meleeType == "PIERCING") {
        addSkillExperience(SkillType::PIERCING, baseExp * 2);
    } else if (meleeType == "HAMMER" || meleeType == "BLUNT") {
        addSkillExperience(SkillType::BLUNT, baseExp * 2);
    } else if (meleeType == "UNARMED") {
        addSkillExperience(SkillType::UNARMED, baseExp * 2);
    }
}

// 身体部位伤害系统实现

// 随机选择身体部位（根据概率权重：50:120:90:90:75:75）
Player::BodyPart Player::selectRandomBodyPart() const {
    // 总权重：50+120+90+90+75+75 = 500
    static const int weights[] = {50, 120, 90, 90, 75, 75};
    static const int totalWeight = 500;
    
    int random = rand() % totalWeight;
    int currentWeight = 0;
    
    for (int i = 0; i < 6; i++) {
        currentWeight += weights[i];
        if (random < currentWeight) {
            return static_cast<BodyPart>(i);
        }
    }
    
    // 默认返回躯干（最大权重）
    return BodyPart::TORSO;
}

// 对特定身体部位造成伤害
void Player::takeDamageToBodyPart(int damage, BodyPart part) {
    if (damage <= 0) return;
    
    const char* partName;
    int* partHealth;
    
    switch (part) {
        case BodyPart::HEAD:
            partName = "头部";
            partHealth = &headHealth;
            break;
        case BodyPart::TORSO:
            partName = "躯干";
            partHealth = &torsoHealth;
            break;
        case BodyPart::LEFT_LEG:
            partName = "左腿";
            partHealth = &leftLegHealth;
            break;
        case BodyPart::RIGHT_LEG:
            partName = "右腿";
            partHealth = &rightLegHealth;
            break;
        case BodyPart::LEFT_ARM:
            partName = "左臂";
            partHealth = &leftArmHealth;
            break;
        case BodyPart::RIGHT_ARM:
            partName = "右臂";
            partHealth = &rightArmHealth;
            break;
        default:
            partName = "未知部位";
            partHealth = &torsoHealth;
            break;
    }
    
    // 减少该部位血量
    *partHealth = std::max(0, *partHealth - damage);
    
    std::cout << "玩家" << partName << "受到 " << damage << " 点伤害，剩余血量: " << *partHealth << std::endl;
    
    // 触发受伤屏幕效果（来自Game类）
    Game* game = Game::getInstance();
    if (game) {
        game->triggerHurtEffect(damage);
    }
    
    // 检查是否死亡
    if (isDeadFromBodyDamage()) {
        health = 0;  // 设置Entity的总血量为0
        std::cout << "玩家死亡！" << std::endl;
    }
}

// 获取身体部位最大血量
int Player::getMaxHealthForBodyPart(BodyPart part) {
    switch (part) {
        case BodyPart::HEAD:
            return MAX_HEAD_HEALTH;
        case BodyPart::TORSO:
            return MAX_TORSO_HEALTH;
        case BodyPart::LEFT_LEG:
        case BodyPart::RIGHT_LEG:
            return MAX_LEG_HEALTH;
        case BodyPart::LEFT_ARM:
        case BodyPart::RIGHT_ARM:
            return MAX_ARM_HEALTH;
        default:
            return MAX_TORSO_HEALTH;
    }
}

// 检查是否因身体部位伤害而死亡
bool Player::isDeadFromBodyDamage() const {
    // 头部血量为0即死亡
    if (headHealth <= 0) {
        return true;
    }
    
    // 躯干血量为0即死亡
    if (torsoHealth <= 0) {
        return true;
    }
    
    // 其他部位血量为0不会立即死亡，但可能影响行动能力
    return false;
}

// 重写takeDamage方法，使用身体部位伤害系统
bool Player::takeDamage(const Damage& damage) {
    if (damage.isEmpty()) return false;
    
    int totalDamage = damage.getTotalDamage();
    if (totalDamage <= 0) return false;
    
    // 闪避判定：假设攻击者敏捷为12（可以根据实际攻击者调整）
    int attackerDexterity = 12;
    if (damage.getSource()) {
        // 如果有攻击来源，尝试获取其敏捷属性
        // 这里简化处理，实际可以从Entity获取
        attackerDexterity = 12; // 默认值
    }
    
    // 尝试闪避
    if (attemptDodge(attackerDexterity)) {
        // 闪避成功，不受伤害，显示MISS飘字
        Game* game = Game::getInstance();
        if (game) {
            game->addDamageNumber(x, y - radius, DamageNumberType::MISS);
        }
        return false;
    }
    
    // 闪避失败，正常受伤
    // 随机选择受伤部位
    BodyPart targetPart = selectRandomBodyPart();
    
    // 对该部位造成伤害
    takeDamageToBodyPart(totalDamage, targetPart);
    
    // 更新Entity的总血量为所有部位血量之和
    health = headHealth + torsoHealth + leftLegHealth + rightLegHealth + leftArmHealth + rightArmHealth;
    
    return true;
}

// 闪避系统实现

// 尝试闪避攻击
bool Player::attemptDodge(int attackerDexterity) {
    // 首先检查是否还有闪避次数
    if (dodgeCount >= getMaxDodgesPerWindow()) {
        std::cout << "闪避次数已用完，无法闪避！当前闪避次数: " << dodgeCount << "/" << getMaxDodgesPerWindow() << std::endl;
        return false;
    }
    
    // 获取闪避技能等级
    int dodgeLevel = getSkillLevel(SkillType::DODGE);
    
    // 使用随机数生成器
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> d3(1, 3); // d3骰子
    
    // 对方的敏捷：攻击者敏捷次d3
    int attackerRoll = 0;
    for (int i = 0; i < attackerDexterity; i++) {
        attackerRoll += d3(gen);
    }
    
    // 我的敏捷d3+闪避技能等级
    int defenderRoll = d3(gen) + AGI + dodgeLevel;
    
    std::cout << "闪避判定: 攻击者(" << attackerDexterity << "次d3=" << attackerRoll 
              << ") vs 防御者(" << AGI << "+" << dodgeLevel << "+d3=" << defenderRoll << ")";
    
    if (defenderRoll > attackerRoll) {
        // 闪避成功
        dodgeCount++;
        std::cout << " -> 闪避成功！剩余闪避次数: " << (getMaxDodgesPerWindow() - dodgeCount) << "/" << getMaxDodgesPerWindow() << std::endl;
        
        // 获得闪避经验
        addSkillExperience(SkillType::DODGE, 1);
        
        return true;
    } else {
        // 闪避失败
        std::cout << " -> 闪避失败！" << std::endl;
        return false;
    }
}

// 更新闪避次数（每3秒重置）
void Player::updateDodgeCount() {
    // 获取当前时间（毫秒）
    Uint64 currentTime = SDL_GetTicks();
    
    // 如果超过3秒（3000毫秒），重置闪避次数
    if (currentTime - lastDodgeResetTime >= 3000) {
        if (dodgeCount > 0) {
            std::cout << "闪避次数重置：" << dodgeCount << " -> 0" << std::endl;
        }
        dodgeCount = 0;
        lastDodgeResetTime = currentTime;
    }
}

// 获取每3秒最大闪避次数
int Player::getMaxDodgesPerWindow() const {
    int dodgeLevel = getSkillLevel(SkillType::DODGE);
    return dodgeLevel / 5 + 2;  // 闪避等级/5+2次
}

// 近战攻击系统实现

// 执行近战攻击
bool Player::performMeleeAttack(Creature* target) {
    if (!target) return false;
    
    // 检查是否装备了近战武器
    Item* equippedWeapon = equipmentSystem->getEquippedItem(EquipSlot::RIGHT_HAND);
    MeleeWeapon* meleeWeapon = dynamic_cast<MeleeWeapon*>(equippedWeapon);
    
    if (!meleeWeapon) {
        std::cout << "没有装备近战武器！" << std::endl;
        return false;
    }
    
    // 检查攻击范围（简化处理，假设在范围内）
    // 实际游戏中需要检查距离
    
    // 获取武器攻击参数
    AttackParams attackParams = meleeWeapon->getAttackParams();
    
    // 创建伤害对象用于技能计算
    Damage weaponDamage;
    if (attackParams.damageType == "blunt") {
        weaponDamage.addDamage(DamageType::BLUNT, attackParams.baseDamage);
    } else if (attackParams.damageType == "piercing") {
        weaponDamage.addDamage(DamageType::PIERCE, attackParams.baseDamage);
    } else if (attackParams.damageType == "slashing") {
        weaponDamage.addDamage(DamageType::SLASH, attackParams.baseDamage);
    } else {
        weaponDamage.addDamage(DamageType::BLUNT, attackParams.baseDamage); // 默认钝击
    }
    
    // 获取最高伤害对应的技能等级
    int highestSkillLevel = getHighestDamageSkillLevel(weaponDamage);
    
    // 获取近战技能等级
    int meleeSkillLevel = getSkillLevel(SkillType::MELEE);
    
    // 获取武器命中加成
    int weaponAccuracyBonus = meleeWeapon->getWeaponAccuracyBonus();
    
    // 计算攻击者命中值：(最高伤害技能等级 + 0.4*近战等级) + 敏捷d3 + 武器命中加成
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> d3(1, 3);
    
    float skillBonus = highestSkillLevel + 0.4f * meleeSkillLevel;
    int attackerRoll = static_cast<int>(skillBonus) + d3(gen) + AGI + weaponAccuracyBonus;
    
    // 计算防御者闪避值：敏捷d3 + 近战命中难度
    int defenderRoll = d3(gen) + target->getDexterity() + target->getMeleeHitDifficulty();
    
    std::cout << "近战命中检定: 攻击者(" << skillBonus << "+" << AGI << "+d3+" << weaponAccuracyBonus 
              << "=" << attackerRoll << ") vs 防御者(" << target->getDexterity() << "+" 
              << target->getMeleeHitDifficulty() << "+d3=" << defenderRoll << ")";
    
    if (attackerRoll > defenderRoll) {
        std::cout << " -> 命中！" << std::endl;
        
        // 目标尝试闪避
        Player* targetPlayer = dynamic_cast<Player*>(target);
        bool dodged = false;
        if (targetPlayer) {
            dodged = targetPlayer->attemptDodge(AGI);
        }
        
        if (!dodged) {
            // 闪避失败，造成伤害
            target->takeDamage(weaponDamage);
            
            // 获得技能经验
            if (skillSystem) {
                // 根据最高伤害类型获得对应技能经验
                SkillType primarySkill = getSkillTypeFromDamage(weaponDamage);
                skillSystem->addExperience(primarySkill, 10);
                skillSystem->addExperience(SkillType::MELEE, 5);
            }
            
            return true;
        } else {
            std::cout << "目标闪避成功！" << std::endl;
        }
    } else {
        std::cout << " -> 未命中！" << std::endl;
        
        // 显示miss飘字
        Game* game = Game::getInstance();
        if (game) {
            game->addDamageNumber(target->getX(), target->getY() - target->getRadius(), DamageNumberType::MISS);
        }
    }
    
    return false;
}

// 获取最高伤害对应的技能等级
int Player::getHighestDamageSkillLevel(const Damage& damage) const {
    if (!skillSystem) return 0;
    
    int maxDamage = 0;
    SkillType correspondingSkill = SkillType::MELEE;
    
    // 检查各种伤害类型
    for (const auto& damageInfo : damage.getDamageList()) {
        std::string damageTypeStr = std::get<0>(damageInfo);
        int damageValue = std::get<1>(damageInfo);
        DamageType damageType = stringToDamageType(damageTypeStr);
        
        if (damageValue > maxDamage) {
            maxDamage = damageValue;
            
            // 根据伤害类型确定对应技能
            switch (damageType) {
                case DamageType::BLUNT:
                    correspondingSkill = SkillType::BLUNT;
                    break;
                case DamageType::PIERCE:
                    correspondingSkill = SkillType::PIERCING;
                    break;
                case DamageType::SLASH:
                    correspondingSkill = SkillType::SLASHING;
                    break;
                default:
                    correspondingSkill = SkillType::MELEE;
                    break;
            }
        }
    }
    
    return skillSystem->getSkillLevel(correspondingSkill);
}

// 根据伤害类型获取对应的技能类型
SkillType Player::getSkillTypeFromDamage(const Damage& damage) const {
    int maxDamage = 0;
    SkillType correspondingSkill = SkillType::MELEE;
    
    for (const auto& damageInfo : damage.getDamageList()) {
        std::string damageTypeStr = std::get<0>(damageInfo);
        int damageValue = std::get<1>(damageInfo);
        DamageType damageType = stringToDamageType(damageTypeStr);
        
        if (damageValue > maxDamage) {
            maxDamage = damageValue;
            
            switch (damageType) {
                case DamageType::BLUNT:
                    correspondingSkill = SkillType::BLUNT;
                    break;
                case DamageType::PIERCE:
                    correspondingSkill = SkillType::PIERCING;
                    break;
                case DamageType::SLASH:
                    correspondingSkill = SkillType::SLASHING;
                    break;
                default:
                    correspondingSkill = SkillType::MELEE;
                    break;
            }
        }
    }
    
    return correspondingSkill;
}
