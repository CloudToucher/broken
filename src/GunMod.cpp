#include "GunMod.h"
#include "ItemFlag.h"
#include <algorithm>

GunMod::GunMod(const std::string& itemName)
    : Item(itemName) {
    // 添加枪械配件标签
    addFlag(ItemFlag::GUNMOD);
}

// 设置枪械配件属性
void GunMod::setModAttributes(float soundLevel, float fireRate, float damageBonus, 
                             float accuracyMOA, float recoil, float ergonomics, 
                             float breathStability, float rangeBonus, float bulletSpeedBonus,
                             float penetrationBonus) {
    // 设置枪械配件影响值
    modSoundLevel = soundLevel;
    modFireRate = fireRate;
    modAccuracyMOA = accuracyMOA;
    modRecoil = recoil;
    modErgonomics = ergonomics;
    modBreathStability = breathStability;
    
    // 设置枪械配件对子弹属性的影响值
    modDamageBonus = damageBonus;
    modRangeBonus = rangeBonus;
    modBulletSpeedBonus = bulletSpeedBonus;
    modPenetrationBonus = penetrationBonus;
    
    // 如果是激光配件，添加激光标签
    if (hasFlag(ItemFlag::MOD_LASER)) {
        addFlag(ItemFlag::LASER);
    }
}

// 实现拷贝构造函数
GunMod::GunMod(const GunMod& other)
    : Item(other),
      // 枪械配件影响值
      modSoundLevel(other.modSoundLevel),
      modFireRate(other.modFireRate),
      modAccuracyMOA(other.modAccuracyMOA),
      modRecoil(other.modRecoil),
      modErgonomics(other.modErgonomics),
      modBreathStability(other.modBreathStability),
      // 枪械配件对子弹属性的影响值
      modDamageBonus(other.modDamageBonus),
      modRangeBonus(other.modRangeBonus),
      modBulletSpeedBonus(other.modBulletSpeedBonus),
      modPenetrationBonus(other.modPenetrationBonus),
      // 新增成员变量
      slotCapacityModifiers(other.slotCapacityModifiers),
      addedAmmoTypes(other.addedAmmoTypes),
      removedAmmoTypes(other.removedAmmoTypes),
      addedMagazineNames(other.addedMagazineNames),
      removedMagazineNames(other.removedMagazineNames),
      compatibleSlots(other.compatibleSlots) {
}

// 实现拷贝赋值运算符
GunMod& GunMod::operator=(const GunMod& other) {
    if (this != &other) {
        Item::operator=(other);
        
        // 枪械配件影响值
        modSoundLevel = other.modSoundLevel;
        modFireRate = other.modFireRate;
        modAccuracyMOA = other.modAccuracyMOA;
        modRecoil = other.modRecoil;
        modErgonomics = other.modErgonomics;
        modBreathStability = other.modBreathStability;
        
        // 枪械配件对子弹属性的影响值
        modDamageBonus = other.modDamageBonus;
        modRangeBonus = other.modRangeBonus;
        modBulletSpeedBonus = other.modBulletSpeedBonus;
        modPenetrationBonus = other.modPenetrationBonus;
        
        // 新增成员变量
        slotCapacityModifiers = other.slotCapacityModifiers;
        addedAmmoTypes = other.addedAmmoTypes;
        removedAmmoTypes = other.removedAmmoTypes;
        addedMagazineNames = other.addedMagazineNames;
        removedMagazineNames = other.removedMagazineNames;
        compatibleSlots = other.compatibleSlots;
    }
    return *this;
}

// 新增：槽位容量修改方法实现
void GunMod::addSlotCapacityModifier(const std::string& slotType, int modifier) {
    slotCapacityModifiers[slotType] = modifier;
}

void GunMod::removeSlotCapacityModifier(const std::string& slotType) {
    slotCapacityModifiers.erase(slotType);
}

const std::map<std::string, int>& GunMod::getSlotCapacityModifiers() const {
    return slotCapacityModifiers;
}

void GunMod::clearSlotCapacityModifiers() {
    slotCapacityModifiers.clear();
}

// 新增：弹药类型修改方法实现
void GunMod::addAmmoTypeSupport(const std::string& ammoType) {
    auto it = std::find(addedAmmoTypes.begin(), addedAmmoTypes.end(), ammoType);
    if (it == addedAmmoTypes.end()) {
        addedAmmoTypes.push_back(ammoType);
    }
    
    // 如果这个弹药类型在移除列表中，从移除列表中删除
    auto removeIt = std::find(removedAmmoTypes.begin(), removedAmmoTypes.end(), ammoType);
    if (removeIt != removedAmmoTypes.end()) {
        removedAmmoTypes.erase(removeIt);
    }
}

void GunMod::removeAmmoTypeSupport(const std::string& ammoType) {
    // 从添加列表中移除
    auto addIt = std::find(addedAmmoTypes.begin(), addedAmmoTypes.end(), ammoType);
    if (addIt != addedAmmoTypes.end()) {
        addedAmmoTypes.erase(addIt);
    }
}

void GunMod::addAmmoTypeRestriction(const std::string& ammoType) {
    auto it = std::find(removedAmmoTypes.begin(), removedAmmoTypes.end(), ammoType);
    if (it == removedAmmoTypes.end()) {
        removedAmmoTypes.push_back(ammoType);
    }
    
    // 如果这个弹药类型在添加列表中，从添加列表中删除
    auto addIt = std::find(addedAmmoTypes.begin(), addedAmmoTypes.end(), ammoType);
    if (addIt != addedAmmoTypes.end()) {
        addedAmmoTypes.erase(addIt);
    }
}

