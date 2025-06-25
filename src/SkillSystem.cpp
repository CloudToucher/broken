#include "SkillSystem.h"
#include <algorithm>
#include <iostream>

// 构造函数：初始化所有技能
SkillSystem::SkillSystem() {
    // 初始化所有技能类型，默认为0级0经验
    skills[SkillType::PISTOL] = Skill();
    skills[SkillType::RIFLE] = Skill();
    skills[SkillType::SHOTGUN] = Skill();
    skills[SkillType::SMG] = Skill();
    skills[SkillType::SNIPER] = Skill();
    skills[SkillType::HEAVY_WEAPONS] = Skill();
    skills[SkillType::MARKSMANSHIP] = Skill();
    
    skills[SkillType::SLASHING] = Skill();
    skills[SkillType::PIERCING] = Skill();
    skills[SkillType::BLUNT] = Skill();
    skills[SkillType::UNARMED] = Skill();
    skills[SkillType::MELEE] = Skill();
    
    skills[SkillType::THROWING] = Skill();
    skills[SkillType::DODGE] = Skill();
    skills[SkillType::CRAFTING] = Skill();
    skills[SkillType::ELECTRONICS] = Skill();
    skills[SkillType::MECHANICS] = Skill();
    skills[SkillType::COMPUTER] = Skill();
    skills[SkillType::ATHLETICS] = Skill();
    skills[SkillType::DRIVING] = Skill();
    skills[SkillType::OTHERWORLD_SENSE] = Skill();
    skills[SkillType::PSYCHIC] = Skill();
    skills[SkillType::COOKING] = Skill();
    skills[SkillType::CONSTRUCTION] = Skill();
    skills[SkillType::SMITHING] = Skill();
    skills[SkillType::FARMING] = Skill();
    skills[SkillType::TAILORING] = Skill();
}

// 添加经验值到指定技能
void SkillSystem::addExperience(SkillType skillType, int experience) {
    if (experience <= 0) return;
    
    auto it = skills.find(skillType);
    if (it != skills.end()) {
        int oldLevel = it->second.level;
        it->second.addExperience(experience);
        
        // 如果升级了，输出提示
        if (it->second.level > oldLevel) {
            std::cout << "技能升级！" << skillTypeToString(skillType) 
                     << " 从 " << oldLevel << " 级升至 " << it->second.level << " 级！" << std::endl;
        }
    }
}

// 获取技能等级
int SkillSystem::getSkillLevel(SkillType skillType) const {
    auto it = skills.find(skillType);
    if (it != skills.end()) {
        return it->second.level;
    }
    return 0;
}

// 获取技能总经验值
int SkillSystem::getTotalExperience(SkillType skillType) const {
    auto it = skills.find(skillType);
    if (it != skills.end()) {
        return it->second.totalExperience;
    }
    return 0;
}

// 获取技能当前等级内的经验值
int SkillSystem::getCurrentLevelExperience(SkillType skillType) const {
    auto it = skills.find(skillType);
    if (it != skills.end()) {
        return it->second.currentLevelExp;
    }
    return 0;
}

// 获取到下一级所需经验
int SkillSystem::getExpToNextLevel(SkillType skillType) const {
    auto it = skills.find(skillType);
    if (it != skills.end()) {
        return it->second.getExpToNextLevel();
    }
    return 100;
}

// 获取技能数据引用
const Skill& SkillSystem::getSkill(SkillType skillType) const {
    auto it = skills.find(skillType);
    if (it != skills.end()) {
        return it->second;
    }
    
    // 如果找不到，返回一个默认的技能对象
    static Skill defaultSkill;
    return defaultSkill;
}

