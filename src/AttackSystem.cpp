#include "AttackSystem.h"
#include "Entity.h"
#include "Game.h"
#include "SoundManager.h"
#include <random>
#include <cmath>
#include <algorithm>
#include <iostream>

// 数学常量
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 构造函数
AttackSystem::AttackSystem(Entity* attacker) 
    : owner(attacker), currentCooldown(0) {
}

// 执行攻击
AttackResult AttackSystem::executeAttack(AttackMethod method, const AttackParams& params) {
    AttackResult result;
    
    // 检查是否可以攻击
    if (!this->canAttack()) {
        return result;
    }
    
    // 设置冷却时间
    currentCooldown = params.cooldownMs;
    
    // 根据攻击方式执行不同的攻击
    switch (method) {
        case AttackMethod::MELEE_SLASH:
        case AttackMethod::MELEE_STAB:
        case AttackMethod::MELEE_CRUSH:
        case AttackMethod::MELEE_QUICK:
            result = performMeleeAttack(method, params);
            break;
            
        case AttackMethod::RANGED_SHOOT:
        case AttackMethod::RANGED_THROW:
            result = performRangedAttack(method, params);
            break;
            
        case AttackMethod::SPECIAL_ABILITY:
            result = performSpecialAttack(method, params);
            break;
    }
    
    // 播放攻击音效
    if (!params.soundFile.empty()) {
        // SoundManager::getInstance().playSound(params.soundFile);
    }
    
    // 调用攻击完成回调
    if (onAttackComplete) {
        onAttackComplete(result);
    }
    
    return result;
}

// 执行攻击（使用武器接口）
AttackResult AttackSystem::executeAttack(WeaponAttackType type) {
    AttackResult result;
    
    // 检查是否可以攻击
    if (!this->canAttack()) {
        return result;
    }
    
    // 这里应该从owner获取武器，而不是将owner转换为武器
    // 暂时返回默认的攻击结果，具体实现需要根据游戏逻辑调整
    // 例如：从owner的装备中获取武器，然后调用武器的攻击方法
    
    // 临时实现：使用默认的近战攻击
    AttackParams params;
    params.baseDamage = 25;
    params.range = 80.0f;
    params.shape = AttackShape::SECTOR;
    params.angle = 1.047f; // 60度
    params.width = 50.0f;
    
    if (type == WeaponAttackType::SECONDARY) {
        // 右键攻击：长条形
        params.shape = AttackShape::RECTANGLE;
        params.range = 100.0f;
        params.width = 30.0f;
        params.baseDamage = 30;
    }
    
    // 执行攻击
    result = this->executeAttack(AttackMethod::MELEE_SLASH, params);
    
    return result;
}

// 更新冷却时间
void AttackSystem::updateCooldown(int deltaTimeMs) {
    if (currentCooldown > 0) {
        currentCooldown = std::max(0, currentCooldown - deltaTimeMs);
    }
}

// 检查是否可以攻击
bool AttackSystem::canAttack() const {
    return currentCooldown <= 0 && owner && owner->canPerformAction();
}

// 执行近战攻击
AttackResult AttackSystem::performMeleeAttack(AttackMethod method, const AttackParams& params) {
    AttackResult result;
    
    // 使用形状检测寻找目标
    Entity* target = findTargetInShape(params);
    if (!target) {
        return result; // 没有找到目标
    }
    
    result.target = target;
    result.hit = true;
    
    // 计算伤害
    bool isCritical = false;
    Damage damage = calculateDamage(params, isCritical);
    result.critical = isCritical;
    
    // 计算总伤害值（用于显示）
    result.totalDamage = 0;
    for (const auto& damageInfo : damage.getDamageList()) {
        result.totalDamage += std::get<1>(damageInfo);
    }
    
    // 造成伤害
    target->takeDamage(damage);
    
    // 应用特殊效果
    applyEffects(target, params, result);
    
    std::cout << "近战攻击命中！造成 " << result.totalDamage << " 点伤害";
    if (result.critical) {
        std::cout << " (暴击!)";
    }
    std::cout << std::endl;
    
    return result;
}

