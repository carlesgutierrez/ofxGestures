//
//  ofxTrickyGestures.cpp
//
//  Carles Gutierrez - Touch Gestures
//
//

#include "ofxTrickyGestures.h"

ofxTrickyGestures & ofxTrickyGestures::get() {
	static ofxTrickyGestures * instance = new ofxTrickyGestures();
	return *instance;
}

ofxTrickyGestures::ofxTrickyGestures() {


	m_isPanning = false;
	m_isPinching = false;

	ofAddListener(ofEvents().touchDown, this, &ofxTrickyGestures::touchDown, OF_EVENT_ORDER_BEFORE_APP);
	ofAddListener(ofEvents().touchMoved, this, &ofxTrickyGestures::touchMoved, OF_EVENT_ORDER_BEFORE_APP);
	ofAddListener(ofEvents().touchUp, this, &ofxTrickyGestures::touchUp, OF_EVENT_ORDER_BEFORE_APP);

	ofAddListener(ofxSimpleTimer::TIMER_COMPLETE, this, &ofxTrickyGestures::timerCompelte);
	myDiffFrameDistsStats.setup(FRAMES_AVG, 0, 1);
	myDiffFrameLocactionsStats.setup(FRAMES_AVG, 0, 1); // 0 means no move, and 1 mean moving.
	myPinchingStats.setup(FRAMES_AVG, 0, 1); // 0 means no pinchin, and 1 mean pinchin.

	//Gui
	gui.setup("Gestures Touch Tunning");
	gui.add(timerGestureUpdate.setup("Millis update", 33, 10, 100));
	gui.add(scalePanTunning.setup("Range Pannig", 1, 0.01, 10));
	gui.add(scalePinchTunning.setup("Range Pinching", 1, 0.5, 5));

	gui.add(minStatsGestureReact.setup("Min Gest and isPinch", 0.005, 0.001, 0.01));
	gui.add(thersholdMinMoveReact.setup("thers isMoving", 0.000001, 0.0000001, 0.01));
	gui.add(minStatsGesturePanReact.setup("min Gesture Pan", 0.13, 0.001, 0.5));
	gui.add(thersholdMinPinchingReact.setup("min Gest Pinch while", 0.13, 0.01, 0.5));


	gui.loadFromFile("settings.xml");

	//Finally Set the Timer that will controll the touch processe at the right time
	//Timer and the statistics realated to measure the status of Gesture in the right frameTime
	_timer = new ofxSimpleTimer();
	_timer->setTime(timerGestureUpdate, 0);//Infinite loop at each 33 milliseconds ->
	_timer->setName("myStatsTimer");
	_timer->start();// start


	//other timers
	singleTap.initSingleTapTime = ofGetElapsedTimeMillis();
	singleTap.initDoubleTapTime = ofGetElapsedTimeMillis();
}

ofxTrickyGestures::~ofxTrickyGestures()
{
	ofRemoveListener(ofEvents().touchDown, this, &ofxTrickyGestures::touchDown, OF_EVENT_ORDER_BEFORE_APP);
	ofRemoveListener(ofEvents().touchMoved, this, &ofxTrickyGestures::touchMoved, OF_EVENT_ORDER_BEFORE_APP);
	ofRemoveListener(ofEvents().touchUp, this, &ofxTrickyGestures::touchUp, OF_EVENT_ORDER_BEFORE_APP);
}

//-------------------------------------------------------------
void ofxTrickyGestures::setupSingleTap(int timerSingle, int timerDouble) {
	singleTap.setup(timerSingle, timerDouble);
}
//-------------------------------------------------------------
void ofxTrickyGestures::setupPinch(ofVec2f sizeItem) {
	pinch.setup(sizeItem);
}

//-------------------------------------------------------------
void ofxTrickyGestures::setupPan(ofVec2f locItem, int width, int height) {
	pan.setup(locItem, width, height);
}

