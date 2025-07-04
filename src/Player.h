#pragma once
#ifndef PLAYER_H
#define PLAYER_H

#include "Creature.h"  // 改为包含Creature.h而不是Entity.h
#include "Item.h" // 包含 Item.h
#include <memory>   // 包含 memory 以使用 std::unique_ptr
#include <functional> // 包含 functional 以使用 std::function
#include <SDL3/SDL.h> // 添加SDL头文件以使用SDL_Texture
#include <string>
#include "PlayerStateManager.h" // 添加PlayerStateManager头文件
#include "AttackSystem.h" // 添加AttackSystem头文件
#include "SkillSystem.h" // 添加技能系统头文件

// 前向声明
class PlayerController; // 新增：前向声明PlayerController

class Player : public Creature {  // 改为继承Creature
    /*
    ======================================================================= 
    重要提醒：修改Player类成员变量时的必要维护清单
    ======================================================================= 
    当您添加/修改/删除任何成员变量时，请确保同时更新以下位置：
    
    必须更新的位置：
    1. 【构造函数】Player.cpp中的Player::Player() - 在初始化列表和函数体中初始化
    2. 【析构函数】Player.cpp中的~Player() - 如果需要特殊清理
    3. 【拷贝构造函数】如果实现了拷贝构造（目前可能没有）
    4. 【移动构造函数】如果实现了移动构造（目前可能没有）
    5. 【render方法】Player.cpp中的render() - 如果影响渲染
    6. 【update方法】Player.cpp中的update() - 如果需要每帧更新
    
    可能需要更新的位置：
    7. 【getter/setter】为新成员添加访问方法
    8. 【状态管理】PlayerStateManager相关逻辑
    9. 【网络同步】setPosition、网络相关方法
    10. 【伤害系统】takeDamage、身体部位相关方法
    11. 【装备系统】EquipmentSystem相关逻辑
    12. 【技能系统】SkillSystem相关逻辑
    13. 【序列化】如果有保存/加载功能
    14. 【UI显示】GameUI中的玩家信息显示
    
    特别注意：
    - 身体部位血量变量组：如果添加新的身体部位，需要更新所有身体部位相关方法
    - 智能指针成员：确保在构造函数中正确初始化（make_unique）
    - 原始指针成员：确保在析构函数中适当处理，避免悬空指针
    ======================================================================= 
    */
private:
    // 新增：角色四大基础属性
    int STR;                       // 力量 - 影响近战伤害和负重
    int AGI;                       // 敏捷 - 影响移动速度和闪避
    int INT;                       // 智力 - 影响技能和魔法
    int PER;                       // 感知 - 影响探测和射击精度
    
    // 身体部位血量系统
    int headHealth;                // 头部血量 (50)
    int torsoHealth;               // 躯干血量 (120)
    int leftLegHealth;             // 左腿血量 (90)
    int rightLegHealth;            // 右腿血量 (90)
    int leftArmHealth;             // 左臂血量 (75)
    int rightArmHealth;            // 右臂血量 (75)
    
    // 身体部位最大血量（常量）
    static const int MAX_HEAD_HEALTH = 50;
    static const int MAX_TORSO_HEALTH = 120;
    static const int MAX_LEG_HEALTH = 90;
    static const int MAX_ARM_HEALTH = 75;
    
    int mouseX, mouseY;        // 鼠标位置（世界坐标）
    int screenMouseX, screenMouseY; // 鼠标位置（屏幕坐标）
    // 添加以下变量以替换现有的鼠标坐标变量
    int mouseWorldX, mouseWorldY;   // 鼠标位置（世界坐标）
    int mouseScreenX, mouseScreenY; // 鼠标位置（屏幕坐标）
    std::unique_ptr<Item> heldItem; // 玩家手持的物品
    // 如果需要，可以专门为枪械创建一个指针，以方便访问 Gun 特有的方法
    // Gun* equippedGun; 
    
    // 添加射击相关变量
    bool isMouseLeftDown;      // 鼠标左键是否按下
    // 射击冷却计时器已迁移到Entity类中
    // std::unique_ptr<Inventory> inventory; // <--- 移除这一行
    
    // 添加纹理相关变量
    SDL_Texture* playerTexture; // 玩家贴图
    bool textureInitialized;    // 纹理是否已初始化
    
    // 新增：网络相关
    int playerId;              // 玩家唯一ID
    std::string playerName;    // 玩家名称
    bool isLocalPlayer;        // 是否为本地玩家
    PlayerController* controller; // 关联的控制器（不拥有）
    
