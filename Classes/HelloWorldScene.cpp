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
    maxScale = 1.5f;
    minScale = MAX(visibleSize.width / size.width, visibleSize.height / size.height);
    log("minScale: %f", minScale);
    
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
            continue;
        }
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
    }
    
    if (touchIDs.at(0) && touchIDs.at(1)) {
        Vec2 midPos = getMidPos(touchPositions[0], touchPositions[1]);
        drawNodeForMidPoint->drawLine(touchPositions[0], touchPositions[1], Color4F::BLUE);
        drawNodeForMidPoint->drawPoint(midPos, 30.f, Color4F::RED);
        
        auto target = event->getCurrentTarget();
        if (target->getTag() != MAP_TAG) {
            return;
        }
        recentMidPos = midPos;
        auto locationInNode = target->convertToNodeSpace(recentMidPos);
        setCurAnchorWithLocalPos(locationInNode, mapSprite);
        recentAnchor = mapSprite->getAnchorPoint();
        recentFingersDistance = touchPositions[0].distance(touchPositions[1]);
    }
    
}

void HelloWorld::touchesMoved(const std::vector<cocos2d::Touch *> &touches, cocos2d::Event *event)
{
    for (auto &touch : touches) {
        int touchId = touch->getID();
        if (touchId > 1) {
            continue;
        }
        auto pos = touch->getLocation();
        fingerDrawNodes[touchId]->clear();
        fingerDrawNodes[touchId]->drawCircle(pos, 140, 360, 20, false, 1, 1, Color4F::YELLOW);
        
        touchPositions[touchId] = pos;
    }
    
    if (touchIDs.at(0) && touchIDs.at(1)) {
        Vec2 midPos = getMidPos(touchPositions[0], touchPositions[1]);
        drawNodeForMidPoint->clear();
        drawNodeForMidPoint->drawLine(touchPositions[0], touchPositions[1], Color4F::BLUE);
        drawNodeForMidPoint->drawPoint(midPos, 30.f, Color4F::RED);
        
        auto midPosDelta = midPos - recentMidPos;
        auto mapCurPos = mapSprite->getPosition();
        mapCurPos.x += midPosDelta.x;
        mapCurPos.y += midPosDelta.y;
        recentMidPos = midPos;
        
        switch (reachScreenBoundary(mapSprite, mapCurPos)) {
            case 0:
                mapSprite->setPosition(mapCurPos);
                break;
            // hit left or right
            case 1:
                mapSprite->setPositionY(mapCurPos.y);
                setCurAnchorWithLocalPos(event->getCurrentTarget()->convertToNodeSpace(recentMidPos), mapSprite);
                recentAnchor = mapSprite->getAnchorPoint();
                break;
            // hit top or bottom
            case 2:
                mapSprite->setPositionX(mapCurPos.x);
                setCurAnchorWithLocalPos(event->getCurrentTarget()->convertToNodeSpace(recentMidPos), mapSprite);
                recentAnchor = mapSprite->getAnchorPoint();
                break;
            default:
                setCurAnchorWithLocalPos(event->getCurrentTarget()->convertToNodeSpace(recentMidPos), mapSprite);
                recentAnchor = mapSprite->getAnchorPoint();
                break;
        }
        
        auto mapCurScale = mapSprite->getScale();
        float curFingersDistance = touchPositions[0].distance(touchPositions[1]);
        auto deltaRatio = (curFingersDistance - recentFingersDistance) / recentFingersDistance;
        mapCurScale += deltaRatio;
        
        // when scaling up
        if (deltaRatio > 0 && mapCurScale < maxScale) {
            setCurAnchor(recentAnchor, mapSprite);
            mapSprite->setScale(mapCurScale);
        }
        
        // when scaling down
        if (deltaRatio < 0 && mapCurScale > minScale) {
            Vec2 anchor;
            switch (reachBoundary(mapSprite, mapCurScale)) {
                case 0:
                    mapSprite->setScale(mapCurScale);
                    break;
                // left
                case 1:
                    anchor = mapSprite->getAnchorPoint();
                    setCurAnchor(Vec2(0, anchor.y), mapSprite);
                    mapSprite->setScale(mapCurScale);
                    break;
                // right
                case 2:
                    anchor = mapSprite->getAnchorPoint();
                    setCurAnchor(Vec2(1, anchor.y), mapSprite);
                    mapSprite->setScale(mapCurScale);
                    break;
                // top
                case 4:
                    anchor = mapSprite->getAnchorPoint();
                    setCurAnchor(Vec2(anchor.x, 1), mapSprite);
                    mapSprite->setScale(mapCurScale);
                    break;
                // bottom
                case 8:
                    anchor = mapSprite->getAnchorPoint();
                    setCurAnchor(Vec2(anchor.x, 0), mapSprite);
                    mapSprite->setScale(mapCurScale);
                    break;
                // left-top
                case 5:
                    setCurAnchor(Vec2(0, 1), mapSprite);
                    mapSprite->setScale(mapCurScale);
                    break;
                // right-top
                case 6:
                    setCurAnchor(Vec2(1, 1), mapSprite);
                    mapSprite->setScale(mapCurScale);
                    break;
                // left-bottom
                case 9:
                    setCurAnchor(Vec2::ZERO, mapSprite);
                    mapSprite->setScale(mapCurScale);
                    break;
                // right-bottom
                case 10:
                    setCurAnchor(Vec2(1, 0), mapSprite);
                    mapSprite->setScale(mapCurScale);
                    break;
                default:
                    break;
            }
            
        }
        recentFingersDistance = curFingersDistance;
    }

}

