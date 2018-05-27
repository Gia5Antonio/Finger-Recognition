#include "immagine.h"
#include <opencv2/imgproc/imgproc.hpp>
#include<opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>

using namespace cv;

Immagine::Immagine(){
}

Immagine::Immagine(int webCamera){
    cameraSrc=webCamera;
    cap=VideoCapture(webCamera);
}




