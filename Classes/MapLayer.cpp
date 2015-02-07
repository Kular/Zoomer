//
//  MapLayer.cpp
//  Zoomer
//
//  Created by Kular on 2/1/15.
//
//

#include "MapLayer.h"
USING_NS_CC;

bool MapLayer::initWithVisibleSize(const cocos2d::Size &visibleSize)
{
    if (!Layer::init()) {
        return false;
    }
    
    m_Map = Sprite::create("res/test_map_grid.png");
    m_Map->setAnchorPoint(Vec2::ZERO);
    m_Map->setPosition(Vec2::ZERO);
    this->addChild(m_Map);
    return true;
}

Size MapLayer::getMapSize() const
{
    return m_Map->getContentSize();
}