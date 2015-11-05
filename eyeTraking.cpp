// #include "opencv2/objdetect.hpp"
// #include "opencv2/videoio.hpp"
// #include "opencv2/highgui.hpp"
// #include "opencv2/imgproc.hpp"

#include <opencv2/opencv.hpp>

// #include "objdetect.hpp"
// #include <opencv2/videoio.hpp>
// #include <opencv2/highgui.hpp>
// #include <opencv2/imgproc.hpp>

#include <iostream>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <string>
#include "constants.h"
#include "findEyeCenter.h"
//#include "PupilStack.cpp"
#include "Target.cpp"
//#include "findEyeCorner.h"
//#include "Settings.h"

using namespace std;
using namespace cv;

/** Function Headers */
Rect detectFace( Mat *frame_in );
Rect detectEye( Mat *frame_in );
Rect detectEyeball( Mat *frame_in );
cv::Point findPupil(cv::Mat frame, cv::Rect face);
void findEyes(cv::Mat frame_gray, cv::Rect face);
int init();
void display();
void print_term(String msg);
void checkStability(Rect *result);
void detectAndDisplay( cv::Mat frame );
Point findGazeFocus();
void getTargets();
void deltaShift();
int selectCamera();
void resetTare();
void tare();
void print_logo();

void gazeDebug();

//------------------Global variables----------------------//
String face_cascade_name = "/usr/local/share/OpenCV/lbpcascades/lbpcascade_frontalface.xml";
String eyes_cascade_name = "/usr/local/share/OpenCV/haarcascades/haarcascade_eye_tree_eyeglasses.xml";
String window_name_face = "Capture - Face detection";
String window_name_eye = "Capture - Eye detection";
String window_name_eyeball = "Capture - Eyeball detection";
CascadeClassifier face_cascade;
CascadeClassifier eyes_cascade;
char *window_title = "Gaze Traking";
int max_tested_camera = 10;

cv::Rect leftEyeRegion;
cv::Rect rightEyeRegion;
Point leftPupil;
Point rightPupil;

Point candi_leftPupil;
Point candi_rightPupil;

//For targetting..
bool waiting_for_lock_target = false;
Point current_target;

//Pupil coding - the actual position of the pupil
bool up_pos = false;
bool down_pos = false;
bool left_pos = false;
bool rigth_pos = false;

float treshold_trigger = 1.0;

float up_trigger = 0;
float down_trigger = 0;
float left_trigger = 0;
float rigth_trigger = 0;

bool tared = false;
bool target_checked = false;

VideoCapture capture;

//Target stack
int targets_num = 9; 
Target **targets = new Target*[targets_num];

// int X_RESOLUTION = 1366;
// int Y_RESOLUTION =  768;
int X_RESOLUTION = 640;
int Y_RESOLUTION = 480;

//-- Note, either copy these two files from opencv/data/haarscascades to your current folder, or change these locations
//cv::String face_cascade_name = "../../../res/haarcascade_frontalface_alt.xml";
//cv::String face_cascade_name = "/usr/local/share/OpenCV/lbpcascades/lbpcascade_frontalface.xml";
//cv::CascadeClassifier face_cascade;
std::string main_window_name = "Capture - Face detection";
std::string face_window_name = "Capture - Face";
cv::RNG rng(12345);
cv::Mat debugImage;
cv::Mat skinCrCbHist = cv::Mat::zeros(cv::Size(256, 256), CV_8UC1);

char lr_eye = 0; //0 for the left, 1 for the rigth

//The original frame, directly from the cam
Mat frame;
//The frame's area where we found a face, and search for eye
Mat faceROI;
//The frame's area where we found am eye, ad search for eyeball
Mat eyeROI; //Maybe not usefull
//The end process result
Mat result;
//The user interface - we write here all the user's stuff...
Mat HUD; //Not used yet

//The candidate
Rect face;
Rect eye;
Rect eyeball;
Point eyeCenter;

