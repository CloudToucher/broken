#pragma once
#include "Item.h"
#include "AttackSystem.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

// 武器类型枚举
enum class WeaponType {
    MELEE,      // 近战武器
    RANGED,     // 远程武器
    THROWN,     // 投掷武器
    SPECIAL     // 特殊武器
};

// 特殊效果类型
enum class SpecialEffectType {
    NONE,
    POISON,         // 中毒
    FIRE,           // 燃烧
    FREEZE,         // 冰冻
    ELECTRIC,       // 电击
    VAMPIRE,        // 吸血
    KNOCKBACK,      // 击退
    STUN,           // 眩晕
    ARMOR_PIERCE,   // 破甲
    MULTI_HIT,      // 多重攻击
    CHAIN_ATTACK,   // 连锁攻击
    EXPLOSIVE,      // 爆炸
    CUSTOM          // 自定义效果（需要代码实现）
};

// 特殊效果数据结构
struct SpecialEffect {
    SpecialEffectType type = SpecialEffectType::NONE;
    float chance = 0.0f;        // 触发概率 (0.0-1.0)
    float duration = 0.0f;      // 持续时间（秒）
    float magnitude = 0.0f;     // 效果强度
    std::string customName;     // 自定义效果名称（用于CUSTOM类型）
    std::unordered_map<std::string, float> parameters; // 额外参数
};

// 通用武器类
class Weapon : public Item, public IWeaponAttack {
private:
    // 基础武器属性
    WeaponType weaponType;
    AttackMethod primaryAttackMethod;
    std::vector<AttackMethod> availableAttackMethods;
    
    // 攻击属性
    float baseDamage;
    float range;
    float attackSpeed;          // 每秒攻击次数
    float criticalChance;       // 暴击率 (0.0-1.0)
    float criticalMultiplier;   // 暴击倍数
    
    // 命中和精度
    float accuracy;             // 命中率 (0.0-1.0)
    float penetration;          // 穿透力
    
    // 耐久系统
    int maxDurability;
    int currentDurability;
    float durabilityLossPerUse; // 每次使用损失的耐久度
    
    // 特殊效果
    std::vector<SpecialEffect> specialEffects;
    
    // 连击系统
    bool supportsCombo;
    int maxComboCount;
    float comboWindow;          // 连击窗口时间
    float comboDamageBonus;     // 每层连击的伤害加成
    
    // 音效和视觉效果
    std::string attackSound;
    std::string hitSound;
    std::string criticalSound;
    std::string comboSound;
    
    // 动画相关
    float animationSpeed;
    std::string animationName;
    
    // 需求属性
    int requiredStrength;
    int requiredDexterity;
    int requiredIntelligence;
    
public:
    // 构造函数
    Weapon(const std::string& name = "Generic Weapon");
    virtual ~Weapon() = default;
    
    // Item接口实现
    virtual Item* clone() const override;
    
    // IWeaponAttack接口实现
    virtual AttackMethod getAttackMethod(WeaponAttackType type = WeaponAttackType::PRIMARY) const override { return primaryAttackMethod; }
    virtual AttackParams getAttackParams(WeaponAttackType type = WeaponAttackType::PRIMARY) const override;
    virtual bool canPerformAttack(WeaponAttackType type = WeaponAttackType::PRIMARY) const override;
    virtual void onAttackPerformed(WeaponAttackType type = WeaponAttackType::PRIMARY) override;
    
    // 武器类型相关
    WeaponType getWeaponType() const { return weaponType; }
    void setWeaponType(WeaponType type) { weaponType = type; }
    
    // 攻击方式相关
    AttackMethod getPrimaryAttackMethod() const { return primaryAttackMethod; }
    void setPrimaryAttackMethod(AttackMethod method) { primaryAttackMethod = method; }
    const std::vector<AttackMethod>& getAvailableAttackMethods() const { return availableAttackMethods; }
    void addAttackMethod(AttackMethod method);
    void removeAttackMethod(AttackMethod method);
    bool hasAttackMethod(AttackMethod method) const;
    
    // 基础属性访问器
    float getBaseDamage() const { return baseDamage; }
    void setBaseDamage(float damage) { baseDamage = damage; }
    
    float getRange() const { return range; }
    void setRange(float r) { range = r; }
    
    float getAttackSpeed() const { return attackSpeed; }
    void setAttackSpeed(float speed) { attackSpeed = speed; }
    
    float getCriticalChance() const { return criticalChance; }
    void setCriticalChance(float chance) { criticalChance = chance; }
    
    float getCriticalMultiplier() const { return criticalMultiplier; }
    void setCriticalMultiplier(float multiplier) { criticalMultiplier = multiplier; }
    
    float getAccuracy() const { return accuracy; }
    void setAccuracy(float acc) { accuracy = acc; }
    
