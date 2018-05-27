#ifndef ROI_H
#define ROI_H

#include <opencv2/imgproc/imgproc.hpp>
#include<opencv2/opencv.hpp>

using namespace cv;

class My_ROI{
    public:
        My_ROI();
        My_ROI(Point AngoloInAlto, Point AngoloInBasso,Mat src);
        Point AngoloInAlto, AngoloInBasso;
        Mat roi_ptr;
        Scalar color;
        int SpessoreBordo;
        void DisegnaRettangolo(Mat src);
};






#endif // ROI_H

