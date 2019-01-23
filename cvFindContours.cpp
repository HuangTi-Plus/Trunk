// NetCopyTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <opencv2\opencv.hpp>   
#include <windows.h>   



// 获取直方图   
// 1. pImageData   图像数据   
// 2. nWidth       图像宽度   
// 3. nHeight      图像高度   
// 4. nWidthStep   图像行大小   
// 5. pHistogram   直方图   
BOOL GetHistogram(unsigned char *pImageData, int nWidth, int nHeight, int nWidthStep,   
				  int *pHistogram)   
{   
	int            i     = 0;   
	int            j     = 0;   
	unsigned char *pLine = NULL;   
	// 清空直方图   
	memset(pHistogram, 0, sizeof(int) * 256);   
	for (pLine = pImageData, j = 0; j < nHeight; j++, pLine += nWidthStep)   
	{   
		for (i = 0; i < nWidth; i++)   
		{   
			pHistogram[pLine[i]]++;   
		}   
	}   
	return TRUE;   
}   

// 大津法取阈值   
// 1. pImageData   图像数据   
// 2. nWidth       图像宽度   
// 3. nHeight      图像高度   
// 4. nWidthStep   图像行大小   
// 函数返回阈值   
int Otsu(unsigned char *pImageData, int nWidth, int nHeight, int nWidthStep)   
{   
	int    i          = 0;   
	int    j          = 0;   
	int    nTotal     = 0;   
	int    nSum       = 0;   
	int    A          = 0;   
	int    B          = 0;   
	double u          = 0;   
	double v          = 0;   
	double dVariance  = 0;   
	double dMaximum   = 0;   
	int    nThreshold = 0;   
	int    nHistogram[256];   
	// 获取直方图   
	GetHistogram(pImageData, nWidth, nHeight, nWidthStep, nHistogram);   
	for (i = 0; i < 256; i++)   
	{   
		nTotal += nHistogram[i];   
		nSum   += (nHistogram[i] * i);   
	}   
	for (j = 0; j < 256; j++)   
	{   
		A = 0;   
		B = 0;   
		for (i = 0; i < j; i++)   
		{   
			A += nHistogram[i];   
			B += (nHistogram[i] * i);   
		}   
		if (A > 0)   
		{   
			u = B / A;   
		}   
		else  
		{   
			u = 0;   
		}   
		if (nTotal - A > 0)    
		{   
			v = (nSum - B) / (nTotal - A);   
		}   
		else  
		{   
			v = 0;   
		}   
		dVariance = A * (nTotal - A) * (u - v) * (u - v);   
		if (dVariance > dMaximum)   
		{   
			dMaximum = dVariance;   
			nThreshold = j;   
		}   
	}   
	return nThreshold;   
}   


// 二值化   
// 1. pImageData   图像数据   
// 2. nWidth       图像宽度   
// 3. nHeight      图像高度   
// 4. nWidthStep   图像行大小   
// 5. nThreshold   阈值   
BOOL Threshold(unsigned char *pImageData, int nWidth, int nHeight, int nWidthStep,   
			   unsigned int nThreshold)   
{   
	int            i     = 0;   
	int            j     = 0;   
	unsigned char *pLine = NULL;   
	for (pLine = pImageData, j = 0; j < nHeight; j++, pLine += nWidthStep)   
	{   
		for (i = 0; i < nWidth; i++)   
		{   
			if (pLine[i] >= nThreshold)   
			{   
				pLine[i] = 0x00;   
			}   
			else  
			{   
				pLine[i] = 0xff;   
			}   
		}   
	}   
	return TRUE;   
}   


// 二值化图像取反   
void Not(IplImage *edge)   
{   
	int i;   
	int j;   
	for(i=0;i<edge->height;i++)   
		for(j=0;j<edge->width;j++)   
		{   
			if(edge->imageData[edge->height*i+j]==0)   
				edge->imageData[edge->height*i+j]=1;   
			else  
				edge->imageData[edge->height*i+j]=0;   

		}   

}   

BOOL FindContours(unsigned char *pImageData, int nWidth, int nHeight, int nWidthStep)   
{   
	int i = 0;   
	int j = 0;   
	unsigned char *pLine[3] = { NULL, NULL, NULL };  

	for (j = 1; j < nHeight - 1; j++)   
	{   
		pLine[0]  = pImageData + nWidthStep * (j - 1);   
		pLine[1]  = pImageData + nWidthStep * j;   
		pLine[2]  = pImageData + nWidthStep * (j + 1);   
		for (i = 1; i < nWidth - 1; i++)   
		{   
			//在8个邻域内找
			if (pLine[0][i-1] == 0xFF && pLine[0][i] == 0xFF && pLine[0][i+1] == 0xFF &&   
				pLine[1][i-1] == 0xFF && pLine[1][i] == 0xFF && pLine[1][i+1] == 0xFF &&   
				pLine[2][i-1] == 0xFF && pLine[2][i] == 0xFF && pLine[2][i+1] == 0xFF)   
			{   
				pLine[0][i-1] = 0;   
			}   
			else  
			{   
				pLine[0][i-1] = pLine[1][i];   
			}   
		}   
	}   
	return TRUE;   
}   

int main(int argc, char* argv[])   
{   
	IplImage *src = cvLoadImage("e://12.bmp", CV_LOAD_IMAGE_GRAYSCALE);   
	IplImage *dsc = cvCreateImage(cvSize(src->width*0.5,src->height*0.5), src->depth, src->nChannels);   
	// 转换图像大小   
	cvResize(src, dsc, CV_INTER_LINEAR);   
	src->width*=0.5;   
	src->height*=0.5;   

	// 复制图像    
	// 两个图像的大小类型通道数必须相同   
	cvCopy(dsc, src );   
	cvNamedWindow("src", CV_WINDOW_AUTOSIZE);   
	cvShowImage("src", src);   

	// 取图像阀值   
	unsigned int threshold = Otsu((unsigned char *)src->imageData, src->width, src->height, src->widthStep);   

	// 图像二值化   
	if (TRUE == Threshold((unsigned char *)src->imageData, src->width, src->height, src->widthStep, threshold))   
	{   
		cvNamedWindow("dsc", CV_WINDOW_AUTOSIZE);   
		cvShowImage("dsc", src);   

		// 图像取反   
		// cvNot(src, src);   
		FindContours((unsigned char *)src->imageData,src->width , src->height, src->widthStep);   
		cvNamedWindow("not", CV_WINDOW_AUTOSIZE);   
		cvShowImage("not", src);   
		cvWaitKey(0);   


	}   
	cvDestroyAllWindows();   
	cvReleaseImage(&src);   
	cvReleaseImage(&dsc);   

	return 0;   
} 