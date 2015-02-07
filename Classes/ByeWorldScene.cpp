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


void ByeWorld::initWithVisibleSize(const Size &visibleSize)
{
    initTouchIdsPositions();
    this->addChild(&m_TwoFingerTouchVisualizer);
    
    auto worldLayer = MapLayer::create();
    worldLayer->initWithVisibleSize(visibleSize);
    worldLayer->setCameraMask(2);
    this->addChild(worldLayer);
    
    m_Fov = FIELD_OF_VIEW;
    m_mTangentHalfFov = tanf(m_Fov / 2 * M_PI / 180);
    m_Aspect = visibleSize.width / visibleSize.height;
    m_Near = NEAR;
    m_Far = FAR;
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
    m_WorldSize = worldSize;
    m_MainCamera->setCameraFlag(CameraFlag::USER1);
    auto widthRatio = visibleSize.width / worldSize.width;
    auto heightRatio = visibleSize.height / worldSize.height;
    m_NormalZ = (visibleSize.height / 2) / (m_mTangentHalfFov);
    
    if (widthRatio >= heightRatio) {
        // should adjust width of camera's view to the width of map
        auto heightOfView = worldSize.width / m_Aspect;
        m_MaxZ = (heightOfView / 2) / m_mTangentHalfFov;
    } else {
        // should adjust height of camera's view to the height of map
        m_MaxZ = (worldSize.height / 2) / m_mTangentHalfFov;
    }
    m_RecentZ = m_MaxZ;
    m_MinZ = 1.f/2.f * m_NormalZ;
    m_RecentScale = m_RecentZ / m_NormalZ;
    log("init scale: %f", m_RecentScale);
    m_MainCamera->setPosition3D(Vec3(worldSize.width / 2, worldSize.height / 2, m_RecentZ));
    m_MainCamera->lookAt(Vec3(worldSize.width / 2, worldSize.height / 2, 0), Vec3(0, 1, 0));
}

Vec2 ByeWorld::getMidPos(const cocos2d::Vec2 &pos1, const cocos2d::Vec2 &pos2)
{
    return (pos1 + pos2) / 2;
}

float ByeWorld::deltaScaleToDeltaZ(const float &deltaScale)
{
    return m_mTangentHalfFov * deltaScale;
}

Vec3 ByeWorld::lerp(const cocos2d::Vec3 &from, const cocos2d::Vec3 &to, float t)
{
    return from + t * (to - from);
}

Size ByeWorld::getCurrentCrossSectionSize() const
{
    auto halfSize = getHalfCrossSectionSize();
    return Size(halfSize.width / 2, halfSize.height / 2);
}

Size ByeWorld::getHalfCrossSectionSize(const float &z) const
{
    auto curZ = (z == -1) ? m_MainCamera->getPosition3D().z : z;
    auto halfCrossSectionHeight = curZ * m_mTangentHalfFov;
    return Size(halfCrossSectionHeight * m_Aspect, halfCrossSectionHeight);
}