    // 新增：状态管理器
    std::unique_ptr<PlayerStateManager> stateManager;
    
    // 新增：攻击系统
    std::unique_ptr<AttackSystem> attackSystem;
    
    // 新增：技能系统
    std::unique_ptr<SkillSystem> skillSystem;
    
    // 闪避系统
    int dodgeCount;                // 当前3秒内的闪避次数
    int lastDodgeResetTime;        // 上次重置闪避次数的时间（毫秒）

public:
    Player(float startX, float startY);
    ~Player(); // 析构函数，用于清理纹理资源
    
    // 处理输入
    void handleInput(const bool* keyState, float deltaTime = 1.0f / 60.0f);
    
    // 处理鼠标移动
    void handleMouseMotion(int x, int y, float cameraX, float cameraY);
    
    // 射击方法 - 简化为调用Entity的接口
    void attemptShoot();
    
    // 通用攻击方法
    void attemptAttack();
    void attemptAttack(WeaponAttackType type);
    bool canAttack() const;
    Entity* findAttackTarget(float range) const;
    
    // 处理鼠标点击
    void handleMouseClick(int button);
    
    // 处理鼠标释放
    void handleMouseRelease(int button);
    
    // 自动换弹方法
    void attemptReload();
    
    // 重写更新方法
    void update(float deltaTime = 1.0f / 60.0f) override;
    
    // =======================================================================
    // 手持物品系统（武器等）- 与装备系统完全分离
    // 注意：手持物品只能有一个，存储在heldItem中
    // 维护提醒：如果修改heldItem相关成员，需要同步更新：
    // - 构造函数初始化列表
    // - 拷贝构造函数（如果存在）
    // - 析构函数清理逻辑  
    // - render()方法中的手持物品渲染
    // - 相关getter/setter方法
    // =======================================================================
    void holdItem(std::unique_ptr<Item> item);           // 手持物品（替代equipItem）
    void releaseHeldItem();                              // 放下手持物品
    Item* getHeldItem() const;                           // 获取当前手持物品
    void useHeldItem();                                  // 使用手持物品
    bool holdItemFromStorage(Item* item, Storage* storage); // 从存储空间中取出物品并手持
    
    // =======================================================================
    // 装备系统（护甲、手套、头盔等穿戴装备）- 与手持系统完全分离
    // 注意：装备系统基于EquipSlot枚举，每个部位可以穿戴不同装备
    // 维护提醒：如果添加新的装备槽位或装备相关成员，需要同步更新：
    // - EquipSlot枚举定义
    // - EquipmentSystem类的槽位处理逻辑
    // - 装备渲染逻辑
    // - 装备属性加成计算
    // - 序列化/反序列化逻辑
    // =======================================================================
    void wearEquipment(std::unique_ptr<Item> equipment, EquipSlot slot);  // 穿戴装备到指定部位
    void unwearEquipment(EquipSlot slot, std::function<void(std::unique_ptr<Item>)> callback = nullptr); // 卸下装备
    Item* getEquippedItem(EquipSlot slot) const;         // 获取指定部位的装备
    
    // 向后兼容的废弃方法（避免破坏现有代码）
    [[deprecated("使用holdItem()替代，该方法容易混淆手持和装备概念")]]
    void equipItem(std::unique_ptr<Item> item) { holdItem(std::move(item)); }
    
    [[deprecated("使用unwearEquipment()替代，保持命名一致性")]]  
    void unequipItem(EquipSlot slot, std::function<void(std::unique_ptr<Item>)> callback = nullptr) {
        unwearEquipment(slot, std::move(callback));
    }
    
    // 重写渲染方法以渲染手持物品
    void render(SDL_Renderer* renderer, float cameraX, float cameraY) override;
    
    // 纹理初始化方法
    bool initializeTexture(SDL_Renderer* renderer);
    
    // 自动换弹方法
    void reloadCurrentWeapon(bool needChamber = true);
    
    // 物品转移方法
    bool transferItem(Item* item, Storage* sourceStorage, Storage* targetStorage, std::function<void(bool)> callback = nullptr);
    
    // 获取当前交互的实体
    Entity* getInteractingEntity() const;
    
    // 新增：网络相关方法
    void setPlayerId(int id) { playerId = id; }
    int getPlayerId() const { return playerId; }
    