//--------------------------------------------------------------
//TODO call this event from Internal Update event. Meanwhile call this Update from the App. 
void ofxTrickyGestures::update() {
	_timer->update();

	//No way so just simgle tap
	if (ofGetElapsedTimeMillis() - singleTap.initDoubleTapTime > singleTap.timerWhileDoubleTapActive) {

		if(singleTap.bIsSingleTap && !singleTap.bIsDoubleTap){
			eventsGestures auxEventGesture;//External Uses events register ofTouchEvent Double and Sigle TAP!!
			auxEventGesture.eventGestureType = eventsGestures::GESTURE_EVENT_SIMPLETAP;
			ofNotifyEvent(eventsGestures::onGestureUpdate, auxEventGesture);

			singleTap.bIsSingleTap = false;
		}

		singleTap.bIsDoubleTap = false;
	}
}



//--------------------------------------------------------------
void ofxTrickyGestures::timerCompelte(string &name)
{
	if (name == "myStatsTimer")
	{
		//add udpate value to stats
		if (myTouchesVector.size() == 1) {

			//Calcs bettween finguers locations ( positions )
			myDiffFrameLocactionsStats.update(getIsAllTouchsMoving()); //Udpating between 0s and 1s. 

			if (!m_isPinching) {
				updatePanGestureRecognition();
			}

			myPinchingStats.update(0);
			myDiffFrameDistsStats.update(0);

		}
		else if (myTouchesVector.size() > 1 && myTouchesVector.size() < 3) { //For Pinch and Pan, between 2 and 3 finguers! // TODO Check more options if 2 , 3, or more fingers.

			//Calcs bettween finguers separations movements ( distances )
			float aux_diff_Pinchdistances = last_PinchdistActual - distPinchActual;		//Compare to the last frame //TODO compart to more than one frame?
			myDiffFrameDistsStats.update(abs(aux_diff_Pinchdistances));	//If diference levels are changing contanstly then mean there are Pinch Actions.
			last_PinchdistActual = distPinchActual;						//then update last
			myPinchingStats.update(getIsPinchigMoving());


			//Calcs bettween finguers locations ( positions )
			myDiffFrameLocactionsStats.update(getIsAllTouchsMoving()); //Udpating between 0s and 1s. 

			if (!m_isPinching) {
				updatePanGestureRecognition();
			}
			if (!m_isPanning) {
				updatePinchGestureRecognition();
			}

		}
		else {

			myPinchingStats.update(0);
			myDiffFrameDistsStats.update(0);
			myDiffFrameLocactionsStats.update(0); //Udpating between 0s and 1s. 
			m_isPreGesture = true; //Reset to find a new gesture

			//reset data gesttures
			if (m_isPanning)resetPan();
			if (m_isPinching)resetPinching();
		}

	}
}

//--------------------------------------------------------------------
void ofxTrickyGestures::resetPan() {
	m_isPanning = false;
	pan.reset();
}

//--------------------------------------------------------------------
void ofxTrickyGestures::resetPinching() {
	m_isPinching = false;
	pinch.reset();
}

//--------------------------------getRelativeLocation------------------------------------
void ofxTrickyGestures::updatePanGestureRecognition() {
	//Small check into Pinching too. If fingers are pinching might need to give a pinching chance.
	if (myPinchingStats.getAverage(FRAMES_AVG) > thersholdMinPinchingReact) { //TODO make this GUI editable thershold_Pinching_status_react
		//initialResetPinchingVars();
		resetPan();
		return;
	}

	//First time Panning, then Memorize initial values to start
	if (myDiffFrameLocactionsStats.getAverage(FRAMES_AVG) > minStatsGestureReact && !m_isPanning) {
		m_isPanning = true;
		m_isPinching = false;
		pan.originPan = pan.locPan;// locInitial;
	}
	else if (m_isPanning) {

		pan.dirPan = (locPanInitial - locFingerActual*ofVec2f(pan.w, pan.h))*scalePanTunning;
		//Apply diff between finger and locPan
		ofVec2f auxOriginPan = diff_finger2locPan + pan.originPan;

		//Quick fix over this vector
		pan.dirPan.x = pan.dirPan.x * -1;
		pan.dirPan.y = pan.dirPan.y * -1;
		pan.locPan = auxOriginPan + pan.dirPan;

		/*
		cout << "*********************** updatePanGestureRecognition ************************" << endl;
		cout << "locPanInitial =  " << locPanInitial << endl;
		cout << "locFingerActual =  " << locFingerActual*ofVec2f(pan.w, pan.h) << "and scalePanTunning = " << scalePanTunning << endl;
		cout << "pan.dirPan =  " << pan.dirPan << endl;
		cout << "auxOriginPan =  " << auxOriginPan << endl;
		cout << "pan.locPan = " << pan.locPan << endl;
		cout << "*********************** ///////////////////////// ************************" << endl;
		*/
	}
}

