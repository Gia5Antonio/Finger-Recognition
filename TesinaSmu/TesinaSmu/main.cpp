#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include "immagine.h"
#include "roi.h"
#include "mano.h"
#include <vector>
#include <cmath>


using namespace cv;
using namespace std;

//costanti utili per la conversione da uno spazio dei colori ad un altro
#define ORIGCOL2COL CV_BGR2HLS
#define COL2ORIGCOL CV_HLS2BGR
#define NUMCAMPIONI 7
#define PI 3.14159



/* Variabili globali */
int fontFace = FONT_HERSHEY_PLAIN;
int lunghezzaQuadrato;
int avgColor[NUMCAMPIONI][3] ;
int c_lower[NUMCAMPIONI][3];
int c_upper[NUMCAMPIONI][3];
int avgBGR[3];
int NrDiDifetti;
int iSinceKFInit;

struct dim{
    int w;
    int h;
}DelimitazioneDim;

VideoWriter out;
Mat edges;
My_ROI roi1, roi2,roi3,roi4,roi5,roi6;
vector <My_ROI> roi;
vector <KalmanFilter> kf;
vector <Mat_<float> > misurazione;

/* Fine vg*/

void inizializza(Immagine *m){
    lunghezzaQuadrato=20;
    iSinceKFInit=0;
}

// Cambia un colore da uno spazio dei colori ad un altro
void col2origCol(int hsv[3], int bgr[3], Mat src){
    Mat avgBGRMat=src.clone();
    for(int i=0;i<3;i++){
        avgBGRMat.data[i]=hsv[i];
    }
    cvtColor(avgBGRMat,avgBGRMat,COL2ORIGCOL);
    for(int i=0;i<3;i++){
        bgr[i]=avgBGRMat.data[i];
    }
}

void printText(Mat src, string text){
    int fontFace = FONT_HERSHEY_PLAIN;
    putText(src,text,Point(src.cols/2, src.rows/10),fontFace, 1.2f,Scalar(200,0,0),2);
}

void AttendiCoperturaPalmo(Immagine* m){
    m->cap >> m->src; //Ottiene nuova frame dal video
    flip(m->src,m->src,1); //Inverte la matrice rispetto all'asse y
    roi.push_back(My_ROI(Point(m->src.cols/3, m->src.rows/6),Point(m->src.cols/3+lunghezzaQuadrato,m->src.rows/6+lunghezzaQuadrato),m->src)); //ottiene roiptr che permetterà di disegnare il rettangolo
    roi.push_back(My_ROI(Point(m->src.cols/4, m->src.rows/2),Point(m->src.cols/4+lunghezzaQuadrato,m->src.rows/2+lunghezzaQuadrato),m->src));
    roi.push_back(My_ROI(Point(m->src.cols/3, m->src.rows/1.5),Point(m->src.cols/3+lunghezzaQuadrato,m->src.rows/1.5+lunghezzaQuadrato),m->src));
    roi.push_back(My_ROI(Point(m->src.cols/2, m->src.rows/2),Point(m->src.cols/2+lunghezzaQuadrato,m->src.rows/2+lunghezzaQuadrato),m->src));
    roi.push_back(My_ROI(Point(m->src.cols/2.5, m->src.rows/2.5),Point(m->src.cols/2.5+lunghezzaQuadrato,m->src.rows/2.5+lunghezzaQuadrato),m->src));
    roi.push_back(My_ROI(Point(m->src.cols/2, m->src.rows/1.5),Point(m->src.cols/2+lunghezzaQuadrato,m->src.rows/1.5+lunghezzaQuadrato),m->src));
    roi.push_back(My_ROI(Point(m->src.cols/2.5, m->src.rows/1.8),Point(m->src.cols/2.5+lunghezzaQuadrato,m->src.rows/1.8+lunghezzaQuadrato),m->src));


    for(int i =0;i<50;i++){
        m->cap >> m->src;
        flip(m->src,m->src,1); //Inverte la matrice rispetto all'asse y
        for(int j=0;j<NUMCAMPIONI;j++){
            roi[j].DisegnaRettangolo(m->src);
        }
        string imgText=string("Copri i quadrati con il palmo");
        printText(m->src,imgText);

        if(i==30){
          //  imwrite("C:\\immagine.jpg",m->src);
        }

        imshow("Immagine", m->src);
        out << m->src;
        if(cv::waitKey(30) >= 0) break;  //
    }
}

