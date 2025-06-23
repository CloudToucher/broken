#include "Gun.h"
#include "Magazine.h"
#include "Ammo.h"
#include <iostream> // For std::cout in use() example
#include <algorithm> // For std::find

// 简化构造函数
Gun::Gun(const std::string& itemName)
    : Item(itemName),
      gunType(GunType::PISTOL),  // 默认值
      currentFiringMode(FiringMode::SEMI_AUTO),  // 默认值
      // 初始化枪械固有的基础属性值为默认值
      baseSoundLevel(0.0f),
      baseFireRate(0.0f),
      baseAccuracyMOA(0.0f),
      baseRecoil(0.0f),
      baseErgonomics(0.0f),
      baseBreathStability(0.0f),
      baseDamageBonus(0.0f),
      baseRangeBonus(0.0f),
      baseBulletSpeedBonus(0.0f),
      basePenetrationBonus(0.0f),
      // 初始化最终属性值为基础值
      soundLevel(0.0f),
      fireRate(0.0f),
      accuracyMOA(0.0f),
      recoil(0.0f),
      ergonomics(0.0f),
      breathStability(0.0f),
      damageBonus(0.0f),
      rangeBonus(0.0f),
      bulletSpeedBonus(0.0f),
      penetrationBonus(0.0f),
      reloadTime(2.0f) { // 默认换弹时间为2秒
    
    // 添加枪械相关标签
    addFlag(ItemFlag::GUN);
    addFlag(ItemFlag::WEAPON);
    
    // 初始化配件槽位容量
    initAttachmentSlots();
}

// 初始化配件槽位容量
void Gun::initAttachmentSlots() {
    // 设置各个槽位的容量
    slotCapacity[AttachmentSlot::STOCK] = 1;          // 枪托，通常只有1个
    slotCapacity[AttachmentSlot::BARREL] = 1;         // 枪管，通常只有1个
    slotCapacity[AttachmentSlot::UNDER_BARREL] = 1;   // 下挂，通常只有1个
    slotCapacity[AttachmentSlot::GRIP] = 1;           // 握把，通常只有1个
    slotCapacity[AttachmentSlot::OPTIC] = 1;          // 光学瞄准镜，通常只有1个
    slotCapacity[AttachmentSlot::SIDE_MOUNT] = 1;     // 侧挂，通常只有1个
    slotCapacity[AttachmentSlot::MUZZLE] = 1;         // 枪口，通常只有1个
    slotCapacity[AttachmentSlot::MAGAZINE_WELL] = 1;  // 弹匣井，通常只有1个
    slotCapacity[AttachmentSlot::RAIL] = 3;           // 导轨，可以有多个配件
    slotCapacity[AttachmentSlot::SPECIAL] = 4;        // 特殊槽位，默认可以容纳4个
    
    // 初始化槽位使用状态为0
    for (const auto& [slot, capacity] : slotCapacity) {
        slotUsage[slot] = 0;
        // 为每个槽位创建空的配件容器
        attachmentSlots[slot] = std::vector<std::unique_ptr<GunMod>>();
    }
}

// 获取槽位容量
int Gun::getSlotCapacity(AttachmentSlot slot) const {
    auto it = slotCapacity.find(slot);
    if (it != slotCapacity.end()) {
        return it->second;
    }
    return 0; // 如果槽位不存在，返回0
}

// 设置槽位容量
void Gun::setSlotCapacity(AttachmentSlot slot, int capacity) {
    // 确保容量不为负数
    if (capacity < 0) capacity = 0;
    slotCapacity[slot] = capacity;
}

// 获取槽位已使用数量
int Gun::getSlotUsage(AttachmentSlot slot) const {
    auto it = slotUsage.find(slot);
    if (it != slotUsage.end()) {
        return it->second;
    }
    return 0; // 如果槽位不存在，返回0
}

// 检查槽位是否已满
bool Gun::isSlotFull(AttachmentSlot slot) const {
    return getSlotUsage(slot) >= getSlotCapacity(slot);
}

