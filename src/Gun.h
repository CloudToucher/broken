#pragma once
#ifndef GUN_H
#define GUN_H

#include "Item.h"
#include "Magazine.h" 
#include "Ammo.h"     
#include "GunMod.h"
#include <string>
#include <vector>
#include <map>
#include <memory>

// ... GunType, FiringMode, AttachmentSlot 枚举保持不变 ...
enum class GunType {
    PISTOL,
    REVOLVER,
    SHOTGUN,
    SMG,
    RIFLE,
    SNIPER_RIFLE,
    DMR,
    MACHINE_GUN,
    GRENADE_LAUNCHER
};

enum class FiringMode {
    SEMI_AUTO,
    FULL_AUTO,
    BOLT_ACTION,
    BURST 
};

enum class AttachmentSlot {
    STOCK,          
    BARREL,         
    UNDER_BARREL,   
    GRIP,           
    OPTIC,          
    SIDE_MOUNT,
    MUZZLE,         
    MAGAZINE_WELL,  
    RAIL,   
    SPECIAL,        // 特殊槽位
};

class Gun : public Item {
private:
    // ... 其他成员变量 ...
    std::unique_ptr<Ammo> chamberedRound; // 枪膛内的一发子弹
    
    GunType gunType;
    FiringMode currentFiringMode;
    std::vector<FiringMode> availableFiringModes;
    
    // 枪械固有的基础属性值
    float baseSoundLevel;        // 基础枪声大小
    float baseFireRate;          // 基础射速
    float baseAccuracyMOA;       // 基础精度
    float baseRecoil;            // 基础后坐力
    float baseErgonomics;        // 基础人体工程学
    float baseBreathStability;   // 基础呼吸稳定性
    
    // 枪械对子弹的基础加成值
    float baseDamageBonus;       // 基础伤害加成
    float baseRangeBonus;        // 基础射程加成
    float baseBulletSpeedBonus;  // 基础子弹速度加成
    float basePenetrationBonus;  // 基础穿透力加成
    
    // 计算后的最终属性值（考虑配件影响后的值）
    float soundLevel;            // 最终枪声大小
    float fireRate;              // 最终射速
    float accuracyMOA;           // 最终精度
    float recoil;                // 最终后坐力
    float ergonomics;            // 最终人体工程学
    float breathStability;       // 最终呼吸稳定性
    float damageBonus;           // 最终伤害加成
    float rangeBonus;            // 最终射程加成
    float bulletSpeedBonus;      // 最终子弹速度加成
    float penetrationBonus;      // 最终穿透力加成
    float reloadTime;            // 换弹时间（秒）
    
    // 配件槽位容量映射表，记录每种槽位可容纳的配件数量
    std::map<AttachmentSlot, int> slotCapacity;
    // 配件槽位使用状态，记录每种槽位已使用的数量
    std::map<AttachmentSlot, int> slotUsage;
    // 存储所有配件
    std::map<AttachmentSlot, std::vector<std::unique_ptr<GunMod>>> attachmentSlots;
    
    std::vector<std::string> acceptedAmmoTypes; 
    std::unique_ptr<Magazine> currentMagazine; 
    
    // 可装填弹匣名称列表
    std::vector<std::string> acceptedMagazineNames;

public:
    // 简化构造函数
    Gun(const std::string& itemName);

    // 初始化配件槽位容量
    void initAttachmentSlots();
    
    // 获取槽位容量
    int getSlotCapacity(AttachmentSlot slot) const;
    
    // 获取所有槽位容量映射表
    const std::map<AttachmentSlot, int>& getSlotCapacity() const { return slotCapacity; }
    
    // 设置槽位容量
    void setSlotCapacity(AttachmentSlot slot, int capacity);
    
    // 获取槽位已使用数量
    int getSlotUsage(AttachmentSlot slot) const;
    
    // 检查槽位是否已满
    bool isSlotFull(AttachmentSlot slot) const;

    // 获取可接受的弹匣名称列表
    const std::vector<std::string>& getAcceptedMagazineNames() const { return acceptedMagazineNames; }
    
    // 设置可接受的弹匣名称列表
    void setAcceptedMagazineNames(const std::vector<std::string>& magazineNames) { acceptedMagazineNames = magazineNames; }
    
    // 添加可接受的弹匣名称
    void addAcceptedMagazineName(const std::string& magazineName) { acceptedMagazineNames.push_back(magazineName); }
    
    // 射击方法 - 返回发射的子弹，同时尝试自动上膛
    std::unique_ptr<Ammo> shoot();
    
    // 手动上膛方法 - 从弹匣取子弹上膛
    bool chamberManually();
    
    // 检查是否可以射击（有子弹在膛内）
    bool canShoot() const { return chamberedRound != nullptr; }
    
    // 检查弹匣是否为空
    bool isMagazineEmpty() const { return !currentMagazine || currentMagazine->isEmpty(); }