int getMediana(vector<int> val){
  int mediana;
  size_t size = val.size();
  sort(val.begin(), val.end());
  if (size  % 2 == 0)  {
      mediana = val[size / 2 - 1] ;
  } else{
      mediana = val[size / 2];
  }
  return mediana;
}


void getAvgColor(Immagine *m,My_ROI roi,int avg[3]){  //
    Mat r;
    roi.roi_ptr.copyTo(r); //matrice che contiene rettangolino
    vector<int>hm; //tonalità (tinta)
    vector<int>sm; //saturazione (sfumatura)
    vector<int>lm; //luminosità  (Tono)
    // genera i vettori
    for(int i=2; i<r.rows-2; i++){
        for(int j=2; j<r.cols-2; j++){
            hm.push_back(r.data[r.channels()*(r.cols*i + j) + 0]) ;
            sm.push_back(r.data[r.channels()*(r.cols*i + j) + 1]) ;
            lm.push_back(r.data[r.channels()*(r.cols*i + j) + 2]) ;
        }
    }
    avg[0]=getMediana(hm);
    avg[1]=getMediana(sm);
    avg[2]=getMediana(lm);
}

void FaiMedia(Immagine *m){
    m->cap >> m->src;
    flip(m->src,m->src,1); //Inverte la matrice rispetto all'asse y (n.b le misure vengono fatte intorno all'esse verticale)
    for(int i=0;i<30;i++){
        m->cap >> m->src;
        flip(m->src,m->src,1);
        cvtColor(m->src,m->src,ORIGCOL2COL); //Converte immagine nello spazio dei colori HSV

        for(int j=0;j<NUMCAMPIONI;j++){
            getAvgColor(m,roi[j],avgColor[j]); //Per ogni campione (rettangolino) ottiene un valore medio di tinta, sfumatura e tono
            roi[j].DisegnaRettangolo(m->src);
        }
        cvtColor(m->src,m->src,COL2ORIGCOL); //Converte immagine nello spazion dei colori RGB
        string imgText=string("Ricerca colore medio");
        printText(m->src,imgText);
        imshow("Immagine", m->src);
        if(cv::waitKey(30) >= 0) break;
    }
}

void initTrackbars(){
    for(int i=0;i<NUMCAMPIONI;i++){
        c_lower[i][0]=12;  //H (TONALITA')
        c_upper[i][0]=7;   //H (TONALITA')
        c_lower[i][1]=30;  //S (SATURAZIONE)
        c_upper[i][1]=40;  //S (SATURAZIONE)
        c_lower[i][2]=80;  //B (LUMINOSITA')
        c_upper[i][2]=80;  //B (LUMINOSITA')
    }
    createTrackbar("H-MIN","trackbars",&c_lower[0][0],255);
    createTrackbar("S-MIN","trackbars",&c_lower[0][1],255);
    createTrackbar("V-MIN","trackbars",&c_lower[0][2],255);
    createTrackbar("H-MAX","trackbars",&c_upper[0][0],255);
    createTrackbar("S-MAX","trackbars",&c_upper[0][1],255);
    createTrackbar("V-MAX","trackbars",&c_upper[0][2],255);
}


void NormalizzaColori(Immagine * m){
   //Copia tutti i valori letti dalla trackball per tutti i diversi confini
    for(int i=1;i<NUMCAMPIONI;i++){
        for(int j=0;j<3;j++){
            c_lower[i][j]=c_lower[0][j];
            c_upper[i][j]=c_upper[0][j];
        }
    }
    // normalizza tutti i limiti in base alla mano rilevata
    // soglia all'interno 0-255
    for(int i=0;i<NUMCAMPIONI;i++){
        if((avgColor[i][0]-c_lower[i][0]) <0){
            c_lower[i][0] = avgColor[i][0] ;
        }if((avgColor[i][1]-c_lower[i][1]) <0){
            c_lower[i][1] = avgColor[i][1] ;
        }if((avgColor[i][2]-c_lower[i][2]) <0){
            c_lower[i][2] = avgColor[i][2] ;
        }if((avgColor[i][0]+c_upper[i][0]) >255){
            c_upper[i][0] = 255-avgColor[i][0] ;
        }if((avgColor[i][1]+c_upper[i][1]) >255){
            c_upper[i][1] = 255-avgColor[i][1] ;
        }if((avgColor[i][2]+c_upper[i][2]) >255){
            c_upper[i][2] = 255-avgColor[i][2] ;
        }
    }
}

