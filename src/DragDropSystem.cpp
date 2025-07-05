#include "DragDropSystem.h"
#include "Item.h"
#include "Storage.h"
#include "Player.h"
#include "EquipmentSystem.h"
#include "EntityFlag.h"
#include "PlayerState.h"
#include "ActionQueue.h"
#include <SDL3/SDL_log.h>

// 检查是否可以从源位置拖拽物品
bool DragDropSystem::canDetachFromSource(const DragOperationInfo& operation) {
    if (!operation.item || !operation.player) {
        return false;
    }
    
    // 检查物品是否正在被Action使用
    if (isItemBeingUsedByAction(operation.item, operation.player)) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "物品 %s 正在被Action使用，无法拖放", operation.item->getName().c_str());
        return false;
    }
    
    switch (operation.source.type) {
        case DragSourceType::STORAGE:
            // 检查存储空间是否有效且包含该物品
            if (!operation.source.storage) return false;
            for (size_t i = 0; i < operation.source.storage->getItemCount(); ++i) {
                if (operation.source.storage->getItem(i) == operation.item) {
                    return true;
                }
            }
            return false;
            
        case DragSourceType::EQUIPMENT_SLOT:
            // 检查装备槽位是否有效且装备了该物品
            if (operation.source.equipmentSlot < 0) return false;
            if (!operation.player->getEquipmentSystem()) return false;
            {
                auto equipSlot = static_cast<EquipSlot>(operation.source.equipmentSlot);
                auto equippedItems = operation.player->getEquipmentSystem()->getEquippedItems(equipSlot);
                for (auto* item : equippedItems) {
                    if (item == operation.item) {
                        return true;
                    }
                }
            }
            return false;
            
        case DragSourceType::HELD_ITEM:
            // 检查是否确实手持该物品
            return operation.player->getHeldItem() == operation.item;
            
        case DragSourceType::WORLD_ITEM:
            // 世界物品拖拽暂时不支持
            return false;
            
        default:
            return false;
    }
}

// 检查是否可以放入目标位置
bool DragDropSystem::canAttachToTarget(const DragOperationInfo& operation) {
    if (!operation.item || !operation.player) {
        return false;
    }
    
    switch (operation.target.type) {
        case DragTargetType::STORAGE:
            // 检查存储空间是否能容纳该物品
            if (!operation.target.storage) return false;
            return operation.target.storage->canFitItem(operation.item);
            
        case DragTargetType::EQUIPMENT_AREA:
            // 检查物品是否可穿戴
            if (!operation.item->isWearable()) return false;
            // 简单检查装备系统是否存在即可，具体的兼容性在装备时检查
            return operation.player->getEquipmentSystem() != nullptr;
            
        case DragTargetType::EQUIPMENT_SLOT:
            // 检查物品是否可以装备到特定槽位
            if (!operation.item->isWearable()) return false;
            if (operation.target.equipmentSlot < 0) return false;
            {
                auto equipSlot = static_cast<EquipSlot>(operation.target.equipmentSlot);
                return operation.item->canEquipToSlot(equipSlot);
            }
            
        case DragTargetType::HELD_ITEM_SLOT:
            // 检查是否可以手持该物品（大部分物品都可以手持）
            return true;
            
        case DragTargetType::WORLD_GROUND:
            // 物品丢弃到地面（总是可以）
            return true;
            
        default:
            return false;
    }
}

// 检查拖拽操作是否兼容
bool DragDropSystem::isOperationCompatible(const DragOperationInfo& operation) {
    // 检查源和目标是否都有效
    if (!canDetachFromSource(operation) || !canAttachToTarget(operation)) {
        return false;
    }
    
    // 检查是否是同一位置的拖拽（无意义操作）
    if (operation.source.type == DragSourceType::STORAGE && 
        operation.target.type == DragTargetType::STORAGE &&
        operation.source.storage == operation.target.storage) {
        return false;
    }
    
    if (operation.source.type == DragSourceType::EQUIPMENT_SLOT && 
        operation.target.type == DragTargetType::EQUIPMENT_SLOT &&
        operation.source.equipmentSlot == operation.target.equipmentSlot) {
        return false;
    }
    
    if (operation.source.type == DragSourceType::HELD_ITEM && 
        operation.target.type == DragTargetType::HELD_ITEM_SLOT) {
        return false;
    }
    
    // 其他兼容性检查可以在这里添加
    return true;
}

