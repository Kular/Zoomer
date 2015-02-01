//
//  ByeWorldScene.cpp
//  Zoomer
//
//  Created by Kular on 2/1/15.
//
//

#include "ByeWorldScene.h"
#include "MapLayer.h"

USING_NS_CC;

#define MAX_TOUCHES 2

void ByeWorld::initWithVisibleSize(const Size &visibleSize)
{
    initTouchIdsPositions();
    this->addChild(&m_TwoFingerTouchVisualizer);
    
    auto worldLayer = MapLayer::create();
    worldLayer->initWithVisibleSize(visibleSize);
    worldLayer->setCameraMask(2);
    this->addChild(worldLayer);
    
    m_Fov = 60.f;
    m_Aspect = visibleSize.width / visibleSize.height;
    m_Near = 1.f;
    m_Far = 9999.f;
    m_MainCamera = Camera::createPerspective(m_Fov, m_Aspect, m_Near, m_Far);
    initWorldCameraWithVisibleSize(visibleSize, worldLayer->getMapSize());
    this->addChild(m_MainCamera);
    
    auto eventDispatcher = Director::getInstance()->getEventDispatcher();
    auto multiTouchEventListener = EventListenerTouchAllAtOnce::create();
    multiTouchEventListener->onTouchesBegan = CC_CALLBACK_2(ByeWorld::touchesBegan, this);
    multiTouchEventListener->onTouchesMoved = CC_CALLBACK_2(ByeWorld::touchesMoved, this);
    multiTouchEventListener->onTouchesEnded = CC_CALLBACK_2(ByeWorld::touchesEnded, this);
    eventDispatcher->addEventListenerWithSceneGraphPriority(multiTouchEventListener, this);
}

void ByeWorld::initTouchIdsPositions()
{
    m_TouchIDs = std::map<int, bool>();
    m_TouchPositions = std::map<int, cocos2d::Vec2>();
    for (int i = 0; i < MAX_TOUCHES; i++) {
        m_TouchIDs[i] = false;
        m_TouchPositions[i] = Vec2::ZERO;
    }
}

void ByeWorld::initWorldCameraWithVisibleSize(const cocos2d::Size &visibleSize, const cocos2d::Size &worldSize)
{
    m_MainCamera->setCameraFlag(CameraFlag::USER1);
    auto widthRatio = visibleSize.width / worldSize.width;
    auto heightRatio = visibleSize.height / worldSize.height;
    m_NormalZ = (visibleSize.height / 2) / (tanf(m_Fov / 2 * M_PI / 180));
    
    if (widthRatio >= heightRatio) {
        // should adjust width of camera's view to the width of map
        auto heightOfView = worldSize.width / m_Aspect;
        m_MaxZ = (heightOfView / 2) / tanf(m_Fov/2 * M_PI / 180);
    } else {
        // should adjust height of camera's view to the height of map
        m_MaxZ = (worldSize.height / 2) / tanf(m_Fov/2 * M_PI / 180);
    }
    m_RecentZ = m_MaxZ;
    m_MinZ = 2.f/3.f * m_NormalZ;
    m_RecentScale = m_RecentZ / m_NormalZ;
    log("init scale: %f", m_RecentScale);
    m_MainCamera->setPosition3D(Vec3(visibleSize.width / 2, visibleSize.height / 2, m_RecentZ));
    m_MainCamera->lookAt(Vec3(visibleSize.width / 2, visibleSize.height / 2, 0), Vec3(0, 1, 0));
}

Vec2 ByeWorld::getMidPos(const cocos2d::Vec2 &pos1, const cocos2d::Vec2 &pos2)
{
    return (pos1 + pos2) / 2;
}

float ByeWorld::deltaScaleToDeltaZ(const float &deltaScale)
{
    return tanf(m_Fov / 2 * M_PI / 180) * deltaScale;
}

Vec3 ByeWorld::lerp(const cocos2d::Vec3 &from, const cocos2d::Vec3 &to, float t)
{
    return from + t * (to - from);
}

void ByeWorld::touchesBegan(const std::vector<cocos2d::Touch *> &touches, cocos2d::Event *event)
{
    for (auto &touch : touches) {
        int touchId = touch->getID();
        if (touchId > 1) {
            continue;
        }
        auto pos = touch->getLocation();
        m_TwoFingerTouchVisualizer.drawCircle(pos, touchId);
        
        m_TouchIDs[touchId] = true;
        
        m_TouchPositions[touchId] = pos;
    }
    
    if (m_TouchIDs.at(0) && m_TouchIDs.at(1)) {
        Vec2 midPos = getMidPos(m_TouchPositions[0], m_TouchPositions[1]);
        m_TwoFingerTouchVisualizer.drawMidPoint(midPos, m_TouchPositions[0], m_TouchPositions[1]);
        m_RecentMidPos = midPos;
        m_RecentFingersDistance = m_TouchPositions[0].distance(m_TouchPositions[1]);
    }
}

void ByeWorld::touchesMoved(const std::vector<cocos2d::Touch *> &touches, cocos2d::Event *event)
{
    for (auto &touch : touches) {
        int touchId = touch->getID();
        if (touchId > 1) {
            continue;
        }
        auto pos = touch->getLocation();
        m_TwoFingerTouchVisualizer.clearCircle(touchId);
        m_TwoFingerTouchVisualizer.drawCircle(pos, touchId);
        m_TouchPositions[touchId] = pos;
    }
    
    if (m_TouchIDs.at(0) && m_TouchIDs.at(1)) {
        Vec2 midPos = getMidPos(m_TouchPositions[0], m_TouchPositions[1]);
        m_TwoFingerTouchVisualizer.clearMidPoint();
        m_TwoFingerTouchVisualizer.drawMidPoint(midPos, m_TouchPositions[0], m_TouchPositions[1]);
        
        auto midPosDelta = midPos - m_RecentMidPos;
        auto recentCamPos = m_MainCamera->getPosition3D();
        recentCamPos.x -= midPosDelta.x;
        recentCamPos.y -= midPosDelta.y;
        
        m_MainCamera->setPosition3D(recentCamPos);
        m_RecentMidPos = midPos;
        
        float curFingersDistance = m_TouchPositions[0].distance(m_TouchPositions[1]);
        auto deltaRatio = (curFingersDistance - m_RecentFingersDistance) / m_RecentFingersDistance;
        m_RecentFingersDistance = curFingersDistance;
        log("deltaRatio: %f", deltaRatio);
        auto targetPos = event->getCurrentTarget()->convertToNodeSpace(midPos);
        Vec3 nextPos = lerp(m_MainCamera->getPosition3D(), Vec3(targetPos.x, targetPos.y, 0), deltaRatio);
        if (nextPos.z > m_MaxZ || nextPos.z < m_MinZ) {
            return;
        }
        log("z: %f", nextPos.z);
        m_MainCamera->setPosition3D(nextPos);
    }
}

void ByeWorld::touchesEnded(const std::vector<cocos2d::Touch *> &touches, cocos2d::Event *event)
{
    for (auto &touch : touches) {
        int touchId = touch->getID();
        if (touchId > 1) {
            continue;
        }
        m_TouchIDs[touchId] = false;
        m_TwoFingerTouchVisualizer.clearCircle(touchId);
    }
    m_TwoFingerTouchVisualizer.clearMidPoint();
}