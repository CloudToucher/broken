#pragma once
#ifndef DRAG_DROP_SYSTEM_H
#define DRAG_DROP_SYSTEM_H

#include <memory>
#include <functional>
#include <string>

// 前置声明
class Item;
class Storage;
class Player;

// 拖放源类型枚举
enum class DragSourceType {
    STORAGE,        // 存储空间（背包等）
    EQUIPMENT_SLOT, // 装备槽位
    HELD_ITEM,      // 手持物品
    WORLD_ITEM,     // 世界中的物品（地上的物品）
    UNKNOWN         // 未知源
};

// 拖放目标类型枚举
enum class DragTargetType {
    STORAGE,        // 存储空间
    EQUIPMENT_AREA, // 装备区域（自动选择槽位）
    EQUIPMENT_SLOT, // 特定装备槽位
    HELD_ITEM_SLOT, // 手持物品槽位
    WORLD_GROUND,   // 世界地面（丢弃）
    UNKNOWN         // 未知目标
};

// 拖放操作结果
enum class DragResult {
    SUCCESS,              // 操作成功
    FAILED_CANNOT_DETACH, // 无法从源位置移除
    FAILED_CANNOT_ATTACH, // 无法放入目标位置
    FAILED_INCOMPATIBLE,  // 不兼容（如武器无法放入弹药槽）
    FAILED_NO_SPACE,      // 目标位置没有空间
    FAILED_INVALID_SOURCE,// 无效的源
    FAILED_INVALID_TARGET,// 无效的目标
    CANCELLED            // 操作被取消
};

// 拖放源信息
struct DragSourceInfo {
    DragSourceType type;
    Storage* storage;           // 如果是存储空间拖拽
    int equipmentSlot;          // 如果是装备槽位拖拽（使用EquipSlot枚举值）
    float worldX, worldY;       // 如果是世界物品拖拽
    
    DragSourceInfo() : type(DragSourceType::UNKNOWN), storage(nullptr), equipmentSlot(-1), worldX(0.0f), worldY(0.0f) {}
    
    // 构造函数
    static DragSourceInfo fromStorage(Storage* stor) {
        DragSourceInfo info;
        info.type = DragSourceType::STORAGE;
        info.storage = stor;
        return info;
    }
    
    static DragSourceInfo fromEquipmentSlot(int slot) {
        DragSourceInfo info;
        info.type = DragSourceType::EQUIPMENT_SLOT;
        info.equipmentSlot = slot;
        return info;
    }
    
    static DragSourceInfo fromHeldItem() {
        DragSourceInfo info;
        info.type = DragSourceType::HELD_ITEM;
        return info;
    }
    
    static DragSourceInfo fromWorldItem(float x, float y) {
        DragSourceInfo info;
        info.type = DragSourceType::WORLD_ITEM;
        info.worldX = x;
        info.worldY = y;
        return info;
    }
};

// 拖放目标信息
struct DragTargetInfo {
    DragTargetType type;
    Storage* storage;           // 如果是存储空间目标
    int equipmentSlot;          // 如果是特定装备槽位目标
    float worldX, worldY;       // 如果是世界位置目标
    
    DragTargetInfo() : type(DragTargetType::UNKNOWN), storage(nullptr), equipmentSlot(-1), worldX(0.0f), worldY(0.0f) {}
    
    // 构造函数
    static DragTargetInfo fromStorage(Storage* stor) {
        DragTargetInfo info;
        info.type = DragTargetType::STORAGE;
        info.storage = stor;
        return info;
    }
    
    static DragTargetInfo fromEquipmentArea() {
        DragTargetInfo info;
        info.type = DragTargetType::EQUIPMENT_AREA;
        return info;
    }
    
    static DragTargetInfo fromEquipmentSlot(int slot) {
        DragTargetInfo info;
        info.type = DragTargetType::EQUIPMENT_SLOT;
        info.equipmentSlot = slot;
        return info;
    }
    
    static DragTargetInfo fromHeldItemSlot() {
        DragTargetInfo info;
        info.type = DragTargetType::HELD_ITEM_SLOT;
        return info;
    }
    
    static DragTargetInfo fromWorldGround(float x, float y) {
        DragTargetInfo info;
        info.type = DragTargetType::WORLD_GROUND;
        info.worldX = x;
        info.worldY = y;
        return info;
    }
};

// 拖放操作信息
struct DragOperationInfo {
    Item* item;                           // 被拖拽的物品
    DragSourceInfo source;                // 拖拽源信息
    DragTargetInfo target;                // 拖拽目标信息
    Player* player;                       // 执行拖拽的玩家
    std::function<void(DragResult)> callback; // 操作完成回调
    
    DragOperationInfo() : item(nullptr), player(nullptr) {}
    
    DragOperationInfo(Item* draggedItem, const DragSourceInfo& sourceInfo, const DragTargetInfo& targetInfo, Player* p, std::function<void(DragResult)> cb = nullptr) 
        : item(draggedItem), source(sourceInfo), target(targetInfo), player(p), callback(cb) {}
};

// 拖放系统接口
class DragDropSystem {
public:
    // 检查是否可以从源位置拖拽物品
    static bool canDetachFromSource(const DragOperationInfo& operation);
    
    // 检查是否可以放入目标位置
    static bool canAttachToTarget(const DragOperationInfo& operation);
    
    // 检查拖拽操作是否兼容
    static bool isOperationCompatible(const DragOperationInfo& operation);
    
    // 从源位置移除物品（第一阶段）
    static void detachFromSource(const DragOperationInfo& operation, std::function<void(std::unique_ptr<Item>, DragResult)> callback);
    
    // 将物品放入目标位置（第二阶段）
    static void attachToTarget(std::unique_ptr<Item> item, const DragOperationInfo& operation, std::function<void(DragResult)> callback);
    
    // 执行完整的拖放操作（内部调用detachFromSource和attachToTarget）
    static void performDragOperation(const DragOperationInfo& operation);
    
    // 获取拖拽操作的描述文本（用于调试和用户提示）
    static std::string getOperationDescription(const DragOperationInfo& operation);
    
    // 获取拖拽失败的原因文本
    static std::string getErrorDescription(DragResult result);
    
private:
    // 检查物品是否正在被Action使用
    static bool isItemBeingUsedByAction(Item* item, Player* player);
};

#endif // DRAG_DROP_SYSTEM_H 