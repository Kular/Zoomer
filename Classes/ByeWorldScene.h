//
//  ByeWorldScene.h
//  Zoomer
//
//  Created by Kular on 2/1/15.
//
//

#ifndef __ZOOMER__BYEWORLDSCENE__
#define __ZOOMER__BYEWORLDSCENE__

#include <iostream>
#include <cocos2d.h>
#include "TwoFingerTouchVisualizer.h"


#define MAX_TOUCHES 2
#define FIELD_OF_VIEW 60.f
#define NEAR 1.f
#define FAR 9999.f
#define MAX_ZOOM 2.5f
#define ZOOM_OUT_SPEED 2.f

class ByeWorld : public cocos2d::Scene
{
public:
    CREATE_FUNC(ByeWorld);
    void initWithVisibleSize(const cocos2d::Size &visibleSize);
    
private:
    
    void initMainCamera(const cocos2d::Size &visibleSize, const cocos2d::Size &worldSize);
    void initTouchIdsPositions();
    float deltaScaleToDeltaZ(const float &deltaScale);
    cocos2d::Vec2 getMidPos(const cocos2d::Vec2 &pos1, const cocos2d::Vec2 &pos2);
    cocos2d::Vec3 lerp(const cocos2d::Vec3 &from, const cocos2d::Vec3 &to, float t);
    cocos2d::Size getCurrentCrossSectionSize() const;
    cocos2d::Size getHalfCrossSectionSize(const float &z = -1) const;
    int getCameraHitBoundary(const cocos2d::Vec3 &nextPos) const;
    cocos2d::Vec2 convertLocalPosToWorldSpace(const cocos2d::Vec2 &pos) const;
    
    void touchesBegan(const std::vector<cocos2d::Touch *> &touches, cocos2d::Event *event);
    void touchesMoved(const std::vector<cocos2d::Touch *> &touches, cocos2d::Event *event);
    void touchesEnded(const std::vector<cocos2d::Touch *> &touches, cocos2d::Event *event);
    
    // perspective camera settings
    float m_mTangentHalfFov;
    float m_Fov;
    float m_Aspect;
    float m_Near, m_Far;
    
    float m_MaxZ; // MaxZ stands for Min Zoom
    float m_MinZ; // MinZ stands for Max Zoom
    float m_NormalZ; // NormalZ stands for Pixel-perfect Zoom
    float m_RecentZ;
    float m_RecentFingersDistance;
    
    std::map<int, bool> m_TouchIDs;
    std::map<int, cocos2d::Vec2> m_TouchPositions;
    
    cocos2d::Camera *m_MainCamera;
    cocos2d::Vec2 m_RecentMidPos;
    cocos2d::Vec2 m_RecentTargetPos;
    
    cocos2d::Size m_VisibleSize;
    cocos2d::Size m_WorldSize;
    
    TwoFingerTouchVisualizer m_TwoFingerTouchVisualizer;
};





#endif /* defined(__ZOOMER__BYEWORLDSCENE__) */
