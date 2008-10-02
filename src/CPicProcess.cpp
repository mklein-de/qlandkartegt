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
#include "CPicProcess.h"
#include <string.h>
#include <QtGui>

#define N 256
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define ABSOLU(x) ( x < 0 ? -x : x )
const double Pi = 3.141592653589793238462643383279;

/*
 * Create empty picture container with size
 */
// CPicProcess::CPicProcess(int Width,int Height, QObject * parent)
// {
// 	myHeight   = Height;
// 	myWidth    = Width;
// 	myGray     = 0;
// 	myRed      = 0;
// 	myGreen    = 0;
// 	myBlue     = 0;
// 	p_anglerad = 0;
// 	p_projX    = 0;
// 	p_projY    = 0;
// 	myGray     = AllocT(myGray,myWidth,myHeight);
// 	p_projX    = AllocT(p_projX,myWidth,myHeight);
// 	p_projY    = AllocT(p_projY,myWidth,myHeight);
//
// 	for(int i=myWidth-1;i>=0;i--){
// 		for(int j=myHeight-1;j>=0;j--){
// 			p_projX[i][j]=0;
// 			p_projY[i][j]=0;
// 			myGray[i][j]=0;
// 		}
//     }
// }

/*
 * Create clone gray picture from another one
 */
// CPicProcess::CPicProcess(const CPicProcess &Img, QObject * parent)
// {
// 	myHeight   = Img.myHeight;
// 	myWidth    = Img.myWidth;
// 	myGray     = 0;
// 	myRed      = 0;
// 	myGreen    = 0;
// 	myBlue     = 0;
// 	p_anglerad = 0;
// 	p_projX    = 0;
// 	p_projY    = 0;
// 	myGray     = AllocT(myGray,myWidth,myHeight);
// 	p_projX    = AllocT(p_projX,myWidth,myHeight);
// 	p_projY    = AllocT(p_projY,myWidth,myHeight);
//
//
// 	for(int i=myWidth-1;i>=0;i--){
// 		for(int j=myHeight-1;j>=0;j--){
// 			myGray[i][j]=Img.myGray[i][j];
//         }
//     }
//
// 	for(int i=myWidth-1;i>=0;i--){
// 		for(int j=myHeight-1;j>=0;j--){
// 			p_projX[i][j]=0;
// 			p_projY[i][j]=0;
// 		}
//     }
// }

/*
 * Create picture container from QImage
 */
CPicProcess::CPicProcess(const QImage &Img, QObject * parent)
: QObject(parent)
, myGray(0)
, myRed(0)
, myGreen(0)
, myBlue(0)
, p_anglerad(0)
, p_projX(0)
, p_projY(0)
{
    myWidth    = Img.width();
    myHeight   = Img.height();
    myGray     = AllocT(myGray,myWidth,myHeight);
    myRed      = AllocT(myRed,myWidth,myHeight);
    myGreen    = AllocT(myGreen,myWidth,myHeight);
    myBlue     = AllocT(myBlue,myWidth,myHeight);
    p_projX    = AllocT(p_projX,myWidth,myHeight);
    p_projY    = AllocT(p_projY,myWidth,myHeight);

    for(int i=0;i<myWidth;i++) {
        for(int j=0;j<myHeight;j++) {
            myRed[i][j]      = qRed( Img.pixel(i,j) );
            myGreen[i][j]    = qGreen( Img.pixel(i,j) );
            myBlue[i][j]     = qBlue( Img.pixel(i,j) );
            myGray[i][j]     = qGray( Img.pixel(i,j) );
            /*
                        if ( (qBlue( Img.pixel(i,j)) > qGreen( Img.pixel(i,j)) ) && (qBlue( Img.pixel(i,j)) > qRed( Img.pixel(i,j)) ) )
                            myGray[i][j]=0;
                        else
                            myGray[i][j]=255;
            */
        }
    }
}


/*
 * Simple copy operator for class
 */
CPicProcess& CPicProcess::operator =(const CPicProcess &Other)
{
    if(this->myHeight!=Other.myHeight || this->myWidth!=Other.myWidth)
        return *this;

    if(this==&Other || Other.myGray==0)
        return *this;

    for(int i=myWidth-1;i>=0;i--) {
        for(int j=myHeight-1;j>=0;j--) {
            myGray[i][j]=Other.myGray[i][j];
        }
    }
    return *this;
}


/*
 * Destructor
 */
CPicProcess::~CPicProcess()
{
    DesAllocT(myGray,myWidth);
    DesAllocT(p_projY,myWidth);
    DesAllocT(p_projX,myWidth);

    DesAllocT(myRed,myWidth);
    DesAllocT(myGreen,myWidth);
    DesAllocT(myBlue,myWidth);
}