// 从源位置移除物品（第一阶段）
void DragDropSystem::detachFromSource(const DragOperationInfo& operation, std::function<void(std::unique_ptr<Item>, DragResult)> callback) {
    if (!callback) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "DragDropSystem::detachFromSource: callback is null");
        return;
    }
    
    if (!canDetachFromSource(operation)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "DragDropSystem::detachFromSource: cannot detach from source");
        callback(nullptr, DragResult::FAILED_CANNOT_DETACH);
        return;
    }
    
    switch (operation.source.type) {
        case DragSourceType::STORAGE:
            // 从存储空间取出物品
            operation.player->takeItemWithAction(operation.item, operation.source.storage, 
                [callback](std::unique_ptr<Item> takenItem) {
                    if (takenItem) {
                        callback(std::move(takenItem), DragResult::SUCCESS);
                    } else {
                        callback(nullptr, DragResult::FAILED_CANNOT_DETACH);
                    }
                });
            break;
            
        case DragSourceType::EQUIPMENT_SLOT:
            // 从装备槽位卸下物品
            operation.player->unequipItemWithAction(operation.item, 
                [callback](std::unique_ptr<Item> unequippedItem) {
                    if (unequippedItem) {
                        callback(std::move(unequippedItem), DragResult::SUCCESS);
                    } else {
                        callback(nullptr, DragResult::FAILED_CANNOT_DETACH);
                    }
                });
            break;
            
        case DragSourceType::HELD_ITEM:
            // 从手持位置卸下物品
            {
                auto equipSlot = EquipSlot::RIGHT_HAND;
                operation.player->unequipItem(equipSlot, 
                    [callback](std::unique_ptr<Item> unequippedItem) {
                        if (unequippedItem) {
                            callback(std::move(unequippedItem), DragResult::SUCCESS);
                        } else {
                            callback(nullptr, DragResult::FAILED_CANNOT_DETACH);
                        }
                    });
            }
            break;
            
        default:
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "DragDropSystem::detachFromSource: unsupported source type");
            callback(nullptr, DragResult::FAILED_INVALID_SOURCE);
            break;
    }
}

// 将物品放入目标位置（第二阶段）
void DragDropSystem::attachToTarget(std::unique_ptr<Item> item, const DragOperationInfo& operation, std::function<void(DragResult)> callback) {
    if (!callback) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "DragDropSystem::attachToTarget: callback is null");
        return;
    }
    
    if (!item) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "DragDropSystem::attachToTarget: item is null");
        callback(DragResult::FAILED_CANNOT_ATTACH);
        return;
    }
    
    // 创建临时操作信息用于检查兼容性
    DragOperationInfo tempOperation = operation;
    tempOperation.item = item.get();
    
    if (!canAttachToTarget(tempOperation)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "DragDropSystem::attachToTarget: cannot attach to target");
        callback(DragResult::FAILED_CANNOT_ATTACH);
        return;
    }
    
    switch (operation.target.type) {
        case DragTargetType::STORAGE:
            // 将物品放入存储空间
            operation.player->storeItemWithAction(std::move(item), operation.target.storage);
            callback(DragResult::SUCCESS);
            break;
            
        case DragTargetType::EQUIPMENT_AREA:
            // 装备物品到合适的槽位
            operation.player->equipItemWithAction(std::move(item));
            callback(DragResult::SUCCESS);
            break;
            
        case DragTargetType::EQUIPMENT_SLOT:
            // 装备物品到特定槽位
            operation.player->equipItemWithAction(std::move(item));
            callback(DragResult::SUCCESS);
            break;
            
        case DragTargetType::HELD_ITEM_SLOT:
            // 将物品设置为手持
            operation.player->holdItem(std::move(item));
            callback(DragResult::SUCCESS);
            break;
            
        case DragTargetType::WORLD_GROUND:
            // 将物品丢弃到地面
            // 这里需要调用游戏的物品掉落系统
            // 暂时简化处理
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "物品已丢弃到地面: %s", item->getName().c_str());
            callback(DragResult::SUCCESS);
            break;
            
        default:
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "DragDropSystem::attachToTarget: unsupported target type");
            callback(DragResult::FAILED_INVALID_TARGET);
            break;
    }
}

