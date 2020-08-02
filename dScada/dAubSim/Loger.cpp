#include "stdafx.h"

static char logmsg[1000];

void PutLogMessage( char *format, ... )
{
   FILE* fh;
   va_list argptr;

   sprintf( logmsg, "<%s %s> ",  cdate2str(), ctime2str() );

   va_start( argptr, format );
   vsprintf( logmsg+strlen(logmsg), format, argptr );
   va_end( argptr );

   strcat( logmsg, "\n" );

   WaitForSingleObject( LogerMTX, INFINITE );
   if( fh=fopen( "LogFile.txt", "at" ) )
   {
      fwrite( logmsg, strlen(logmsg), 1, fh );
      fclose( fh );
   }
   ReleaseMutex( LogerMTX );
}


void ShowNetError( char *file, int line )
{
   char *f;
   if( f=strrchr(file, '\\') )
      f++;
   else 
      f = file;
   PutLogMessage( "%s / %d: NetError %d !", f, line, WSAGetLastError() );
}