    // 获取枪械属性（计算后的最终值）
    float getSoundLevel() const { return soundLevel; }
    float getFireRate() const;
    float getAccuracyMOA() const { return accuracyMOA; }
    float getRecoil() const { return recoil; }
    float getErgonomics() const { return ergonomics; }
    float getBreathStability() const { return breathStability; }
    float getReloadTime() const { return reloadTime; }
    
    // 获取枪械对子弹属性的加成（计算后的最终值）
    float getDamageBonus() const { return damageBonus; }
    float getRangeBonus() const { return rangeBonus; }
    float getBulletSpeedBonus() const { return bulletSpeedBonus; }
    float getPenetrationBonus() const { return penetrationBonus; }
    
    // 获取枪械基础属性值
    float getBaseSoundLevel() const { return baseSoundLevel; }
    float getBaseFireRate() const { return baseFireRate; }
    float getBaseAccuracyMOA() const { return baseAccuracyMOA; }
    float getBaseRecoil() const { return baseRecoil; }
    float getBaseErgonomics() const { return baseErgonomics; }
    float getBaseBreathStability() const { return baseBreathStability; }
    
    // 获取枪械对子弹的基础加成值
    float getBaseDamageBonus() const { return baseDamageBonus; }
    float getBaseRangeBonus() const { return baseRangeBonus; }
    float getBaseBulletSpeedBonus() const { return baseBulletSpeedBonus; }
    float getBasePenetrationBonus() const { return basePenetrationBonus; }
    
    // 获取枪械基础属性值的声明（实现在 cpp 文件中）
    // 设置方法在下方声明

    Ammo* getChamberedRound() const { return chamberedRound.get(); }

    bool canAcceptMagazine(const Magazine* mag) const;
    void loadMagazine(std::unique_ptr<Magazine> mag);
    std::unique_ptr<Magazine> unloadMagazine();
    Magazine* getCurrentMagazine() const;
    
    // 配件相关方法
    bool attach(AttachmentSlot slot, std::unique_ptr<GunMod> attachment); // 添加配件到指定槽位
    std::unique_ptr<GunMod> detach(AttachmentSlot slot, int index = 0); // 从指定槽位移除配件，默认移除第一个
    GunMod* getAttachment(AttachmentSlot slot, int index = 0) const; // 获取指定槽位的配件，默认获取第一个
    
    // 获取指定槽位的所有配件
    const std::vector<GunMod*> getAllAttachments(AttachmentSlot slot) const;
    
    // 获取所有槽位的配件总数
    int getTotalAttachmentCount() const;
    
    // 获取枪械属性
    GunType getGunType() const { return gunType; }
    
    void use() override;
    
    // 获取当前射击模式
    FiringMode getCurrentFiringMode() const { return currentFiringMode; }
    
    // 切换射击模式
    void toggleFiringMode() {
        if (availableFiringModes.size() > 1) {
            // 找到当前模式的索引
            auto it = std::find(availableFiringModes.begin(), availableFiringModes.end(), currentFiringMode);
            if (it != availableFiringModes.end()) {
                // 计算下一个模式的索引
                size_t currentIndex = std::distance(availableFiringModes.begin(), it);
                size_t nextIndex = (currentIndex + 1) % availableFiringModes.size();
                currentFiringMode = availableFiringModes[nextIndex];
            }
        }
    }
    
    // 根据枪械类型设置对应的标签
    void setGunTypeFlags();
    
    // 设置枪械类型
    void setGunType(GunType type);
    
    // 设置可用射击模式
    void setAvailableFiringModes(const std::vector<FiringMode>& modes);
    
    // 设置接受的弹药类型
    void setAcceptedAmmoTypes(const std::vector<std::string>& ammoTypes);
    
    // 设置基础声音级别
    void setBaseSoundLevel(float level);
    
    // 设置基础射速
    void setBaseFireRate(float rate);
    
    // 设置基础精度
    void setBaseAccuracyMOA(float accuracy);
    
    // 设置基础后坐力
    void setBaseRecoil(float recoil);
    
    // 设置基础人机工效学
    void setBaseErgonomics(float ergonomics);
    
    // 设置基础呼吸稳定性
    void setBaseBreathStability(float stability);
    
    // 设置基础伤害加成
    void setBaseDamageBonus(float bonus);
    
    // 设置基础射程加成
    void setBaseRangeBonus(float bonus);
    
    // 设置基础子弹速度加成
    void setBaseBulletSpeedBonus(float bonus);
    
    // 设置基础穿透加成
    void setBasePenetrationBonus(float bonus);
    
    // 更新枪械属性（根据配件影响值）
    void updateGunStats();
    
    // 重写克隆方法，用于创建Gun的深拷贝
    Item* clone() const override {
        return new Gun(*this);
    }
    
    // 拷贝构造函数
    Gun(const Gun& other);
    
    // 拷贝赋值运算符
    Gun& operator=(const Gun& other);

    // 设置换弹时间
    void setReloadTime(float time);
};

#endif // GUN_H