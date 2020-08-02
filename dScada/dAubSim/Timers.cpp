#include "stdafx.h"

TIMER *sys_timers;
static CRITICAL_SECTION CS_TimerList; 

/*------------------------------------------------------------------*/
/*  Funkcije za rad sa tajmerima                                    */
/*------------------------------------------------------------------*/

void OpenTimerList( int update_period )
{
    InitializeCriticalSection( &CS_TimerList );
    sys_timer_period = update_period;
}

void CloseTimerList( void )
{
    // obrisi prvo sve aktivne timere
    if( sys_timers )
    {
        TIMER *tp, *prev;
        EnterCriticalSection( &CS_TimerList );
        // idi od kraja i brisi redom
        tp = sys_timers;                          // pocetak lanca (prvi element)
        while( tp->next ) tp = tp->next;          // nadji zadnji
        prev = tp->prev;

        while( tp )
        {
            prev = tp->prev;
            free( tp );
            tp = prev;
        }
        LeaveCriticalSection( &CS_TimerList );
    }
    // obrisi kriticnu sekciju
    DeleteCriticalSection( &CS_TimerList );
}

TIMER* GetTimer( char *info )
{
    TIMER *tp, *novi;

    novi = (TIMER *)calloc( 1, sizeof(TIMER) );  // prev i next = NULL 
    strcpy_s( novi->info, INFO_TIMER, info );

    EnterCriticalSection( &CS_TimerList );
    if( !sys_timers )            
        sys_timers = novi;                        // prvi timer u lancu 
    else
    {
        tp = sys_timers;                          // pocetak lanca (prvi element)
        while( tp->next ) tp = tp->next;          // nadji zadnji
        tp->next = novi;
        novi->prev = tp;
    }
    LeaveCriticalSection( &CS_TimerList );

    return novi;
}

void DelTimer( TIMER *tp )
{
    EnterCriticalSection( &CS_TimerList );
    if( tp->prev == NULL )                       // prvi u lancu 
        sys_timers = tp->next;
    else
        tp->prev->next = tp->next;

    if(tp->next)                                 // u oba slucaja 
        tp->next->prev = tp->prev;
    LeaveCriticalSection( &CS_TimerList );
    free( tp );
}

void StartTimer( TIMER *timer, int T, int ponder )
{
    timer->time_set = timer->value = T * ponder;
    timer->value -= sys_timer_period;            // zbog prvog kasnjenja
    timer->time_left = timer->value;
    timer->time_ponder = ponder;
    timer->susp  = 0;
    timer->exp   = 0;
    timer->act   = 1;
}

void RestartTimer( TIMER *timer )
{
    timer->time_left = timer->value = timer->time_set - sys_timer_period;
    timer->susp  = 0;
    timer->exp   = 0;
    timer->act   = 1;
}

void StopTimer( TIMER *timer )
{
    timer->act = 0;
    timer->exp = 1;
}

void SuspendTimer( TIMER *timer )
{
    timer->susp = 1;
}

void ResumeTimer( TIMER *timer )
{
    timer->susp = 0;
}

int TimeLeftTimer( TIMER *timer, int ponder )
{
    if( ponder < 1 )
        ponder = timer->time_ponder;
    return (timer->time_left / ponder);
}

void UpdateTimers( void )
{
    EnterCriticalSection( &CS_TimerList );
    TIMER *timer = sys_timers;
    while( timer )
    {
        if( timer->act && !timer->susp )
        {
            if( timer->value > 0 )
            {
                timer->value -= sys_timer_period;
                timer->time_left = timer->value;
            }
            else
            {
                timer->time_left = 0;
                timer->exp = 1;
                timer->act = 0;
            }
        }
        timer = timer->next;
    }
    LeaveCriticalSection( &CS_TimerList );
}
