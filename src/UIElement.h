#pragma once

#include <SDL3_ttf/SDL_ttf.h>
#include <string>

// 定义UI元素类型枚举
enum class UIElementType {
    TEXT,       // 普通文本
    TITLE,      // 标题
    SUBTITLE    // 副标题
};

class UIElement {
private:
    std::string text;
    float xOffset;     // 相对于窗口的X轴偏移量
    float yOffset;     // 元素自身的Y轴偏移量（渲染后累加到窗口的Y轴累计偏移量）
    SDL_Color color;
    UIElementType type; // 元素类型
    void* dataPtr;     // 指向关联数据的指针（如物品指针等）

public:
    UIElement();
    UIElement(const std::string& text, float xOffset, float yOffset, 
              SDL_Color color = {255, 255, 255, 255}, 
              UIElementType type = UIElementType::TEXT, void* dataPtr = nullptr);
    
    // 基本属性设置和获取
    void setText(const std::string& text);
    std::string getText() const;
    
    void setXOffset(float offset);
    float getXOffset() const;
    
    void setYOffset(float offset);
    float getYOffset() const;
    
    void setColor(SDL_Color color);
    SDL_Color getColor() const;
    
    void setType(UIElementType type);
    UIElementType getType() const;
    
    // 兼容旧代码的方法
    bool getIsTitle() const { return type == UIElementType::TITLE; }
    bool getIsSubtitle() const { return type == UIElementType::SUBTITLE; }
    
    void setDataPtr(void* ptr);
    void* getDataPtr() const;
};