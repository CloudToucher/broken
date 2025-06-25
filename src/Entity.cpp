#include "Entity.h"
#include <cmath>
#include <iostream>
#include "Gun.h"
#include "Magazine.h"
#include "SoundManager.h" // <--- 添加这一行
#include "Game.h" // 添加Game.h头文件
#include "Map.h" // 添加Map.h头文件用于地形碰撞检测
#include "Storage.h" // 添加Game.h头文件
#include "Bullet.h" // 添加Bullet.h头文件用于前向声明
#include "Action.h" // 添加Action.h头文件
#include "EntityStateEffect.h" // 添加EntityStateEffect.h头文件
#include "Damage.h" // 添加Damage.h头文件
#include "EntityFlag.h" // 添加EntityFlag.h头文件

// 在现有的Entity.cpp文件中添加以下内容

// 修改构造函数，初始化状态管理器和新成员变量
Entity::Entity(float startX, float startY, int entityRadius, int entitySpeed, int entityHealth, SDL_Color entityColor, Faction entityFaction)
    : x(startX), y(startY), prevX(startX), prevY(startY), 
      radius(entityRadius), speed(entitySpeed), health(entityHealth), 
      color(entityColor), collider(startX, startY, static_cast<float>(entityRadius), "entity", ColliderPurpose::ENTITY, 0),
      faction(entityFaction), currentState(EntityState::IDLE), stateTimer(0.0f), speedModifier(1.0f),
      shootCooldown(0),
      // 初始化物理引擎属性
      velocityX(0.0f), velocityY(0.0f), desiredVelocityX(0.0f), desiredVelocityY(0.0f),
      mass(1.0f), isStatic(false),
      // 初始化碰撞信息向量
      collisions(),
      // 初始化新增属性
      volume(0.0f), weight(70.0f), height(1.75f),  // 默认人类体重70kg，身高1.75m
      smellIntensity(0), soundIntensity(0), soundFile(""),
      // 初始化属性值
      strength(10), dexterity(10), perception(10), intelligence(10) {
    // 初始化装备系统
    equipmentSystem = std::make_unique<EquipmentSystem>();
    
    // 初始化行为队列
    actionQueue = std::make_unique<ActionQueue>(this);
    
    // 初始化状态管理器
    stateManager = std::make_unique<EntityStateManager>();
    
    // 设置状态变化回调
    stateManager->setOnStateAdded([this](EntityStateEffect* state) {
        // 当添加新状态时，更新实体属性
        updateStateEffects();
    });
    
    stateManager->setOnStateRemoved([this](EntityStateEffect* state) {
        // 当移除状态时，更新实体属性
        updateStateEffects();
    });
    
    stateManager->setOnStateUpdated([this](EntityStateEffect* state) {
        // 当状态更新时，可以在这里添加额外的逻辑
    });
}

// 实现自动换弹函数 - 通过行为队列系统实现
float Entity::reloadWeaponAuto(Gun* weapon) {
    if (!weapon || !canPerformAction() || !equipmentSystem || !actionQueue) {
        return 0.0f;
    }

    // 计算预期的总换弹时间（用于返回值）
    float totalReloadTime = 0.0f;
    
    // 获取枪支兼容的弹药类型
    std::vector<std::string> compatibleTypes;
    if (weapon->getCurrentMagazine()) {
        compatibleTypes = weapon->getCurrentMagazine()->getCompatibleAmmoTypes();
        totalReloadTime += weapon->getCurrentMagazine()->getUnloadTime(); // 卸载时间
    } else {
        // 如果没有弹匣，尝试从枪支获取兼容类型
        // 这里假设Gun类有一个方法可以获取兼容的弹药类型
        // compatibleTypes = weapon->getAcceptedAmmoTypes();
        return 0.0f; // 无法确定兼容类型
    }

    // 在所有存储空间中查找最满的兼容弹匣
    Magazine* fullestMag = findFullestMagazine(compatibleTypes);
    if (!fullestMag) {
        return 0.0f; // 没有找到兼容的弹匣
    }
    
    // 添加装填时间
    totalReloadTime += fullestMag->getReloadTime();
    
    // 如果需要上膛，添加上膛时间
    if (!weapon->getChamberedRound()) {
        totalReloadTime += 0.3f; // 上膛时间
    }
    
    // 使用行为队列系统执行换弹操作
    reloadWeaponWithActions(weapon);
    
    return totalReloadTime;
}

// 修改update方法，更新状态管理器
void Entity::update(float deltaTime) {
    // 更新状态计时器（旧系统）
    updateStateTimer(deltaTime);
    
    // 更新状态管理器（新系统）
    if (stateManager) {
        stateManager->update(static_cast<int>(deltaTime * 1000)); // 转换为毫秒
    }
    
    // 更新射击冷却计时器
    updateShootCooldown();
    
    // 更新行为队列
    if (actionQueue) {
        actionQueue->update(deltaTime);
    }
    
    // 使用新的物理系统更新位置
    updatePhysics(deltaTime);
}

// 实现新的状态管理方法

// 添加状态
EntityStateEffect* Entity::addState(
    EntityStateEffect::Type type,
    const std::string& name,
    int duration,
    int priority
) {
    if (!stateManager) return nullptr;
    return stateManager->addState(type, name, duration, priority);
}

// 移除状态
bool Entity::removeState(const std::string& name) {
    if (!stateManager) return false;
    return stateManager->removeState(name);
}

bool Entity::removeState(EntityStateEffect::Type type) {
    if (!stateManager) return false;
    return stateManager->removeState(type);
}

// 检查状态是否存在
bool Entity::hasState(const std::string& name) const {
    if (!stateManager) return false;
    return stateManager->hasState(name);
}

bool Entity::hasState(EntityStateEffect::Type type) const {
    if (!stateManager) return false;
    return stateManager->hasState(type);
}

// 获取状态
EntityStateEffect* Entity::getState(const std::string& name) {
    if (!stateManager) return nullptr;
    return stateManager->getState(name);
}

EntityStateEffect* Entity::getState(EntityStateEffect::Type type) {
    if (!stateManager) return nullptr;
    return stateManager->getState(type);
}

// 获取所有状态
const std::vector<std::unique_ptr<EntityStateEffect>>& Entity::getAllStates() const {
    static std::vector<std::unique_ptr<EntityStateEffect>> emptyVector;
    if (!stateManager) return emptyVector;
    return stateManager->getAllStates();
}

// 清除所有状态
void Entity::clearStates() {
    if (stateManager) {
        stateManager->clearStates();
    }
}

