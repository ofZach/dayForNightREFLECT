

#include "bounceManager.h"
#include "appConstants.h"



//--------------------------------------------------------------
void bounceManager::calcNormals(ofPoint const a, ofPoint const b, ofPoint & norma, ofPoint & normb){
    ofPoint diff = b - a;
    diff.normalize();
    norma = ofPoint(diff.y * -1, diff.x);
    normb = ofPoint(diff.y, diff.x * -1);
}

//--------------------------------------------------------------
void bounceManager::makeMask(vector < plotPoint > & pts){
    pts.clear();
    
    for (int i = 0, j = 0; i < INPUT_WIDTH * INPUT_HEIGHT; i++, j+=4){
        if (data[ j + 0 ] << 16 | data[ j + 1 ] << 8 | data[ j + 2 ] > 0){
            plotPoint p;
            p.pixelId = data[ j + 0 ] << 16 | data[ j + 1 ] << 8 | data[ j + 2 ];
            p.position = i;
            pts.push_back(p);
        }
    }
    
}



//--------------------------------------------------------------
void bounceManager::setup(){
    
    LC.setup();
    
    fbo.allocate(INPUT_WIDTH, INPUT_HEIGHT);
    pix.allocate(INPUT_WIDTH, INPUT_HEIGHT, OF_IMAGE_COLOR_ALPHA);
    
    ofSetVerticalSync(false);
    ofSetFrameRate(0);
    
    for (int i = 0; i < NUM_BOUNCE_THREADS; i++){
        BT[i] = new bounceThread();
        BT[i]->seed = i * 10000;
        BT[i]->startThread();
    }
    
    ofSetWindowShape(INPUT_WIDTH, INPUT_HEIGHT);
    
       //  mt = new std::mt19937(0);
}

