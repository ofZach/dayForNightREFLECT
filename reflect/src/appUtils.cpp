
#include "appUtils.h"


ofPoint getMidscreenPoint(){
    return ofPoint(ofGetWidth()/2, ofGetHeight()/2);
}

ofPoint getRandomScreenPoint(){
    return ofPoint( ofRandom(ofGetWidth()), ofRandom(ofGetHeight()));
}