// 根据状态更新实体属性
void Entity::updateStateEffects() {
    if (!stateManager) return;
    
    // 默认速度修正系数为1.0
    float newSpeedModifier = 1.0f;
    
    // 检查是否有移动状态
    if (stateManager->hasState(EntityStateEffect::Type::MOVING)) {
        // 移动状态不影响速度
    }
    
    // 检查是否有射击状态
    if (stateManager->hasState(EntityStateEffect::Type::SHOOTING)) {
        // 射击状态不影响速度
    }
    
    // 检查是否有换弹状态
    if (stateManager->hasState(EntityStateEffect::Type::RELOADING)) {
        newSpeedModifier *= 0.5f; // 换弹时速度减半
    }
    
    // 检查是否有交互状态
    if (stateManager->hasState(EntityStateEffect::Type::INTERACTING)) {
        newSpeedModifier *= 0.6f; // 交互时速度降低40%
    }
    
    // 检查是否有眩晕状态
    if (stateManager->hasState(EntityStateEffect::Type::STUNNED)) {
        newSpeedModifier = 0.0f; // 眩晕状态下不能移动
    }
    
    // 应用新的速度修正系数
    speedModifier = newSpeedModifier;
    
    // 更新旧的状态系统（过渡期间）
    // 如果有眩晕状态，设置为眩晕
    if (stateManager->hasState(EntityStateEffect::Type::STUNNED)) {
        currentState = EntityState::STUNNED;
    }
    // 如果有换弹状态，设置为换弹
    else if (stateManager->hasState(EntityStateEffect::Type::RELOADING)) {
        currentState = EntityState::RELOADING;
    }
    // 如果没有特殊状态，设置为空闲
    else {
        currentState = EntityState::IDLE;
    }
}

// 状态管理方法实现
void Entity::setState(EntityState newState, float duration) {
    currentState = newState;
    stateTimer = duration;
    
    // 根据状态设置速度修正系数
    switch (newState) {
        case EntityState::IDLE:
            speedModifier = 1.0f;
            break;
        case EntityState::RELOADING:
        case EntityState::UNLOADING:
        case EntityState::CHAMBERING:
            speedModifier = 0.5f; // 装弹、卸弹和上膛时移动速度减半
            // 同时添加到新状态系统
            addState(EntityStateEffect::Type::RELOADING, "reloading", static_cast<int>(duration * 1000));
            break;
        case EntityState::EQUIPPING:
        case EntityState::UNEQUIPPING:
            speedModifier = 0.7f; // 装备和卸装备时移动速度降低30%
            break;
        case EntityState::STORING_ITEM:
        case EntityState::TAKING_ITEM:
        case EntityState::TRANSFERRING_ITEM:
            speedModifier = 0.6f; // 存取物品和转移物品时移动速度降低40%
            // 同时添加到新状态系统
            addState(EntityStateEffect::Type::INTERACTING, "interacting", static_cast<int>(duration * 1000));
            break;
        case EntityState::STUNNED:
            speedModifier = 0.0f; // 眩晕状态下不能移动
            // 同时添加到新状态系统
            addState(EntityStateEffect::Type::STUNNED, "stunned", static_cast<int>(duration * 1000));
            break;
        case EntityState::SLOWED:
            speedModifier = 0.5f; // 减速状态下移动速度减半
            // 同时添加到新状态系统
            addState(EntityStateEffect::Type::DEBUFFED, "slowed", static_cast<int>(duration * 1000));
            break;
        case EntityState::DEAD:
            speedModifier = 0.0f; // 死亡状态下不能移动
            break;
    }
}

// 实现穿戴物品方法 - 只处理可穿戴物品，手持物品由子类处理
float Entity::equipItem(std::unique_ptr<Item> item) {
    if (!item || !equipmentSystem || !canPerformAction()) {
        return 0.0f;
    }
    
    // 检查物品是否可穿戴
    if (!item->isWearable()) {
        // 不处理非可穿戴物品（如武器、工具等），这些应由子类处理
        return 0.0f;
    }
    
    // 获取穿戴时间
    float equipTime = equipmentSystem->equipItem(std::move(item));
    
    // 如果穿戴时间大于0，设置实体状态为装备中
    if (equipTime > 0.0f) {
        setState(EntityState::EQUIPPING, equipTime);
    }
    
    return equipTime;
}

// 实现使用Action穿戴物品方法 - 只处理可穿戴物品，手持物品由子类处理
void Entity::equipItemWithAction(std::unique_ptr<Item> item) {
    if (!item || !equipmentSystem || !actionQueue) {
        return;
    }
    
    // 检查物品是否可穿戴
    if (!item->isWearable()) {
        // 不处理非可穿戴物品（如武器、工具等），这些应由子类处理
        return;
    }
    
    // 创建穿戴物品行为并添加到行为队列
    auto equipAction = std::make_unique<EquipItemAction>(this, std::move(item));
    actionQueue->addAction(std::move(equipAction));
}

// 实现卸下物品方法 - 按槽位卸下
std::pair<float, std::unique_ptr<Item>> Entity::unequipItem(EquipSlot slot) {
    if (!equipmentSystem || !canPerformAction()) {
        return {0.0f, nullptr};
    }
    
    // 检查槽位是否有装备
    if (!equipmentSystem->isSlotEquipped(slot)) {
        return {0.0f, nullptr};
    }
    
    // 卸下物品
    auto [unequipTime, item] = equipmentSystem->unequipItem(slot);
    
    // 如果卸下时间大于0，设置实体状态为卸装备中
    if (unequipTime > 0.0f) {
        setState(EntityState::UNEQUIPPING, unequipTime);
    }
    
    return {unequipTime, std::move(item)};
}

// 实现卸下物品方法 - 按物品卸下
std::pair<float, std::unique_ptr<Item>> Entity::unequipItem(Item* item) {
    if (!equipmentSystem || !canPerformAction() || !item) {
        return {0.0f, nullptr};
    }
    
    // 检查物品是否已装备
    if (!equipmentSystem->isItemEquipped(item)) {
        return {0.0f, nullptr};
    }
    
    // 卸下物品
    auto [unequipTime, unequippedItem] = equipmentSystem->unequipItem(item);
    
    // 如果卸下时间大于0，设置实体状态为卸装备中
    if (unequipTime > 0.0f) {
        setState(EntityState::UNEQUIPPING, unequipTime);
    }
    
    return {unequipTime, std::move(unequippedItem)};
}