//--------------------------------------------------------------------
void ofxTrickyGestures::initialResetPinchingVars() {
	m_isPinching = true; //DETECTED pinch
	pinch.distOrigin = distInitial;
	pinch.distActual = distPinchActual;
	pinch.originPinch = getMiddlePosTouches();// from here and avoid Jump values effect
	pinch.itemSize = pinch.itemSizeScaled;
}
//--------------------------------------------------------------------
void ofxTrickyGestures::updatePinchGestureRecognition() {

	//Small check into Panning too. If fingers are really panning might need to give a pinching chance.
	if (myDiffFrameLocactionsStats.getAverage(FRAMES_AVG) > minStatsGestureReact && myDiffFrameDistsStats.getAverage(FRAMES_AVG) < minStatsGestureReact) { //TODO make this GUI editable thershold_Pinching_status_react

		m_isPanning = true;//initialResetPanningVars:
		pan.originPan = pan.locPan;

		resetPinching();

		return;
	}

	//First time Pinching, then Memorize initial values to start
	if (myPinchingStats.getAverage(FRAMES_AVG) > minStatsGestureReact && !m_isPinching) {
		initialResetPinchingVars();

	}
	else if (bCasePinchOneFingerLeftKeep) {
		initialResetPinchingVars();
		bCasePinchOneFingerLeftKeep = false;
	}
	else if (m_isPinching) {
		pinch.distActual = distPinchActual;
	}

}

//--------------------------------------------------------------
void ofxTrickyGestures::touchDown(ofTouchEventArgs & touch) {

	bool attended = false;

	if (touch.id > 0) { //That always happends... 
		attended = true;
		myTouchesVector.push_back(touch);
	}

	if (attended) {

		/////////////////////
		//Just 1 finger Down
		if (myTouchesVector.size() == 1) {

			if (last_numTouchesActive == 0) { //First one
				locPanInitial = pan.locPan;
				locFingerActual = getFirstPosTouches();
				last_locFingerActual = locFingerActual;
				//Diff between the pan.locPan and the control finguer 1
				//in other to be able to move our pan from where that was loc
				diff_finger2locPan = pan.locPan - ofVec2f(locFingerActual.x * pan.w, locFingerActual.y * pan.h);
			}


			///////////////////////////////////
			// Single and Double tap cases
			//Just 1 finger touchDown after previous tap interaction -> bIsSingleTap
			if (myTouchesVector.size() == 1) {
				if (last_numTouchesActive == 0) { //First one

					//give a chance to Single or DoubleTap
					//start a timer singleTap
					singleTap.initSingleTapTime = ofGetElapsedTimeMillis();

					//still in fisrt tap timer
					if (ofGetElapsedTimeMillis() - singleTap.initSingleTapTime < singleTap.timerWhileSingleTapActive) {
						
						//if single was previous activated check doutbletap timers
						if (singleTap.bIsSingleTap) {//then lets check double TAP

							if (ofGetElapsedTimeMillis() - singleTap.initDoubleTapTime < singleTap.timerWhileDoubleTapActive) {//Double tap seems to requiere a bit more time than singleTap

								eventsGestures auxEventGesture;//External Uses events register ofTouchEvent Double and Sigle TAP!!
								auxEventGesture.eventGestureType = eventsGestures::GESTURE_EVENT_DOUBLETAP;
								ofNotifyEvent(eventsGestures::onGestureUpdate, auxEventGesture);

								//reset
								singleTap.bIsDoubleTap = true;
								singleTap.bIsSingleTap = false;
							}
						}
					}
				}
			}

			///////////////////////////////////

		}
		else if (myTouchesVector.size() > 1 && last_numTouchesActive > 0) {

			distPinchActual = getNormDistanceTouches();
			last_PinchdistActual = distPinchActual;
			distInitial = distPinchActual;

		}

	}

	//mem last number of touchs processed
	last_numTouchesActive = myTouchesVector.size();
}

