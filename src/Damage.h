#pragma once
#ifndef DAMAGE_H
#define DAMAGE_H

#include <vector>
#include <tuple>
#include <string>

// 前向声明
class Entity;

// 伤害类型枚举
enum class DamageType {
    // 主要防护类型（用于防护系统）
    BLUNT,       // 钝击伤害
    SLASH,       // 斩击伤害  
    PIERCE,      // 刺击伤害
    ELECTRIC,    // 电击伤害
    BURN,        // 火焰伤害
    HEAT,        // 高温伤害
    COLD,        // 寒冷伤害
    EXPLOSION,   // 爆炸伤害
    SHOOTING,    // 射击伤害
    
    // 其他伤害类型（兼容现有系统）
    TOXIC,       // 毒素伤害
    HUNGER,      // 饥饿伤害
    THIRST,      // 口渴伤害
    SUFFOCATION, // 窒息伤害
    FALL,        // 坠落伤害
    RADIATION,   // 辐射伤害
    ACID,        // 酸性伤害
    PSYCHIC,     // 精神伤害
    PURE         // 纯粹伤害（无视防御）
};

// 将DamageType转换为字符串
std::string damageTypeToString(DamageType type);

// 将字符串转换为DamageType
DamageType stringToDamageType(const std::string& typeStr);

// 伤害类，用于存储伤害信息
class Damage {
private:
    // 伤害列表：<伤害类型, 伤害值, 穿透值>
    // 穿透值默认为-1，表示不考虑穿透
    std::vector<std::tuple<std::string, int, int>> damageList;
    
    // 伤害来源，如果为nullptr表示环境伤害或生命流失
    Entity* source;
    
    // 精准度，用于计算暴击或弱点伤害
    float precision;

public:
    // 构造函数
    Damage();
    
    // 带参数的构造函数
    Damage(Entity* damageSource, float precisionValue = 0.0f);
    
    // 析构函数
    ~Damage();
    
    // 添加伤害
    void addDamage(const std::string& type, int amount, int penetration = -1);
    void addDamage(DamageType type, int amount, int penetration = -1);
    
    // 获取伤害列表
    const std::vector<std::tuple<std::string, int, int>>& getDamageList() const;
    
    // 获取伤害来源
    Entity* getSource() const;
    
    // 设置伤害来源
    void setSource(Entity* damageSource);
    
    // 获取精准度
    float getPrecision() const;
    
    // 设置精准度
    void setPrecision(float precisionValue);
    
    // 获取总伤害值
    int getTotalDamage() const;
    
    // 获取特定类型的伤害值
    int getDamageByType(const std::string& type) const;
    int getDamageByType(DamageType type) const;
    
    // 清空伤害列表
    void clear();
    
    // 检查伤害列表是否为空
    bool isEmpty() const;
    
    // 合并伤害
    void merge(const Damage& other);
    
    // 缩放伤害（乘以系数）
    void scale(float factor);
};

#endif // DAMAGE_H 