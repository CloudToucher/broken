// 在 Game.h 中
#pragma once
#ifndef GAME_H
#define GAME_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <memory>
#include <iostream>
#include <vector>
#include <unordered_set>
#include "SoundManager.h"
#include "Tile.h" // 添加Tile.h以使用clearTextureCache方法
#include "Bullet.h" // 确保包含Bullet.h
#include "GameUI.h" // 添加GameUI.h
#include "ItemSpawnCluster.h" // 添加新的头文件
#include "PlayerController.h" // 添加PlayerController头文件
#include "RemotePlayerController.h" // 添加RemotePlayerController头文件
#include "Creature.h" // 添加Creature头文件
#include "Zombie.h" // 添加Zombie头文件
#include "Pathfinding.h" // 添加寻路系统头文件
#include "DamageNumber.h" // 添加伤害数字头文件
#include "EventManager.h" // 添加事件管理器头文件
#include "Fragment.h" // 添加弹片系统头文件

// 前向声明
class Player;
class Map;
class HUD;
class CreaturePathfinder;

class Game {
private:
    static Game* instance;

    // SDL ??  
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;

    // ????  
    std::unique_ptr<Player> player;
    std::unique_ptr<Map> gameMap;
    std::unique_ptr<HUD> hud;

    // ????  
    float cameraX;
    float cameraY;

    // ????  
    int windowWidth;
    int windowHeight;

    // ????????????  
    Game();
    std::vector<std::unique_ptr<Creature>> creatures; // 生物容器
    std::vector<std::unique_ptr<Zombie>> zombies; // 丧尸容器
    TTF_Font* font;            // 添加字体指针
    int frameCount;            // 帧数计数器
    int fps;                   // 当前FPS
    Uint64 fpsLastTime;        // 上次更新FPS的时间
    float deltaTime;           // 每帧的时间间隔（秒）

    // 添加缩放相关变量
    float zoomLevel;
    const float MIN_ZOOM;
    const float MAX_ZOOM;
    
    // 添加游戏倍率相关变量
    float timeScale;
    const float MIN_TIME_SCALE;
    const float MAX_TIME_SCALE;
    
    // 添加子弹管理容器
    std::vector<std::unique_ptr<Bullet>> bullets; // 游戏管理的所有子弹

    // 添加调试模式变量
    bool debugMode;
    
    // 添加游戏UI
    std::unique_ptr<GameUI> gameUI;

    // 添加物品刷新集群
    std::shared_ptr<ItemSpawnCluster> ammoSpawnCluster;

    // 添加指针方向到障碍物距离
    float pointerToObstacleDistance;

    // 添加远程玩家相关成员
    std::vector<std::unique_ptr<Player>> remotePlayers;
    std::vector<std::unique_ptr<PlayerController>> remoteControllers;

    // 添加寻路管理器
    std::unique_ptr<CreaturePathfinder> pathfinder;

    // 添加伤害数字管理
    std::vector<std::unique_ptr<DamageNumber>> damageNumbers; // 所有伤害数字
    
    // 受伤屏幕效果
    float hurtEffectIntensity; // 受伤效果强度 (0.0-1.0)
    float hurtEffectTime;      // 受伤效果持续时间

    // 渲染激光效果
    void renderLaserEffect(SDL_Renderer* renderer, float mouseX, float mouseY, Gun* gun);
    
    // 更新和渲染准星状态
    void updateAndRenderCrosshair(SDL_Renderer* renderer, float mouseX, float mouseY, Gun* gun);

    // 物理系统相关方法
    void processEntityPhysics();

public:
    // ??????  
    static Game* getInstance();

    // ?????  
    bool init();

    // ????  
    void handleEvents();

    // ??????  
    void update();

    // ????  
    void render();

    // ????  
    void clean();

    // ?????  
    void run();

    // ?????????  
    bool isRunning() const { return running; }

    // ?????  
    SDL_Renderer* getRenderer() const { return renderer; }

    // ??????  
    int getWindowWidth() const { return windowWidth; }
    int getWindowHeight() const { return windowHeight; }

