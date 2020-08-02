/**
*  @file  AubTime.cpp
*
*  @brief Function implementation for application specific support.
*
*/

#include "StdAfx.h"
#include <time.h>

struct tm Now;
long get_seconds()
{
    time_t Time1970 = time(NULL);
    _localtime64_s(&Now, &Time1970);
    return (long)Time1970;
}

char* ctime2str()
{
    static char timebuf[30];
    _strtime_s(timebuf, 30);
    return timebuf;
}

char* time2str(long timestamp)
{
    static char timebuf[30];
    time_t Time1970 = (time_t)timestamp;
    struct tm *ltm = _localtime64(&Time1970);
    sprintf( timebuf, "%02d:%02d:%02d %02d.%02d.%04d", ltm->tm_hour, ltm->tm_min, ltm->tm_sec, ltm->tm_mday, ltm->tm_mon+1, 1900+ltm->tm_year );
    return timebuf;
}

char* cdate2str()
{
    static char datebuf[30];
    _strdate_s(datebuf, 30);
    return datebuf;
}