// 执行完整的拖放操作
void DragDropSystem::performDragOperation(const DragOperationInfo& operation) {
    if (!isOperationCompatible(operation)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "DragDropSystem::performDragOperation: operation is not compatible");
        if (operation.callback) {
            operation.callback(DragResult::FAILED_INCOMPATIBLE);
        }
        return;
    }
    
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "开始拖放操作: %s", getOperationDescription(operation).c_str());
    
    // 第一阶段：从源位置移除物品
    detachFromSource(operation, [operation](std::unique_ptr<Item> detachedItem, DragResult detachResult) {
        if (detachResult != DragResult::SUCCESS) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "拖放操作失败 - 无法从源位置移除物品: %s", 
                        getErrorDescription(detachResult).c_str());
            if (operation.callback) {
                operation.callback(detachResult);
            }
            return;
        }
        
        if (!detachedItem) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "拖放操作失败 - 从源位置移除的物品为空");
            if (operation.callback) {
                operation.callback(DragResult::FAILED_CANNOT_DETACH);
            }
            return;
        }
        
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "成功从源位置移除物品: %s", detachedItem->getName().c_str());
        
        // 第二阶段：将物品放入目标位置
        attachToTarget(std::move(detachedItem), operation, [operation](DragResult attachResult) {
            if (attachResult == DragResult::SUCCESS) {
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "拖放操作成功完成");
            } else {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "拖放操作失败 - 无法放入目标位置: %s", 
                            getErrorDescription(attachResult).c_str());
            }
            
            if (operation.callback) {
                operation.callback(attachResult);
            }
        });
    });
}

// 获取拖拽操作的描述文本
std::string DragDropSystem::getOperationDescription(const DragOperationInfo& operation) {
    if (!operation.item) {
        return "未知物品的拖拽操作";
    }
    
    std::string sourceDesc;
    switch (operation.source.type) {
        case DragSourceType::STORAGE:
            sourceDesc = operation.source.storage ? operation.source.storage->getName() : "未知存储空间";
            break;
        case DragSourceType::EQUIPMENT_SLOT:
            sourceDesc = "装备槽位" + std::to_string(operation.source.equipmentSlot);
            break;
        case DragSourceType::HELD_ITEM:
            sourceDesc = "手持位置";
            break;
        case DragSourceType::WORLD_ITEM:
            sourceDesc = "世界物品";
            break;
        default:
            sourceDesc = "未知源";
            break;
    }
    
    std::string targetDesc;
    switch (operation.target.type) {
        case DragTargetType::STORAGE:
            targetDesc = operation.target.storage ? operation.target.storage->getName() : "未知存储空间";
            break;
        case DragTargetType::EQUIPMENT_AREA:
            targetDesc = "装备区域";
            break;
        case DragTargetType::EQUIPMENT_SLOT:
            targetDesc = "装备槽位" + std::to_string(operation.target.equipmentSlot);
            break;
        case DragTargetType::HELD_ITEM_SLOT:
            targetDesc = "手持位置";
            break;
        case DragTargetType::WORLD_GROUND:
            targetDesc = "地面";
            break;
        default:
            targetDesc = "未知目标";
            break;
    }
    
    return "将 " + operation.item->getName() + " 从 " + sourceDesc + " 拖拽到 " + targetDesc;
}

// 获取拖拽失败的原因文本
std::string DragDropSystem::getErrorDescription(DragResult result) {
    switch (result) {
        case DragResult::SUCCESS:
            return "操作成功";
        case DragResult::FAILED_CANNOT_DETACH:
            return "无法从源位置移除物品";
        case DragResult::FAILED_CANNOT_ATTACH:
            return "无法放入目标位置";
        case DragResult::FAILED_INCOMPATIBLE:
            return "不兼容的操作";
        case DragResult::FAILED_NO_SPACE:
            return "目标位置没有空间";
        case DragResult::FAILED_INVALID_SOURCE:
            return "无效的源位置";
        case DragResult::FAILED_INVALID_TARGET:
            return "无效的目标位置";
        case DragResult::CANCELLED:
            return "操作被取消";
        default:
            return "未知错误";
    }
}

// 检查物品是否正在被Action使用
bool DragDropSystem::isItemBeingUsedByAction(Item* item, Player* player) {
    if (!item || !player) {
        return false;
    }
    
    // 获取玩家的行为队列
    ActionQueue* actionQueue = player->getActionQueue();
    if (!actionQueue) {
        return false;
    }
    
    // 检查是否有正在执行的Action
    Action* currentAction = actionQueue->getCurrentAction();
    if (!currentAction) {
        return false; // 没有正在执行的Action
    }
    
    // 如果物品是手持物品，且有正在执行的Action，则阻止拖放
    // 这是为了防止换弹等操作过程中拖动武器导致闪退
    if (player->getHeldItem() == item) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "检测到手持物品 %s 有正在执行的Action，状态: %d", 
                   item->getName().c_str(), static_cast<int>(currentAction->getActionState()));
        return true; // 阻止拖放
    }
    
    // 对于其他物品，暂时允许拖放
    // 后续可以根据需要添加更详细的检查
    return false;
} 