//This value is incremented when the fresh found result ( face, eye or eyeball ) 
// is to far away (treshold_stability) from the candidate
//When it == 100, the program steps back to the previos stage
int lost_probaility = 0;
int found_probability = 0;
float treshold_stability = 10.0;
float eye_treshold_stability = 1.0;

float delta_shift_trigger = 20.0;

int pupil_instability;

//Used to check for errors and stop the main cicle..
int errors = 0;

//Set to 1 to get ALL debug informations
int debug_mode = 0; 

//To flip the image
bool flip_input = false;

//----------------------------------------------------------//
//std::vector<Target> targets;

//Settings settings;

//----------------------------------------------------------//
/**
 * @function main
 */
int main( void ){

    print_logo();


    Rect temp; //Temporary rect for the fresh new result from the detection
    cout<<"Loading cascade and opening webcam..."<<endl;
    errors = init();
    cout<<"All ok. Startin capturing..."<<endl;

    while ( capture.read(frame) && !errors){

        if( frame.empty() ){
            print_term("No captured frame -- Break!");
            errors = 1;
        }

        //Well.. i'm fucked up... I NEED REFACTORING :'C
        result = frame;

        //Flip the image: on my laptop, the webcam (for some f**king reason) is flipped upside down
        //Comment this line if you get a flipped output...
        if(flip_input)
            flip( frame, frame, 0);

        //Gray the frame
        cvtColor( frame, frame, COLOR_BGR2GRAY );
        //Equalize the frame
        //equalizeHist( frame, frame);
        //Blurrrrrrrrrr!!!
        //GaussianBlur( frame, frame, Size(9, 9), 2, 2 );

        //Apply the classifier to the frame
        temp = detectFace( &frame );
        checkStability( &temp );

        if(face.area() != 0){
            

            findEyes(frame,face);

            //Pupil stability
            if( sqrt( pow(leftPupil.x - candi_leftPupil.x ,2) + pow(rightPupil.x - candi_rightPupil.x ,2) ) >= eye_treshold_stability ){
                candi_leftPupil.x = (candi_leftPupil.x + leftPupil.x ) / 2;
                candi_leftPupil.y = (candi_leftPupil.y + leftPupil.y ) / 2;

                candi_rightPupil.x = (candi_rightPupil.x + rightPupil.x) / 2;
                candi_rightPupil.y = (candi_rightPupil.y + rightPupil.y) / 2;

            }

            //We first must understand the eye's mobility
            if(!tared){
                tare();
            } else {
                    deltaShift();
            }

            if(debug_mode){
                int skip = 0;
                if(skip % 10 == 0){
                    skip = 0;
                    gazeDebug();
                }
            }

        }

        if(debug_mode){
            rectangle(frame, Point(face.x, face.y), Point(face.x + face.height, face.y + face.width), Scalar(255,0,0));
            rectangle(frame, Point(temp.x, temp.y), Point(temp.x + temp.height, temp.y + temp.width), Scalar(255,0,0));
            circle(frame, candi_rightPupil + Point(face.x, face.y), 5, 255);
            circle(frame, candi_rightPupil + Point(face.x, face.y), 5, 255);
        }

        if(waiting_for_lock_target)
                rectangle(result, Point(0,0), Point(result.cols, result.rows), Scalar(255,0,0));
            

        //if(debug_mode)
            //display();

        //-- bail out if escape was pressed
        int c = waitKey(10);
        if( (char)c == 27 ) { break; }
        if( (char)c == 'd' ) { debug_mode = !debug_mode; }
        if( (char)c == '-' ) { treshold_trigger--; }
        if( (char)c == '+' ) { treshold_trigger++; }
        if( (char)c == 'f' ) { flip_input = !flip_input; }
        if( (char)c == 't' ) { target_checked = true; }
        if( (char)c == 'l' ) { resetTare() ;}
        
        if(face.area() != 0)
            circle(result, findGazeFocus(), 5, 5);

        //imshow(window_title, result);
        imshow(window_title, frame);
        
    }

    if(errors){
        print_term("Program ended with errors!!!!");
    } else {
        print_term("Program ended without errors... GOODBYE!");
    }

    destroyWindow(window_title);
    return 0;
}


