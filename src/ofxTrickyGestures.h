//
//  ofxTrickyGestures.h
//
//  Created by Carles Gutierrez 2016 at http://unstatic.tech/
//  http://carlesgutierrez.github.io
//

#pragma once

#include "ofMain.h"
//dependencies
#include "ofxStatistics.h"
#include "ofxSimpleTimer.h"
#include "ofxGui.h"
#include "eventsGestures.h"

#define FRAMES_AVG 10

class ofxTrickyGestures
{
public:

	
	void update();//TODO set this as an automatic call to update too.

	//Get the statistics of the actual gestures
	ofxStatistics myDiffFrameDistsStats;
	ofxStatistics myDiffFrameLocactionsStats;
    ofxStatistics myPinchingStats;
	ofxSimpleTimer *_timer;
	void timerCompelte(string &name);

    void setupPan(ofVec2f locItem, int w, int h);
	void setupPinch(ofVec2f sizeItem);
    void setupSingleTap(int timerSingleTap, int timerDoubleTap);

    void resetPan();
    void resetPinching();


    class SingleTapEvent{
    public:

        void setup(int timerSingle, int timerDouble) {
            timerWhileSingleTapActive = timerSingle;
            timerWhileDoubleTapActive = timerDouble;
        }

        bool getIsSingleTap() {
            return bIsSingleTap;
        }

        bool bIsSingleTap = false;
        bool bIsDoubleTap = false;
        int timerWhileSingleTapActive = 0;
        int timerWhileDoubleTapActive = 0;
        int initSingleTapTime = 0;
        int initDoubleTapTime = 0;
        int endTime = 0;

        void reset() {

        }
    };

    class PanEvent{
    public:

        void setup(ofVec2f posItem, int _w, int _h) {
			locPan = posItem;
            w = _w;
            h = _h;
            ofLog() << "Setting locPan to ";
		}

        ofVec2f getOrigin() {
            return originPan;
		}

        ofVec2f getRelativeLocation() {
			return locPan;//dirPan;
        }

		ofVec2f getPanDirection() {
			return dirPan;
		}

        ofVec2f originPan = ofVec2f(0, 0);
        ofVec2f locPan = ofVec2f(0, 0);
		ofVec2f dirPan = ofVec2f(0, 0);
        int w;
        int h;

        void reset() {
		}
    };

    class PinchEvent{
    public:

		void setup(ofVec2f sizeItem) {
			itemSize = sizeItem;
            ofLog() << "Seting itemSize to " << itemSize;
		}

		//Setting generic names
		bool isZoomIn() {
			return bPinchPlus;
		}

		bool isZoomOut() {
			return bPinchPlus;
		}
        
		//all dist values are from 0 to 1.
		//Differences will be Minimum Stats Gesture Reactbetwen -1 and +1 
		// 0 will mean stable pinch value
		ofVec2f getRelativeScale() {

			float diffDist = -1*(distOrigin - distActual);
			diffDist *= 2;

			if (diffDist > 0) { //Pinch +
                bPinchPlus = true;
			}
			else if (diffDist < 0) { //Pinch -
				bPinchPlus = false;
			}
			
            itemSizeScaled = itemSize + diffDist*itemSize;

			return itemSizeScaled;
		}

        ofVec2f originPinch = ofVec2f(0, 0);
		float distOrigin = 0;
		float distActual = 0;
		bool bPinchPlus = false;

		//Scale value for zoom in/out
		ofVec2f itemSizeScaled = ofVec2f(1, 1);
		ofVec2f itemSize = ofVec2f(1, 1);

		void reset() {
			//originPinch = ofVec2f(0, 0);
			//distOrigin = 0;
            //distActual = 0;
            bPinchPlus = false;
		}

	private:
		
    };
    
    static ofxTrickyGestures & get();
    bool isPanning() const {return m_isPanning;}
    bool isSingleTap() const {return m_isSingleTap;}
    void resetSingleTap()  {m_isSingleTap = false;}
    bool isDoubleTap() const {return m_isDoubleTap;}
    void resetDoubleTap()  {m_isDoubleTap = false;}

    ofVec2f getPanOrigin();
	ofVec2f getPanDirection();
    ofVec2f getPanRelativeLocation();
    
    bool isPinching(bool &bPinchPlus) const {
        bPinchPlus = pinch.bPinchPlus; //this replay direction pinch
		return m_isPinching;
	}

    ofVec2f getPinchOrigin();
	ofVec2f getPinchScale();
	
	//Tricky functions
	float getNormDistanceTouches();//Pinch
    ofVec2f getMiddlePosTouches();//Pan
    ofVec2f getFirstPosTouches();//DoubleTap
	int getIsAllTouchsMoving();
    int getIsPinchigMoving();

	bool m_isPreGesture = true;
	float distInitial = 0;
    float distPinchActual = 0, last_PinchdistActual = 0;
    ofVec2f locPanInitial, locFingerActual, last_locFingerActual; //of the midPoint or just one finguer its fine too

    //Gui Params for fine tunning
    bool bGuiHide = true;
    ofxPanel gui;
    ofxFloatSlider scalePanTunning;
    ofxFloatSlider scalePinchTunning;
    ofxIntSlider timerGestureUpdate;
    ofxFloatSlider minStatsGestureReact;
    ofxFloatSlider thersholdMinMoveReact;
    ofxFloatSlider minStatsGesturePanReact;
	ofxFloatSlider thersholdMinPinchingReact;

private:
    void touchDown(ofTouchEventArgs & touch);
    void touchMoved(ofTouchEventArgs & touch);
    bool updateTouchVectorData(ofTouchEventArgs & touch);
    void touchUp(ofTouchEventArgs & touch);

	void updatePanGestureRecognition();
	void updatePinchGestureRecognition();
	void initialResetPinchingVars();

    vector<ofTouchEventArgs> myTouchesVector; //updated Data of Touches
    int last_numTouchesActive = 0;
    ofVec2f diff_finger2locPan;

    bool m_isSingleTap = false;
	bool m_isDoubleTap = false;
	bool m_isPanning = false;

	bool m_isPinching = false;
	bool bCasePinchOneFingerLeftKeep = false;

    PinchEvent pinch;
    PanEvent pan;
    SingleTapEvent singleTap;

public:
	ofxTrickyGestures();
    ~ofxTrickyGestures();
};

