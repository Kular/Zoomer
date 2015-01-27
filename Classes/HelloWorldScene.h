#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"

class HelloWorld : public cocos2d::Layer
{
public:

    static cocos2d::Scene* createScene();

    virtual bool init();
    
    void menuCloseCallback(cocos2d::Ref* pSender);
    
    void update(float delta);
    
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
    
    cocos2d::DrawNode *drawNodeForFinger1;
    cocos2d::DrawNode *drawNodeForFinger2;
    cocos2d::DrawNode *drawNodeForMidPoint;
    
    std::map<int, cocos2d::DrawNode*> fingerDrawNodes;
    
    void setCurAnchor(const cocos2d::Vec2 &localPos, cocos2d::Node *node);
    
    bool reachScreenBoundary(cocos2d::Vec2 &nextPos, cocos2d::Node *node);

    float recentFingersDistance;
    
    cocos2d::Size getCurrentSize(cocos2d::Node *node);
};

#endif // __HELLOWORLD_SCENE_H__
