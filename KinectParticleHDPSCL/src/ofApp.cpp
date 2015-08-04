#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	/* Kinect Setup */
	kinect.open();
	kinect.initColorSource();
	kinect.initBodySource();
	kinect.initDepthSource();
	kinectScale = 1;
	camera.setDistance(1);	// MOVING THE CAMERA CLOSER

	/* of Setup */
	ofSetVerticalSync(true);
	ofSetWindowShape(1920, 1080);
	
	
	/* PArticle Setup */
	int num = 1500;
	p.assign(num, demoParticle());
	currentMode = PARTICLE_MODE_ATTRACT;

	currentModeStr = "1 - PARTICLE_MODE_ATTRACT: attracts to mouse"; 

	resetParticles();
}

//--------------------------------------------------------------
void ofApp::resetParticles(){

	//these are the attraction points used in the forth demo 
	attractPoints.clear();
	for(int i = 0; i < 4; i++){
		attractPoints.push_back( ofPoint( ofMap(i, 0, 4, 100, ofGetWidth()-100) , ofRandom(100, ofGetHeight()-100) ) );
	}
	
	attractPointsWithMovement = attractPoints;
	
	for(unsigned int i = 0; i < p.size(); i++){
		p[i].setMode(currentMode);		
		p[i].setAttractPoints(&attractPointsWithMovement);;
		p[i].reset();
	}	
}

void ofApp::setHandCursor(ofVec3f pos){
	
	float displayX = 0;
	float displayY = 0;
	/*
	
	if(x > 0){
		displayX = 0.5 * ofGetWidth() + ofGetWidth()/4 * x; 
		if(displayX > ofGetWidth()){
			displayX = ofGetWidth();
		}
	}else{
		displayX = 0.5 * ofGetWidth() + ofGetWidth()/4 * x; 
		if(displayX < 0){
			displayX = 0;
		}
	}
	if(y > 0){
		displayY = 0.5 * ofGetHeight() - ofGetHeight()/2 * y; 
		if(displayY < 0){
			displayY = 0;
		}
		
	}else{
		displayY = 0.5 * ofGetHeight() - ofGetHeight()/2 * y; 
		if(displayY > ofGetHeight()){
			displayY = ofGetHeight();
		}
	}
	*/
	
	
	CameraSpacePoint csp;
	ColorSpacePoint colorSpacePoint ={0.0f, 0.0f};
	csp.X = pos.x;
	csp.Y = pos.y;
	csp.Z = pos.z;
	
	ICoordinateMapper* mapper;
	HRESULT hresult = kinect.getSensor()->get_CoordinateMapper(&mapper);
	if(FAILED(hresult)){
		ofLog() << "nix mapper da";
	}
	
	//mapper->MapCameraPointsToColorSpace(1, &csp, 1, &colorSpacePoint);
	hresult = mapper->MapCameraPointToColorSpace(csp, &colorSpacePoint);
	if(FAILED(hresult)){
		ofLog() << "nix mapper da";
	}
	
	
	p[0].setPosX(colorSpacePoint.X);
	p[0].setPosY(colorSpacePoint.Y);
}

//--------------------------------------------------------------
void ofApp::update(){
	// UDPATE ALL INITIALIZED KINECT STREAMS
	kinect.update();

	for(unsigned int i = 0; i < p.size(); i++){
		p[i].setMode(currentMode);
		p[i].update();
	}
	
	//lets add a bit of movement to the attract points
	for(unsigned int i = 0; i < attractPointsWithMovement.size(); i++){
		attractPointsWithMovement[i].x = attractPoints[i].x + ofSignedNoise(i * 10, ofGetElapsedTimef() * 0.7) * 12.0;
		attractPointsWithMovement[i].y = attractPoints[i].y + ofSignedNoise(i * -10, ofGetElapsedTimef() * 0.7) * 12.0;
	}	
}

