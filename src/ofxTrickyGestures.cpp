//
//  ofxTrickyGestures.cpp
//
//  Carles Gutierrez - Touch Gestures
//
//

#include "ofxTrickyGestures.h"

ofxTrickyGestures & ofxTrickyGestures::get(){
	static ofxTrickyGestures * instance = new ofxTrickyGestures();
	return *instance;
}

ofxTrickyGestures::ofxTrickyGestures():
m_isPanning(false),
m_isPinching(false)
{
    ofAddListener(ofEvents().touchDown, this, &ofxTrickyGestures::touchDown, OF_EVENT_ORDER_BEFORE_APP);
	ofAddListener(ofEvents().touchMoved, this, &ofxTrickyGestures::touchMoved, OF_EVENT_ORDER_BEFORE_APP);
    ofAddListener(ofEvents().touchUp, this, &ofxTrickyGestures::touchUp, OF_EVENT_ORDER_BEFORE_APP);

	//Timer and the statistics realated to measure the status of Gesture in the right frameTime
	_timer = new ofxSimpleTimer();
	_timer->setTime(TIMER_GESTURES_UPDATE, 0);//Infinite loop at each 33 milliseconds -> 
	_timer->setName("myStatsTimer");
	_timer->start();// start

	ofAddListener(ofxSimpleTimer::TIMER_COMPLETE, this, &ofxTrickyGestures::timerCompelte);
	myDiffFrameDistsStats.setup(FRAMES_AVG, 0, 1);
	myDiffFrameLocactionsStats.setup(FRAMES_AVG, 0, 1); // 0 means no move, and 1 mean moving.
}

ofxTrickyGestures::~ofxTrickyGestures()
{
    ofRemoveListener(ofEvents().touchDown, this, &ofxTrickyGestures::touchDown, OF_EVENT_ORDER_BEFORE_APP);
	ofRemoveListener(ofEvents().touchMoved, this, &ofxTrickyGestures::touchMoved, OF_EVENT_ORDER_BEFORE_APP);
    ofRemoveListener(ofEvents().touchUp, this, &ofxTrickyGestures::touchUp, OF_EVENT_ORDER_BEFORE_APP);
}

//-------------------------------------------------------------
void ofxTrickyGestures::setup(ofVec2f sizeItem) {
	pinch.setup(sizeItem);
}

//--------------------------------------------------------------
//TODO call this event from Internal Update event. Meanwhile call this Update from the App. 
void ofxTrickyGestures::update() {
	_timer->update(); //cout << "timer Update" << endl;
}


//--------------------------------------------------------------
void ofxTrickyGestures::timerCompelte(string &name)
{
	//cout << "timerCompelte " << name << endl;
	if (name == "myStatsTimer")
	{
		//add udpate value to stats
		if (myTouchesVector.size() > 1 && myTouchesVector.size() < 3) { //For Pinch and Pan, between 2 and 3 finguers! // TODO Check more options if 2 , 3, or more fingers.
			
			//Calcs bettween fingers distances
			last_distActual = last_distActual - distActual;		//Compare to the last frame //TODO compart to more than one frame?
			myDiffFrameDistsStats.update(abs(last_distActual));			//If diference levels are changing contanstly then mean there are Pinch Actions.
			last_distActual = distActual;						//then update last

			//Calcs bettween finguers locations ( positions )
			myDiffFrameLocactionsStats.update(getIsAllTouchsMoving()); //Udpating between 0s and 1s. 

			//PRIORITY ORDER: DOUBLE TAP / PAN / PINCH

			//else if(myDiffFrameDistsStats.getAverage(FRAMES_AVG) < MINSTATS_GESTURE_REACT &&  !m_isPanning)

			if (!m_isDoubleTap && !m_isPinching) {
				updatePanGestureRecognition();
			}
			if (!m_isPanning && !m_isDoubleTap) {
				updatePinchGestureRecognition();
			}
			
			
		}
		else {
			myDiffFrameDistsStats.update(0);
			m_isPreGesture = true; //Reset to find a new gesture

			//reset data gesttures
			if(m_isPanning)resetPan();
			if(m_isPinching)resetPinching();
		}
	}
}

//--------------------------------------------------------------------
void ofxTrickyGestures::resetPan() {
	m_isPanning = false;
	cout << "resetPan()" << endl;
	pan.reset();
}

//--------------------------------------------------------------------
void ofxTrickyGestures::resetPinching() {
	m_isPinching = false;
	cout << "resetPinching()" << endl;
	pinch.reset();
}

