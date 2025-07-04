#include "Gun.h"
#include "Magazine.h"
#include "Ammo.h"
#include "FlagMapper.h"
#include <iostream>
#include <algorithm>

// 重构构造函数
Gun::Gun(const std::string& itemName)
    : Item(itemName),
      currentFiringMode("SEMI_AUTO"),  // 默认射击模式
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
    
    // 设置默认射击模式
    availableFiringModes.push_back("SEMI_AUTO");
    
    // 初始化FlagMapper
    FlagMapper::initializeMappings();
    
    // 初始化配件槽位容量
    initAttachmentSlots();
}

// 重构初始化配件槽位容量
void Gun::initAttachmentSlots() {
    // 设置各个槽位的基础容量（字符串版本）
    baseSlotCapacity["STOCK"] = 1;          // 枪托，通常只有1个
    baseSlotCapacity["BARREL"] = 1;         // 枪管，通常只有1个
    baseSlotCapacity["UNDER_BARREL"] = 1;   // 下挂，通常只有1个
    baseSlotCapacity["GRIP"] = 1;           // 握把，通常只有1个
    baseSlotCapacity["OPTIC"] = 1;          // 光学瞄准镜，通常只有1个
    baseSlotCapacity["SIDE_MOUNT"] = 1;     // 侧挂，通常只有1个
    baseSlotCapacity["MUZZLE"] = 1;         // 枪口，通常只有1个
    baseSlotCapacity["MAGAZINE_WELL"] = 1;  // 弹匣井，通常只有1个
    baseSlotCapacity["RAIL"] = 0;           // 导轨，默认为0，需要护木等配件才能增加
    baseSlotCapacity["SPECIAL"] = 1;        // 特殊槽位，默认可以容纳1个
    
    // 初始化当前槽位容量（复制基础容量）
    currentSlotCapacity = baseSlotCapacity;
    
    // 初始化槽位使用状态为0
    for (const auto& [slotType, capacity] : baseSlotCapacity) {
        slotUsage[slotType] = 0;
        // 为每个槽位创建空的配件容器
        attachmentSlots[slotType] = std::vector<std::unique_ptr<GunMod>>();
        // 为每个槽位创建默认白名单（允许所有）
        slotWhitelists[slotType] = SlotWhitelist(false); // 默认不允许所有，需要设置规则
    }
    
    // 设置默认白名单规则
    setupDefaultSlotWhitelists();
}

// 新增：设置默认槽位白名单规则
void Gun::setupDefaultSlotWhitelists() {
    // 枪托槽位
    slotWhitelists["STOCK"].addRequiredFlag(ItemFlag::MOD_STOCK);
    
    // 枪管槽位
    slotWhitelists["BARREL"].addRequiredFlag(ItemFlag::MOD_BARREL);
    
    // 下挂槽位
    slotWhitelists["UNDER_BARREL"].addRequiredFlag(ItemFlag::MOD_UNDER_BARREL);
    
    // 握把槽位
    slotWhitelists["GRIP"].addRequiredFlag(ItemFlag::MOD_GRIP);
    
    // 瞄准镜槽位
    slotWhitelists["OPTIC"].addRequiredFlag(ItemFlag::MOD_OPTIC);
    
    // 侧挂槽位
    slotWhitelists["SIDE_MOUNT"].addRequiredFlag(ItemFlag::MOD_SIDE_MOUNT);
    
    // 枪口槽位
    slotWhitelists["MUZZLE"].addRequiredFlag(ItemFlag::MOD_MUZZLE);
    
    // 弹匣井槽位
    slotWhitelists["MAGAZINE_WELL"].addRequiredFlag(ItemFlag::MOD_MAGAZINE_WELL);
    
    // 导轨槽位
    slotWhitelists["RAIL"].addRequiredFlag(ItemFlag::MOD_RAIL);
    
    // 特殊槽位（允许任何配件）
    slotWhitelists["SPECIAL"].setAllowAll(true);
}

// 重构槽位管理方法
int Gun::getSlotCapacity(const std::string& slotType) const {
    auto it = baseSlotCapacity.find(slotType);
    if (it != baseSlotCapacity.end()) {
        return it->second;
    }
    return 0; // 如果槽位不存在，返回0
}

int Gun::getEffectiveSlotCapacity(const std::string& slotType) const {
    auto it = currentSlotCapacity.find(slotType);
    if (it != currentSlotCapacity.end()) {
        return it->second;
    }
    return 0; // 如果槽位不存在，返回0
}