// 根据枪械类型设置对应的标签
void Gun::setGunTypeFlags() {
    // 清除所有枪械类型标签
    removeFlag(ItemFlag::PISTOL);
    removeFlag(ItemFlag::REVOLVER);
    removeFlag(ItemFlag::SHOTGUN);
    removeFlag(ItemFlag::SMG);
    removeFlag(ItemFlag::RIFLE);
    removeFlag(ItemFlag::SNIPER_RIFLE);
    removeFlag(ItemFlag::DMR);
    removeFlag(ItemFlag::MACHINE_GUN);
    removeFlag(ItemFlag::GRENADE_LAUNCHER);
    
    // 根据枪械类型添加对应标签
    switch (gunType) {
        case GunType::PISTOL:
            addFlag(ItemFlag::PISTOL);
            break;
        case GunType::REVOLVER:
            addFlag(ItemFlag::REVOLVER);
            break;
        case GunType::SHOTGUN:
            addFlag(ItemFlag::SHOTGUN);
            break;
        case GunType::SMG:
            addFlag(ItemFlag::SMG);
            break;
        case GunType::RIFLE:
            addFlag(ItemFlag::RIFLE);
            break;
        case GunType::SNIPER_RIFLE:
            addFlag(ItemFlag::SNIPER_RIFLE);
            break;
        case GunType::DMR:
            addFlag(ItemFlag::DMR);
            break;
        case GunType::MACHINE_GUN:
            addFlag(ItemFlag::MACHINE_GUN);
            break;
        case GunType::GRENADE_LAUNCHER:
            addFlag(ItemFlag::GRENADE_LAUNCHER);
            break;
    }
    
    // 根据射击模式设置标签
    for (const auto& mode : availableFiringModes) {
        switch (mode) {
            case FiringMode::SEMI_AUTO:
                addFlag(ItemFlag::SEMI_AUTO);
                break;
            case FiringMode::FULL_AUTO:
                addFlag(ItemFlag::FULL_AUTO);
                break;
            case FiringMode::BOLT_ACTION:
                addFlag(ItemFlag::BOLT_ACTION);
                break;
            case FiringMode::BURST:
                addFlag(ItemFlag::BURST);
                break;
        }
    }
    
    // 根据声音设置标签
    if (soundLevel < 50.0f) {
        addFlag(ItemFlag::SILENCED);
    }
    
    // 初始化完成后更新枪械属性（考虑配件影响）
    updateGunStats();
}

// 设置枪械类型
void Gun::setGunType(GunType type) {
    gunType = type;
    setGunTypeFlags();
}

// 设置可用射击模式
void Gun::setAvailableFiringModes(const std::vector<FiringMode>& modes) {
    availableFiringModes = modes;
    if (!availableFiringModes.empty()) {
        currentFiringMode = availableFiringModes[0];
    }
}

// 设置接受的弹药类型
void Gun::setAcceptedAmmoTypes(const std::vector<std::string>& ammoTypes) {
    acceptedAmmoTypes = ammoTypes;
}

// 设置基础声音级别
void Gun::setBaseSoundLevel(float level) {
    baseSoundLevel = level;
    soundLevel = level; // 更新最终值
    updateGunStats(); // 考虑配件影响
}

// 设置基础射速
void Gun::setBaseFireRate(float rate) {
    baseFireRate = rate;
    fireRate = rate; // 更新最终值
    updateGunStats(); // 考虑配件影响
}

// 设置基础精度
void Gun::setBaseAccuracyMOA(float accuracy) {
    baseAccuracyMOA = accuracy;
    accuracyMOA = accuracy; // 更新最终值
    updateGunStats(); // 考虑配件影响
}

// 设置基础后坐力
void Gun::setBaseRecoil(float recoil) {
    baseRecoil = recoil;
    this->recoil = recoil; // 更新最终值
    updateGunStats(); // 考虑配件影响
}

// 设置基础人机工效学
void Gun::setBaseErgonomics(float ergonomics) {
    baseErgonomics = ergonomics;
    this->ergonomics = ergonomics; // 更新最终值
    updateGunStats(); // 考虑配件影响
}

