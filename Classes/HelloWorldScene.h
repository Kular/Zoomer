#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
#define SCALE_SPEED 0.6f
#define MAX_SCALE 2.5f
#define MIN_DISTANCE 300.f

class HelloWorld : public cocos2d::Layer
{
public:

    static cocos2d::Scene* createScene();

    virtual bool init();
    
    void menuCloseCallback(cocos2d::Ref* pSender);
    
    CREATE_FUNC(HelloWorld);
    
    cocos2d::Sprite *mapSprite;
    cocos2d::EventListenerTouchAllAtOnce *listener;
    
    void touchesBegan(const std::vector<cocos2d::Touch *> &touches, cocos2d::Event *event);
    void touchesMoved(const std::vector<cocos2d::Touch *> &touches, cocos2d::Event *event);
    void touchesEnded(const std::vector<cocos2d::Touch *> &touches, cocos2d::Event *event);
    
    std::map<int, bool> touchIDs;
    std::map<int, cocos2d::Vec2> touchPositions;
    
    cocos2d::Vec2 getMidPos(const cocos2d::Vec2 &pos1, const cocos2d::Vec2 &pos2);
    cocos2d::Vec2 recentMidPos;
    cocos2d::Vec2 recentAnchor;
    
    cocos2d::DrawNode *drawNodeForFinger1;
    cocos2d::DrawNode *drawNodeForFinger2;
    cocos2d::DrawNode *drawNodeForMidPoint;
    
    std::map<int, cocos2d::DrawNode*> fingerDrawNodes;
    
    void setCurAnchorWithLocalPos(const cocos2d::Vec2 &localPos, cocos2d::Node *node);
    void setCurAnchor(const cocos2d::Vec2 &curAnchor, cocos2d::Node *node);
    
    int reachScreenBoundary(const cocos2d::Node *node, const cocos2d::Vec2 &nextPos);
    bool reachScreenBoundary(const cocos2d::Node *node, const float &nextScale);
    int reachBoundary(const cocos2d::Node *node, const float &nextScale);

    float recentFingersDistance;
    
    cocos2d::Size getCurrentSize(const cocos2d::Node *node);
    
    float minScale, maxScale;
    
};

#endif // __HELLOWORLD_SCENE_H__
