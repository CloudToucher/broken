#include "Ammo.h"

Ammo::Ammo(const std::string& itemName)
    : Item(itemName),
      baseDamage(0),
      basePenetration(0.0f),
      baseRange(0.0f),
      baseSpeed(0.0f),
      ammoType("") {
    
    // 添加弹药相关标签
    addFlag(ItemFlag::AMMO);
}



void Ammo::use() {
    // 子弹的使用逻辑通常是被射出，这里可以留空或添加特殊效果
    // 例如，可以记录子弹被使用的次数，或触发特殊效果
}

Ammo::Ammo(const Ammo& other)
    : Item(other),
    baseDamage(other.baseDamage),
    basePenetration(other.basePenetration),
    baseRange(other.baseRange),
    baseSpeed(other.baseSpeed),
    modRecoil(other.modRecoil),
    modAccuracyMOA(other.modAccuracyMOA),
    modErgonomics(other.modErgonomics),
    ammoType(other.ammoType) {
}


// 实现拷贝赋值运算符
Ammo& Ammo::operator=(const Ammo& other) {
    if (this != &other) {
        Item::operator=(other);
        baseDamage = other.baseDamage;
        basePenetration = other.basePenetration;
        baseRange = other.baseRange;
        baseSpeed = other.baseSpeed;
        modRecoil = other.modRecoil;
        modAccuracyMOA = other.modAccuracyMOA;
        modErgonomics = other.modErgonomics;
        ammoType = other.ammoType;
    }
    return *this;
}