// 设置基础呼吸稳定性
void Gun::setBaseBreathStability(float stability) {
    baseBreathStability = stability;
    breathStability = stability; // 更新最终值
    updateGunStats(); // 考虑配件影响
}

// 设置基础伤害加成
void Gun::setBaseDamageBonus(float bonus) {
    baseDamageBonus = bonus;
    damageBonus = bonus; // 更新最终值
    updateGunStats(); // 考虑配件影响
}

// 设置基础射程加成
void Gun::setBaseRangeBonus(float bonus) {
    baseRangeBonus = bonus;
    rangeBonus = bonus; // 更新最终值
    updateGunStats(); // 考虑配件影响
}

// 设置基础子弹速度加成
void Gun::setBaseBulletSpeedBonus(float bonus) {
    baseBulletSpeedBonus = bonus;
    bulletSpeedBonus = bonus; // 更新最终值
    updateGunStats(); // 考虑配件影响
}

// 设置基础穿透加成
void Gun::setBasePenetrationBonus(float bonus) {
    basePenetrationBonus = bonus;
    penetrationBonus = bonus; // 更新最终值
    updateGunStats(); // 考虑配件影响
}

// 拉栓/上膛操作 (手动将弹匣子弹上膛)
bool Gun::chamberManually() {
    if (!chamberedRound && currentMagazine && !currentMagazine->isEmpty()) {
        chamberedRound = currentMagazine->consumeAmmo();
        return chamberedRound != nullptr;
    }
    return false; // 膛内已有子弹或弹匣为空
}

// 射击逻辑：每次都抛出枪膛内的ammo类，同时将弹匣栈顶的子弹压入枪膛
// 修改shoot方法，明确区分射击和上膛
std::unique_ptr<Ammo> Gun::shoot() {
    if (chamberedRound) {
        std::unique_ptr<Ammo> firedRound = std::move(chamberedRound); // 抛出膛内子弹
        
        // 尝试从弹匣中取一发子弹上膛
        if (currentMagazine && !currentMagazine->isEmpty()) {
            chamberedRound = currentMagazine->consumeAmmo();
        } else {
            chamberedRound = nullptr; // 弹匣空了，膛内也空了
        }
        
        return firedRound;
    }
    return nullptr; // 膛内无弹，无法射击
}

void Gun::loadMagazine(std::unique_ptr<Magazine> mag) {
    if (mag && canAcceptMagazine(mag.get())) {
        currentMagazine = std::move(mag);
    }
}

std::unique_ptr<Magazine> Gun::unloadMagazine() {
    return std::move(currentMagazine);
}

bool Gun::canAcceptMagazine(const Magazine* mag) const {
    if (!mag) return false;
    
    // 首先检查弹匣名称是否在可接受列表中
    if (!acceptedMagazineNames.empty()) {
        return std::find(acceptedMagazineNames.begin(), acceptedMagazineNames.end(), mag->getName()) != acceptedMagazineNames.end();
    }
    
    // 如果没有指定可接受的弹匣名称，则检查弹药类型兼容性
    const auto& magAmmoTypes = mag->getCompatibleAmmoTypes();
    for (const auto& gunAmmoType : acceptedAmmoTypes) {
        for (const auto& magAmmoTypeEntry : magAmmoTypes) { // magAmmoTypes is std::vector<std::string>
            if (gunAmmoType == magAmmoTypeEntry) {
                return true;
            }
        }
    }
    return false;
}

Magazine* Gun::getCurrentMagazine() const {
    return currentMagazine.get();
}

float Gun::getFireRate() const { 
    // 如果fireRate存储的是每分钟发射子弹数(RPM)，则需要转换为毫秒/发
    // 例如：900发/分钟 = 60000毫秒/900发 = 66.67毫秒/发
    return 60000.0f / fireRate; 
}

// 注意：getFinalXXXX方法已被移除
// 枪械属性和ammo没有关系，只在shoot时才计算最终结果

