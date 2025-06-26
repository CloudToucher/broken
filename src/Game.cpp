// 在 Game.cpp 中
#include "Game.h"
#include "Player.h"
#include "Map.h"
#include "HUD.h"
#include "Collider.h"
#include <algorithm>
#include "Item.h" 
#include "Gun.h"      // 添加 Gun.h
#include "Magazine.h" // 添加 Magazine.h
#include "Storage.h" // 添加 Magazine.h
#include "Ammo.h"     // 添加 Ammo.h
#include "GunMod.h"   // 添加 GunMod.h
#include "SoundManager.h" // <--- 添加这一行
#include "ItemLoader.h" // 添加 ItemLoader.h
#include <SDL3/SDL_mouse.h>
#include <iostream>
#include <cmath>
#include <random>
#include <chrono>
#include <algorithm>

// 数学常量
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Game 类实现
Game* Game::instance = nullptr;

Game* Game::getInstance() {
    if (instance == nullptr) {
        instance = new Game();
    }
    return instance;
}

// 在构造函数中初始化缩放相关变量和游戏倍率相关变量
Game::Game() : 
    window(nullptr), 
    renderer(nullptr), 
    running(false),
    cameraX(0), 
    cameraY(0),
    windowWidth(800), 
    windowHeight(600),
    font(nullptr),
    frameCount(0),
    fps(0),
    fpsLastTime(0),
    deltaTime(0.0f),
    animationTime(0.0f),
    zoomLevel(1.0f),
    MIN_ZOOM(0.25f),
    MAX_ZOOM(4.0f),
    timeScale(1.0f),
    MIN_TIME_SCALE(0.1f),
    MAX_TIME_SCALE(10.0f),
    debugMode(false), // 初始化调试模式为关闭状态
    gameUI(std::make_unique<GameUI>()), // 初始化游戏UI
    hurtEffectIntensity(0.0f), // 初始化受伤效果
    hurtEffectTime(0.0f)
    // bullets 向量会自动初始化为空
{
    instance = this;
}

// 实现缩放调整方法
void Game::adjustZoomLevel(float change) {
    // 保存旧的缩放级别
    float oldZoom = zoomLevel;
    
    // 调整缩放级别
    zoomLevel += change;
    
    // 限制缩放范围
    if (zoomLevel < MIN_ZOOM) zoomLevel = MIN_ZOOM;
    if (zoomLevel > MAX_ZOOM) zoomLevel = MAX_ZOOM;
    
    // 应用缩放到渲染器
    SDL_SetRenderScale(renderer, zoomLevel, zoomLevel);
    
    // 不需要调整相机位置，因为我们在update中始终保持玩家在屏幕中心
    // 但需要立即更新一次相机位置以避免视觉跳动
    if (player) {
        setCamera(player->getX() - (windowWidth / 2) / zoomLevel, 
                 player->getY() - (windowHeight / 2) / zoomLevel);
    }
}

// 实现游戏倍率调整方法
void Game::adjustTimeScale(float change) {
    // 调整游戏倍率
    timeScale += change;
    
    // 限制倍率范围
    if (timeScale < MIN_TIME_SCALE) timeScale = MIN_TIME_SCALE;
    if (timeScale > MAX_TIME_SCALE) timeScale = MAX_TIME_SCALE;
    
    std::cout << "游戏倍率已调整为: " << timeScale << "x" << std::endl;
}

// 实现游戏倍率设置方法
void Game::setTimeScale(float scale) {
    // 设置游戏倍率
    timeScale = scale;
    
    // 限制倍率范围
    if (timeScale < MIN_TIME_SCALE) timeScale = MIN_TIME_SCALE;
    if (timeScale > MAX_TIME_SCALE) timeScale = MAX_TIME_SCALE;
    
    std::cout << "游戏倍率已设置为: " << timeScale << "x" << std::endl;
}

