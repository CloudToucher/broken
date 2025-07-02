#include "Storage.h"
#include <algorithm>
#include <map>
#include <iostream>

Storage::Storage(const std::string& name, float maxW, float maxV, float maxL, float accessT, int maxItems, bool expandsWithContents)
    : name(name), maxWeight(maxW), maxVolume(maxV), maxLength(maxL), accessTime(accessT), 
      currentWeight(0.0f), currentVolume(0.0f), maxItems(maxItems), storageTime(0.0f), isCollapsed(false),
      expandsWithContents(expandsWithContents) {
}

// 实现析构函数
Storage::~Storage() {
    // items会在unique_ptr被销毁时自动清理
}

// 实现拷贝构造函数
Storage::Storage(const Storage& other)
    : name(other.name),
      maxWeight(other.maxWeight),
      maxVolume(other.maxVolume),
      maxLength(other.maxLength),
      accessTime(other.accessTime),
      currentWeight(other.currentWeight),
      currentVolume(other.currentVolume),
      maxItems(other.maxItems),
      storageTime(other.storageTime),
      isCollapsed(other.isCollapsed),
      expandsWithContents(other.expandsWithContents) {
    // 深拷贝items向量中的每个Item
    for (const auto& item : other.items) {
        if (item) {
            items.push_back(std::unique_ptr<Item>(item->clone()));
        }
    }
}

// 实现拷贝赋值运算符
Storage& Storage::operator=(const Storage& other) {
    if (this != &other) {
        name = other.name;
        maxWeight = other.maxWeight;
        maxVolume = other.maxVolume;
        maxLength = other.maxLength;
        accessTime = other.accessTime;
        currentWeight = other.currentWeight;
        currentVolume = other.currentVolume;
        maxItems = other.maxItems;
        storageTime = other.storageTime;
        isCollapsed = other.isCollapsed;
        expandsWithContents = other.expandsWithContents;
        
        // 清空当前items
        items.clear();
        
        // 深拷贝items向量中的每个Item
        for (const auto& item : other.items) {
            if (item) {
                items.push_back(std::unique_ptr<Item>(item->clone()));
            }
        }
    }
    return *this;
}

bool Storage::addItem(std::unique_ptr<Item> item) {
    if (!item) return false;
    
    // 如果物品可堆叠，尝试与现有物品合并
    if (item->isStackable()) {
        // 查找名称相同的现有物品进行合并
        for (auto& existingItem : items) {
            if (existingItem && existingItem->isStackable() && 
                existingItem->getName() == item->getName() && 
                !existingItem->isStackFull()) {
                
                // 计算可以添加的数量
                int toAdd = item->getStackSize();
                int actuallyAdded = existingItem->addToStack(toAdd);
                
                if (actuallyAdded > 0) {
                    // 减少要添加物品的数量
                    item->removeFromStack(actuallyAdded);
                    
                    // 如果全部合并了，更新容器大小并返回成功
                    if (item->getStackSize() <= 0) {
                        updateContainerSize();
                        return true;
                    }
                    // 如果还有剩余，继续寻找其他可合并的物品
                }
            }
        }
        
        // 如果还有物品没有合并完，检查是否可以添加新物品
        if (item->getStackSize() > 0) {
            if (!canFitItem(item.get())) {
                return false;
            }
            items.push_back(std::move(item));
        }
    } else {
        // 非堆叠物品直接添加
        if (!canFitItem(item.get())) {
            return false;
        }
        items.push_back(std::move(item));
    }
    
    // 更新容器的当前重量和体积
    updateContainerSize();
    
    return true;
}

std::unique_ptr<Item> Storage::removeItem(int index) {
    if (index < 0 || index >= static_cast<int>(items.size())) {
        return nullptr;
    }

    std::unique_ptr<Item> removedItem = std::move(items[index]);
    items.erase(items.begin() + index);
    
    // 更新容器的当前重量和体积
    updateContainerSize();
    
    return removedItem;
}

size_t Storage::getItemCount() const {
    return items.size();
}

Item* Storage::getItem(int index) const {
    if (index < 0 || index >= static_cast<int>(items.size())) {
        return nullptr;
    }
    return items[index].get();
}

std::vector<int> Storage::findItemsByCategory(ItemFlag flag) const {
    std::vector<int> result;
    for (size_t i = 0; i < items.size(); ++i) {
        if (items[i]->hasFlag(flag)) {
            result.push_back(static_cast<int>(i));
        }
    }
    return result;
}

std::vector<int> Storage::findItemsByName(const std::string& name) const {
    std::vector<int> result;
    for (size_t i = 0; i < items.size(); ++i) {
        if (items[i]->getName() == name) {
            result.push_back(static_cast<int>(i));
        }
    }
    return result;
}

float Storage::getCurrentWeight() const {
    return currentWeight;
}

float Storage::getMaxWeight() const {
    return maxWeight;
}

void Storage::setMaxWeight(float newMaxWeight) {
    maxWeight = newMaxWeight;
}

float Storage::getCurrentVolume() const {
    return currentVolume;
}

float Storage::getMaxVolume() const {
    return maxVolume;
}

void Storage::setMaxVolume(float newMaxVolume) {
    maxVolume = newMaxVolume;
}

float Storage::getMaxLength() const {
    return maxLength;
}

void Storage::setMaxLength(float newMaxLength) {
    maxLength = newMaxLength;
}

float Storage::getAccessTime() const {
    return accessTime;
}

void Storage::setAccessTime(float newAccessTime) {
    accessTime = newAccessTime;
}

std::string Storage::getName() const {
    return name;
}

void Storage::setName(const std::string& newName) {
    name = newName;
}

int Storage::getMaxItems() const {
    return maxItems;
}

