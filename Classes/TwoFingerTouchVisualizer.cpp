//
//  TwoFingerTouchVisualizer.cpp
//  Zoomer
//
//  Created by Kular on 2/1/15.
//
//

#include "TwoFingerTouchVisualizer.h"

USING_NS_CC;

#define NUM_OF_CIRCLES 2

TwoFingerTouchVisualizer::TwoFingerTouchVisualizer()
{
    m_Circles = std::map<int, cocos2d::DrawNode *>();
    for (int i = 0; i < NUM_OF_CIRCLES; i++) {
        m_Circles[i] = DrawNode::create();
        this->addChild(m_Circles[i]);
    }
    m_MidPoint = DrawNode::create();
    this->addChild(m_MidPoint);
}

TwoFingerTouchVisualizer::~TwoFingerTouchVisualizer()
{
    log("TwoFingerTouchVisualizer distructed");
}

void TwoFingerTouchVisualizer::drawCircle(const cocos2d::Vec2 &pos, const int &index)
{
    m_Circles[index]->drawCircle(pos, 140, 360, 20, false, 1, 1, Color4F::YELLOW);
}

void TwoFingerTouchVisualizer::drawMidPoint(const cocos2d::Vec2 &midPos, const cocos2d::Vec2 &pos1, const cocos2d::Vec2 &pos2)
{
    m_MidPoint->drawLine(pos1, pos2, Color4F::BLUE);
    m_MidPoint->drawPoint(midPos, 30.f, Color4F::RED);
}

void TwoFingerTouchVisualizer::clearCircle(const int &index)
{
    m_Circles[index]->clear();
}

void TwoFingerTouchVisualizer::clearMidPoint()
{
    m_MidPoint->clear();
}