int ByeWorld::getCameraHitBoundary(const cocos2d::Vec3 &nextPos) const
{
    int retval = 0;
    auto halfCurrentCrossSectionSize = getHalfCrossSectionSize(nextPos.z);
    
    // left
    if (nextPos.x < halfCurrentCrossSectionSize.width) {
        retval += 1;
    }
    // right
    if (m_WorldSize.width - nextPos.x < halfCurrentCrossSectionSize.width) {
        retval += 2;
    }
    
    // top
    if (m_WorldSize.height - nextPos.y < halfCurrentCrossSectionSize.height) {
        retval += 4;
    }
    
    // bottom
    if (nextPos.y < halfCurrentCrossSectionSize.height) {
        retval += 8;
    }
    
    return retval;
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
        
        // moving
        auto midPosDelta = midPos - m_RecentMidPos;
        auto recentCamPos = m_MainCamera->getPosition3D();
        recentCamPos.x -= midPosDelta.x;
        recentCamPos.y -= midPosDelta.y;

        auto hitBoundary = getCameraHitBoundary(recentCamPos);
        if (hitBoundary == 0) {
            m_MainCamera->setPosition3D(recentCamPos);
        } else if (hitBoundary == 1 || hitBoundary == 2 || hitBoundary == 3) {
            m_MainCamera->setPositionY(recentCamPos.y);
        } else if (hitBoundary == 4 || hitBoundary == 8 || hitBoundary == 12) {
            m_MainCamera->setPositionX(recentCamPos.x);
        }
        m_RecentMidPos = midPos;
        
        // zooming-in/out
        float curFingersDistance = m_TouchPositions[0].distance(m_TouchPositions[1]);
        auto deltaRatio = (curFingersDistance - m_RecentFingersDistance) / m_RecentFingersDistance;
        if (deltaRatio == 0) {
            return;
        }
        
        m_RecentFingersDistance = curFingersDistance;
        auto targetPos = event->getCurrentTarget()->convertToNodeSpace(midPos);
        Vec3 nextPos = lerp(m_MainCamera->getPosition3D(), Vec3(targetPos.x, targetPos.y, 0), deltaRatio);
        if (nextPos.z > m_MaxZ || nextPos.z < m_MinZ) {
            return;
        }
        
        if (deltaRatio >= 0) {
            m_MainCamera->setPosition3D(nextPos);
            return;
        }
        
        hitBoundary = getCameraHitBoundary(nextPos);
        Vec2 tmpTargetPos = Vec2::ZERO;
        switch (hitBoundary) {
            case 0:
                m_MainCamera->setPosition3D(nextPos);
                break;
            case 1:
                tmpTargetPos.x = 0;
                tmpTargetPos.y = targetPos.y;
                nextPos = lerp(m_MainCamera->getPosition3D(), Vec3(tmpTargetPos.x, tmpTargetPos.y, 0), deltaRatio);
                m_MainCamera->setPosition3D(nextPos);
                break;
            case 2:
                tmpTargetPos.x = m_WorldSize.width;
                tmpTargetPos.y = targetPos.y;
                nextPos = lerp(m_MainCamera->getPosition3D(), Vec3(tmpTargetPos.x, tmpTargetPos.y, 0), deltaRatio);
                m_MainCamera->setPosition3D(nextPos);
                break;
            case 4:
                tmpTargetPos.x = targetPos.x;
                tmpTargetPos.y = m_WorldSize.height;
                nextPos = lerp(m_MainCamera->getPosition3D(), Vec3(tmpTargetPos.x, tmpTargetPos.y, 0), deltaRatio);
                m_MainCamera->setPosition3D(nextPos);
                break;
            case 8:
                tmpTargetPos.x = targetPos.x;
                tmpTargetPos.y = 0;
                nextPos = lerp(m_MainCamera->getPosition3D(), Vec3(tmpTargetPos.x, tmpTargetPos.y, 0), deltaRatio);
                m_MainCamera->setPosition3D(nextPos);
                break;
            case 5:
                tmpTargetPos.x = 0;
                tmpTargetPos.y = m_WorldSize.height;
                nextPos = lerp(m_MainCamera->getPosition3D(), Vec3(tmpTargetPos.x, tmpTargetPos.y, 0), deltaRatio);
                m_MainCamera->setPosition3D(nextPos);
                break;
            case 6:
                tmpTargetPos.x = m_WorldSize.width;
                tmpTargetPos.y = m_WorldSize.height;
                nextPos = lerp(m_MainCamera->getPosition3D(), Vec3(tmpTargetPos.x, tmpTargetPos.y, 0), deltaRatio);
                m_MainCamera->setPosition3D(nextPos);
                break;
            case 10:
                tmpTargetPos.x = m_WorldSize.width;
                tmpTargetPos.y = 0;
                nextPos = lerp(m_MainCamera->getPosition3D(), Vec3(tmpTargetPos.x, tmpTargetPos.y, 0), deltaRatio);
                m_MainCamera->setPosition3D(nextPos);
                break;
            case 9:
                tmpTargetPos.x = 0;
                tmpTargetPos.y = 0;
                nextPos = lerp(m_MainCamera->getPosition3D(), Vec3(tmpTargetPos.x, tmpTargetPos.y, 0), deltaRatio);
                m_MainCamera->setPosition3D(nextPos);
                break;
            default:
                break;
        }

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