void Storage::setMaxItems(int newMaxItems) {
    maxItems = newMaxItems;
}

float Storage::getStorageTime() const {
    return storageTime;
}

void Storage::setStorageTime(float time) {
    storageTime = time;
}

bool Storage::getIsCollapsed() const {
    return isCollapsed;
}

void Storage::setIsCollapsed(bool collapsed) {
    isCollapsed = collapsed;
}

bool Storage::getExpandsWithContents() const {
    return expandsWithContents;
}

void Storage::setExpandsWithContents(bool expands) {
    expandsWithContents = expands;
    // 注意：expandsWithContents标志只影响显示的体积，不会改变maxVolume
}

void Storage::updateContainerSize() {
    // 重新计算当前重量和体积
    currentWeight = 0.0f;
    currentVolume = 0.0f;
    
    for (const auto& item : items) {
        if (item) {
            currentWeight += item->getTotalWeight();
            currentVolume += item->getVolume();
        }
    }
    
    // 注意：expandsWithContents标志只影响显示的体积，不会改变maxVolume
}

bool Storage::canFitItem(const Item* item) const {
    if (!item) return false;
    
    // 检查是否达到最大物品数量限制
    if (maxItems != -1 && items.size() >= maxItems) {
        return false;
    }
    
    float itemWeight = item->getTotalWeight();
    float itemVolume = item->getVolume();
    float itemLength = item->getLength();
    
    // 检查是否超过重量限制
    if (currentWeight + itemWeight > maxWeight) {
        return false;
    }
    
    // 检查是否超过体积限制
    if (currentVolume + itemVolume > maxVolume) {
        return false;
    }
    
    // 检查是否超过长度限制
    if (itemLength > maxLength) {
        return false;
    }
    
    // 检查存储空间不是物品本身的存储空间
    for (size_t i = 0; i < item->getStorageCount(); i++) {
        if (item->getStorage(i) == this) {
            return false;
        }
    }
    
    return true;
}

// 堆叠相关方法实现
bool Storage::tryStackItem(std::unique_ptr<Item>& item) {
    if (!item || !item->isStackable()) {
        return false;
    }
    
    // 查找可以堆叠的现有物品
    for (auto& existingItem : items) {
        if (existingItem && existingItem->canStackWith(item.get()) && !existingItem->isStackFull()) {
            int toAdd = item->getStackSize();
            int actuallyAdded = existingItem->addToStack(toAdd);
            
            if (actuallyAdded > 0) {
                item->removeFromStack(actuallyAdded);
                
                // 如果全部堆叠完毕
                if (item->getStackSize() <= 1) {
                    item.reset(); // 释放物品
                    updateContainerSize();
                    return true;
                }
            }
        }
    }
    
    return false; // 没有找到可堆叠的物品或堆叠未完成
}

std::vector<int> Storage::findStackableItems(const Item* item) const {
    std::vector<int> result;
    if (!item || !item->isStackable()) {
        return result;
    }
    
    for (size_t i = 0; i < items.size(); ++i) {
        if (items[i] && items[i]->canStackWith(item) && !items[i]->isStackFull()) {
            result.push_back(static_cast<int>(i));
        }
    }
    
    return result;
}

// 整理合并同名物品
void Storage::consolidateItems() {
    std::cout << "开始整理合并物品..." << std::endl;
    int originalCount = items.size();
    
    // 使用map来按名称分组物品
    std::map<std::string, std::vector<std::unique_ptr<Item>>> itemGroups;
    
    // 将所有物品按名称分组
    for (auto& item : items) {
        if (item) {
            itemGroups[item->getName()].push_back(std::move(item));
        }
    }
    
    // 清空原items列表
    items.clear();
    
    int mergedCount = 0;
    
    // 处理每个分组
    for (auto& [itemName, itemList] : itemGroups) {
        if (itemList.empty()) continue;
        
        // 检查第一个物品是否可堆叠
        bool canStack = itemList[0]->isStackable();
        
        if (canStack && itemList.size() > 1) {
            std::cout << "合并物品: " << itemName << " (共" << itemList.size() << "个)";
            
            // 计算总数量
            int totalStack = 0;
            for (auto& item : itemList) {
                totalStack += item->getStackSize();
            }
            
            std::cout << " -> 总数量: " << totalStack << std::endl;
            
            // 取第一个物品作为基础，设置最大堆叠数
            auto& baseItem = itemList[0];
            int maxStackSize = baseItem->getMaxStackSize();
            bool baseItemUsed = false;
            
            // 如果总数量超过最大堆叠数，需要分成多个堆叠
            while (totalStack > 0) {
                int currentStackSize = std::min(totalStack, maxStackSize);
                
                if (!baseItemUsed) {
                    // 使用第一个物品
                    baseItem->setStackSize(currentStackSize);
                    items.push_back(std::move(baseItem));
                    baseItemUsed = true;
                } else {
                    // 创建新的物品副本（使用clone方法确保正确的类型）
                    auto newItem = std::unique_ptr<Item>(items.back()->clone());
                    newItem->setStackSize(currentStackSize);
                    items.push_back(std::move(newItem));
                }
                
                totalStack -= currentStackSize;
            }
            
            mergedCount += itemList.size();
        } else {
            // 不可堆叠或只有一个物品，直接添加
            for (auto& item : itemList) {
                items.push_back(std::move(item));
            }
        }
    }
    
    // 更新容器大小
    updateContainerSize();
    
    if (mergedCount > 0) {
        std::cout << "整理完成: " << originalCount << " -> " << items.size() << " 个物品" << std::endl;
    } else {
        std::cout << "无需整理，当前物品数: " << items.size() << std::endl;
    }
}