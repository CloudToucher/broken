#include "Action.h"
#include "Entity.h"
#include "Gun.h"
#include "Magazine.h"
#include "SoundManager.h"
#include "Storage.h"


// Action基类实现
Action::Action(Entity* entity, float actionDuration, EntityState state, const std::string& sound)
    : owner(entity), duration(actionDuration), actionState(state), soundId(sound),
      isCompleted(false), isStarted(false) {
}

void Action::start() {
    if (!isStarted && owner) {
        isStarted = true;
        // 设置实体状态
        owner->setState(actionState, duration);
        
        // 播放音效
        if (!soundId.empty()) {
            SoundManager::getInstance()->playSound(soundId);
        }
    }
}

void Action::update(float deltaTime) {
    if (!owner) {
        // 如果所有者不存在，直接标记为完成
        isCompleted = true;
        return;
    }
    
    if (isStarted && !isCompleted) {
        duration -= deltaTime;
        if (duration <= 0) {
            // 确保duration不会小于0，避免进度条计算问题
            duration = 0;
            end();
        }
    }
}

void Action::end() {
    if (isStarted && !isCompleted) {
        isCompleted = true;
        // 恢复实体状态为IDLE
        if (owner) {
            owner->setState(EntityState::IDLE, 0.0f);
        }
    }
}

void Action::interrupt() {
    if (isStarted && !isCompleted) {
        isCompleted = true;
        // 恢复实体状态为IDLE
        if (owner) {
            owner->setState(EntityState::IDLE, 0.0f);
        }
    }
}

// UnloadMagazineAction实现
UnloadMagazineAction::UnloadMagazineAction(Entity* entity, Gun* gun, Storage* storage, std::function<void(std::unique_ptr<Magazine>)> callback)
    : Action(entity, 
             gun && gun->getCurrentMagazine() ? 
                 (gun->getReloadTime() + gun->getCurrentMagazine()->getUnloadTime() + (storage ? storage->getStorageTime() : 0.0f)) : 
                 0.0f,
             EntityState::UNLOADING, "unload"),
      weapon(gun), onMagazineUnloaded(callback) {
}

void UnloadMagazineAction::start() {
    if (!isStarted && owner && weapon && weapon->getCurrentMagazine()) {
        std::cout << "开始卸下弹匣，预计时间: " << duration << " 秒" << std::endl;
        Action::start();
    } else {
        // 如果没有弹匣，直接标记为完成
        isStarted = true;
        isCompleted = true;
    }
}

void UnloadMagazineAction::end() {
    if (isStarted && !isCompleted) {
        // 卸下弹匣
        if (weapon) {
            std::unique_ptr<Magazine> unloadedMag = weapon->unloadMagazine();
            // 如果有回调函数，调用它
            if (onMagazineUnloaded && unloadedMag) {
                onMagazineUnloaded(std::move(unloadedMag));
            }
        }
        
        Action::end();
    }
}

// LoadMagazineAction实现
LoadMagazineAction::LoadMagazineAction(Entity* entity, Gun* gun, std::unique_ptr<Magazine> mag, Storage* storage)
    : Action(entity, 
             mag && gun ? 
                 (gun->getReloadTime() + mag->getReloadTime() + mag->getModReloadTime() + (storage ? storage->getStorageTime() : 0.0f)) : 
                 0.0f, 
             EntityState::RELOADING, "reload"),
      weapon(gun), magazine(std::move(mag)) {
}

void LoadMagazineAction::start() {
    if (!isStarted && owner && weapon && magazine && weapon->canAcceptMagazine(magazine.get())) {
        std::cout << "开始装填弹匣，预计时间: " << duration << " 秒" << std::endl;
        Action::start();
    } else {
        // 如果无法装填弹匣，直接标记为完成
        isStarted = true;
        isCompleted = true;
    }
}

void LoadMagazineAction::end() {
    if (isStarted && !isCompleted) {
        // 装填弹匣 - 考虑Magazine作为GunMod的特性，但保持传统装填方式
        if (weapon && magazine) {
            weapon->loadMagazine(std::move(magazine));
        }
        
        Action::end();
    }
}