//--------------------------------------------------------------
void ofxTrickyGestures::touchMoved(ofTouchEventArgs & touch) {

	bool attended = false;

	attended = updateTouchVectorData(touch); //udpating that specific touch info

	if (attended) {

		//Update Finger locations
		last_locFingerActual = locFingerActual;
		locFingerActual = getFirstPosTouches(); //getting finger location

		// So > 1 fingers. Usefull for pinch.
		if (myTouchesVector.size() > 1) {

			if (m_isPreGesture) {
				m_isPreGesture = false;
				distInitial = getNormDistanceTouches();
			}
			else {
				distPinchActual = getNormDistanceTouches();
			}
		}

		//mem last number of touchs processed
		last_numTouchesActive = myTouchesVector.size();

		//Send the events gestures
		eventsGestures auxEventGesture;//External Uses events register ofTouchEvent Double and Sigle TAP!!
		auxEventGesture.eventGestureType = eventsGestures::GESTURE_EVENT_PINCHPANNING;
		auxEventGesture.panningLoc = pan.getRelativeLocation();
		auxEventGesture.bPanning = isPanning();

		auxEventGesture.pinchScale = pinch.getRelativeScale();
		bool pinchDir;// = pinch.isZoomIn();
		bool bPinching = isPinching(pinchDir);
		auxEventGesture.bpinchPlus = pinchDir;
		auxEventGesture.bpinching = bPinching;
		ofNotifyEvent(eventsGestures::onGestureUpdate, auxEventGesture);
	}

}

//---------------------------------------------------------
void ofxTrickyGestures::touchUp(ofTouchEventArgs & touch) {


	bool attended = false;
	for (int i = 0; i < myTouchesVector.size(); i++) {
		if (myTouchesVector[i].id == touch.id) {
			//Found then remove it
			myTouchesVector.erase(myTouchesVector.begin() + i);
			attended = true;
		}
	}

	if (attended) {

		/////////////////////
		//Just 0 finger touchUp
		if (myTouchesVector.size() == 0) {

			//check first timer pf singleTap
			if (ofGetElapsedTimeMillis() - singleTap.initSingleTapTime < singleTap.timerWhileSingleTapActive) {
				if (!singleTap.bIsSingleTap && !singleTap.bIsDoubleTap) { // If this time is not doubleTap lets activate singleTap
					singleTap.initDoubleTapTime = ofGetElapsedTimeMillis();
					singleTap.bIsSingleTap = true; //check first Candidate to single tap in update process
				}
			}
		
			
			//special case for pinching remaingin one finger
			 if(m_isPreGesture) {
				bCasePinchOneFingerLeftKeep = true;
			}
		
		}

		/////////////////////
		//Just 1 finger touchUp
		if (myTouchesVector.size() == 1) {//&& !m_isPreGesture

			if (last_numTouchesActive > 1) {

				//Panning rol, just One finger
				locFingerActual = getFirstPosTouches();
				last_locFingerActual = locFingerActual;//reset new last location
				diff_finger2locPan = pan.locPan - ofVec2f(locFingerActual.x * pan.w, locFingerActual.y * pan.h);//recalc Diff with the pan.locPan

				for (int i = 0; i < FRAMES_AVG; i++)myDiffFrameDistsStats.update(0); // this will auto reset the //m_isPinching = false; State
				bCasePinchOneFingerLeftKeep = true;//Special case for pinching intitializations when 1 Finger remain there doing zoom / pinching
			}
		}

		//mem last number of touchs processed
		last_numTouchesActive = myTouchesVector.size();
	}

}
//-------------------------------------------------------------
bool ofxTrickyGestures::updateTouchVectorData(ofTouchEventArgs & touch) {

	bool bFind = false;

	//Update all vector values with actual teouch events
	for (int i = 0; i < myTouchesVector.size(); i++) {
		if (myTouchesVector[i].id == touch.id) {
			myTouchesVector[i] = touch;
			bFind = true;
		}
	}

	return bFind;
}
//---------------------------------------
ofVec2f ofxTrickyGestures::getFirstPosTouches() {
	//OF centroid calc ofvec2f
	ofVec2f centroid = ofVec2f(0, 0);
	int num = myTouchesVector.size();

	if (0 == num) {
		return ofVec2f(-1, -1);
	}
	else {
		//lets just return the fisrt touch
		centroid.x += myTouchesVector[0].x;
		centroid.y += myTouchesVector[0].y;
	}

	return centroid;
}

