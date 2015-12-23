/*
 * bounceThread.cpp
 *
 *  Created on: Oct 8, 2014
 *      Author: arturo
 */

#include "bounceThread.h"
#include "ofConstants.h"
#include "appUtils.h"







inline ofPoint getNormalFacing ( ofPoint const incident, ofPoint const norma, ofPoint const normb){        // assume normalized....
    
    float a = incident.dot(norma);
    float b = incident.dot(normb);
    if (a < b) return norma;
    else return normb;
}

ofPoint bounceThread::getReflect( ofPoint const incident, ofPoint const norma, ofPoint const normb){     // assume normalized....
    ofPoint normal = getNormalFacing(incident, norma, normb);
    return incident - normal * 2.f * normal.dot(incident);
}



// todo: optimize this:
int bounceThread::plot( bounceRequest * r, int x, int y){
    
    int p = y * INPUT_WIDTH + x;
    
    if (mask[p] > 0){
        return dataAsInt[p]; //(data[ pix + 0 ] << 16 | data[ pix + 1 ] << 8 | data[ pix + 2 ]);
    } else {
        return 0;
    }
}



int bounceThread::Bresenham(bounceRequest * r, float x0, float y0, float x1, float y1, int whichToSkip = -1){
    
    
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int x = x0;
    int y = y0;
    int n = 1 + dx + dy;
    int x_inc = (x1 > x0) ? 1 : -1;
    int y_inc = (y1 > y0) ? 1 : -1;
    int error = dx - dy;
    dx *= 2;
    dy *= 2;
    
    
    for (; n > 0; --n)
    {
        
        if (x < 0 || x > INPUT_WIDTH-1 || y < 0 || y > INPUT_HEIGHT-1) return 0;
        
        int which = plot(r,x,y);
        
        if (which != 0){
            
            
            ofPoint intersection;
            int t = which-1;
        
            //cout << "t " << t << endl;
            // do this BIG or SMALL ???
            // before I did this small.
            // bool b = ofLineSegmentIntersection(ofPoint(x0, y0), ofPoint(x1, y1),
            // lines[t*2]/SCALE_CHANGE, lines[t*2+1]/SCALE_CHANGE, intersection);
            
            // diff to do this big?
            bool b = ofLineSegmentIntersection(ofPoint(x0, y0), ofPoint(x1, y1),
                                               r->lines[t*2]/OUTPUT_LARGE_FACTOR_D_2, r->lines[t*2+1]/OUTPUT_LARGE_FACTOR_D_2, intersection);
            if (b == true && which != whichToSkip){
                
                if (which < 0){
                    //cout << "w " << which << endl;
                }
                //ofSetColor(255,0,0);
                //ofCircle(x,y,2);
                return which;
            } else {
                //ofSetColor(0,255,0);
                //ofCircle(x,y,2);
            }
        }
        //visit(x, y);
        
        if (error > 0)
        {
            x += x_inc;
            
            error -= dy;
        }
        else
        {
            y += y_inc;
            error += dx;
        }
    }
}

void bounceThread::bounce(bounceRequest * r, bounceInput & bi, bounceOutput &bo){
    
    for (int i = 0; i < bi.segs.size(); i++){
        bi.segs[i].calcIdenity();
    }
    
    
    
    for (int i = 0; i < bi.segs.size(); i++){
        
        
        
        int t = Bresenham(r, bi.segs[i].a.x/OUTPUT_LARGE_FACTOR_D_2, bi.segs[i].a.y/OUTPUT_LARGE_FACTOR_D_2, bi.segs[i].b.x/OUTPUT_LARGE_FACTOR_D_2, bi.segs[i].b.y/OUTPUT_LARGE_FACTOR_D_2, bi.segs[i].lastHit);
        
        if (t != 0){
            t = t - 1;
            
            bool b;;
            ofPoint intersection;
            
            if (t < 0 || t > r->lines.size()/2-1){
                //cout << i << " " << bi.segs[i].a.x/OUTPUT_LARGE_FACTOR_D_2<< " " <<  bi.segs[i].a.y/OUTPUT_LARGE_FACTOR_D_2<< " " <<  bi.segs[i].b.x/OUTPUT_LARGE_FACTOR_D_2<< " " <<  bi.segs[i].b.y/OUTPUT_LARGE_FACTOR_D_2<< " " <<  bi.segs[i].lastHit;
                b = false;
            } else {
            // pragma mark todo: optimize this.
            
                // DO THE BOUNCE
                // (a) get intersection
                
                b = ofLineSegmentIntersection(bi.segs[i].a, bi.segs[i].b, r->lines[t*2], r->lines[t*2+1], intersection);
                
            }
            
            if (b == true){
                //cout << "bounce " << endl;
                // (b) add pre bounce
                lineSeg temp;
                temp.a = bi.segs[i].a;
                temp.b = intersection;
                bo.ptsPreBounce.push_back( temp );
                
                // (c) calc bounce
                //cout << "bouncing " << i << " " << t << endl;
                
                ofPoint reflect = getReflect(bi.segs[i].identity, r->normalsA[t], r->normalsB[t]);
                
                // (d) post bounce
                temp.a = intersection + reflect;
                temp.b = intersection + reflect * OUTPUT_RAY;
                temp.lastHit = t+1;
                bo.ptsPostBounce.push_back( temp );
                
            } else {
                
                // ¬Ø\_(„ÉÑ)_/¬Ø
                
            }
            
        } else {
            
            // DONT DO THE BOUNCE
            bo.ptsNoBounce.push_back(bi.segs[i]);
            
            
            
        }
        
    }
    
    
}




