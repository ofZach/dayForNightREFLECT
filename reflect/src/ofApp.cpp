#include "ofApp.h"
#include "appUtils.h"
#include "appConstants.h"


using namespace ofxCv;
using namespace cv;


//--------------------------------------------------------------
void ofApp::setup(){
    //ofSetWindowPosition( -920, 0 );
    ofSetCircleResolution(100);
    //                  k g
    
//    cout << "------------------------------------------ " << endl;
//    cout << "INPUT_WIDTH : " << INPUT_WIDTH << endl;
//    cout << "INPUT_HEIGHT : " << INPUT_HEIGHT << endl;
//    cout << "INPUT_WIDTH_CACHED : " << INPUT_WIDTH_CACHED << endl;
//    cout << "INPUT_HEIGHT_CACHED : " << INPUT_HEIGHT_CACHED << endl;
//    cout << "OUTPUT_WIDTH : " << OUTPUT_WIDTH << endl;
//    cout << "OUTPUT_HEIGHT : " << OUTPUT_HEIGHT << endl;
//    cout << "OUTPUT_LARGE_FACTOR_D_2 : " << OUTPUT_LARGE_FACTOR_D_2 << endl;
//    cout << "OUTPUT_LARGE_FACTOR_D_2_CACHED : " << OUTPUT_LARGE_FACTOR_D_2_CACHED << endl;
//    cout << "------------------------------------------ " << endl;
//
//#ifdef FIXING_CONSTANTS
//    std::exit(0);
//#endif
    
    motionLevel = 0;
    
    outputFbo.allocate(OUTPUT_WIDTH, OUTPUT_HEIGHT, GL_RGBA32F_ARB);
    BM.setup();
    
    contourFinder.setMinAreaRadius(1);
    contourFinder.setMaxAreaRadius(1000000);
    contourFinder.setThreshold(15);
    // wait for half a frame before forgetting something
    contourFinder.getTracker().setPersistence(15);
    // an object can move up to 32 pixels per frame
    contourFinder.getTracker().setMaximumDistance(32);
    
    showLabels = true;
    
    
    // do 3x2 for NOW
    tempPixels.allocate(1024, 1024 * (2.0/3.0), OF_IMAGE_COLOR);
    tempPixels2.allocate(1024, 1024 * (2.0/3.0), OF_IMAGE_COLOR);
    
    redFromLastFrame = new unsigned char[(int)(1024 * (1024 * (2.0/3.0)))];
    memset(redFromLastFrame, 0, (int)(1024 * (1024 * (2.0/3.0))));
    bFirstFrameOfMotion = true;
    motion = 0;
    motionEnergy = 0;
    
    
#ifdef USE_LIVE_VIDEO
    magicCam.setup(1920, 1080, 30);
#else
    player.loadMovie("ScopeBox Movie-1-blah movie.mov");
    player.play();
#endif
    
    bDebugView = false;
    setupGui();
    
    outputFbo.begin();
    ofClear(0,0,0, 255);
    outputFbo.end();
    
    nFramesOfMotion = 0;
    nFramesNoMotion = 0;
}


//--------------------------------------------------------------
void ofApp::setupGui(){
    parameters.setName("parameters");
    parameters.add(bDrawFrameRate.set("bDrawFrameRateOnBigScreen", false));
    parameters.add(threshold.set("threshold",50,0,255));
    parameters.add(bDrawDebugGrid.set("bDrawDebugGrid", false));
    parameters.add(motionThreshold.set("motionThreshold", 3, 0, 10));
    parameters.add(nFramesNoMotionTimeout.set("nFramesNoMotionTimeout",100, 0, 6000));
    
 //   nFramesNoMotionTimeout
    
    //parameters.add(color.set("color",100,ofColor(0,0),255));
    gui.setup(parameters);
    ofSetBackgroundColor(0);
    
    gui.loadFromFile("settings.xml");
}



