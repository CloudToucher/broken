#include "UIElement.h"

UIElement::UIElement() : 
    text(""), 
    xOffset(0.0f), 
    yOffset(0.0f), 
    color({255, 255, 255, 255}), 
    type(UIElementType::TEXT), 
    dataPtr(nullptr) {
}

UIElement::UIElement(const std::string& text, float xOffset, float yOffset, 
                     SDL_Color color, UIElementType type, void* dataPtr) :
    text(text),
    xOffset(xOffset),
    yOffset(yOffset),
    color(color),
    type(type),
    dataPtr(dataPtr) {
}

void UIElement::setText(const std::string& text) {
    this->text = text;
}

std::string UIElement::getText() const {
    return text;
}

void UIElement::setXOffset(float offset) {
    this->xOffset = offset;
}

float UIElement::getXOffset() const {
    return xOffset;
}

void UIElement::setYOffset(float offset) {
    this->yOffset = offset;
}

float UIElement::getYOffset() const {
    return yOffset;
}

void UIElement::setColor(SDL_Color color) {
    this->color = color;
}

SDL_Color UIElement::getColor() const {
    return color;
}

void UIElement::setType(UIElementType type) {
    this->type = type;
}

UIElementType UIElement::getType() const {
    return type;
}

void UIElement::setDataPtr(void* ptr) {
    this->dataPtr = ptr;
}

void* UIElement::getDataPtr() const {
    return dataPtr;
}