// ChamberRoundAction实现
ChamberRoundAction::ChamberRoundAction(Entity* entity, Gun* gun)
    : Action(entity, 0.3f, EntityState::CHAMBERING, ""),
      weapon(gun), wasEmpty(gun ? gun->getChamberedRound() == nullptr : true) {
}

void ChamberRoundAction::start() {
    if (!isStarted && owner && weapon) {
        // 检查是否可以上膛
        if (weapon->chamberManually()) {
            // 如果之前枪膛为空，播放空仓挂机释放音效
            if (wasEmpty) {
                soundId = "bolt_release";
            }
            std::cout << "开始上膛，预计时间: " << duration << " 秒" << std::endl;
            Action::start();
        } else {
            // 如果无法上膛，直接标记为完成
            isStarted = true;
            isCompleted = true;
        }
    } else {
        // 如果没有武器，直接标记为完成
        isStarted = true;
        isCompleted = true;
    }
}

void ChamberRoundAction::end() {
    if (isStarted && !isCompleted) {
        // 上膛操作已在start中完成，这里只需要结束行为
        Action::end();
    }
}

// StoreItemAction实现
StoreItemAction::StoreItemAction(Entity* entity, std::unique_ptr<Item> itemToStore, Storage* storage)
    : Action(entity, storage ? storage->getStorageTime() : 0.0f, EntityState::STORING_ITEM),
      item(std::move(itemToStore)), targetStorage(storage) {
}

void StoreItemAction::start() {
    if (!isStarted && owner && targetStorage && item) {
        // 检查存储空间是否可以容纳该物品
        if (targetStorage->getCurrentWeight() + item->getWeight() <= targetStorage->getMaxWeight() &&
            targetStorage->getCurrentVolume() + item->getVolume() <= targetStorage->getMaxVolume() &&
            item->getLength() <= targetStorage->getMaxLength()) {
            Action::start();
        } else {
            // 如果存储空间不足，直接标记为完成
            isStarted = true;
            isCompleted = true;
        }
    } else {
        // 如果没有物品或存储空间，直接标记为完成
        isStarted = true;
        isCompleted = true;
    }
}

void StoreItemAction::end() {
    if (isStarted && !isCompleted) {
        // 将物品添加到存储空间
        if (targetStorage && item) {
            targetStorage->addItem(std::move(item));
        }
        
        Action::end();
    }
}

// HoldItemAction实现
HoldItemAction::HoldItemAction(Entity* entity, Item* itemToHold, Storage* storage, std::function<void(std::unique_ptr<Item>)> callback)
    : Action(entity, storage ? storage->getStorageTime() : 0.0f, EntityState::TAKING_ITEM),
      item(itemToHold), sourceStorage(storage), onItemHeld(callback) {
}

void HoldItemAction::start() {
    if (!isStarted && owner && sourceStorage && item) {
        // 检查物品是否在存储空间中
        bool itemFound = false;
        for (size_t i = 0; i < sourceStorage->getItemCount(); ++i) {
            if (sourceStorage->getItem(i) == item) {
                itemFound = true;
                break;
            }
        }
        
        if (itemFound) {
            Action::start();
        } else {
            // 如果物品不在存储空间中，直接标记为完成
            isStarted = true;
            isCompleted = true;
        }
    } else {
        // 如果没有物品或存储空间，直接标记为完成
        isStarted = true;
        isCompleted = true;
    }
}

void HoldItemAction::end() {
    if (isStarted && !isCompleted) {
        // 从存储空间中移除物品并通过回调函数传递给玩家手持
        if (sourceStorage && item) {
            // 查找物品在存储空间中的索引
            for (size_t i = 0; i < sourceStorage->getItemCount(); ++i) {
                if (sourceStorage->getItem(i) == item) {
                    // 移除物品
                    std::unique_ptr<Item> removedItem = sourceStorage->removeItem(i);
                    
                    // 如果有回调函数，调用它
                    if (onItemHeld && removedItem) {
                        onItemHeld(std::move(removedItem));
                    }
                    break;
                }
            }
        }
        
        Action::end();
    }
}