//--------------------------------------------------------------------
void ofxTrickyGestures::updatePanGestureRecognition() {
	//First time Panning, then Memorize initial values to start
	if (myDiffFrameDistsStats.getAverage(FRAMES_AVG) < MINSTATS_GESTURE_REACT && myDiffFrameLocactionsStats.getAverage(FRAMES_AVG) > MINSTATS_GESTURE_PAN_REACT && !m_isPanning) {
		cout << " myDiffFrameLocactionsStats.getAverage(FRAMES_AVG) = " << myDiffFrameLocactionsStats.getAverage(FRAMES_AVG) << endl;

		m_isPanning = true; //DETECTED pan if it's moving that finguers paralele for a some time. So we need stats paning is moving. Or diffence moving last por to new pos
		pan.originPan = getMiddlePosTouches();
		pan.locPan = pan.originPan;
		pan.dirPan = (pan.locPan - pan.originPan); //TODO check between 0 and 1¿?

	}
	else if (m_isPanning) { //myDiffFrameDistsStats.getAverage(FRAMES_AVG) < MINSTATS_GESTURE_REACT
		pan.locPan = locActual;
		pan.dirPan = (pan.locPan - pan.originPan); //TODO check between 0 and 1¿?
	}
}

//--------------------------------------------------------------------
void ofxTrickyGestures::updatePinchGestureRecognition() {
	
	//First time Pinching, then Memorize initial values to start
	if (myDiffFrameDistsStats.getAverage(FRAMES_AVG) > MINSTATS_GESTURE_REACT && !m_isPinching) {
		//distInitial = getNormDistanceTouches(); 

		m_isPinching = true; //DETECTED pinch
		pinch.distOrigin = distInitial;
		pinch.distActual = distActual;
		pinch.originPinch = getMiddlePosTouches();// from here and avoid Jump values effect
		pinch.itemSize = pinch.itemSizeScaled;

	}
	else if (m_isPinching) { //myDiffFrameDistsStats.getAverage(FRAMES_AVG) > MINSTATS_GESTURE_REACT &&
		pinch.distActual = distActual;
	}

}

bool ofxTrickyGestures::touchDown(ofTouchEventArgs & touch) {
    
	bool attended = false;

	if (touch.id > 0) { //That always happends... 
		attended = true;
		myTouchesVector.push_back(touch);
	}

    return attended;
};

bool ofxTrickyGestures::touchMoved(ofTouchEventArgs & touch) {

	bool attended = false;

	//update my touch events and update related variables

	//TODO PAN FOR 1 Item too

	if (myTouchesVector.size() > 1 && m_isPreGesture) { //That might the moment to save a init gesture
		m_isPreGesture = false;
		
		//{c}Real initial Distance and Location for gestures in action are saved at Update Process when 
		//there is enought info to say that is this gesture happening

		distInitial = getNormDistanceTouches();

		locInitial = getMiddlePosTouches();
		last_locActual = locActual = locInitial;

	}
	else if (myTouchesVector.size() > 1 && !m_isPreGesture) {//m_isPrePinching
		//Update all vector values with actual teouch events
		for (int i = 0; i < myTouchesVector.size(); i++) {
			if (myTouchesVector[i].id == touch.id) {
				myTouchesVector[i] = touch;
			}
		}

		//tricky Data touch updates
		distActual = getNormDistanceTouches();
		last_locActual = locActual;
		locActual = getMiddlePosTouches();
	}

    return attended;
};


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

	//cout << "Norm dist is" << allNormDistances << endl;

	return allNormDistances;
}

//--------------------------------------------------------
int ofxTrickyGestures::getIsAllTouchsMoving() {

	int bMoving = 0;

	ofVec2f pos = ofVec2f(locActual);
	ofVec2f last_pos = ofVec2f(last_locActual);
	
	double dist_moved = pos.squareDistance(last_pos);

	if (dist_moved > THERSHOLD_MINMOVEMENT_REACT) {
		bMoving = 1;
	}
	else {
		bMoving = 0;
	}

	//cout << "getIsAllTouchsMoving is " << bMoving << " dist_moved = " << dist_moved << endl;
	return bMoving;
}

//---------------------------------------------------------
bool ofxTrickyGestures::touchUp(ofTouchEventArgs & touch) {

	bool attended = false;
	for (int i = 0; i < myTouchesVector.size(); i++) {
		if (myTouchesVector[i].id == touch.id) {
			//Found then remove it
			myTouchesVector.erase(myTouchesVector.begin() + i);
			attended = true;
		}
	}

    return attended;
};

//--------------------------------------------------
ofVec2f ofxTrickyGestures::getPanOrigin() {
	return pan.getOrigin();
}

//--------------------------------------------------
ofVec2f ofxTrickyGestures::getPanDelta(){
	ofVec2f auxPanDelta;
	auxPanDelta = pan.getRelativeLocation();
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
	return pinch.getRelativeScale();
}