void produciImgBianarie(Immagine *m){
    Scalar lowerBound;  //limite inferiore
    Scalar upperBound;  // limite superiore
    Mat foo;
    for(int i=0;i<NUMCAMPIONI;i++){
        NormalizzaColori(m);
        lowerBound=Scalar( avgColor[i][0] - c_lower[i][0] , avgColor[i][1] - c_lower[i][1], avgColor[i][2] - c_lower[i][2] );
        upperBound=Scalar( avgColor[i][0] + c_upper[i][0] , avgColor[i][1] + c_upper[i][1], avgColor[i][2] + c_upper[i][2] );
        m->bwList.push_back(Mat(m->srcLR.rows,m->srcLR.cols,CV_8U));
        inRange(m->srcLR,lowerBound,upperBound,m->bwList[i]); //Controlla se gli elementi di srcLR si trovano tra gli elementi di altri 2 array (lowerBound,upperBound) mbiw output //specie di passabanda
    }
    m->bwList[0].copyTo(m->bw);
    for(int i=1;i<NUMCAMPIONI;i++){
        m->bw+=m->bwList[i];  //Somma tutte le immagini binarie
    }
    medianBlur(m->bw, m->bw,7);  //Leviga l'immagine utilizzando un filtro mediano con apertura di 7*7
}

void initWindows(Immagine m){
     namedWindow("trackbars",CV_WINDOW_KEEPRATIO);
     namedWindow("Immagine",CV_WINDOW_FULLSCREEN);
}

void showWindows(Immagine m){
    pyrDown(m.bw,m.bw);
    pyrDown(m.bw,m.bw);
    Rect roi( Point( 3*m.src.cols/4,0 ), m.bw.size());
    vector<Mat> channels;
    Mat result;
    for(int i=0;i<3;i++)
        channels.push_back(m.bw);
    merge(channels,result);
    result.copyTo( m.src(roi));
    imshow("Immagine",m.src);
}

int findBiggestContour(vector<vector<Point> > Contorni){
    int indexOfBiggestContour = -1;
    int sizeOfBiggestContour = 0;
    for (int i = 0; i < Contorni.size(); i++){
        if(Contorni[i].size() > sizeOfBiggestContour){
            sizeOfBiggestContour = Contorni[i].size();
            indexOfBiggestContour = i;
        }
    }
    return indexOfBiggestContour;
}

void myDrawContours(Immagine *m,Mano *manog){

        drawContours(m->src,manog->hullP,manog->cIdx,cv::Scalar(200,0,0),2, 8, vector<Vec4i>(), 0, Point());
        rectangle(m->src,manog->bRect.tl(),manog->bRect.br(),Scalar(0,0,200));
        //m->src:immagine destinazione,manog->bRect.tl():Contorni memorizzati come vettore di punti
        //manog->cIdx:Indica i contorni da disegnare, scalar:colore

        vector<Vec4i>::iterator d=manog->difetti[manog->cIdx].begin();
        vector<Mat> channels;
        Mat result;
        for(int i=0;i<3;i++)
            channels.push_back(m->bw);
        merge(channels,result); //Trasforma un array multicanale in un array monocanale, il numero di canali di output corrisponde al numero di matrici di origine

        drawContours(result,manog->hullP,manog->cIdx,cv::Scalar(0,0,250),10, 8, vector<Vec4i>(), 0, Point());


        while( d!=manog->difetti[manog->cIdx].end() ) {

           Vec4i& v=(*d);
           int startidx=v[0]; Point ptStart(manog->Contorni[manog->cIdx][startidx] );
           int endidx=v[1]; Point ptEnd(manog->Contorni[manog->cIdx][endidx] );
           int faridx=v[2]; Point ptFar(manog->Contorni[manog->cIdx][faridx] );
           //float depth = v[3] / 256;

           //linee e cerchi interni
           line( m->src, ptStart, ptFar, Scalar(0,255,0), 1 );
           line( m->src, ptEnd, ptFar, Scalar(0,255,0), 1 );
           circle( m->src, ptFar,   4, Scalar(0,255,0), 2 );
           circle( m->src, ptEnd,   4, Scalar(0,0,255), 2 );
           circle( m->src, ptStart,   4, Scalar(255,0,0), 2 );

           circle( result, ptFar,   9, Scalar(0,205,0), 5 );


           d++;

     }


}

