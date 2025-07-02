# Gun、GunMod、Magazine 系统框架分析

## 概述

当前的武器系统基于继承架构，所有武器相关类都继承自 `Item` 基类。系统支持：
- 枪械本体 (Gun)
- 枪械配件 (GunMod) 
- 弹匣系统 (Magazine)
- 弹药系统 (Ammo)

## 1. 核心类结构

### 1.1 Gun 类

**继承关系**: `Gun : public Item`

**核心特性**:
- 基础属性与最终属性分离设计
- 多槽位配件系统
- 弹匣兼容性系统
- 射击模式支持
- 枪膛管理

**关键成员变量**:

```cpp
// 枪膛与弹匣
std::unique_ptr<Ammo> chamberedRound;           // 枪膛内子弹
std::unique_ptr<Magazine> currentMagazine;      // 当前弹匣
std::vector<std::string> acceptedMagazineNames; // 兼容弹匣列表

// 枪械类型与射击模式
GunType gunType;                                // 枪械类型
FiringMode currentFiringMode;                   // 当前射击模式
std::vector<FiringMode> availableFiringModes;   // 可用射击模式

// 基础属性 (base*) 与最终属性 (无前缀)
float baseSoundLevel, soundLevel;               // 声音级别
float baseFireRate, fireRate;                  // 射速
float baseAccuracyMOA, accuracyMOA;            // 精度
float baseRecoil, recoil;                      // 后坐力
float baseErgonomics, ergonomics;              // 人体工程学
float baseBreathStability, breathStability;    // 呼吸稳定性

// 对子弹的加成
float baseDamageBonus, damageBonus;            // 伤害加成
float baseRangeBonus, rangeBonus;              // 射程加成
float baseBulletSpeedBonus, bulletSpeedBonus; // 速度加成
float basePenetrationBonus, penetrationBonus; // 穿透加成

// 配件系统
std::map<AttachmentSlot, int> slotCapacity;    // 槽位容量
std::map<AttachmentSlot, int> slotUsage;       // 槽位使用情况
std::map<AttachmentSlot, std::vector<std::unique_ptr<GunMod>>> attachmentSlots; // 配件存储
```

**配件槽位类型**:
```cpp
enum class AttachmentSlot {
    STOCK,          // 枪托
    BARREL,         // 枪管  
    UNDER_BARREL,   // 下挂
    GRIP,           // 握把
    OPTIC,          // 光学瞄准镜
    SIDE_MOUNT,     // 侧挂
    MUZZLE,         // 枪口
    MAGAZINE_WELL,  // 弹匣井
    RAIL,           // 导轨
    SPECIAL,        // 特殊槽位
};
```

**关键方法**:
```cpp
// 射击系统
std::unique_ptr<Ammo> shoot();                 // 射击
bool chamberManually();                        // 手动上膛
bool canShoot() const;                         // 检查能否射击

// 弹匣系统
bool canAcceptMagazine(const Magazine* mag);   // 检查弹匣兼容性
void loadMagazine(std::unique_ptr<Magazine>);  // 装载弹匣
std::unique_ptr<Magazine> unloadMagazine();    // 卸载弹匣

// 配件系统
bool attach(AttachmentSlot, std::unique_ptr<GunMod>);     // 安装配件
std::unique_ptr<GunMod> detach(AttachmentSlot, int);     // 拆卸配件
void updateGunStats();                                    // 更新最终属性

// 槽位管理
int getSlotCapacity(AttachmentSlot) const;               // 获取槽位容量
bool isSlotFull(AttachmentSlot) const;                   // 检查槽位是否已满
```

### 1.2 GunMod 类

**继承关系**: `GunMod : public Item`

**核心特性**:
- 对枪械属性的修正值设计
- 支持正负修正
- 分为枪械属性修正和子弹属性修正两类

**关键成员变量**:
```cpp
// 对枪械属性的影响
float modSoundLevel;        // 声音级别影响
float modFireRate;          // 射速影响  
float modAccuracyMOA;       // 精度影响
float modRecoil;            // 后坐力影响
float modErgonomics;        // 人体工程学影响
float modBreathStability;   // 呼吸稳定性影响

// 对子弹属性的影响
float modDamageBonus;       // 伤害加成影响
float modRangeBonus;        // 射程影响
float modBulletSpeedBonus;  // 子弹速度影响
float modPenetrationBonus;  // 穿透力加成影响
```

