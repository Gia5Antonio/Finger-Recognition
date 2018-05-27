#ifndef MANO_H
#define MANO_H


#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include "immagine.h"

using namespace cv;
using namespace std;

class Mano{
    public:
        Immagine m;
        Mano();
        vector<vector<Point> > Contorni;
        vector<vector<int> >hullI;
        vector<vector<Point> >hullP;//Involucro convesso di un set di punti
        vector<vector<Vec4i> > difetti;
        vector <Point> PuntaDita;
        Rect rect;
        void StampaInfoGesto(Mat src);
        int cIdx;   //Indice contorni della mano più esterni
        int NumeroFrame;
        int NumeroDitaFrequenti;
        int NrDiDifetti;
        Rect bRect;  //rettangolo esterno
        double bRect_width; //larghezza rettangolo esterno.
        double bRect_height; //Altezza rettangolo esterno.
        bool isHand;
        bool RilevaSeMano();
        void InializzaVettori();
        void getNumeroDita(Immagine *m);
        void EliminaDifetti(Immagine *m);
        void getPuntaDita(Immagine *m);
        void DisegnaPuntaDita(Immagine *m);
    private:
        string bool2string(bool tf);
        int fontFace;
        int prevNrFingerTips;
        void VerificaPerUnDito(Immagine *m);
        float getAngolo(Point s,Point f,Point e);
        vector<int> NumeroDita;
        void AnalizzaContorni();
        string intToString(int number);
        void CalcolaNumDita();
        void DisegnaNuovoNumero(Immagine *m);
        void AggiungiNumerotoImg(Immagine *m);
        vector<int> numbers2Display;
        void AggiungiNumeroDitaAlVettore();
        Scalar numeroColore;
        int NrDita;
        float distanzaP2P(Point a,Point b);
        void RimuoviPuntiFinaliRidondanti(vector<Vec4i> newDefects,Immagine *m);
        void RimuoviPuntaDelleDitaRidondanti();
};



//Costanti
#define ORIGCOL2COL CV_BGR2HLS
#define COL2ORIGCOL CV_HLS2BGR
#define NUMCAMPIONI 7
#define PI 3.14159


#endif // MANO_H



