//
//  ZoomerScene.cpp
//  Zoomer
//
//  Created by Kular on 2/8/15.
//
//

#include "ZoomerScene.h"

using namespace Lx;
USING_NS_CC;

void ZoomerScene::initWithVisibleSizeWorldLayer(const cocos2d::Size &visibleSize, cocos2d::Layer *worldLayer)
{
    m_VisibleSize = visibleSize;
    m_Fov = FIELD_OF_VIEW;
    m_TangentHalfFov = tanf(m_Fov / 2 * M_PI / 180);
    m_Aspect = visibleSize.width / visibleSize.height;
    m_Near = NEAR;
    m_Far = FAR;
    
    m_WorldLayer = worldLayer;
    m_WorldLayer->setAnchorPoint(Vec2::ZERO);
    m_WorldLayer->setPosition(Vec2::ZERO);
    m_WorldSize = m_WorldLayer->getChildren().at(0)->getContentSize();
    m_WorldLayer->setCameraMask(2);
    this->addChild(m_WorldLayer);
    
    initTouchIdsPositions();
    
    m_MainCamera = Camera::createPerspective(m_Fov, m_Aspect, m_Near, m_Far);
    initMainCamera();
    this->addChild(m_MainCamera);

#ifdef COCOS2D_DEBUG
    this->addChild(&m_TwoFingerTouchVisualizer);
#endif
    
    auto eventDispatcher = Director::getInstance()->getEventDispatcher();
    auto multiTouchEventListener = EventListenerTouchAllAtOnce::create();
    multiTouchEventListener->onTouchesBegan = CC_CALLBACK_2(ZoomerScene::touchesBegan, this);
    multiTouchEventListener->onTouchesMoved = CC_CALLBACK_2(ZoomerScene::touchesMoved, this);
    multiTouchEventListener->onTouchesEnded = CC_CALLBACK_2(ZoomerScene::touchesEnded, this);
    eventDispatcher->addEventListenerWithSceneGraphPriority(multiTouchEventListener, this);
}

ZoomerScene::~ZoomerScene()
{
    // Destructor
}

inline void ZoomerScene::addChildToWorldLayer(cocos2d::Node *node)
{
    m_WorldLayer->addChild(node);
}

inline void ZoomerScene::addChildToWorldLayer(cocos2d::Node *node, const int &zOrder, const int &tag)
{
    m_WorldLayer->addChild(node, zOrder, tag);
}

void ZoomerScene::initMainCamera()
{
    m_MainCamera->setCameraFlag(CameraFlag::USER1);
    auto widthRatio = m_VisibleSize.width / m_WorldSize.width;
    auto heightRatio = m_VisibleSize.height / m_WorldSize.height;
    m_NormalZ = (m_VisibleSize.height / 2) / (m_TangentHalfFov);
    
    if (widthRatio >= heightRatio) {
        // should adjust width of camera's view to the width of map
        auto heightOfView = m_WorldSize.width / m_Aspect;
        m_MaxZ = (heightOfView / 2) / m_TangentHalfFov;
    } else {
        // should adjust height of camera's view to the height of map
        m_MaxZ = (m_WorldSize.height / 2) / m_TangentHalfFov;
    }
    
    m_MinZ = m_NormalZ / MAX_ZOOM;
    
    m_MainCamera->setPosition3D(Vec3(m_WorldSize.width / 2, m_WorldSize.height / 2, m_MaxZ));
    m_MainCamera->lookAt(Vec3(m_WorldSize.width / 2, m_WorldSize.height / 2, 0), Vec3(0, 1, 0));
}

void ZoomerScene::initTouchIdsPositions()
{
    m_TouchIDs = std::map<int, bool>();
    m_TouchPositions = std::map<int, cocos2d::Vec2>();
    for (int i = 0; i < MAX_TOUCHES; i++) {
        m_TouchIDs[i] = false;
        m_TouchPositions[i] = Vec2::ZERO;
    }
}

inline Vec2 ZoomerScene::getMidPos(const cocos2d::Vec2 &pos1, const cocos2d::Vec2 &pos2) const
{
    return (pos1 + pos2) / 2;
}

inline Vec3 ZoomerScene::lerp(const cocos2d::Vec3 &from, const cocos2d::Vec3 &to, const float &t) const
{
    return from + t * (to - from);
}

Size ZoomerScene::getCurrentCrossSectionSize() const
{
    auto halfSize = getHalfCrossSectionSize();
    return Size(halfSize.width * 2, halfSize.height * 2);
}

