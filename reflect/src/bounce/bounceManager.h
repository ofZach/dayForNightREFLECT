

#pragma once

#include "ofMain.h"
#include "ofxFastFboReader.h"
#include "bounceTypes.h"
#include "bounceThread.h"
#include "ofxPanel.h"
#include "appConstants.h"
#include <random>


//
// I do the whole show...
//




class lightController {
    
    public:

    
    float timeAccume;
    
    void setup(){
        timeAccume = 0;
    }
    void update(){
        timeAccume += ofGetLastFrameTime() * 3 *  (sin(ofGetElapsedTimef()) * 0.5 + 0.5);
    }
    
    vector < ofPoint > getLights(int numLights){
        
        float timeNow = ofGetElapsedTimef();
        float movementPct = sin(timeNow * 0.1) * 0.5 + 0.5;
        ofPoint noiseMove( ofSignedNoise(100000,timeAccume*0.2), ofSignedNoise(-10000, timeAccume*0.2));
        vector < ofPoint > pts;
        
        
        float horizSpread = powf(sin(timeNow * 0.5) * 0.5 + 0.5, 8.0);
        
        
        for (int k = 0; k < numLights; k++){
            ofPoint a = ofPoint(OUTPUT_WIDTH/2, OUTPUT_HEIGHT/2);
            a += movementPct * noiseMove * ofPoint(1000, 500);
            
            ofPoint offset(horizSpread*ofRandom(-200,200), 0);
            
            pts.push_back(a  + offset );
            float angle = timeNow + ofRandom(0, TWO_PI);
            float dist = 3900;
            pts.push_back( a + ofPoint(cos(angle), sin(angle)) * dist + offset);
            
        }
        return pts;
        
    }

};



//------------------------------------------------------
class frameData {
public:
    vector < ofPoint > lines;
    vector < ofPoint > normalsA;
    vector < ofPoint > normalsB;
    int frameNum;
    bool bRemove;
};

class bounceManager {
    
    
public:
    
    
    void setup();
    void update(vector < ofPolyline > linesToAdd, float tablePct);
    void draw();
    
    
    vector < ofPoint > lines;
    vector < ofPoint > normalsA;
    vector < ofPoint > normalsB;
    ofFbo fbo;
    ofPixels pix;
    void makeMask();
    void makeMask(vector < plotPoint > & pts);
    unsigned char * data;
    unsigned char * mask;
    int * dataAsInt;
    ofxFastFboReader fastReader;
    void calcNormals(ofPoint const a, ofPoint const b, ofPoint & norma, ofPoint & normb);
    bounceThread  * BT[NUM_BOUNCE_THREADS];
    vector < frameData > frames;
    frameData lastReadFrame;
    ofVboMesh meshes[NUM_BOUNCE_THREADS];
    
    
    lightController LC;
    
    std::mt19937 mt;
    
    
};