bool Gun::attach(AttachmentSlot slot, std::unique_ptr<GunMod> attachment) {
    // 检查是否为有效的配件
    if (!attachment || !attachment->isGunMod()) {
        return false;
    }
    
    // 检查槽位是否已满
    if (isSlotFull(slot)) {
        return false;
    }
    
    // 根据槽位类型检查配件是否适合该槽位
    bool validSlot = false;
    switch (slot) {
        case AttachmentSlot::STOCK:
            validSlot = attachment->hasFlag(ItemFlag::MOD_STOCK);
            break;
        case AttachmentSlot::BARREL:
            validSlot = attachment->hasFlag(ItemFlag::MOD_BARREL);
            break;
        case AttachmentSlot::UNDER_BARREL:
            validSlot = attachment->hasFlag(ItemFlag::MOD_UNDER_BARREL);
            break;
        case AttachmentSlot::GRIP:
            validSlot = attachment->hasFlag(ItemFlag::MOD_GRIP);
            break;
        case AttachmentSlot::OPTIC:
            validSlot = attachment->hasFlag(ItemFlag::MOD_OPTIC);
            break;
        case AttachmentSlot::SIDE_MOUNT:
            validSlot = attachment->hasFlag(ItemFlag::MOD_SIDE_MOUNT);
            break;
        case AttachmentSlot::MUZZLE:
            validSlot = attachment->hasFlag(ItemFlag::MOD_MUZZLE);
            break;
        case AttachmentSlot::MAGAZINE_WELL:
            validSlot = attachment->hasFlag(ItemFlag::MOD_MAGAZINE_WELL);
            break;
        case AttachmentSlot::RAIL:
            validSlot = attachment->hasFlag(ItemFlag::MOD_RAIL) || 
                       attachment->hasFlag(ItemFlag::MOD_LASER) || 
                       attachment->hasFlag(ItemFlag::MOD_FLASHLIGHT);
            break;
        case AttachmentSlot::SPECIAL:
            // 特殊槽位可以接受任何类型的配件
            validSlot = true;
            break;
    }
    
    if (!validSlot) {
        return false;
    }
    
    // 添加配件到对应槽位的容器中
    attachmentSlots[slot].push_back(std::move(attachment));
    
    // 更新槽位使用状态
    slotUsage[slot]++;
    
    // 更新枪械属性
    updateGunStats();
    
    return true;
}

GunMod* Gun::getAttachment(AttachmentSlot slot, int index) const {
    auto slotIt = attachmentSlots.find(slot);
    if (slotIt != attachmentSlots.end() && !slotIt->second.empty() && index < slotIt->second.size()) {
        return slotIt->second[index].get();
    }
    return nullptr;
}

const std::vector<GunMod*> Gun::getAllAttachments(AttachmentSlot slot) const {
    std::vector<GunMod*> result;
    auto slotIt = attachmentSlots.find(slot);
    if (slotIt != attachmentSlots.end()) {
        for (const auto& attachment : slotIt->second) {
            result.push_back(attachment.get());
        }
    }
    return result;
}

int Gun::getTotalAttachmentCount() const {
    int total = 0;
    for (const auto& [slot, usage] : slotUsage) {
        total += usage;
    }
    return total;
}

std::unique_ptr<GunMod> Gun::detach(AttachmentSlot slot, int index) {
    auto slotIt = attachmentSlots.find(slot);
    if (slotIt != attachmentSlots.end() && !slotIt->second.empty() && index < slotIt->second.size()) {
        // 获取要移除的配件
        std::unique_ptr<GunMod> detachedItem = std::move(slotIt->second[index]);
        
        // 从容器中移除该配件
        slotIt->second.erase(slotIt->second.begin() + index);
        
        // 更新槽位使用状态
        slotUsage[slot]--;
        
        // 更新枪械属性
        updateGunStats();
        
        return detachedItem;
    }
    return nullptr;
}

