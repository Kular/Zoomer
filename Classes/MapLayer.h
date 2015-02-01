//
//  MapLayer.h
//  Zoomer
//
//  Created by Kular on 2/1/15.
//
//

#ifndef __ZOOMER__MAPLAYER__
#define __ZOOMER__MAPLAYER__

#include <cocos2d.h>

class MapLayer : public cocos2d::Layer
{
public:
    CREATE_FUNC(MapLayer);
    bool initWithVisibleSize(const cocos2d::Size &visibleSize);
    cocos2d::Size getMapSize() const;
    
private:
    cocos2d::Sprite *m_Map;
    
};

#endif /* defined(__ZOOMER__MAPLAYER__) */
