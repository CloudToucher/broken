#include "UIWindow.h"
#include <SDL3/SDL.h>
#include <iostream>
#include <algorithm>
#include <sstream>

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
    normalFont(nullptr),
    maxContentWidth(400.0f),
    padding(20.0f),
    autoResize(false),
    blocksEnabled(false),
    scrollOffset(0.0f),
    totalContentHeight(0.0f),
    scrollEnabled(true) {
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

// 计算文本宽度
float UIWindow::calculateTextWidth(const std::string& text, TTF_Font* font) const {
    if (!font || text.empty()) return 0.0f;
    
    int width = 0;
    TTF_GetStringSize(font, text.c_str(), 0, &width, nullptr);
    return static_cast<float>(width);
}

// 计算文本高度
float UIWindow::calculateTextHeight(TTF_Font* font) const {
    if (!font) return 0.0f;
    
    return static_cast<float>(TTF_GetFontHeight(font));
}

// 文本换行
std::vector<WrappedTextLine> UIWindow::wrapText(const std::string& text, TTF_Font* font, float maxWidth) const {
    std::vector<WrappedTextLine> lines;
    if (!font || text.empty()) {
        return lines;
    }
    
    // 按空格分割文本
    std::istringstream iss(text);
    std::vector<std::string> words;
    std::string word;
    while (iss >> word) {
        words.push_back(word);
    }
    
    if (words.empty()) {
        // 如果没有单词，但文本不为空（可能只有空格），创建一个空行
        WrappedTextLine line;
        line.text = text;
        line.width = calculateTextWidth(text, font);
        line.height = calculateTextHeight(font);
        lines.push_back(line);
        return lines;
    }
    
    std::string currentLine = words[0];
    float currentLineWidth = calculateTextWidth(currentLine, font);
    
    for (size_t i = 1; i < words.size(); ++i) {
        std::string testLine = currentLine + " " + words[i];
        float testWidth = calculateTextWidth(testLine, font);
        
        if (testWidth <= maxWidth) {
            // 可以添加到当前行
            currentLine = testLine;
            currentLineWidth = testWidth;
        } else {
            // 需要换行
            WrappedTextLine line;
            line.text = currentLine;
            line.width = currentLineWidth;
            line.height = calculateTextHeight(font);
            lines.push_back(line);
            
            // 开始新行
            currentLine = words[i];
            currentLineWidth = calculateTextWidth(currentLine, font);
        }
    }
    
    // 添加最后一行
    if (!currentLine.empty()) {
        WrappedTextLine line;
        line.text = currentLine;
        line.width = currentLineWidth;
        line.height = calculateTextHeight(font);
        lines.push_back(line);
    }
    
    return lines;
}

// 计算布局
LayoutCalculationResult UIWindow::calculateLayout() const {
    LayoutCalculationResult result;
    result.totalWidth = 0.0f;
    result.totalHeight = padding; // 游戏简约风格 - 标准顶部间距
    result.elementLines.clear();
    
    for (const auto& element : elements) {
        // 选择字体
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
        
        if (!font) continue;
        
        // 计算文本换行
        std::vector<WrappedTextLine> lines = wrapText(element.getText(), font, maxContentWidth);
        result.elementLines.push_back(lines);
        
        // 计算最大宽度
        for (const auto& line : lines) {
            result.totalWidth = std::max(result.totalWidth, line.width + element.getXOffset());
        }
        
        // 累加高度 - 游戏简约风格的标准间距
        for (const auto& line : lines) {
            result.totalHeight += line.height;
        }
        
        // 添加元素的Y偏移量
        result.totalHeight += element.getYOffset();
    }
    
    // 游戏简约风格 - 标准底部间距
    result.totalHeight += padding;
    
    // 确保最小宽度包含左右边距
    result.totalWidth = std::max(result.totalWidth + 2 * padding, 200.0f);
    
    return result;
}

// 自动调整窗口大小以适应内容
void UIWindow::autoSizeToContent() {
    if (!autoResize) return;
    
    LayoutCalculationResult layout = calculateLayout();
    setWidth(layout.totalWidth);
    setHeight(layout.totalHeight);
}

// 居中显示在屏幕上
void UIWindow::centerOnScreen(float screenWidth, float screenHeight) {
    if (autoResize) {
        autoSizeToContent();
    }
    
    setX((screenWidth - getWidth()) / 2.0f);
    setY((screenHeight - getHeight()) / 2.0f);
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
    
    // 检查点击是否在滚动条区域（如果启用了滚动且内容超出窗口高度）
    if (scrollEnabled && totalContentHeight > height) {
        float scrollbarWidth = 8.0f;
        float scrollbarX = x + width - scrollbarWidth - 2.0f;
        if (mouseX >= scrollbarX && mouseX <= scrollbarX + scrollbarWidth &&
            mouseY >= y && mouseY <= y + height) {
            // 点击在滚动条区域，不处理元素点击
            return true; // 消费事件但不触发元素点击
        }
    }
    
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

// 带文本换行的渲染方法
void UIWindow::renderWithWrapping(SDL_Renderer* renderer, float windowWidth, float windowHeight) {
    if (!isVisible) return;
    
    // 游戏简约风格 - 主窗口背景
    SDL_SetRenderDrawColor(renderer, 32, 32, 34, 255); // 游戏风格深色背景
    SDL_FRect bgRect = {x, y, width, height};
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_RenderFillRect(renderer, &bgRect);
    
    // 游戏简约风格 - 简洁的投影阴影
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    
    // 简单的单层阴影，更适合游戏风格
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 40);
    SDL_FRect simpleShadow = {x + 3.0f, y + 3.0f, width, height};
    SDL_RenderFillRect(renderer, &simpleShadow);
    
    // 游戏简约风格 - 清晰的窗口边框
    SDL_SetRenderDrawColor(renderer, 64, 64, 68, 255); // 游戏风格边框
    SDL_FRect borderRect = {x - 1, y - 1, width + 2, height + 2};
    SDL_RenderRect(renderer, &borderRect);
    
    // 重置Y轴累计偏移量
    currentYOffset = padding; // 使用标准间距
    
    // 清空元素渲染区域映射
    elementRects.clear();
    
    // 计算布局
    LayoutCalculationResult layout = calculateLayout();
    
    // 渲染所有元素（带换行）
    for (size_t i = 0; i < elements.size() && i < layout.elementLines.size(); ++i) {
        const auto& element = elements[i];
        const auto& lines = layout.elementLines[i];
        
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
        
        // 记录该元素的渲染区域开始位置
        float elementStartY = currentYOffset;
        float elementMaxWidth = 0.0f;
        float elementTotalHeight = 0.0f;
        
        // 渲染每一行
        for (const auto& line : lines) {
            if (line.text.empty()) {
                // 空行，只增加高度
                float lineHeight = calculateTextHeight(font);
                currentYOffset += lineHeight;
                elementTotalHeight += lineHeight;
                continue;
            }
            
            // 创建文本表面
            SDL_Surface* textSurface = TTF_RenderText_Solid(font, line.text.c_str(), 0, element.getColor());
            if (textSurface) {
                // 创建纹理
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                if (textTexture) {
                    // 计算渲染位置
                    SDL_FRect renderRect = {
                        x + padding + element.getXOffset(),
                        y + currentYOffset,
                        static_cast<float>(textSurface->w),
                        static_cast<float>(textSurface->h)
                    };
                    
                    // 渲染文本
                    SDL_RenderTexture(renderer, textTexture, nullptr, &renderRect);
                    
                    // 更新元素最大宽度
                    elementMaxWidth = std::max(elementMaxWidth, renderRect.w);
                    
                    // 更新高度
                    elementTotalHeight += renderRect.h;
                    currentYOffset += renderRect.h;
                    
                    // 释放纹理
                    SDL_DestroyTexture(textTexture);
                }
                
                // 释放表面
                SDL_DestroySurface(textSurface);
            }
        }
        
        // 存储整个元素的渲染区域（包含所有行）
        elementRects[i] = {
            x + padding + element.getXOffset(),
            y + elementStartY,
            elementMaxWidth,
            elementTotalHeight
        };
        
        // 添加元素的Y偏移量
        currentYOffset += element.getYOffset();
    }
    
    // 分析并创建UI块，然后先渲染背景块（如果启用）
    if (blocksEnabled) {
        analyzeAndCreateBlocks();
        renderBlocks(renderer);
        
        // 重新渲染所有文本元素（在背景块之上）
        // 重置Y偏移量以保持一致性
        currentYOffset = padding;
        
        for (size_t i = 0; i < elements.size() && i < layout.elementLines.size(); ++i) {
            const auto& element = elements[i];
            const auto& lines = layout.elementLines[i];
            
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
            
            // 重新渲染每一行，保持与第一次渲染的一致性
            for (const auto& line : lines) {
                if (line.text.empty()) {
                    // 空行，只增加高度
                    float lineHeight = calculateTextHeight(font);
                    currentYOffset += lineHeight;
                    continue;
                }
                
                // 创建文本表面
                SDL_Surface* textSurface = TTF_RenderText_Solid(font, line.text.c_str(), 0, element.getColor());
                if (textSurface) {
                    // 创建纹理
                    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                    if (textTexture) {
                        // 计算渲染位置（与第一次渲染保持一致）
                        SDL_FRect renderRect = {
                            x + padding + element.getXOffset(),
                            y + currentYOffset,
                            static_cast<float>(textSurface->w),
                            static_cast<float>(textSurface->h)
                        };
                        
                        // 渲染文本
                        SDL_RenderTexture(renderer, textTexture, nullptr, &renderRect);
                        
                        // 更新Y偏移量
                        currentYOffset += renderRect.h;
                        
                        // 释放纹理
                        SDL_DestroyTexture(textTexture);
                    }
                    
                    // 释放表面
                    SDL_DestroySurface(textSurface);
                }
            }
            
            // 添加元素的Y偏移量
            currentYOffset += element.getYOffset();
        }
    }
}

// 游戏简约风格 - 原始渲染方法更新
void UIWindow::render(SDL_Renderer* renderer, float windowWidth, float windowHeight) {
    if (!isVisible) return;
    
    // 游戏简约风格 - 主窗口背景
    SDL_SetRenderDrawColor(renderer, 32, 32, 34, 255); // 游戏风格深色背景
    SDL_FRect bgRect = {x, y, width, height};
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_RenderFillRect(renderer, &bgRect);
    
    // 游戏简约风格 - 简洁的投影阴影
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 40);
    SDL_FRect simpleShadow = {x + 3.0f, y + 3.0f, width, height};
    SDL_RenderFillRect(renderer, &simpleShadow);
    
    // 游戏简约风格 - 清晰的窗口边框
    SDL_SetRenderDrawColor(renderer, 64, 64, 68, 255);
    SDL_FRect borderRect = {x - 1, y - 1, width + 2, height + 2};
    SDL_RenderRect(renderer, &borderRect);
    
    // 设置剪切区域以限制渲染在窗口内
    SDL_Rect clipRect = {static_cast<int>(x), static_cast<int>(y), static_cast<int>(width), static_cast<int>(height)};
    SDL_SetRenderClipRect(renderer, &clipRect);
    
    // 重置Y轴累计偏移量，应用滚动偏移
    currentYOffset = padding - scrollOffset; // 使用标准间距并应用滚动偏移
    
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
    
    // 分析并创建UI块，然后先渲染背景块（如果启用）
    if (blocksEnabled) {
        analyzeAndCreateBlocks();
        renderBlocks(renderer);
        
        // 重新渲染所有文本元素（在背景块之上）
        currentYOffset = padding - scrollOffset; // 重置Y偏移量并应用滚动偏移
        
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
            
            // 创建文本表面
            SDL_Surface* textSurface = TTF_RenderText_Solid(font, element.getText().c_str(), 0, element.getColor());
            if (textSurface) {
                // 创建纹理
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                if (textTexture) {
                    // 计算渲染位置
                    float fontSizeRatio = getFontSizeRatio(element.getType());
                    
                    SDL_FRect renderRect = {
                        x + element.getXOffset() * fontSizeRatio,
                        y + currentYOffset,
                        static_cast<float>(textSurface->w),
                        static_cast<float>(textSurface->h)
                    };
                    
                    // 渲染文本
                    SDL_RenderTexture(renderer, textTexture, nullptr, &renderRect);
                    
                    // 释放纹理
                    SDL_DestroyTexture(textTexture);
                }
                
                // 释放表面
                SDL_DestroySurface(textSurface);
            }
            
            // 累加Y轴偏移量
            float yOffsetRatio = getFontSizeRatio(element.getType());
            currentYOffset += element.getYOffset() * yOffsetRatio;
        }
    }
    
    // 更新总内容高度用于滚动计算
    totalContentHeight = currentYOffset + scrollOffset + padding;
    
    // 清除剪切区域
    SDL_SetRenderClipRect(renderer, nullptr);
    
    // 如果启用滚动并且内容超出窗口高度，绘制滚动指示器
    if (scrollEnabled && totalContentHeight > height) {
        float scrollbarWidth = 8.0f;
        float scrollbarHeight = height;
        float scrollbarX = x + width - scrollbarWidth - 2.0f;
        float scrollbarY = y;
        
        // 绘制滚动条背景
        SDL_SetRenderDrawColor(renderer, 64, 64, 68, 180);
        SDL_FRect scrollbarBg = {scrollbarX, scrollbarY, scrollbarWidth, scrollbarHeight};
        SDL_RenderFillRect(renderer, &scrollbarBg);
        
        // 计算滚动条滑块的位置和大小
        float contentRatio = height / totalContentHeight;
        float thumbHeight = std::max(20.0f, scrollbarHeight * contentRatio);
        float maxThumbOffset = scrollbarHeight - thumbHeight;
        float scrollRatio = scrollOffset / (totalContentHeight - height);
        float thumbY = scrollbarY + maxThumbOffset * scrollRatio;
        
        // 绘制滚动条滑块
        SDL_SetRenderDrawColor(renderer, 128, 128, 132, 220);
        SDL_FRect scrollbarThumb = {scrollbarX + 1.0f, thumbY, scrollbarWidth - 2.0f, thumbHeight};
        SDL_RenderFillRect(renderer, &scrollbarThumb);
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

// UI块管理方法实现
void UIWindow::clearBlocks() {
    uiBlocks.clear();
}

void UIWindow::addBlock(const std::string& name, float topY, float bottomY,
                        SDL_Color backgroundColor, SDL_Color borderColor) {
    UIBlock block;
    block.name = name;
    block.topY = topY;
    block.bottomY = bottomY;
    block.backgroundColor = backgroundColor;
    block.borderColor = borderColor;
    uiBlocks.push_back(block);
}

void UIWindow::analyzeAndCreateBlocks() {
    if (!isVisible || !blocksEnabled) return;
    
    // 清空现有块
    clearBlocks();
    
    // 当前处理的块
    std::string currentBlockName = "";
    float currentBlockTopY = 0.0f;
    SDL_Color currentBackgroundColor = {0, 0, 0, 0};
    SDL_Color currentBorderColor = {0, 0, 0, 0};
    
    // 遍历所有元素，根据内容识别逻辑块
    for (size_t i = 0; i < elements.size(); ++i) {
        const auto& element = elements[i];
        
        // 获取元素的渲染区域
        ElementRenderRect rect;
        if (!getElementRect(i, rect)) {
            continue;
        }
        
        // 根据元素文本内容识别块边界
        std::string text = element.getText();
        
        // 检查是否是新块的开始
        bool isNewBlockStart = false;
        std::string newBlockName = "";
        SDL_Color newBackgroundColor = {0, 0, 0, 0};
        SDL_Color newBorderColor = {0, 0, 0, 0};
        
        if (text == "玩家背包" && element.getType() == UIElementType::TITLE) {
            // 游戏简约风格 - 标题块（淡蓝色）
            isNewBlockStart = true;
            newBlockName = "标题";
            newBackgroundColor = {48, 52, 64, 200}; // 游戏风格淡蓝灰色
            newBorderColor = {72, 85, 106, 255};
        }
        else if (text == "手持物品" && element.getType() == UIElementType::SUBTITLE) {
            // 游戏简约风格 - 手持物品块（淡橙色）
            isNewBlockStart = true;
            newBlockName = "手持物品";
            newBackgroundColor = {64, 52, 48, 200}; // 游戏风格淡橙灰色
            newBorderColor = {106, 85, 72, 255};
        }
        else if (text == "已装备物品:" && element.getType() == UIElementType::SUBTITLE) {
            // 游戏简约风格 - 已装备物品块（淡绿色）
            isNewBlockStart = true;
            newBlockName = "已装备物品";
            newBackgroundColor = {48, 64, 52, 200}; // 游戏风格淡绿灰色
            newBorderColor = {72, 106, 85, 255};
        }
        else if (text == "背包物品:" && element.getType() == UIElementType::SUBTITLE) {
            // 游戏简约风格 - 背包物品块（淡黄色）
            isNewBlockStart = true;
            newBlockName = "背包物品";
            newBackgroundColor = {64, 60, 48, 200}; // 游戏风格淡黄灰色
            newBorderColor = {106, 100, 72, 255};
        }
        
        // 如果检测到新块开始，先完成当前块（如果有）
        if (isNewBlockStart && !currentBlockName.empty()) {
            // 完成当前块，使用当前元素的顶部作为当前块的底部
            float blockPadding = 8.0f; // 游戏风格适中间距
            addBlock(currentBlockName, 
                    currentBlockTopY - blockPadding, 
                    rect.y - blockPadding,
                    currentBackgroundColor, currentBorderColor);
        }
        
        // 开始新块
        if (isNewBlockStart) {
            currentBlockName = newBlockName;
            currentBlockTopY = rect.y;
            currentBackgroundColor = newBackgroundColor;
            currentBorderColor = newBorderColor;
        }
    }
    
    // 完成最后一个块（如果有）
    if (!currentBlockName.empty()) {
        float blockPadding = 8.0f;
        addBlock(currentBlockName, 
                currentBlockTopY - blockPadding, 
                y + height - blockPadding,
                currentBackgroundColor, currentBorderColor);
    }
}

void UIWindow::renderBlocks(SDL_Renderer* renderer) {
    if (!isVisible || !blocksEnabled || uiBlocks.empty()) return;
    
    // 保存当前渲染器状态
    SDL_Color originalColor;
    SDL_GetRenderDrawColor(renderer, &originalColor.r, &originalColor.g, &originalColor.b, &originalColor.a);
    SDL_BlendMode originalBlendMode;
    SDL_GetRenderDrawBlendMode(renderer, &originalBlendMode);
    
    // 设置混合模式
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    
    // 渲染每个块 - 游戏简约风格
    for (const auto& block : uiBlocks) {
        float blockX = x + 8.0f; // 游戏风格适中边距
        float blockY = block.topY;
        float blockWidth = width - 16.0f;
        float blockHeight = block.bottomY - block.topY;
        
        // 1. 游戏简约风格 - 简单的块阴影
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 25);
        SDL_FRect blockShadow = {blockX + 2.0f, blockY + 2.0f, blockWidth, blockHeight};
        SDL_RenderFillRect(renderer, &blockShadow);
        
        // 2. 游戏简约风格 - 主背景
        SDL_SetRenderDrawColor(renderer, block.backgroundColor.r, block.backgroundColor.g, 
                              block.backgroundColor.b, block.backgroundColor.a);
        SDL_FRect bgRect = {blockX, blockY, blockWidth, blockHeight};
        SDL_RenderFillRect(renderer, &bgRect);
        
        // 3. 游戏简约风格 - 简洁的边框
        SDL_SetRenderDrawColor(renderer, block.borderColor.r, block.borderColor.g, 
                              block.borderColor.b, 150);
        SDL_FRect borderRect = {blockX, blockY, blockWidth, blockHeight};
        SDL_RenderRect(renderer, &borderRect);
        
        // 4. 游戏简约风格 - 左侧强调线
        SDL_SetRenderDrawColor(renderer, block.borderColor.r + 20, block.borderColor.g + 20, 
                              block.borderColor.b + 20, 200);
        SDL_FRect accentLine = {blockX + 1.0f, blockY + 2.0f, 2.0f, blockHeight - 4.0f};
        SDL_RenderFillRect(renderer, &accentLine);
    }
    
    // 恢复渲染器状态
    SDL_SetRenderDrawColor(renderer, originalColor.r, originalColor.g, originalColor.b, originalColor.a);
    SDL_SetRenderDrawBlendMode(renderer, originalBlendMode);
}

