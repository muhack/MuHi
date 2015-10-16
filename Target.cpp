class Target{

    public:
        Point target;
        cv::Rect leftEyeRegion;
        cv::Rect rightEyeRegion;
        Point leftPupil;
        Point rightPupil;

    Target(Point t, cv::Rect lER, cv::Rect rER, Point lP, Point rP){
        target = t;
        leftEyeRegion = lER;
        rightEyeRegion = rER;
        leftPupil = lP;
        rightPupil = rP;
    }
    
};

