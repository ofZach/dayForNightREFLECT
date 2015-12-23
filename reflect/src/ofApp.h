#pragma once

#include "ofMain.h"
#include "ofxFastFboReader.h"
#include "bounceTypes.h"
#include "bounceThread.h"
#include "ofxFastFboReader.h"
#include "ofxPanel.h"
#include "bounceManager.h"
#include "ofxCv.h"
#include "ofxBlackMagic.h"
#include "ofxCv.h"


//#define USE_LIVE_VIDEO

//------------------------------------------------------------------------------
class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    

        bounceManager BM;
    
    
        ofFbo outputFbo;
        //--------------------
    
        void setupGui();
        void drawGui(ofEventArgs & args);
        ofParameterGroup parameters;
        ofParameter<float> threshold;
        ofParameter<bool> bDrawFrameRate;
        ofParameter<bool> bDrawDebugGrid;
    
        ofParameter <float> blending;                           // todo
        ofParameter <float> lineTransparency;                   // todo
    
        ofParameter < int > numLightsPerThread;                  // todo
        ofParameter < int > numThreadsToUse;                     // todo
        ofParameter < int > numLevelsDeep;                       // todo
    
        
        ofParameter <float> motionThreshold;
        ofParameter <int> nFramesNoMotionTimeout;
    

        //ofParameter<ofColor> color;
    
    
    
        ofxPanel gui;

        ofImage img;
        ofPixels img2;
    
    float motionLevel;
    int nFramesOfMotion;
    int nFramesNoMotion;
    float motionEnergy;
    
    
    #ifdef USE_LIVE_VIDEO
        ofxBlackMagic magicCam;
    #else
        ofVideoPlayer player;
    
    #endif
    
    ofxCv::ContourFinder contourFinder;
    bool showLabels;
    ofImage        tempPixels;
    ofImage        tempPixels2;
    
    unsigned char   * redFromLastFrame;
    bool bFirstFrameOfMotion;
    float motion;
    
    
        bool bDebugView;
    
};
