//
//  ZoomerScene.h
//  Zoomer
//
//  Created by Kular on 2/8/15.
//
//

#ifndef __ZOOMER__ZOOMERSCENE__
#define __ZOOMER__ZOOMERSCENE__

#ifdef COCOS2D_DEBUG
#include "TwoFingerTouchVisualizer.h"
#endif

#include <cocos2d.h>

#define MAX_TOUCHES 2
#define FIELD_OF_VIEW 60.f
#define NEAR 1.f
#define FAR 9999.f
#define MAX_ZOOM 2.5f
#define ZOOM_OUT_SPEED 2.f

namespace Lx {
    class ZoomerScene: public cocos2d::Scene
    {
        
    public:
        
        CREATE_FUNC(ZoomerScene);
        void initWithVisibleSizeWorldLayer(const cocos2d::Size &visibleSize, cocos2d::Layer *worldLayer);
        virtual ~ZoomerScene();
        inline void addChildToWorldLayer(cocos2d::Node *node);
        inline void addChildToWorldLayer(cocos2d::Node *node, const int &zOrder, const int &tag);
        
    private:
        
        void initMainCamera();
        void initTouchIdsPositions();
        inline cocos2d::Vec2 getMidPos(const cocos2d::Vec2 &pos1, const cocos2d::Vec2 &pos2) const;
        inline cocos2d::Vec3 lerp(const cocos2d::Vec3 &from, const cocos2d::Vec3 &to, const float &t) const;
        cocos2d::Size getCurrentCrossSectionSize() const;
        cocos2d::Size getHalfCrossSectionSize(const float &z = -1) const;
        cocos2d::Vec2 convertLocalPosToWorldSpace(const cocos2d::Vec2 &pos) const;
        int getCameraHitBoundary(const cocos2d::Vec3 &nextPos) const;
        
        void touchesBegan(const std::vector<cocos2d::Touch *> &touches, cocos2d::Event *event);
        void touchesMoved(const std::vector<cocos2d::Touch *> &touches, cocos2d::Event *event);
        void touchesEnded(const std::vector<cocos2d::Touch *> &touches, cocos2d::Event *event);
        
        // perspective camera settings
        float m_TangentHalfFov;
        float m_Fov;
        float m_Aspect;
        float m_Near, m_Far;
        
        float m_MaxZ; // MaxZ stands for Min Zoom
        float m_MinZ; // MinZ stands for Max Zoom
        float m_NormalZ; // NormalZ stands for Pixel-perfect Zoom
        float m_RecentFingersDistance;
        
        std::map<int, bool> m_TouchIDs;
        std::map<int, cocos2d::Vec2> m_TouchPositions;
        
        cocos2d::Camera *m_MainCamera;
        cocos2d::Vec2 m_RecentMidPos;
        cocos2d::Vec2 m_RecentTargetPos;

        cocos2d::Layer *m_WorldLayer;
        cocos2d::Size m_VisibleSize;
        cocos2d::Size m_WorldSize;
      
    #ifdef COCOS2D_DEBUG
        TwoFingerTouchVisualizer m_TwoFingerTouchVisualizer;
    #endif
    };

}



#endif /* defined(__ZOOMER__ZOOMERSCENE__) */
