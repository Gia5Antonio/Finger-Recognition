#include "immagine.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include "roi.h"

using namespace cv;
using namespace std;

My_ROI::My_ROI(){
    AngoloInAlto=Point(0,0);
    AngoloInBasso=Point(0,0);

}

My_ROI::My_ROI(Point AngoloAlto, Point AngoloBasso, Mat src){
    AngoloInAlto=AngoloAlto;  //punto x,y angolo in alto
    AngoloInBasso=AngoloBasso;
    color=Scalar(0,255,0);
    SpessoreBordo=2;
    roi_ptr=src(Rect(AngoloAlto.x, AngoloAlto.y, AngoloBasso.x-AngoloAlto.x,AngoloBasso.y-AngoloAlto.y)); //si memorizza rettangolo in una matrice (x,y,larghezza,altezza)
}

void My_ROI::DisegnaRettangolo(Mat src){
    rectangle(src,AngoloInAlto,AngoloInBasso,color,SpessoreBordo);

}
