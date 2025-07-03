#pragma once
#ifndef ACTION_H
#define ACTION_H

#include <string>
#include <memory>
#include <functional>
#include "EntityState.h"
#include "Item.h"

// 前向声明
class Entity;
class Gun;
class Magazine;
class Item;
class Storage;
class Ammo;

// 行为基类
class Action {
protected:
    Entity* owner;            // 行为的执行者
    float duration;           // 行为持续时间
    EntityState actionState;  // 行为对应的状态
    std::string soundId;      // 行为音效ID
    bool isCompleted;         // 行为是否已完成
    bool isStarted;           // 行为是否已开始

public:
    Action(Entity* entity, float actionDuration, EntityState state, const std::string& sound = "");
    virtual ~Action() = default;

    // 开始行为
    virtual void start();
    
    // 更新行为
    virtual void update(float deltaTime);
    
    // 结束行为
    virtual void end();
    
    // 中断行为
    virtual void interrupt();
    
    // 获取行为状态
    bool isActionCompleted() const { return isCompleted; }
    bool isActionStarted() const { return isStarted; }
    
    // 获取行为持续时间
    float getDuration() const { return duration; }
    
    // 获取行为状态
    EntityState getActionState() const { return actionState; }
};

// 卸下弹匣行为
class UnloadMagazineAction : public Action {
private:
    Gun* weapon;
    std::function<void(std::unique_ptr<Magazine>)> onMagazineUnloaded; // 卸下弹匣后的回调

public:
    UnloadMagazineAction(Entity* entity, Gun* gun, Storage* storage = nullptr, std::function<void(std::unique_ptr<Magazine>)> callback = nullptr);
    
    void start() override;
    void end() override;
};

// 装填弹匣行为
class LoadMagazineAction : public Action {
private:
    Gun* weapon;
    std::unique_ptr<Magazine> magazine;

public:
    LoadMagazineAction(Entity* entity, Gun* gun, std::unique_ptr<Magazine> mag, Storage* storage = nullptr);
    
    void start() override;
    void end() override;
};

// 上膛行为
class ChamberRoundAction : public Action {
private:
    Gun* weapon;
    bool wasEmpty;

public:
    ChamberRoundAction(Entity* entity, Gun* gun);
    
    void start() override;
    void end() override;
};

// 存储物品行为
class StoreItemAction : public Action {
private:
    std::unique_ptr<Item> item;    // 要存储的物品
    Storage* targetStorage;        // 目标存储空间

public:
    StoreItemAction(Entity* entity, std::unique_ptr<Item> itemToStore, Storage* storage);

    void start() override;
    void end() override;
};

// 手持物品行为
class HoldItemAction : public Action {
private:
    Item* item;                    // 要手持的物品
    Storage* sourceStorage;        // 源存储空间
    std::function<void(std::unique_ptr<Item>)> onItemHeld;  // 物品手持后的回调

public:
    HoldItemAction(Entity* entity, Item* itemToHold, Storage* storage, std::function<void(std::unique_ptr<Item>)> callback = nullptr);
    
    void start() override;
    void end() override;
};

// 取出物品行为
class TakeItemAction : public Action {
private:
    Item* item;                    // 要取出的物品
    Storage* sourceStorage;        // 源存储空间
    std::function<void(std::unique_ptr<Item>)> onItemTaken; // 取出物品后的回调

public:
    TakeItemAction(Entity* entity, Item* itemToTake, Storage* storage, std::function<void(std::unique_ptr<Item>)> callback = nullptr);
    
    void start() override;
    void end() override;
};

// 穿戴物品行为
class EquipItemAction : public Action {
private:
    std::unique_ptr<Item> item;    // 要穿戴的物品

public:
    EquipItemAction(Entity* entity, std::unique_ptr<Item> itemToEquip);
    
    void start() override;
    void end() override;
};

// 卸下装备行为 - 按槽位卸下
class UnequipItemAction : public Action {
private:
    EquipSlot slot;                // 要卸下的装备槽位
    std::function<void(std::unique_ptr<Item>)> onItemUnequipped; // 卸下装备后的回调

public:
    UnequipItemAction(Entity* entity, EquipSlot equipSlot, std::function<void(std::unique_ptr<Item>)> callback = nullptr);
    
    void start() override;
    void end() override;
};

// 卸下装备行为 - 按物品卸下
class UnequipItemByItemAction : public Action {
private:
    Item* targetItem;              // 要卸下的物品
    std::function<void(std::unique_ptr<Item>)> onItemUnequipped; // 卸下装备后的回调

public:
    UnequipItemByItemAction(Entity* entity, Item* item, std::function<void(std::unique_ptr<Item>)> callback = nullptr);
    
    void start() override;
    void end() override;
};

// 添加物品转移行为类
class TransferItemAction : public Action {
private:
    Item* item;                    // 要转移的物品
    Storage* sourceStorage;        // 源存储空间
    Storage* targetStorage;        // 目标存储空间
    std::function<void(bool)> onTransferComplete; // 转移完成后的回调，参数表示是否成功

public:
    TransferItemAction(Entity* entity, Item* itemToTransfer, Storage* source, Storage* target, std::function<void(bool)> callback = nullptr);
    
    void start() override;
    void end() override;
};

// 卸除一发子弹行为
class UnloadSingleAmmoAction : public Action {
private:
    Magazine* magazine;            // 要卸除子弹的弹匣
    Storage* targetStorage;        // 目标存储空间
    std::function<void(std::unique_ptr<Ammo>)> onAmmoUnloaded; // 卸除子弹后的回调

public:
    UnloadSingleAmmoAction(Entity* entity, Magazine* mag, Storage* storage, std::function<void(std::unique_ptr<Ammo>)> callback = nullptr);
    
    void start() override;
    void end() override;
};

// 装填一发子弹行为
class LoadSingleAmmoAction : public Action {
private:
    Magazine* magazine;            // 要装填子弹的弹匣
    Ammo* ammo;                    // 要装填的子弹
    Storage* sourceStorage;        // 源存储空间
    std::function<void(bool)> onAmmoLoaded; // 装填完成后的回调，参数表示是否成功

public:
    LoadSingleAmmoAction(Entity* entity, Magazine* mag, Ammo* ammoToLoad, Storage* storage, std::function<void(bool)> callback = nullptr);
    
    void start() override;
    void end() override;
};

#endif // ACTION_H