#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <vector>


using namespace std;


class Target{

    public:
        cv::Point target;
        cv::Rect face;
        cv::Point leftPupil;
        cv::Point rightPupil;


    Target(cv::Point t, cv::Rect _face, cv::Point lP, cv::Point rP){
        target = t;
        face = _face;
        leftPupil = lP;
        rightPupil = rP;
    }
    
};

