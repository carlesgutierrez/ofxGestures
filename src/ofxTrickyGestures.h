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

#define TIMER_GESTURES_UPDATE 33 
#define MINSTATS_GESTURE_REACT 0.005
#define THERSHOLD_MINMOVEMENT_REACT 0.00001
#define MINSTATS_GESTURE_PAN_REACT 0.13
#define FRAMES_AVG 10

class ofxTrickyGestures
{
public:

	
	void update();//TODO set this as an automatic call to update too.

	//Get the statistics of the actual gestures
	ofxStatistics myDiffFrameDistsStats;
	ofxStatistics myDiffFrameLocactionsStats;
	ofxSimpleTimer *_timer;
	void timerCompelte(string &name);

	void setupPan(ofVec2f locItem);
	void setupPinch(ofVec2f sizeItem);
	void resetPan();
	void resetPinching();

    class PanEvent{
    public:

		void setup(ofVec2f posItem) {
			locPan = posItem;
			cout << "Setting locPan to " << locPan << endl;
		}
		ofVec2f getOrigin() {
			return originPan; //Check that might be 0 .. 1 ?
		}
		ofVec2f getRelativeLocation() { 
			return locPan;//dirPan;
		};

		ofVec2f getPanDirection() {
			return dirPan;
		}

		ofVec2f originPan = ofVec2f(0, 0);
		ofVec2f locPan = ofVec2f(0, 0);
		ofVec2f dirPan = ofVec2f(0, 0);

		void reset() {
			//originPan = ofVec2f(0, 0);
			//locPan = ofVec2f(0, 0);
			//dirPan = ofVec2f(0, 0);
		}
    };

   // ofEvent<PanEvent> panGestureEvent;
   // ofEvent<PanEvent> panGestureEndedEvent;

    class PinchEvent{
    public:

		void setup(ofVec2f sizeItem) {
			itemSize = sizeItem;
			cout << "Seting itemSize to " << itemSize << endl; //always 1...?! TODO check if size start ok at setup size 
		}

        //ofVec2f getDelta();  // ofVec2f getRelativeDelta() const; // double getAngle() const;  // double getScale() const; // double getRelativeAngle() const;
     
		//Setting generic names
		bool isZoomIn() {
			return bPinchPlus;
		}

		bool isZoomOut() {
			return bPinchPlus;
		}

		ofVec2f getRelativeScale() {
			//all dist values are from 0 to 1. Differences will be betwen -1 and +1 // 0 will mean stable pinch value
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
			originPinch = ofVec2f(0, 0);
			distOrigin = 0;
			distActual = 0;
			bPinchPlus = false;
			//itemSizeScaled = ofVec2f(1, 1);
			//itemSize = ofVec2f(1, 1);
		}

	private:
		
    };
    
    static ofxTrickyGestures & get();
    bool isPanning() const {return m_isPanning;}
    ofVec2f getPanOrigin();
	ofVec2f getPanDirection();
    ofVec2f getPanRelativeLocation();
    
    bool isPinching(bool &bPinchPlus) const {
		bPinchPlus = pinch.bPinchPlus; //value updated by ref
		return m_isPinching;
	}

    ofVec2f getPinchOrigin();
	ofVec2f getPinchScale();
	
	//tricky functions//Pinch//Pan//DoubleTap?
	float getNormDistanceTouches();
	ofVec2f getMiddlePosTouches();
	bool m_isPreGesture = true;
	float distInitial = 0;
	float distActual = 0, last_distActual = 0;
	ofVec2f locInitial, locActual, last_locActual; //of the midPoint or just one finguer its fine too
	
	int getIsAllTouchsMoving();



private:
    bool touchDown(ofTouchEventArgs & touch);
    bool touchMoved(ofTouchEventArgs & touch);
    bool touchUp(ofTouchEventArgs & touch);

	void updatePanGestureRecognition();
	void updatePinchGestureRecognition();

    //std::map<int, ofTouchEventArgs> m_touches;
	vector<ofTouchEventArgs> myTouchesVector;

	bool m_isDoubleTap = false;
	bool m_isPanning = false;

	bool m_isPinching = false;
	bool m_isPrePinching = false;

    PinchEvent pinch;
    PanEvent pan;

public:
	ofxTrickyGestures();
    ~ofxTrickyGestures();
};