// 使用Action卸下装备 - 按槽位卸下
void Entity::unequipItemWithAction(EquipSlot slot, std::function<void(std::unique_ptr<Item>)> callback) {
    if (!equipmentSystem || !actionQueue || !canPerformAction()) {
        // 如果参数无效或实体无法执行操作，直接返回
        if (callback) {
            callback(nullptr);
        }
        return;
    }
    
    // 检查槽位是否有装备
    if (!equipmentSystem->isSlotEquipped(slot)) {
        // 如果槽位没有装备物品，直接返回
        if (callback) {
            callback(nullptr);
        }
        return;
    }
    
    // 创建卸下装备的行为并添加到队列
    auto unequipAction = std::make_unique<UnequipItemAction>(this, slot, callback);
    actionQueue->addAction(std::move(unequipAction));
}

// 使用Action按物品卸下装备
void Entity::unequipItemWithAction(Item* item, std::function<void(std::unique_ptr<Item>)> callback) {
    if (!equipmentSystem || !actionQueue || !canPerformAction() || !item) {
        // 如果参数无效或实体无法执行操作，直接返回
        if (callback) {
            callback(nullptr);
        }
        return;
    }
    
    // 检查物品是否已装备
    if (!equipmentSystem->isItemEquipped(item)) {
        // 如果物品未装备，直接返回
        if (callback) {
            callback(nullptr);
        }
        return;
    }
    
    // 创建一个新的Action类型来处理按物品卸下装备
    class UnequipItemByItemAction : public Action {
    private:
        Item* targetItem;
        std::function<void(std::unique_ptr<Item>)> callback;
        
    public:
        UnequipItemByItemAction(Entity* entity, Item* item, std::function<void(std::unique_ptr<Item>)> cb)
            : Action(entity, entity->getEquipmentSystem() ? entity->getEquipmentSystem()->calculateUnequipTime(item) : 0.0f, EntityState::UNEQUIPPING),
              targetItem(item), callback(cb) {}
        
        void start() override {
            Action::start();
        }
        
        void end() override {
            if (!owner) {
                if (callback) callback(nullptr);
                Action::end();
                return;
            }
            
            // 卸下物品并调用回调函数
            auto [_, item] = owner->unequipItem(targetItem);
            if (callback) callback(std::move(item));
            Action::end();
        }
        
        void interrupt() override {
            if (callback) callback(nullptr);
            Action::interrupt();
        }
    };
    
    // 创建卸下装备的行为并添加到队列
    auto unequipAction = std::make_unique<UnequipItemByItemAction>(this, item, callback);
    actionQueue->addAction(std::move(unequipAction));
}

// 实现获取所有可用存储空间的方法
std::vector<std::pair<EquipSlot, Storage*>> Entity::getAllAvailableStorages() const {
    if (!equipmentSystem) {
        return {};
    }
    
    // 调用装备系统的getAllStorages方法获取所有存储空间
    return equipmentSystem->getAllStorages();
}

// 实现新的存储相关方法

bool Entity::addItem(std::unique_ptr<Item> item) {
    if (!item || !equipmentSystem) {
        return false;
    }
    
    // 获取所有可用的存储空间
    auto storages = getAllAvailableStorages();
    if (storages.empty()) {
        return false; // 没有可用的存储空间
    }
    
    // 尝试将物品添加到第一个有足够空间的存储空间中
    for (const auto& [slot, storage] : storages) {
        if (storage->addItem(std::move(item))) {
            return true; // 添加成功
        }
    }
    
    return false; // 所有存储空间都无法容纳该物品
}

// 将物品存储到最大容量的存储空间中
bool Entity::storeItemInLargestStorage(std::unique_ptr<Item> item) {
    if (!item || !equipmentSystem) {
        return false;
    }
    
    // 获取所有可用的存储空间
    auto storages = getAllAvailableStorages();
    if (storages.empty()) {
        return false; // 没有可用的存储空间
    }
    
    // 找到容量最大的存储空间
    Storage* largestStorage = nullptr;
    float maxVolume = -1.0f;
    EquipSlot targetSlot = EquipSlot::NONE;
    
    for (const auto& [slot, storage] : storages) {
        float availableVolume = storage->getMaxVolume() - storage->getCurrentVolume();
        if (availableVolume > maxVolume && availableVolume >= item->getVolume()) {
            maxVolume = availableVolume;
            largestStorage = storage;
            targetSlot = slot;
        }
    }
    
    // 如果找到了合适的存储空间，将物品添加进去
    if (largestStorage) {
        return largestStorage->addItem(std::move(item));
    }
    
    return false; // 没有找到合适的存储空间
}

// 使用Action存储物品
void Entity::storeItemWithAction(std::unique_ptr<Item> item, Storage* storage) {
    if (!item || !storage || !actionQueue || !canPerformAction()) {
        return;
    }
    
    // 验证存储空间是否属于该实体
    bool isValidStorage = false;
    auto storages = getAllAvailableStorages();
    for (const auto& [slot, stor] : storages) {
        if (stor == storage) {
            isValidStorage = true;
            break;
        }
    }
    
    if (!isValidStorage) {
        return; // 存储空间不属于该实体
    }
    
    // 创建存储物品的行为并添加到队列
    actionQueue->addAction(std::make_unique<StoreItemAction>(this, std::move(item), storage));
}

// 使用Action取出物品
void Entity::takeItemWithAction(Item* item, Storage* storage, std::function<void(std::unique_ptr<Item>)> callback) {
    if (!item || !storage || !actionQueue || !canPerformAction()) {
        return;
    }
    
    // 验证存储空间是否属于该实体
    bool isValidStorage = false;
    auto storages = getAllAvailableStorages();
    for (const auto& [slot, stor] : storages) {
        if (stor == storage) {
            isValidStorage = true;
            break;
        }
    }
    
    if (!isValidStorage) {
        return; // 存储空间不属于该实体
    }
    
    // 验证物品是否在存储空间中
    bool itemFound = false;
    for (size_t i = 0; i < storage->getItemCount(); ++i) {
        if (storage->getItem(i) == item) {
            itemFound = true;
            break;
        }
    }
    
    if (!itemFound) {
        return; // 物品不在存储空间中
    }
    
    // 创建取出物品的行为并添加到队列
    actionQueue->addAction(std::make_unique<TakeItemAction>(this, item, storage, callback));
}

