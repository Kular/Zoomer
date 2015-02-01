//
//  TwoFingerTouchVisualizer.h
//  Zoomer
//
//  Created by Kular on 2/1/15.
//
//

#ifndef __ZOOMER__TWOFINGERTOUCHVISUALIZER__
#define __ZOOMER__TWOFINGERTOUCHVISUALIZER__

#include <cocos2d.h>
class TwoFingerTouchVisualizer : public cocos2d::Node
{
public:
    TwoFingerTouchVisualizer();
    virtual ~TwoFingerTouchVisualizer();
    
    void drawCircle(const cocos2d::Vec2 &pos, const int &index);
    void drawMidPoint(const cocos2d::Vec2 &midPos, const cocos2d::Vec2 &pos1, const cocos2d::Vec2 &pos2);
    void clearCircle(const int &index);
    void clearMidPoint();
    
private:
    std::map<int, cocos2d::DrawNode *> m_Circles;
    cocos2d::DrawNode *m_MidPoint;
};

#endif /* defined(__Zoomer__TwoFingerTouchVisualizer__) */
