#pragma once
#ifndef MAGAZINE_H
#define MAGAZINE_H

#include "Item.h"
#include "Ammo.h"
#include <vector>
#include <string>
#include <stack>
#include <memory>

class Magazine : public Item {
private:
    std::vector<std::string> compatibleAmmoTypes;  // 兼容的弹药类型列表
    int maxCapacity;                             // 最大容量（发数）
    std::stack<std::unique_ptr<Ammo>> currentAmmo; // 当前装填的弹药
    float unloadTime;                            // 卸载时间（秒）
    float reloadTime;                            // 装填时间（秒）
public:
    // 简化构造函数
    Magazine(const std::string& itemName);
    
    // 拷贝构造函数
    Magazine(const Magazine& other);
    
    // 拷贝赋值运算符
    Magazine& operator=(const Magazine& other);
             
    const std::vector<std::string>& getCompatibleAmmoTypes() const;
    bool isEmpty() const;
    bool isFull() const;
    void use() override;
    float getUnloadTime() const;
    float getReloadTime() const;
    bool canAcceptAmmo(const std::string& ammoType) const;
    bool loadAmmo(std::unique_ptr<Ammo> ammo);
    std::unique_ptr<Ammo> consumeAmmo();
    int getCurrentAmmoCount() const;
    int getCapacity() const;
    
    // 设置弹匣属性
    void setCapacity(int capacity) { maxCapacity = capacity; }
    void setCompatibleAmmoTypes(const std::vector<std::string>& ammoTypes) { compatibleAmmoTypes = ammoTypes; }
    void setUnloadTime(float time) { unloadTime = time; }
    void setReloadTime(float time) { reloadTime = time; }
    
    // 重写克隆方法，用于创建Magazine的深拷贝
    Item* clone() const override {
        return new Magazine(*this);
    }
    
    // 添加有效性检查方法
    bool isValid() const {
        try {
            // 检查基本属性是否合理
            if (maxCapacity < 0 || maxCapacity > 1000) return false;
            if (unloadTime < 0 || reloadTime < 0) return false;
            
            // 尝试访问stack的size，如果stack损坏会抛出异常
            size_t size = currentAmmo.size();
            return size <= static_cast<size_t>(maxCapacity);
        } catch (...) {
            return false;
        }
    }
};

#endif // MAGAZINE_H