// 使用Action在两个存储空间之间转移物品
void Entity::transferItemWithAction(Item* item, Storage* sourceStorage, Storage* targetStorage, std::function<void(bool)> callback) {
    if (!item || !sourceStorage || !targetStorage || !actionQueue || !canPerformAction()) {
        // 如果参数无效或实体无法执行操作，直接返回
        if (callback) {
            callback(false);
        }
        return;
    }
    
    // 验证源存储空间和目标存储空间是否属于该实体
    bool isValidSourceStorage = false;
    bool isValidTargetStorage = false;
    auto storages = getAllAvailableStorages();
    
    for (const auto& [slot, storage] : storages) {
        if (storage == sourceStorage) {
            isValidSourceStorage = true;
        }
        if (storage == targetStorage) {
            isValidTargetStorage = true;
        }
    }
    
    if (!isValidSourceStorage || !isValidTargetStorage) {
        // 如果存储空间不属于该实体，直接返回
        if (callback) {
            callback(false);
        }
        return;
    }
    
    // 验证物品是否在源存储空间中
    bool itemFound = false;
    for (size_t i = 0; i < sourceStorage->getItemCount(); ++i) {
        if (sourceStorage->getItem(i) == item) {
            itemFound = true;
            break;
        }
    }
    
    if (!itemFound) {
        // 如果物品不在源存储空间中，直接返回
        if (callback) {
            callback(false);
        }
        return;
    }
    
    // 创建物品转移行为并添加到队列
    actionQueue->addAction(std::make_unique<TransferItemAction>(this, item, sourceStorage, targetStorage, callback));
}

// 取物品函数
std::unique_ptr<Item> Entity::takeItem(Item* item) {
    if (!item || !equipmentSystem) {
        return nullptr;
    }
    
    // 获取所有可用的存储空间
    auto storages = getAllAvailableStorages();
    if (storages.empty()) {
        return nullptr; // 没有可用的存储空间
    }
    
    // 在所有存储空间中查找该物品
    for (const auto& [slot, storage] : storages) {
        for (size_t i = 0; i < storage->getItemCount(); ++i) {
            if (storage->getItem(i) == item) {
                // 找到物品，从存储空间中移除
                return storage->removeItem(static_cast<int>(i));
            }
        }
    }
    
    return nullptr; // 没有找到该物品
}

std::unique_ptr<Item> Entity::removeItem(EquipSlot slot, Storage* storage, int index) {
    if (!storage || !equipmentSystem) {
        return nullptr;
    }
    
    // 验证存储空间是否属于该实体
    bool isValidStorage = false;
    auto storages = getAllAvailableStorages();
    for (const auto& [s, stor] : storages) {
        if (s == slot && stor == storage) {
            isValidStorage = true;
            break;
        }
    }
    
    if (!isValidStorage) {
        return nullptr; // 存储空间不属于该实体
    }
    
    // 从存储空间中移除物品
    return storage->removeItem(index);
}

size_t Entity::getTotalItemCount() const {
    if (!equipmentSystem) {
        return 0;
    }
    
    size_t totalCount = 0;
    auto storages = getAllAvailableStorages();
    for (const auto& [slot, storage] : storages) {
        totalCount += storage->getItemCount();
    }
    
    return totalCount;
}

std::vector<std::tuple<EquipSlot, Storage*, int, Item*>> Entity::findItemsByCategory(ItemFlag flag) const {
    std::vector<std::tuple<EquipSlot, Storage*, int, Item*>> result;
    if (!equipmentSystem) {
        return result;
    }
    
    auto storages = getAllAvailableStorages();
    for (const auto& [slot, storage] : storages) {
        auto indices = storage->findItemsByCategory(flag);
        for (int index : indices) {
            Item* item = storage->getItem(index);
            if (item) {
                result.emplace_back(slot, storage, index, item);
            }
        }
    }
    
    return result;
}

std::vector<std::tuple<EquipSlot, Storage*, int, Item*>> Entity::findItemsByName(const std::string& name) const {
    std::vector<std::tuple<EquipSlot, Storage*, int, Item*>> result;
    if (!equipmentSystem) {
        return result;
    }
    
    auto storages = getAllAvailableStorages();
    for (const auto& [slot, storage] : storages) {
        auto indices = storage->findItemsByName(name);
        for (int index : indices) {
            Item* item = storage->getItem(index);
            if (item) {
                result.emplace_back(slot, storage, index, item);
            }
        }
    }
    
    return result;
}

Magazine* Entity::findFullestMagazine(const std::vector<std::string>& compatibleTypes) const {
    if (!equipmentSystem) {
        return nullptr;
    }
    
    Magazine* fullestMag = nullptr;
    int maxAmmoCount = -1;
    
    // 查找所有弹匣
    auto magazineItems = findItemsByCategory(ItemFlag::MAGAZINE);
    
    for (const auto& [slot, storage, index, item] : magazineItems) {
        Magazine* mag = static_cast<Magazine*>(item);
        
        // 检查弹匣类型是否兼容
        bool isCompatible = false;
        for (const auto& magType : mag->getCompatibleAmmoTypes()) {
            for (const auto& gunType : compatibleTypes) {
                if (magType == gunType) {
                    isCompatible = true;
                    break;
                }
            }
            if (isCompatible) break;
        }
        
        if (isCompatible) {
            int currentAmmo = mag->getCurrentAmmoCount();
            if (currentAmmo > maxAmmoCount) {
                maxAmmoCount = currentAmmo;
                fullestMag = mag;
            }
        }
    }
    
    return fullestMag;
}

std::unique_ptr<Magazine> Entity::removeMagazine(Magazine* magazine) {
    if (!magazine || !equipmentSystem) {
        return nullptr;
    }
    
    // 查找所有弹匣
    auto magazineItems = findItemsByCategory(ItemFlag::MAGAZINE);
    
    for (const auto& [slot, storage, index, item] : magazineItems) {
        if (item == magazine) {
            // 从存储空间中移除弹匣
            std::unique_ptr<Item> removedItem = storage->removeItem(index);
            if (removedItem) {
                // 转换为Magazine类型并返回
                return std::unique_ptr<Magazine>(static_cast<Magazine*>(removedItem.release()));
            }
        }
    }
    
    return nullptr;
}

float Entity::getTotalStorageWeight() const {
    if (!equipmentSystem) {
        return 0.0f;
    }
    
    float totalWeight = 0.0f;
    auto storages = getAllAvailableStorages();
    for (const auto& [slot, storage] : storages) {
        totalWeight += storage->getCurrentWeight();
    }
    
    return totalWeight;
}

// 在Entity.cpp文件末尾添加以下方法实现