//I REALLY DON'T LIKE HOW I DID THIS METHOD
//The stack index is based on the tare_phase
//There are 9 target: rigth/center/left for upper/center/lower zone

int tare_phase = 0;

void tare(){

    Point target;
    
    switch (tare_phase){
        case 0: // Left upper corner
            target = Point(0,0);
        break;

        case 1: // Center upper
            target = Point(X_RESOLUTION/2 , 0);
        break;

        case 2: // Rigth upper corner
            target = Point(X_RESOLUTION, 0);
        break;

        case 3: // Left center side
            target = Point(0, Y_RESOLUTION/2);
        break;

        case 4: // Center of the screen
            target = Point(X_RESOLUTION/2 , Y_RESOLUTION/2);
        break;

        case 5: // Rigth center side
            target = Point(X_RESOLUTION , Y_RESOLUTION/2);
        break;

        case 6: // Left lower corner
            target = Point(0, Y_RESOLUTION);
        break;

        case 7: // Center lower
            target = Point(X_RESOLUTION/2 , Y_RESOLUTION);
        break;

        case 8: // Rigth lower corner
            target = Point(X_RESOLUTION, Y_RESOLUTION);
        break;
    }

    circle(frame, target, 10, 255, -1); 

    if(target_checked){
        target_checked = false;
        targets[tare_phase] = new Target(target, face, leftPupil, rightPupil);
        tare_phase ++;

        if (tare_phase == 9){
            //We have all the targets' data! Let's define the moving triggers! 
            //For now, just focus ONLY on the leftPupil
            up_trigger = (targets[0]->leftPupil.x + targets[1]->leftPupil.x + targets[2]->leftPupil.x) / 3;
            down_trigger = (targets[6]->leftPupil.x + targets[7]->leftPupil.x + targets[8]->leftPupil.x) / 3;
            left_trigger = (targets[0]->leftPupil.y + targets[3]->leftPupil.y + targets[6]->leftPupil.y) / 3;
            rigth_trigger = (targets[2]->leftPupil.y + targets[5]->leftPupil.y + targets[8]->leftPupil.y) / 3;

            //With the center target (index = 4) we should do some adjustmets.. but I have no idea about it...

            for( int i = 0; i < 9; i++){
                cout<<"-----Target number "<<i<<"------"<<endl<<targets[i]->toString()<<"--------------------------"<<endl;
            }

            printf("Triggers: %f - %f - %f - %f \n", up_trigger, down_trigger, left_trigger, rigth_trigger);

            tared = true;
        }
    }

}

void resetTare(){
    tared = false;
    tare_phase = 0;
}

void checkStability(Rect *result){
    Rect *candidate;

    //In this way, we have a feedback from the detection process, allowing us to detect when the algorith didn't find nothings...
    if ( *result == Rect(0,0,0,0) ){
        lost_probaility ++;
        return;
    }

    if( face.area() == 0) //------ATTENTION! I think this is not the rigth way to check whatever the rect exist or not...
        face = *result; //For the first time, we need to start from something! 
   
    candidate = &face;

    //Sincerely, i don't know how to do this....... :>
    //Upper left corner
    float diff = sqrt(pow((candidate->x - result->x),2) + pow((candidate->y - result->y),2));
    if( diff > treshold_stability){ //A little difference, we update the candidate..
        candidate->x = (candidate->x + result->x) / 2;
        candidate->y = (candidate->y + result->y) / 2;
        if (diff > treshold_stability * 2){ //Too much difference, we need to update candidate and take note of it...!
            lost_probaility++;
        }
    } else {
        found_probability ++;
        lost_probaility--;
    }

    //Bottom rigth corner
    diff = sqrt(pow(((candidate->x+candidate->width) - (result->x+result->width)),2) + pow(((candidate->y + candidate->height) - (result->y + result->height)),2));
    if( diff > treshold_stability){ //A little difference, we update the candidate..
        candidate->width = (candidate->width + result->width) / 2;
        candidate->height = (candidate->height + result->height) / 2;
        if (diff > treshold_stability * 2){ //Too much difference, we need to update candidate and take note of it...!
            lost_probaility++;
        }
    } else {
        found_probability ++;
        lost_probaility--;
    }
}

