#pragma once
#ifndef SKILL_SYSTEM_H
#define SKILL_SYSTEM_H

#include <unordered_map>
#include <string>

// 技能类型枚举
enum class SkillType {
    // 火器技能
    PISTOL,         // 手枪
    RIFLE,          // 步枪
    SHOTGUN,        // 霰弹枪
    SMG,            // 冲锋枪
    SNIPER,         // 狙击枪
    HEAVY_WEAPONS,  // 重武器
    MARKSMANSHIP,   // 枪法（所有用枪的副技能）
    
    // 近战技能
    SLASHING,       // 斩击武器
    PIERCING,       // 刺击武器
    BLUNT,          // 钝击武器
    UNARMED,        // 徒手
    MELEE,          // 近战（所有近战副技能）
    
    // 其他技能
    THROWING,       // 投掷
    DODGE,          // 闪避
    CRAFTING,       // 制造
    ELECTRONICS,    // 电子
    MECHANICS,      // 机械
    COMPUTER,       // 计算机
    ATHLETICS,      // 运动
    DRIVING,        // 驾驶
    OTHERWORLD_SENSE, // 异界感知
    PSYCHIC,        // 异能
    COOKING,        // 烹饪
    CONSTRUCTION,   // 建造
    SMITHING,       // 锻打
    FARMING,        // 耕作
    TAILORING       // 裁缝
};

// 单个技能数据
struct Skill {
    int totalExperience;    // 总经验值
    int level;              // 当前等级 (0-20)
    int currentLevelExp;    // 当前等级内的经验值 (0-99)
    
    Skill() : totalExperience(0), level(0), currentLevelExp(0) {}
    
    // 根据总经验值计算等级和当前等级经验
    void updateLevelFromTotal() {
        level = std::min(20, totalExperience / 100);
        currentLevelExp = totalExperience % 100;
    }
    
    // 添加经验值
    void addExperience(int exp) {
        totalExperience += exp;
        updateLevelFromTotal();
    }
    
    // 获取到下一级所需经验
    int getExpToNextLevel() const {
        if (level >= 20) return 0; // 已满级
        return 100 - currentLevelExp;
    }
};

// 技能系统管理类
class SkillSystem {
private:
    std::unordered_map<SkillType, Skill> skills;
    
public:
    SkillSystem();
    
    // 添加经验值到指定技能
    void addExperience(SkillType skillType, int experience);
    
    // 获取技能等级
    int getSkillLevel(SkillType skillType) const;
    
    // 获取技能总经验值
    int getTotalExperience(SkillType skillType) const;
    
    // 获取技能当前等级内的经验值
    int getCurrentLevelExperience(SkillType skillType) const;
    
    // 获取到下一级所需经验
    int getExpToNextLevel(SkillType skillType) const;
    
    // 获取技能数据引用
    const Skill& getSkill(SkillType skillType) const;
    
    // 获取所有技能数据
    const std::unordered_map<SkillType, Skill>& getAllSkills() const { return skills; }
    
    // 技能类型转字符串（用于显示和调试）
    static std::string skillTypeToString(SkillType skillType);
    
    // 字符串转技能类型
    static SkillType stringToSkillType(const std::string& skillName);
    
    // 获取技能分类名称
    static std::string getSkillCategoryName(SkillType skillType);
};

#endif // SKILL_SYSTEM_H 