// 检查是否可以执行手上操作
bool Entity::canPerformAction() const {
    // 在以下状态下不能执行操作
    return !(isInState(EntityState::RELOADING) ||
             isInState(EntityState::UNLOADING) ||
             isInState(EntityState::CHAMBERING) ||
             isInState(EntityState::EQUIPPING) ||
             isInState(EntityState::UNEQUIPPING) ||
             isInState(EntityState::STORING_ITEM) ||
             isInState(EntityState::TAKING_ITEM) ||
             isInState(EntityState::DEAD));
}

// 检查是否可以移动
bool Entity::canMove() const {
    // 在以下状态下不能移动
    return !(isInState(EntityState::DEAD) ||
             isInState(EntityState::STUNNED));
}

// 更新状态计时器
void Entity::updateStateTimer(float deltaTime) {
    if (stateTimer > 0) {
        stateTimer -= deltaTime;
        if (stateTimer <= 0) {
            // 状态结束，恢复到IDLE状态
            stateTimer = 0;
            setState(EntityState::IDLE, 0);
        }
    }
}

// 更新射击冷却计时器
void Entity::updateShootCooldown() {
    if (shootCooldown > 0) {
        // 获取游戏实例以获取调整后的deltaTime
        Game* game = Game::getInstance();
        float adjustedDeltaTime = game ? game->getAdjustedDeltaTime() : (1.0f / 60.0f);
        
        // 减少冷却时间
        shootCooldown -= static_cast<int>(1000.0f * adjustedDeltaTime); // 转换为毫秒
        if (shootCooldown < 0) {
            shootCooldown = 0;
        }
    }
}

// 检查是否可以射击
bool Entity::canShoot(Gun* weapon) const {
    //if (!weapon || !canPerformAction() || !canShootByCooldown()) {
    //    return false;
    //}
    if (!weapon)
        return false;
    if (!canPerformAction())
        return false;
    if (!canShootByCooldown())
        return false;
    // 检查枪是否有子弹
    if( !weapon->canShoot()) weapon->chamberManually();
    return weapon->canShoot();
}

// 检查是否需要换弹
bool Entity::needsReload(Gun* weapon) const {
    if (!weapon) {
        return false;
    }
    
    // 如果没有弹匣或弹匣为空，需要换弹
    return !weapon->getCurrentMagazine() || weapon->getCurrentMagazine()->isEmpty();
}

// 自动维护武器状态,为其它生物准备
void Entity::maintainWeapon(Gun* weapon) {
    if (!weapon || !canPerformAction()) {
        return;
    }
    
    // 如果需要换弹
    if (needsReload(weapon)) {
        reloadWeaponAuto(weapon);
    }
}

// 向指定方向射击
Bullet* Entity::shootInDirection(Gun* weapon, float dirX, float dirY) {
    if (!canShoot(weapon)) {
        return nullptr;
    }
    
    // 获取游戏实例
    Game* game = Game::getInstance();
    if (!game) {
        return nullptr;
    }
    
    // 射击武器
    auto shotAmmo = weapon->shoot();
    if (!shotAmmo) {
        return nullptr; // 射击失败
    }
    
    // 设置射击冷却 - 修正射速计算
    // 将每分钟发数转换为毫秒/发
    // 例如：900发/分钟 = 60000毫秒/900发 = 66.67毫秒/发
    float fireRateInMs = weapon->getFireRate();
    shootCooldown = static_cast<int>(fireRateInMs);
    
    // 播放射击声音
    SoundManager::getInstance()->playSound("shoot_ar15");
    
    // 计算子弹起始位置（从实体中心稍微偏移）
    float bulletStartX = x + dirX * (static_cast<float>(radius) + 5.0f); // 偏移5个单位
    float bulletStartY = y + dirY * (static_cast<float>(radius) + 5.0f);
    
    // 计算最终子弹属性（子弹基础属性 + 枪械加成）
    int finalDamage = static_cast<int>(shotAmmo->getBaseDamage() + weapon->getDamageBonus());
    float finalBulletSpeed = shotAmmo->getBaseSpeed() + weapon->getBulletSpeedBonus();
    float finalRange = shotAmmo->getBaseRange() + weapon->getRangeBonus();
    float finalPenetration = shotAmmo->getBasePenetration() + weapon->getPenetrationBonus();
    
    std::string damageType = "shooting"; // 默认伤害类型为射击
    int penetrationValue = static_cast<int>(finalPenetration);
    
    // 创建子弹 - 使用计算后的最终属性和新的伤害系统
    return game->createBullet(bulletStartX, bulletStartY, dirX, dirY, finalBulletSpeed, this, finalDamage, damageType, penetrationValue, finalRange);
}

//// 手动上膛操作
//float Entity::chamberRound(Gun* weapon) {
//    if (!weapon || !canPerformAction()) {
//        return 0.0f;
//    }
//    
//    // 检查是否已经上膛
//    if (weapon->getChamberedRound()) {
//        return 0.0f; // 已经上膛，无需操作
//    }
//    
//    // 检查是否有弹匣且弹匣不为空
//    if (!weapon->getCurrentMagazine() || weapon->getCurrentMagazine()->isEmpty()) {
//        return 0.0f; // 无法上膛
//    }
//    
//    // 上膛操作时间
//    float chamberTime = 0.3f; // 秒
//    
//    // 设置状态为上膛中
//    setState(EntityState::CHAMBERING, chamberTime);
//    
//    // 执行上膛操作
//    weapon->chamberManually();
//    
//    // 播放上膛音效
//    SoundManager::getInstance()->playSound("bolt_release");
//    
//    return chamberTime;
//}