// 在init方法中初始化HUD字体
bool Game::init() {
    // 初始化 SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL 初始化失败: " << SDL_GetError() << std::endl;
        return false;
    }

    // 初始化 SoundManager (只初始化一次)
    if (!SoundManager::getInstance()->init()) {
        std::cerr << "SoundManager 初始化失败" << std::endl;
        // 根据您的错误处理策略，可能需要返回 false 或继续
    }

    // 加载音效文件 (使用.wav文件)
    SoundManager::getInstance()->loadSound("assets/ar15_shoot.wav", "shoot_ar15");
    SoundManager::getInstance()->loadSound("assets/ar15_reload.wav", "reload");
    SoundManager::getInstance()->loadSound("assets/ar15_unload.wav", "unload");
    SoundManager::getInstance()->loadSound("assets/ar15_bolt_release.wav", "bolt_release");

    // 初始化 SDL_ttf
    if (TTF_Init() < 0) {
        std::cerr << "SDL_ttf 初始化失败: " << SDL_GetError() << std::endl;
        return false;
    }

    // 初始化音频
    if (!SoundManager::getInstance()->init()) {
        std::cerr << "音频 初始化失败" << std::endl;
        return false;
    }

    // 获取显示器分辨率以实现全屏
    SDL_DisplayID displayID = SDL_GetPrimaryDisplay();
    SDL_Rect displayBounds;
    SDL_GetDisplayBounds(displayID, &displayBounds);
    windowWidth = displayBounds.w;
    windowHeight = displayBounds.h;

    // 创建全屏窗口
    window = SDL_CreateWindow("简单 SDL3 游戏", windowWidth, windowHeight, SDL_WINDOW_FULLSCREEN);
    if (!window) {
        std::cerr << "窗口创建失败: " << SDL_GetError() << std::endl;
        return false;
    }

    // 创建渲染器 - 修复 SDL_RENDERER_ACCELERATED 未定义的问题
    renderer = SDL_CreateRenderer(window, nullptr); // 在 SDL3 中不再使用 SDL_RENDERER_ACCELERATED
    if (!renderer) {
        std::cerr << "渲染器创建失败: " << SDL_GetError() << std::endl;
        return false;
    }

    // 加载字体 - 使用Windows系统字体
    font = TTF_OpenFont("C:\\Windows\\Fonts\\simhei.ttf", 24); // 使用黑体
    if (!font) {
        // 尝试其他常见字体
        font = TTF_OpenFont("C:\\Windows\\Fonts\\msyh.ttc", 24); // 尝试微软雅黑
        if (!font) {
            font = TTF_OpenFont("C:\\Windows\\Fonts\\simsun.ttc", 24); // 尝试宋体
            if (!font) {
                font = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 24); // 尝试Arial
                if (!font) {
                    std::cerr << "无法加载字体: " << SDL_GetError() << std::endl;
                    // 继续执行，不要因为字体加载失败而退出游戏
                }
            }
        }
    }

    // 创建无限地图
    gameMap = std::make_unique<Map>(renderer);
    // 初始化地图（生成初始网格）
    gameMap->initialize();

    // 初始化寻路系统
    initPathfinder();

    // 创建玩家，初始位置为(0,0)，位于(0,0)网格的左下角
    player = std::make_unique<Player>(0.0f, 0.0f);

    // 初始化ItemLoader并加载items.json
    if (!ItemLoader::getInstance()->loadItemsFromFile("jsons/items.json")) {
        std::cerr << "Failed to load items from JSON!" << std::endl;

    }
    else {
        std::cout << "Successfully loaded items from JSON!" << std::endl;

            // 使用ItemLoader创建大背包并让玩家装备上
            auto backpack = ItemLoader::getInstance()->createItem("Large Backpack");
            if (backpack) {
                // 让玩家装备背包
                player->getEquipmentSystem()->equipItem(std::move(backpack));
                
                // 注意：不再需要重置状态，因为Action系统会自动处理状态

                std::cout << "Player equipped with Large Backpack." << std::endl;
            }

            // 使用ItemLoader创建弹匣挂并让玩家装备上
            auto magazineCarrier = ItemLoader::getInstance()->createItem("弹挂");
            if (magazineCarrier) {
                // 创建三个弹匣并放入弹匣挂中
                // 创建第一个弹匣并添加到弹匣包1
                auto magazine1 = ItemLoader::getInstance()->createMagazine("StandardRifleMag");
                if (magazine1) {
                    // 装填一些子弹
                    for (int i = 0; i < 15; ++i) {
                        auto ammo = ItemLoader::getInstance()->createAmmo("5.56mm_M855");
                        if (ammo) {
                            magazine1->loadAmmo(std::move(ammo));
                        }
                    }
                    // 获取弹匣包1并添加弹匣
                    Storage* magPouch1 = magazineCarrier->getStorage(0);
                    if (magPouch1) {
                        magPouch1->addItem(std::move(magazine1));
                    }
                }

                // 创建第二个弹匣并添加到弹匣包2
                auto magazine2 = ItemLoader::getInstance()->createMagazine("StandardRifleMag");
                if (magazine2) {
                    // 装填一些子弹
                    for (int i = 0; i < 25; ++i) {
                        auto ammo = ItemLoader::getInstance()->createAmmo("5.56mm_M855A1");
                        if (ammo) {
                            magazine2->loadAmmo(std::move(ammo));
                        }
                    }
                    // 获取弹匣包2并添加弹匣
                    Storage* magPouch2 = magazineCarrier->getStorage(1);
                    if (magPouch2) {
                        magPouch2->addItem(std::move(magazine2));
                    }
                }

                // 创建第三个弹匣并添加到弹匣包3
                auto magazine3 = ItemLoader::getInstance()->createMagazine("StandardRifleMag");
                if (magazine3) {
                    // 装填一些子弹
                    for (int i = 0; i < 30; ++i) {
                        auto ammo = ItemLoader::getInstance()->createAmmo("5.56mm_M995");
                        if (ammo) {
                            magazine3->loadAmmo(std::move(ammo));
                        }
                    }
                    // 获取弹匣包3并添加弹匣
                    Storage* magPouch3 = magazineCarrier->getStorage(2);
                    if (magPouch3) {
                        magPouch3->addItem(std::move(magazine3));
                    }
                }

                // 让玩家装备弹匣挂
                player->getEquipmentSystem()->equipItem(std::move(magazineCarrier));

                std::cout << "Player equipped with 弹挂." << std::endl;
                
                // 创建连体作战服并让玩家装备
                auto combatSuit = ItemLoader::getInstance()->createItem("连体作战服");
                if (combatSuit) {
                    player->getEquipmentSystem()->equipItem(std::move(combatSuit));
                    std::cout << "Player equipped with 连体作战服." << std::endl;
                }
            }

            hud = std::make_unique<HUD>();
            hud->initFont(); // 初始化HUD字体

            // 初始化游戏UI
            if (!gameUI->initFonts()) {
                std::cerr << "Failed to initialize Game UI fonts!" << std::endl;
                return false;
            }

            // 初始化伤害数字字体
            if (!DamageNumber::initFont(font)) {
                std::cerr << "Failed to initialize DamageNumber font!" << std::endl;
                // 继续执行，字体失败不应该终止游戏
            }

            // 初始化ItemLoader并加载items.json
            if (!ItemLoader::getInstance()->loadItemsFromFile("jsons/items.json")) {
                std::cerr << "Failed to load items from JSON!" << std::endl;
                // 继续执行，不要因为物品加载失败而退出游戏
            }
            else {
                std::cout << "Successfully loaded items from JSON!" << std::endl;
            }

            // --- 为玩家生成一把砍刀 ---
            if (player) {
                // 使用通用的MeleeWeapon类，通过JSON配置驱动
                auto meleeWeapon = ItemLoader::getInstance()->createMeleeWeapon("军用砍刀");
                if (meleeWeapon) {
                    meleeWeapon->setRarity(ItemRarity::COMMON);
                    
                                // 将砍刀装备到玩家手上
            player->equipItem(std::move(meleeWeapon));
            std::cout << "Player equipped with MeleeWeapon: 军用砍刀 (data-driven creation)." << std::endl;
        } else {
            std::cout << "Failed to create MeleeWeapon: 军用砍刀" << std::endl;
        }
        
        // 设置测试技能数据：建造等级5级70%，闪避18级
        if (player && player->getSkillSystem()) {
            // 5级70%意味着总经验值为：5*100 + 70 = 570
            player->getSkillSystem()->addExperience(SkillType::CONSTRUCTION, 570);
            std::cout << "测试数据：建造技能设置为5级70%" << std::endl;
            
            // 18级闪避：18*100 = 1800经验值
            player->getSkillSystem()->addExperience(SkillType::DODGE, 1800);
            std::cout << "测试数据：闪避技能设置为18级" << std::endl;
        }
            }
            
            /*
            // --- 为玩家生成一把测试枪 --- (暂时注释掉，专注测试砍刀)
            if (player) {
                // 1. 使用ItemLoader创建一把枪
                auto testGun = ItemLoader::getInstance()->createGun("HK416");
                if (testGun) {
                    // 设置稀有度
                    testGun->setRarity(ItemRarity::EPIC);

                    // 2. 使用ItemLoader创建一个弹匣
                    auto testMagazine = ItemLoader::getInstance()->createMagazine("StandardRifleMag");

                    // 3. 为弹匣装填子弹
                    if (testMagazine) {
                        for (int i = 0; i < 20; ++i) { // 装填20发测试子弹
                            auto ammo = ItemLoader::getInstance()->createAmmo("5.56mm_M855");
                            if (ammo && !testMagazine->loadAmmo(std::move(ammo))) {
                                break; // 弹匣满了
                            }
                        }
                    }

                    // 4. 将弹匣装到枪上并上膛
                    if (testMagazine) {
                        testGun->loadMagazine(std::move(testMagazine));
                        testGun->chamberManually(); // 手动上膛第一发
                    }

                    // 5. 将枪交给玩家
                    player->equipItem(std::move(testGun)); // 注意：枪械使用普通的equipItem方法，因为它们是手持物品
                    std::cout << "Player equipped with HK416." << std::endl;
                }

                // 6. 创建额外的弹匣并添加到玩家背包
                // 创建三个额外弹匣，分别装填不同类型的子弹
                std::vector<std::string> ammoTypes = { "5.56mm_M855", "5.56mm_M855A1", "5.56mm_M995" };
                std::vector<int> ammoCount = { 15, 10, 30 }; // 每个弹匣装填的子弹数量

                for (int i = 0; i < 3; ++i) {
                    auto extraMag = ItemLoader::getInstance()->createMagazine("StandardRifleMag");
                    if (extraMag) {
                        // 设置稀有度
                        extraMag->setRarity(ItemRarity::EPIC);

                        // 装填子弹
                        for (int j = 0; j < ammoCount[i]; ++j) {
                            auto ammo = ItemLoader::getInstance()->createAmmo(ammoTypes[i]);
                            if (ammo && !extraMag->loadAmmo(std::move(ammo))) {
                                break; // 弹匣满了
                            }
                        }

                        // 添加到玩家背包
                        player->addItem(std::move(extraMag));
                        std::cout << "Added magazine with " << ammoTypes[i] << " ammo to player inventory." << std::endl;
                    }
                }
            }
            // --- 测试枪生成结束 ---

            // 使用ItemLoader创建一个消音器并安装到玩家的枪上
            auto suppressor = ItemLoader::getInstance()->createGunMod("Silencer");
            auto laser = ItemLoader::getInstance()->createGunMod("SureFireX400U");
            if (suppressor) {
                suppressor->setRarity(ItemRarity::RARE);

                // 获取玩家的枪并安装消音器
                if (player) {
                    Gun* playerGun = dynamic_cast<Gun*>(player->getHeldItem());
                    if (playerGun) {
                        // 安装消音器到枪的枪口位置
                        if (playerGun->attach(AttachmentSlot::MUZZLE, std::move(suppressor))) {
                            std::cout << "Silencer attached to player's gun." << std::endl;
                        }
                        else {
                            std::cout << "Failed to attach silencer to player's gun." << std::endl;
                            // 如果安装失败，将消音器添加到玩家背包
                            player->addItem(std::move(suppressor));
                        }
                        playerGun->attach(AttachmentSlot::RAIL, std::move(laser));
                    }
                    else {
                        std::cout << "Player has no gun equipped." << std::endl;
                        // 将消音器添加到玩家背包
                        player->addItem(std::move(suppressor));
                    }
                }
            }
            else {
                std::cout << "Failed to create silencer from ItemLoader." << std::endl;
            }
            */


            // 生成一些丧尸
            spawnZombie(2500, 500, ZombieType::NORMAL);
            spawnZombie(1500, 1000, ZombieType::RUNNER);
            spawnZombie(1500, 500, ZombieType::NORMAL);
            spawnZombie(1700, 500, ZombieType::BLOATER);
            spawnZombie(1900, 500, ZombieType::NORMAL);
            spawnZombie(2100, 500, ZombieType::SPITTER);
            spawnZombie(2300, 500, ZombieType::NORMAL);
            spawnZombie(2500, 500, ZombieType::TANK);
            spawnZombie(1500, 1000, ZombieType::RUNNER);
            spawnZombie(1500, 500, ZombieType::NORMAL);
            spawnZombie(1700, 500, ZombieType::NORMAL);
            spawnZombie(1900, 500, ZombieType::RUNNER);
            spawnZombie(2100, 500, ZombieType::NORMAL);
            spawnZombie(2300, 500, ZombieType::BLOATER);
            spawnZombie(2500, 500, ZombieType::NORMAL);

            // 初始化相机位置（以玩家为中心）
            setCamera(player->getX() - windowWidth / 2, player->getY() - windowHeight / 2);

            // 初始化物品刷新集群
            initItemSpawnClusters();

            // 创建一个远程玩家进行测试
            // 注释掉重复的砍刀装备代码
            /*
            // --- 为玩家生成一把砍刀 ---
            if (player) {
                // 使用ItemLoader创建一把砍刀
                auto weapon = ItemLoader::getInstance()->createWeapon("军用砍刀");
                if (weapon) {
                    // 设置武器的稀有度
                    weapon->setRarity(ItemRarity::COMMON);

                    // 将武器装备到玩家手上
                    player->equipItem(std::move(weapon));
                    std::cout << "Player equipped with 军用砍刀." << std::endl;
                }
            }
            */

            // 生成测试丧尸（不同类型）
            spawnZombie(200, 200, ZombieType::NORMAL);
            spawnZombie(250, 200, ZombieType::RUNNER);
            spawnZombie(300, 200, ZombieType::BLOATER);

            // 设置丧尸的寻路智能程度
            for (auto& zombie : zombies) {
                switch (zombie->getZombieType()) {
                    case ZombieType::NORMAL:
                        zombie->setPathfindingIntelligence(1.2f); // 低智能
                        break;
                    case ZombieType::RUNNER:
                        zombie->setPathfindingIntelligence(2.5f); // 中等智能
                        break;
                    case ZombieType::BLOATER:
                        zombie->setPathfindingIntelligence(1.5f); // 较低智能
                        break;
                    case ZombieType::SPITTER:
                        zombie->setPathfindingIntelligence(3.0f); // 较高智能
                        break;
                    case ZombieType::TANK:
                        zombie->setPathfindingIntelligence(1.8f); // 中低智能
                        break;
                }
            }
            
            // 生成测试地形来验证寻路系统
            generateTestTerrain();

            running = true;
            return true;
        }
    }