void gazeDebug(){
    // cout<<"Actual status: "<<endl;
    if(up_pos){
        circle(frame, Point(X_RESOLUTION/2 , 0), 10, 255, -1);
        // cout<<"UP!"<<endl;
        // system("espeak Up");
    }

    if(down_pos){
        circle(frame, Point(X_RESOLUTION/2 , Y_RESOLUTION), 10, 255, -1);
        // cout<<"DOWN!"<<endl;
        // system("espeak Down");
    }

    if(rigth_pos){
        circle(frame,Point(X_RESOLUTION , Y_RESOLUTION/2), 10, 255, -1);
        // cout<<"Right!"<<endl;  
        // system("espeak Right"); 
    }

    if(left_pos){
        circle(frame, Point(0 , Y_RESOLUTION/2), 10, 255, -1);
        // cout<<"Left!"<<endl;  
        // system("espeak Left"); 
    }

    // cout<<"---------------"<<endl<<endl;
}

Point prev_left_pos;
Point left_center;


void deltaShift(){

    // left_center = Point(0,0);

    // //Prima prova di riconoscimento posizione basandosi sulla posizione attuale della pupilla
    // //in relazione al centro del rettangolo regone occhio -- Non credo sia una buona soluzione :\
    // left_center = Point(leftEyeRegion.width/2 , leftEyeRegion.width/2);

    // left_center.x += rightEyeRegion.x;
    // left_center.y += rightEyeRegion.y;

    //circle(frame, left_center, 10, 255);    

    //ATTENTION - the X COORDINATE ARE UPSIDE DOWN
    //This means that the x grows going from the top to the bottom of the streen!
    if( leftPupil.x < up_trigger - treshold_trigger ){
        down_pos = false;
        up_pos = true; //Fault tollerance
    } else if( leftPupil.x > down_trigger + treshold_trigger ){
        down_pos = true;
        up_pos = false; 
    } else { //Center position
        down_pos = false;
        up_pos = false;
    }


    if( leftPupil.y > rigth_trigger + treshold_trigger ){
        rigth_pos = true;
        left_pos = false;
    } else if( leftPupil.y  < left_trigger - treshold_trigger ){
        rigth_pos = false;
        left_pos = true;
    } else {
        rigth_pos = false;
        left_pos = false;
    }


    // if(prev_left_pos != NULL ){
    //     float diff = sqrt( pow( (prev_left_pos.x - leftPupil.x) ,2) + pow( (prev_left_pos.y - leftPupil.y) ,2)  );
    //     //Now we know that the pupil moved more than the trigger distance..! So we must chech in wich direction it moved..
    //     if(diff > move_trigger){
    //         //From the center it moves up
    //         if()
    //         //F
    //     }
    // } 

    // //We update the last position with the actual 
    // prev_left_pos = leftPupil;

}

void getTargets(){

    //There are no targets.. let's create one! :D
    if(!waiting_for_lock_target){
        current_target = Point(0,0);
        circle(result, current_target, 10, 10);
        waiting_for_lock_target = true;
    } else {
        //Target new_target = new Target(current_target, leftEyeRegion, rightEyeRegion, leftPupil, rightPupil);
        //targets.insert( new Target(current_target, leftEyeRegion, rightEyeRegion, leftPupil, rightPupil) );
        waiting_for_lock_target = false;
        printf("TARGET ACQUIRED!!!\n");
    }

}



