/**********************************************************************************************
    Copyright (C) 2008 Fabrice Crohas fcrohas@gmail.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

**********************************************************************************************/

#ifndef CPICPROCESS_H
#define CPICPROCESS_H

#include "math.h"
#include <QObject>
#include <QImage>
#include <vector>
#include <queue>


struct Point
{
	int x,y;
};

using namespace std;
class CPicProcess : public QObject
{
    Q_OBJECT;
	public:
		/* Constructor */
		CPicProcess(const CPicProcess &Img, QObject * parent);
		CPicProcess(const QImage &Img, QObject * parent);
		CPicProcess(int Width,int Height, QObject * parent);
		virtual ~CPicProcess();

        CPicProcess& operator =(const CPicProcess &Other);
		/* Pixel works */
		int **GetGris(){return myGray;}
		int **GetRed(){return myRed;}
		int **GetGreen(){return myGreen;}
		int **GetBlue(){return myBlue;}

		int GetHeight(){return myHeight;}
		int GetWidth(){return myWidth;}
		int GetPixel(int i,int j){return myGray[i][j];}
		void SetPixel(int i,int j,int val){myGray[i][j]=val;}

		/* debug tools */
		void DrawCosinus(double R,double Theta,float Ponderation);
		void DrawLine(double X,double Y,double Theta,int Val);
		void writeOut(QString szout);
		void writeProj(QString szout);

		/* Picture works */
		void setAutoContrast();
		void setThreashold(int level);
		void Hough(CPicProcess *ImgHough);
		void HoughInv(CPicProcess *ImgHoughInv,CPicProcess *ImgHough);
		void Optimisation();
		void Invert();

		/* Picture binarize */
		double* ComputeGrayLeveldistibutionHistogramm(void);
		int  Binarize(void);
		int ComputeThreshold(double *p);
		void Sobel(CPicProcess *Img);


	private:
		void getGrayLevel(int *min,int *max);

		int **myGray,**myRed,**myGreen,**myBlue;
		int **p_anglerad, **p_projX, **p_projY;
		int myHeight,myWidth;


};


int **AllocT(int **Tab,int Width,int Height);
void DesAllocT(int **Tab,int Width);
void HSL_to_RGB(double H,double S,double L,double *R,double *G,double*B);

#endif
