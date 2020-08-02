#include "stdafx.h"
#include <share.h>

static FILE* efh = NULL;         // Event File Handler
static HANDLE event_mutex;
static char EvPath[MAX_PATH];

void RepPutEvent( int whereto, char *emsg );

void FireEvent( int whereto, char *emsg )
{
   // ispisi lokalno
   lputs( emsg+7 );

   // posalji klijentu i zapisi u fajl
   strcat( emsg, "\n" );
   SendToPipe( emsg );

   if( whereto != RCV_EVNT )
   {
      // zapisi na disk
      if( efh == NULL )
      {
         sprintf( EvPath, "%s\\%s", StnCfg.StaFolder, "EventFile.txt" );
         //if( !(efh = fopen( EvPath, "wt")) )
         if( !(efh = _fsopen( EvPath, "at", _SH_DENYNO)) )
            return;
      }
      // zapisi event
      fwrite( emsg+7, strlen(emsg)-7, 1, efh );
      fflush( efh );
   }
}


// salje dogadjaj 
void put_event( int whereto, PVID *ppvid, char *format, ... )
{
   char msg[256];
   va_list argptr;

   // Pripremi poruku koja se salje klijentu
   if( ppvid )
   {
      WORD status = get_status( ppvid, OPR_PART );
      // odustani ako je ispis dogadjaja zabranjen
      if( get_bit(status, OPR_EVENT_INH) )
         return;
      // ipak nije zabranjen, ispisi
      if( ppvid->type == PVT_RTU )
         sprintf(msg , "event; %s %03d %-8s ", ctime2str(), TimeMS, rtdb.rtut[ppvid->rtu].fix.name );
      else
         sprintf(msg , "event; %s %03d %s:%-8s ", ctime2str(), TimeMS, rtdb.rtut[ppvid->rtu].fix.name, get_name(ppvid) );
   }
   else
      sprintf( msg, "event; %s %03d ",  ctime2str(), TimeMS );

   va_start( argptr, format );
   vsprintf( msg+strlen(msg), format, argptr );
   va_end( argptr );

   FireEvent( whereto, msg );

}

// default funkcija koja salje svima u regularnoj vezi
void PutEvent( PVID *ppvid, char *format, ... )
{
   va_list argptr;
   char msg[256];

   va_start( argptr, format );
   vsprintf( msg, format, argptr );
   va_end( argptr );

   put_event( REP_EVNT, ppvid, msg );
}

// salje tandemu i adresiranoj stanici
void PutStaEvent( int dsId, PVID *ppvid, char *format, ... )
{
   va_list argptr;
   char msg[256];

   va_start( argptr, format );
   vsprintf( msg, format, argptr );
   va_end( argptr );

   if( dsId >= 0 )
   {
      put_event( dsId, ppvid, msg );
   }
}


// samo na lokalnoj masini
void PutLocEvent( PVID *ppvid, char *format, ... )
{
   va_list argptr;
   char msg[256];

   va_start( argptr, format );
   vsprintf( msg, format, argptr );
   va_end( argptr );

   put_event( LOC_EVNT, ppvid, msg );
}