**关键方法**:
```cpp
// 设置所有修正值
void setModAttributes(float soundLevel, float fireRate, float damageBonus, 
                     float accuracyMOA, float recoil, float ergonomics, 
                     float breathStability, float range, float bulletSpeed, 
                     float penetrationBonus);

// Getter/Setter 方法
float getModSoundLevel() const;
void setModSoundLevel(float value);
// ... 其他属性的 getter/setter
```

### 1.3 Magazine 类

**继承关系**: `Magazine : public Item`

**核心特性**:
- 基于 std::stack 的弹药存储 (LIFO)
- 弹药类型兼容性检查
- 装填/卸载时间模拟

**关键成员变量**:
```cpp
std::vector<std::string> compatibleAmmoTypes;  // 兼容弹药类型
int maxCapacity;                              // 最大容量
std::stack<std::unique_ptr<Ammo>> currentAmmo; // 当前弹药栈
float unloadTime;                             // 卸载时间
float reloadTime;                             // 装填时间
```

**关键方法**:
```cpp
bool canAcceptAmmo(const std::string& ammoType);  // 检查弹药兼容性
bool loadAmmo(std::unique_ptr<Ammo> ammo);        // 装载弹药
std::unique_ptr<Ammo> consumeAmmo();              // 消耗弹药
bool isEmpty() const;                             // 检查是否为空
bool isFull() const;                              // 检查是否已满
int getCurrentAmmoCount() const;                  // 获取当前弹药数量
```

### 1.4 Ammo 类

**继承关系**: `Ammo : public Item`

**核心特性**:
- 基础弹药属性
- 对枪械性能的修正值

**关键成员变量**:
```cpp
// 基础属性
int baseDamage;              // 基础伤害
float basePenetration;       // 基础穿透力
float baseRange;             // 基础射程
float baseSpeed;             // 基础速度

// 对枪械的修正
float modRecoil;             // 后坐力修正
float modAccuracyMOA;        // 精度修正
float modErgonomics;         // 人体工程学修正

std::string ammoType;        // 弹药类型标识
```

## 2. 枚举定义

### 2.1 枪械类型
```cpp
enum class GunType {
    PISTOL,           // 手枪
    REVOLVER,         // 左轮手枪
    SHOTGUN,          // 霰弹枪
    SMG,              // 冲锋枪
    RIFLE,            // 步枪
    SNIPER_RIFLE,     // 狙击步枪
    DMR,              // 精确射手步枪
    MACHINE_GUN,      // 机枪
    GRENADE_LAUNCHER  // 榴弹发射器
};
```

### 2.2 射击模式
```cpp
enum class FiringMode {
    SEMI_AUTO,   // 半自动
    FULL_AUTO,   // 全自动
    BOLT_ACTION, // 手动上膛
    BURST        // 点射
};
```

## 3. JSON 配置结构

### 3.1 Gun 配置示例
```json
{
    "name": "HK416",
    "weight": 3.5,
    "volume": 0.8,
    "length": 1.0,
    "value": 1500,
    "gunType": "RIFLE",
    "fireRate": 850.0,
    "accuracy": 1.2,
    "recoil": 12.0,
    "ergonomics": 60.0,
    "breathStability": 0.9,
    "soundLevel": 150.0,
    "damageBonus": 0.0,
    "rangeBonus": 5.0,
    "bulletSpeedBonus": 0.0,
    "penetrationBonus": 0.0,
    "firingModes": ["SEMI_AUTO", "FULL_AUTO"],
    "ammoTypes": ["5.56x45mm"],
    "acceptedMagazines": ["StandardRifleMag"],
    "flags": ["GUN", "RIFLE", "WEAPON"],
    "slotCapacity": {
        "STOCK": 1,
        "BARREL": 1,
        "UNDER_BARREL": 1,
        "GRIP": 1,
        "OPTIC": 1,
        "RAIL": 4,
        "MUZZLE": 1,
        "MAGAZINE_WELL": 1,
        "SPECIAL": 2
    }
}
```

### 3.2 GunMod 配置示例
```json
{
    "name": "Silencer",
    "weight": 0.3,
    "volume": 0.15,
    "length": 0.2,
    "value": 300,
    "modSoundLevel": -30.0,
    "modFireRate": 0.0,
    "modDamageBonus": -2.0,
    "modAccuracyMOA": -0.3,
    "modRecoil": -5.0,
    "modErgonomics": -10.0,
    "modBreathStability": 0.0,
    "modRange": -5.0,
    "modBulletSpeed": -10.0,
    "modPenetrationBonus": -0.1,
    "flags": ["GUNMOD", "MOD_MUZZLE"],
    "slotType": "MUZZLE"
}
```