void Gun::setSlotCapacity(const std::string& slotType, int capacity) {
    // 确保容量不为负数
    if (capacity < 0) capacity = 0;
    baseSlotCapacity[slotType] = capacity;
    // 同时更新当前容量（如果没有配件影响的话）
    if (currentSlotCapacity.find(slotType) == currentSlotCapacity.end()) {
        currentSlotCapacity[slotType] = capacity;
    }
}

int Gun::getSlotUsage(const std::string& slotType) const {
    auto it = slotUsage.find(slotType);
    if (it != slotUsage.end()) {
        return it->second;
    }
    return 0; // 如果槽位不存在，返回0
}

bool Gun::isSlotFull(const std::string& slotType) const {
    return getSlotUsage(slotType) >= getEffectiveSlotCapacity(slotType);
}

// 新增：白名单管理方法
void Gun::setSlotWhitelist(const std::string& slotType, const SlotWhitelist& whitelist) {
    slotWhitelists[slotType] = whitelist;
}

SlotWhitelist& Gun::getSlotWhitelist(const std::string& slotType) {
    return slotWhitelists[slotType];
}

bool Gun::canAttachToSlot(const std::string& slotType, const GunMod* mod) const {
    if (!mod) return false;
    
    // 检查槽位是否存在
    if (slotWhitelists.find(slotType) == slotWhitelists.end()) {
        return false;
    }
    
    // 检查白名单
    const SlotWhitelist& whitelist = slotWhitelists.at(slotType);
    if (!whitelist.isAllowed(mod)) {
        return false;
    }
    
    // 检查配件是否声明兼容此槽位
    if (!mod->canAttachToSlot(slotType)) {
        return false;
    }
    
    return true;
}

// 新增：弹药类型管理方法实现
void Gun::setBaseAcceptedAmmoTypes(const std::vector<std::string>& types) {
    baseAcceptedAmmoTypes = types;
    recalculateAmmoTypes(); // 重新计算当前有效弹药类型
}

const std::vector<std::string>& Gun::getBaseAcceptedAmmoTypes() const {
    return baseAcceptedAmmoTypes;
}

const std::vector<std::string>& Gun::getEffectiveAmmoTypes() const {
    return currentAcceptedAmmoTypes;
}

bool Gun::canAcceptAmmoType(const std::string& ammoType) const {
    return std::find(currentAcceptedAmmoTypes.begin(), currentAcceptedAmmoTypes.end(), ammoType) 
           != currentAcceptedAmmoTypes.end();
}

// 新增：射击模式管理方法实现
void Gun::setAvailableFiringModes(const std::vector<std::string>& modes) {
    availableFiringModes = modes;
    if (!availableFiringModes.empty()) {
        currentFiringMode = availableFiringModes[0];
    }
    
    // 更新射击模式标签
    updateFiringModeFlags();
}

const std::vector<std::string>& Gun::getAvailableFiringModes() const {
    return availableFiringModes;
}

const std::string& Gun::getCurrentFiringMode() const {
    return currentFiringMode;
}

void Gun::setCurrentFiringMode(const std::string& mode) {
    auto it = std::find(availableFiringModes.begin(), availableFiringModes.end(), mode);
    if (it != availableFiringModes.end()) {
        currentFiringMode = mode;
    }
}

void Gun::toggleFiringMode() {
    if (availableFiringModes.size() > 1) {
        auto it = std::find(availableFiringModes.begin(), availableFiringModes.end(), currentFiringMode);
        if (it != availableFiringModes.end()) {
            // 计算下一个模式的索引
            size_t currentIndex = std::distance(availableFiringModes.begin(), it);
            size_t nextIndex = (currentIndex + 1) % availableFiringModes.size();
            currentFiringMode = availableFiringModes[nextIndex];
        }
    }
}

// 新增：基于Flag的类型检查方法
bool Gun::isGunType(const std::string& gunType) const {
    ItemFlag flag = FlagMapper::stringToItemFlag(gunType);
    if (flag != ItemFlag::FLAG_COUNT) {
        return hasFlag(flag);
    }
    return false;
}

bool Gun::hasFiringMode(const std::string& mode) const {
    return std::find(availableFiringModes.begin(), availableFiringModes.end(), mode) 
           != availableFiringModes.end();
}