void Gun::updateGunStats() {
    // 重置为基础属性值
    soundLevel = baseSoundLevel;
    fireRate = baseFireRate;
    accuracyMOA = baseAccuracyMOA;
    recoil = baseRecoil;
    ergonomics = baseErgonomics;
    breathStability = baseBreathStability;
    damageBonus = baseDamageBonus;
    rangeBonus = baseRangeBonus;
    bulletSpeedBonus = baseBulletSpeedBonus;
    penetrationBonus = basePenetrationBonus;
    
    // 移除可能由配件添加的标签
    removeFlag(ItemFlag::LASER);
    removeFlag(ItemFlag::FLASHLIGHT);
    
    // 应用所有槽位中所有配件的影响值
    for (const auto& [slot, attachmentVector] : attachmentSlots) {
        for (const auto& attachment : attachmentVector) {
            if (attachment) {
                // 应用配件的影响值
                soundLevel += attachment->getModSoundLevel();
                fireRate += attachment->getModFireRate();
                accuracyMOA += attachment->getModAccuracyMOA();
                recoil += attachment->getModRecoil();
                ergonomics += attachment->getModErgonomics();
                breathStability += attachment->getModBreathStability();
                damageBonus += attachment->getModDamageBonus();
                rangeBonus += attachment->getModRangeBonus();
                bulletSpeedBonus += attachment->getModBulletSpeedBonus();
                penetrationBonus += attachment->getModPenetrationBonus();
                
                // 检查特殊标签
                if (attachment->hasFlag(ItemFlag::MOD_LASER)) {
                    addFlag(ItemFlag::LASER);
                }
                if (attachment->hasFlag(ItemFlag::MOD_FLASHLIGHT)) {
                    addFlag(ItemFlag::FLASHLIGHT);
                }
            }
        }
    }
    
    // 确保属性值在合理范围内
    soundLevel = std::max(0.0f, soundLevel);
    fireRate = std::max(1.0f, fireRate);
    accuracyMOA = std::max(0.1f, accuracyMOA);
    recoil = std::max(0.0f, recoil);
    ergonomics = std::max(0.0f, ergonomics);
    breathStability = std::max(0.0f, breathStability);
    
    // 根据声音设置标签
    removeFlag(ItemFlag::SILENCED);
    if (soundLevel < 50.0f) {
        addFlag(ItemFlag::SILENCED);
    }
}

// 实现虚函数 use
void Gun::use() {
    // 作为武器，'use' 可能意味着准备射击、切换射击模式、查看等
    // 具体逻辑根据游戏设计决定
    std::cout << getName() << " is being used." << std::endl;
    // 例如，如果玩家按下特定键，可以在这里切换射击模式
    // if (!availableFiringModes.empty()) {
    //     // Cycle through firing modes or open a selection menu
    // }
}

// 拷贝构造函数
Gun::Gun(const Gun& other)
    : Item(other) {
    // 拷贝基本类型成员
    gunType = other.gunType;
    currentFiringMode = other.currentFiringMode;
    availableFiringModes = other.availableFiringModes;
    acceptedAmmoTypes = other.acceptedAmmoTypes;
    acceptedMagazineNames = other.acceptedMagazineNames; // 复制可接受的弹匣名称列表
    
    // 拷贝基础属性值
    baseSoundLevel = other.baseSoundLevel;
    baseFireRate = other.baseFireRate;
    baseAccuracyMOA = other.baseAccuracyMOA;
    baseRecoil = other.baseRecoil;
    baseErgonomics = other.baseErgonomics;
    baseBreathStability = other.baseBreathStability;
    baseDamageBonus = other.baseDamageBonus;
    baseRangeBonus = other.baseRangeBonus;
    baseBulletSpeedBonus = other.baseBulletSpeedBonus;
    basePenetrationBonus = other.basePenetrationBonus;
    
    // 拷贝最终属性值
    soundLevel = other.soundLevel;
    fireRate = other.fireRate;
    accuracyMOA = other.accuracyMOA;
    recoil = other.recoil;
    ergonomics = other.ergonomics;
    breathStability = other.breathStability;
    damageBonus = other.damageBonus;
    rangeBonus = other.rangeBonus;
    bulletSpeedBonus = other.bulletSpeedBonus;
    penetrationBonus = other.penetrationBonus;
    
    // 拷贝槽位容量和使用状态
    slotCapacity = other.slotCapacity;
    slotUsage = other.slotUsage;
    
    // 深拷贝枪膛内的子弹
    if (other.chamberedRound) {
        chamberedRound = std::unique_ptr<Ammo>(static_cast<Ammo*>(other.chamberedRound->clone()));
    }
    
    // 深拷贝当前弹匣
    if (other.currentMagazine) {
        currentMagazine = std::unique_ptr<Magazine>(static_cast<Magazine*>(other.currentMagazine->clone()));
    }
    
    // 深拷贝所有槽位的所有配件
    for (const auto& [slot, attachmentVector] : other.attachmentSlots) {
        // 为每个槽位创建一个新的配件容器
        attachmentSlots[slot] = std::vector<std::unique_ptr<GunMod>>();
        
        // 复制该槽位的所有配件
        for (const auto& attachment : attachmentVector) {
            if (attachment) {
                attachmentSlots[slot].push_back(
                    std::unique_ptr<GunMod>(static_cast<GunMod*>(attachment->clone()))
                );
            }
        }
    }
}

