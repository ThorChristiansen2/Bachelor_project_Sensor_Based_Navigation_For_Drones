 #include <iostream>
#include <say-hello/hello.hpp>
#include "mainCamera.hpp"
#include <unistd.h>

// Include directories for raspicam
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <raspicam/raspicam_cv.h>

//#include "pthread.h"
//#include <cstdlib>

/* ########################
 * Name: mainCamera.cpp
 * Made by: Thor Christiansen - s173949 
 * Date: 29.06.2020
 * Objective: The source file mainCamera.cpp contains the functions used
 * by main.cpp to treat the images - find features in the images using 
 * Harris corner etc.
 * Project: Bachelor project 2020
 * 
 * Problems: 
 * Sæt angle threshold 
 * OVERSkyggende problem: Det ser ud til, at der er et problem med matricen K, som skal kalibreres.
 * Lær hvordan man SSH'er ind på raspberry pi'en
 * Units seem to be meters 
 * The units are in meters.
 * Maybe you fuck up when you try to use KLT in the initialization 
 * See MatLab code week 3 for an efficient way to find Harris Corners 
 * When using KLT: Remember to use gray-scale images // Did not work with resizing the image with a factor of 4 
 * Implementation of ransac  in pose estimation in processFrame function 
 * 
 * Optimize and make match descriptors better - see why there is something wrong with it
 * ########################
*/

// ####################### Raspicam Functions #######################
// Namespace and constants 
using namespace cv;
using namespace std;
const char* source_window = "Source image"; 
bool doTestSpeedOnly=false;
using namespace std::chrono;

//#define NUM_SIFT_THREADS 6
//#define NUM_THREADS 29

//parse command line
//returns the index of a command line param in argv. If not found, return -1
int findParam ( string param,int argc,char **argv ) {
    int idx=-1;
    for ( int i=0; i<argc && idx==-1; i++ )
        if ( string ( argv[i] ) ==param ) idx=i;
    return idx;

}
//parse command line
//returns the value of a command line param. If not found, defvalue is returned
float getParamVal ( string param,int argc,char **argv,float defvalue=-1 ) {
    int idx=-1;
    for ( int i=0; i<argc && idx==-1; i++ )
        if ( string ( argv[i] ) ==param ) idx=i;
    if ( idx==-1 ) return defvalue;
    else return atof ( argv[  idx+1] );
}

// Purpose: To set the camera properties - resolution etc.
void processCommandLine ( int argc,char **argv,raspicam::RaspiCam_Cv &Camera ) {
    //Camera.set ( cv::CAP_PROP_FRAME_WIDTH,  getParamVal ( "-w",argc,argv,1280 ) );
    Camera.set ( cv::CAP_PROP_FRAME_WIDTH,  getParamVal ( "-w",argc,argv, 640 ) );
    //Camera.set ( cv::CAP_PROP_FRAME_HEIGHT, getParamVal ( "-h",argc,argv,960 ) );
    Camera.set ( cv::CAP_PROP_FRAME_HEIGHT, getParamVal ( "-h",argc,argv,480 ) );
    Camera.set ( cv::CAP_PROP_BRIGHTNESS,getParamVal ( "-br",argc,argv,50 ) );
    Camera.set ( cv::CAP_PROP_CONTRAST ,getParamVal ( "-co",argc,argv,50 ) );
    Camera.set ( cv::CAP_PROP_SATURATION, getParamVal ( "-sa",argc,argv,50 ) );
    Camera.set ( cv::CAP_PROP_GAIN, getParamVal ( "-g",argc,argv ,50 ) );
    Camera.set ( cv::CAP_PROP_FPS, getParamVal ( "-fps",argc,argv, 0 ) );
    if ( findParam ( "-gr",argc,argv ) !=-1 )
        Camera.set ( cv::CAP_PROP_FORMAT, CV_8UC1 );
    if ( findParam ( "-test_speed",argc,argv ) !=-1 )
        doTestSpeedOnly=true;
    if ( findParam ( "-ss",argc,argv ) !=-1 )
        Camera.set ( cv::CAP_PROP_EXPOSURE, getParamVal ( "-ss",argc,argv )  );
}