// 在handleEvents方法中添加鼠标释放事件处理
void Game::handleEvents() {
    SDL_Event event;
    // 获取键盘状态
    const bool* keyState = SDL_GetKeyboardState(nullptr);
    
    while (SDL_PollEvent(&event)) {
        // 处理退出事件
        if (event.type == SDL_EVENT_QUIT) {
            running = false;
        }
        // 处理鼠标移动事件
        else if (event.type == SDL_EVENT_MOUSE_MOTION) {
            // 更新玩家的鼠标位置
            player->handleMouseMotion(event.motion.x, event.motion.y, cameraX, cameraY);
            
            // 如果UI界面打开，更新鼠标悬停的物品并处理拖拽
            if (gameUI->isAnyUIOpen()) {
                // 获取实际窗口尺寸
                int actualWidth = 0, actualHeight = 0;
                SDL_GetWindowSizeInPixels(window, &actualWidth, &actualHeight);
                
                // 处理鼠标移动事件
                gameUI->handleMouseMotion(event.motion.x, event.motion.y, 
                                         static_cast<float>(actualWidth), static_cast<float>(actualHeight));
            }
        }
        // 处理鼠标按下事件
        else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            // 检查是否点击了UI元素
            bool uiClicked = false;
            
            // 检查是否点击了玩家UI元素
            if (gameUI->isAnyUIOpen() && event.button.button == SDL_BUTTON_LEFT) {
                // 获取实际窗口尺寸
                int actualWidth = 0, actualHeight = 0;
                SDL_GetWindowSizeInPixels(window, &actualWidth, &actualHeight);
                
                // 处理玩家UI点击
                if (gameUI->isPlayerUIOpen()) {
                    uiClicked = gameUI->handleClick(event.button.x, event.button.y, player.get(), 
                                                  static_cast<float>(actualWidth), static_cast<float>(actualHeight));
                    
                    // 处理Storage折叠/展开按钮点击
                    if (!uiClicked) {
                        uiClicked = gameUI->handleStorageClick(event.button.x, event.button.y, player.get(), nullptr, 
                                                           static_cast<float>(actualWidth), static_cast<float>(actualHeight));
                    }
                }
            }
            
            // 检查是否点击了退出按钮
            if (event.button.button == SDL_BUTTON_LEFT && !uiClicked) {
                if (hud->isExitButtonClicked(event.button.x, event.button.y)) {
                    running = false;
                    uiClicked = true;
                }
            }
            
            // 如果没有点击UI元素，则处理玩家的鼠标点击
            if (!uiClicked) {
                player->handleMouseClick(event.button.button);
            }
        }
        // 处理鼠标释放事件
        else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            // 处理鼠标释放
            player->handleMouseRelease(event.button.button);
            
            // 如果UI界面打开，处理物品拖拽释放
            if (gameUI->isAnyUIOpen() && event.button.button == SDL_BUTTON_LEFT) {
                // 获取实际窗口尺寸
                int actualWidth = 0, actualHeight = 0;
                SDL_GetWindowSizeInPixels(window, &actualWidth, &actualHeight);
                
                // 处理鼠标释放事件
                gameUI->handleMouseRelease(event.button.x, event.button.y, player.get(),
                                          static_cast<float>(actualWidth), static_cast<float>(actualHeight));
            }
        }
        // 处理鼠标滚轮事件
        else if (event.type == SDL_EVENT_MOUSE_WHEEL) {
            // 检查Ctrl键是否被按下
            if (event.wheel.which != SDL_TOUCH_MOUSEID && (SDL_GetModState() & SDL_KMOD_CTRL)) {
                // 根据滚轮方向调整缩放比例
                float zoomChange = event.wheel.y * 0.1f; // 滚轮向上为正，向下为负
                adjustZoomLevel(zoomChange);
            }
        }
        // 处理键盘事件
        else if (event.type == SDL_EVENT_KEY_DOWN) {
            // 处理按键按下事件
            switch (event.key.key) { // 修改这里，从event.key.keysym.sym改为event.key.key
            case SDLK_ESCAPE: // Esc键关闭背包界面或退出游戏
                if (gameUI->isAnyUIOpen()) {
                    if (gameUI->isPlayerUIOpen()) {
                        gameUI->closePlayerUI(this);
                    }
                }
                else {
                    running = false;
                }
                break;
            case SDLK_EQUALS: // 加号键
                if (SDL_GetModState() & SDL_KMOD_SHIFT) { // 检查Shift键是否按下
                    adjustZoomLevel(0.1f); // 放大
                }
                break;
            case SDLK_MINUS: // 减号键
                adjustZoomLevel(-0.1f); // 缩小
                break;
            case SDLK_RIGHTBRACKET: // ']'键
                adjustTimeScale(0.1f); // 增加游戏速度
                break;
            case SDLK_LEFTBRACKET: // '['键
                adjustTimeScale(-0.1f); // 减少游戏速度
                break;
            case SDLK_F3: // F3键切换调试模式
                toggleDebugMode();
                break;
            case SDLK_F4: // F4键测试确认框
                if (gameUI) {
                    gameUI->testConfirmationDialog();
                }
                break;
            case SDLK_F5: // F5键测试存储选择确认框
                if (gameUI) {
                    gameUI->testStorageSelectionDialog();
                }
                break;
            case SDLK_F6: // F6键测试miss显示
                if (player) {
                    addDamageNumber(player->getX(), player->getY() - 50, DamageNumberType::MISS);
                    std::cout << "测试miss显示已触发" << std::endl;
                }
                break;
            case SDLK_F7: // F7键测试覆盖率系统
                testCoverageSystem();
                break;
            case SDLK_F8: // F8键测试背包弹药
                testAmmoInInventory();
                break;
            case SDLK_TAB: // Tab键切换背包界面
                togglePlayerUI();
                break;
            }
            
            // 检查Shift键是否被按下
            if (SDL_GetModState() & SDL_KMOD_SHIFT) {
                // 使用UP键增加游戏倍率
                if (keyState[SDL_SCANCODE_UP]) {
                    adjustTimeScale(0.1f);
                }
                // 使用DOWN键减少游戏倍率
                else if (keyState[SDL_SCANCODE_DOWN]) {
                    adjustTimeScale(-0.1f);
                }
                // 使用数字0键重置游戏倍率为1.0
                else if (keyState[SDL_SCANCODE_F1]) {
                    setTimeScale(1.0f);
                }
            }
        }
    }

    // 处理角色输入 - 传入调整后的deltaTime
    float adjustedDeltaTime = getAdjustedDeltaTime();
    
    // 只有在背包界面关闭时才处理玩家输入
    if (!gameUI || !gameUI->isAnyUIOpen()) {
        player->handleInput(keyState, adjustedDeltaTime);
    }
}

// 切换玩家背包界面显示状态
void Game::togglePlayerUI() {
    if (gameUI) {
        gameUI->togglePlayerUI(this);
        
        // 如果界面打开，更新界面内容
        if (gameUI->isPlayerUIOpen()) {
            gameUI->updatePlayerUI(player.get());
        }
    }
}


// 修改update方法，添加对生物的更新
void Game::update() {
    // 计算deltaTime
    static Uint64 lastFrameTime = SDL_GetTicks();
    Uint64 currentFrameTime = SDL_GetTicks();
    deltaTime = (currentFrameTime - lastFrameTime) / 1000.0f; // 转换为秒
    lastFrameTime = currentFrameTime;
    
    // 限制deltaTime，防止过大的值（例如，在调试器中暂停时）
    if (deltaTime > 0.1f) {
        deltaTime = 0.1f;
    }
    
    // 计算调整后的deltaTime
    float adjustedDeltaTime = getAdjustedDeltaTime();
    
    // 处理所有子弹的更新和碰撞检测
    processBullets();

    // 更新指针到障碍物距离
    if (player) {
        // 获取玩家位置和鼠标方向
        int playerX = player->getX();
        int playerY = player->getY();
        float mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        // 将鼠标坐标转换为世界坐标
        float worldMouseX = mouseX / zoomLevel + cameraX;
        float worldMouseY = mouseY / zoomLevel + cameraY;
        
        // 计算方向向量
        float dirX = worldMouseX - playerX;
        float dirY = worldMouseY - playerY;
        float length = std::sqrt(dirX * dirX + dirY * dirY);
        if (length > 0) {
            dirX /= length;
            dirY /= length;
        }
        
        // 初始化最小距离为最大值
        pointerToObstacleDistance = std::numeric_limits<float>::max();
        
        // 检查与所有障碍物的碰撞
        for (const auto& obstacle : gameMap->getObstacles()) {
            float distance = obstacle.raycast(playerX, playerY, dirX, dirY);
            if (distance >= 0 && distance < pointerToObstacleDistance) {
                pointerToObstacleDistance = distance;
            }
        }

        // 检查与所有怪物的碰撞
        // 检查丧尸碰撞
        for (const auto& zombie : zombies) {
            if (zombie && zombie->getHealth() > 0) {
                const Collider& zombieCollider = zombie->getCollider();
                float distance = zombieCollider.raycast(playerX, playerY, dirX, dirY);
                if (distance >= 0 && distance < pointerToObstacleDistance) {
                    pointerToObstacleDistance = distance;
                }
            }
        }
    
        // 如果没有找到障碍物，设置为最大距离
        if (pointerToObstacleDistance == std::numeric_limits<float>::max()) {
            pointerToObstacleDistance = 1500.0f; // 设置为激光最大长度
        }
    }

    // 移除已死亡的丧尸（丧尸死亡后可以立即移除）
    zombies.erase(
        std::remove_if(zombies.begin(), zombies.end(),
            [](const std::unique_ptr<Zombie>& zombie) {
                return zombie->getHealth() <= 0;
            }),
        zombies.end()
    );

    // 改进的实体间碰撞检测
    // 使用新的物理系统处理实体间碰撞
    processEntityPhysics();

    // 然后处理玩家与丧尸和生物的碰撞
    for (auto& zombie : zombies) {
        player->resolveCollision(zombie.get());
    }

    for (auto& creature : creatures) {
        player->resolveCollision(creature.get());
    }

    // 更新玩家
    if (player) {
        player->update(adjustedDeltaTime);
        
        // 更新动画时间
        animationTime += adjustedDeltaTime * 2.0f; // 2倍速度让动画更活跃
        if (animationTime > 2 * M_PI) {
            animationTime -= 2 * M_PI; // 保持在0-2π范围内
        }
    }
    
    // 更新丧尸
    updateZombies(adjustedDeltaTime);
    
    // 更新生物
    updateCreatures(adjustedDeltaTime);

    // 更新地图（根据玩家位置动态加载/卸载网格）
    // 测试期间禁用动态地图系统
    // gameMap->updatePlayerPosition(player->getX(), player->getY());
    
    // 处理异步加载队列
    // 测试期间禁用动态地图加载
    // gameMap->update();
    
    // 更新相机位置（以玩家为中心）
    setCamera(player->getX() - (windowWidth / 2) / zoomLevel, 
         player->getY() - (windowHeight / 2) / zoomLevel);

    // 更新FPS计数
    frameCount++;
    Uint64 currentTime = SDL_GetTicks();
    if (currentTime - fpsLastTime >= 1000) {
        fps = frameCount;
        frameCount = 0;
        fpsLastTime = currentTime;
    }

    // 更新远程玩家
    updateRemotePlayers(adjustedDeltaTime);
    
    // 更新伤害数字
    updateDamageNumbers();
    
    // 更新受伤屏幕效果
    updateHurtEffect();
}