// 滚动相关方法实现

void UIWindow::setScrollOffset(float offset) {
    if (!scrollEnabled) return;
    
    // 限制滚动范围
    float maxScroll = std::max(0.0f, totalContentHeight - height);
    scrollOffset = std::max(0.0f, std::min(offset, maxScroll));
}

void UIWindow::scroll(float deltaY) {
    if (!scrollEnabled) return;
    
    setScrollOffset(scrollOffset + deltaY);
}

void UIWindow::scrollToTop() {
    if (!scrollEnabled) return;
    
    scrollOffset = 0.0f;
}

void UIWindow::scrollToBottom() {
    if (!scrollEnabled) return;
    
    float maxScroll = std::max(0.0f, totalContentHeight - height);
    scrollOffset = maxScroll;
}

bool UIWindow::canScrollUp() const {
    return scrollEnabled && scrollOffset > 0.0f;
}

bool UIWindow::canScrollDown() const {
    if (!scrollEnabled) return false;
    
    float maxScroll = std::max(0.0f, totalContentHeight - height);
    return scrollOffset < maxScroll;
}

bool UIWindow::handleScroll(int mouseX, int mouseY, float scrollDelta) {
    if (!scrollEnabled) return false;
    
    // 检查鼠标是否在窗口内
    if (mouseX < x || mouseX > x + width || mouseY < y || mouseY > y + height) {
        return false;
    }
    
    // 执行滚动 - 修复滚动方向（向上滚动应该显示上面的内容）
    float scrollAmount = -scrollDelta * 30.0f; // 反转滚动方向并调整滚动速度
    scroll(scrollAmount);
    
    return true; // 表示事件已处理
}