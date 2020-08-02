// stdafx.cpp : source file that includes just the standard includes
//	DemoAUB.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#include <math.h>

int sys_timer_period = 10;       // ms
int timeS, TimeMS;               // Trenutno vreme h:min:sec;msec

// Definicija semafora
HANDLE periodicProcessingTimer, smCOM, smACQ, LogerMTX;

STATION_CFG StnCfg;

/*********************************************************/
void refresh_WD( void )
{
}

void disable_WD( void )
{
}

void enable_WD( void )
{
}

// promena prioriteta thread-a
int SetPriority( int priority )
{
    HANDLE th = GetCurrentThread();
    int old_priority = GetThreadPriority( th );
    SetThreadPriority( th, priority );
    return old_priority;
}

// buffer split - pomocna funkcija za raspakivanje primljene poruke
void bsplit( void *dst, BYTE **src, int size )
{
    memcpy( dst, *src, size );
    *src += size;
}
