/**
*  @file  Conversion.h
*
*  @brief Function prototypes for conversions between physical and
*         engineering units.
*
*/

#ifndef CONVERSION_H
#define CONVERSION_H

/**
* @brief     Universal function for linear conversion from physical value
*            to raw binary format and vice versa.
*
* @param     data   [in] -  Input signal value in physical units (Option = 0)
*                          or raw (Option = 1).
*            minRaw [in] - Minimum raw value.
*            maxRaw [in] - Maximum raw value.
*            minV   [in] - Minimum physical value.
*            maxV   [in] - Maximum physical value.
*            option [in] - Direction of  conversion, when 0 conversion is done
*                          from physical to raw and otherwise when 1.
* @return    Calculated value.
*
*/
float LinearConversion(float data, unsigned int minRaw, unsigned int maxRaw, float minF,float maxF,int option);  

/**
* @brief     Calculates base raised to the power of y.
*
* @param     base [in]     - Base
*            exponent [in] - Exponent
*
* @return    base^exponent
*
*/
int pow_i( int base, int exponent );

#endif