/**
 * @function detectFace
 */
Rect detectFace( Mat *frame_in ){
    std::vector<Rect> faces;
    cv::Rect out(0,0,0,0);

    //-- Detect faces
    face_cascade.detectMultiScale( *frame_in, faces, 1.1, 2, 0, Size(80, 80) );

    //We consider only ONE face, the first found... maybe not the best solution...
    if(faces.size() != 0){
        for(int i=0; i< faces.size(); i++){
            if(faces[i].area() >= out.area()){
                out = faces[i];
            }
        }

        return out; 
    }
    else
        return out;
    // //faceROI = frame( faces[0] ); 
}

Rect detectEye( Mat frame_in, int lr_eye){
    std::vector<Rect> eyes;

    //Detect faces
    eyes_cascade.detectMultiScale( frame_in, eyes, 1.1, 2, 0 |CASCADE_SCALE_IMAGE, Size(30, 30) );

    //Detect the left or rigth eye
    if(eyes.size() != 0 && eyes.size() == 2){
        if(eyes[0].x > face.width/2 && lr_eye == 1){
            return eyes[0]; 
        } else if (eyes[1].x > face.width/2 && lr_eye == 1){
            return eyes[1];
        } else if (eyes[0].x < face.width/2 && lr_eye == 0){
            return eyes[0];
        } else if (eyes[1].x < face.width/2 && lr_eye == 0){
            return eyes[1];
        } 
    }else
        return Rect(0,0,0,0);
}


int init(){
    //Load the cascade
    if( !face_cascade.load( face_cascade_name ) ){ printf("--(!)Error loading face cascade\n"); return -1; };
    if( !eyes_cascade.load( eyes_cascade_name ) ){ printf("--(!)Error loading eyes cascade\n"); return -1; };

    //Open video
    //capture.open( 0 ); //-1 for the default video source
    //capture.open("test_video/test_video.mp4");
    //int web = selectCamera();

    //cout<<"Selected index "<<web<<endl;

    capture.open( -1 );

    if ( ! capture.isOpened() ) { printf("--(!)Error opening video capture\n"); return -1; }

    //Try to set an higer resolution
    capture.set(CV_CAP_PROP_FRAME_WIDTH, 1000);
    capture.set(CV_CAP_PROP_FRAME_HEIGHT, 1000);

    face = Rect(0,0,face.height, face.width);

    //Create the window
    cvNamedWindow(window_title, CV_WINDOW_NORMAL);
    cvSetWindowProperty(window_title, CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);

    //We need to resize the mat as the main window!!!!
    //result = cvCreateImage(cvGetSize(frame),8,frame.nChannels);
    //result = Mat(frame);



    //If all went ok..
    return 0;

}

int selectCamera(){
    cv::VideoCapture temp_camera;

    for(int i = 0; i < max_tested_camera; i++){
        temp_camera.open(i);
        if(temp_camera.isOpened()){
            cout<<"Found camera with index "<<i<<endl;
        }
        temp_camera.release();
    }

    cout<<endl<<"Camera number? ";

    int web;
    cin >> web;
    return web;

}

void display(){
    //-- Show what you got
    imshow( window_name_face, frame );
    if(!faceROI.empty()){
        imshow( window_name_eye, faceROI);
    }

    if(!eyeROI.empty()){
        imshow( window_name_eyeball, eyeROI);
        
    }
}

void print_term(String msg){
    std::cout<<msg<<endl;
}

// cv::Point findPupil(cv::Mat frame, cv::Rect face){ //Cambiare il nome variabile "face" a "eye"
//     cv::Mat faceROI = frame(face);