// 执行远程攻击
AttackResult AttackSystem::performRangedAttack(AttackMethod method, const AttackParams& params) {
    AttackResult result;
    
    // 远程攻击的实现（这里简化处理）
    Entity* target = findTarget(params.range);
    if (!target) {
        return result;
    }
    
    result.target = target;
    result.hit = true;
    
    // 计算伤害
    bool isCritical = false;
    Damage damage = calculateDamage(params, isCritical);
    result.critical = isCritical;
    
    // 计算总伤害值
    result.totalDamage = 0;
    for (const auto& damageInfo : damage.getDamageList()) {
        result.totalDamage += std::get<1>(damageInfo);
    }
    
    // 造成伤害
    target->takeDamage(damage);
    
    std::cout << "远程攻击命中！造成 " << result.totalDamage << " 点伤害";
    if (result.critical) {
        std::cout << " (暴击!)";
    }
    std::cout << std::endl;
    
    return result;
}

// 执行特殊攻击
AttackResult AttackSystem::performSpecialAttack(AttackMethod method, const AttackParams& params) {
    AttackResult result;
    
    // 特殊攻击的实现（根据具体需求定制）
    Entity* target = findTarget(params.range);
    if (!target) {
        return result;
    }
    
    result.target = target;
    result.hit = true;
    
    // 特殊攻击通常有特殊的伤害计算
    bool isCritical = false;
    Damage damage = calculateDamage(params, isCritical);
    result.critical = isCritical;
    
    // 计算总伤害值
    result.totalDamage = 0;
    for (const auto& damageInfo : damage.getDamageList()) {
        result.totalDamage += std::get<1>(damageInfo);
    }
    
    // 造成伤害
    target->takeDamage(damage);
    
    // 应用特殊效果
    applyEffects(target, params, result);
    
    std::cout << "特殊攻击命中！造成 " << result.totalDamage << " 点伤害";
    if (result.critical) {
        std::cout << " (暴击!)";
    }
    std::cout << std::endl;
    
    return result;
}

// 计算伤害
Damage AttackSystem::calculateDamage(const AttackParams& params, bool& isCritical) {
    // 基础伤害 + 攻击者属性加成
    int finalDamage = params.baseDamage;
    if (owner) {
        finalDamage += owner->getStrength() / 2; // 力量加成
    }
    
    // 检查暴击
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    
    isCritical = dis(gen) < params.criticalChance;
    if (isCritical) {
        finalDamage = static_cast<int>(finalDamage * params.criticalMultiplier);
    }
    
    // 创建伤害对象
    Damage damage;
    damage.addDamage(params.damageType, finalDamage, params.armorPenetration);
    
    return damage;
}

// 应用特殊效果
void AttackSystem::applyEffects(Entity* target, const AttackParams& params, AttackResult& result) {
    if (!target) return;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    
    // 流血效果
    if (params.canBleed && dis(gen) < params.bleedChance) {
        target->addState(EntityStateEffect::Type::DEBUFFED, "Bleeding", 10000, 1);
        result.causedBleeding = true;
        std::cout << "造成流血效果！" << std::endl;
    }
    
    // 眩晕效果
    if (params.canStun && dis(gen) < params.stunChance) {
        target->addState(EntityStateEffect::Type::STUNNED, "Stunned", params.stunDuration, 3);
        result.causedStun = true;
        std::cout << "造成眩晕效果！" << std::endl;
    }
}

// 寻找攻击目标
Entity* AttackSystem::findTarget(float range) {
    // 如果设置了自定义寻找目标函数，使用它
    if (findTargetFunction) {
        return findTargetFunction(range);
    }
    
    // 默认寻找目标逻辑
    if (!owner) return nullptr;
    
    Game* game = Game::getInstance();
    if (!game) return nullptr;
    
    Entity* closestTarget = nullptr;
    float closestDistance = range + 1;
    
    // 检查所有僵尸
    for (const auto& zombie : game->getZombies()) {
        if (!zombie || zombie->getHealth() <= 0) {
            continue;
        }
        
        float dx = zombie->getX() - owner->getX();
        float dy = zombie->getY() - owner->getY();
        float distance = std::sqrt(dx * dx + dy * dy);
        
        if (distance <= range && distance < closestDistance) {
            closestDistance = distance;
            closestTarget = zombie.get();
        }
    }
    
    // 检查其他生物
    for (const auto& creature : game->getCreatures()) {
        if (!creature || creature.get() == owner || creature->getHealth() <= 0) {
            continue;
        }
        
        // 检查是否是敌对阵营
        if (creature->getFaction() == Faction::ENEMY || creature->getFaction() == Faction::HOSTILE) {
            float dx = creature->getX() - owner->getX();
            float dy = creature->getY() - owner->getY();
            float distance = std::sqrt(dx * dx + dy * dy);
            
            if (distance <= range && distance < closestDistance) {
                closestDistance = distance;
                closestTarget = creature.get();
            }
        }
    }
    
    return closestTarget;
}