/*
 * Get Min / Max level of gray from current picture
 */
void CPicProcess::getGrayLevel(int *min,int *max)
{
    *min=N-1;
    *max=0;
    for(int i=myWidth-1;i>=0;i--) {
        for(int j=myHeight-1;j>=0;j--) {
            if(myGray[i][j]>*max) {
                *max = myGray[i][j];
            }
            if(myGray[i][j]<*min) {
                *min = myGray[i][j];
            }
        }
    }
}


/*
 * Set contrast from gray levvels
 */
void CPicProcess::setAutoContrast()
{
    int Min,Max;

    getGrayLevel(&Min,&Max);
    if(Max==0)return;
    for(int i=myWidth-1;i>=0;i--) {
        for(int j=myHeight-1;j>=0;j--) {
            myGray[i][j]=(myGray[i][j]*(N-1))/(Max-Min)-((N-1)*Min)/(Max-Min);
        }
    }

}


/*
 * set threshold value
 */
void CPicProcess::setThreashold(int level)
{
    for(int i=myWidth-1;i>=0;i--) {
        for(int j=myHeight-1;j>=0;j--) {
            if(myGray[i][j]>level) {
                myGray[i][j] = N-1;
            }
            else {
                myGray[i][j]=0;
            }
        }
    }
}


/*
 * Invert picture gray levels
 */
void CPicProcess::Invert()
{
    for(int i=myWidth-1;i>=0;i--) {
        for(int j=myHeight-1;j>=0;j--) {
            myGray[i][j]=(N-1)-myGray[i][j];
        }
    }

}


/*
 * Simple Sobel filter
 */
void CPicProcess::Sobel(CPicProcess *Img)
{
    int ConvY[3][3]= { {
            -1,-2,-1
        }
        ,
        {
            0,0,0
        }
        ,
        {
            1,2,1
        }
    };
    int ConvX[3][3]= { {
            -1,0,1
        }
        ,
        {
            -2,0,2
        }
        ,
        {
            -1,0,1
        }
    };
    int mX,mY;

    for(int i=myWidth-2;i>=1;i--) {
        for(int j=myHeight-2;j>=1;j--) {
            mX=0;
            mY=0;

            for(int k=-1;k<=1;k++) {
                for(int l=-1;l<=1;l++) {
                    mX+=myGray[i+k][j+l]*ConvX[k+1][l+1];
                    mY+=myGray[i+k][j+l]*ConvY[k+1][l+1];
                }
            }

            Img->myGray[i][j]=(int) MIN(sqrt((double)mX*mX+mY*mY),255);
        }
    }
}


/*
 * Create gray level histogram
 */
double* CPicProcess::ComputeGrayLeveldistibutionHistogramm(void)
{
    double * GrayLevelHist = new double [256];
    memset(GrayLevelHist, 0.0, 256 * sizeof(double)) ;
    for(int j = 0 ; j < myHeight ; j++ ) {
        for(int i = 0 ; i < myWidth ; i++ ) {
            GrayLevelHist[myGray[i][j]] += 1.0 ;
        }
    }
    // calculate for each pixel which one have same gray level

    unsigned Sum = 0;
    for(int j = 0 ; j < 256 ; j++ ) {
        Sum += GrayLevelHist[j] ;
    }

    for(int j = 0 ; j < 256 ; j++ ) {
        GrayLevelHist[j] /= Sum ;
    }
    return GrayLevelHist;
}


/*
 * Otsu binarize analyst
 */
int  CPicProcess::Binarize(void)
{
    //1: histogram
    double* pHist = ComputeGrayLeveldistibutionHistogramm();
    //2:
    int nThreshold = ComputeThreshold(pHist);
    //3: free  Memory
    delete [] pHist;

    return nThreshold ;
}


/*
 * OTsu threshold finder
 * Description:
 * :S_W=p1*s1+p2*s2
 * : mu1 is the mean graylevel of pixels bellow the treshold,
 * : mu2 for is the mean graylevel of pixels +pixels above the treshold,
 * : mu  is the mean graylevel for all pixels.
 * : s1 is a variance for pixels bellow the treshold,
 * : s2 for those above.
 * : S_W is a so called within-class variance, which states, how big variance
 * :is there in average in our two classes. We would like to keep this small,
 * :since if we group components correctly, it should be smallest.
 * :S_B is a so called between-class variance, which states, how far from
 * each other are our two classes. For binarization we wish to keep them as far
 * from each other as possible, so we want to maximize this.
 *Thus, we maxmimize S_B/S_W, or minimize S_W/S_B to find the treshold.
 * S_B=p1(mu1-mu)^2+p2(mu2-mu)^2
 *
 * J(t)=S_B/S_W
 */
