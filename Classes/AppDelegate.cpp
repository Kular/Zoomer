#include "AppDelegate.h"
#include "HelloWorldScene.h"

USING_NS_CC;

AppDelegate::AppDelegate() {

}

AppDelegate::~AppDelegate() 
{
}

void AppDelegate::initGLContextAttrs()
{
    //set OpenGL context attributions,now can only set six attributions:
    //red,green,blue,alpha,depth,stencil
    GLContextAttrs glContextAttrs = {8, 8, 8, 8, 24, 8};

    GLView::setGLContextAttrs(glContextAttrs);
}

bool AppDelegate::applicationDidFinishLaunching() {

    auto director = Director::getInstance();
    auto glview = director->getOpenGLView();
    if(!glview) {
        glview = GLViewImpl::create("Zoomer!");
        director->setOpenGLView(glview);
    }
    
    auto visibleSize = director->getWinSize();
    log("winSize: (%f, %f)", visibleSize.width, visibleSize.height);
    
    // check if the running device is iPhone6 plus
    if (visibleSize.width > 2048) {
        glview->setDesignResolutionSize(1920, 1080, ResolutionPolicy::SHOW_ALL);
        visibleSize = director->getWinSize();
        log("new winSize: (%f, %f)", visibleSize.width, visibleSize.height);
    }

    director->setDisplayStats(true);
    director->setAnimationInterval(1.0 / 60);
    
    auto scene = HelloWorld::createScene();
    director->runWithScene(scene);

    return true;
}

// This function will be called when the app is inactive. When comes a phone call,it's be invoked too
void AppDelegate::applicationDidEnterBackground() {
    Director::getInstance()->stopAnimation();

    // if you use SimpleAudioEngine, it must be pause
    // SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground() {
    Director::getInstance()->startAnimation();

    // if you use SimpleAudioEngine, it must resume here
    // SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
}
