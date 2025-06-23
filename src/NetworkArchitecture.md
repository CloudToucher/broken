# 游戏网络架构设计

## 概述

本文档描述了游戏的网络架构设计，用于支持多人联机模式。该架构采用了 MVC（模型-视图-控制器）模式的思想，将玩家实体（模型）与控制逻辑（控制器）分离。

## 核心组件

### 1. Player（模型）

`Player` 类代表游戏中的玩家实体，负责：
- 存储玩家状态（位置、健康值等）
- 提供操作接口（移动、射击等）
- 渲染玩家角色

### 2. PlayerState（数据）

`PlayerState` 结构体用于网络同步，包含：
- 位置和方向
- 健康状态
- 动作状态（射击、换弹等）
- 装备状态
- 网络标识（玩家ID、名称）

### 3. PlayerController（控制器）

`PlayerController` 类负责处理输入并控制 `Player` 实体：
- 处理键盘和鼠标输入
- 将输入转换为玩家动作
- 在本地和远程模式下工作
- 管理状态同步

## 工作流程

### 本地玩家

1. `PlayerController` 收集输入（键盘、鼠标）
2. 输入被应用到 `Player` 实体
3. `Player` 实体更新状态
4. `PlayerState` 从 `Player` 更新
5. `PlayerState` 通过网络发送给其他玩家

### 远程玩家

1. 接收远程 `PlayerState`
2. 更新本地 `PlayerController` 的状态
3. `PlayerController` 将状态应用到 `Player` 实体
4. `Player` 实体更新和渲染

## 使用方法

### 创建本地玩家

```cpp
// 创建玩家实体
auto player = std::make_unique<Player>(startX, startY);
player->setIsLocalPlayer(true);
player->setPlayerId(1);
player->setPlayerName("LocalPlayer");

// 创建控制器
auto controller = std::make_unique<PlayerController>(player.get(), true);
player->setController(controller.get());

// 在游戏循环中更新控制器
controller->update(deltaTime);
```

### 创建远程玩家

```cpp
// 创建玩家实体
auto remotePlayer = std::make_unique<Player>(0, 0);
remotePlayer->setIsLocalPlayer(false);
remotePlayer->setPlayerId(remoteId);
remotePlayer->setPlayerName(remoteName);

// 创建控制器
auto remoteController = std::make_unique<PlayerController>(remotePlayer.get(), false);
remotePlayer->setController(remoteController.get());

// 当接收到网络更新时
PlayerState remoteState = receiveFromNetwork();
remoteController->setState(remoteState);
```

## 后续步骤

1. 实现网络通信层
2. 添加状态插值和预测
3. 处理延迟和断线重连
4. 实现权威服务器模型 