// 修改render方法
void Game::render() {
    // 清空屏幕，设置为纯黑色背景以完全清除上一帧内容
    SDL_SetRenderDrawColor(renderer, 125, 125, 125, 255); // 使用灰色背景
    SDL_RenderClear(renderer);
    
    // 保存当前缩放
    float currentScaleX, currentScaleY;
    SDL_GetRenderScale(renderer, &currentScaleX, &currentScaleY);

    // 应用当前缩放
    SDL_SetRenderScale(renderer, zoomLevel, zoomLevel);

    // 渲染地图
    gameMap->render(renderer, cameraX, cameraY);

    // 渲染所有丧尸
    renderZombies();
    
    // 渲染所有生物
    renderCreatures();
    
    // 渲染所有子弹
    renderBullets();

    // 渲染角色
    player->render(renderer, cameraX, cameraY);
    
    // 渲染攻击范围（如果玩家手持近战武器）
    renderAttackRange();
    
    // 如果调试模式开启，渲染所有碰撞箱和寻路路径
    if (debugMode) {
        renderColliders();
        renderCreaturePaths();
    }
    
    // 渲染远程玩家
    renderRemotePlayers();
    
    // 渲染伤害数字（在缩放环境下）
    renderDamageNumbers();
    
    // 恢复原始缩放以渲染HUD
    SDL_SetRenderScale(renderer, 1.0f, 1.0f);

    // 获取当前弹药信息（砍刀测试期间暂时不需要）
    int currentAmmo = 0;
    int maxAmmo = 0;
    
    Item* heldItem = player->getHeldItem();
    if (heldItem && heldItem->hasFlag(ItemFlag::WEAPON)) {
        // 检查是否是近战武器
        if (heldItem->hasFlag(ItemFlag::MELEE)) {
            // 近战武器不需要弹药信息，保持默认值
            currentAmmo = 0;
            maxAmmo = 0;
        }
        /*
        // 枪械处理代码（暂时注释掉）
        Gun* gun = static_cast<Gun*>(heldItem);
        if (gun) {  // 添加gun的空指针检查
            Magazine* mag = gun->getCurrentMagazine();
            
            if (mag) {
                try {
                    // 检查Magazine的有效性
                    if (mag->isValid()) {
                        currentAmmo = mag->getCurrentAmmoCount();
                        maxAmmo = mag->getCapacity();
                    } else {
                        std::cerr << "Magazine对象无效，使用默认值" << std::endl;
                        currentAmmo = 0;
                        maxAmmo = 0;
                    }
                } catch (...) {
                    // 如果访问Magazine时发生异常，使用默认值
                    std::cerr << "访问Magazine时发生异常，使用默认值" << std::endl;
                    currentAmmo = 0;
                    maxAmmo = 0;
                }
            }

            // 获取鼠标位置
            float mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);

            // 更新和渲染准星状态
            updateAndRenderCrosshair(renderer, mouseX, mouseY, gun);
        }
        */
    }
    
    // 显示默认鼠标指针（砍刀不需要特殊准星）
    SDL_ShowCursor();
    
    // 渲染 HUD
    hud->render(renderer, player->getHealth(), currentAmmo, maxAmmo);
    
    // 检查玩家是否有正在执行的行动，如果有则渲染行动进度条
    ActionQueue* actionQueue = player->getActionQueue();
    if (actionQueue) {
        Action* currentAction = actionQueue->getCurrentAction();
        if (currentAction && currentAction->isActionStarted() && !currentAction->isActionCompleted()) {
            float duration = currentAction->getDuration();
            float elapsed = actionQueue->getElapsedTime();
            
            // 确保进度条在Action完成时立即消失
            if (duration <= 0 || currentAction->isActionCompleted()) {
                // Action已完成，不渲染进度条
            } else {
                hud->renderActionProgress(renderer, currentAction, duration, elapsed);
            }
        }
    }

    // 渲染游戏UI（如果有任何UI打开）
    if (gameUI && gameUI->isAnyUIOpen()) {
        // 如果玩家背包界面打开，更新界面内容
        if (gameUI->isPlayerUIOpen()) {
            gameUI->updatePlayerUI(player.get());
        }
        // 获取实际窗口尺寸
        int actualWidth = 0, actualHeight = 0;
        SDL_GetWindowSizeInPixels(window, &actualWidth, &actualHeight);
        // 渲染界面
        gameUI->render(renderer, static_cast<float>(actualWidth), static_cast<float>(actualHeight));
    }
    
    // 渲染FPS
    if (font) {
        // 创建FPS文本
        char fpsText[16];
        SDL_snprintf(fpsText, sizeof(fpsText), "FPS: %d", fps);

        // 设置文本颜色（白色）
        SDL_Color textColor = { 255, 0, 255, 255 };

        // 创建文本表面 - 修复参数顺序问题
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, fpsText,0, textColor);
        if (textSurface) {
            // 创建纹理
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            if (textTexture) {
                // 设置渲染位置（左上角）
                SDL_FRect textRect = { 10.0f, 10.0f, static_cast<float>(textSurface->w), static_cast<float>(textSurface->h) };

                // 渲染文本
                SDL_RenderTexture(renderer, textTexture, nullptr, &textRect);

                // 释放纹理
                SDL_DestroyTexture(textTexture);
            }

            // 释放表面
            SDL_DestroySurface(textSurface);
        }
    }
    
    // 渲染调试模式文本
    if (debugMode && font) {
        // 创建调试模式文本
        const char* debugText = "调试模式已开启 (F3切换)";

        // 设置文本颜色（紫色）
        SDL_Color debugColor = { 255, 0, 255, 255 };

        // 创建文本表面
        SDL_Surface* debugSurface = TTF_RenderText_Solid(font, debugText, 0, debugColor);
        if (debugSurface) {
            // 创建纹理
            SDL_Texture* debugTexture = SDL_CreateTextureFromSurface(renderer, debugSurface);
            if (debugTexture) {
                // 设置渲染位置（左上角FPS下方）
                SDL_FRect debugRect = { 10.0f, 40.0f, static_cast<float>(debugSurface->w), static_cast<float>(debugSurface->h) };

                // 渲染文本
                SDL_RenderTexture(renderer, debugTexture, nullptr, &debugRect);

                // 释放纹理
                SDL_DestroyTexture(debugTexture);
            }

            // 释放表面
            SDL_DestroySurface(debugSurface);
        }
    }
    
    // 恢复原始缩放
    SDL_SetRenderScale(renderer, currentScaleX, currentScaleY);

    // 渲染受伤屏幕效果（在最后渲染，覆盖在所有UI之上）
    renderHurtEffect();

    // 呈现渲染的内容
    SDL_RenderPresent(renderer);
}

void Game::clean() {
    // 首先暂停所有实体的ActionQueue，防止在清理过程中继续执行
    if (player) {
        if (auto* actionQueue = player->getActionQueue()) {
            actionQueue->pause();
            actionQueue->clearActions();
        }
    }
    
    for (auto& zombie : zombies) {
        if (auto* actionQueue = zombie->getActionQueue()) {
            actionQueue->pause();
            actionQueue->clearActions();
        }
    }
    
    for (auto& creature : creatures) {
        if (auto* actionQueue = creature->getActionQueue()) {
            actionQueue->pause();
            actionQueue->clearActions();
        }
    }
    
    // 清理游戏对象
    player.reset();
    gameMap.reset();
    hud.reset();
    zombies.clear();
    creatures.clear();  // 确保也清理creatures容器
    bullets.clear(); // 清理子弹容器

    // 清理纹理缓存
    Tile::clearTextureCache();

    // 清理 SoundManager
    SoundManager::getInstance()->clean();

    // 清理字体
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }

    // 清理 SDL
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    TTF_Quit();
    SDL_Quit();

    std::cout << "游戏已清理并退出。" << std::endl;
}

void Game::run() {
    const int FPS = 60;
    const int frameDelay = 1000 / FPS;

    Uint64 frameStart;
    Uint64 lastFrameTime = SDL_GetTicks();
    int frameTime;

    // 在游戏开始时生成一些子弹
    spawnItemsFromCluster(ammoSpawnCluster);
    
    // 生成一些测试丧尸来测试寻路系统（放置在测试地形区域）
    spawnZombie(1000, 200, ZombieType::NORMAL);   // 远处的丧尸
    spawnZombie(800, -200, ZombieType::RUNNER);   // 测试地形区域的丧尸
    spawnZombie(1200, 300, ZombieType::BLOATER);  // 测试地形右侧的丧尸
    
    // 设置丧尸的寻路智能程度
    for (auto& zombie : zombies) {
        switch (zombie->getZombieType()) {
            case ZombieType::NORMAL:
                zombie->setPathfindingIntelligence(1.2f); // 低智能
                break;
            case ZombieType::RUNNER:
                zombie->setPathfindingIntelligence(2.5f); // 中等智能
                break;
            case ZombieType::BLOATER:
                zombie->setPathfindingIntelligence(1.5f); // 较低智能
                break;
            case ZombieType::SPITTER:
                zombie->setPathfindingIntelligence(3.0f); // 较高智能
                break;
            case ZombieType::TANK:
                zombie->setPathfindingIntelligence(1.8f); // 中低智能
                break;
        }
    }
    
    // 生成测试地形来验证寻路系统
    generateTestTerrain();

    while (running) {
        frameStart = SDL_GetTicks();
        
        // 计算deltaTime（秒）
        deltaTime = (frameStart - lastFrameTime) / 1000.0f;
        lastFrameTime = frameStart;
        
        // 限制deltaTime，防止过大的值（例如，在调试器中暂停时）
        if (deltaTime > 0.1f) {
            deltaTime = 0.1f;
        }

        handleEvents();
        update();
        render();

        frameTime = static_cast<int>(SDL_GetTicks() - frameStart);

        if (frameDelay > frameTime) {
            SDL_Delay(frameDelay - frameTime);
        }
    }

    clean();
}

void Game::setCamera(float x, float y) {
    // 设置相机位置（无限地图不需要边界检查）
    cameraX = x;
    cameraY = y;
}