    void setPlayerName(const std::string& name) { playerName = name; }
    const std::string& getPlayerName() const { return playerName; }
    
    void setIsLocalPlayer(bool local) { isLocalPlayer = local; }
    bool getIsLocalPlayer() const { return isLocalPlayer; }
    
    void setController(PlayerController* ctrl) { controller = ctrl; }
    PlayerController* getController() const { return controller; }
    
    // 新增：设置位置方法
    void setPosition(float newX, float newY);
    
    // 新增：状态管理器相关方法
    PlayerStateManager* getStateManager() const { return stateManager.get(); }
    
    // 新增：攻击系统相关方法
    AttackSystem* getAttackSystem() const { return attackSystem.get(); }
    
    // 新增：状态管理快捷方法
    EntityStateEffect* addPlayerState(
        EntityStateEffect::Type type,
        const std::string& name,
        int duration = -1,
        int priority = 0
    );
    
    bool removePlayerState(const std::string& name);
    bool removePlayerState(EntityStateEffect::Type type);
    
    bool hasPlayerState(const std::string& name) const;
    bool hasPlayerState(EntityStateEffect::Type type) const;
    
    // 新增：四大基础属性getter和setter
    int getSTR() const { return STR; }
    void setSTR(int value) { STR = std::max(1, std::min(100, value)); }
    
    int getAGI() const { return AGI; }
    void setAGI(int value) { AGI = std::max(1, std::min(100, value)); }
    
    int getINT() const { return INT; }
    void setINT(int value) { INT = std::max(1, std::min(100, value)); }
    
    int getPER() const { return PER; }
    void setPER(int value) { PER = std::max(1, std::min(100, value)); }
    
    // 新增：属性修改方法
    void modifySTR(int delta) { setSTR(STR + delta); }
    void modifyAGI(int delta) { setAGI(AGI + delta); }
    void modifyINT(int delta) { setINT(INT + delta); }
    void modifyPER(int delta) { setPER(PER + delta); }
    
    // 新增：技能系统相关方法
    SkillSystem* getSkillSystem() const { return skillSystem.get(); }
    
    // 技能经验值管理
    void addSkillExperience(SkillType skillType, int experience);
    int getSkillLevel(SkillType skillType) const;
    int getTotalSkillExperience(SkillType skillType) const;
    int getCurrentLevelSkillExperience(SkillType skillType) const;
    int getExpToNextSkillLevel(SkillType skillType) const;
    
    // 便捷方法：根据武器类型获得经验
    void gainWeaponExperience(const std::string& weaponType, int baseExp = 1);
    void gainMeleeExperience(const std::string& meleeType, int baseExp = 1);
    
    // 身体部位伤害系统
    enum class BodyPart {
        HEAD,
        TORSO, 
        LEFT_LEG,
        RIGHT_LEG,
        LEFT_ARM,
        RIGHT_ARM
    };
    
    // 身体部位伤害方法
    void takeDamageToBodyPart(int damage, BodyPart part);
    BodyPart selectRandomBodyPart() const;
    
    // 身体部位血量getter
    int getHeadHealth() const { return headHealth; }
    int getTorsoHealth() const { return torsoHealth; }
    int getLeftLegHealth() const { return leftLegHealth; }
    int getRightLegHealth() const { return rightLegHealth; }
    int getLeftArmHealth() const { return leftArmHealth; }
    int getRightArmHealth() const { return rightArmHealth; }
    
    // 获取身体部位最大血量
    static int getMaxHealthForBodyPart(BodyPart part);
    
    // 检查是否死亡
    bool isDeadFromBodyDamage() const;
    
    // 重写基类的takeDamage方法，使用身体部位伤害
    bool takeDamage(const Damage& damage) override;
    
    // 闪避系统方法
    bool attemptDodge(int attackerDexterity);  // 尝试闪避攻击
    void updateDodgeCount();                   // 更新闪避次数（每3秒重置）
    int getMaxDodgesPerWindow() const;         // 获取每3秒最大闪避次数
    int getCurrentDodgeCount() const { return dodgeCount; }
    
    // 近战攻击系统
    bool performMeleeAttack(Creature* target);  // 执行近战攻击
    int getHighestDamageSkillLevel(const Damage& damage) const;  // 获取最高伤害对应的技能等级
    SkillType getSkillTypeFromDamage(const Damage& damage) const;  // 根据伤害类型获取技能类型
};

#endif // PLAYER_H