void HelloWorld::touchesEnded(const std::vector<cocos2d::Touch *> &touches, cocos2d::Event *event)
{
    for (auto &touch : touches) {
        int touchId = touch->getID();
        if (touchId > 1) {
            continue;
        }
        auto iter = touchIDs.find(touchId);
        if (iter != touchIDs.end()) {
            iter->second = false;
        }
        fingerDrawNodes[touchId]->clear();
    }
    drawNodeForMidPoint->clear();
}

Size HelloWorld::getCurrentSize(const cocos2d::Node *node)
{
    auto curScale = node->getScale();
    auto contentSize = node->getContentSize();
    return Size(contentSize.width * curScale, contentSize.height * curScale);
}

void HelloWorld::setCurAnchorWithLocalPos(const cocos2d::Vec2 &localPos, cocos2d::Node *node)
{
    auto contentSize = node->getContentSize();
    auto curAnchor = Vec2(localPos.x / contentSize.width, localPos.y / contentSize.height);
    
    setCurAnchor(curAnchor, node);
}

void HelloWorld::setCurAnchor(const cocos2d::Vec2 &curAnchor, cocos2d::Node *node) {
    auto curSize = getCurrentSize(node);
    auto delta = curAnchor - node->getAnchorPoint();
    auto deltaDistance = Vec2(delta.x * curSize.width, delta.y * curSize.height);
    node->setAnchorPoint(curAnchor);
    auto newPos = node->getPosition() + deltaDistance;
    node->setPosition(newPos);
}


int HelloWorld::reachScreenBoundary(const cocos2d::Node *node, const cocos2d::Vec2 &nextPos)
{
    auto curSize = getCurrentSize(node);
    auto curAnchor = node->getAnchorPoint();
    float leftSelf = curAnchor.x * curSize.width;
    float rightSelf = (1- curAnchor.x) * curSize.width;
    float topSelf = (1 - curAnchor.y) * curSize.height;
    float bottomSelf = curAnchor.y * curSize.height;
    
    auto winSize = Director::getInstance()->getWinSize();
    
    float leftScreen = nextPos.x;
    float rightScreen = winSize.width - nextPos.x;
    float topScreen = winSize.height - nextPos.y;
    float bottomScreen = nextPos.y;
    
    int retval = 0;
    
    if ((leftScreen - leftSelf) >= 0 || (rightScreen - rightSelf) >= 0) {
        retval += 1;
    }
    if ((bottomScreen - bottomSelf) >= 0 || (topScreen - topSelf) >= 0) {
        retval += 2;
    }
    
    return retval;
}

bool HelloWorld::reachScreenBoundary(const cocos2d::Node *node, const float &nextScale)
{
    auto contentSize = node->getContentSize();
    auto nextSize = Size(nextScale * contentSize.width, nextScale * contentSize.height);
    
    auto curAnchor = node->getAnchorPoint();
    float leftSelf = curAnchor.x * nextSize.width;
    float rightSelf = (1- curAnchor.x) * nextSize.width;
    float topSelf = (1 - curAnchor.y) * nextSize.height;
    float bottomSelf = curAnchor.y * nextSize.height;
    
    auto pos = node->getPosition();
    auto winSize = Director::getInstance()->getWinSize();
    
    float leftScreen = pos.x;
    float rightScreen = winSize.width - pos.x;
    float topScreen = winSize.height - pos.y;
    float bottomScreen = pos.y;
    
    if ((leftScreen - leftSelf) > 0 || (rightScreen - rightSelf) > 0 || (bottomScreen - bottomSelf) > 0 || (topScreen - topSelf) > 0) {
        return true;
    } else {
        return false;
    }
}

int HelloWorld::reachBoundary(const cocos2d::Node *node, const float &nextScale)
{
    int retval = 0;
    auto contentSize = node->getContentSize();
    auto nextSize = Size(nextScale * contentSize.width, nextScale * contentSize.height);
    
    auto curAnchor = node->getAnchorPoint();
    float leftSelf = curAnchor.x * nextSize.width;
    float rightSelf = (1- curAnchor.x) * nextSize.width;
    float topSelf = (1 - curAnchor.y) * nextSize.height;
    float bottomSelf = curAnchor.y * nextSize.height;
    
    auto pos = node->getPosition();
    auto winSize = Director::getInstance()->getWinSize();
    
    float leftScreen = pos.x;
    float rightScreen = winSize.width - pos.x;
    float topScreen = winSize.height - pos.y;
    float bottomScreen = pos.y;
    
    if (leftScreen - leftSelf >= 0) {
        retval += 1;
    }
    if (rightScreen - rightSelf >= 0) {
        retval += 2;
    }
    if (topScreen - topSelf >= 0) {
        retval += 4;
    }
    if (bottomScreen - bottomSelf >= 0) {
        retval += 8;
    }

    return retval;
}

Vec2 HelloWorld::getMidPos(const cocos2d::Vec2 &pos1, const cocos2d::Vec2 &pos2)
{
    return (pos1 + pos2) / 2;
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