// 新增processBullets方法，集中处理子弹逻辑，使用调整后的deltaTime
// 在 Game.cpp 中的 processBullets 方法中
void Game::processBullets() {
    // 获取调整后的deltaTime
    float adjustedDeltaTime = getAdjustedDeltaTime();
    
    // 使用迭代器遍历所有子弹，以便安全删除
    auto it = bullets.begin();
    while (it != bullets.end()) {
        Bullet* bullet = it->get();
        if (!bullet->isActive()) {
            // 子弹已失活，删除它
            it = bullets.erase(it);
            continue;
        }
        
        // 更新子弹位置，使用调整后的deltaTime
        bullet->update(adjustedDeltaTime);
        
        // 收集所有实体用于碰撞检测
        std::vector<Entity*> allEntities;
        allEntities.push_back(player.get());
        for (auto& zombie : zombies) {
            allEntities.push_back(zombie.get());
        }
        for (auto& creature : creatures) {
            allEntities.push_back(creature.get());
        }
        
        // 检查与障碍物的碰撞
        if (bullet->checkObstacleCollisions(gameMap->getObstacles())) {
            // 子弹已失活，但保留到本帧结束再删除
            ++it;
            continue;
        }
        
        // 检查与实体的碰撞
        bullet->checkEntityCollisions(allEntities);
        
        // 移动到下一个子弹
        ++it;
    }
}

// 添加创建子弹的方法实现
Bullet* Game::createBullet(float startX, float startY, float dirX, float dirY, float speed, 
                          Entity* owner, int damageValue, const std::string& damageType, int penetration, float range) {
    // 创建新子弹并添加到管理容器中
    auto bullet = std::make_unique<Bullet>(startX, startY, dirX, dirY, speed, owner, damageValue, damageType, penetration, range);
    
    // 获取指针用于返回
    Bullet* bulletPtr = bullet.get();
    
    // 将子弹所有权转移到游戏管理容器
    bullets.push_back(std::move(bullet));
    
    return bulletPtr;
}

// 添加渲染所有子弹的方法实现
void Game::renderBullets() {
    for (const auto& bullet : bullets) {
        bullet->render(renderer, cameraX, cameraY);
    }
}

// 实现renderColliders方法，用于在调试模式下显示所有碰撞箱
void Game::renderColliders() {
    if (!debugMode) return;
    
    // 渲染玩家碰撞箱（紫色）
    const Collider& playerCollider = player->getCollider();
    SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
    playerCollider.render(renderer, cameraX, cameraY);
    
    // 渲染丧尸碰撞箱（红色）
    for (const auto& zombie : zombies) {
        const Collider& zombieCollider = zombie->getCollider();
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        zombieCollider.render(renderer, cameraX, cameraY);
    }
    
    // 渲染生物碰撞箱（橙色）
    for (const auto& creature : creatures) {
        const Collider& creatureCollider = creature->getCollider();
        SDL_SetRenderDrawColor(renderer, 255, 100, 0, 255);
        creatureCollider.render(renderer, cameraX, cameraY);
    }
    
    // 渲染子弹轨迹（黄色）
    for (const auto& bullet : bullets) {
        if (bullet->isActive()) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_RenderLine(renderer,
                static_cast<int>(bullet->getX() - cameraX),
                static_cast<int>(bullet->getY() - cameraY),
                static_cast<int>(bullet->getX() - cameraX + bullet->getDirX() * 10),
                static_cast<int>(bullet->getY() - cameraY + bullet->getDirY() * 10));
        }
    }
    
    // 渲染地图中的地形碰撞箱（绿色）
    // 获取当前屏幕可见区域的tile范围
    int minTileX = static_cast<int>(cameraX / 64) - 1;
    int maxTileX = static_cast<int>((cameraX + windowWidth / zoomLevel) / 64) + 1;
    int minTileY = static_cast<int>(cameraY / 64) - 1;
    int maxTileY = static_cast<int>((cameraY + windowHeight / zoomLevel) / 64) + 1;
    
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 128); // 半透明绿色
    for (int tileX = minTileX; tileX <= maxTileX; tileX++) {
        for (int tileY = minTileY; tileY <= maxTileY; tileY++) {
            Tile* tile = gameMap->getTileAt(tileX * 64, tileY * 64);
            if (tile && tile->hasColliderWithPurpose(ColliderPurpose::TERRAIN)) {
                // 渲染该tile的所有地形碰撞箱
                auto terrainColliders = tile->getCollidersByPurpose(ColliderPurpose::TERRAIN);
                for (Collider* terrainCollider : terrainColliders) {
                    terrainCollider->render(renderer, cameraX, cameraY);
                }
            }
        }
    }
    
    // 渲染旧的障碍物碰撞箱（蓝色，用于向后兼容）
    const std::vector<Collider>& obstacles = gameMap->getObstacles();
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    for (const auto& obstacle : obstacles) {
        obstacle.render(renderer, cameraX, cameraY);
    }
}

// 实现创建物品掉落的方法
void Game::createItemDrop(std::unique_ptr<Item> item, float x, float y) {
    if (!item) return;
    
    // 打印掉落信息
    std::cout << "Item dropped: " << item->getName() << " at position (" << x << ", " << y << ")" << std::endl;
    
    // 检查玩家是否在附近可以拾取物品
    if (player) {
        // 计算玩家与掉落物的距离
        float playerX = player->getX();
        float playerY = player->getY();
        float dx = playerX - x;
        float dy = playerY - y;
        float distanceSquared = dx * dx + dy * dy;
        
        // 如果玩家在拾取范围内（这里设置为100像素），自动拾取物品
        float pickupRangeSquared = 100.0f * 100.0f;
        if (distanceSquared <= pickupRangeSquared) {
            // 尝试将物品添加到玩家的存储空间
            if (player->addItem(std::move(item))) {
                std::cout << "Player picked up the item automatically." << std::endl;
                return; // 物品已被拾取，不需要进一步处理
            }
        }
    }
    
    // TODO: 如果需要实现物品掉落在地上并等待玩家拾取的功能，
    // 可以在这里创建一个ItemDrop类的实例，并将其添加到游戏世界中
    // 目前简单处理：如果玩家没有自动拾取，物品就消失
    std::cout << "Item disappeared as it was not picked up." << std::endl;
}

// 处理物品掉落
//void Game::handleItemDrop(std::unique_ptr<Item> item, float x, float y) {
//    // 检查玩家是否在附近（自动拾取范围内）
//    float dx = player->getX() - x;
//    float dy = player->getY() - y;
//    float distanceSquared = dx * dx + dy * dy;
//    
//    // 如果玩家在自动拾取范围内（这里设置为100像素），直接将物品添加到玩家背包
//    const float AUTOPICKUP_RANGE = 100.0f;
//    if (distanceSquared <= AUTOPICKUP_RANGE * AUTOPICKUP_RANGE) {
//        // 尝试将物品添加到玩家背包
//        if (player->addItem(std::move(item))) {
//            std::cout << "Item automatically picked up by player." << std::endl;
//            return;
//        }
//    }
//    
//    // 如果玩家不在范围内或背包已满，物品掉落在地上
//    // TODO: 实现物品掉落在地上的逻辑
//    // 可以在这里创建一个ItemDrop类的实例，并将其添加到游戏世界中
//    // 目前简单处理：如果玩家没有自动拾取，物品就消失
//    std::cout << "Item disappeared as it was not picked up." << std::endl;
//}

void Game::initItemSpawnClusters() {
    // 创建5.56x45 NATO子弹包集群
    ammoSpawnCluster = std::make_shared<ItemSpawnCluster>();
    ammoSpawnCluster->setQuantityRange(3, 20); // 设置生成3-20发子弹

    // 添加各种5.56x45子弹
    ammoSpawnCluster->addItem("5.56mm_M855", 1.0f);  // 普通弹
    ammoSpawnCluster->addItem("5.56mm_M855A1", 0.3f);   // 穿甲弹
    ammoSpawnCluster->addItem("5.56mm_MK262", 0.5f);   // 空尖弹
    ammoSpawnCluster->addItem("5.56mm_MK318", 0.2f); // 曳光弹
}

void Game::spawnItemsFromCluster(const std::shared_ptr<ItemSpawnCluster>& cluster) {
    if (!cluster) return;

    // 生成物品名称列表
    std::vector<std::string> itemNames = cluster->generateItems();

    // 通过ItemLoader创建实际的物品并添加到玩家背包
    for (const auto& itemName : itemNames) {
        std::unique_ptr<Item> item = nullptr;
        
        // 尝试不同的创建方法，优先使用专门的方法
        if (ItemLoader::getInstance()->hasAmmoTemplate(itemName)) {
            // 如果是弹药，使用createAmmo方法
            auto ammo = ItemLoader::getInstance()->createAmmo(itemName);
            if (ammo) {
                item = std::move(ammo);
            }
        } else if (ItemLoader::getInstance()->hasGunTemplate(itemName)) {
            // 如果是枪械，使用createGun方法
            auto gun = ItemLoader::getInstance()->createGun(itemName);
            if (gun) {
                item = std::move(gun);
            }
        } else if (ItemLoader::getInstance()->hasMagazineTemplate(itemName)) {
            // 如果是弹匣，使用createMagazine方法
            auto magazine = ItemLoader::getInstance()->createMagazine(itemName);
            if (magazine) {
                item = std::move(magazine);
            }
        } else if (ItemLoader::getInstance()->hasGunModTemplate(itemName)) {
            // 如果是枪械配件，使用createGunMod方法
            auto gunMod = ItemLoader::getInstance()->createGunMod(itemName);
            if (gunMod) {
                item = std::move(gunMod);
            }
        } else if (ItemLoader::getInstance()->hasWeaponTemplate(itemName)) {
            // 如果是武器，使用createWeapon方法
            auto weapon = ItemLoader::getInstance()->createWeapon(itemName);
            if (weapon) {
                item = std::move(weapon);
            }
        } else {
            // 使用通用的createItem方法作为备选
            item = ItemLoader::getInstance()->createItem(itemName);
        }
        
        if (item && player) {
            // 将物品添加到玩家的存储空间中
            player->addItem(std::move(item));
        } else {
            std::cout << "警告：无法创建物品 " << itemName << std::endl;
        }
    }
}

