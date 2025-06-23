#include "Damage.h"
#include "Entity.h"
#include <algorithm>
#include <unordered_map>

// 实现DamageType转换为字符串的函数
std::string damageTypeToString(DamageType type) {
    switch (type) {
        case DamageType::SHOOTING: return "shooting";
        case DamageType::BLUNT: return "blunt";
        case DamageType::SLASH: return "slash";
        case DamageType::PIERCE: return "pierce";
        case DamageType::ELECTRIC: return "electric";
        case DamageType::BURN: return "burn";
        case DamageType::COLD: return "cold";
        case DamageType::HEAT: return "heat";
        case DamageType::TOXIC: return "toxic";
        case DamageType::HUNGER: return "hunger";
        case DamageType::THIRST: return "thirst";
        case DamageType::SUFFOCATION: return "suffocation";
        case DamageType::FALL: return "fall";
        case DamageType::EXPLOSION: return "explosion";
        case DamageType::RADIATION: return "radiation";
        case DamageType::ACID: return "acid";
        case DamageType::PSYCHIC: return "psychic";
        case DamageType::PURE: return "pure";
        default: return "unknown";
    }
}

// 实现字符串转换为DamageType的函数
DamageType stringToDamageType(const std::string& typeStr) {
    static const std::unordered_map<std::string, DamageType> typeMap = {
        {"shooting", DamageType::SHOOTING},
        {"blunt", DamageType::BLUNT},
        {"slash", DamageType::SLASH},
        {"pierce", DamageType::PIERCE},
        {"electric", DamageType::ELECTRIC},
        {"burn", DamageType::BURN},
        {"cold", DamageType::COLD},
        {"heat", DamageType::HEAT},
        {"toxic", DamageType::TOXIC},
        {"hunger", DamageType::HUNGER},
        {"thirst", DamageType::THIRST},
        {"suffocation", DamageType::SUFFOCATION},
        {"fall", DamageType::FALL},
        {"explosion", DamageType::EXPLOSION},
        {"radiation", DamageType::RADIATION},
        {"acid", DamageType::ACID},
        {"psychic", DamageType::PSYCHIC},
        {"pure", DamageType::PURE}
    };
    
    auto it = typeMap.find(typeStr);
    if (it != typeMap.end()) {
        return it->second;
    }
    
    return DamageType::PURE; // 默认为纯粹伤害
}

// 实现Damage类的构造函数
Damage::Damage()
    : source(nullptr), precision(0.0f) {
}

Damage::Damage(Entity* damageSource, float precisionValue)
    : source(damageSource), precision(precisionValue) {
}

// 实现析构函数
Damage::~Damage() {
    // 不需要特殊清理，因为没有动态分配的资源
}

// 添加伤害（字符串类型）
void Damage::addDamage(const std::string& type, int amount, int penetration) {
    // 如果伤害值小于等于0，不添加
    if (amount <= 0) {
        return;
    }
    
    // 检查是否已存在相同类型的伤害
    for (auto& damage : damageList) {
        if (std::get<0>(damage) == type) {
            // 如果存在，累加伤害值，并更新穿透值（取较大值）
            std::get<1>(damage) += amount;
            if (penetration > std::get<2>(damage)) {
                std::get<2>(damage) = penetration;
            }
            return;
        }
    }
    
    // 如果不存在，添加新的伤害类型
    damageList.emplace_back(type, amount, penetration);
}

// 添加伤害（枚举类型）
void Damage::addDamage(DamageType type, int amount, int penetration) {
    addDamage(damageTypeToString(type), amount, penetration);
}

// 获取伤害列表
const std::vector<std::tuple<std::string, int, int>>& Damage::getDamageList() const {
    return damageList;
}

// 获取伤害来源
Entity* Damage::getSource() const {
    return source;
}

// 设置伤害来源
void Damage::setSource(Entity* damageSource) {
    source = damageSource;
}

// 获取精准度
float Damage::getPrecision() const {
    return precision;
}

// 设置精准度
void Damage::setPrecision(float precisionValue) {
    precision = precisionValue;
}

// 获取总伤害值
int Damage::getTotalDamage() const {
    int total = 0;
    for (const auto& damage : damageList) {
        total += std::get<1>(damage);
    }
    return total;
}

// 获取特定类型的伤害值（字符串类型）
int Damage::getDamageByType(const std::string& type) const {
    for (const auto& damage : damageList) {
        if (std::get<0>(damage) == type) {
            return std::get<1>(damage);
        }
    }
    return 0;
}

// 获取特定类型的伤害值（枚举类型）
int Damage::getDamageByType(DamageType type) const {
    return getDamageByType(damageTypeToString(type));
}

// 清空伤害列表
void Damage::clear() {
    damageList.clear();
}

// 检查伤害列表是否为空
bool Damage::isEmpty() const {
    return damageList.empty();
}

// 合并伤害
void Damage::merge(const Damage& other) {
    // 合并伤害列表
    for (const auto& damage : other.damageList) {
        addDamage(std::get<0>(damage), std::get<1>(damage), std::get<2>(damage));
    }
    
    // 如果当前伤害没有来源，但其他伤害有来源，则使用其他伤害的来源
    if (!source && other.source) {
        source = other.source;
    }
    
    // 取较高的精准度
    precision = std::max(precision, other.precision);
}

// 缩放伤害（乘以系数）
void Damage::scale(float factor) {
    if (factor <= 0) {
        clear();
        return;
    }
    
    for (auto& damage : damageList) {
        std::get<1>(damage) = static_cast<int>(std::get<1>(damage) * factor);
    }
    
    // 移除伤害值为0的项
    damageList.erase(
        std::remove_if(damageList.begin(), damageList.end(),
            [](const auto& damage) { return std::get<1>(damage) <= 0; }),
        damageList.end()
    );
} 