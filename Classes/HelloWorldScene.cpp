#include "HelloWorldScene.h"

USING_NS_CC;

#define LAYER_TAG 1
#define MAP_TAG 2

Scene* HelloWorld::createScene()
{
    auto scene = Scene::create();

    auto layer = HelloWorld::create();

    scene->addChild(layer, 0, LAYER_TAG);

    return scene;
}

bool HelloWorld::init()
{
    if ( !Layer::init() )
    {
        return false;
    }
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));
    
	closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2 ,
                                origin.y + closeItem->getContentSize().height/2));

    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);
    
    mapSprite = Sprite::create("res/test_map_grid.png");
    mapSprite->setAnchorPoint(Vec2(0.5f, 0.5f));
    mapSprite->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
    auto size = mapSprite->getContentSize();
    log("map size: (%.0f, %.0f)", size.width, size.height);
    this->addChild(mapSprite, 0, MAP_TAG);
    
    touchIDs = std::map<int, bool>();
    touchIDs[0] = false;
    touchIDs[1] = false;
    
    touchPositions = std::map<int, Vec2>();
    touchPositions[0] = Vec2::ZERO;
    touchPositions[1] = Vec2::ZERO;
    
    
    drawNodeForFinger1 = DrawNode::create();
    drawNodeForFinger2 = DrawNode::create();
    drawNodeForMidPoint = DrawNode::create();
    this->addChild(drawNodeForFinger1, 10);
    this->addChild(drawNodeForFinger2, 10);
    this->addChild(drawNodeForMidPoint, 10);
    
    fingerDrawNodes = std::map<int, DrawNode*>();
    fingerDrawNodes[0] = drawNodeForFinger1;
    fingerDrawNodes[1] = drawNodeForFinger2;
    
    auto eventDispatcher = Director::getInstance()->getEventDispatcher();
    listener = EventListenerTouchAllAtOnce::create();
    listener->onTouchesBegan = CC_CALLBACK_2(HelloWorld::touchesBegan, this);
    listener->onTouchesMoved = CC_CALLBACK_2(HelloWorld::touchesMoved, this);
    listener->onTouchesEnded = CC_CALLBACK_2(HelloWorld::touchesEnded, this);
    
    eventDispatcher->addEventListenerWithSceneGraphPriority(listener, mapSprite);
    
    //    this->scheduleUpdate();
    return true;
}

void HelloWorld::touchesBegan(const std::vector<cocos2d::Touch *> &touches, cocos2d::Event *event)
{
    for (auto &touch : touches) {
        int touchId = touch->getID();
        if (touchId > 1) {
            break;
        }
        log("TouchID: %d", touchId);
        auto pos = touch->getLocation();
        fingerDrawNodes[touchId]->drawCircle(pos, 140, 360, 20, false, 1, 1, Color4F::YELLOW);
        
        auto iter1 = touchIDs.find(touchId);
        if (iter1 == touchIDs.end()) {
            touchIDs[touchId] = true;
        } else {
            iter1->second = true;
        }
        
        auto iter2 = touchPositions.find(touchId);
        if (iter2 == touchPositions.end()) {
            touchPositions[touchId] = pos;
        } else {
            iter2->second = pos;
        }

        /*
        log("Touch Pos: (%f, %f)", pos.x, pos.y);
        auto target = event->getCurrentTarget();
        if (target->getTag() != MAP_TAG) {
            continue;
        }
        auto locationInNode = target->convertToNodeSpace(pos);
        log("Pos in node: (%f, %f)", locationInNode.x, locationInNode.y);
        */
    }
    
    if (touches.size() == 2) {
        Vec2 midPos = getMidPos(touchPositions[0], touchPositions[1]);
        drawNodeForMidPoint->drawLine(touchPositions[0], touchPositions[1], Color4F::BLUE);
        drawNodeForMidPoint->drawPoint(midPos, 30.f, Color4F::RED);
    }
    
}

void HelloWorld::touchesMoved(const std::vector<cocos2d::Touch *> &touches, cocos2d::Event *event)
{
    for (auto &touch : touches) {
        int touchId = touch->getID();
        if (touchId > 1) {
            break;
        }
        if (touchIDs.at(0) && touchIDs.at(1)) {
            auto pos = touchPositions[touchId];
            auto delta = touch->getDelta();
            pos.x += delta.x;
            pos.y += delta.y;
            touchPositions[touchId] = pos;
            fingerDrawNodes[touchId]->clear();
            fingerDrawNodes[touchId]->drawCircle(pos, 140, 360, 20, false, 1, 1, Color4F::YELLOW);
        }
    }
    if (touches.size() == 2) {
        Vec2 midPos = getMidPos(touchPositions[0], touchPositions[1]);
        drawNodeForMidPoint->clear();
        drawNodeForMidPoint->drawLine(touchPositions[0], touchPositions[1], Color4F::BLUE);
        drawNodeForMidPoint->drawPoint(midPos, 30.f, Color4F::RED);
    }
}

void HelloWorld::touchesEnded(const std::vector<cocos2d::Touch *> &touches, cocos2d::Event *event)
{
    for (auto &touch : touches) {
        int touchId = touch->getID();
        if (touchId > 1) {
            break;
        }
        auto iter = touchIDs.find(touchId);
        if (iter != touchIDs.end()) {
            iter->second = false;
        }
        fingerDrawNodes[touchId]->clear();
    }
    drawNodeForMidPoint->clear();
}

Vec2 HelloWorld::getMidPos(const cocos2d::Vec2 &pos1, const cocos2d::Vec2 &pos2)
{
    return Vec2((pos1.x + pos2.x) / 2, (pos1.y + pos2.y) / 2);
}

void HelloWorld::update(float delta)
{
    auto anchor = mapSprite->getAnchorPoint();
    mapSprite->setAnchorPoint(Vec2(anchor.x + delta / 5, anchor.y + delta / 5));
}


void HelloWorld::menuCloseCallback(Ref* pSender)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
	MessageBox("You pressed the close button. Windows Store Apps do not implement a close button.","Alert");
    return;
#endif

    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}