void Game::renderLaserEffect(SDL_Renderer* renderer, float mouseX, float mouseY, Gun* gun) {
    if (!gun->hasFlag(ItemFlag::LASER)) return;

    // 将鼠标坐标转换为世界坐标
    float worldMouseX = mouseX / zoomLevel + cameraX;
    float worldMouseY = mouseY / zoomLevel + cameraY;
    
    // 获取玩家位置
    int playerX = player->getX();
    int playerY = player->getY();
    
    // 计算方向向量
    float dirX = worldMouseX - playerX;
    float dirY = worldMouseY - playerY;
    float length = std::sqrt(dirX * dirX + dirY * dirY);
    if (length > 0) {
        dirX /= length;
        dirY /= length;
    }
    
    // 计算激光长度（考虑障碍物距离）
    float laserLength = std::min(1500.0f, pointerToObstacleDistance);
    
    // 计算激光终点
    float endX = playerX + dirX * laserLength;
    float endY = playerY + dirY * laserLength;
    
    // 转换为屏幕坐标
    int screenStartX = static_cast<int>((playerX - cameraX) * zoomLevel);
    int screenStartY = static_cast<int>((playerY - cameraY) * zoomLevel);
    int screenEndX = static_cast<int>((endX - cameraX) * zoomLevel);
    int screenEndY = static_cast<int>((endY - cameraY) * zoomLevel);
    
    // 固定渐变参数
    const float fadeStartDistance = 0.0f;    // 开始渐变的位置
    const float fadeEndDistance = 1000.0f;   // 完全消失的位置
    const float segmentLength = 10.0f;       // 每段长度（世界单位）
    
    // 渲染渐变激光效果
    float currentDistance = 0.0f;
    while (currentDistance < laserLength) {
        // 计算当前段的终点距离
        float nextDistance = currentDistance + segmentLength;
        if (nextDistance > laserLength) {
            nextDistance = laserLength;
        }
        
        // 计算当前段的起点和终点（世界坐标）
        float startX = playerX + dirX * currentDistance;
        float startY = playerY + dirY * currentDistance;
        float endX = playerX + dirX * nextDistance;
        float endY = playerY + dirY * nextDistance;
        
        // 转换为屏幕坐标
        int segStartX = static_cast<int>((startX - cameraX) * zoomLevel);
        int segStartY = static_cast<int>((startY - cameraY) * zoomLevel);
        int segEndX = static_cast<int>((endX - cameraX) * zoomLevel);
        int segEndY = static_cast<int>((endY - cameraY) * zoomLevel);
        
        // 计算当前段的透明度（基于距离）
        float alpha = 1.0f;
        if (currentDistance > fadeStartDistance) {
            alpha = 1.0f - (currentDistance - fadeStartDistance) / (fadeEndDistance - fadeStartDistance);
            alpha = std::max(0.0f, std::min(1.0f, alpha));
        }
        
        // 渲染当前段
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, static_cast<Uint8>(alpha * 255));
        SDL_RenderLine(renderer, segStartX, segStartY, segEndX, segEndY);
        
        // 更新距离
        currentDistance = nextDistance;
        
        // 如果已经到达激光终点，退出循环
        if (currentDistance >= laserLength) {
            break;
        }
    }
}

void Game::updateAndRenderCrosshair(SDL_Renderer* renderer, float mouseX, float mouseY, Gun* gun) {
    // 检查是否有UI打开
    if (!gameUI->isAnyUIOpen()) {
        // 渲染准星（十字）
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // 白色准星
        int crosshairSize = 10;
        // 水平线
        SDL_RenderLine(renderer, mouseX - crosshairSize, mouseY, mouseX + crosshairSize, mouseY);
        // 垂直线
        SDL_RenderLine(renderer, mouseX, mouseY - crosshairSize, mouseX, mouseY + crosshairSize);

        // 隐藏默认鼠标指针
        SDL_HideCursor();

        // 渲染激光效果
        renderLaserEffect(renderer, mouseX, mouseY, gun);
    } else {
        // 如果有UI打开，显示默认鼠标指针
        SDL_ShowCursor();
    }
}

// 创建远程玩家
void Game::createRemotePlayer(float x, float y) {
    // 创建远程玩家实体
    auto remotePlayer = std::make_unique<Player>(x, y);
    remotePlayer->setIsLocalPlayer(false);
    
    // 确保远程玩家的纹理已初始化
    remotePlayer->initializeTexture(renderer);
    
    // 创建远程玩家控制器
    auto remoteController = std::make_unique<RemotePlayerController>(remotePlayer.get(), 200.0f, 1.0f);
    remotePlayer->setController(remoteController.get());
    
    // 添加到容器
    remotePlayers.push_back(std::move(remotePlayer));
    remoteControllers.push_back(std::move(remoteController));
    
    std::cout << "Remote player created and added to game" << std::endl;
}

// 更新远程玩家
void Game::updateRemotePlayers(float deltaTime) {
    // 调整后的deltaTime
    float adjustedDeltaTime = deltaTime * timeScale;
    
    for (auto& controller : remoteControllers) {
        controller->update(adjustedDeltaTime);
    }
}

// 渲染远程玩家
void Game::renderRemotePlayers() {
    for (auto& remotePlayer : remotePlayers) {
        remotePlayer->render(renderer, cameraX, cameraY);
    }
}

// 生成丧尸
void Game::spawnZombie(float x, float y, ZombieType type) {
    // 检查生成位置是否与地形碰撞
    if (gameMap) {
        // 创建临时丧尸碰撞箱进行检测（丧尸通常是圆形碰撞箱，半径约15像素）
        Collider tempCollider(x, y, 15.0f, "temp_zombie", ColliderPurpose::ENTITY, 0);
        
        // 检查是否与地形碰撞
        int minTileX = static_cast<int>((x - 20) / 64);
        int maxTileX = static_cast<int>((x + 20) / 64);
        int minTileY = static_cast<int>((y - 20) / 64);
        int maxTileY = static_cast<int>((y + 20) / 64);
        
        bool hasCollision = false;
        for (int tileX = minTileX; tileX <= maxTileX && !hasCollision; tileX++) {
            for (int tileY = minTileY; tileY <= maxTileY && !hasCollision; tileY++) {
                Tile* tile = gameMap->getTileAt(tileX * 64, tileY * 64);
                if (tile && tile->hasColliderWithPurpose(ColliderPurpose::TERRAIN)) {
                    auto terrainColliders = tile->getCollidersByPurpose(ColliderPurpose::TERRAIN);
                    for (Collider* terrainCollider : terrainColliders) {
                        if (tempCollider.intersects(*terrainCollider)) {
                            hasCollision = true;
                            break;
                        }
                    }
                }
            }
        }
        
        if (hasCollision) {
            // std::cout << "警告：丧尸生成位置(" << x << ", " << y << ")与地形碰撞，已取消生成" << std::endl;
            return; // 不生成丧尸
        }
    }
    
    zombies.push_back(std::make_unique<Zombie>(x, y, type));
    // std::cout << "丧尸已生成在位置(" << x << ", " << y << ")" << std::endl;
}

// 更新所有丧尸
void Game::updateZombies(float deltaTime) {
    for (auto& zombie : zombies) {
        zombie->update(deltaTime);
    }
}

// 渲染所有丧尸
void Game::renderZombies() {
    for (auto& zombie : zombies) {
        zombie->render(renderer, cameraX, cameraY);
    }
}

// 更新所有生物
void Game::updateCreatures(float deltaTime) {
    // 调整为游戏倍率
    float adjustedDeltaTime = deltaTime * timeScale;
    
    // 更新每个生物
    for (auto& creature : creatures) {
        // 如果生物不是玩家阵营，处理与玩家的敌对关系
        if (creature->getFaction() == Faction::ENEMY && 
            creature->getCreatureType() == CreatureType::UNDEAD && 
            creature->getCurrentTarget() == nullptr) {
            
            // 如果玩家不是不死生物，将玩家设为目标
            // 这是一个简单的敌对策略实现：丧尸攻击任何非不死生物
            if (player && !player->hasFlag(EntityFlag::IS_ZOMBIE)) {
                // 计算与玩家的距离
                int dx = player->getX() - creature->getX();
                int dy = player->getY() - creature->getY();
                float distSq = dx * dx + dy * dy;
                
                // 检查是否在视觉范围内（1格 = 64像素）
                float visualRangePx = creature->getVisualRange() * 64.0f;
                
                if (distSq <= visualRangePx * visualRangePx) {
                    creature->setCurrentTarget(player.get());
                    creature->setState(CreatureState::HUNTING);
                }
            }
        }
        
        // 如果生物处于攻击状态，智能选择攻击方式
        if (creature->getState() == CreatureState::ATTACKING && 
            creature->getCreatureType() == CreatureType::UNDEAD) {
            
            Entity* target = creature->getCurrentTarget();
            if (target) {
                // 计算与目标的距离
                int dx = target->getX() - creature->getX();
                int dy = target->getY() - creature->getY();
                float dist = std::sqrt(dx * dx + dy * dy);
                
                // 根据距离选择不同的攻击方式
                if (dist < 32) { // 非常近距离
                    // 使用咬击攻击（索引2）
                    CreatureAttack* biteAttack = creature->getAttack(2);
                    if (biteAttack && biteAttack->canAttack()) {
                        creature->setCurrentAttack(2);
                    }
                } else if (dist < 40) { // 中等距离
                    // 使用爪击攻击（索引1）
                    CreatureAttack* clawAttack = creature->getAttack(1);
                    if (clawAttack && clawAttack->canAttack()) {
                        creature->setCurrentAttack(1);
                    }
                } else { // 较远距离
                    // 使用抓取攻击（索引0）
                    creature->setCurrentAttack(0);
                }
            }
        }
        
        // 更新生物
        creature->update(adjustedDeltaTime);
    }
}

// 渲染所有生物
void Game::renderCreatures() {
    for (auto& creature : creatures) {
        creature->render(renderer, cameraX, cameraY);
    }
}

// 寻路系统相关方法实现

void Game::initPathfinder() {
    if (gameMap) {
        pathfinder = std::make_unique<CreaturePathfinder>(gameMap.get());
        std::cout << "寻路系统初始化完成" << std::endl;
    } else {
        std::cerr << "无法初始化寻路系统：地图未创建" << std::endl;
    }
}

