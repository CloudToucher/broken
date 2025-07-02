#pragma once
#ifndef STORAGE_H
#define STORAGE_H

#include <vector>
#include <memory>
#include "Item.h"

class Storage {
private:
    std::string name;     // 存储空间名称
    float maxWeight;      // 最大重量
    float maxVolume;      // 最大空间/体积
    float maxLength;      // 最大长度
    float accessTime;     // 存取耗时
    float currentWeight;  // 当前重量
    float currentVolume;  // 当前体积
    int maxItems;         // 最大物品数量，-1表示无限制，1表示只能存放一个物品
    float storageTime;    // 存储时间，单位是秒
    bool isCollapsed;     // 是否折叠显示
    bool expandsWithContents; // 是否显示当前内容物体积（不改变最大容量）
    
    std::vector<std::unique_ptr<Item>> items; // 存储的物品

public:
    Storage(const std::string& name = "存储空间", float maxW = 10.0f, float maxV = 10.0f, float maxL = 10.0f, float accessT = 1.0f, int maxItems = -1, bool expandsWithContents = false);
    ~Storage();
    
    // 拷贝构造函数
    Storage(const Storage& other);
    
    // 拷贝赋值运算符
    Storage& operator=(const Storage& other);
    
    // 添加物品到存储空间
    bool addItem(std::unique_ptr<Item> item);
    
    // 从存储空间移除物品
    std::unique_ptr<Item> removeItem(int index);
    
    // 获取存储空间中的物品数量
    size_t getItemCount() const;
    
    // 获取存储空间中指定索引的物品
    Item* getItem(int index) const;
    
    // 查找特定类型的物品
    std::vector<int> findItemsByCategory(ItemFlag flag) const;
    
    // 查找特定名称的物品
    std::vector<int> findItemsByName(const std::string& name) const;
    
    // 获取当前重量
    float getCurrentWeight() const;
    
    // 获取最大重量
    float getMaxWeight() const;
    
    // 设置最大重量
    void setMaxWeight(float maxWeight);
    
    // 获取当前体积
    float getCurrentVolume() const;
    
    // 获取最大体积
    float getMaxVolume() const;
    
    // 设置最大体积
    void setMaxVolume(float maxVolume);
    
    // 获取最大长度
    float getMaxLength() const;
    
    // 设置最大长度
    void setMaxLength(float maxLength);
    
    // 获取存取耗时
    float getAccessTime() const;
    
    // 设置存取耗时
    void setAccessTime(float accessTime);
    
    // 获取存储空间名称
    std::string getName() const;
    
    // 设置存储空间名称
    void setName(const std::string& name);
    
    // 获取最大物品数量
    int getMaxItems() const;
    
    // 设置最大物品数量
    void setMaxItems(int maxItems);
    
    // 获取存储时间
    float getStorageTime() const;
    
    // 设置存储时间
    void setStorageTime(float time);
    
    // 获取是否折叠显示
    bool getIsCollapsed() const;
    
    // 设置是否折叠显示
    void setIsCollapsed(bool collapsed);
    
    // 获取是否显示当前内容物体积（不改变最大容量）
    bool getExpandsWithContents() const;
    
    // 设置是否显示当前内容物体积（不改变最大容量）
    void setExpandsWithContents(bool expands);
    
    // 更新容器大小（基于内容物）
    void updateContainerSize();
    
    // 检查是否可以容纳指定物品
    bool canFitItem(const Item* item) const;
    
    // 堆叠相关方法
    bool tryStackItem(std::unique_ptr<Item>& item);         // 尝试将物品堆叠到现有物品上
    std::vector<int> findStackableItems(const Item* item) const; // 查找可以堆叠的物品索引
    void consolidateItems();                                 // 整理合并同名物品
};

#endif // STORAGE_H