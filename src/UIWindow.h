#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <vector>
#include <string>
#include <functional>
#include <map>
#include "UIElement.h"

// 元素点击回调函数类型定义
typedef std::function<void(const UIElement&)> ElementClickCallback;

// 元素渲染区域结构体
struct ElementRenderRect {
    float x;
    float y;
    float width;
    float height;
};

// UI块结构体
struct UIBlock {
    std::string name;           // 块名称
    float topY;                 // 块顶部Y坐标
    float bottomY;              // 块底部Y坐标
    SDL_Color backgroundColor;  // 背景色
    SDL_Color borderColor;      // 边框色
};

// 文本换行结果结构体
struct WrappedTextLine {
    std::string text;
    float width;
    float height;
};

// 布局计算结果结构体
struct LayoutCalculationResult {
    float totalWidth;
    float totalHeight;
    std::vector<std::vector<WrappedTextLine>> elementLines; // 每个元素的换行结果
};

class UIWindow {
private:
    bool isVisible;
    float x;
    float y;
    float width;
    float height;
    SDL_Color borderColor;
    Uint8 opacity;
    std::vector<UIElement> elements;
    
    // 存储每个元素的渲染区域
    std::map<size_t, ElementRenderRect> elementRects;
    
    // 用于累计Y轴偏移量
    float currentYOffset;
    
    // UI块管理
    std::vector<UIBlock> uiBlocks;
    bool blocksEnabled;
    
    // 元素点击回调
    ElementClickCallback elementClickCallback;

    // 字体
    TTF_Font* titleFont;      // 标题字体
    TTF_Font* subtitleFont;   // 副标题字体
    TTF_Font* normalFont;     // 普通文本字体
    
    // 自动布局相关参数
    float maxContentWidth;    // 最大内容宽度（用于换行计算）
    float padding;            // 内边距
    bool autoResize;          // 是否自动调整窗口大小
    
    // 滚动相关参数
    float scrollOffset;       // 滚动偏移量（负值表示向上滚动）
    float totalContentHeight; // 总内容高度
    bool scrollEnabled;       // 是否启用滚动
    
    // 文本换行相关方法
    std::vector<WrappedTextLine> wrapText(const std::string& text, TTF_Font* font, float maxWidth) const;
    float calculateTextWidth(const std::string& text, TTF_Font* font) const;
    float calculateTextHeight(TTF_Font* font) const;

public:
    UIWindow(float x, float y, float width, float height, SDL_Color borderColor = {255, 255, 255, 255}, Uint8 opacity = 180);
    ~UIWindow();
    
    // 设置字体
    void setFonts(TTF_Font* titleFont, TTF_Font* subtitleFont, TTF_Font* normalFont);
    
    // 基本属性设置和获取
    void setVisible(bool visible);
    bool getVisible() const;
    void setBorderColor(SDL_Color color);
    void setOpacity(Uint8 opacity);
    
    // 位置和尺寸获取
    float getX() const { return x; }
    float getY() const { return y; }
    float getWidth() const { return width; }
    float getHeight() const { return height; }

    void setX(float x)  { this->x = x; }
    void setY(float y)  { this->y = y; }
    void setWidth(float w)  { this->width=w; }
    void setHeight(float h)  { this->height=h; }
    
    // 自动布局相关方法
    void setMaxContentWidth(float maxWidth) { this->maxContentWidth = maxWidth; }
    float getMaxContentWidth() const { return maxContentWidth; }
    void setPadding(float padding) { this->padding = padding; }
    float getPadding() const { return padding; }
    void setAutoResize(bool autoResize) { this->autoResize = autoResize; }
    bool getAutoResize() const { return autoResize; }
    
    // 布局计算方法
    LayoutCalculationResult calculateLayout() const;
    void autoSizeToContent();
    void centerOnScreen(float screenWidth, float screenHeight);
    
    // 元素管理
    void addElement(const UIElement& element);
    void clearElements();
    const std::vector<UIElement>& getElements() const { return elements; }
    
    // 设置元素点击回调
    void setElementClickCallback(ElementClickCallback callback);
    
    // 处理点击事件
    bool handleClick(int mouseX, int mouseY, float windowWidth, float windowHeight);
    
    // 获取指定位置下的元素索引，如果没有元素则返回-1
    int getElementAtPosition(int mouseX, int mouseY) const;
    
    // 获取元素渲染区域
    bool getElementRect(size_t elementIndex, ElementRenderRect& outRect) const;
    
    // 更新和渲染
    void update();
    void render(SDL_Renderer* renderer, float windowWidth, float windowHeight);
    void renderWithWrapping(SDL_Renderer* renderer, float windowWidth, float windowHeight);
    
    // 调试功能：绘制元素边框
    void renderElementBorders(SDL_Renderer* renderer, SDL_Color borderColor = {255, 0, 0, 255});
    
    // UI块管理方法
    void setBlocksEnabled(bool enabled) { blocksEnabled = enabled; }
    bool getBlocksEnabled() const { return blocksEnabled; }
    void clearBlocks();
    void addBlock(const std::string& name, float topY, float bottomY,
                  SDL_Color backgroundColor, SDL_Color borderColor);
    void analyzeAndCreateBlocks(); // 动态分析元素并创建块
    void renderBlocks(SDL_Renderer* renderer);

    // 获取字体
    TTF_Font* getTitleFont() const { return titleFont; }
    TTF_Font* getSubtitleFont() const { return subtitleFont; }
    TTF_Font* getNormalFont() const { return normalFont; }
    
    // 获取字体大小比例
    float getFontSizeRatio(UIElementType type) const;
    
    // 滚动相关方法
    void setScrollEnabled(bool enabled) { scrollEnabled = enabled; }
    bool getScrollEnabled() const { return scrollEnabled; }
    void setScrollOffset(float offset);
    float getScrollOffset() const { return scrollOffset; }
    void scroll(float deltaY);
    void scrollToTop();
    void scrollToBottom();
    float getTotalContentHeight() const { return totalContentHeight; }
    bool canScrollUp() const;
    bool canScrollDown() const;
    
    // 处理滚动事件
    bool handleScroll(int mouseX, int mouseY, float scrollDelta);
};