void GunMod::removeAmmoTypeRestriction(const std::string& ammoType) {
    auto removeIt = std::find(removedAmmoTypes.begin(), removedAmmoTypes.end(), ammoType);
    if (removeIt != removedAmmoTypes.end()) {
        removedAmmoTypes.erase(removeIt);
    }
}

const std::vector<std::string>& GunMod::getAddedAmmoTypes() const {
    return addedAmmoTypes;
}

const std::vector<std::string>& GunMod::getRemovedAmmoTypes() const {
    return removedAmmoTypes;
}

void GunMod::clearAmmoTypeChanges() {
    addedAmmoTypes.clear();
    removedAmmoTypes.clear();
}

// 新增：弹匣兼容性修改方法实现
void GunMod::addMagazineSupport(const std::string& magazineName) {
    auto it = std::find(addedMagazineNames.begin(), addedMagazineNames.end(), magazineName);
    if (it == addedMagazineNames.end()) {
        addedMagazineNames.push_back(magazineName);
    }
    
    // 如果这个弹匣在移除列表中，从移除列表中删除
    auto removeIt = std::find(removedMagazineNames.begin(), removedMagazineNames.end(), magazineName);
    if (removeIt != removedMagazineNames.end()) {
        removedMagazineNames.erase(removeIt);
    }
}

void GunMod::removeMagazineSupport(const std::string& magazineName) {
    // 从添加列表中移除
    auto addIt = std::find(addedMagazineNames.begin(), addedMagazineNames.end(), magazineName);
    if (addIt != addedMagazineNames.end()) {
        addedMagazineNames.erase(addIt);
    }
}

void GunMod::addMagazineRestriction(const std::string& magazineName) {
    auto it = std::find(removedMagazineNames.begin(), removedMagazineNames.end(), magazineName);
    if (it == removedMagazineNames.end()) {
        removedMagazineNames.push_back(magazineName);
    }
    
    // 如果这个弹匣在添加列表中，从添加列表中删除
    auto addIt = std::find(addedMagazineNames.begin(), addedMagazineNames.end(), magazineName);
    if (addIt != addedMagazineNames.end()) {
        addedMagazineNames.erase(addIt);
    }
}

void GunMod::removeMagazineRestriction(const std::string& magazineName) {
    auto removeIt = std::find(removedMagazineNames.begin(), removedMagazineNames.end(), magazineName);
    if (removeIt != removedMagazineNames.end()) {
        removedMagazineNames.erase(removeIt);
    }
}

const std::vector<std::string>& GunMod::getAddedMagazineNames() const {
    return addedMagazineNames;
}

const std::vector<std::string>& GunMod::getRemovedMagazineNames() const {
    return removedMagazineNames;
}

void GunMod::clearMagazineChanges() {
    addedMagazineNames.clear();
    removedMagazineNames.clear();
}

// 新增：槽位兼容性方法实现
void GunMod::addCompatibleSlot(const std::string& slotType) {
    auto it = std::find(compatibleSlots.begin(), compatibleSlots.end(), slotType);
    if (it == compatibleSlots.end()) {
        compatibleSlots.push_back(slotType);
    }
}

void GunMod::removeCompatibleSlot(const std::string& slotType) {
    auto it = std::find(compatibleSlots.begin(), compatibleSlots.end(), slotType);
    if (it != compatibleSlots.end()) {
        compatibleSlots.erase(it);
    }
}

const std::vector<std::string>& GunMod::getCompatibleSlots() const {
    return compatibleSlots;
}

bool GunMod::canAttachToSlot(const std::string& slotType) const {
    return std::find(compatibleSlots.begin(), compatibleSlots.end(), slotType) != compatibleSlots.end();
}

void GunMod::clearCompatibleSlots() {
    compatibleSlots.clear();
}

// 新增：从Flag自动确定兼容槽位
void GunMod::updateCompatibleSlotsFromFlags() {
    clearCompatibleSlots();
    
    // 根据配件类型Flag确定兼容的槽位
    if (hasFlag(ItemFlag::MOD_STOCK)) {
        addCompatibleSlot("STOCK");
    }
    if (hasFlag(ItemFlag::MOD_BARREL)) {
        addCompatibleSlot("BARREL");
    }
    if (hasFlag(ItemFlag::MOD_UNDER_BARREL)) {
        addCompatibleSlot("UNDER_BARREL");
    }
    if (hasFlag(ItemFlag::MOD_GRIP)) {
        addCompatibleSlot("GRIP");
    }
    if (hasFlag(ItemFlag::MOD_OPTIC)) {
        addCompatibleSlot("OPTIC");
    }
    if (hasFlag(ItemFlag::MOD_SIDE_MOUNT)) {
        addCompatibleSlot("SIDE_MOUNT");
    }
    if (hasFlag(ItemFlag::MOD_MUZZLE)) {
        addCompatibleSlot("MUZZLE");
    }
    if (hasFlag(ItemFlag::MOD_MAGAZINE_WELL)) {
        addCompatibleSlot("MAGAZINE_WELL");
    }
    if (hasFlag(ItemFlag::MOD_RAIL)) {
        addCompatibleSlot("RAIL");
    }
}