//--------------------------------------------------------------
void ofApp::update(){
 
    
#ifdef USE_LIVE_VIDEO
    //BM.update(contourFinder.getPolylines());
    bool bFrame = magicCam.update();
    if (bFrame == true){
//        magicCam.getColorPixels().cropTo(img2, 70, 50, 1000, 800);
//        ofxCv::invert(img2);
//        contourFinder.setThreshold(213); //ofMap(mouseX, 0, ofGetWidth(), 0, 255));
        
        
            contourFinder.setThreshold(threshold);
       
            
            vector<Point2f> warpPoints;
            
            warpPoints.push_back(Point2f(16,75));
            warpPoints.push_back(Point2f(1380,21));
            warpPoints.push_back(Point2f(1408,900));
            warpPoints.push_back(Point2f(94,971));
            
            
            ofxCv::unwarpPerspective(magicCam.getColorPixels(),tempPixels, warpPoints, INTER_LINEAR);
            ofxCv::invert((tempPixels));
            tempPixels.update();
            
        int diff = 0;
        unsigned char * data = tempPixels.getPixels().getData();
        for (int i = 0; i < tempPixels.getWidth() * tempPixels.getHeight(); i++){
            
            unsigned char val = data[i*3];
            diff += abs(val - redFromLastFrame[i]);
            redFromLastFrame[i] = val;
            
        }
        
        if (diff != 0){
            float diff2 = (float)diff / (float)(tempPixels.getWidth() * tempPixels.getHeight());
            if (!bFirstFrameOfMotion){
                motionLevel = diff2;
                //cout << "-----------> " << (int)diff2 << " " << diff <<  endl;
            }
            bFirstFrameOfMotion = false;
        }
        
        
        
            
            ofxCv::blur(tempPixels, 10);
            contourFinder.setFindHoles(true);
            //contourFinder.setInvert(true);
            contourFinder.findContours(tempPixels);
        
    }
#else
    player.update();
    if (player.isFrameNew()){
        
        contourFinder.setThreshold(threshold);
        
        
        vector<Point2f> warpPoints;
        
        warpPoints.push_back(Point2f(16,75));
        warpPoints.push_back(Point2f(1380,21));
        warpPoints.push_back(Point2f(1408,900));
        warpPoints.push_back(Point2f(94,971));
        
        
        ofxCv::unwarpPerspective(player.getPixels(),tempPixels, warpPoints, INTER_LINEAR);
        ofxCv::invert((tempPixels));
        tempPixels.update();
        
        
        int diff = 0;
        unsigned char * data = tempPixels.getPixels().getData();
        for (int i = 0; i < tempPixels.getWidth() * tempPixels.getHeight(); i++){
            
            unsigned char val = data[i*3];
            diff += abs(val - redFromLastFrame[i]);
            redFromLastFrame[i] = val;
            
        }
        
        if (diff != 0){
            float diff2 = (float)diff / (float)(tempPixels.getWidth() * tempPixels.getHeight());
            if (!bFirstFrameOfMotion){
                motionLevel = diff2;
                //cout << "-----------> " << (int)diff2 << " " << diff <<  endl;
            }
            bFirstFrameOfMotion = false;
        }
        
        
        
        ofxCv::blur(tempPixels, 10);
        contourFinder.setFindHoles(true);
        //contourFinder.setInvert(true);
        contourFinder.findContours(tempPixels);
        
        
    }
    #endif
    
    if (motionLevel > motionThreshold){
        nFramesOfMotion ++;
        nFramesNoMotion = 0;
    } else {
        nFramesOfMotion = 0;
        nFramesNoMotion++;
    }
    
    if (nFramesNoMotion > nFramesNoMotionTimeout){
        motionEnergy = 0.99f * motionEnergy + 0.01 * 0;
    } else {
        motionEnergy = 0.97f * motionEnergy + 0.03 * 1.0;
    }
    
    BM.update(contourFinder.getPolylines(), motionEnergy);
    
    
    


    
}