//     if (kSmoothFaceImage) {
//         double sigma = kSmoothFaceFactor * face.width;
//         GaussianBlur( faceROI, faceROI, cv::Size( 0, 0 ), sigma);
//     }
//     //-- Find eye regions and draw them
//     int eye_region_width = face.width * (kEyePercentWidth/100.0);
//     int eye_region_height = face.width * (kEyePercentHeight/100.0);
//     int eye_region_top = face.height * (kEyePercentTop/100.0);
//     cv::Rect leftEyeRegion(face.width*(kEyePercentSide/100.0),
//                          eye_region_top,eye_region_width,eye_region_height);
//     cv::Rect rightEyeRegion(face.width - eye_region_width - face.width*(kEyePercentSide/100.0),
//                           eye_region_top,eye_region_width,eye_region_height);

//     //-- Find Eye Centers
//     if(lr_eye){
//         return findEyeCenter(faceROI,rightEyeRegion, window_name_eyeball); 
//     } else {
//         return findEyeCenter(faceROI,leftEyeRegion, window_name_eyeball);
//     }

// }

void findEyes(cv::Mat frame_gray, cv::Rect face) {
  cv::Mat faceROI = frame_gray(face);
  cv::Mat debugFace = faceROI;


  //Qui ci sta il problema che la regione dell'occhio non è gestita in maniera sensata
  //Da una parte possiamo cercare di ottenerla proporzionalmente (veloce), ma dobbiamo aggiungere sistemi di controllo
  //Dall'altra possiamo usare i cascade appositi (lento) ma siamo più sicuri...


  //Find eye regions -- We must work on better region definition
  int eye_region_width = face.width * (kEyePercentWidth/100.0);
  int eye_region_height = face.width * (kEyePercentHeight/100.0);
  int eye_region_top = face.height * (kEyePercentTop/100.0);

  leftEyeRegion = Rect(face.width*(kEyePercentSide/100.0), eye_region_top,eye_region_width,eye_region_height);
  rightEyeRegion = Rect(face.width - eye_region_width - face.width*(kEyePercentSide/100.0), eye_region_top,eye_region_width,eye_region_height);

  //leftEyeRegion = detectEye( faceROI ,0);


  //Find Eye Centers
  // cv::Point leftPupil = findEyeCenter(faceROI,leftEyeRegion,"Left Eye");
  // cv::Point rightPupil = findEyeCenter(faceROI,rightEyeRegion,"Right Eye");

  equalizeHist( faceROI, faceROI);


  //if(leftEyeRegion.area() > 0)
  leftPupil = findEyeCenter(faceROI,leftEyeRegion);

  rightPupil = findEyeCenter(faceROI,rightEyeRegion);

  imshow("debug left", faceROI(leftEyeRegion));
  
  // change eye centers to face coordinates
  rightPupil.x += rightEyeRegion.x;
  rightPupil.y += rightEyeRegion.y;
  leftPupil.x += leftEyeRegion.x;
  leftPupil.y += leftEyeRegion.y;
 
  // draw eye centers
  // circle(debugFace, rightPupil, 3, 1234);
  // circle(debugFace, leftPupil, 3, 1234);

  //Vertical line
  line(debugFace, Point(rightPupil.x, rightPupil.y-5), Point(rightPupil.x, rightPupil.y+5), 1234 );
  line(debugFace, Point(leftPupil.x, leftPupil.y-5), Point(leftPupil.x, leftPupil.y+5), 1234 );

  //Horizzontal line
  line(debugFace, Point(rightPupil.x-5, rightPupil.y), Point(rightPupil.x+5, rightPupil.y), 1234 );
  line(debugFace, Point(leftPupil.x-5, leftPupil.y), Point(leftPupil.x+5, leftPupil.y), 1234 );


}