// 获取攻击范围内的所有目标（用于可视化）
std::vector<Entity*> AttackSystem::getTargetsInRange(const AttackParams& params) const {
    std::vector<Entity*> targets;
    
    if (!owner) return targets;
    
    Game* game = Game::getInstance();
    if (!game) return targets;
    
    // 检查所有僵尸
    for (const auto& zombie : game->getZombies()) {
        if (!zombie || zombie->getHealth() <= 0) {
            continue;
        }
        
        bool inRange = false;
        switch (params.shape) {
            case AttackShape::CIRCLE:
                inRange = isTargetInCircle(zombie.get(), params.range);
                break;
            case AttackShape::SECTOR:
                inRange = isTargetInSector(zombie.get(), params);
                break;
            case AttackShape::RECTANGLE:
                inRange = isTargetInRectangle(zombie.get(), params);
                break;
            case AttackShape::LINE:
                inRange = isTargetInLine(zombie.get(), params);
                break;
        }
        
        if (inRange) {
            targets.push_back(zombie.get());
        }
    }
    
    // 检查其他生物
    for (const auto& creature : game->getCreatures()) {
        if (!creature || creature.get() == owner || creature->getHealth() <= 0) {
            continue;
        }
        
        // 检查是否是敌对阵营
        if (creature->getFaction() == Faction::ENEMY || creature->getFaction() == Faction::HOSTILE) {
            bool inRange = false;
            switch (params.shape) {
                case AttackShape::CIRCLE:
                    inRange = isTargetInCircle(creature.get(), params.range);
                    break;
                case AttackShape::SECTOR:
                    inRange = isTargetInSector(creature.get(), params);
                    break;
                case AttackShape::RECTANGLE:
                    inRange = isTargetInRectangle(creature.get(), params);
                    break;
                case AttackShape::LINE:
                    inRange = isTargetInLine(creature.get(), params);
                    break;
            }
            
            if (inRange) {
                targets.push_back(creature.get());
            }
        }
    }
    
    return targets;
}

// 渲染攻击范围
void AttackSystem::renderAttackRange(SDL_Renderer* renderer, const AttackParams& params, float cameraX, float cameraY) const {
    if (!owner || !renderer) return;
    
    // 设置渲染颜色（半透明红色）
    SDL_SetRenderDrawColor(renderer, 255, 100, 100, 128);
    
    float ownerX = owner->getX() - cameraX;
    float ownerY = owner->getY() - cameraY;
    
    switch (params.shape) {
        case AttackShape::CIRCLE:
            renderCircleRange(renderer, ownerX, ownerY, params.range);
            break;
        case AttackShape::SECTOR:
            renderSectorRange(renderer, ownerX, ownerY, params);
            break;
        case AttackShape::RECTANGLE:
            renderRectangleRange(renderer, ownerX, ownerY, params);
            break;
        case AttackShape::LINE:
            renderLineRange(renderer, ownerX, ownerY, params);
            break;
    }
}