// 技能类型转字符串
std::string SkillSystem::skillTypeToString(SkillType skillType) {
    switch (skillType) {
        // 火器技能
        case SkillType::PISTOL: return "手枪";
        case SkillType::RIFLE: return "步枪";
        case SkillType::SHOTGUN: return "霰弹枪";
        case SkillType::SMG: return "冲锋枪";
        case SkillType::SNIPER: return "狙击枪";
        case SkillType::HEAVY_WEAPONS: return "重武器";
        case SkillType::MARKSMANSHIP: return "枪法";
        
        // 近战技能
        case SkillType::SLASHING: return "斩击武器";
        case SkillType::PIERCING: return "刺击武器";
        case SkillType::BLUNT: return "钝击武器";
        case SkillType::UNARMED: return "徒手";
        case SkillType::MELEE: return "近战";
        
        // 其他技能
        case SkillType::THROWING: return "投掷";
        case SkillType::DODGE: return "闪避";
        case SkillType::CRAFTING: return "制造";
        case SkillType::ELECTRONICS: return "电子";
        case SkillType::MECHANICS: return "机械";
        case SkillType::COMPUTER: return "计算机";
        case SkillType::ATHLETICS: return "运动";
        case SkillType::DRIVING: return "驾驶";
        case SkillType::OTHERWORLD_SENSE: return "异界感知";
        case SkillType::PSYCHIC: return "异能";
        case SkillType::COOKING: return "烹饪";
        case SkillType::CONSTRUCTION: return "建造";
        case SkillType::SMITHING: return "锻打";
        case SkillType::FARMING: return "耕作";
        case SkillType::TAILORING: return "裁缝";
        
        default: return "未知技能";
    }
}

// 字符串转技能类型
SkillType SkillSystem::stringToSkillType(const std::string& skillName) {
    if (skillName == "手枪") return SkillType::PISTOL;
    if (skillName == "步枪") return SkillType::RIFLE;
    if (skillName == "霰弹枪") return SkillType::SHOTGUN;
    if (skillName == "冲锋枪") return SkillType::SMG;
    if (skillName == "狙击枪") return SkillType::SNIPER;
    if (skillName == "重武器") return SkillType::HEAVY_WEAPONS;
    if (skillName == "枪法") return SkillType::MARKSMANSHIP;
    
    if (skillName == "斩击武器") return SkillType::SLASHING;
    if (skillName == "刺击武器") return SkillType::PIERCING;
    if (skillName == "钝击武器") return SkillType::BLUNT;
    if (skillName == "徒手") return SkillType::UNARMED;
    if (skillName == "近战") return SkillType::MELEE;
    
    if (skillName == "投掷") return SkillType::THROWING;
    if (skillName == "闪避") return SkillType::DODGE;
    if (skillName == "制造") return SkillType::CRAFTING;
    if (skillName == "电子") return SkillType::ELECTRONICS;
    if (skillName == "机械") return SkillType::MECHANICS;
    if (skillName == "计算机") return SkillType::COMPUTER;
    if (skillName == "运动") return SkillType::ATHLETICS;
    if (skillName == "驾驶") return SkillType::DRIVING;
    if (skillName == "异界感知") return SkillType::OTHERWORLD_SENSE;
    if (skillName == "异能") return SkillType::PSYCHIC;
    if (skillName == "烹饪") return SkillType::COOKING;
    if (skillName == "建造") return SkillType::CONSTRUCTION;
    if (skillName == "锻打") return SkillType::SMITHING;
    if (skillName == "耕作") return SkillType::FARMING;
    if (skillName == "裁缝") return SkillType::TAILORING;
    
    // 默认返回手枪技能
    return SkillType::PISTOL;
}

// 获取技能分类名称
std::string SkillSystem::getSkillCategoryName(SkillType skillType) {
    switch (skillType) {
        case SkillType::PISTOL:
        case SkillType::RIFLE:
        case SkillType::SHOTGUN:
        case SkillType::SMG:
        case SkillType::SNIPER:
        case SkillType::HEAVY_WEAPONS:
        case SkillType::MARKSMANSHIP:
            return "火器技能";
            
        case SkillType::SLASHING:
        case SkillType::PIERCING:
        case SkillType::BLUNT:
        case SkillType::UNARMED:
        case SkillType::MELEE:
            return "近战技能";
            
        case SkillType::THROWING:
        case SkillType::DODGE:
        case SkillType::CRAFTING:
        case SkillType::ELECTRONICS:
        case SkillType::MECHANICS:
        case SkillType::COMPUTER:
        case SkillType::ATHLETICS:
        case SkillType::DRIVING:
        case SkillType::OTHERWORLD_SENSE:
        case SkillType::PSYCHIC:
        case SkillType::COOKING:
        case SkillType::CONSTRUCTION:
        case SkillType::SMITHING:
        case SkillType::FARMING:
        case SkillType::TAILORING:
            return "生活技能";
            
        default:
            return "其他技能";
    }
} 