#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofBackground(200, 200, 200);
	ofSetVerticalSync(true);
	
	//Connect to Port
	myTuio.connect(3333);
	
	myImage.load("images/CarolinaGiraldo.jpg"); //picture made by Carolina Giraldo --> http://festivodesign.estampable.com/
	myImage.setAnchorPoint(myImage.getWidth()*0.5, myImage.getHeight()*0.5);

	lastImageLoc = ofVec2f(ofGetWidth()*0.5, ofGetHeight()*0.5);

	myTrickyGestures.setupPan(lastImageLoc, ofGetScreenWidth(), ofGetScreenHeight()); //Setup this pinch event to zoom in / out a scale Value.  Lets start at the original size
	myTrickyGestures.setupPinch(ofVec2f(1, 1)); //Setup this pinch event to zoom in / out a scale Value.  Lets start at the original size

	lastSizeItem = ofVec2f(myImage.getWidth(), myImage.getHeight());
}

//--------------------------------------------------------------
void ofApp::update(){

	myTrickyGestures.update();

	if (myTrickyGestures.isPanning()) {	
		ofVec2f auxPanVector = myTrickyGestures.getPanRelativeLocation();
		lastImageLoc = ofVec2f(auxPanVector.x, auxPanVector.y);// * myImage.getWidth(),  * myImage.getHeight()
		//cout << "myGestures.isPanning()!!! timer" << ofToString(ofGetElapsedTimeMillis(), 2) <<  " lastImageLoc = " << lastImageLoc << endl;
	}

	if (myTrickyGestures.isPinching(lastbPinchPlus)) {
		//cout << "PINCHIN!!! timer" << ofToString(ofGetElapsedTimeMillis(), 2) << endl;
		lastSizeItem = ofVec2f(myTrickyGestures.getPinchScale().x*ofGetWidth(), myTrickyGestures.getPinchScale().y*ofGetHeight()); //x and y equal values for the moment
		myImage.setAnchorPoint(lastSizeItem.x*0.5, lastSizeItem.y*0.5);//Update anchor point with the new scale factor
	}


}

//--------------------------------------------------------------
void ofApp::draw(){
	
	//render TUIO Cursors and Objects
	myTuio.drawCursors();
	myTuio.drawObjects();

	if (myTrickyGestures.isPinching(lastbPinchPlus)) {
		ofDrawBitmapString("Initial Dist Pinch  = " + ofToString(myTrickyGestures.distInitial), 20, 20);
		//ofDrawBitmapString("Actual Distance Pinch  = " + ofToString(myTrickyGestures.distActual), 20, 40);//TODO See why was Removed... 
		
		ofDrawBitmapString("Pinching point X =" + ofToString(myTrickyGestures.getPinchOrigin().x, 2) + " Y = " + ofToString(myTrickyGestures.getPinchOrigin().y, 2), 20, 80);
		ofDrawBitmapString("Scaling by " + ofToString(lastSizeItem, 2), 20, 100);
		//myImage.draw(ofGetWidth()*0.5, ofGetHeight()*0.5, lastSizeItem*myImage.getWidth(), lastSizeItem*myImage.getHeight());
		if (lastbPinchPlus) {
			ofDrawBitmapStringHighlight("Zoom In", 20, 120);
		}else ofDrawBitmapStringHighlight("Zoom Out", 20, 120);
	}
	
	ofDrawBitmapStringHighlight("Stats Distance Pinch  = " + ofToString(myTrickyGestures.myDiffFrameDistsStats.getAverage(FRAMES_AVG)), 20, 140);
	ofDrawBitmapStringHighlight("Stats Pan is Moving = " + ofToString(myTrickyGestures.myDiffFrameLocactionsStats.getAverage(FRAMES_AVG)), 20, 160);

	if (myTrickyGestures.isPanning()) {
			ofDrawBitmapString("Initial Pan Pos  = " + ofToString(myTrickyGestures.getPanOrigin()), 20, 20);
			ofDrawBitmapString("Dir Pan Vector = " + ofToString(myTrickyGestures.getPanDirection()), 20, 40);

			ofDrawBitmapString("New Pan Result X =" + ofToString(lastImageLoc.x, 2) + " Y = " + ofToString(lastImageLoc.y, 2), 20, 60);
			ofDrawBitmapStringHighlight("Panning", 20, 120);
		}

	ofEnableAlphaBlending();

	//myTrickyGestures.getPinchOrigin().x, myTrickyGestures.getPinchOrigin().y
	myImage.draw(lastImageLoc.x, lastImageLoc.y, lastSizeItem.x, lastSizeItem.y);

	ofDisableAlphaBlending();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}