// 形状检测方法
Entity* AttackSystem::findTargetInShape(const AttackParams& params) const {
    if (!owner) return nullptr;
    
    Game* game = Game::getInstance();
    if (!game) return nullptr;
    
    Entity* closestTarget = nullptr;
    float closestDistance = params.range + 1;
    
    // 检查所有僵尸
    for (const auto& zombie : game->getZombies()) {
        if (!zombie || zombie->getHealth() <= 0) {
            continue;
        }
        
        bool inRange = false;
        switch (params.shape) {
            case AttackShape::CIRCLE:
                inRange = isTargetInCircle(zombie.get(), params.range);
                break;
            case AttackShape::SECTOR:
                inRange = isTargetInSector(zombie.get(), params);
                break;
            case AttackShape::RECTANGLE:
                inRange = isTargetInRectangle(zombie.get(), params);
                break;
            case AttackShape::LINE:
                inRange = isTargetInLine(zombie.get(), params);
                break;
        }
        
        if (inRange) {
            float distance = getDistanceToTarget(zombie.get());
            if (distance < closestDistance) {
                closestDistance = distance;
                closestTarget = zombie.get();
            }
        }
    }
    
    // 检查其他生物
    for (const auto& creature : game->getCreatures()) {
        if (!creature || creature.get() == owner || creature->getHealth() <= 0) {
            continue;
        }
        
        // 检查是否是敌对阵营
        if (creature->getFaction() == Faction::ENEMY || creature->getFaction() == Faction::HOSTILE) {
            bool inRange = false;
            switch (params.shape) {
                case AttackShape::CIRCLE:
                    inRange = isTargetInCircle(creature.get(), params.range);
                    break;
                case AttackShape::SECTOR:
                    inRange = isTargetInSector(creature.get(), params);
                    break;
                case AttackShape::RECTANGLE:
                    inRange = isTargetInRectangle(creature.get(), params);
                    break;
                case AttackShape::LINE:
                    inRange = isTargetInLine(creature.get(), params);
                    break;
            }
            
            if (inRange) {
                float distance = getDistanceToTarget(creature.get());
                if (distance < closestDistance) {
                    closestDistance = distance;
                    closestTarget = creature.get();
                }
            }
        }
    }
    
    return closestTarget;
}

bool AttackSystem::isTargetInCircle(Entity* target, float range) const {
    if (!target || !owner) return false;
    
    float distance = getDistanceToTarget(target);
    return distance <= range;
}

bool AttackSystem::isTargetInSector(Entity* target, const AttackParams& params) const {
    if (!target || !owner) return false;
    
    float distance = getDistanceToTarget(target);
    if (distance > params.range) return false;
    
    float angleToTarget = getAngleToTarget(target);
    float attackDirection = params.direction; // 攻击方向（相对于世界坐标）
    
    // 计算目标相对于攻击方向的角度差
    float angleDiff = std::abs(normalizeAngle(angleToTarget - attackDirection));
    
    // 检查是否在扇形角度范围内
    return angleDiff <= params.angle / 2.0f;
}

bool AttackSystem::isTargetInRectangle(Entity* target, const AttackParams& params) const {
    if (!target || !owner) return false;
    
    float dx = target->getX() - owner->getX();
    float dy = target->getY() - owner->getY();
    
    // 将目标坐标转换到以攻击方向为X轴的坐标系
    float cos_dir = std::cos(params.direction);
    float sin_dir = std::sin(params.direction);
    
    float local_x = dx * cos_dir + dy * sin_dir;
    float local_y = -dx * sin_dir + dy * cos_dir;
    
    // 检查是否在长条形范围内
    return (local_x >= 0 && local_x <= params.range && 
            std::abs(local_y) <= params.width / 2.0f);
}

bool AttackSystem::isTargetInLine(Entity* target, const AttackParams& params) const {
    if (!target || !owner) return false;
    
    float dx = target->getX() - owner->getX();
    float dy = target->getY() - owner->getY();
    
    // 计算目标到攻击线的距离
    float cos_dir = std::cos(params.direction);
    float sin_dir = std::sin(params.direction);
    
    float local_x = dx * cos_dir + dy * sin_dir;
    float local_y = -dx * sin_dir + dy * cos_dir;
    
    // 检查是否在直线范围内（很窄的长条）
    return (local_x >= 0 && local_x <= params.range && 
            std::abs(local_y) <= 10.0f); // 线宽度固定为20像素
}

// 辅助方法
float AttackSystem::getAngleToTarget(Entity* target) const {
    if (!target || !owner) return 0.0f;
    
    float dx = target->getX() - owner->getX();
    float dy = target->getY() - owner->getY();
    
    return std::atan2(dy, dx);
}

float AttackSystem::normalizeAngle(float angle) const {
    while (angle > M_PI) angle -= 2 * M_PI;
    while (angle < -M_PI) angle += 2 * M_PI;
    return angle;
}

