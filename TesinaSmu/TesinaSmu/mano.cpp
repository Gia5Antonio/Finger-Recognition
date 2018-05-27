#include "mano.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>

using namespace cv;
using namespace std;

Mano::Mano(){
    NumeroFrame=0;
    NrDita=0;
    fontFace = FONT_HERSHEY_PLAIN;
}

void Mano::InializzaVettori(){
    hullI=vector<vector<int> >(Contorni.size());
    hullP=vector<vector<Point> >(Contorni.size());
    difetti=vector<vector<Vec4i> > (Contorni.size());
}

void Mano::AnalizzaContorni(){
    bRect_height=bRect.height;
    bRect_width=bRect.width;
}

string Mano::bool2string(bool tf){
    if(tf)
        return "true";
    else
        return "false";
}

string Mano::intToString(int number){
        stringstream ss;
        ss << number;
        string str = ss.str();
        return str;
}

void Mano::StampaInfoGesto(Mat src){
    int fontFace = FONT_HERSHEY_TRIPLEX;
    Scalar fColor(0,0,255);

    int xpos=55;
    int ypos=220;
    float fontSize=1.50f;
    if(!isHand)
    putText(src,"MANO NON RILEVATA!",Point(xpos,ypos),fontFace,fontSize,fColor);
}

bool Mano::RilevaSeMano(){  //Analizza il rettangolo esterno
    AnalizzaContorni();
    double h = bRect_height;
    double w = bRect_width;
    isHand=true;
    if(PuntaDita.size() > 5 ){
        isHand=false;
    }else if(h==0 || w == 0){
        isHand=false;
    }else if(h/w > 4 || w/h >4){
        isHand=false;
    }else if(bRect.x<20){
        isHand=false;
    }
    return isHand;
}

float Mano::distanzaP2P(Point a, Point b){
    float d= sqrt(fabs( pow(a.x-b.x,2) + pow(a.y-b.y,2) )) ;
    return d;
}

// rimuovi punta della dita che sono troppo vicini
void Mano::RimuoviPuntaDelleDitaRidondanti(){
    vector<Point> newFingers;
    for(int i=0;i<PuntaDita.size();i++){
        for(int j=i;j<PuntaDita.size();j++){
            if(distanzaP2P(PuntaDita[i],PuntaDita[j])<10 && i!=j){
            }else{
                newFingers.push_back(PuntaDita[i]);
                break;
            }
        }
    }
    PuntaDita.swap(newFingers);
}

void Mano::CalcolaNumDita(){
    std::sort(NumeroDita.begin(), NumeroDita.end());
    int frequentNr;
    int thisNumberFreq=1;
    int highestFreq=1;
    frequentNr=NumeroDita[0];
    for(int i=1;i<NumeroDita.size(); i++){
        if(NumeroDita[i-1]!=NumeroDita[i]){
            if(thisNumberFreq>highestFreq){
                frequentNr=NumeroDita[i-1];
                highestFreq=thisNumberFreq;
            }
            thisNumberFreq=0;
        }
        thisNumberFreq++;
    }
    if(thisNumberFreq>highestFreq){
        frequentNr=NumeroDita[NumeroDita.size()-1];
    }
    NumeroDitaFrequenti=frequentNr;
}

void Mano::AggiungiNumeroDitaAlVettore(){
    int i=PuntaDita.size();
    NumeroDita.push_back(i);
}

// Aggiungere il num calcolato di dita per immagine(frame) m->src
void Mano::AggiungiNumerotoImg(Immagine *m){
    int xPos=10;
    int yPos=25;
    int offset=10;
    float fontSize=1.7f;
    int fontFace = FONT_HERSHEY_PLAIN;
    for(int i=0;i<numbers2Display.size();i++){
        circle(m->src,Point(xPos+offset,yPos+offset),25,numeroColore, 2);

        putText(m->src, intToString(numbers2Display[i]),Point(xPos+3,yPos+offset+5),fontFace,fontSize,numeroColore);
        xPos+=50;
        if(xPos>(m->src.cols-m->src.cols/3.2)){
            yPos+=50;
            xPos=10;
        }
    }


}

// Calcola il numero di dita più frequenti
// su 20 frames
void Mano::getNumeroDita(Immagine *m){
    RimuoviPuntaDelleDitaRidondanti();
    if(bRect.height > m->src.rows/2 && NrDita>12 && isHand ){
        numeroColore=Scalar(0,200,0);
        AggiungiNumeroDitaAlVettore();
        if(NumeroFrame>12){
            NrDita=0;
            NumeroFrame=0;
            CalcolaNumDita();
            numbers2Display.push_back(NumeroDitaFrequenti);
            NumeroDita.clear();
        }else{
            NumeroFrame++;
        }
    }else{
        NrDita++;
        numeroColore=Scalar(200,200,200);
    }
    AggiungiNumerotoImg(m);
}