bounceThread::bounceThread()
:newFrame(true){
    // start the thread as soon as the
    // class is created, it won't use any CPU
    // until we send a new frame to be analyzed
    //startThread();
    
    mt = new std::mt19937(0);
    
    bReadyForData = true;
}

bounceThread::~bounceThread(){
    // when the class is destroyed
    // close both channels and wait for
    // the thread to finish
    toAnalyze.close();
    analyzed.close();
    waitForThread(true);
}

void bounceThread::analyze(bounceRequest & bounce){
    // send the frame to the thread for analyzing
    // this makes a copy but we can't avoid it anyway if
    // we want to update the grabber while analyzing
    // previous frames
    toAnalyze.send(bounce);
    bReadyForData = false;
}

void bounceThread::update(){
    // check if there's a new analyzed frame and upload
    // it to the texture. we use a while loop to drop any
    // extra frame in case the main thread is slower than
    // the analysis
    // tryReceive doesn't reallocate or make any copies
    newFrame = false;
    while(analyzed.tryReceive(res)){
        
        newFrame = true;
    }
    if(newFrame){
        bReadyForData = true;
        pts = res.pts;
    }
    
//    ofSetColor(255,255,255,20);
//    for (int i = 0; i < pts.size()/2; i++){
//        ofLine(pts[i*2], pts[i*2+1]);
//    }
//    
    
}

bool bounceThread::isFrameNew(){
    return newFrame;
}


void bounceThread::threadedFunction(){
    
    while(isThreadRunning()){
        // wait until there's a new frame
        // this blocks the thread, so it doesn't use
        // the CPU at all, until a frame arrives.
        // also receive doesn't allocate or make any copies
        bounceRequest req;
        bounceResult res;
        
        
        if(toAnalyze.receive(req)){
            
            
            
            memset(mask, 0, INPUT_WIDTH*INPUT_HEIGHT);
            memset(dataAsInt, 0, INPUT_WIDTH*INPUT_HEIGHT * 4);

            int maxId = 0;
            for (auto p : req.pts){
                dataAsInt[p.position] = p.pixelId;
                mask[p.position] = 255;
                
                if (p.pixelId > maxId){
                    maxId = p.pixelId;
                }
            }
           
//            
//            bounceInput bi;
//            for (int i = 0; i < req.nLines; i++){
//                lineSeg temp;
//                
//                
//                temp.a = ofPoint(ofGetMouseX(), ofGetMouseY());
//                temp.b = ofPoint(ofGetMouseX(), ofGetMouseY());
//                ofPoint offset = ofPoint(dist(*mt), dist(*mt));
//                
//                temp.a *= OUTPUT_LARGE_FACTOR_D_2;
//                temp.b *= OUTPUT_LARGE_FACTOR_D_2;
//                ofPoint aaa = ofPoint(dist(*mt), dist(*mt));;;
//                ofPoint bbb = ofPoint(dist2(*mt), dist2(*mt));;;
//                temp.a += aaa;
//                temp.b += bbb + aaa;
//                
//                
//                temp.lastHit = -1;
//                bi.segs.push_back(temp);
//                
//            }
            
            bounceInput bi;
            
            for (int i = 0; i < req.lights.size()/2; i++){
                lineSeg temp;
                temp.a = req.lights[i * 2];
                temp.b = req.lights[i * 2 + 1];
                
                bi.segs.push_back(temp);
            }
            
            
            for (int j = 0; j < req.nLevelsDeep; j++){
                bounceOutput bo;
                bounce(&req, bi, bo);
                
                //ofSetColor(255,0,0);
                //cout << bo.ptsNoBounce.size() << endl;
                
                for (int i = 0; i < bo.ptsNoBounce.size(); i++){
                    res.pts.push_back(bo.ptsNoBounce[i].a);
                    res.pts.push_back(bo.ptsNoBounce[i].b);
                }
                
                //ofSetColor(0,255,0);
                for (int i = 0; i < bo.ptsPreBounce.size(); i++){
                    
                    res.pts.push_back(bo.ptsPreBounce[i].a);
                    res.pts.push_back(bo.ptsPreBounce[i].b);
                    
                    //cout << bo.ptsPreBounce[i].a << " " << bo.ptsPreBounce[i].b << endl;
                    //ofLine(bo.ptsPreBounce[i].a, bo.ptsPreBounce[i].b);
                }
                
                //ofSetColor(0,0,255);
                for (int i = 0; i < bo.ptsPreBounce.size(); i++){
                    
                    //temp.addVertex(bo.ptsPostBounce[i].a);
                    //temp.addVertex(bo.ptsPostBounce[i].b);
                    
                    
                    //ofLine(bo.ptsPostBounce[i].a, bo.ptsPostBounce[i].b);
                }
                
                bi.segs.clear();
                bi.segs = bo.ptsPostBounce;
            }
            
            
            res.frameId = req.frameId;
            
            
            
            // once processed send the result back to the
            // main thread. in c++11 we can move it to
            // avoid a copy
#if __cplusplus>=201103
            analyzed.send(std::move(res));
#else
            analyzed.send(res);
#endif
        }else{
            // if receive returns false the channel
            // has been closed, go out of the while loop
            // to end the thread
            break;
        }
    }
}

