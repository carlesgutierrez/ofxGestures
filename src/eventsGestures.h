//
//  eventsGestures.h
//
//  Created by Carles Gutierrez 2016 at http://unstatic.tech/
//  http://carlesgutierrez.github.io
//

#pragma once

#include "ofMain.h"

class eventsGestures : public ofEventArgs {
public:

	enum GestureEvent {
		GESTURE_EVENT_SIMPLETAP,
		GESTURE_EVENT_DOUBLETAP, 
		GESTURE_EVENT_PINCHPANNING
		//Add more here if this is usefull for the other gestures too
	};

	GestureEvent eventGestureType;

	//int idCurrentValue;//Add here the value to send in the gesture
	ofVec2f panningLoc = ofVec2f(0, 0);
	bool bPanning = false;

	ofVec2f pinchScale = ofVec2f(0, 0);
	bool bpinchPlus = false;
	bool bpinching = false;

	static ofEvent <eventsGestures> onGestureUpdate;
};