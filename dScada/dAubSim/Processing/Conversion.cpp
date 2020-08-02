/**
*  @file  Conversion.h
*
*  @brief Function implementation for conversions between physical and
*         engineering units.
*
*/

#include "StdAfx.h"

float LinearConversion( float data, unsigned int minRaw, unsigned int maxRaw, float minF, float maxF, int option )
{
    double Acc = 0;

    if (option==0)
    {
        Acc = (double)((data-minF)/(maxF-minF)) ;
        Acc = (double)(Acc*(float)(maxRaw-minRaw)) ;
        Acc = (float)minRaw+Acc ;

        if (Acc>maxRaw)
        {
            Acc = maxRaw;
        }

        if (Acc<minRaw)
        {
            Acc = minRaw;
        }
    }
    else
    {
        Acc = (double)((float)(data-minRaw)/(maxRaw-minRaw));
        Acc = (double)(Acc*(maxF-minF)) ;
        Acc = (float)minF+Acc ;

        if (Acc>maxF)
        {
            Acc=maxF;
        }
        
        if (Acc<minF)
        {
            Acc=minF;
        }
    }
    return ((float)Acc) ;
}


// int version of pow function
int pow_i( int base, int exponent )
{
    int ret = 1;
    for( int i = 0; i<exponent; i++ )
    {
        ret *= base;
    }
    return ret;
}