std::vector<std::string> Gun::getGunTypes() const {
    std::vector<std::string> types;
    
    // 检查所有枪械类型标签
    if (hasFlag(ItemFlag::PISTOL)) types.push_back("PISTOL");
    if (hasFlag(ItemFlag::REVOLVER)) types.push_back("REVOLVER");
    if (hasFlag(ItemFlag::SHOTGUN)) types.push_back("SHOTGUN");
    if (hasFlag(ItemFlag::SMG)) types.push_back("SMG");
    if (hasFlag(ItemFlag::RIFLE)) types.push_back("RIFLE");
    if (hasFlag(ItemFlag::SNIPER_RIFLE)) types.push_back("SNIPER_RIFLE");
    if (hasFlag(ItemFlag::DMR)) types.push_back("DMR");
    if (hasFlag(ItemFlag::MACHINE_GUN)) types.push_back("MACHINE_GUN");
    if (hasFlag(ItemFlag::GRENADE_LAUNCHER)) types.push_back("GRENADE_LAUNCHER");
    
    return types;
}

// 新增：更新射击模式标签
void Gun::updateFiringModeFlags() {
    // 清除所有射击模式标签
    removeFlag(ItemFlag::SEMI_AUTO);
    removeFlag(ItemFlag::FULL_AUTO);
    removeFlag(ItemFlag::BOLT_ACTION);
    removeFlag(ItemFlag::BURST);
    
    // 根据可用射击模式添加对应标签
    for (const std::string& mode : availableFiringModes) {
        ItemFlag flag = FlagMapper::stringToItemFlag(mode);
        if (flag != ItemFlag::FLAG_COUNT) {
            addFlag(flag);
        }
    }
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
    // 使用动态计算的当前有效弹药类型
    const auto& magAmmoTypes = mag->getCompatibleAmmoTypes();
    const auto& gunAmmoTypes = getEffectiveAmmoTypes();
    
    for (const auto& gunAmmoType : gunAmmoTypes) {
        for (const auto& magAmmoType : magAmmoTypes) {
            if (gunAmmoType == magAmmoType) {
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

// 重构配件安装方法
bool Gun::attach(const std::string& slotType, std::unique_ptr<GunMod> attachment) {
    // 检查是否为有效的配件
    if (!attachment) {
        return false;
    }
    
    // 检查槽位是否已满
    if (isSlotFull(slotType)) {
        return false;
    }
    
    // 检查白名单和兼容性
    if (!canAttachToSlot(slotType, attachment.get())) {
        return false;
    }
    
    // 添加配件到对应槽位的容器中
    attachmentSlots[slotType].push_back(std::move(attachment));
    
    // 更新槽位使用状态
    slotUsage[slotType]++;
    
    // 重新计算所有属性（考虑新配件的影响）
    recalculateAllStats();
    
    return true;
}

// 重构detach方法
std::unique_ptr<GunMod> Gun::detach(const std::string& slotType, int index) {
    auto slotIt = attachmentSlots.find(slotType);
    if (slotIt != attachmentSlots.end() && !slotIt->second.empty() && index < slotIt->second.size()) {
        // 获取要移除的配件
        auto it = slotIt->second.begin() + index;
        std::unique_ptr<GunMod> detachedMod = std::move(*it);
        
        // 从容器中移除
        slotIt->second.erase(it);
        
        // 更新槽位使用状态
        slotUsage[slotType]--;
        
        // 重新计算所有属性（移除配件的影响）
        recalculateAllStats();
        
        return detachedMod;
    }
    return nullptr;
}

GunMod* Gun::getAttachment(const std::string& slotType, int index) const {
    auto slotIt = attachmentSlots.find(slotType);
    if (slotIt != attachmentSlots.end() && !slotIt->second.empty() && index < slotIt->second.size()) {
        return slotIt->second[index].get();
    }
    return nullptr;
}

const std::vector<GunMod*> Gun::getAllAttachments(const std::string& slotType) const {
    std::vector<GunMod*> result;
    auto slotIt = attachmentSlots.find(slotType);
    if (slotIt != attachmentSlots.end()) {
        for (const auto& attachment : slotIt->second) {
            result.push_back(attachment.get());
        }
    }
    return result;
}

int Gun::getTotalAttachmentCount() const {
    int total = 0;
    for (const auto& [slotType, usage] : slotUsage) {
        total += usage;
    }
    return total;
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
    for (const auto& [slotType, attachmentVector] : attachmentSlots) {
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

// 新增：重计算方法实现
void Gun::recalculateAllStats() {
    recalculateSlotCapacities();
    recalculateAmmoTypes();
    recalculateMagazineNames();
    updateGunStats();
}

void Gun::recalculateSlotCapacities() {
    // 从基础容量开始
    currentSlotCapacity = baseSlotCapacity;
    
    // 应用所有配件的槽位容量影响
    for (const auto& [slotType, attachmentVector] : attachmentSlots) {
        for (const auto& attachment : attachmentVector) {
            if (attachment) {
                const auto& modifiers = attachment->getSlotCapacityModifiers();
                for (const auto& [targetSlot, modifier] : modifiers) {
                    currentSlotCapacity[targetSlot] += modifier;
                    // 确保容量不为负数
                    if (currentSlotCapacity[targetSlot] < 0) {
                        currentSlotCapacity[targetSlot] = 0;
                    }
                }
            }
        }
    }
}

void Gun::recalculateAmmoTypes() {
    // 从基础弹药类型开始
    currentAcceptedAmmoTypes = baseAcceptedAmmoTypes;
    
    // 应用所有配件的弹药类型影响
    for (const auto& [slotType, attachmentVector] : attachmentSlots) {
        for (const auto& attachment : attachmentVector) {
            if (attachment) {
                // 添加配件支持的弹药类型
                const auto& addedTypes = attachment->getAddedAmmoTypes();
                for (const std::string& ammoType : addedTypes) {
                    if (std::find(currentAcceptedAmmoTypes.begin(), currentAcceptedAmmoTypes.end(), ammoType) 
                        == currentAcceptedAmmoTypes.end()) {
                        currentAcceptedAmmoTypes.push_back(ammoType);
                    }
                }
                
                // 移除配件禁止的弹药类型
                const auto& removedTypes = attachment->getRemovedAmmoTypes();
                for (const std::string& ammoType : removedTypes) {
                    auto it = std::find(currentAcceptedAmmoTypes.begin(), currentAcceptedAmmoTypes.end(), ammoType);
                    if (it != currentAcceptedAmmoTypes.end()) {
                        currentAcceptedAmmoTypes.erase(it);
                    }
                }
            }
        }
    }
}

// 新增：弹匣兼容性管理方法实现
void Gun::setBaseAcceptedMagazineNames(const std::vector<std::string>& names) {
    baseAcceptedMagazineNames = names;
    recalculateMagazineNames(); // 重新计算当前有效弹匣类型
}

const std::vector<std::string>& Gun::getBaseAcceptedMagazineNames() const {
    return baseAcceptedMagazineNames;
}

const std::vector<std::string>& Gun::getEffectiveMagazineNames() const {
    return currentAcceptedMagazineNames;
}

bool Gun::canAcceptMagazine(const std::string& magazineName) const {
    return std::find(currentAcceptedMagazineNames.begin(), currentAcceptedMagazineNames.end(), magazineName) 
           != currentAcceptedMagazineNames.end();
}

void Gun::recalculateMagazineNames() {
    // 从基础弹匣类型开始
    currentAcceptedMagazineNames = baseAcceptedMagazineNames;
    
    // 应用所有配件的弹匣类型影响
    for (const auto& [slotType, attachmentVector] : attachmentSlots) {
        for (const auto& attachment : attachmentVector) {
            if (attachment) {
                // 添加配件支持的弹匣类型
                const auto& addedMagazines = attachment->getAddedMagazineNames();
                for (const std::string& magazineName : addedMagazines) {
                    if (std::find(currentAcceptedMagazineNames.begin(), currentAcceptedMagazineNames.end(), magazineName) 
                        == currentAcceptedMagazineNames.end()) {
                        currentAcceptedMagazineNames.push_back(magazineName);
                    }
                }
                
                // 移除配件禁止的弹匣类型
                const auto& removedMagazines = attachment->getRemovedMagazineNames();
                for (const std::string& magazineName : removedMagazines) {
                    auto it = std::find(currentAcceptedMagazineNames.begin(), currentAcceptedMagazineNames.end(), magazineName);
                    if (it != currentAcceptedMagazineNames.end()) {
                        currentAcceptedMagazineNames.erase(it);
                    }
                }
            }
        }
    }
    
    // 为了向后兼容，同步更新旧的acceptedMagazineNames字段
    acceptedMagazineNames = currentAcceptedMagazineNames;
}

// 新增：属性聚合方法实现
float Gun::getTotalWeight() const {
    float totalWeight = getWeight();
    
    // 添加所有配件的重量
    for (const auto& [slotType, attachmentVector] : attachmentSlots) {
        for (const auto& attachment : attachmentVector) {
            if (attachment) {
                totalWeight += attachment->getWeight();
            }
        }
    }
    
    // 添加当前弹匣的重量
    if (currentMagazine) {
        totalWeight += currentMagazine->getWeight();
    }
    
    // 添加膛内子弹的重量
    if (chamberedRound) {
        totalWeight += chamberedRound->getWeight();
    }
    
    return totalWeight;
}

float Gun::getTotalValue() const {
    float totalValue = getValue();
    
    // 添加所有配件的价值
    for (const auto& [slotType, attachmentVector] : attachmentSlots) {
        for (const auto& attachment : attachmentVector) {
            if (attachment) {
                totalValue += attachment->getValue();
            }
        }
    }
    
    // 添加当前弹匣的价值
    if (currentMagazine) {
        totalValue += currentMagazine->getValue();
    }
    
    // 添加膛内子弹的价值
    if (chamberedRound) {
        totalValue += chamberedRound->getValue();
    }
    
    return totalValue;
}

// 拷贝构造函数（重构）
Gun::Gun(const Gun& other)
    : Item(other) {
    // 拷贝射击模式相关成员
    currentFiringMode = other.currentFiringMode;
    availableFiringModes = other.availableFiringModes;
    
    // 拷贝弹药类型相关成员
    baseAcceptedAmmoTypes = other.baseAcceptedAmmoTypes;
    currentAcceptedAmmoTypes = other.currentAcceptedAmmoTypes;
    acceptedMagazineNames = other.acceptedMagazineNames;
    
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
    reloadTime = other.reloadTime;
    
    // 拷贝槽位相关成员
    baseSlotCapacity = other.baseSlotCapacity;
    currentSlotCapacity = other.currentSlotCapacity;
    slotUsage = other.slotUsage;
    slotWhitelists = other.slotWhitelists;
    
    // 深拷贝枪膛内的子弹
    if (other.chamberedRound) {
        chamberedRound = std::unique_ptr<Ammo>(static_cast<Ammo*>(other.chamberedRound->clone()));
    }
    
    // 深拷贝当前弹匣
    if (other.currentMagazine) {
        currentMagazine = std::unique_ptr<Magazine>(static_cast<Magazine*>(other.currentMagazine->clone()));
    }
    
    // 深拷贝所有槽位的所有配件
    for (const auto& [slotType, attachmentVector] : other.attachmentSlots) {
        // 为每个槽位创建一个新的配件容器
        attachmentSlots[slotType] = std::vector<std::unique_ptr<GunMod>>();
        
        // 复制该槽位的所有配件
        for (const auto& attachment : attachmentVector) {
            if (attachment) {
                attachmentSlots[slotType].push_back(
                    std::unique_ptr<GunMod>(static_cast<GunMod*>(attachment->clone()))
                );
            }
        }
    }
}

// 拷贝赋值运算符实现（重构）
Gun& Gun::operator=(const Gun& other) {
    if (this != &other) {
        // 先调用基类的赋值运算符
        Item::operator=(other);
        
        // 拷贝射击模式相关成员
        currentFiringMode = other.currentFiringMode;
        availableFiringModes = other.availableFiringModes;
        
        // 拷贝弹药类型相关成员
        baseAcceptedAmmoTypes = other.baseAcceptedAmmoTypes;
        currentAcceptedAmmoTypes = other.currentAcceptedAmmoTypes;
        acceptedMagazineNames = other.acceptedMagazineNames;
        
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
        reloadTime = other.reloadTime;
        
        // 拷贝槽位相关成员
        baseSlotCapacity = other.baseSlotCapacity;
        currentSlotCapacity = other.currentSlotCapacity;
        slotUsage = other.slotUsage;
        slotWhitelists = other.slotWhitelists;
        
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
        for (const auto& [slotType, attachmentVector] : other.attachmentSlots) {
            // 为每个槽位创建一个新的配件容器
            attachmentSlots[slotType] = std::vector<std::unique_ptr<GunMod>>();
            
            // 复制该槽位的所有配件
            for (const auto& attachment : attachmentVector) {
                if (attachment) {
                    attachmentSlots[slotType].push_back(
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