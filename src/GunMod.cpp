#include "GunMod.h"
#include "ItemFlag.h"

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
      modPenetrationBonus(other.modPenetrationBonus) {
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
    }
    return *this;
}