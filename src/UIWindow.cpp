#include "UIWindow.h"
#include <SDL3/SDL.h>
#include <iostream>

UIWindow::UIWindow(float x, float y, float width, float height, SDL_Color borderColor, Uint8 opacity) :
    isVisible(true),
    x(x),
    y(y),
    width(width),
    height(height),
    borderColor(borderColor),
    opacity(opacity),
    currentYOffset(0.0f),
    elementClickCallback(nullptr),
    titleFont(nullptr),
    subtitleFont(nullptr),
    normalFont(nullptr) {
    // 字体将由GameUI设置
}

UIWindow::~UIWindow() {
    // 不需要释放字体资源，因为字体由GameUI管理
    titleFont = nullptr;
    subtitleFont = nullptr;
    normalFont = nullptr;
}

void UIWindow::setFonts(TTF_Font* titleFont, TTF_Font* subtitleFont, TTF_Font* normalFont) {
    this->titleFont = titleFont;
    this->subtitleFont = subtitleFont;
    this->normalFont = normalFont;
}

void UIWindow::setVisible(bool visible) {
    this->isVisible = visible;
}

bool UIWindow::getVisible() const {
    return isVisible;
}

void UIWindow::setBorderColor(SDL_Color color) {
    this->borderColor = color;
}

void UIWindow::setOpacity(Uint8 opacity) {
    this->opacity = opacity;
}

void UIWindow::addElement(const UIElement& element) {
    elements.push_back(element);
}

void UIWindow::clearElements() {
    elements.clear();
}

void UIWindow::setElementClickCallback(ElementClickCallback callback) {
    this->elementClickCallback = callback;
}

int UIWindow::getElementAtPosition(int mouseX, int mouseY) const {
    if (!isVisible) return -1;
    
    // 检查点击是否在窗口区域内
    if (mouseX < x || mouseX > x + width || mouseY < y || mouseY > y + height) {
        return -1;
    }
    
    // 遍历所有元素，检查点击是否在元素上
    for (size_t i = 0; i < elements.size(); ++i) {
        // 检查该元素是否有渲染区域记录
        auto rectIt = elementRects.find(i);
        if (rectIt == elementRects.end()) continue;
        
        // 获取元素的渲染区域
        const ElementRenderRect& rect = rectIt->second;
        
        // 检查点击是否在元素区域内
        if (mouseX >= rect.x && mouseX <= rect.x + rect.width &&
            mouseY >= rect.y && mouseY <= rect.y + rect.height) {
            
            return static_cast<int>(i); // 返回元素索引
        }
    }
    
    return -1; // 没有找到元素
}

bool UIWindow::handleClick(int mouseX, int mouseY, float windowWidth, float windowHeight) {
    if (!isVisible) return false;
    
    // 获取点击位置下的元素索引
    int elementIndex = getElementAtPosition(mouseX, mouseY);
    
    // 如果找到了元素并且设置了回调函数，调用它
    if (elementIndex >= 0 && elementClickCallback) {
        elementClickCallback(elements[elementIndex]);
        return true; // 点击已处理
    }
    
    // 检查点击是否在窗口区域内（即使没有点击到具体元素）
    if (mouseX >= x && mouseX <= x + width && mouseY >= y && mouseY <= y + height) {
        return true; // 点击在窗口内，但没有点击到元素
    }
    
    return false; // 点击未处理
}

bool UIWindow::getElementRect(size_t elementIndex, ElementRenderRect& outRect) const {
    // 检查元素索引是否有效
    if (elementIndex >= elements.size()) {
        return false;
    }
    
    // 查找元素的渲染区域
    auto it = elementRects.find(elementIndex);
    if (it == elementRects.end()) {
        return false; // 该元素尚未渲染或没有记录渲染区域
    }
    
    // 返回渲染区域
    outRect = it->second;
    return true;
}

void UIWindow::update() {
    // 在这里可以添加窗口状态更新逻辑
    // 例如动画效果、元素状态更新等
}