Size ZoomerScene::getHalfCrossSectionSize(const float &z) const
{
    auto curZ = (z == -1) ? m_MainCamera->getPosition3D().z : z;
    auto halfCrossSectionHeight = curZ * m_TangentHalfFov;
    return Size(halfCrossSectionHeight * m_Aspect, halfCrossSectionHeight);
}

Vec2 ZoomerScene::convertLocalPosToWorldSpace(const cocos2d::Vec2 &pos) const
{
    auto curCrossSectionSize = getCurrentCrossSectionSize();
    auto posInCrossSection = Vec2(curCrossSectionSize.width * pos.x / m_VisibleSize.width, curCrossSectionSize.height * pos.y / m_VisibleSize.height);
    auto delta = posInCrossSection - Vec2(curCrossSectionSize.width / 2, curCrossSectionSize.height / 2);
    return m_MainCamera->getPosition() + delta;
}

int ZoomerScene::getCameraHitBoundary(const cocos2d::Vec3 &nextPos) const
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

void ZoomerScene::touchesEnded(const std::vector<cocos2d::Touch *> &touches, cocos2d::Event *event)
{
    for (auto &touch : touches) {
        int touchId = touch->getID();
        if (touchId > 1) {
            continue;
        }
        m_TouchIDs[touchId] = false;
#ifdef COCOS2D_DEBUG
        m_TwoFingerTouchVisualizer.clearCircle(touchId);
#endif
    }
#ifdef COCOS2D_DEBUG
    m_TwoFingerTouchVisualizer.clearMidPoint();
#endif
}

void ZoomerScene::touchesBegan(const std::vector<cocos2d::Touch *> &touches, cocos2d::Event *event)
{
    for (auto &touch : touches) {
        int touchId = touch->getID();
        if (touchId > 1) {
            continue;
        }
        auto pos = touch->getLocation();
#ifdef COCOS2D_DEBUG
        m_TwoFingerTouchVisualizer.drawCircle(pos, touchId);
#endif
        
        m_TouchIDs[touchId] = true;
        
        m_TouchPositions[touchId] = pos;
    }
    
    if (m_TouchIDs.at(0) && m_TouchIDs.at(1)) {
        Vec2 midPos = getMidPos(m_TouchPositions[0], m_TouchPositions[1]);
#ifdef COCOS2D_DEBUG
        m_TwoFingerTouchVisualizer.drawMidPoint(midPos, m_TouchPositions[0], m_TouchPositions[1]);
#endif
        m_RecentMidPos = midPos;
        m_RecentTargetPos = convertLocalPosToWorldSpace(m_RecentMidPos);
        
        m_RecentFingersDistance = m_TouchPositions[0].distance(m_TouchPositions[1]);
    }
}