//---------------------------------------
ofVec2f ofxTrickyGestures::getMiddlePosTouches() {
	//OF centroid calc ofvec2f
	ofVec2f centroid = ofVec2f(0, 0);
	int num = myTouchesVector.size();

	if (0 == num) {
		return ofVec2f(-1, -1);
	}
	else {

		for (int i = 0; i < num; i++) {
			centroid.x += myTouchesVector[i].x;
			centroid.y += myTouchesVector[i].y;
		}
		centroid.x /= num;
		centroid.y /= num;
	}

	return centroid;
}
//---------------------------------------
float ofxTrickyGestures::getNormDistanceTouches() {

	float allSumDistances = 0.0;
	float allNormDistances = 0;

	//Check distance bettweem points in line. Just this should work?
	for (int i = 0; i < myTouchesVector.size(); i++) {
		ofVec2f pos = ofVec2f(myTouchesVector[i].x, myTouchesVector[i].y);
		if (i > 0) {
			ofVec2f Prepos = ofVec2f(myTouchesVector[i - 1].x, myTouchesVector[i - 1].y);
			float dist = pos.distanceSquared(Prepos);
			allSumDistances += dist;
		}
	}

	//Normalized Dist Calc
	if (myTouchesVector.size() != 0)allNormDistances = allSumDistances / myTouchesVector.size();
	else allNormDistances = 0;


	return allNormDistances;
}

//--------------------------------------------------------
int ofxTrickyGestures::getIsPinchigMoving() {

	int bPinching = 0;

	if (myDiffFrameDistsStats.getAverage(FRAMES_AVG) > minStatsGestureReact) {
		bPinching = 1;
	}
	else {
		bPinching = 0;
	}

	return bPinching;
}

//--------------------------------------------------------
int ofxTrickyGestures::getIsAllTouchsMoving() {

	int bMoving = 0;

	ofVec2f pos = ofVec2f(locFingerActual);
	ofVec2f last_pos = ofVec2f(last_locFingerActual);

	double dist_moved = pos.squareDistance(last_pos);

	if (dist_moved > thersholdMinMoveReact) {
		bMoving = 1;
	}
	else {
		bMoving = 0;
	}

	return bMoving;
}



//--------------------------------------------------
ofVec2f ofxTrickyGestures::getPanOrigin() {
	return pan.getOrigin();
}

//--------------------------------------------------
ofVec2f ofxTrickyGestures::getPanDirection() {
	return pan.getPanDirection();
}

//--------------------------------------------------
ofVec2f ofxTrickyGestures::getPanRelativeLocation() {
	ofVec2f auxPanDelta;
	auxPanDelta = pan.getRelativeLocation();// + (pan.locInitial - pan.getOrigen());
	return auxPanDelta;
}

//--------------------------------------------------
ofVec2f ofxTrickyGestures::getPinchOrigin()
{
	return pinch.originPinch;
}

//--------------------------------------------------
ofVec2f ofxTrickyGestures::getPinchScale()
{
	ofVec2f auxScale = scalePinchTunning * pinch.getRelativeScale();
	return auxScale;
}