int CPicProcess::ComputeThreshold(double *p)
{
    // histogram histo;
    int nThreshold;
    double criterion;
    double expr_1;

    // double p[256];
    double omega_k;
    double sigma_b_k;
    double sigma_T;
    double fMu_T;
    double fMu_k;
    int k_low, k_high;
    double fMu_0;
    double fMu_1;
    double fMu;

    fMu_T = 0.0;
    for (int i = 0 ; i < 256 ; i++) {
        fMu_T += i * p[i];
    }
    //Standard deviation
    sigma_T = 0.0;
    for (int i = 0; i < 256 ; i++) {
        sigma_T += (i - fMu_T ) * (i - fMu_T ) * p[i];
    }

    //Means for classes c1 and c2
    //c'est un peu lger!!!!
    for (k_low = 0; (p[k_low] == 0) && (k_low < 255); k_low++);
    for (k_high =255; (p[k_high] == 0) && (k_high > 0); k_high--);

    criterion = 0.0;
    nThreshold = 127;
    fMu_0 = 126.0;
    fMu_1 = 128.0;

    omega_k = 0.0;
    fMu_k = 0.0;
    //minimize S_W/S_B to find the treshold
    for (int k = k_low ; k <= k_high ; k++) {
        omega_k += p[k];
        if( omega_k == 0 ||omega_k == 1 )
            continue;
        fMu_k += k*p[k];

        expr_1 = ( fMu_T * omega_k - fMu_k );
        sigma_b_k = expr_1 * expr_1 / ( omega_k * ( 1 - omega_k ));
        if (  criterion < sigma_b_k / sigma_T ) {
            criterion = sigma_b_k / sigma_T;
            nThreshold = k;
            fMu_0 = fMu_k / omega_k;
            fMu_1 = (fMu_T-fMu_k) / ( 1 - omega_k );
        }
    }
    fMu = fMu_T;
    return ( nThreshold );

}


/*
 * Hough Analyze
 */
void CPicProcess::Hough(CPicProcess *ImgHough)
{
    int X,Y;
    double R,Theta;
    int angle=0;
    p_anglerad=0;
    ImgHough->p_anglerad=0;
    ImgHough->p_anglerad = AllocT( ImgHough->p_anglerad,63,1);
    //initialisation  0
    for(int i=0;i<63;i++) {
        ImgHough->p_anglerad[i][0]=0;
    }

    //initialisation  0
    for(int i=ImgHough->myWidth-1;i>=0;i--) {
        for(int j=ImgHough->myHeight-1;j>=0;j--) {
            ImgHough->myGray[i][j]=0;
        }
    }

    //Pour tous les pixels , on trace le cosinus correspondant pondr
    //suivant la valeur du gradient
    int projY=0;
    int projX=0;
    for(int i=myWidth-1;i>=0;i--) {
        projX=0;
        for(int j=myHeight-1;j>=0;j--) {
            if(myGray[i][j]==N-1) {
                X        = i - (myWidth/2);
                Y        = (myHeight/2) - j;
                R        = sqrt((double)X*X+Y*Y);
                Theta    = atan((double)Y/X)+Pi/2;
                if(X>0)Theta+=Pi;
                angle    = int(Theta*10);
                //ImgHough->p_anglerad[angle][0]+=1;
                // Project only pixel for 0 to 2 rad
                if ( 1.55 < Theta && Theta < 1.59) {
                    ImgHough->DrawCosinus(R,Theta,1);
                    p_projX[i][projX] = 255;
                    //p_projX[i][0]+=1;
                    //p_projX[i][1]+=angle;
                    projX += 1;
                }

            }
        }
    }

    for(int j=myHeight-1;j>=0;j--) {
        projY=0;
        for(int i=myWidth-1;i>=0;i--) {
            if(myGray[i][j]==N-1) {
                X        = i - (myWidth/2);
                Y        = (myHeight/2) - j;
                R        = sqrt((double)X*X+Y*Y);
                Theta    = atan((double)Y/X)+Pi/2;
                if(X>0)Theta += Pi;
                angle    = int(Theta*10);
                //ImgHough->p_anglerad[angle][0]+=1;
                // Project only pixel for 0 to 2 rad
                if ( Theta < 0.2) {
                    p_projY[projY][j] = 255;
                    //p_projY[0][j]+=1;
                    //p_projY[1][j]+=angle;
                    projY += 1;
                }
            }
        }
    }

    setAutoContrast();
    double Red,G,B;
    if(ImgHough->myRed==0) {
        ImgHough->myRed     =   AllocT(ImgHough->myRed,ImgHough->myWidth,ImgHough->myHeight);
        ImgHough->myGreen   =   AllocT(ImgHough->myGreen,ImgHough->myWidth,ImgHough->myHeight);
        ImgHough->myBlue    =   AllocT(ImgHough->myBlue,ImgHough->myWidth,ImgHough->myHeight);
    }
    //On normalise et on augmente le contraste de 0 a 255

    for(int i=ImgHough->myWidth-1;i>=0;i--) {
        for(int j=ImgHough->myHeight-1;j>=0;j--) {
            HSL_to_RGB(20,240,ImgHough->myGray[i][j],&Red,&G,&B);
            ImgHough->myRed[i][j]   = Red;
            ImgHough->myGreen[i][j] = G;
            ImgHough->myBlue[i][j]  = B;
        }
    }
    DesAllocT(ImgHough->p_anglerad,63);
}