// TakeItemAction实现
TakeItemAction::TakeItemAction(Entity* entity, Item* itemToTake, Storage* storage, std::function<void(std::unique_ptr<Item>)> callback)
    : Action(entity, storage ? storage->getStorageTime() : 0.0f, EntityState::TAKING_ITEM),
      item(itemToTake), sourceStorage(storage), onItemTaken(callback) {
}

void TakeItemAction::start() {
    if (!isStarted && owner && sourceStorage && item) {
        // 检查物品是否在存储空间中
        bool itemFound = false;
        for (size_t i = 0; i < sourceStorage->getItemCount(); ++i) {
            if (sourceStorage->getItem(i) == item) {
                itemFound = true;
                break;
            }
        }
        
        if (itemFound) {
            Action::start();
        } else {
            // 如果物品不在存储空间中，直接标记为完成
            isStarted = true;
            isCompleted = true;
        }
    } else {
        // 如果没有物品或存储空间，直接标记为完成
        isStarted = true;
        isCompleted = true;
    }
}

void TakeItemAction::end() {
    if (isStarted && !isCompleted) {
        // 从存储空间中取出物品
        if (sourceStorage && item) {
            // 查找物品在存储空间中的索引
            for (size_t i = 0; i < sourceStorage->getItemCount(); ++i) {
                if (sourceStorage->getItem(i) == item) {
                    // 移除物品
                    std::unique_ptr<Item> takenItem = sourceStorage->removeItem(static_cast<int>(i));
                    
                    // 如果有回调函数，调用它
                    if (onItemTaken && takenItem) {
                        onItemTaken(std::move(takenItem));
                    }
                    
                    break;
                }
            }
        }
        
        Action::end();
    }
}

// EquipItemAction实现
EquipItemAction::EquipItemAction(Entity* entity, std::unique_ptr<Item> itemToEquip)
    : Action(entity, itemToEquip && entity ? entity->getEquipmentSystem()->calculateEquipTime(itemToEquip.get()) : 0.0f, 
             EntityState::EQUIPPING, "equip"),
      item(std::move(itemToEquip)) {
}

void EquipItemAction::start() {
    if (!isStarted && owner && item && item->isWearable()) {
        Action::start();
    } else {
        // 如果物品不可wearable，直接标记为完成
        isStarted = true;
        isCompleted = true;
    }
}

void EquipItemAction::end() {
    if (isStarted && !isCompleted) {
        // 穿戴物品
        if (owner && item && owner->getEquipmentSystem()) {
            // 直接调用EquipmentSystem的equipItem方法，而不是通过Entity::equipItem
            owner->getEquipmentSystem()->equipItem(std::move(item));
        }
        
        Action::end();
    }
}

// TransferItemAction实现
TransferItemAction::TransferItemAction(Entity* entity, Item* itemToTransfer, Storage* source, Storage* target, std::function<void(bool)> callback)
    : Action(entity, 
             // 计算总时间：取出时间 + 存入时间
             (source ? source->getAccessTime() : 0.0f) + (target ? target->getAccessTime() : 0.0f),
             EntityState::TRANSFERRING_ITEM),
      item(itemToTransfer), sourceStorage(source), targetStorage(target), onTransferComplete(callback) {
}

void TransferItemAction::start() {
    if (!isStarted && owner && sourceStorage && targetStorage && item) {
        // 检查物品是否在源存储空间中
        bool itemFound = false;
        for (size_t i = 0; i < sourceStorage->getItemCount(); ++i) {
            if (sourceStorage->getItem(i) == item) {
                itemFound = true;
                break;
            }
        }
        
        // 检查目标存储空间是否有足够空间
        bool hasSpace = targetStorage->canFitItem(item);
        
        if (itemFound && hasSpace) {
            Action::start();
        } else {
            // 如果物品不在源存储空间中或目标存储空间不足，直接标记为完成
            isStarted = true;
            isCompleted = true;
            
            // 调用回调函数，传递失败状态
            if (onTransferComplete) {
                onTransferComplete(false);
            }
        }
    } else {
        // 如果参数无效，直接标记为完成
        isStarted = true;
        isCompleted = true;
        
        // 调用回调函数，传递失败状态
        if (onTransferComplete) {
            onTransferComplete(false);
        }
    }
}