float Mano::getAngolo(Point s, Point f, Point e){
    float l1 = distanzaP2P(f,s);
    float l2 = distanzaP2P(f,e);
    float dot=(s.x-f.x)*(e.x-f.x) + (s.y-f.y)*(e.y-f.y);
    float angle = acos(dot/(l1*l2));
    angle=angle*180/PI;
    return angle;
}

void Mano::EliminaDifetti(Immagine *m){
    int tolleranza =  bRect_height/5;
    float angleTol=95;
    vector<Vec4i> newdifetti;
    int startidx, endidx, faridx;
    vector<Vec4i>::iterator d=difetti[cIdx].begin();
    while( d!=difetti[cIdx].end() ) {
        Vec4i& v=(*d);
        startidx=v[0]; Point ptStart(Contorni[cIdx][startidx] );
        endidx=v[1]; Point ptEnd(Contorni[cIdx][endidx] );
        faridx=v[2]; Point ptFar(Contorni[cIdx][faridx] );
        if(distanzaP2P(ptStart, ptFar) > tolleranza && distanzaP2P(ptEnd, ptFar) > tolleranza && getAngolo(ptStart, ptFar, ptEnd  ) < angleTol ){
            if( ptEnd.y > (bRect.y + bRect.height -bRect.height/4 ) ){
            }else if( ptStart.y > (bRect.y + bRect.height -bRect.height/4 ) ){
            }else {
                newdifetti.push_back(v);
            }
        }
        d++;
    }
    NrDiDifetti=newdifetti.size();
    difetti[cIdx].swap(newdifetti);
    RimuoviPuntiFinaliRidondanti(difetti[cIdx], m);
}

//Rimuove i punti estremi se sono sullo stesso dito
void Mano::RimuoviPuntiFinaliRidondanti(vector<Vec4i> newdifetti,Immagine *m){
    Vec4i temp;
    float avgX, avgY;
    float tolleranza=bRect_width/6;
    int startidx, endidx, faridx;
    int startidx2, endidx2;
    for(int i=0;i<newdifetti.size();i++){
        for(int j=i;j<newdifetti.size();j++){
            startidx=newdifetti[i][0]; Point ptStart(Contorni[cIdx][startidx] );
            endidx=newdifetti[i][1]; Point ptEnd(Contorni[cIdx][endidx] );
            startidx2=newdifetti[j][0]; Point ptStart2(Contorni[cIdx][startidx2] );
            endidx2=newdifetti[j][1]; Point ptEnd2(Contorni[cIdx][endidx2] );
            if(distanzaP2P(ptStart,ptEnd2) < tolleranza ){
                Contorni[cIdx][startidx]=ptEnd2;
                break;
            }if(distanzaP2P(ptEnd,ptStart2) < tolleranza ){
                Contorni[cIdx][startidx2]=ptEnd;
            }
        }
    }
}


// metodo che controlla l'assenza dei difetti convessità
void Mano::VerificaPerUnDito(Immagine *m){
    int yTol=bRect.height/6;
    Point highestP;
    highestP.y=m->src.rows;
    vector<Point>::iterator d=Contorni[cIdx].begin();
    while( d!=Contorni[cIdx].end() ) {
        Point v=(*d);
        if(v.y<highestP.y){
            highestP=v;

        }
        d++;
    }int n=0;
    d=hullP[cIdx].begin();
    while( d!=hullP[cIdx].end() ) {
        Point v=(*d);

        if(v.y<highestP.y+yTol && v.y!=highestP.y && v.x!=highestP.x){
            n++;
        }
        d++;
    }if(n==0){
        PuntaDita.push_back(highestP);
    }
}

void Mano::DisegnaPuntaDita(Immagine *m){  //Disegna i punti per ogni dito
    Point p;
    //int k=0;
    for(int i=0;i<PuntaDita.size();i++){
        p=PuntaDita[i];
        putText(m->src,intToString(i),p-Point(0,30),fontFace, 1.2f,Scalar(220,210,200),2);
        circle( m->src,p, 5, Scalar(100,255,100), 4 );
     }
}

void Mano::getPuntaDita(Immagine *m){
    PuntaDita.clear();
    int i=0;
    vector<Vec4i>::iterator d=difetti[cIdx].begin();
    while( d!=difetti[cIdx].end() ) {
        Vec4i& v=(*d);
        int startidx=v[0]; Point ptStart(Contorni[cIdx][startidx] );
        int endidx=v[1]; Point ptEnd(Contorni[cIdx][endidx] );
        int faridx=v[2]; Point ptFar(Contorni[cIdx][faridx] );
        if(i==0){
            PuntaDita.push_back(ptStart);
            i++;
        }
        PuntaDita.push_back(ptEnd);
        d++;
        i++;
    }
    if(PuntaDita.size()==0){
        VerificaPerUnDito(m); //verifica la presenza di difetti di convessità
    }
}