    float getPenetration() const { return penetration; }
    void setPenetration(float pen) { penetration = pen; }
    
    // 耐久系统
    int getMaxDurability() const { return maxDurability; }
    void setMaxDurability(int durability) { maxDurability = durability; }
    
    int getCurrentDurability() const { return currentDurability; }
    void setCurrentDurability(int durability) { currentDurability = durability; }
    
    float getDurabilityPercentage() const;
    void reduceDurability(float amount = 1.0f);
    void repairWeapon(int amount);
    bool isBroken() const { return currentDurability <= 0; }
    
    // 特殊效果系统
    void addSpecialEffect(const SpecialEffect& effect);
    void removeSpecialEffect(SpecialEffectType type);
    const std::vector<SpecialEffect>& getSpecialEffects() const { return specialEffects; }
    bool hasSpecialEffect(SpecialEffectType type) const;
    
    // 连击系统
    bool getSupportsCombo() const { return supportsCombo; }
    void setSupportsCombo(bool supports) { supportsCombo = supports; }
    
    int getMaxComboCount() const { return maxComboCount; }
    void setMaxComboCount(int count) { maxComboCount = count; }
    
    float getComboWindow() const { return comboWindow; }
    void setComboWindow(float window) { comboWindow = window; }
    
    float getComboDamageBonus() const { return comboDamageBonus; }
    void setComboDamageBonus(float bonus) { comboDamageBonus = bonus; }
    
    // 音效相关
    const std::string& getAttackSound() const { return attackSound; }
    void setAttackSound(const std::string& sound) { attackSound = sound; }
    
    const std::string& getHitSound() const { return hitSound; }
    void setHitSound(const std::string& sound) { hitSound = sound; }
    
    const std::string& getCriticalSound() const { return criticalSound; }
    void setCriticalSound(const std::string& sound) { criticalSound = sound; }
    
    const std::string& getComboSound() const { return comboSound; }
    void setComboSound(const std::string& sound) { comboSound = sound; }
    
    // 动画相关
    float getAnimationSpeed() const { return animationSpeed; }
    void setAnimationSpeed(float speed) { animationSpeed = speed; }
    
    const std::string& getAnimationName() const { return animationName; }
    void setAnimationName(const std::string& name) { animationName = name; }
    
    // 需求属性
    int getRequiredStrength() const { return requiredStrength; }
    void setRequiredStrength(int strength) { requiredStrength = strength; }
    
    int getRequiredDexterity() const { return requiredDexterity; }
    void setRequiredDexterity(int dexterity) { requiredDexterity = dexterity; }
    
    int getRequiredIntelligence() const { return requiredIntelligence; }
    void setRequiredIntelligence(int intelligence) { requiredIntelligence = intelligence; }
    
    // 计算实际属性（考虑耐久度影响）
    float getEffectiveDamage() const;
    float getEffectiveAccuracy() const;
    float getEffectiveAttackSpeed() const;
    
    // 实用方法
    bool canBeUsedBy(Entity* entity) const;
    std::string getWeaponTypeString() const;
    std::string getDetailedInfo() const;
    
private:
    // 内部方法
    void applySpecialEffects(Entity* target);
    float calculateDurabilityModifier() const;
};

// 特殊效果管理器
class SpecialEffectManager {
public:
    static SpecialEffectManager* getInstance();
    
    // 注册自定义特殊效果
    void registerCustomEffect(const std::string& name, std::function<void(Entity*, Entity*, const SpecialEffect&)> effect);
    
    // 应用特殊效果
    void applyEffect(Entity* attacker, Entity* target, const SpecialEffect& effect);
    
    // 从字符串解析特殊效果类型
    static SpecialEffectType parseEffectType(const std::string& typeStr);
    static std::string effectTypeToString(SpecialEffectType type);
    
private:
    static SpecialEffectManager* instance;
    std::unordered_map<std::string, std::function<void(Entity*, Entity*, const SpecialEffect&)>> customEffects;
    
    SpecialEffectManager() = default;
    
    // 内置特殊效果实现
    void applyPoisonEffect(Entity* target, const SpecialEffect& effect);
    void applyFireEffect(Entity* target, const SpecialEffect& effect);
    void applyFreezeEffect(Entity* target, const SpecialEffect& effect);
    void applyElectricEffect(Entity* target, const SpecialEffect& effect);
    void applyVampireEffect(Entity* attacker, Entity* target, const SpecialEffect& effect);
    void applyKnockbackEffect(Entity* attacker, Entity* target, const SpecialEffect& effect);
    void applyStunEffect(Entity* target, const SpecialEffect& effect);
    void applyArmorPierceEffect(Entity* target, const SpecialEffect& effect);
    void applyExplosiveEffect(Entity* attacker, Entity* target, const SpecialEffect& effect);
}; 