// 使用行为队列系统进行换弹操作
void Entity::reloadWeaponWithActions(Gun* weapon) {
    if (!weapon || !canPerformAction() || !equipmentSystem || !actionQueue) {
        return;
    }
    
    // 获取枪支兼容的弹药类型
    std::vector<std::string> compatibleTypes;
    if (weapon->getCurrentMagazine()) {
        compatibleTypes = weapon->getCurrentMagazine()->getCompatibleAmmoTypes();
    } else {
        // 如果没有弹匣，尝试从枪支获取兼容类型
        // 这里假设Gun类有一个方法可以获取兼容的弹药类型
        // compatibleTypes = weapon->getAcceptedAmmoTypes();
        return; // 无法确定兼容类型
    }
    
    // 在所有存储空间中查找最满的兼容弹匣
    Magazine* fullestMag = findFullestMagazine(compatibleTypes);
    if (!fullestMag) {
        return; // 没有找到兼容的弹匣
    }
    
    // 创建一系列行为并添加到队列
    
    // 1. 如果当前有弹匣，先卸下
    if (weapon->getCurrentMagazine()) {
        // 创建卸下弹匣的行为
        actionQueue->addAction(std::make_unique<UnloadMagazineAction>(this, weapon));
    }
    
    // 2. 装入新弹匣 - 从存储空间中移除弹匣并获取其所有权
    std::unique_ptr<Magazine> magPtr = removeMagazine(fullestMag);
    if (magPtr) {
        actionQueue->addAction(std::make_unique<LoadMagazineAction>(this, weapon, std::move(magPtr)));
    }
    
    // 3. 如果需要上膛，添加上膛行为
    if (!weapon->getChamberedRound()) {
        actionQueue->addAction(std::make_unique<ChamberRoundAction>(this, weapon));
    }
}

// 虚析构函数实现
Entity::~Entity() {
    // 清理ActionQueue，防止内存访问冲突
    if (actionQueue) {
        actionQueue->pause();
        actionQueue->clearActions();
    }
    
    // 清理状态管理器
    if (stateManager) {
        stateManager->clearStates();
    }
    
    // 清理资源
    // 注意：unique_ptr会自动清理资源
}

// 渲染方法实现
void Entity::render(SDL_Renderer* renderer, float cameraX, float cameraY) {
    // 计算屏幕坐标
    int screenX = static_cast<int>(x - cameraX);
    int screenY = static_cast<int>(y - cameraY);
    
    // 设置渲染颜色
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    
    // 绘制实体（简单圆形）
    for (int w = -radius; w <= radius; w++) {
        for (int h = -radius; h <= radius; h++) {
            if (w * w + h * h <= radius * radius) {
                SDL_RenderPoint(renderer, screenX + w, screenY + h);
            }
        }
    }
    
    // 可以添加额外的渲染逻辑，如状态指示器等
}

// 旧的碰撞检测方法已删除，使用新的物理系统

// 处理与其他实体的碰撞 - 使用新的物理系统
void Entity::resolveCollision(Entity* other) {
    CollisionInfo info;
    if (checkCollisionWith(other, info)) {
        separateFromEntity(other, info);
    }
}

// 受伤方法
bool Entity::takeDamage(const Damage& damage) {
    // 如果已经死亡，不再受伤
    if (health <= 0) {
        return false;
    }
    
    // 计算总伤害值
    int totalDamage = 0;
    
    // 处理每种类型的伤害
    for (const auto& damageInfo : damage.getDamageList()) {
        std::string type = std::get<0>(damageInfo);
        int amount = std::get<1>(damageInfo);
        int penetration = std::get<2>(damageInfo);
        
        // 根据不同伤害类型应用不同的防御计算
        float damageReduction = 0.0f;
        
        // 这里可以根据实体的防御属性和伤害类型计算伤害减免
        // 例如，如果实体有装甲，可以减少某些类型的伤害
        
        // 暂时简单处理，不同类型的伤害有不同的基础减免
        if (type == "shooting") {
            // 射击伤害
            damageReduction = 0.1f; // 10%减免
        } else if (type == "blunt") {
            // 钝击伤害
            damageReduction = 0.2f; // 20%减免
        } else if (type == "slash") {
            // 斩击伤害
            damageReduction = 0.15f; // 15%减免
        } else if (type == "pierce") {
            // 刺击伤害
            damageReduction = 0.05f; // 5%减免
        } else if (type == "pure") {
            // 纯粹伤害无减免
            damageReduction = 0.0f;
        } else {
            // 其他类型的伤害默认有一定减免
            damageReduction = 0.1f;
        }
        
        // 如果有穿透值，减少伤害减免效果
        if (penetration > 0) {
            // 穿透值越高，减免效果越低
            damageReduction = std::max(0.0f, damageReduction - (penetration * 0.01f));
        }
        
        // 计算实际伤害
        int actualDamage = static_cast<int>(amount * (1.0f - damageReduction));
        
        // 累加总伤害
        totalDamage += actualDamage;
        
        // 根据伤害类型添加特定状态效果
        if (type == "electric" && stateManager) {
            // 电击伤害可能导致眩晕
            if (std::rand() % 100 < 30) { // 30%几率
                stateManager->addState(EntityStateEffect::Type::STUNNED, "electrocuted", 1000); // 1秒眩晕
            }
        } else if (type == "burn" && stateManager) {
            // 灼烧伤害可能导致持续伤害
            if (std::rand() % 100 < 50) { // 50%几率
                // 添加灼烧状态（这里假设有BURNING类型）
                // 如果没有，可以使用DEBUFFED类型
                stateManager->addState(EntityStateEffect::Type::DEBUFFED, "burning", 3000); // 3秒灼烧
            }
        }
    }
    
    // 应用伤害
    health -= totalDamage;
    
    // 生成伤害数字飘字（如果造成了伤害）
    if (totalDamage > 0) {
        // 暴击判断逻辑：基于伤害值的简单判断
        bool isCritical = false;
        
        // 高伤害或精准度高时判定为暴击
        if (totalDamage >= 40) { // 伤害>=40认为是暴击
            isCritical = true;
        } else if (damage.getSource() && damage.getPrecision() > 0.9f) {
            // 或者精准度极高时也是暴击
            isCritical = true;
        }
        
        // 添加飘字数字到游戏系统
        Game* gameInstance = Game::getInstance();
        if (gameInstance) {
            // 在实体位置稍微偏上一点生成飘字
            gameInstance->addDamageNumber(x, y - radius - 10, totalDamage, isCritical);
            
            // 如果受伤的是玩家，触发受伤屏幕效果
            if (this == gameInstance->getPlayer()) {
                float intensity = std::min(1.0f, totalDamage / 100.0f); // 根据伤害计算强度
                gameInstance->triggerHurtEffect(intensity);
            }
        }
    }
    
    // 如果生命值降至0或以下，实体死亡
    if (health <= 0) {
        health = 0;
        // 设置死亡状态
        setState(EntityState::DEAD, -1); // -1表示永久状态
        
        // 清除所有状态效果
        if (stateManager) {
            stateManager->clearStates();
    }
    
        return true; // 返回true表示实体已死亡
    }
    
    return false; // 返回false表示实体仍然存活
}