//--------------------------------------------------------------
void ofApp::draw(){
	

    ofBackgroundGradient(ofColor(60,60,60), ofColor(10,10,10));

	for(unsigned int i = 0; i < p.size(); i++){
		p[i].draw();
	}
	
	ofSetColor(190);
	if( currentMode == PARTICLE_MODE_NEAREST_POINTS ){
		for(unsigned int i = 0; i < attractPoints.size(); i++){
			ofNoFill();
			ofCircle(attractPointsWithMovement[i], 10);
			ofFill();
			ofCircle(attractPointsWithMovement[i], 4);
		}
	}

	ofSetColor(230);	
	ofDrawBitmapString(currentModeStr + "\n\nSpacebar to reset. \nKeys 1-4 to change mode.", 10, 20);

	
	// WE ARE NOW SWITCHING TO THE 3D VIEW
	camera.begin();
	ofPushStyle();
	// FOR THIS MESH, I SCALED THE Z NEGATIVE TO MATCH DEPTH
	// THIS IS DEVELOPER PREFERENCE, BUT IT'S HOW I CONCEIVE
	// THE MESH...
	ofScale(10, 10, -10);

	// NOW DRAW OUR JOINTS IN THE SAME 3D SPACE
	drawJoints3D();



	ofPopStyle();

	camera.end();
}

void ofApp::drawJoints3D()
{
	// DRAW THE JOINTS IN A SALMON COLOR
	ofSetColor(240, 120, 90);

	ofVec3f pos;

	// THERE IS A MAXIMUM OF 6 BODIES TRACKED BY KINECT
	for (int i = 0; i<6; i++){
		// IF THE BODY IS BEING TRACKED...
		if (this->kinect.getBodySource()->getBodies()[i].tracked){

			auto b = this->kinect.getBodySource()->getBodies()[i];
			HandState leftHandState = b.leftHandState;
			

			// ITERATE THROUGH ALL JOINTS IN THE TRACKED BODY...
			for (std::map<JointType, ofxKFW2::Data::Joint>::iterator it = b.joints.begin(); it != b.joints.end(); ++it)
			{
				if (it->second.getTrackingState() == TrackingState_Tracked)
				{
					// GRAB THE JOINT'S 3D POSITION
					pos = it->second.getPosition();


					if(it->second.getType() == JointType_HandLeft){
						if(leftHandState == HandState::HandState_Closed)
							ofSetColor(255,0,0);
						else
							ofSetColor(240, 120, 90);
					}
					else if(i == 0){
						ofSetColor(50, 255, 50);
					}
					else if(i == 1){
						ofSetColor(250, 255, 50);
					}
					else if(i == 2){
						ofSetColor(25, 255, 250);
					}

					if(it->second.getType() == JointType_HandRight)
						setHandCursor(pos);

					// AND RENDER. JOINTS AND THE MESH POINTS SHARE THE 
					// SAME DEPTH SPACE COORDINATES SO NO COORDINATE MAPPING
					// IS NEEDED FOR THIS. 
					ofBox(pos.x*kinectScale, pos.y*kinectScale, pos.z*kinectScale, .01, .01, .01);
				}
			}
		}
	}
	ofSetColor(255);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if( key == '1'){
		currentMode = PARTICLE_MODE_ATTRACT;
		currentModeStr = "1 - PARTICLE_MODE_ATTRACT: attracts to mouse"; 		
	}
	if( key == '2'){
		currentMode = PARTICLE_MODE_REPEL;
		currentModeStr = "2 - PARTICLE_MODE_REPEL: repels from mouse"; 				
	}
	if( key == '3'){
		currentMode = PARTICLE_MODE_NEAREST_POINTS;
		currentModeStr = "3 - PARTICLE_MODE_NEAREST_POINTS: hold 'f' to disable force"; 						
	}
	if( key == '4'){
		currentMode = PARTICLE_MODE_NOISE;
		currentModeStr = "4 - PARTICLE_MODE_NOISE: snow particle simulation"; 						
		resetParticles();
	}	
		
	if( key == ' ' ){
		resetParticles();
	}
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

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
