//#include "Monster.h"
//#include <cmath>
//#include "Game.h"
//#include "Map.h"
//#include "Gun.h"      // 包含 Gun.h
//#include "Ammo.h"     // 可能需要，如果直接创建 Ammo
//#include "Magazine.h" // 可能需要，如果直接创建 Magazine
//
//Monster::Monster(int startX, int startY, Entity* targetEntity)
//    : Entity(startX, startY, 25, 15, 50, { 0, 255, 0, 255 }, Faction::ENEMY),
//    currentState(MonsterState::IDLE),
//    target(targetEntity),
//    detectionRange(1000.0f),
//    chaseDistance(600.0f),
//    attackRange(300.0f),
//    backpack(std::make_unique<Storage>("怪物背包")), // 初始化背包存储空间
//    itemsDropped(false), // 初始化物品掉落标记
//    colliderRemoved(false) { // 初始化碰撞体积移除标记
//    // 设置背包属性
//    backpack->setMaxWeight(20.0f);
//    backpack->setMaxVolume(15.0f);
//    backpack->setMaxLength(10.0f);
//    backpack->setAccessTime(1.5f);
//    backpack->setMaxItems(-1); // 无物品数量限制
//    // 修正碰撞箱位置，使其中心与实体中心对齐
//    collider = Collider(startX - radius, startY - radius, radius * 2, radius * 2, "monster");
//    
//    // 注意：不再在构造函数中配发枪械和弹匣
//    // 枪械和弹匣将在Game::init之后分发
//}
//
//
//// 实现Storage相关方法
//bool Monster::addItemToBackpack(std::unique_ptr<Item> item) {
//    if (backpack) {
//        return backpack->addItem(std::move(item));
//    }
//    return false;
//}
//
//std::unique_ptr<Item> Monster::removeItemFromBackpack(int index) {
//    if (backpack) {
//        return backpack->removeItem(index);
//    }
//    return nullptr;
//}
//
//// 实现移除碰撞体积的方法
//void Monster::removeCollider() {
//    if (!colliderRemoved) {
//        // 创建一个空的碰撞体积（宽高为0）
//        collider = Collider(x, y, 0, 0, "dead_monster");
//        colliderRemoved = true;
//    }
//}
//
//// 实现装备物品方法，区分手持武器和穿戴物品
//float Monster::equipItem(std::unique_ptr<Item> item) {
//    if (!item || !Entity::canPerformAction()) {
//        return 0.0f;
//    }
//    
//    // 检查是否是枪械或其他武器/工具类物品
//    if (item->hasFlag(ItemFlag::WEAPON) || item->hasFlag(ItemFlag::MISC)) {
//        // 如果是枪械，特殊处理
//        if (item->hasFlag(ItemFlag::GUN)) {
//            // 尝试转换为Gun类型
//            Gun* gunPtr = dynamic_cast<Gun*>(item.get());
//            if (gunPtr) {
//                // 设置手持武器
//                equippedWeapon = std::unique_ptr<Gun>(dynamic_cast<Gun*>(item.release()));
//                
//                // 设置状态为装备中，假设装备枪需要1.5秒
//                float equipTime = 1.5f;
//                // 使用Entity::setState方法设置实体状态
//                Entity::setState(EntityState::EQUIPPING, equipTime);
//                
//                return equipTime;
//            }
//        } else {
//            // 其他武器或工具类物品的处理逻辑可以在这里添加
//            // 目前怪物只支持枪械作为手持物品
//            return 0.0f;
//        }
//    }
//    
//    // 如果是可穿戴物品，则使用Action系统进行装备
//    if (item->isWearable()) {
//        // 使用Action系统进行装备，确保有正确的装备动画和时间
//        equipItemWithAction(std::move(item));
//        return 0.0f; // 返回0因为实际时间由Action系统控制
//    }
//    
//    // 如果既不是武器也不是可穿戴物品，则无法装备
//    return 0.0f;
//}
//
//void Monster::setState(MonsterState newState) {
//    // 如果状态没有改变，直接返回
//    if (currentState == newState) return;
//
//    // 退出当前状态的逻辑
//    switch (currentState) {
//    case MonsterState::IDLE:
//        // 退出静止状态的逻辑
//        break;
//    case MonsterState::CHASE:
//        // 退出追逐状态的逻辑
//        break;
//    case MonsterState::ATTACK:
//        // 退出攻击状态的逻辑
//        break;
//    case MonsterState::DEAD:
//        // 退出死亡状态的逻辑
//        break;
//    }
//
//    // 更新状态
//    currentState = newState;
//
//    // 进入新状态的逻辑
//    switch (currentState) {
//    case MonsterState::IDLE:
//        // 进入静止状态的逻辑
//        break;
//    case MonsterState::CHASE:
//        // 进入追逐状态的逻辑
//        break;
//    case MonsterState::ATTACK:
//        // 进入攻击状态的逻辑
//        break;
//    case MonsterState::DEAD:
//        // 进入死亡状态的逻辑
//        break;
//    }
//}
//
//void Monster::update(float deltaTime) {
//    // 如果怪物已死亡，只处理死亡状态
//    if (health <= 0) {
//        setState(MonsterState::DEAD);
//        handleDeadState();
//        // 即使在死亡状态，也要调用基类update以更新位置历史
//        Entity::update(deltaTime);
//        return;
//    }
//
//    // 如果没有目标，保持静止状态
//    if (!target) {
//        setState(MonsterState::IDLE);
//        handleIdleState();
//        // 即使在静止状态，也要调用基类update以更新位置历史
//        Entity::update(deltaTime);
//        return;
//    }
//
//    // 检查是否有目标
//    // 计算与目标的距离平方（避免开方运算）
//    float distanceSq = distanceToTargetSquared();
//    float detectionRangeSq = detectionRange * detectionRange;
//    float attackRangeSq = attackRange * attackRange;
//    
//    // 检查是否能看到目标
//    bool canSee = canSeeTarget();
//
//    // 根据距离和视线情况决定状态
//    if (distanceSq > detectionRangeSq) {
//        // 超出检测范围，保持静止
//        setState(MonsterState::IDLE);
//    }
//    else if (!canSee) {
//        // 在检测范围内但看不到目标，进入警戒状态（使用追逐状态）
//        setState(MonsterState::CHASE);
//    }
//    else if (distanceSq <= attackRangeSq && canSee) {
//        // 在攻击范围内且能看到目标，进入攻击状态
//        setState(MonsterState::ATTACK);
//    }
//    else {
//        // 在检测范围内且能看到目标，但不在攻击范围内，追逐目标
//        setState(MonsterState::CHASE);
//    }
//
//    // 根据当前状态执行相应的处理
//    switch (currentState) {
//    case MonsterState::IDLE:
//        handleIdleState();
//        break;
//    case MonsterState::CHASE:
//        handleChaseState();
//        break;
//    case MonsterState::ATTACK:
//        handleAttackState();
//        break;
//    case MonsterState::DEAD:
//        handleDeadState();
//        break;
//    }
//    
//    // 在所有状态处理完成后调用基类的update方法
//    // 这确保了Entity::update()中的prevX/prevY保存发生在所有位置修改之后
//    // 从而解决了摄像机移动时的渲染错位问题
//    Entity::update(deltaTime);
//}
//
//void Monster::render(SDL_Renderer* renderer, int cameraX, int cameraY) {
//    // 如果怪物死亡但未被搜刮（背包中还有物品），使用红色渲染
//    if (currentState == MonsterState::DEAD && backpack && backpack->getItemCount() > 0) {
//        // 保存当前颜色
//        SDL_Color originalColor = color;
//        
//        // 设置为红色
//        color = {255, 0, 0, 255};
//        
//        // 调用基类的render方法
//        Entity::render(renderer, cameraX, cameraY);
//        
//        // 恢复原始颜色
//        color = originalColor;
//        
//        // 在怪物上方绘制"未搜刮"指示器（红色）
//        int screenX = x - cameraX;
//        int screenY = y - cameraY;
//        
//        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // 红色
//        
//        // 绘制一个小图标表示可搜刮
//        SDL_FRect lootIndicator = {
//            static_cast<float>(screenX - 10),
//            static_cast<float>(screenY - radius - 15),
//            20.0f,
//            10.0f
//        };
//        SDL_RenderFillRect(renderer, &lootIndicator);
//        
//        // 绘制一个"F"字样提示按F键搜刮
//        // 注意：这里只是简单地用矩形绘制了一个类似F的形状
//        // 实际游戏中可以使用文本渲染或更精美的图标
//        SDL_FRect fVertical = {
//            static_cast<float>(screenX - 5),
//            static_cast<float>(screenY - radius - 30),
//            5.0f,
//            15.0f
//        };
//        SDL_FRect fHorizontalTop = {
//            static_cast<float>(screenX - 5),
//            static_cast<float>(screenY - radius - 30),
//            10.0f,
//            5.0f
//        };
//        SDL_FRect fHorizontalMiddle = {
//            static_cast<float>(screenX - 5),
//            static_cast<float>(screenY - radius - 23),
//            8.0f,
//            5.0f
//        };
//        SDL_RenderFillRect(renderer, &fVertical);
//        SDL_RenderFillRect(renderer, &fHorizontalTop);
//        SDL_RenderFillRect(renderer, &fHorizontalMiddle);
//    } else {
//        // 正常渲染
//        // 调用基类的render方法
//        Entity::render(renderer, cameraX, cameraY);
//
//        // 根据状态添加额外的渲染效果
//        int screenX = x - cameraX;
//        int screenY = y - cameraY;
//
//        // 根据状态设置不同的颜色
//        switch (currentState) {
//        case MonsterState::IDLE:
//            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // 绿色
//            break;
//        case MonsterState::CHASE:
//            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // 黄色
//            break;
//        case MonsterState::ATTACK:
//            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // 红色
//            break;
//        case MonsterState::DEAD:
//            SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255); // 灰色
//            break;
//        }
//
//        // 在怪物上方绘制状态指示器
//        SDL_FRect stateIndicator = {
//            static_cast<float>(screenX - 5),
//            static_cast<float>(screenY - radius - 10),
//            10.0f,
//            5.0f
//        };
//        SDL_RenderFillRect(renderer, &stateIndicator);
//    }
//}
//
//void Monster::handleIdleState() {
//    // 静止状态下的行为
//    // 什么都不做，保持静止
//}
//
//void Monster::handleChaseState() {
//    if (!target) return;
//
//    // 检查是否能看到目标
//    bool canSee = canSeeTarget();
//    
//    // 计算基本追逐方向
//    float dx = target->getX() - x;
//    float dy = target->getY() - y;
//    float distanceSq = dx * dx + dy * dy;
//    float stopDistanceSq = 50 * 50;
//
//    // 如果看不到目标，继续靠近最后已知位置
//    // 如果能看到目标且距离足够近，停止移动
//    if (canSee && distanceSq <= stopDistanceSq) {
//        return;
//    }
//
//    // 标准化追逐方向
//    float distance = std::sqrt(distanceSq);
//    if (distance > 0) {
//        dx /= distance;
//        dy /= distance;
//    }
//
//    // 添加避障力（避开其他怪物）
//    float avoidanceX = 0, avoidanceY = 0;
//    Game* game = Game::getInstance();
//    const auto& monsters = game->getMonsters();
//    
//    for (const auto& other : monsters) {
//        if (other.get() != this) {
//            float otherDx = x - other->getX();
//            float otherDy = y - other->getY();
//            float otherDistSq = otherDx * otherDx + otherDy * otherDy;
//            
//            // 如果距离太近，添加排斥力
//            float avoidanceRange = 80.0f; // 避障范围
//            if (otherDistSq < avoidanceRange * avoidanceRange && otherDistSq > 0) {
//                float otherDist = std::sqrt(otherDistSq);
//                float avoidanceStrength = (avoidanceRange - otherDist) / avoidanceRange;
//                avoidanceX += (otherDx / otherDist) * avoidanceStrength;
//                avoidanceY += (otherDy / otherDist) * avoidanceStrength;
//            }
//        }
//    }
//
//    // 组合追逐力和避障力
//    float finalDx = dx + avoidanceX * 2.5f; // 避障力权重
//    float finalDy = dy + avoidanceY * 2.5f;
//    
//    // 重新标准化
//    float finalLength = std::sqrt(finalDx * finalDx + finalDy * finalDy);
//    if (finalLength > 0) {
//        finalDx /= finalLength;
//        finalDy /= finalLength;
//    }
//
//    // 移动 - 基于deltaTime并受timeScale影响
//    // 使用60作为基准帧率，确保在timeScale=1.0时速度不变
//    float adjustedDeltaTime = Game::getInstance()->getAdjustedDeltaTime();
//    x += static_cast<int>(finalDx * speed * adjustedDeltaTime * 60);
//    y += static_cast<int>(finalDy * speed * adjustedDeltaTime * 60);
//
//    // 注意：不需要在这里更新碰撞体位置
//    // 碰撞体位置更新会在Entity::update()中统一处理
//}
//
//void Monster::handleAttackState() {
//    Gun* weapon = equippedWeapon.get();
//    if (weapon && target) {
//        // 使用Entity的武器维护方法
//        maintainWeapon(weapon);
//        
//        // 使用Entity的射击能力
//        if (canShoot(weapon)) {
//            // 计算射击方向
//            float dirX = target->getX() - x;
//            float dirY = target->getY() - y;
//            float length = std::sqrt(dirX * dirX + dirY * dirY);
//            
//            if (length > 0) {
//                dirX /= length;
//                dirY /= length;
//                shootInDirection(weapon, dirX, dirY);
//            }
//        }
//    }
//}
//
//void Monster::handleDeadState() {
//    // 死亡状态下的行为
//    // 可以添加死亡动画或粒子效果
//    
//    // 注释掉自动掉落物品的逻辑，改为让玩家通过按F键搜刮怪物背包
//    // 物品掉落逻辑将在玩家搜刮背包时处理
//    // if (!itemsDropped && backpack) {
//    //     // 获取背包中的所有物品
//    //     size_t itemCount = backpack->getItemCount();
//    //     
//    //     // 获取游戏实例
//    //     Game* game = Game::getInstance();
//    //     
//    //     // 掉落所有物品
//    //     for (int i = itemCount - 1; i >= 0; i--) {
//    //         // 从背包中移除物品
//    //         auto item = backpack->removeItem(i);
//    //         
//    //         if (item) {
//    //             // 在怪物位置创建掉落物
//    //             game->createItemDrop(std::move(item), x, y);
//    //         }
//    //     }
//    //     
//    //     itemsDropped = true; // 标记物品已掉落
//    // }
//    
//    // 移除碰撞体积
//    if (!colliderRemoved) {
//        removeCollider();
//    }
//}
//
//// 添加一个新方法计算距离平方，避免开方运算
//float Monster::distanceToTargetSquared() const {
//    if (!target) return FLT_MAX;
//
//    float dx = target->getX() - x;
//    float dy = target->getY() - y;
//    return dx * dx + dy * dy;
//}
//
//// 保留原来的方法以兼容其他代码
//float Monster::distanceToTarget() const {
//    return std::sqrt(distanceToTargetSquared());
//}
//
//// 在文件末尾添加以下方法
//
//// 检查是否能看到目标
//bool Monster::canSeeTarget() const {
//    if (!target) return false;
//    
//    // 获取游戏实例和障碍物列表
//    Game* game = Game::getInstance();
//    const auto& obstacles = game->getMap()->getObstacles();
//    
//    // 使用射线检测判断是否有障碍物阻挡视线
//    return !raycast(x, y, target->getX(), target->getY(), obstacles);
//}
//
//// 射线检测方法
//bool Monster::raycast(float startX, float startY, float endX, float endY, const std::vector<Collider>& obstacles) const {
//    // 计算射线方向和长度
//    float dirX = endX - startX;
//    float dirY = endY - startY;
//    float length = std::sqrt(dirX * dirX + dirY * dirY);
//    
//    // 标准化方向向量
//    if (length > 0) {
//        dirX /= length;
//        dirY /= length;
//    }
//    
//    // 射线检测步长
//    float stepSize = 10.0f;
//    float currentDist = 0.0f;
//    
//    // 沿射线方向检测
//    while (currentDist < length) {
//        float currentX = startX + dirX * currentDist;
//        float currentY = startY + dirY * currentDist;
//        
//        // 检查当前点是否与任何障碍物相交
//        for (const auto& obstacle : obstacles) {
//            if (obstacle.getTag() == "obstacle") {
//                // 创建一个临时的点碰撞体用于检测
//                Collider pointCollider(currentX, currentY, 1.0f, "raycast");
//                
//                if (pointCollider.checkCollision(obstacle)) {
//                    return true; // 射线被障碍物阻挡
//                }
//            }
//        }
//        
//        // 前一步
//        currentDist += stepSize;
//    }
//    
//    return false; // 射线没有被阻挡
//}
//
//// 物品转移方法实现
//bool Monster::transferItem(Item* item, Storage* sourceStorage, Storage* targetStorage, std::function<void(bool)> callback) {
//    // 检查参数有效性
//    if (!item || !sourceStorage || !targetStorage) {
//        if (callback) {
//            callback(false);
//        }
//        return false;
//    }
//    
//    // 检查物品是否在源存储空间中
//    bool itemFound = false;
//    for (size_t i = 0; i < sourceStorage->getItemCount(); ++i) {
//        if (sourceStorage->getItem(i) == item) {
//            itemFound = true;
//            break;
//        }
//    }
//    
//    if (!itemFound) {
//        if (callback) {
//            callback(false);
//        }
//        return false;
//    }
//    
//    // 检查目标存储空间是否有足够空间
//    if (!targetStorage->canFitItem(item)) {
//        if (callback) {
//            callback(false);
//        }
//        return false;
//    }
//    
//    // 将物品转移操作添加到行为队列中
//    transferItemWithAction(item, sourceStorage, targetStorage, callback);
//    return true; // 如果执行到这里，说明参数检查都通过了，返回true表示操作已添加到队列
//}