// 获取武器状态信息
std::string Entity::getWeaponStatus(Gun* weapon) const {
    if (!weapon) {
        return "No weapon";
    }

    std::string status = "Weapon: " + weapon->getName() + "\n";

    // 弹匣状态
    Magazine* currentMag = weapon->getCurrentMagazine();
    if (currentMag) {
        status += "Magazine: " + std::to_string(currentMag->getCurrentAmmoCount()) + "/" + std::to_string(currentMag->getCapacity()) + "\n";
    }
    else {
        status += "Magazine: None\n";
    }

    // 膛内子弹状态
    if (weapon->getChamberedRound()) {
        status += "Chambered: Yes\n";
    }
    else {
        status += "Chambered: No\n";
    }

    // 射击冷却状态
    if (shootCooldown > 0) {
        status += "Cooldown: " + std::to_string(shootCooldown) + "ms\n";
    }
    else {
        status += "Ready to fire\n";
    }

    return status;
}

// ========== 物理引擎实现 ==========

// 物理更新主方法 - 简化版本，无惯性无加速度
void Entity::updatePhysics(float deltaTime) {
    if (isStatic) return;
    
    // 清空上一帧的碰撞信息
    collisions.clear();
    
    // 1. 直接使用期望速度，应用速度修正系数
    float maxSpeed = speed * speedModifier;
    
    // 计算期望速度的方向和大小
    float desiredSpeed = std::sqrt(desiredVelocityX * desiredVelocityX + desiredVelocityY * desiredVelocityY);
    if (desiredSpeed > maxSpeed) {
        // 限制期望速度不超过最大速度
        float scale = maxSpeed / desiredSpeed;
        desiredVelocityX *= scale;
        desiredVelocityY *= scale;
    }
    
    // 2. 直接设置当前速度为期望速度（无惯性）
    velocityX = desiredVelocityX;
    velocityY = desiredVelocityY;
    
    // 3. 预测新位置
    float newX = x + velocityX * deltaTime;
    float newY = y + velocityY * deltaTime;
    
    // 4. 检查地形碰撞
    checkTerrainCollisionAtPosition(newX, newY);
    
    // 5. 更新位置
    prevX = x;
    prevY = y;
    x = newX;
    y = newY;
    
    // 6. 更新碰撞体位置
    collider.updatePosition(x, y);
}

// 应用外力 - 简化版本，直接设置期望速度
void Entity::applyForce(float forceX, float forceY) {
    if (isStatic) return;
    
    // 无惯性系统，直接设置期望速度
    setDesiredVelocity(forceX, forceY);
}

// 检查与其他实体的碰撞
bool Entity::checkCollisionWith(Entity* other, CollisionInfo& info) {
    if (!other || other == this) return false;
    
    // 计算距离
    float dx = x - other->getX();
    float dy = y - other->getY();
    float distance = std::sqrt(dx * dx + dy * dy);
    
    // 计算最小分离距离
    float minDistance = radius + other->getRadius();
    
    if (distance < minDistance) {
        // 发生碰撞
        info.other = other;
        info.penetrationDepth = minDistance - distance;
        
        // 计算碰撞法向量
        if (distance > 0.01f) {
            info.normalX = dx / distance;
            info.normalY = dy / distance;
        } else {
            // 实体完全重叠，随机选择分离方向
            float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159f;
            info.normalX = std::cos(angle);
            info.normalY = std::sin(angle);
        }
        
        // 计算接触点
        info.contactX = x - info.normalX * radius;
        info.contactY = y - info.normalY * radius;
        
        return true;
    }
    
    return false;
}

// 与其他实体分离 - 考虑重量和力量属性
void Entity::separateFromEntity(Entity* other, const CollisionInfo& info) {
    if (!other || isStatic) return;
    
    // 计算基础分离距离
    float baseSeparationDistance = info.penetrationDepth;
    
    if (other->getIsStatic()) {
        // 如果对方是静态的，我完全分离
        x += info.normalX * baseSeparationDistance;
        y += info.normalY * baseSeparationDistance;
        collider.updatePosition(x, y);
        return;
    }
    
    // 计算推开能力和抗推能力
    float myPushPower = calculatePushPower();
    float otherPushPower = other->calculatePushPower();
    float myResistance = calculatePushResistance();
    float otherResistance = other->calculatePushResistance();
    
    // 计算净推力差异
    float myNetForce = myPushPower - otherResistance;
    float otherNetForce = otherPushPower - myResistance;
    
    // 确保净推力不为负数
    myNetForce = std::max(0.0f, myNetForce);
    otherNetForce = std::max(0.0f, otherNetForce);
    
    // 计算总净推力
    float totalNetForce = myNetForce + otherNetForce;
    
    if (totalNetForce <= 0.0f) {
        // 如果没有净推力，双方都无法移动对方，平均分离
        float halfSeparation = baseSeparationDistance * 0.5f;
        x += info.normalX * halfSeparation;
        y += info.normalY * halfSeparation;
        
        other->x -= info.normalX * halfSeparation;
        other->y -= info.normalY * halfSeparation;
    } else {
        // 根据净推力比例分配分离距离
        float myRatio = myNetForce / totalNetForce;
        float otherRatio = otherNetForce / totalNetForce;
        
        // 我推开对方的距离
        float iPushOther = baseSeparationDistance * myRatio;
        // 对方推开我的距离
        float otherPushMe = baseSeparationDistance * otherRatio;
        
        // 调试信息（可选）
        #ifdef _DEBUG
        if (totalNetForce > 0.1f) {  // 只在有明显推力差异时输出
            // std::cout << "碰撞分离: 我推开=" << iPushOther << ", 被推开=" << otherPushMe 
            //           << " (推开能力:" << myPushPower << " vs " << otherPushPower 
            //           << ", 抗推:" << myResistance << " vs " << otherResistance << ")" << std::endl;
        }
        #endif
        
        // 应用分离
        x += info.normalX * otherPushMe;  // 我被推开
        y += info.normalY * otherPushMe;
        
        other->x -= info.normalX * iPushOther;  // 对方被推开
        other->y -= info.normalY * iPushOther;
    }
    
    // 更新碰撞体位置
    collider.updatePosition(x, y);
    other->collider.updatePosition(other->x, other->y);
}

