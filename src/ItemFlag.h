#pragma once
#ifndef ITEM_FLAG_H
#define ITEM_FLAG_H

#include <string>

// 物品标签枚举
enum class ItemFlag {
    // 基本属性标签
    WEARABLE,           // 可穿戴的
    STACKABLE,          // 可堆叠的
    CONSUMABLE,         // 可消耗的
    CONTAINER,          // 容器
    SINGLE_SLOT,        // 容器只能容纳一个物品
    EXPANDS_WITH_CONTENTS, // 显示当前内容物体积（不改变最大容量）
    
    //类别
    ARMOR,              // 护甲
    FOOD,               // 食物
    MEDICAL,            // 医疗
    TOOL,               // 工具
    MISC,               // 杂项
    
    // 新增标志
    ONLY_ARMOR_PLATE,   // 只能容纳防弹甲片
    USES_POWER,         // 消耗电力
    ARMOR_PLATE,        // 防弹甲片
    STRENGTH_BOOST,     // 力量增强
    HEAVY,              // 重型装备


    // 稀有度标签
    COMMON,             // 常规
    RARE,               // 稀有
    EPIC,               // 史诗
    LEGENDARY,          // 传说
    MYTHIC,             // 神话
    
    // 身体部位标签
    SLOT_HEAD,          // 头部
    SLOT_CHEST,         // 胸部
    SLOT_ABDOMEN,       // 腹部
    SLOT_LEFT_LEG,      // 左腿
    SLOT_RIGHT_LEG,     // 右腿
    SLOT_LEFT_FOOT,     // 左脚
    SLOT_RIGHT_FOOT,    // 右脚
    SLOT_LEFT_ARM,      // 左臂
    SLOT_RIGHT_ARM,     // 右臂
    SLOT_LEFT_HAND,     // 左手
    SLOT_RIGHT_HAND,    // 右手
    SLOT_BACK,          // 背部
    
    // 武器类型标签
    WEAPON,             // 武器
    GUN,                // 枪械
    MELEE,              // 近战武器
    THROWABLE,          // 投掷武器
    GUNMOD,             // 枪械配件
    
    // 近战武器子类型标签
    SWORD,              // 剑类
    AXE,                // 斧类
    HAMMER,             // 锤类
    SPEAR,              // 矛类
    DAGGER,             // 匕首类
    
    // 枪械类型标签
    PISTOL,             // 手枪
    REVOLVER,           // 左轮手枪
    SHOTGUN,            // 霰弹枪
    SMG,                // 冲锋枪
    RIFLE,              // 步枪
    DMR,                // 精确射手步枪
    SNIPER_RIFLE,       // 狙击步枪
    MACHINE_GUN,        // 机枪
    GRENADE_LAUNCHER,   // 榴弹发射器
    
    // 弹药相关标签
    MAGAZINE,           // 弹匣
    AMMO,               // 子弹
    
    
    // 射击模式标签
    SEMI_AUTO,          // 半自动
    FULL_AUTO,          // 全自动
    BOLT_ACTION,        // 栓动
    BURST,              // 点射
    
    // 枪械配件槽位标签
    GUN_MOD,            // 配件
    MOD_STOCK,          // 枪托
    MOD_BARREL,         // 枪管
    MOD_UNDER_BARREL,   // 下挂
    MOD_GRIP,           // 握把
    MOD_OPTIC,          // 瞄准镜
    MOD_SIDE_MOUNT,     // 侧挂
    MOD_MUZZLE,         // 枪口
    MOD_MAGAZINE_WELL,  // 弹匣井
    MOD_RAIL,           // 导轨

    // 导轨效果
    MOD_LASER,          // 激光
    MOD_FLASHLIGHT,     // 战术灯
    
    // 其他特性标签
    SILENCED,           // 带消音
    SCOPE,              // 带瞄准镜
    LASER,              // 带激光瞄准器
    FLASHLIGHT,         // 带手电筒
    
    // 必须是最后一个元素，用于循环计数
    FLAG_COUNT
};