void Game::renderCreaturePaths() {
    if (!pathfinder || !debugMode) return;
    
    // 渲染所有生物的路径
    for (const auto& creature : creatures) {
        const std::vector<PathPoint>* path = pathfinder->getCreaturePath(creature.get());
        if (path && !path->empty()) {
            // 设置路径颜色（黄色）
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            
            // 绘制路径点之间的连线
            for (size_t i = 1; i < path->size(); ++i) {
                const PathPoint& prev = (*path)[i - 1];
                const PathPoint& current = (*path)[i];
                
                int screenX1 = static_cast<int>(prev.x - cameraX);
                int screenY1 = static_cast<int>(prev.y - cameraY);
                int screenX2 = static_cast<int>(current.x - cameraX);
                int screenY2 = static_cast<int>(current.y - cameraY);
                
                SDL_RenderLine(renderer, screenX1, screenY1, screenX2, screenY2);
            }
            
            // 绘制路径点
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 128);
            for (const PathPoint& point : *path) {
                int screenX = static_cast<int>(point.x - cameraX);
                int screenY = static_cast<int>(point.y - cameraY);
                
                SDL_FRect pointRect = {
                    static_cast<float>(screenX - 3),
                    static_cast<float>(screenY - 3),
                    6.0f, 6.0f
                };
                SDL_RenderFillRect(renderer, &pointRect);
            }
        }
    }
    
    // 渲染所有丧尸的路径
    for (const auto& zombie : zombies) {
        const std::vector<PathPoint>* path = pathfinder->getCreaturePath(zombie.get());
        if (path && !path->empty()) {
            // 设置路径颜色（红色）
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            
            // 绘制路径点之间的连线
            for (size_t i = 1; i < path->size(); ++i) {
                const PathPoint& prev = (*path)[i - 1];
                const PathPoint& current = (*path)[i];
                
                int screenX1 = static_cast<int>(prev.x - cameraX);
                int screenY1 = static_cast<int>(prev.y - cameraY);
                int screenX2 = static_cast<int>(current.x - cameraX);
                int screenY2 = static_cast<int>(current.y - cameraY);
                
                SDL_RenderLine(renderer, screenX1, screenY1, screenX2, screenY2);
            }
            
            // 绘制路径点
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 128);
            for (const PathPoint& point : *path) {
                int screenX = static_cast<int>(point.x - cameraX);
                int screenY = static_cast<int>(point.y - cameraY);
                
                SDL_FRect pointRect = {
                    static_cast<float>(screenX - 2),
                    static_cast<float>(screenY - 2),
                    4.0f, 4.0f
                };
                SDL_RenderFillRect(renderer, &pointRect);
            }
        }
    }
}

void Game::generateTestTerrain() {
    std::cout << "开始生成测试地形..." << std::endl;
    
    // 设置随机数生成器
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> tileDis(0, 1); // 0=test_brick, 1=test_hard
    std::uniform_int_distribution<> placeDis(0, 100); // 0-100概率
    
    // 在玩家右侧（x增大方向）生成测试地形
    // 覆盖范围：x: 128-1280 (20个格子), y: -640-640 (20个格子)
    int startX = 128;  // 玩家右侧2格开始
    int endX = 1280;   // 20格宽度
    int startY = -640; // 玩家上方10格
    int endY = 640;    // 玩家下方10格
    int tileSize = 64; // 每格64像素
    
    int testTileCount = 0;
    
    for (int worldX = startX; worldX < endX; worldX += tileSize) {
        for (int worldY = startY; worldY < endY; worldY += tileSize) {
            // 30%概率生成测试地形
            if (placeDis(gen) < 30) {
                int tileType = tileDis(gen);
                
                if (tileType == 0) {
                    // 生成test_brick - 不可通过的障碍物
                    auto testBrick = std::make_unique<Tile>(
                        "test_brick",
                        "assets/tiles/brick.bmp",
                        true,   // 有碰撞箱（不可通过）
                        true,   // 透明（不阻挡视线）- 修正
                        true,   // 可破坏
                        worldX, worldY,
                        tileSize,
                        100.0f  // 默认移动耗时（虽然不可通过）
                    );
                    
                    // 将brick tile添加到对应的网格中
                    int gridX, gridY;
                    Map::worldToGridCoord(worldX, worldY, gridX, gridY);
                    
                    Grid* grid = gameMap->getGridAtCoord(gridX, gridY);
                    if (!grid) {
                        // 如果网格不存在，创建一个新的草地网格
                        int gridWorldX, gridWorldY;
                        Map::gridCoordToWorld(gridX, gridY, gridWorldX, gridWorldY);
                        auto newGrid = Grid::createGrasslandGrid(gridWorldX, gridWorldY);
                        grid = newGrid.get();
                        gameMap->addGrid(std::move(newGrid), gridX, gridY);
                        grid->initializeTextures(renderer);
                    }
                    
                    // 计算在网格中的相对位置
                    int relativeX = (worldX - grid->getX()) / tileSize;
                    int relativeY = (worldY - grid->getY()) / tileSize;
                    
                    // 添加到网格中
                    grid->addTile(std::move(testBrick), relativeX, relativeY);
                    testTileCount++;
                    
                } else {
                    // 生成test_hard - 难以通过的地形
                    auto testHard = std::make_unique<Tile>(
                        "test_hard",
                        "assets/tiles/grassland2.bmp",
                        false,  // 无碰撞箱（可通过）
                        true,   // 透明（不阻挡视线）- 修正
                        false,  // 不可破坏
                        worldX, worldY,
                        tileSize,
                        500.0f  // 高移动耗时
                    );
                    
                    // 将hard tile添加到对应的网格中
                    int gridX, gridY;
                    Map::worldToGridCoord(worldX, worldY, gridX, gridY);
                    
                    Grid* grid = gameMap->getGridAtCoord(gridX, gridY);
                    if (!grid) {
                        // 如果网格不存在，创建一个新的草地网格
                        int gridWorldX, gridWorldY;
                        Map::gridCoordToWorld(gridX, gridY, gridWorldX, gridWorldY);
                        auto newGrid = Grid::createGrasslandGrid(gridWorldX, gridWorldY);
                        grid = newGrid.get();
                        gameMap->addGrid(std::move(newGrid), gridX, gridY);
                        grid->initializeTextures(renderer);
                    }
                    
                    // 计算在网格中的相对位置
                    int relativeX = (worldX - grid->getX()) / tileSize;
                    int relativeY = (worldY - grid->getY()) / tileSize;
                    
                    // 添加到网格中
                    grid->addTile(std::move(testHard), relativeX, relativeY);
                    testTileCount++;
                }
            }
        }
    }
    
    // 更新地图障碍物列表
    // 测试期间禁用动态地图更新
    // gameMap->update();
    
    std::cout << "测试地形生成完成！共生成了 " << testTileCount << " 个测试地形块。" << std::endl;
    std::cout << "  - test_brick (红色砖块): 不可通过的障碍物" << std::endl;
    std::cout << "  - test_hard (黄色方块): 移动耗时500点的困难地形" << std::endl;
    std::cout << "按F3键开启调试模式查看寻路路径！" << std::endl;
    std::cout << "\n=== 寻路系统测试说明 ===" << std::endl;
    std::cout << "智能程度设置：" << std::endl;
    std::cout << "  - 普通丧尸: 1.2倍（基础智能）" << std::endl;
    std::cout << "  - 奔跑者: 2.5倍（中等智能）" << std::endl;
    std::cout << "  - 臃肿者: 1.5倍（较低智能）" << std::endl;
    std::cout << "\n优化特性：" << std::endl;
    std::cout << "  ✅ 智能距离限制：距离 >= 智能度×直线距离 + (智能度-1)×8 时停止" << std::endl;
    std::cout << "  ✅ 无障碍检测：没有障碍物时直接走直线" << std::endl;
    std::cout << "  ✅ 部分路径：找不到完整路径时返回最接近的路径" << std::endl;
    std::cout << "  ✅ 直线回退：寻路失败时自动切换为直线移动" << std::endl;
    std::cout << "\n观察要点：" << std::endl;
    std::cout << "  🎯 丧尸应该绕过红色砖块" << std::endl;
    std::cout << "  🐌 丧尸在可能时避开黄色困难地形" << std::endl;
    std::cout << "  🧠 不同智能等级显示不同的寻路深度" << std::endl;
    std::cout << "  ⚡ 无障碍时应该直线移动（更快）" << std::endl;
}

// 物理系统：处理所有实体间的碰撞
void Game::processEntityPhysics() {
    // 收集所有实体
    std::vector<Entity*> allEntities;
    
    // 添加玩家
    if (player) {
        allEntities.push_back(player.get());
    }
    
    // 添加所有丧尸
    for (auto& zombie : zombies) {
        if (zombie && zombie->getHealth() > 0) {
            allEntities.push_back(zombie.get());
        }
    }
    
    // 添加所有生物
    for (auto& creature : creatures) {
        if (creature && creature->getHealth() > 0) {
            allEntities.push_back(creature.get());
        }
    }
    
    // 处理所有实体间的碰撞
    for (size_t i = 0; i < allEntities.size(); ++i) {
        for (size_t j = i + 1; j < allEntities.size(); ++j) {
            Entity* entity1 = allEntities[i];
            Entity* entity2 = allEntities[j];
            
            if (!entity1 || !entity2) continue;
            
            // 检查碰撞
            Entity::CollisionInfo info;
            if (entity1->checkCollisionWith(entity2, info)) {
                // 处理碰撞分离
                entity1->separateFromEntity(entity2, info);
            }
        }
    }
}

// 渲染攻击范围
void Game::renderAttackRange() {
    if (!player) return;
    
    Item* heldItem = player->getHeldItem();
    if (!heldItem || !heldItem->hasFlag(ItemFlag::MELEE)) {
        return; // 不是近战武器，不渲染攻击范围
    }
    
    // 检查是否是武器攻击接口
    IWeaponAttack* weaponAttack = dynamic_cast<IWeaponAttack*>(heldItem);
    if (!weaponAttack) {
        return;
    }
    
    // 检查玩家是否可以攻击
    if (!player->canAttack()) {
        return;
    }
    
    // 获取鼠标位置
    float mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    
    // 转换为世界坐标
    float worldMouseX = (mouseX / zoomLevel) + cameraX;
    float worldMouseY = (mouseY / zoomLevel) + cameraY;
    
    // 计算攻击方向
    float dx = worldMouseX - player->getX();
    float dy = worldMouseY - player->getY();
    float direction = std::atan2(dy, dx);
    
    // 获取主攻击参数（左键扇形）
    AttackParams primaryParams = weaponAttack->getAttackParams(WeaponAttackType::PRIMARY);
    primaryParams.direction = direction;
    
    // 获取副攻击参数（右键长条形）
    AttackParams secondaryParams = weaponAttack->getAttackParams(WeaponAttackType::SECONDARY);
    secondaryParams.direction = direction;
    
    // 获取玩家的攻击系统
    if (auto* attackSystem = player->getAttackSystem()) {
        // 设置混合模式
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        
        // 渲染动画攻击范围（使用白色渐变效果）
        // 主攻击范围（左键扇形）
        attackSystem->renderAnimatedAttackRange(renderer, primaryParams, cameraX, cameraY, animationTime);
        
        // 副攻击范围（右键长条形）- 稍微不同的动画相位
        attackSystem->renderAnimatedAttackRange(renderer, secondaryParams, cameraX, cameraY, animationTime + M_PI / 3);
        
        // 恢复默认混合模式
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }
}

