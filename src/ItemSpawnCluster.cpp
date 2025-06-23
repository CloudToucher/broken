#include "ItemSpawnCluster.h"
#include <algorithm>

void ItemSpawnCluster::addItem(const std::string& itemName, float weight) {
    items.push_back({itemName, weight});
}

void ItemSpawnCluster::addNestedCluster(std::shared_ptr<ItemSpawnCluster> cluster, float weight) {
    nestedClusters.push_back({cluster, weight});
}

void ItemSpawnCluster::setQuantityRange(int min, int max) {
    minQuantity = min;
    maxQuantity = max;
}

std::string ItemSpawnCluster::selectRandomItem() const {
    // 计算总权重
    float totalWeight = 0.0f;
    for (const auto& item : items) {
        totalWeight += item.weight;
    }
    for (const auto& cluster : nestedClusters) {
        totalWeight += cluster.weight;
    }

    // 生成随机数
    std::uniform_real_distribution<float> dist(0.0f, totalWeight);
    float randomValue = dist(gen);

    // 选择物品或集群
    float currentWeight = 0.0f;
    
    // 首先检查普通物品
    for (const auto& item : items) {
        currentWeight += item.weight;
        if (randomValue <= currentWeight) {
            return item.itemName;
        }
    }

    // 然后检查嵌套集群
    for (const auto& cluster : nestedClusters) {
        currentWeight += cluster.weight;
        if (randomValue <= currentWeight) {
            return cluster.cluster->selectRandomItem();
        }
    }

    // 如果出现问题，返回第一个物品
    return items.empty() ? "" : items[0].itemName;
}

std::vector<std::string> ItemSpawnCluster::generateItems() const {
    std::vector<std::string> result;
    
    // 生成随机数量
    std::uniform_int_distribution<int> quantityDist(minQuantity, maxQuantity);
    int quantity = quantityDist(gen);

    // 生成指定数量的物品
    for (int i = 0; i < quantity; ++i) {
        result.push_back(selectRandomItem());
    }

    return result;
} 