void CreaContorni(Immagine *m, Mano* manog){
    Mat aBw;
    pyrUp(m->bw,m->bw); //sovraCampiona immagine (semplifica il filtraggo)
    m->bw.copyTo(aBw);
    //Creazione box di delimitazione e ellisse per contorni
    findContours(aBw,manog->Contorni,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE); //Trova contorni di una immagine binaria (array contorni è di output). Mode: I contorni esterni, Method: Memorizza tutti i punti successivi, senza comprimerli
    manog->InializzaVettori();
    manog->cIdx=findBiggestContour(manog->Contorni); //Trova l'indice del più grande contorno(profilo)(l'indice del vettore di punti con dimensione maggiore)
    if(manog->cIdx!=-1){
        manog->bRect=boundingRect(Mat(manog->Contorni[manog->cIdx])); //Calcola il rettangolo di delimitazione minimo per il set di punti specificato
        convexHull(Mat(manog->Contorni[manog->cIdx]),manog->hullP[manog->cIdx],false,true); //Trova l'involucro convesso di un set di punti (hullP output) In questo caso gli elementi sono punti convex hull
        convexHull(Mat(manog->Contorni[manog->cIdx]),manog->hullI[manog->cIdx],false,false); //Trova l'involucro convesso di un set di punti (hullI output) In questo caso gli elementi hull sono gli indici dell'involucro convesso dell'array originale(interi)
        approxPolyDP( Mat(manog->hullP[manog->cIdx]), manog->hullP[manog->cIdx], 18, true ); //Approssima il poligono con un'altra curva con meno vertici, in modo che la distanza tra di loro è <= alla precisione specificata(18), true: curva chiusa
        if(manog->Contorni[manog->cIdx].size()>3 ){
            convexityDefects(manog->Contorni[manog->cIdx],manog->hullI[manog->cIdx],manog->difetti[manog->cIdx]); //Trova i difetti di convessità di un contorno. Output: difetti
            manog->EliminaDifetti(m); //Un difetto è respinto se respinto se l'angolo tra le linee che vanno dal difetto ai vicini vertici poligono convesso ha lunghezza <0.44 lung rettangolo  ed è angolo >95°
        }
        bool isHand=manog->RilevaSeMano();
          //Stampa informazioni sulla mano
         //  manog->StampaInfoGesto(m->src);
        if(isHand){
              manog->getPuntaDita(m);
              manog->DisegnaPuntaDita(m); //Stampa a video il numero di dita rilevate
              myDrawContours(m,manog);
        }

    }
}


int main(){
    Immagine m(0);  //Oggetto immagine e inizializzazione webcam
    Mano manog; // oggetto mano
    inizializza(&m); //Inizializzo il poligono
    m.cap >>m.src; //Ottiene una nuova frame dalla camera
    namedWindow("immagine",CV_WINDOW_KEEPRATIO);
    AttendiCoperturaPalmo(&m);
    FaiMedia(&m); //Fa la media del colore per ogni rettangolino
    destroyWindow("Immagine");
    initWindows(m);
    initTrackbars();
     for(;;){
        manog.NumeroFrame++;  //Incrementa numero frame
        m.cap >> m.src;
        flip(m.src,m.src,1); //Inverte matrice rispetto asse y
        pyrDown(m.src,m.srcLR);//Leviga e campiona l' immagine, srcLR sarà l'immagine di output. Inoltre diminuisce il numero di pixel
        blur(m.srcLR,m.srcLR,Size(3,3)); //Elimina le sfumature e il rumore, srcLR sarà l'immagine di output
        cvtColor(m.srcLR,m.srcLR,ORIGCOL2COL); //Converte immagine nello spazio HSV
        produciImgBianarie(&m); //Produce le immagini binarie, le somma e leviga l'immagine utilizzando un filtro mediano
        cvtColor(m.srcLR,m.srcLR,COL2ORIGCOL); //Converte immagine nello spazio RGB
        CreaContorni(&m, &manog); //Crea contorni (sia rettangolo esterno che quello dei poligoni convessi)
        manog.getNumeroDita(&m); //Rileva num. dita mostrate
        showWindows(m);

        if(cv::waitKey(30) == char('q')) break;
    }
    destroyAllWindows();
   // out.release();
    m.cap.release();
    return 0;
}