//Here goes the magic....!
//Fuck.. it's really difficult.... 
Point findGazeFocus(){
    // cv::Rect leftEyeRegion;
    // cv::Rect rightEyeRegion;
    // Point leftPupil;
    // Point rightPupil;

    // printf("Posizioni: leftEyeRegion: %i - %i // leftPupil: %i - %i \n", leftEyeRegion.x, leftEyeRegion.y, leftPupil.x, leftPupil.y);

    // circle(frame, Point(leftEyeRegion.x + face.x, leftEyeRegion.y + face.y), 10,10);
    // circle(frame, leftPupil + Point(face.x, face.y), 20,20);

    // rectangle(frame, leftEyeRegion + Point(face.x, face.y), 100 );

    // for(int i = 0; i<targets.size(); i++){


    // }

    
    Point focusLeft;
    Point focusRigth;
    Point focus;


    float leftOrizzProportion;
    float leftVertiProportion;
    float rigthOrizzProportion;
    float rigthVertiProportion;

    // //Transport the coordinate to face's upper left angle
    // Point absLeftPupil = leftPupil + Point(face.x, face.y);
    // Point absRigthPupil = rightPupil + Point(face.x, face.y);

    // Rect absLeftEyeRegion = leftEyeRegion + Point(face.x, face.y);
    // Rect absRigthEyeRegion = rightEyeRegion + Point(face.x, face.y);

    // x : 100 = eye.x : regio.x
    leftOrizzProportion = (candi_leftPupil.x * 100) / (leftEyeRegion.x + leftEyeRegion.width);
    rigthOrizzProportion =  (candi_rightPupil.x * 100) / (rightEyeRegion.x + rightEyeRegion.width);

    leftVertiProportion = (candi_leftPupil.y * 100) / (leftEyeRegion.y + leftEyeRegion.height);
    rigthVertiProportion = (candi_rightPupil.y * 100) / (rightEyeRegion.y + rightEyeRegion.height);

    //Find the position with the proportion
    //proportion : 100 = x : windowsSize
    focusLeft.x = (leftOrizzProportion * result.cols) / 100;
    focusLeft.y = (leftVertiProportion * result.rows) / 100;

    focusRigth.x = (rigthOrizzProportion * result.cols) / 100;
    focusRigth.y = (rigthVertiProportion * result.rows) / 100;

    //xe : wif = x : 1336
    focusLeft.x = ( leftPupil.x * 1336) / face.width;
    focusRigth.x = ( rightPupil.x * 1336) / face.width;


    //ye : hef = x : 768
    focusLeft.y = ( leftPupil.y * 768) / face.width;
    focusRigth.y = ( rightPupil.y * 768) / face.width;



    focus.x =- (focusLeft.x - focusRigth.x) / 2;
    focus.y =- (focusLeft.y - focusRigth.y) / 2;

    //return focus;

    return focus;

}