void ZoomerScene::touchesMoved(const std::vector<cocos2d::Touch *> &touches, cocos2d::Event *event)
{
    for (auto &touch : touches) {
        int touchId = touch->getID();
        if (touchId > 1) {
            continue;
        }
        auto pos = touch->getLocation();
        
#ifdef COCOS2D_DEBUG
        m_TwoFingerTouchVisualizer.clearCircle(touchId);
        m_TwoFingerTouchVisualizer.drawCircle(pos, touchId);
#endif
        m_TouchPositions[touchId] = pos;
    }
    
    if (m_TouchIDs.at(0) && m_TouchIDs.at(1)) {
        Vec2 midPos = getMidPos(m_TouchPositions[0], m_TouchPositions[1]);
        
#ifdef COCOS2D_DEBUG
        m_TwoFingerTouchVisualizer.clearMidPoint();
        m_TwoFingerTouchVisualizer.drawMidPoint(midPos, m_TouchPositions[0], m_TouchPositions[1]);
#endif
        
        // moving
        auto midPosDelta = convertLocalPosToWorldSpace(midPos) - convertLocalPosToWorldSpace(m_RecentMidPos);
        auto recentCamPos = m_MainCamera->getPosition3D();
        recentCamPos.x -= midPosDelta.x;
        recentCamPos.y -= midPosDelta.y;
        
        auto hitBoundary = getCameraHitBoundary(recentCamPos);
        if (hitBoundary == 0) {
            m_MainCamera->setPosition3D(recentCamPos);
        } else if (hitBoundary == 1 || hitBoundary == 2 || hitBoundary == 3) {
            m_MainCamera->setPositionY(recentCamPos.y);
            m_RecentTargetPos = convertLocalPosToWorldSpace(midPos);
        } else if (hitBoundary == 4 || hitBoundary == 8 || hitBoundary == 12) {
            m_MainCamera->setPositionX(recentCamPos.x);
            m_RecentTargetPos = convertLocalPosToWorldSpace(midPos);
        } else {
            m_RecentTargetPos = convertLocalPosToWorldSpace(midPos);
        }
        m_RecentMidPos = midPos;
        
        // zooming-in/out
        float curFingersDistance = m_TouchPositions[0].distance(m_TouchPositions[1]);
        auto deltaRatio = (curFingersDistance - m_RecentFingersDistance) / m_RecentFingersDistance;
        m_RecentFingersDistance = curFingersDistance;
        
        if (deltaRatio == 0) {
            return;
        }
        
        Vec3 nextPos = lerp(m_MainCamera->getPosition3D(), Vec3(m_RecentTargetPos.x, m_RecentTargetPos.y, 0), deltaRatio);
        
        // zooming-in
        if (deltaRatio > 0) {
            if (nextPos.z >= m_MinZ) {
                m_MainCamera->setPosition3D(nextPos);
            }
            return;
        }
        
        // zooming-out
        if (nextPos.z > m_MaxZ) {
            m_MainCamera->setPositionX(m_WorldSize.width / 2);
            return;
        }
        
        hitBoundary = getCameraHitBoundary(nextPos);
        Vec3 tmpTargetPos = Vec3::ZERO;
        deltaRatio *= ZOOM_OUT_SPEED;
        switch (hitBoundary) {
            case 0:
                m_MainCamera->setPosition3D(nextPos);
                break;
            case 1:
                tmpTargetPos.x = 0;
                tmpTargetPos.y = m_RecentTargetPos.y;
                nextPos = lerp(m_MainCamera->getPosition3D(), tmpTargetPos, deltaRatio);
                if (nextPos.z > m_MaxZ) {
                    break;
                }
                m_MainCamera->setPosition3D(nextPos);
                break;
            case 2:
                tmpTargetPos.x = m_WorldSize.width;
                tmpTargetPos.y = m_RecentTargetPos.y;
                nextPos = lerp(m_MainCamera->getPosition3D(), tmpTargetPos, deltaRatio);
                if (nextPos.z > m_MaxZ) {
                    break;
                }
                m_MainCamera->setPosition3D(nextPos);
                break;
            case 4:
                tmpTargetPos.x = m_RecentTargetPos.x;
                tmpTargetPos.y = m_WorldSize.height;
                nextPos = lerp(m_MainCamera->getPosition3D(), tmpTargetPos, deltaRatio);
                if (nextPos.z > m_MaxZ) {
                    break;
                }
                m_MainCamera->setPosition3D(nextPos);
                break;
            case 8:
                tmpTargetPos.x = m_RecentTargetPos.x;
                tmpTargetPos.y = 0;
                nextPos = lerp(m_MainCamera->getPosition3D(), tmpTargetPos, deltaRatio);
                if (nextPos.z > m_MaxZ) {
                    break;
                }
                m_MainCamera->setPosition3D(nextPos);
                break;
            case 5:
                tmpTargetPos.x = 0;
                tmpTargetPos.y = m_WorldSize.height;
                nextPos = lerp(m_MainCamera->getPosition3D(), tmpTargetPos, deltaRatio);
                if (nextPos.z > m_MaxZ) {
                    break;
                }
                m_MainCamera->setPosition3D(nextPos);
                break;
            case 6:
                tmpTargetPos.x = m_WorldSize.width;
                tmpTargetPos.y = m_WorldSize.height;
                nextPos = lerp(m_MainCamera->getPosition3D(), tmpTargetPos, deltaRatio);
                if (nextPos.z > m_MaxZ) {
                    break;
                }
                m_MainCamera->setPosition3D(nextPos);
                break;
            case 10:
                tmpTargetPos.x = m_WorldSize.width;
                tmpTargetPos.y = 0;
                nextPos = lerp(m_MainCamera->getPosition3D(), tmpTargetPos, deltaRatio);
                if (nextPos.z > m_MaxZ) {
                    break;
                }
                m_MainCamera->setPosition3D(nextPos);
                break;
            case 9:
                tmpTargetPos.x = 0;
                tmpTargetPos.y = 0;
                nextPos = lerp(m_MainCamera->getPosition3D(), tmpTargetPos, deltaRatio);
                if (nextPos.z > m_MaxZ) {
                    break;
                }
                m_MainCamera->setPosition3D(nextPos);
                break;
            default:
                break;
        }
        
    }

}