// 获取标签的字符串表示
inline std::string GetItemFlagName(ItemFlag flag) {
    switch (flag) {
        // 基本属性标签
        case ItemFlag::WEARABLE: return "可穿戴的";
        case ItemFlag::STACKABLE: return "可堆叠的";
        case ItemFlag::CONSUMABLE: return "可消耗的";
        case ItemFlag::CONTAINER: return "容器";
        case ItemFlag::SINGLE_SLOT: return "单槽容器";
        case ItemFlag::EXPANDS_WITH_CONTENTS: return "随内容物变大";
        
        // 类别标签
        case ItemFlag::ARMOR: return "护甲";
        case ItemFlag::FOOD: return "食物";
        case ItemFlag::MEDICAL: return "医疗";
        case ItemFlag::TOOL: return "工具";
        case ItemFlag::MISC: return "杂项";
        
        // 新增标志
        case ItemFlag::ONLY_ARMOR_PLATE: return "只能容纳防弹甲片";
        case ItemFlag::USES_POWER: return "消耗电力";
        case ItemFlag::ARMOR_PLATE: return "防弹甲片";
        case ItemFlag::STRENGTH_BOOST: return "力量增强";
        case ItemFlag::HEAVY: return "重型装备";
        
        // 稀有度标签
        case ItemFlag::COMMON: return "常规";
        case ItemFlag::RARE: return "稀有";
        case ItemFlag::EPIC: return "史诗";
        case ItemFlag::LEGENDARY: return "传说";
        case ItemFlag::MYTHIC: return "神话";
        
        // 身体部位标签
        case ItemFlag::SLOT_HEAD: return "头部";
        case ItemFlag::SLOT_CHEST: return "胸部";
        case ItemFlag::SLOT_ABDOMEN: return "腹部";
        case ItemFlag::SLOT_LEFT_LEG: return "左腿";
        case ItemFlag::SLOT_RIGHT_LEG: return "右腿";
        case ItemFlag::SLOT_LEFT_FOOT: return "左脚";
        case ItemFlag::SLOT_RIGHT_FOOT: return "右脚";
        case ItemFlag::SLOT_LEFT_ARM: return "左臂";
        case ItemFlag::SLOT_RIGHT_ARM: return "右臂";
        case ItemFlag::SLOT_LEFT_HAND: return "左手";
        case ItemFlag::SLOT_RIGHT_HAND: return "右手";
        case ItemFlag::SLOT_BACK: return "背部";

        
        // 武器类型标签
        case ItemFlag::WEAPON: return "武器";
        case ItemFlag::GUN: return "枪械";
        case ItemFlag::MELEE: return "近战武器";
        case ItemFlag::THROWABLE: return "投掷武器";
        case ItemFlag::GUNMOD: return "枪械配件";
        
        // 近战武器子类型标签
        case ItemFlag::SWORD: return "剑类";
        case ItemFlag::AXE: return "斧类";
        case ItemFlag::HAMMER: return "锤类";
        case ItemFlag::SPEAR: return "矛类";
        case ItemFlag::DAGGER: return "匕首类";
        
        // 枪械类型标签
        case ItemFlag::PISTOL: return "手枪";
        case ItemFlag::RIFLE: return "步枪";
        case ItemFlag::SMG: return "冲锋枪";
        case ItemFlag::SHOTGUN: return "霰弹枪";
        case ItemFlag::SNIPER_RIFLE: return "狙击步枪";
        case ItemFlag::DMR: return "精确射手步枪";
        case ItemFlag::MACHINE_GUN: return "轻机枪";
        
        // 弹药相关标签
        case ItemFlag::MAGAZINE: return "弹匣";
        case ItemFlag::AMMO: return "子弹";
        
        // 射击模式标签
        case ItemFlag::SEMI_AUTO: return "半自动";
        case ItemFlag::FULL_AUTO: return "全自动";
        case ItemFlag::BOLT_ACTION: return "栓动";
        case ItemFlag::BURST: return "点射";
        
        // 枪械配件槽位标签
        case ItemFlag::MOD_STOCK: return "枪托";
        case ItemFlag::MOD_BARREL: return "枪管";
        case ItemFlag::MOD_UNDER_BARREL: return "下挂";
        case ItemFlag::MOD_GRIP: return "握把";
        case ItemFlag::MOD_OPTIC: return "瞄准镜";
        case ItemFlag::MOD_SIDE_MOUNT: return "侧挂";
        case ItemFlag::MOD_MUZZLE: return "枪口";
        case ItemFlag::MOD_MAGAZINE_WELL: return "弹匣井";
        case ItemFlag::MOD_LASER: return "激光";
        case ItemFlag::MOD_FLASHLIGHT: return "战术灯";
        
        // 其他特性标签
        case ItemFlag::SILENCED: return "消音";
        case ItemFlag::SCOPE: return "带瞄准镜";
        
        default: return "未知标签";
    }
}

#endif // ITEM_FLAG_H