void print_logo(){

    unsigned char logo_txt[] = {
      0x0a, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x5f, 0x5f, 0x5f, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x5f, 0x5f,
      0x5f, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x5f, 0x5f, 0x5f, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x0a, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x2f, 0x5c, 0x5f, 0x5f, 0x5c, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x2f, 0x5c, 0x5f, 0x5f, 0x5c, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x2f, 0x5c, 0x5f, 0x5f, 0x5c, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x5f, 0x5f, 0x5f,
      0x20, 0x20, 0x20, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x2f, 0x3a, 0x3a, 0x7c,
      0x20, 0x20, 0x7c, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x2f, 0x3a,
      0x2f, 0x20, 0x20, 0x2f, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x2f, 0x3a, 0x2f, 0x20, 0x20, 0x2f, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x2f, 0x5c, 0x20, 0x20, 0x5c, 0x20, 0x20, 0x0a, 0x20,
      0x20, 0x20, 0x2f, 0x3a, 0x7c, 0x3a, 0x7c, 0x20, 0x20, 0x7c, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x2f, 0x3a, 0x2f, 0x20, 0x20, 0x2f, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x2f, 0x3a, 0x2f, 0x5f, 0x5f, 0x2f,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x5c, 0x3a,
      0x5c, 0x20, 0x20, 0x5c, 0x20, 0x0a, 0x20, 0x20, 0x2f, 0x3a, 0x2f, 0x7c,
      0x3a, 0x7c, 0x5f, 0x5f, 0x7c, 0x5f, 0x5f, 0x20, 0x20, 0x20, 0x2f, 0x3a,
      0x2f, 0x20, 0x20, 0x2f, 0x20, 0x20, 0x5f, 0x5f, 0x5f, 0x20, 0x20, 0x20,
      0x2f, 0x3a, 0x3a, 0x5c, 0x20, 0x20, 0x5c, 0x20, 0x5f, 0x5f, 0x5f, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x2f, 0x3a, 0x3a, 0x5c, 0x5f, 0x5f, 0x5c,
      0x0a, 0x20, 0x2f, 0x3a, 0x2f, 0x20, 0x7c, 0x3a, 0x3a, 0x3a, 0x3a, 0x5c,
      0x5f, 0x5f, 0x5c, 0x20, 0x2f, 0x3a, 0x2f, 0x5f, 0x5f, 0x2f, 0x20, 0x20,
      0x2f, 0x5c, 0x5f, 0x5f, 0x5c, 0x20, 0x2f, 0x3a, 0x2f, 0x5c, 0x3a, 0x5c,
      0x20, 0x20, 0x2f, 0x5c, 0x5f, 0x5f, 0x5c, 0x20, 0x20, 0x5f, 0x5f, 0x2f,
      0x3a, 0x2f, 0x5c, 0x2f, 0x5f, 0x5f, 0x2f, 0x0a, 0x20, 0x5c, 0x2f, 0x5f,
      0x5f, 0x2f, 0x7e, 0x7e, 0x2f, 0x3a, 0x2f, 0x20, 0x20, 0x2f, 0x20, 0x5c,
      0x3a, 0x5c, 0x20, 0x20, 0x5c, 0x20, 0x2f, 0x3a, 0x2f, 0x20, 0x20, 0x2f,
      0x20, 0x5c, 0x2f, 0x5f, 0x5f, 0x5c, 0x3a, 0x5c, 0x2f, 0x3a, 0x2f, 0x20,
      0x20, 0x2f, 0x20, 0x2f, 0x5c, 0x2f, 0x3a, 0x2f, 0x20, 0x20, 0x2f, 0x20,
      0x20, 0x20, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x2f, 0x3a,
      0x2f, 0x20, 0x20, 0x2f, 0x20, 0x20, 0x20, 0x5c, 0x3a, 0x5c, 0x20, 0x20,
      0x2f, 0x3a, 0x2f, 0x20, 0x20, 0x2f, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x5c, 0x3a, 0x3a, 0x2f, 0x20, 0x20, 0x2f, 0x20, 0x20, 0x5c, 0x3a,
      0x3a, 0x2f, 0x5f, 0x5f, 0x2f, 0x20, 0x20, 0x20, 0x20, 0x0a, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x2f, 0x3a, 0x2f, 0x20, 0x20, 0x2f, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x5c, 0x3a, 0x5c, 0x2f, 0x3a, 0x2f, 0x20, 0x20, 0x2f,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x2f, 0x3a, 0x2f, 0x20,
      0x20, 0x2f, 0x20, 0x20, 0x20, 0x20, 0x5c, 0x3a, 0x5c, 0x5f, 0x5f, 0x5c,
      0x20, 0x20, 0x20, 0x20, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x2f, 0x3a,
      0x2f, 0x20, 0x20, 0x2f, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x5c,
      0x3a, 0x3a, 0x2f, 0x20, 0x20, 0x2f, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x2f, 0x3a, 0x2f, 0x20, 0x20, 0x2f, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x5c, 0x2f, 0x5f, 0x5f, 0x2f, 0x20, 0x20, 0x20, 0x20, 0x0a,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x5c, 0x2f, 0x5f, 0x5f, 0x2f, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x5c, 0x2f, 0x5f, 0x5f, 0x2f,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x5c, 0x2f, 0x5f,
      0x5f, 0x2f, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x0a, 0x0a, 0x0a
    };

    printf("%s\n",logo_txt );
}