// ####################### Main function #######################
int main ( int argc,char **argv ) {
	
	// File with data
	ofstream positionData("VO_output.txt");
	positionData << "framecount" << ";" << "X"<< ";" << "Y" << ";" << "Z" << endl;
	
	// RaspiCam
	raspicam::RaspiCam_Cv Camera;
	processCommandLine ( argc,argv,Camera );
	cout<<"Connecting to camera"<<endl;
	if ( !Camera.open() ) {
		cerr<<"Error opening camera"<<endl;
		return -1;
	}
	cout<<"Connected to camera = " << Camera.getId() <<endl;
	
	// Calibrate camera to get intrinsic parameters K 
	//Mat K = (Mat_<double>(3,3) << 769.893, 0, 2.5, 0,1613.3, 4, 0, 0, 1);
	//Mat K = (Mat_<double>(3,3) << 667.082, 0, 2.5, 0, 1879.18, 4, 0, 0, 1);
	Mat K = (Mat_<double>(3,3) << 710.84, 0, 2.49861, 0, 3051.89, 3.99984, 0, 0, 1); // This is the correct one
	
	cv::Mat I_i0, I_i1, image;
	
	int nCount=getParamVal ( "-nframes",argc,argv, 100 );
	cout<<"Capturing"<<endl;
	
	// Initialization
	Camera.grab(); // You need to take an initial image in order to make the camera work
	Camera.retrieve( image ); 
	//cout << "Image captured" <<endl;
	waitKey(1000);
	
	
	/*
	// Initial frame 0 
	Camera.grab();
	Camera.retrieve( I_i0 ); 
	imshow("Frame I_i0 displayed", I_i0);
	//imwrite("Fliser_640_480_Image0.png",I_i0);
	waitKey(0);


	// First frame 1
	cout << "Prepare to take image 1" << endl; 
	Camera.grab();
	Camera.retrieve ( I_i1 ); // Frame 1 
	imshow("Frame I_i1 displayed", I_i1);
	//imwrite("Fliser_640_480_Image1.png",I_i1);
	waitKey(0);
	*/
	
	
	Mat frame;
	int counter = 0;
	VideoCapture cap("output_stream.avi");
	
	/**
	 * 
	 * HUSK at ændre K til at være for det nye camera
	 * Husk at sætte konstanten c ind fra Rangefinder
	 * Husk at gange x- og y-koordinaterne med konstanten c
	 * Husk at vende om på dem, så x i frame bliver -y og og y i frame bliver -x.
	 * 
	 * 
	 * 
	 **/
	int start_frame, end_frame, base_line;
	
	
	while (true) {
		cout << "Image number = " << counter << endl;
		if (cap.isOpened()) {
			cap >> frame;
		}
		
		if (frame.empty()) {
			cout << "Frame empty" << endl;
			break;
		}
		
		
		if (counter == 1000) {
			start_frame = counter;
			frame.copyTo(I_i0);
		}
		if (counter == 1002) {
			end_frame = counter;
			frame.copyTo(I_i1);
			break;
		}
		
		
		
		

	
		
		//if (counter > 964) {
		//	imshow("Frame", frame);
		//	waitKey(0);
		//}
		
		
		
		//imshow("Frame", frame);
		//waitKey(0);
		
		

		counter++;
	
	
	}
	
	
	/*
	I_i0 = imread("testImg0.png", IMREAD_UNCHANGED);
	imshow("I_i0 frame - new", I_i0);
	waitKey(0);
	
	I_i1 = imread("testImg3.png", IMREAD_UNCHANGED);
	imshow("I_i1 frame - new", I_i1);
	waitKey(0);
	*/
	
	
	// Code used to load the log 
	
	
	//cap.release();
	//destroyAllWindows();

	// ############### VO initializaiton ###############
	// VO-pipeline: Initialization. Bootstraps the initial position.	
	state Si_1;
	Mat transformation_matrix;
	bool initialization_okay;
	Mat Ii_1;
	I_i1.copyTo(Ii_1);
	tie(Si_1, transformation_matrix, initialization_okay) = initialization(I_i0, I_i1, K, Si_1); // One variable extra 
	positionData << counter << ";" << -transformation_matrix.at<double>(1,3) << ";" << -transformation_matrix.at<double>(0,3) << ";" << abs(transformation_matrix.at<double>(2,3)) << endl;
	
	
	// ############### VO Continuous ###############
	bool continueVOoperation = true;
	bool processFrame_okay;
	
	// Needed variables
	state Si;
	Mat Ii;
	double threshold_angle = new_landmarks_threshold_angle; // In degrees
	
	
	// Debug variable
	int stop = 0;
	int iter = 0;
	int failed_attempts = 0;
	
	cout << "Begin Continuous VO operation " << endl;
	//while (continueVOoperation == true && stop < 30) {
	while ( continueVOoperation == true) {
		cout << "Continuous Operation " << endl;

		cout << "Number of keypoints = " << Si_1.Pi.cols << endl;
		cout << "Number of landmarks = " << Si_1.Xi.cols << endl;
	
		/*
		Camera.grab();
		Camera.retrieve ( Ii );
		imshow("Continuous Operation", Ii);
		waitKey(0);
		*/
		base_line = 0;
		for (int i = 0; i < base_line; i++) {
			cap >> frame;
			counter++;
		}
		
		cap >> frame;
		frame.copyTo(Ii);
		//imshow("New Ii", Ii);
		//waitKey(0);
	
		if (frame.empty() ) {
			break;
		}

		
		// Estimate pose 
		tie(Si, transformation_matrix, processFrame_okay) = processFrame(Ii, Ii_1, Si_1, K);
		
		if ( processFrame_okay == false ) {
			failed_attempts++;
		}
		
		// Find new candidate keypoints for the first itme 
		if (iter == 0 && processFrame_okay == true) {
			cout << "Find new candidate keypoints for the first time" << endl;
			positionData << counter << ";" << -transformation_matrix.at<double>(1,3) << ";" << -transformation_matrix.at<double>(0,3) << ";" << abs(transformation_matrix.at<double>(2,3)) << endl;
			
			Si = newCandidateKeypoints(Ii, Si, transformation_matrix);
			iter++;
			failed_attempts = 0;
			
			// Set current values to old values 
			Ii.copyTo(Ii_1);
			Si_1 = Si;
		}
		// Keep finding new candidate keypoints and see if candidate keypoints can become real keypoints
		else if ( (iter > 0) && processFrame_okay == true) {
			cout << "Inside continuous operation " << endl;
			
			positionData << counter << ";" << -transformation_matrix.at<double>(1,3) << ";" << -transformation_matrix.at<double>(0,3) << ";" << abs(transformation_matrix.at<double>(2,3)) << endl;
			
			Si = continuousCandidateKeypoints( Ii_1, Ii, Si, transformation_matrix );
			cout << "ContinuousCandidateKeypoints" << endl;
			cout << "Number of keypoints = " << Si.Ci.cols << endl;
			
			Si = triangulateNewLandmarks( Ii, Si, K, transformation_matrix, threshold_angle);
			// Output it as X, Y, Z in a right handed coordinate system
			
			// Set current values to old values  
			Ii.copyTo(Ii_1);
			
			// Change made here
			Si_1 = Si;			
			
			// Reset failed attempts 
			failed_attempts = 0;
		}
		
		if ( processFrame_okay == true ) {
			cout << "Update of Si.num_candidates = " << Si.num_candidates << endl;
		}
		
		cout << "End of ContinVO. Numb. keypoints = " << Si_1.Pi.cols << endl;
		cout << "End of ContinVO. Numb. landmarks = " << Si_1.Xi.cols << endl;
		
		// Debug variable
		stop++;
		
		// To adjust for the image numbers - should maybe be changed.
		counter++;
		
		if ( failed_attempts > failed_attempts_limit ) {
			cout << "VO-pipeline has failed and is terminated " << endl;
			break;
		}		
	}
	
	positionData << "Parameters Used: " << "Harris_keypoints = " << Harris_keypoints << " Harris_Corner_strengh = " << Harris_Corner_strengh << " num_candidate_keypoints = " << num_candidate_keypoints << " ransac_pixel_tolerance = " << ransac_pixel_tolerance << " ransac_min_inlier_count = " << ransac_min_inlier_count << " new_landmarks_threshold_angle = " << new_landmarks_threshold_angle << " min_nr_inliers_initialization = " << min_nr_inliers_processFrame << endl;
	positionData << "Start frame = " << start_frame << " end frame = " << end_frame << " base_line = " << base_line << endl;
	positionData.close();
	cout << "VO-pipeline terminated" << endl;
	cap.release();
	destroyAllWindows();
	
	double time_=cv::getTickCount();
	Camera.release();

	
	return 0;
}