### 3.3 Magazine 配置示例
```json
{
    "name": "StandardRifleMag",
    "weight": 0.2,
    "volume": 0.15,
    "length": 0.2,
    "value": 30,
    "capacity": 30,
    "unloadTime": 1.5,
    "reloadTime": 2.0,
    "compatibleAmmoTypes": ["5.56x45mm", ".300 Blackout"],
    "flags": ["MAGAZINE"]
}
```

### 3.4 Ammo 配置示例
```json
{
    "name": "9mm_PST",
    "weight": 0.008,
    "volume": 0.005,
    "length": 0.03,
    "value": 1,
    "damage": 25,
    "ammoType": "9mm",
    "penetration": 15,
    "range": 25.0,
    "speed": 35.0,
    "recoilMod": 0.0,
    "accuracyMod": 0.0,
    "ergoMod": 0.0,
    "stackable": true,
    "maxStackSize": 50,
    "flags": ["AMMO"]
}
```

## 4. 数据流与交互关系

### 4.1 属性计算流程
```
基础属性 (Gun) + 配件修正 (GunMod) + 弹药修正 (Ammo) = 最终属性
```

1. **Gun.updateGunStats()** 计算最终属性
2. 遍历所有配件，累加修正值
3. 考虑当前弹药的修正值
4. 更新最终属性值

### 4.2 射击流程
```
Gun.shoot() -> chamberedRound -> Magazine.consumeAmmo() -> chamberManually()
```

1. 检查枪膛是否有子弹
2. 发射枪膛内子弹
3. 从弹匣自动上膛下一发子弹
4. 返回发射的子弹对象

### 4.3 配件安装流程
```
Gun.attach() -> 检查槽位容量 -> 更新槽位使用 -> 重计算属性
```

1. 验证槽位是否有空间
2. 检查配件类型兼容性 
3. 安装配件到指定槽位
4. 更新槽位使用计数
5. 重新计算枪械最终属性

## 5. ItemLoader 集成

### 5.1 加载流程
1. **loadItemsFromJson()** - 主加载入口
2. **loadGunFromJson()** - 加载Gun配置
3. **loadGunModFromJson()** - 加载GunMod配置  
4. **loadMagazineFromJson()** - 加载Magazine配置
5. **loadAmmoFromJson()** - 加载Ammo配置

### 5.2 实例创建
- **createGun()** - 基于模板创建Gun实例
- **createGunMod()** - 基于模板创建GunMod实例
- **createMagazine()** - 基于模板创建Magazine实例
- **createAmmo()** - 基于模板创建Ammo实例

### 5.3 模板存储
```cpp
std::unordered_map<std::string, std::unique_ptr<Gun>> gunTemplates;
std::unordered_map<std::string, std::unique_ptr<GunMod>> gunModTemplates;
std::unordered_map<std::string, std::unique_ptr<Magazine>> magazineTemplates;
std::unordered_map<std::string, std::unique_ptr<Ammo>> ammoTemplates;
```

## 6. 当前架构的特点

### 6.1 优势
- **模块化设计**: 各组件职责清晰
- **可扩展性**: 支持新的枪械类型和配件
- **数据驱动**: JSON配置易于修改
- **深拷贝支持**: 完整的克隆机制
- **类型安全**: 强类型枚举和类型检查

### 6.2 存在问题
- **配件槽位管理复杂**: 多重映射表维护
- **属性计算分散**: updateGunStats()逻辑复杂
- **弹匣兼容性**: 基于名称字符串匹配，不够灵活
- **内存管理**: 大量 unique_ptr 管理
- **标签系统依赖**: ItemFlag 与业务逻辑耦合

### 6.3 性能考虑
- 配件属性计算在每次安装/拆卸时触发
- 弹匣使用 std::stack，查询性能有限
- 大量动态内存分配
- 深拷贝开销较大

## 7. 重构建议

### 7.1 配件系统重构
- 考虑使用组件式架构
- 简化槽位管理逻辑
- 引入配件预设系统

### 7.2 属性系统重构  
- 统一属性修正计算
- 缓存计算结果
- 支持条件性修正

### 7.3 兼容性系统重构
- 基于类型而非名称的兼容性
- 支持版本化配置
- 更灵活的匹配规则 