// 拷贝赋值运算符实现
Gun& Gun::operator=(const Gun& other) {
    if (this != &other) {
        // 先调用基类的赋值运算符
        Item::operator=(other);
        
        // 拷贝基本类型成员
        gunType = other.gunType;
        currentFiringMode = other.currentFiringMode;
        availableFiringModes = other.availableFiringModes;
        acceptedAmmoTypes = other.acceptedAmmoTypes;
        acceptedMagazineNames = other.acceptedMagazineNames; // 复制可接受的弹匣名称列表
        
        // 拷贝基础属性值
        baseSoundLevel = other.baseSoundLevel;
        baseFireRate = other.baseFireRate;
        baseAccuracyMOA = other.baseAccuracyMOA;
        baseRecoil = other.baseRecoil;
        baseErgonomics = other.baseErgonomics;
        baseBreathStability = other.baseBreathStability;
        baseDamageBonus = other.baseDamageBonus;
        baseRangeBonus = other.baseRangeBonus;
        baseBulletSpeedBonus = other.baseBulletSpeedBonus;
        basePenetrationBonus = other.basePenetrationBonus;
    
        // 拷贝最终属性值
        soundLevel = other.soundLevel;
        fireRate = other.fireRate;
        accuracyMOA = other.accuracyMOA;
        recoil = other.recoil;
        ergonomics = other.ergonomics;
        breathStability = other.breathStability;
        damageBonus = other.damageBonus;
        rangeBonus = other.rangeBonus;
        bulletSpeedBonus = other.bulletSpeedBonus;
        penetrationBonus = other.penetrationBonus;
        
        // 拷贝槽位容量和使用状态
        slotCapacity = other.slotCapacity;
        slotUsage = other.slotUsage;
        
        // 清除当前的指针成员，准备深拷贝
        chamberedRound.reset();
        currentMagazine.reset();
        attachmentSlots.clear();
        
        // 深拷贝枪膛内的子弹
        if (other.chamberedRound) {
            chamberedRound = std::unique_ptr<Ammo>(static_cast<Ammo*>(other.chamberedRound->clone()));
        }
        
        // 深拷贝当前弹匣
        if (other.currentMagazine) {
            currentMagazine = std::unique_ptr<Magazine>(static_cast<Magazine*>(other.currentMagazine->clone()));
        }
        
        // 深拷贝所有槽位的所有配件
        for (const auto& [slot, attachmentVector] : other.attachmentSlots) {
            // 为每个槽位创建一个新的配件容器
            attachmentSlots[slot] = std::vector<std::unique_ptr<GunMod>>();
            
            // 复制该槽位的所有配件
            for (const auto& attachment : attachmentVector) {
                if (attachment) {
                    attachmentSlots[slot].push_back(
                        std::unique_ptr<GunMod>(static_cast<GunMod*>(attachment->clone()))
                    );
                }
            }
        }
    }
    return *this;
}

// 设置换弹时间
void Gun::setReloadTime(float time) {
    reloadTime = time;
}