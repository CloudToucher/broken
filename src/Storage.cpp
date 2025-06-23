#include "Storage.h"
#include <algorithm>

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
    
    // 使用canFitItem方法进行统一检查
    if (!canFitItem(item.get())) {
        return false;
    }
    
    // 添加物品
    items.push_back(std::move(item));
    
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