/**
*  @file   AubTime.h
*
*  @brief  Function prototypes for time conversion.
*
*/

#ifndef AUBTIME_H
#define AUBTIME_H

/**
 * @brief     Returns Unix time in seconds.
 * 
 * @return    Unix time in seconds.
 *
 */
long get_seconds();

/**
 * @brief     Returns current time in string format.
 * 
 * @return    Time in string format.
 *
 */
char* ctime2str();
/**
 * @brief     Converts time provided to string.
 * 
 * @param     timestamp
 *
 * @return    Time in string format.
 *
 */
char* time2str(long timestamp);

/**
 * @brief     Returns data in string format.
 * 
 * @return    Date in string format.
 *
 */
char* cdate2str();

extern struct tm Now;

#endif