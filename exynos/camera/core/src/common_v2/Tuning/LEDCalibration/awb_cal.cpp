/* #define LOG_NDEBUG 0 */
#define LOG_TAG "ExynosCameraComputeAvgValue"
#include <log/log.h>
#include <iostream>

#include "awb_cal.h"

#define AVG_BOUNDARY 25

int ComputeAvgValue(const int w, const int h, const int bayerOrder,
                    char* bayerMemory,
                    const int cx, const int cy, const int pedestal, unsigned short* out)
{
	int c, i, j, w1, h1, nSize, nCentX, nCentY;
	w1 = w / 2;
	h1 = h / 2;
	nCentX = cx / 2;
	nCentY = cy / 2;
	nSize = (2 * AVG_BOUNDARY + 1) * (2 * AVG_BOUNDARY + 1);
	double pAvg[4] = {0.0};
	double avg[4] = {0.0};
	unsigned short **p2Img = NULL;
	unsigned short ***p3Img = NULL;

	if(nCentX - AVG_BOUNDARY < 0 || nCentX + AVG_BOUNDARY > w1 - 1 || nCentY - AVG_BOUNDARY < 0 || nCentY + AVG_BOUNDARY > h1 - 1)
	{
        ALOGE("%s[%d]:[MotFactory] nCentX(%d) w1(%d). or, nCentY(%d) h1(%d) is wierd. please check",
            __FUNCTION__,
            __LINE__,
            nCentX, w1,
            nCentY, h1);
		return ERROR_COORDINATE;
	}

	p2Img = new unsigned short*[h];
    char *curPtr = bayerMemory;

	for(i = 0; i < h; i++)
	{
		p2Img[i] = new unsigned short[w];
        memcpy(p2Img[i], curPtr, sizeof(unsigned short) * w);

        curPtr += (sizeof(unsigned short) * w);
	}

	// Spilt 4 channels
	p3Img = new unsigned short**[4];
	for(c = 0; c < 4; c++)
	{
		p3Img[c] = new unsigned short*[h1];
		for(i = 0; i < h1; i++)
		{
			p3Img[c][i] = new unsigned short[w1];
		}
	}

	for(i = 0 ; i < h; i++)
	{
		for(j = 0; j < w; j++)
		{
			if((i%2==0) && (j%2==0))
			{
				p3Img[0][(i/2)][(j/2)] = p2Img[i][j];
			}
			else if((i%2==0) && (j%2==1))
			{
				p3Img[1][(i/2)][(j/2)] = p2Img[i][j];
			}
			else if((i%2==1) && (j%2==0))
			{
				p3Img[2][(i/2)][(j/2)] = p2Img[i][j];
			}
			else if((i%2==1) && (j%2==1))
			{
				p3Img[3][(i/2)][(j/2)] = p2Img[i][j];
			}
		}
	}

	for(c = 0; c < 4; c++)
	{
		for(i = -AVG_BOUNDARY; i <= AVG_BOUNDARY; i++)
		{
			for(j = -AVG_BOUNDARY; j <= AVG_BOUNDARY; j++)
			{
				pAvg[c] += p3Img[c][nCentY + i][nCentX + j];
			}
		}
	}

	for(c = 0; c < 4; c++)
	{
		pAvg[c] /= nSize;
	}

	if(bayerOrder == BAYER_R_FIRST) // rggb to rggb
	{
		memcpy(avg, pAvg, sizeof(double) * 4);
	}
	else if(bayerOrder == BAYER_GR_FIRST) // grbg to rggb
	{
		avg[0] = pAvg[1];
		avg[1] = pAvg[0];
		avg[2] = pAvg[3];
		avg[3] = pAvg[2];
	}
	else if(bayerOrder == BAYER_GB_FIRST) // gbrg
	{
		avg[0] = pAvg[2];
		avg[1] = pAvg[3];
		avg[2] = pAvg[0];
		avg[3] = pAvg[1];
	}
	else if(bayerOrder == BAYER_B_FIRST) // bggr
	{
		avg[0] = pAvg[3];
		avg[1] = pAvg[2];
		avg[2] = pAvg[1];
		avg[3] = pAvg[0];
	}

	for(int i = 0; i < 4; i++)
	{
		out[i] = (unsigned short)(avg[i] - (double)pedestal + 0.5f); // + 0.5f is for round()

        /*
        ALOGE("%s[%d]:[MotFactory] out[%d] : %d = avg[%d] : %f  pedestal(%d)",
            __FUNCTION__,
            __LINE__,
            i, out[i],
            i, avg[i],
            pedestal);
        */
	}

	// Memory release
	if(p3Img)
	{
		for(c = 0; c < 4; c++)
		{
			for(i = 0; i < h1; i++)
			{
				delete[] p3Img[c][i];
			}
			delete[] p3Img[c];
		}
		delete[] p3Img;
	}

	if(p2Img)
	{
		for(i = 0; i < h; i++)
		{
			delete[] p2Img[i];
		}
		delete[] p2Img;
	}

	return SUCCESS;
}
