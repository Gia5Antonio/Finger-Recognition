#ifndef IMMAGINE_H
#define IMMAGINE_H



#include <opencv2/imgproc/imgproc.hpp>
#include<opencv2/opencv.hpp>
#include <vector>

using namespace cv;
using namespace std;

class Immagine{
    public:
        Immagine(int webCamera);
        Immagine();
        Mat srcLR;  //Matrice destinazione
        Mat src;
        Mat bw;  //Somma di tutte le immagini binarie
        vector<Mat> bwList; //Lista immagini binarie
        VideoCapture cap;
        int cameraSrc;
        void InizializzaWebCamera(int i);
};






#endif // IMMAGINE_H