float AttackSystem::getDistanceToTarget(Entity* target) const {
    if (!target || !owner) return 0.0f;
    
    float dx = target->getX() - owner->getX();
    float dy = target->getY() - owner->getY();
    
    return std::sqrt(dx * dx + dy * dy);
}

// 渲染方法（简化实现）
void AttackSystem::renderCircleRange(SDL_Renderer* renderer, float x, float y, float radius) const {
    // 简化的圆形渲染 - 画一个圆的轮廓
    int segments = 32;
    for (int i = 0; i < segments; i++) {
        float angle1 = (2.0f * M_PI * i) / segments;
        float angle2 = (2.0f * M_PI * (i + 1)) / segments;
        
        int x1 = static_cast<int>(x + radius * std::cos(angle1));
        int y1 = static_cast<int>(y + radius * std::sin(angle1));
        int x2 = static_cast<int>(x + radius * std::cos(angle2));
        int y2 = static_cast<int>(y + radius * std::sin(angle2));
        
        SDL_RenderLine(renderer, x1, y1, x2, y2);
    }
}

void AttackSystem::renderSectorRange(SDL_Renderer* renderer, float x, float y, const AttackParams& params) const {
    // 扇形渲染
    int segments = 16;
    float startAngle = params.direction - params.angle / 2.0f;
    float endAngle = params.direction + params.angle / 2.0f;
    
    // 画扇形边界
    for (int i = 0; i < segments; i++) {
        float angle1 = startAngle + (endAngle - startAngle) * i / segments;
        float angle2 = startAngle + (endAngle - startAngle) * (i + 1) / segments;
        
        int x1 = static_cast<int>(x + params.range * std::cos(angle1));
        int y1 = static_cast<int>(y + params.range * std::sin(angle1));
        int x2 = static_cast<int>(x + params.range * std::cos(angle2));
        int y2 = static_cast<int>(y + params.range * std::sin(angle2));
        
        SDL_RenderLine(renderer, x1, y1, x2, y2);
    }
    
    // 画扇形的两条边
    int startX = static_cast<int>(x + params.range * std::cos(startAngle));
    int startY = static_cast<int>(y + params.range * std::sin(startAngle));
    int endX = static_cast<int>(x + params.range * std::cos(endAngle));
    int endY = static_cast<int>(y + params.range * std::sin(endAngle));
    
    SDL_RenderLine(renderer, static_cast<int>(x), static_cast<int>(y), startX, startY);
    SDL_RenderLine(renderer, static_cast<int>(x), static_cast<int>(y), endX, endY);
}

void AttackSystem::renderRectangleRange(SDL_Renderer* renderer, float x, float y, const AttackParams& params) const {
    // 长条形渲染
    float cos_dir = std::cos(params.direction);
    float sin_dir = std::sin(params.direction);
    
    // 计算长条形的四个角点
    float half_width = params.width / 2.0f;
    
    // 前方两个角点
    float front_x = x + params.range * cos_dir;
    float front_y = y + params.range * sin_dir;
    
    int x1 = static_cast<int>(x - half_width * sin_dir);
    int y1 = static_cast<int>(y + half_width * cos_dir);
    int x2 = static_cast<int>(x + half_width * sin_dir);
    int y2 = static_cast<int>(y - half_width * cos_dir);
    int x3 = static_cast<int>(front_x + half_width * sin_dir);
    int y3 = static_cast<int>(front_y - half_width * cos_dir);
    int x4 = static_cast<int>(front_x - half_width * sin_dir);
    int y4 = static_cast<int>(front_y + half_width * cos_dir);
    
    // 画长条形的轮廓
    SDL_RenderLine(renderer, x1, y1, x2, y2);
    SDL_RenderLine(renderer, x2, y2, x3, y3);
    SDL_RenderLine(renderer, x3, y3, x4, y4);
    SDL_RenderLine(renderer, x4, y4, x1, y1);
}

void AttackSystem::renderLineRange(SDL_Renderer* renderer, float x, float y, const AttackParams& params) const {
    // 直线渲染
    int endX = static_cast<int>(x + params.range * std::cos(params.direction));
    int endY = static_cast<int>(y + params.range * std::sin(params.direction));
    
    SDL_RenderLine(renderer, static_cast<int>(x), static_cast<int>(y), endX, endY);
} 