    // ??????  
    float getCameraX() const { return cameraX; }
    float getCameraY() const { return cameraY; }

    // ??????  
    void setCamera(float x, float y);

    // 生物相关方法
    void spawnZombie(float x, float y, ZombieType type = ZombieType::NORMAL); // 生成丧尸
    void updateCreatures(float deltaTime); // 更新所有生物
    void renderCreatures(); // 渲染所有生物
    void updateZombies(float deltaTime); // 更新所有丧尸
    void renderZombies(); // 渲染所有丧尸

    // 修改子弹处理方法
    void processBullets();
    
    // 添加创建子弹的方法
    Bullet* createBullet(float startX, float startY, float dirX, float dirY, float speed, 
                        Entity* owner, int damageValue, const std::string& damageType = "shooting", int penetration = -1, float range = 1000.0f);
    
    // 添加创建物品掉落的方法
    void createItemDrop(std::unique_ptr<Item> item, float x, float y);
    
    // 添加处理物品掉落的方法
    void handleItemDrop(std::unique_ptr<Item> item, float x, float y);
    
    // 添加渲染所有子弹的方法
    void renderBullets();

    // 添加获取生物列表的方法
    const std::vector<std::unique_ptr<Creature>>& getCreatures() const { return creatures; }
    
    // 添加获取丧尸列表的方法
    const std::vector<std::unique_ptr<Zombie>>& getZombies() const { return zombies; }
    
    // 在Game类的public部分添加以下方法
    // SoundManager* getSoundManager() const { return soundManager.get(); } // <--- 如果不使用单例则添加
    Map* getMap() const { return gameMap.get(); }
    
    // 添加缩放相关方法
    void adjustZoomLevel(float change);
    float getZoomLevel() const { return zoomLevel; }
    
    // 添加游戏倍率相关方法
    void adjustTimeScale(float change);
    void setTimeScale(float scale);
    float getTimeScale() const { return timeScale; }
    float getAdjustedDeltaTime() const { return deltaTime * timeScale; }
    
    // 获取玩家指针
    Player* getPlayer() const { return player.get(); }

    // 调试模式相关方法
    void toggleDebugMode() { debugMode = !debugMode; }
    bool isDebugMode() const { return debugMode; }
    void renderColliders(); // 渲染所有碰撞箱
    
    // 游戏UI相关方法
    GameUI* getGameUI() const { return gameUI.get(); }
    void togglePlayerUI(); // 切换玩家背包界面显示状态

    // 添加初始化物品刷新集群的方法
    void initItemSpawnClusters();
    
    // 添加生成物品的方法
    void spawnItemsFromCluster(const std::shared_ptr<ItemSpawnCluster>& cluster);

    // 添加远程玩家相关方法
    void createRemotePlayer(float x, float y);
    void updateRemotePlayers(float deltaTime);
    void renderRemotePlayers();
    
    // 添加寻路管理器相关方法
    CreaturePathfinder* getPathfinder() const { return pathfinder.get(); }
    void initPathfinder(); // 初始化寻路系统
    
    // 调试：渲染生物路径
    void renderCreaturePaths();
    
    // 渲染攻击范围
    void renderAttackRange();
    
    // 动画时间跟踪（新增）
    float animationTime;
    
    // 生成测试地形来验证寻路系统
    void generateTestTerrain();
    
    // 伤害数字相关方法
    void addDamageNumber(float x, float y, int damage, bool critical = false);
    void addDamageNumber(float x, float y, DamageNumberType type, int damage = 0);
    void updateDamageNumbers();
    void renderDamageNumbers();
    
    // 受伤屏幕效果相关方法
    void triggerHurtEffect(float intensity = 0.8f);
    void updateHurtEffect();
    void renderHurtEffect();
    
    // 测试覆盖率系统
    void testCoverageSystem();
    
    // 测试背包中的弹药
    void testAmmoInInventory();
    
    // 测试堆叠系统
    void testStackingSystem();
    
    // 测试物品切换功能（把HK416放到背包，创建MDX和枪管）
    void testItemSwitch();
    
    // 爆炸系统
    void triggerExplosionAtMouse();
};

#endif // GAME_H