// 检查地形碰撞
void Entity::checkTerrainCollisionAtPosition(float& newX, float& newY) {
    Game* game = Game::getInstance();
    if (!game || !game->getMap()) return;
    
    // 创建临时碰撞箱
    std::unique_ptr<Collider> tempCollider;
    if (collider.getType() == ColliderType::CIRCLE) {
        tempCollider = std::make_unique<Collider>(
            newX, newY, collider.getRadius(), 
            "temp_physics", ColliderPurpose::ENTITY, 0);
    } else {
        tempCollider = std::make_unique<Collider>(
            newX, newY, collider.getWidth(), collider.getHeight(), 
            "temp_physics", ColliderPurpose::ENTITY, 0);
    }
    
    // 获取可能影响的tile范围
    int minTileX = static_cast<int>((newX - radius) / 64);
    int maxTileX = static_cast<int>((newX + radius) / 64);
    int minTileY = static_cast<int>((newY - radius) / 64);
    int maxTileY = static_cast<int>((newY + radius) / 64);
    
    // 检查与地形的碰撞
    for (int tileX = minTileX; tileX <= maxTileX; tileX++) {
        for (int tileY = minTileY; tileY <= maxTileY; tileY++) {
            Tile* tile = game->getMap()->getTileAt(tileX * 64, tileY * 64);
            if (tile && tile->hasColliderWithPurpose(ColliderPurpose::TERRAIN)) {
                auto terrainColliders = tile->getCollidersByPurpose(ColliderPurpose::TERRAIN);
                for (Collider* terrainCollider : terrainColliders) {
                    if (tempCollider->intersects(*terrainCollider)) {
                        // 发生地形碰撞，计算分离
                        resolveTerrainCollision(newX, newY, *terrainCollider);
                    }
                }
            }
        }
    }
}

// 解决地形碰撞
void Entity::resolveTerrainCollision(float& newX, float& newY, const Collider& terrainCollider) {
    // 计算重叠区域
    float overlapLeft = (newX + radius) - (terrainCollider.getX() - terrainCollider.getWidth() / 2);
    float overlapRight = (terrainCollider.getX() + terrainCollider.getWidth() / 2) - (newX - radius);
    float overlapTop = (newY + radius) - (terrainCollider.getY() - terrainCollider.getHeight() / 2);
    float overlapBottom = (terrainCollider.getY() + terrainCollider.getHeight() / 2) - (newY - radius);
    
    // 找到最小重叠方向
    float minOverlap = std::min({overlapLeft, overlapRight, overlapTop, overlapBottom});
    
    if (minOverlap > 0) {
        if (minOverlap == overlapLeft) {
            // 从左侧分离
            newX = terrainCollider.getX() - terrainCollider.getWidth() / 2 - radius - 1;
        } else if (minOverlap == overlapRight) {
            // 从右侧分离
            newX = terrainCollider.getX() + terrainCollider.getWidth() / 2 + radius + 1;
        } else if (minOverlap == overlapTop) {
            // 从上方分离
            newY = terrainCollider.getY() - terrainCollider.getHeight() / 2 - radius - 1;
        } else if (minOverlap == overlapBottom) {
            // 从下方分离
            newY = terrainCollider.getY() + terrainCollider.getHeight() / 2 + radius + 1;
        }
    }
}

// 计算推开能力 - 基于力量属性和装备
float Entity::calculatePushPower() const {
    // 基础推开能力基于力量属性
    float basePushPower = static_cast<float>(strength);
    
    // 考虑装备系统的影响
    float equipmentBonus = 0.0f;
    if (equipmentSystem) {
        // 检查是否装备了增强力量的物品
        // 例如：外骨骼、力量增强器等
        
        // 获取所有装备槽位
        for (int slot = static_cast<int>(EquipSlot::HEAD); 
             slot <= static_cast<int>(EquipSlot::BACK); 
             ++slot) {
            EquipSlot equipSlot = static_cast<EquipSlot>(slot);
            Item* equippedItem = equipmentSystem->getEquippedItem(equipSlot);
            
            if (equippedItem) {
                // 检查装备是否有力量加成标志
                if (equippedItem->hasFlag(ItemFlag::STRENGTH_BOOST)) {
                    equipmentBonus += 5.0f; // 每件力量装备+5推开能力
                }
                // 重型装备会增加推开能力但减少速度
                if (equippedItem->hasFlag(ItemFlag::HEAVY)) {
                    equipmentBonus += 3.0f;
                }
            }
        }
    }
    
    // 考虑状态效果
    float stateModifier = 1.0f;
    if (stateManager) {
        // 眩晕状态大幅降低推开能力
        if (stateManager->hasState(EntityStateEffect::Type::STUNNED)) {
            stateModifier *= 0.1f;
        }
        // 增益状态可能提升推开能力
        if (stateManager->hasState(EntityStateEffect::Type::BUFFED)) {
            stateModifier *= 1.5f;
        }
    }
    
    // 考虑生命值影响（受伤会降低推开能力）
    float healthRatio = static_cast<float>(health) / 100.0f; // 假设满血是100
    float healthModifier = 0.5f + 0.5f * healthRatio; // 生命值影响50%-100%的推开能力
    
    return (basePushPower + equipmentBonus) * stateModifier * healthModifier;
}

// 计算抗推能力 - 基于重量和稳定性
float Entity::calculatePushResistance() const {
    // 基础抗推能力基于重量
    float baseResistance = weight;
    
    // 考虑体型影响（半径越大越难推动）
    float sizeBonus = static_cast<float>(radius) * 0.5f;
    
    // 考虑装备重量
    float equipmentWeight = 0.0f;
    if (equipmentSystem) {
        equipmentWeight = equipmentSystem->getTotalEquipmentWeight();
    }
    
    // 考虑敏捷属性（敏捷高的实体更容易被推动）
    float agilityPenalty = static_cast<float>(dexterity) * 0.3f;
    
    // 考虑状态效果
    float stateModifier = 1.0f;
    if (stateManager) {
        // 眩晕状态降低抗推能力
        if (stateManager->hasState(EntityStateEffect::Type::STUNNED)) {
            stateModifier *= 0.5f;
        }
        // 防御状态提高抗推能力
        if (stateManager->hasState(EntityStateEffect::Type::BUFFED)) {
            stateModifier *= 1.3f;
        }
    }
    
    // 考虑地面类型影响（未来可扩展）
    float groundModifier = 1.0f;
    // 在冰面上抗推能力降低，在泥地上抗推能力提高等
    
    // 最终抗推能力 = (基础重量 + 装备重量 + 体型加成 - 敏捷惩罚) * 状态修正 * 地面修正
    float totalResistance = (baseResistance + equipmentWeight + sizeBonus - agilityPenalty) * stateModifier * groundModifier;
    
    // 确保抗推能力不为负数
    return std::max(1.0f, totalResistance);
}