//--------------------------------------------------------------
void bounceManager::update(vector < ofPolyline > linesToAdd, float tablePct){
    
    
    frameData fData;
    fData.frameNum = ofGetFrameNum();
    fData.bRemove = false;
    
    
    LC.update();
    
    
    //-----------------------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------------------
    
    //---------------------------- non table...
    if (true){
        
//        ofPolyline lineTemp;
//        for (int i = 0; i < 150; i++){
//            lineTemp.addVertex(ofPoint(200 + i * 1, sin(i/10.0 + ofGetElapsedTimef()) * 100 + 900));
//        }
//        lineTemp = lineTemp.getResampledBySpacing(10);
//        
//        ofPolyline tLineTemp;
//        for (int i = 0; i < lineTemp.size() * (1-tablePct); i++){
//            tLineTemp.addVertex(lineTemp[i]);
//        }
//        
//        if (tLineTemp.size() > 1){
//            for (int i = 0; i < lineTemp.size()-1; i++){
//                
//                ofPoint a = lineTemp[i];
//                ofPoint b = lineTemp[i+1];
//                ofPoint midPt = (a + b) / 2.0;
//                float dist = (a - b).length() / 2.0;
//                dist *= 1.1;
//                ofPoint angleD = (a-b).getNormalized();
//                
//                fData.lines.push_back(midPt + dist * angleD);
//                fData.lines.push_back(midPt - dist * angleD);
//            }
//        }
        
        
        // YOOO....
        
        std::uniform_real_distribution<> dis(0,1);
        mt.seed(100);
        cout << dis(mt) << endl;
        
        
        float angle =  PI/2; //ofGetMousePressed() ? 0 : ofGetElapsedTimef()*0.4;
        float ttt = ofGetElapsedTimef() * 0.4;
        float currentTime = ofGetElapsedTimef();
        int count = 300  * (1-tablePct);
        
        float pct1 = powf(sin(ofGetElapsedTimef() * 0.2) * 0.5 + 0.5, 4.0);
        
        
        
        for (int i = 0; i < count; i++){
            ofVec2f random = ofPoint( dis(mt) * OUTPUT_WIDTH, dis(mt) * OUTPUT_HEIGHT);
            
            
            float baseLength = sin(currentTime + i/100.0) * 200 + 200;
            
            float angle =  (TWO_PI * dis(mt)) * pct1 + (1-pct1) * PI; //ofRandom(0,TWO_PI);
            float dist = 50 + baseLength * dis(mt);
            ofPoint p = ofPoint(cos(angle), sin(angle));
            
            ofVec2f newPt = random + p * dist;
            
            //random += ofPoint(sin(ttt + i/10.0), 0) * 100;
            //newPt += ofPoint(sin(ttt + i/10.0), 0) * 100;
            
            if ( (random - ofVec2f(OUTPUT_WIDTH/2, OUTPUT_HEIGHT/2)).length() > 200){
                fData.lines.push_back(random);
                fData.lines.push_back(newPt);
            }
        }
        
        
        
        
    }
    
    //---------------------------- table...
    if (true){
        
        for (auto line : linesToAdd){
            
            ofPolyline temp = line;//.getResampledBySpacing(3);
            //temp = temp.getSmoothed(3);
            
            temp.simplify();
            
            temp = temp.getResampledBySpacing(3.0);
            //temp = temp.getResampledBySpacing(15);
            temp = temp.getSmoothed(3);
            //cout << ofGetMouseX() * 0.3 << endl;
            
            
            float ww = 1024;
            float hh = 1024 * (2.0/3.0);
            
            float scale = 1080 / (float)hh;
            float offset = (OUTPUT_WIDTH - ww)/2.0;
            
            int count = (temp.size()-1) * tablePct;
            if (count > 1){
                for (int i = 0; i < count; i++){
                    ofPoint a = temp[i] * scale + ofPoint(offset, 0);
                    ofPoint b = temp[i+1] * scale + ofPoint(offset, 0);
                    fData.lines.push_back(a);
                    fData.lines.push_back(b);
                }
            }
            
        }
        
        
    }
    
    //-----------------------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------------------
    
    fData.normalsA.resize(fData.lines.size()/2);
    fData.normalsB.resize(fData.lines.size()/2);
    for (int i = 0; i < fData.lines.size()/2; i++){
        calcNormals(fData.lines[i*2], fData.lines[i*2+1], fData.normalsA[i], fData.normalsB[i]);
    }
    
    frames.push_back(fData);
    
    
    
    //    //--------------------------------------------------------------
    //    // embed the frame rate in the FBO.  be careful !
    fbo.begin();
    ofClear(0,0,0, 255);
    for (int i = 0; i < fData.lines.size()/2; i++){
        ofSetHexColor((int)(i+1));
        ofSetLineWidth(3);
        ofDrawLine(fData.lines[i*2] * ONE_D_OUTPUT_LARGE_FACTOR_D_2, fData.lines[i*2+1] * ONE_D_OUTPUT_LARGE_FACTOR_D_2);
    }
    ofSetHexColor( ofGetFrameNum() );
    ofFill();
    ofDrawRectangle(0, 0, 3, 3);
    fbo.end();
    
    
    
    //--------------------------------------------------------------
    fastReader.readToPixels(fbo, pix, OF_IMAGE_COLOR_ALPHA);
    
    // get the frameNum
    data = pix.getData();
    int frameNum = (int)(data[0] << 16 | data[1] << 8 | data[2] << 0);
    
    //--------------------------------------------------------------
    // black like my heart...
    
    for (int i = 0; i < 10; i++){
        for (int j = 0; j < 10; j++){
            int pix = (j * INPUT_WIDTH + i) * 4;
            data[pix + 0] = 0;
            data[pix + 1] = 0;
            data[pix + 2] = 0;
            data[pix + 3] = 0;
            
        }
    }
    
    
    if (ofGetFrameNum() - frameNum <= 2){
        if (frames.size() > 4){
            frames.erase(frames.begin());
        }
    }
    
    
    float timeNow = PI/2;
    
    for (int i = 0; i < frames.size(); i++){
        if (frames[i].frameNum == frameNum){
 
            lastReadFrame = frames[i];              // copy it :)
            
            vector < plotPoint > pts;
            
            makeMask(pts);
            
            
            
            // we can safely call the threads....
            
            for (int j = 0; j < NUM_BOUNCE_THREADS; j++){
                if (BT[j]->bReadyForData == true){
                    
                    if ( (j % 5) == (ofGetFrameNum() % 5)){
                        bounceRequest br;
                        
                        br.lines = lastReadFrame.lines;
                        br.normalsA = lastReadFrame.normalsA;
                        br.normalsB = lastReadFrame.normalsB;
                        br.pts = pts;
                        
                        // lights:
                        br.lights = LC.getLights(600);
                        
                        br.nLevelsDeep = 10;
                        //br.nLines =  1000;
                        br.frameId = ofGetFrameNum();
                        BT[j]->analyze(br);
                    }
                }
            }
        }
    }
    
    
    // other way of removing....
    
    ////    //--------------------------------------------------------------
    ////    // remove
    ////    for (int i = 0; i < frames.size(); i++){
    ////        if (frames[i].frameNum <= frameNum){
    ////            frames[i].bRemove = true;
    ////        }
    ////    }
    ////
    ////
    ////
    ////    auto new_end = std::remove_if(frames.begin(), frames.end(),
    ////                                  [](const frameData & data)
    ////                                  { return data.bRemove; });
    ////
    ////    frames.erase(new_end, frames.end());
    ////
    //    //cout << frames.size() << endl;
    
    
    for (int i = 0; i < NUM_BOUNCE_THREADS; i++){
        BT[i]->update();
//        if (BT[i]->isFrameNew()){
//            
//        }
        
        meshes[i].clear();
        meshes[i].setMode(OF_PRIMITIVE_LINES);
        meshes[i].addVertices(BT[i]->pts);
        meshes[i].setUsage(GL_STREAM_DRAW);
    }
    
}

//--------------------------------------------------------------
void bounceManager::draw(){
    
    for (int i = 0; i < NUM_BOUNCE_THREADS; i++){
      
        meshes[i].draw();
    }
    
}