void UIWindow::render(SDL_Renderer* renderer, float windowWidth, float windowHeight) {
    if (!isVisible) return;
    
    // 绘制半透明背景
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, opacity);
    SDL_FRect bgRect = {x, y, width, height};
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect(renderer, &bgRect);
    
    // 绘制边框
    SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
    SDL_RenderRect(renderer, &bgRect);
    
    // 重置Y轴累计偏移量
    currentYOffset = 30.0f;
    
    // 清空元素渲染区域映射
    elementRects.clear();
    
    // 渲染所有元素
    for (size_t i = 0; i < elements.size(); ++i) {
        const auto& element = elements[i];
        
        // 根据元素类型选择合适的字体
        TTF_Font* font = nullptr;
        switch (element.getType()) {
            case UIElementType::TITLE:
                font = titleFont;
                break;
            case UIElementType::SUBTITLE:
                font = subtitleFont;
                break;
            case UIElementType::TEXT:
            default:
                font = normalFont;
                break;
        }
        
        // 确保字体已加载
        if (!font) continue;
        
        // 创建文本表面 - 使用SDL3_ttf的四参数版本
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, element.getText().c_str(), 0, element.getColor());
        if (textSurface) {
            // 创建纹理
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            if (textTexture) {
                // 获取文本尺寸
                int textWidth = textSurface->w;
                int textHeight = textSurface->h;
                
                // 创建渲染矩形，基于窗口位置、元素X偏移和当前Y累计偏移
                // X偏移量也应该与字体大小成比例
                float fontSizeRatio = getFontSizeRatio(element.getType());
                
                SDL_FRect renderRect = {
                    x + element.getXOffset() * fontSizeRatio,
                    y + currentYOffset,
                    static_cast<float>(textWidth),
                    static_cast<float>(textHeight)
                };
                
                // 存储元素渲染区域
                elementRects[i] = {
                    renderRect.x,
                    renderRect.y,
                    renderRect.w,
                    renderRect.h
                };
                
                // 渲染文本
                SDL_RenderTexture(renderer, textTexture, nullptr, &renderRect);
                
                // 释放纹理
                SDL_DestroyTexture(textTexture);
            }
            
            // 释放表面
            SDL_DestroySurface(textSurface);
        }
        
        // 累加Y轴偏移量，根据字体大小调整
        float yOffsetRatio = getFontSizeRatio(element.getType());
        currentYOffset += element.getYOffset() * yOffsetRatio;
    }
}

float UIWindow::getFontSizeRatio(UIElementType type) const {
    switch (type) {
        case UIElementType::TITLE:
            return 1.5f; // 标题字体大小比例，增大影响
        case UIElementType::SUBTITLE:
            return 1.5f; // 副标题字体大小比例，增大影响
        case UIElementType::TEXT:
        default:
            return 1.3f; // 普通文本字体大小比例
    }
}

void UIWindow::renderElementBorders(SDL_Renderer* renderer, SDL_Color borderColor) {
    if (!isVisible) return;
    
    // 保存当前渲染器状态
    SDL_Color originalColor;
    SDL_GetRenderDrawColor(renderer, &originalColor.r, &originalColor.g, &originalColor.b, &originalColor.a);
    SDL_BlendMode originalBlendMode;
    SDL_GetRenderDrawBlendMode(renderer, &originalBlendMode);
    
    // 设置渲染器状态
    SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    
    // 绘制每个元素的边框
    for (const auto& rectPair : elementRects) {
        const ElementRenderRect& rect = rectPair.second;
        SDL_FRect borderRect = {
            rect.x,
            rect.y,
            rect.width,
            rect.height
        };
        SDL_RenderRect(renderer, &borderRect);
    }
    
    // 恢复渲染器状态
    SDL_SetRenderDrawColor(renderer, originalColor.r, originalColor.g, originalColor.b, originalColor.a);
    SDL_SetRenderDrawBlendMode(renderer, originalBlendMode);
}