/*
 * Hough line drawing
 */
void CPicProcess::HoughInv(CPicProcess *ImgHoughInv,CPicProcess *ImgHough)
{
    double X,Y,Theta,R;
    //initialisation  0
    for(int i=ImgHoughInv->myWidth-1;i>=0;i--) {
        for(int j=ImgHoughInv->myHeight-1;j>=0;j--) {
            ImgHoughInv->myGray[i][j]=0;
        }
    }

    double Pas=Pi/(myWidth);

    for(int i=myWidth-1;i>=0;i--) {
        for(int j=myHeight-1;j>=0;j--) {
            if(myGray[i][j]>0) { //==N-1)
                Theta   =  i*Pas;
                R       =  (myHeight/2)-j;
                Y       =  R*cos(Theta);
                X       = -R*sin(Theta);
                ImgHoughInv->DrawLine(X,Y,Theta, ImgHough->myGray[i][j]);
            }
        }
    }
}


/*
 * Draw cosinus from R and Theta
 */
void CPicProcess::DrawCosinus(double R,double Theta,float Ponderation)
{

    double x,y;
    double Pas=Pi/(myWidth/2);

    //for each pixel of the graph, we evaluate the y coord with cosinus
    for(double i=0;i<myWidth;i++) {
        x  = i;
        x *= Pas;                //conversion into radian
        y  = R*cos(x-Theta);
        myGray[(int)i][(int)(myHeight/2-y)]+=Ponderation;
    }
}


/*
 * Draw line from angle
 */
void CPicProcess::DrawLine(double X,double Y,double Theta,int Val)
{
    //Vecteur is the direction
    double Vecteur=tan(Theta);
    double Vecteur2=tan(Theta-(double)Pi/2);

    X+=myWidth/2;
    Y=myHeight/2-Y;
    if(X>myWidth || X<0 || Y>myHeight || Y<0)
        return;

    double i,j;
    if(X>=200 && Y<=200 )
        return;

    for(i=X;i<myWidth;i++) {
        if((j=Y-Vecteur*(i-X))>myHeight || j<=0)
            break;
        myGray[(int)i][(int)j]=Val;
    }

    //We do the same thing in the other way
    for(i=X;i>=0;i--) {
        if((j=Y-Vecteur*(i-X))>myHeight || j<0)
            break;
        myGray[(int)i][(int)j]=Val;
    }

    for(j=Y;j<myHeight;j++) {
        if((i=X+Vecteur2*(j-Y))>myWidth || i<0)
            break;
        myGray[(int)i][(int)j]=Val;
    }

    //We do the same thing in the other way
    for(j=Y;j>=0;j--) {
        if((i=X+Vecteur2*(j-Y))>myWidth || i<0)
            break;
        myGray[(int)i][(int)j]=Val;
    }
}


/*
 * Detect connexes pixel and keept only one
 */