//--------------------------------------------------------------
void ofApp::draw(){

    
    outputFbo.begin();
    //ofClear(0,0,0, 255);
    
    
    ofSetColor(255,255,255,20);
    BM.draw();
    
    
  
    bool bDrawLastReadFrameLines = false;
    
    if (bDrawLastReadFrameLines){
        for (int i = 0; i < BM.lastReadFrame.lines.size()/2; i++){
            ofSetColor(255,255,0,mouseX);
            ofLine(BM.lastReadFrame.lines[i*2], BM.lastReadFrame.lines[i*2+1]);
        }
    }
    
    ofFill();
    ofSetColor(0,0,0, 60);
    ofDrawRectangle(0,0,OUTPUT_WIDTH,OUTPUT_HEIGHT);
    
    ofClearAlpha();
    outputFbo.end();
    

//    ofSetColor(255,255,255);
//    outputFbo.draw(0,0,OUTPUT_WIDTH*0.4, OUTPUT_HEIGHT*0.4);
    
    
    /// \param x X position to draw cropped image at.
    /// \param y Y position to draw cropped image at.
    /// \param w Width of subsection to draw.
    /// \param h Height of subsection to draw.
    /// \param sx X position in image to begin cropping from.
    /// \param sy Y position in image to begin cropping from.
    //void drawSubsection(float x, float y, float w, float h, float sx, float sy) const;
    
   // ofScale(0.1, 0.1);
    
//    outputFbo.begin();
//    ofClear(0,0,0);
//    if (bDrawDebugGrid){
//        
//            ofSetColor(255);
//            ofSetLineWidth(10);
//            ofLine(0,0,outputFbo.getWidth(), outputFbo.getHeight());
//            ofLine(0, outputFbo.getHeight(),outputFbo.getWidth(),0);
//            ofNoFill();;
//            ofCircle(outputFbo.getWidth()/2, outputFbo.getHeight()/2, outputFbo.getHeight()/2);
//        
//            ofSetLineWidth(10);
//            ofDrawRectangle(3,3,outputFbo.getWidth()-6, outputFbo.getHeight()-6);
//        
//        
//    } else {
//        
//        
//    }
//    
//    outputFbo.end();
   
//
    
    ofSetColor(255);
    //outputFbo.draw(0,0,ofGetWidth(), (float)(outputFbo.getHeight()) / (float)(outputFbo.getWidth()) * ofGetWidth());
    //outputFbo.getTexture().drawSubsection(0,0)
    //cout << ofGetFrameRate() << endl;
   
 
    //contourFinder.draw();
    
   
    
//    int nCountour = contourFinder.getPolyline(<#unsigned int i#>)
//    contourFinder.getContour(i);
//
    
    
    if (bDebugView == true){
        
        
        ofPushMatrix();
        ofScale(0.5, 0.5);
        
        RectTracker& tracker = contourFinder.getTracker();
   
        
        ofSetColor(255);
        tempPixels.draw(0, 0);
        contourFinder.draw();
        for(int i = 0; i < contourFinder.size(); i++) {
            ofPoint center = toOf(contourFinder.getCenter(i));
            ofPushMatrix();
            ofTranslate(center.x, center.y);
            int label = contourFinder.getLabel(i);
            string msg = ofToString(label) + ":" + ofToString(tracker.getAge(label));
            ofDrawBitmapString(msg, 0, 0);
            ofVec2f velocity = toOf(contourFinder.getVelocity(i));
            ofScale(5, 5);
            ofDrawLine(0, 0, velocity.x, velocity.y);
            ofPopMatrix();
        }
        ofPopMatrix();
        
        
        ofDrawBitmapString(ofToString(ofGetFrameRate()), 20,20);
        gui.setPosition(ofPoint(ofGetWidth() - 600, 10));
        gui.draw();
        
        outputFbo.draw(tempPixels.getWidth()/2.0,0,INPUT_WIDTH*0.3, INPUT_HEIGHT*0.3);
        
        
        
        ofDrawBitmapStringHighlight("motion level : " + ofToString(motionLevel), 50, tempPixels.getHeight()*0.5+50, motionLevel > motionThreshold ? ofColor::green : ofColor::red, ofColor::white);
        ofDrawBitmapStringHighlight("nFramesMotion " + ofToString(nFramesOfMotion), 50, tempPixels.getHeight()*0.5+75);
        ofDrawBitmapStringHighlight("nFramesNoMotion " + ofToString(nFramesNoMotion), 50, tempPixels.getHeight()*0.5+100);
        ofDrawBitmapStringHighlight("motionEnergy " + ofToString(motionEnergy), 50, tempPixels.getHeight()*0.5+125);
        
       
        
        
    } else {
        ofSetColor(255);
        outputFbo.getTexture().drawSubsection(0,0,1920, 1080, 0,0);
        outputFbo.getTexture().drawSubsection(1920,0,1920, 1080, 1920-230,0);
        
        if (bDrawFrameRate == true){
            ofDrawBitmapStringHighlight(ofToString(ofGetFrameRate()), ofPoint(50,50));
        }
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    if (key == 'f'){
        ofToggleFullscreen();
    }

    if (key == 'h'){
        bDebugView = !bDebugView;
    }
    
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
