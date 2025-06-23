#include "Magazine.h"
#include "Ammo.h"
#include <iostream> // For std::cout in use() example
#include <algorithm> // For std::find

// 简化构造函数
Magazine::Magazine(const std::string& itemName)
    : Item(itemName),
      compatibleAmmoTypes(), maxCapacity(0), unloadTime(0.0f), reloadTime(0.0f) {
    // 添加弹匣相关标签
    addFlag(ItemFlag::MAGAZINE);
}

const std::vector<std::string>& Magazine::getCompatibleAmmoTypes() const {
    return compatibleAmmoTypes;
}

bool Magazine::isEmpty() const {
    return currentAmmo.empty();
}

bool Magazine::isFull() const {
    return static_cast<int>(currentAmmo.size()) >= maxCapacity;
}

void Magazine::use() {
    // TODO: 实现Magazine的use逻辑
}

float Magazine::getUnloadTime() const {
    return unloadTime;
}

float Magazine::getReloadTime() const {
    return reloadTime;
}

bool Magazine::canAcceptAmmo(const std::string& ammoType) const {
    return std::find(compatibleAmmoTypes.begin(), compatibleAmmoTypes.end(), ammoType) != compatibleAmmoTypes.end();
}

bool Magazine::loadAmmo(std::unique_ptr<Ammo> ammo) {
    if (isFull() || !canAcceptAmmo(ammo->getAmmoType())) return false;
    currentAmmo.push(std::move(ammo));
    return true;
}

std::unique_ptr<Ammo> Magazine::consumeAmmo() {
    if (isEmpty()) return nullptr;
    auto ammo = std::move(currentAmmo.top());
    currentAmmo.pop();
    return ammo;
}

int Magazine::getCurrentAmmoCount() const {
    try {
        return static_cast<int>(currentAmmo.size());
    } catch (const std::exception& e) {
        // 如果发生异常，返回0并输出错误信息
        std::cerr << "Magazine::getCurrentAmmoCount() 异常: " << e.what() << std::endl;
        return 0;
    } catch (...) {
        // 捕获所有其他异常
        std::cerr << "Magazine::getCurrentAmmoCount() 未知异常" << std::endl;
        return 0;
    }
}

int Magazine::getCapacity() const {
    return maxCapacity;
}

// 实现拷贝构造函数
Magazine::Magazine(const Magazine& other)
    : Item(other),
      compatibleAmmoTypes(other.compatibleAmmoTypes),
      maxCapacity(other.maxCapacity),
      unloadTime(other.unloadTime),
      reloadTime(other.reloadTime) {
    // 不复制弹药栈，新创建的弹匣为空
    // currentAmmo默认为空栈
}

// 实现拷贝赋值运算符
Magazine& Magazine::operator=(const Magazine& other) {
    if (this != &other) {
        Item::operator=(other);
        compatibleAmmoTypes = other.compatibleAmmoTypes;
        maxCapacity = other.maxCapacity;
        unloadTime = other.unloadTime;
        reloadTime = other.reloadTime;
        
        // 清空当前弹药栈
        while (!currentAmmo.empty()) {
            currentAmmo.pop();
        }
        
        // 不复制弹药栈，赋值后的弹匣为空
        // currentAmmo保持为空栈
    }
    return *this;
}