void CPicProcess::Optimisation()
{
    int i,j,MoyX,MoyY,Cpt;
    queue<Point>File,Save;
    Point Pt,Pt_Neighbor;

    //pour tous les pixels de l'image
    for(Pt.x=myWidth-1;Pt.x>=0;Pt.x--) {
        for(Pt.y=myHeight-1;Pt.y>=0;Pt.y--) {
                                 //Si on est sur un pixel significatif
            if(myGray[Pt.x][Pt.y]==(N-1)) {
                myGray[Pt.x][Pt.y]=0;
                File.push(Pt);   //on empile ce pixel

                Cpt=0;
                MoyX=0;
                MoyY=0;

                do {
                                 //on le dpile et on regarde ses voisins
                    Pt=File.front();
                    File.pop();

                    Cpt++;
                    MoyX+=Pt.x;
                    MoyY+=Pt.y;

                    for(i=-1;i<=1;i++) {
                        for(j=-1;j<=1;j++) {
                            if((Pt.x + i >= 0) && (Pt.y + j >= 0) && (Pt.x + i < myWidth) && (Pt.y + j < myHeight) && (myGray[Pt.x+i][Pt.y+j] == (N-1))) {
                                Pt_Neighbor.x = Pt.x + i;
                                Pt_Neighbor.y = Pt.y + j;
                                myGray[Pt_Neighbor.x][Pt_Neighbor.y] = 0;
                                 //on empile le pixel voisin
                                File.push(Pt_Neighbor);
                            }
                        }
                    }
                }
                while(!File.empty());
                if(Cpt!=0) {
                    Pt.x=MoyX/Cpt;
                    Pt.y=MoyY/Cpt;
                    Save.push(Pt);
                }
            }
            while(!Save.empty()) {
                Pt=Save.front(); //on le dpile le pixel et on le marque
                Save.pop();
                myGray[Pt.x][Pt.y]=(N-1);
            }
        }
    }
}


/*
 * Debug write output png picture
 */
void CPicProcess::writeOut(QString szout)
{
    QImage imgOut( myWidth, myHeight, QImage::Format_RGB32);
    for(int i=myWidth-1;i>=0;i--) {
        for(int j=myHeight-1;j>=0;j--) {
            imgOut.setPixel(i,j, qRgb(myGray[i][j], myGray[i][j], myGray[i][j]) );
        }
    }
    imgOut.save(szout, "PNG" );
}


CPicProcess::operator const QPixmap&()
{
    QImage imgOut = QImage( myWidth, myHeight, QImage::Format_RGB32);
    for(int i=myWidth-1;i>=0;i--) {
        for(int j=myHeight-1;j>=0;j--) {
            imgOut.setPixel(i,j, qRgb(myGray[i][j], myGray[i][j], myGray[i][j]) );
        }
    }

    buffer = QPixmap::fromImage(imgOut);
    return buffer;
}


/*
 * Debug write projection X / Y table
 */
void CPicProcess::writeProj(QString szout)
{
    QImage imgOut( myWidth, myHeight, QImage::Format_RGB32);
    for(int i=myWidth-1;i>=0;i--) {
        for(int j=myHeight-1;j>=0;j--) {
            imgOut.setPixel(i,j, qRgb(p_projX[i][j], myGray[i][j], p_projY[i][j]) );
        }
    }
    imgOut.save(szout, "PNG" );
}


/*
 * Allocation of table
 */
int **AllocT(int **Tab,int Width,int Height)
{
    if(Tab!=0)
        return Tab;

    Tab = new int*[Width];

    for(int i=0;i<Width;i++) {
        Tab[i] = new int[Height];
    }

    return Tab;
}


/*
 * DesAllocation of table
 */
void DesAllocT(int **Tab,int Width)
{
    if(Tab == 0)return;

    for(int i=0;i<Width;i++) {
        delete []Tab[i];
    }

    delete [] Tab;
    Tab = 0;
}


/*
 * Convert color
 */
                                 //Function Hue_2_RGB
double Hue_2_RGB(double v1,double v2,double vH )
{
    if ( vH < 0 ) vH += 1;
    if ( vH > 1 ) vH -= 1;
    if ( ( 6 * vH ) < 1 ) return ( v1 + ( v2 - v1 ) * 6 * vH );
    if ( ( 2 * vH ) < 1 ) return ( v2 );
    if ( ( 3 * vH ) < 2 ) return ( v1 + ( v2 - v1 ) * ( ( 0.666666 ) - vH ) * 6 );
    return v1;
}


/*
 * Convert color
 */

void HSL_to_RGB(double H,double S,double L,double *R,double *G,double*B)
{
    double tmp1,tmp2;
    H=(double)H/239;             //h,s,l compris entre 0 et 1
    S=(double)S/240;
    L=(double)L/240;

    if(S==0) {
        *R = 255 * L;
        *G = 255 * L;
        *B = 255 * L;
    }
    else {
        if(L<0.5) {
            tmp2 = L * (1 + S);
        }
        else {
            tmp2 = (L + S) - (L * S);
        }
        tmp1=2 * L - tmp2;

        *R = 255 * Hue_2_RGB( tmp1, tmp2, H + ( 0.333333 ) );
        *G = 255 * Hue_2_RGB( tmp1, tmp2, H );
        *B = 255 * Hue_2_RGB( tmp1, tmp2, H - ( 0.333333) );

    }
}