// 伤害数字相关方法实现
void Game::addDamageNumber(float x, float y, int damage, bool critical) {
    damageNumbers.push_back(std::make_unique<DamageNumber>(x, y, damage, critical));
}

void Game::addDamageNumber(float x, float y, DamageNumberType type, int damage) {
    damageNumbers.push_back(std::make_unique<DamageNumber>(x, y, type, damage));
}

void Game::updateDamageNumbers() {
    // 更新所有伤害数字
    for (auto& damageNumber : damageNumbers) {
        damageNumber->update(getAdjustedDeltaTime());
    }
    
    // 移除生命周期结束的伤害数字
    damageNumbers.erase(
        std::remove_if(damageNumbers.begin(), damageNumbers.end(),
            [](const std::unique_ptr<DamageNumber>& number) {
                return number->shouldDestroy();
            }),
        damageNumbers.end()
    );
}

void Game::renderDamageNumbers() {
    // 渲染所有伤害数字
    for (const auto& damageNumber : damageNumbers) {
        damageNumber->render(renderer, cameraX, cameraY);
    }
}

// 受伤屏幕效果相关方法实现
void Game::triggerHurtEffect(float intensity) {
    hurtEffectIntensity = std::min(1.0f, intensity);
    hurtEffectTime = 0.8f; // 效果持续时间
}

void Game::updateHurtEffect() {
    if (hurtEffectTime > 0.0f) {
        hurtEffectTime -= getAdjustedDeltaTime();
        if (hurtEffectTime <= 0.0f) {
            hurtEffectTime = 0.0f;
            hurtEffectIntensity = 0.0f;
        } else {
            // 根据剩余时间调整强度（渐变效果）
            float normalizedTime = hurtEffectTime / 0.8f;
            hurtEffectIntensity = hurtEffectIntensity * normalizedTime;
        }
    }
}

void Game::renderHurtEffect() {
    if (hurtEffectIntensity <= 0.0f) {
        return;
    }
    
    // 设置混合模式用于透明度渲染
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    
    // 计算红色透明度
    Uint8 alpha = static_cast<Uint8>(hurtEffectIntensity * 150); // 最大150透明度，避免完全遮挡
    
    // 获取窗口尺寸
    int windowW, windowH;
    SDL_GetWindowSize(window, &windowW, &windowH);
    
    // 创建渐变效果 - 四边向内渐变
    const int gradientWidth = 150; // 渐变带的宽度
    
    // 顶边渐变
    for (int i = 0; i < gradientWidth; i++) {
        float gradientAlpha = (alpha * (gradientWidth - i)) / gradientWidth;
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, static_cast<Uint8>(gradientAlpha));
        SDL_FRect rect = {0, static_cast<float>(i), static_cast<float>(windowW), 1};
        SDL_RenderFillRect(renderer, &rect);
    }
    
    // 底边渐变
    for (int i = 0; i < gradientWidth; i++) {
        float gradientAlpha = (alpha * (gradientWidth - i)) / gradientWidth;
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, static_cast<Uint8>(gradientAlpha));
        SDL_FRect rect = {0, static_cast<float>(windowH - gradientWidth + i), static_cast<float>(windowW), 1};
        SDL_RenderFillRect(renderer, &rect);
    }
    
    // 左边渐变
    for (int i = 0; i < gradientWidth; i++) {
        float gradientAlpha = (alpha * (gradientWidth - i)) / gradientWidth;
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, static_cast<Uint8>(gradientAlpha));
        SDL_FRect rect = {static_cast<float>(i), static_cast<float>(gradientWidth), 1, static_cast<float>(windowH - 2 * gradientWidth)};
        SDL_RenderFillRect(renderer, &rect);
    }
    
    // 右边渐变
    for (int i = 0; i < gradientWidth; i++) {
        float gradientAlpha = (alpha * (gradientWidth - i)) / gradientWidth;
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, static_cast<Uint8>(gradientAlpha));
        SDL_FRect rect = {static_cast<float>(windowW - gradientWidth + i), static_cast<float>(gradientWidth), 1, static_cast<float>(windowH - 2 * gradientWidth)};
        SDL_RenderFillRect(renderer, &rect);
    }
    
    // 恢复默认混合模式
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

// 测试覆盖率系统
void Game::testCoverageSystem() {
    std::cout << "\n=== 装备覆盖率系统测试 ===" << std::endl;
    
    // 获取ItemLoader实例
    ItemLoader* loader = ItemLoader::getInstance();
    if (!loader) {
        std::cout << "错误：无法获取ItemLoader实例" << std::endl;
        return;
    }
    
    // 测试物品列表
    std::vector<std::string> testItems = {
        "连体作战服",
        "防弹衣", 
        "战术头盔"
    };
    
    for (const auto& itemName : testItems) {
        std::cout << "\n--- 测试物品: " << itemName << " ---" << std::endl;
        
        // 创建物品
        auto item = loader->createItem(itemName);
        if (!item) {
            std::cout << "错误：无法创建物品 " << itemName << std::endl;
            continue;
        }
        
        // 显示基本信息
        std::cout << "物品名称: " << item->getName() << std::endl;
        std::cout << "是否可穿戴: " << (item->hasFlag(ItemFlag::WEARABLE) ? "是" : "否") << std::endl;
        
        // 显示传统装备槽位
        const auto& equipSlots = item->getEquipSlots();
        std::cout << "传统装备槽位数量: " << equipSlots.size() << std::endl;
        for (const auto& slot : equipSlots) {
            std::cout << "  - 槽位: " << static_cast<int>(slot) << std::endl;
        }
        
        // 显示覆盖率信息
        const auto& coverageSlots = item->getCoverageSlots();
        std::cout << "覆盖率槽位数量: " << coverageSlots.size() << std::endl;
        
        if (!coverageSlots.empty()) {
            std::cout << "覆盖率详情:" << std::endl;
            for (const auto& coverage : coverageSlots) {
                std::string slotName;
                switch (coverage.slot) {
                    case EquipSlot::HEAD: slotName = "头部"; break;
                    case EquipSlot::CHEST: slotName = "胸部"; break;
                    case EquipSlot::ABDOMEN: slotName = "腹部"; break;
                    case EquipSlot::LEFT_LEG: slotName = "左腿"; break;
                    case EquipSlot::RIGHT_LEG: slotName = "右腿"; break;
                    case EquipSlot::LEFT_ARM: slotName = "左臂"; break;
                    case EquipSlot::RIGHT_ARM: slotName = "右臂"; break;
                    default: slotName = "其他"; break;
                }
                std::cout << "  - " << slotName << ": " << coverage.coverage << "%" << std::endl;
            }
        } else {
            std::cout << "  无覆盖率信息" << std::endl;
        }
        
        // 测试覆盖率查询
        std::vector<EquipSlot> testSlots = {
            EquipSlot::HEAD, EquipSlot::CHEST, EquipSlot::ABDOMEN,
            EquipSlot::LEFT_LEG, EquipSlot::RIGHT_LEG
        };
        
        std::cout << "覆盖率查询测试:" << std::endl;
        for (const auto& slot : testSlots) {
            int coverage = item->getCoverage(slot);
            std::string slotName;
            switch (slot) {
                case EquipSlot::HEAD: slotName = "头部"; break;
                case EquipSlot::CHEST: slotName = "胸部"; break;
                case EquipSlot::ABDOMEN: slotName = "腹部"; break;
                case EquipSlot::LEFT_LEG: slotName = "左腿"; break;
                case EquipSlot::RIGHT_LEG: slotName = "右腿"; break;
                default: slotName = "其他"; break;
            }
            std::cout << "  " << slotName << " 覆盖率: " << coverage << "%" << std::endl;
        }
    }
    
    std::cout << "\n=== 测试完成 ===" << std::endl;
}

// 添加一个新的测试方法来验证背包中的弹药
void Game::testAmmoInInventory() {
    std::cout << "\n=== 背包弹药测试 ===" << std::endl;
    
    if (!player) {
        std::cout << "错误：玩家不存在" << std::endl;
        return;
    }
    
    // 获取玩家的所有存储空间
    auto storages = player->getAllAvailableStorages();
    if (storages.empty()) {
        std::cout << "错误：玩家没有任何存储空间" << std::endl;
        return;
    }
    
    std::cout << "检查玩家的所有存储空间..." << std::endl;
    std::cout << "玩家共有 " << storages.size() << " 个存储空间" << std::endl;
    
    int ammoCount = 0;
    int totalItemCount = 0;
    
    for (const auto& [slot, storage] : storages) {
        std::cout << "\n检查存储空间: " << storage->getName() << std::endl;
        size_t itemCount = storage->getItemCount();
        totalItemCount += itemCount;
        std::cout << "  物品数量: " << itemCount << std::endl;
        
        for (size_t i = 0; i < itemCount; ++i) {
            Item* item = storage->getItem(i);
            if (item && item->hasFlag(ItemFlag::AMMO)) {
                ammoCount++;
                std::cout << "\n发现弹药: " << item->getName() << std::endl;
                
                // 尝试转换为Ammo类型
                Ammo* ammo = dynamic_cast<Ammo*>(item);
                if (ammo) {
                    std::cout << "  转换成功 - 类型: " << ammo->getAmmoType() << std::endl;
                    std::cout << "  伤害: " << ammo->getBaseDamage() << std::endl;
                    std::cout << "  穿透力: " << ammo->getBasePenetration() << std::endl;
                } else {
                    std::cout << "  转换失败 - 物品标记为弹药但无法转换为Ammo类型" << std::endl;
                }
            }
        }
    }
    
    std::cout << "\n总共检查了 " << totalItemCount << " 个物品，找到 " << ammoCount << " 个弹药物品" << std::endl;
    std::cout << "=== 弹药测试完成 ===" << std::endl;
}