void TransferItemAction::end() {
    if (isStarted && !isCompleted) {
        // 从源存储空间中取出物品并添加到目标存储空间
        bool success = false;
        
        // 查找物品在源存储空间中的索引
        for (size_t i = 0; i < sourceStorage->getItemCount(); ++i) {
            if (sourceStorage->getItem(i) == item) {
                // 移除物品
                std::unique_ptr<Item> transferredItem = sourceStorage->removeItem(static_cast<int>(i));
                
                // 添加到目标存储空间
                if (transferredItem) {
                    success = targetStorage->addItem(std::move(transferredItem));
                }
                
                break;
            }
        }
        
        // 调用回调函数，传递成功状态
        if (onTransferComplete) {
            onTransferComplete(success);
        }
        
        Action::end();
    }
}

// UnequipItemAction实现 - 按槽位卸下
UnequipItemAction::UnequipItemAction(Entity* entity, EquipSlot equipSlot, std::function<void(std::unique_ptr<Item>)> callback)
    : Action(entity, entity && entity->getEquipmentSystem() ? 
             entity->getEquipmentSystem()->calculateUnequipTime(entity->getEquipmentSystem()->getEquippedItem(equipSlot)) : 0.0f, 
             EntityState::UNEQUIPPING, "unequip"),
      slot(equipSlot), onItemUnequipped(callback) {
}

void UnequipItemAction::start() {
    if (!isStarted && owner && owner->getEquipmentSystem() && owner->getEquipmentSystem()->isSlotEquipped(slot)) {
        Action::start();
    } else {
        // 如果槽位没有装备物品，直接标记为完成
        isStarted = true;
        isCompleted = true;
        
        // 如果有回调，调用回调函数并传递nullptr
        if (onItemUnequipped) {
            onItemUnequipped(nullptr);
        }
    }
}

void UnequipItemAction::end() {
    if (isStarted && !isCompleted) {
        // 卸下物品
        std::unique_ptr<Item> unequippedItem = nullptr;
        if (owner && owner->getEquipmentSystem()) {
            auto [_, item] = owner->getEquipmentSystem()->unequipItem(slot);
            unequippedItem = std::move(item);
        }
        
        // 如果有回调，调用回调函数
        if (onItemUnequipped) {
            onItemUnequipped(std::move(unequippedItem));
        }
        
        Action::end();
    }
}

// UnequipItemByItemAction实现 - 按物品卸下
UnequipItemByItemAction::UnequipItemByItemAction(Entity* entity, Item* item, std::function<void(std::unique_ptr<Item>)> callback)
    : Action(entity, entity && entity->getEquipmentSystem() && item ? 
             entity->getEquipmentSystem()->calculateUnequipTime(item) : 0.0f, 
             EntityState::UNEQUIPPING, "unequip"),
      targetItem(item), onItemUnequipped(callback) {
}

void UnequipItemByItemAction::start() {
    if (!isStarted && owner && owner->getEquipmentSystem() && targetItem && 
        owner->getEquipmentSystem()->isItemEquipped(targetItem)) {
        Action::start();
    } else {
        // 如果物品未装备，直接标记为完成
        isStarted = true;
        isCompleted = true;
        
        // 如果有回调，调用回调函数并传递nullptr
        if (onItemUnequipped) {
            onItemUnequipped(nullptr);
        }
    }
}

void UnequipItemByItemAction::end() {
    if (isStarted && !isCompleted) {
        // 卸下物品
        std::unique_ptr<Item> unequippedItem = nullptr;
        if (owner && owner->getEquipmentSystem() && targetItem) {
            auto [_, item] = owner->getEquipmentSystem()->unequipItem(targetItem);
            unequippedItem = std::move(item);
        }
        
        // 如果有回调，调用回调函数
        if (onItemUnequipped) {
            onItemUnequipped(std::move(unequippedItem));
        }
        
        Action::end();
    }
}
