#pragma once
#include <string>
#include <vector>
#include <memory>
#include <random>
#include <map>

class ItemSpawnCluster {
public:
    struct SpawnEntry {
        std::string itemName;
        float weight;
    };

    struct NestedCluster {
        std::shared_ptr<ItemSpawnCluster> cluster;
        float weight;
    };

    ItemSpawnCluster() = default;
    ~ItemSpawnCluster() = default;

    // 添加单个物品
    void addItem(const std::string& itemName, float weight);

    // 添加嵌套集群
    void addNestedCluster(std::shared_ptr<ItemSpawnCluster> cluster, float weight);

    // 设置数量范围
    void setQuantityRange(int min, int max);

    // 生成物品
    std::vector<std::string> generateItems() const;

private:
    std::vector<SpawnEntry> items;
    std::vector<NestedCluster> nestedClusters;
    int minQuantity = 1;
    int maxQuantity = 1;

    // 随机数生成器
    mutable std::random_device rd;
    mutable std::mt19937 gen{rd()};

    // 根据权重选择物品
    std::string selectRandomItem() const;
}; 