/*
ofVec2f ofxTrickyGestures::getPinchDelta() const
{
	ofVec2f auxDelta;
	//auxDelta = (m_pinchCurrent2 + m_pinchCurrent1 - m_pinchOrigin2 - m_pinchOrigin1) / 2.0;
    return auxDelta;
}


double ofxGestures::getPinchAngle() const
{
    ofVec2f currentDelta = m_pinchCurrent2 - m_pinchCurrent1;
    ofVec2f originDelta = m_pinchOrigin2 - m_pinchOrigin1;
    
    double currentLength = sqrt(pow(currentDelta.x, 2) + pow(currentDelta.y, 2));
    double originLength = sqrt(pow(originDelta.x, 2) + pow(originDelta.y, 2));
    
    double currentAngle = ofRadToDeg(asin(currentDelta.x / currentLength));
    double originAngle = ofRadToDeg(asin(originDelta.x / originLength));
    
    if (m_pinchCurrent2.y > m_pinchCurrent1.y) currentAngle = 180.0 - currentAngle;
    if ( m_pinchOrigin2.y >  m_pinchOrigin1.y) originAngle = 180.0 - originAngle;
    
    return currentAngle - originAngle;
}

double ofxGestures::getPinchScale() const
{
    ofVec2f currentDelta = m_pinchCurrent2 - m_pinchCurrent1;
    ofVec2f originDelta = m_pinchOrigin2 - m_pinchOrigin1;
    
    double currentLength = sqrt(pow(currentDelta.x, 2) + pow(currentDelta.y, 2));
    double originLength = sqrt(pow(originDelta.x, 2) + pow(originDelta.y, 2));
    
    return currentLength / originLength;
}


ofVec2f ofxGestures::getPinchPrevious() const{
    return (m_pinchPrevious2 + m_pinchPrevious1) / 2.0;
}

ofVec2f ofxGestures::getPinchRelativeDelta() const{
    return (m_pinchCurrent2 + m_pinchCurrent1 - m_pinchPrevious2 - m_pinchPrevious1) / 2.0;
}

double ofxGestures::getPinchRelativeAngle() const{
    ofVec2f currentDelta = m_pinchCurrent2 - m_pinchCurrent1;
    ofVec2f previousDelta = m_pinchPrevious2 - m_pinchPrevious1;

    double currentLength = sqrt(pow(currentDelta.x, 2) + pow(currentDelta.y, 2));
    double previousLength = sqrt(pow(previousDelta.x, 2) + pow(previousDelta.y, 2));

    double currentAngle = ofRadToDeg(asin(currentDelta.x / currentLength));
    double previousAngle = ofRadToDeg(asin(previousDelta.x / previousLength));

    if (m_pinchCurrent2.y > m_pinchCurrent1.y) currentAngle = 180.0 - currentAngle;
    if (m_pinchPrevious2.y >  m_pinchPrevious1.y) previousAngle = 180.0 - previousAngle;

    return currentAngle - previousAngle;
}

double ofxGestures::getPinchRelativeScale() const{
    ofVec2f currentDelta = m_pinchCurrent2 - m_pinchCurrent1;
    ofVec2f previousDelta = m_pinchPrevious2 - m_pinchPrevious1;

    double currentLength = sqrt(pow(currentDelta.x, 2) + pow(currentDelta.y, 2));
    double previousLength = sqrt(pow(previousDelta.x, 2) + pow(previousDelta.y, 2));

    return currentLength / previousLength;
}

ofVec2f ofxGestures::PinchEvent::getOrigin() const{
	return ofxGestures::get().getPinchOrigin();
}

ofVec3f ofxGestures::PinchEvent::getPrevious() const{
	return ofxGestures::get().getPinchPrevious();
}

ofVec2f ofxGestures::PinchEvent::getDelta() const{
	return ofxGestures::get().getPinchDelta();
}

ofVec2f ofxGestures::PinchEvent::getRelativeDelta() const{
	return ofxGestures::get().getPinchRelativeDelta();
}

double ofxGestures::PinchEvent::getAngle() const{
	return ofxGestures::get().getPinchAngle();
}

double ofxGestures::PinchEvent::getScale() const{
	return ofxGestures::get().getPinchScale();
}

double ofxGestures::PinchEvent::getRelativeAngle() const{
	return ofxGestures::get().getPinchRelativeAngle();
}

double ofxGestures::PinchEvent::getRelativeScale() const{
	return ofxGestures::get().getPinchRelativeScale();
}

ofVec2f ofxGestures::PanEvent::getOrigin() const{
	return ofxGestures::get().getPanOrigin();
}

ofVec2f ofxGestures::PanEvent::getDelta() const{
	return ofxGestures::get().getPanDelta();
}
*/


