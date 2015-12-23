

#pragma once


typedef struct {
    int position;
    int pixelId;
} plotPoint;

typedef struct {
    ofPoint a;
    ofPoint b;
    ofPoint identity;
    void calcIdenity(){
        identity = (b-a).getNormalized();
    }
    int lastHit;                        // which did we JUST hit?
} lineSeg;

typedef struct {
    vector < lineSeg > segs;             // a -> b
    vector < lineSeg > bounceAgainstTest;
} bounceInput;

typedef struct {
    vector < lineSeg > ptsNoBounce;
    vector < lineSeg > ptsPreBounce;
    vector < lineSeg > ptsPostBounce;
} bounceOutput;
