#pragma once
#include "ofMain.h"
#include "bounceTypes.h"
#include "appConstants.h"
#include <random>



typedef struct {
    
    vector < ofPoint > lights;  // all the shiny lights. 
    vector < ofPoint > lines;
    vector < ofPoint > normalsA;
    vector < ofPoint > normalsB;
    vector < plotPoint > pts;

    int nLevelsDeep;
    int frameId;
    
} bounceRequest;

typedef struct {
    vector < ofPoint > pts;
    int frameId;
    int timeTookMillis;
} bounceResult;

class bounceThread: public ofThread {
public:
    
    
    //--------------------------------------------
    // stuff I need to bounce

    unsigned char mask[INPUT_WIDTH*INPUT_HEIGHT];
    int dataAsInt[INPUT_WIDTH*INPUT_HEIGHT];
    
    int seed;
    std::mt19937 * mt;
    
    ofPoint getReflect( ofPoint const incident, ofPoint const norma, ofPoint const normb);
    int plotNoMask( bounceRequest * r, int x, int y);
    // todo: optimize this:
    int plot( bounceRequest * r,int x, int y);
    int Bresenham(bounceRequest * r,float x0, float y0, float x1, float y1, int whichToSkip);
    void bounce(bounceRequest * r,bounceInput & bi, bounceOutput &bo);
    
    
    bounceThread();
    ~bounceThread();
    void analyze(bounceRequest & bounce);
    void update();
    
    ofMesh mesh;
    
    bool isFrameNew();
    
    bounceResult res;
    bool bReadyForData;
    
    vector < ofPoint > pts;
    
    
private:
    
    void threadedFunction();
    ofThreadChannel<bounceRequest> toAnalyze;
    ofThreadChannel<bounceResult> analyzed;
    bool newFrame;
    
};
