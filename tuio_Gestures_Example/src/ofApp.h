#pragma once

#include "ofMain.h"
#include "ofxTuioClient.h"
#include "ofxTrickyGestures.h"


class ofApp : public ofBaseApp{
	
public:
	void setup();
	void update();
	void draw();
	
	void keyPressed  (int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	
	ofxTuioClient myTuio;
	
	ofImage myImage;
	ofVec2f lastSizeItem;
	bool lastbPinchPlus;
	ofVec2f lastImageLoc;
	ofxTrickyGestures myTrickyGestures;


	
};

