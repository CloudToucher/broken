#include "Weapon.h"
#include "Entity.h"
#include <iostream>

// 自定义特殊效果示例
void registerCustomWeaponEffects() {
    auto* effectManager = SpecialEffectManager::getInstance();
    
    // 注册"生命窃取"效果
    effectManager->registerCustomEffect("LIFE_STEAL", 
        [](Entity* attacker, Entity* target, const SpecialEffect& effect) {
            if (attacker && target) {
                float healAmount = effect.magnitude;
                std::cout << "生命窃取效果触发: 攻击者回复 " << healAmount << " 生命值" << std::endl;
                // attacker->heal(healAmount);
            }
        });
    
    // 注册"连锁闪电"效果
    effectManager->registerCustomEffect("CHAIN_LIGHTNING", 
        [](Entity* attacker, Entity* target, const SpecialEffect& effect) {
            if (target) {
                float damage = effect.magnitude;
                float range = effect.parameters.count("range") ? effect.parameters.at("range") : 100.0f;
                int maxTargets = effect.parameters.count("maxTargets") ? static_cast<int>(effect.parameters.at("maxTargets")) : 3;
                
                std::cout << "连锁闪电效果触发: 伤害=" << damage 
                         << ", 范围=" << range 
                         << ", 最大目标数=" << maxTargets << std::endl;
                
                // 这里应该实现寻找范围内的敌人并对它们造成伤害的逻辑
                // findNearbyEnemies(target->getX(), target->getY(), range, maxTargets);
            }
        });
    
    // 注册"时间减缓"效果
    effectManager->registerCustomEffect("TIME_SLOW", 
        [](Entity* attacker, Entity* target, const SpecialEffect& effect) {
            if (target) {
                float slowFactor = effect.magnitude; // 0.5 = 50%速度
                float duration = effect.duration;
                
                std::cout << "时间减缓效果触发: 减速至" << (slowFactor * 100) << "%, 持续" << duration << "秒" << std::endl;
                // target->addStatusEffect(StatusEffectType::SLOWED, duration, slowFactor);
            }
        });
    
    // 注册"武器充能"效果
    effectManager->registerCustomEffect("WEAPON_CHARGE", 
        [](Entity* attacker, Entity* target, const SpecialEffect& effect) {
            if (attacker) {
                float damageBonus = effect.magnitude;
                float duration = effect.duration;
                
                std::cout << "武器充能效果触发: 伤害提升" << (damageBonus * 100) << "%, 持续" << duration << "秒" << std::endl;
                // attacker->addStatusEffect(StatusEffectType::DAMAGE_BOOST, duration, damageBonus);
            }
        });
    
    std::cout << "自定义武器特殊效果已注册完成!" << std::endl;
} 