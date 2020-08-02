#include "stdafx.h"

extern TIMER *tmCOM;

static HANDLE portH[MAX_PORTS];

/*------------------------------------------------------------------*/
/* Funkcija za otvaranje serijskog porta                            */
/*------------------------------------------------------------------*/
int OpenUartPort( CHANET *chp )
{
   COMMCONFIG Config;
   COMMTIMEOUTS Times = {MAXDWORD, 0, 0, 0, 0};
   HANDLE phnd;

   // inicijalizuj serijski port 
   char portname[20];
   sprintf( portname, "COM%d:", chp->fix.port+1 );
   phnd = CreateFile( portname, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
   if( phnd == INVALID_HANDLE_VALUE )
      return NOK;

   if( SetupComm(phnd, 512, 512) != TRUE )
   {
	   DWORD Error = GetLastError();
      return NOK;
   }

   memset( &Config, 0, sizeof(COMMCONFIG) );
   memset( &Config.dcb, 0, sizeof(DCB) );
   Config.dwSize = sizeof(COMMCONFIG) + sizeof(DCB);
   Config.dcb.DCBlength = sizeof(DCB);

   Config.dcb.fBinary   =  TRUE;
   Config.dcb.fParity   =  TRUE;
   Config.dcb.BaudRate  =  chp->fix.comm.uart.baud;
   Config.dcb.ByteSize  =  chp->fix.comm.uart.lenght;
   Config.dcb.Parity    =  chp->fix.comm.uart.parity;
   Config.dcb.StopBits  =  chp->fix.comm.uart.stop_bits;

   SetCommState( phnd, &Config.dcb );
   PurgeComm( phnd, PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR );   
   SetCommTimeouts( phnd, &Times );

   if( !SetCommMask(phnd, 0) )
      return NOK;

   // sacuvaj handle porta za kasnije
   portH[chp->fix.port] = phnd;

   return OK;
}

/*------------------------------------------------------------------*/
/* Funkcija za zatvaranje serijskog porta                           */
/*------------------------------------------------------------------*/
int CloseUartPort( CHANET *chp )
{
   PurgeComm( portH[chp->fix.port], PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR );
	CloseHandle( portH[chp->fix.port] );
   return OK;
}

/*------------------------------------------------------------------*/
/* Slanje IORB poruke preko serijskog porta                         */
/*------------------------------------------------------------------*/
int putUartMsg( IORB *iop )
{
   CHANET *chp = rtdb.chanet + iop->chn;
   DWORD sent;
   WriteFile( portH[chp->fix.port], iop->sbuf, iop->slen, &sent, NULL );
   iop->no_snd = (int)sent;
   if( iop->slen != iop->no_snd )
      return ERR_SND;
   else
      return OK;
}
/*------------------------------------------------------------------*/
/* prima na duzinu (rlen>0) ili na specijalni EOM znak (duz <= 0)   */
/*------------------------------------------------------------------*/
int getUartMsg( IORB *iop )
{
   BYTE inBuffer[512];
   DWORD rcnt;
   
   CHANET *chp = rtdb.chanet + iop->chn;

   // pokreni timer na prijem
   if( iop->io_request != RECV )
      StartTimer( tmCOM, chp->fix.timout, SEC );
   else
      StartTimer( tmCOM, 20, MSEC );
   do
   {
      // asinhroni poziv, vrati broj ucitanih ako ih ima
      ReadFile( portH[chp->fix.port], inBuffer, sizeof(inBuffer), &rcnt, NULL );
      if( rcnt > 0 )
      {
         for( int i=0; i < (int)rcnt; i++ )
         {
            if( RTU_protokoli[chp->fix.protocol]->get_char( inBuffer[i], iop) )
            {
               StopTimer( tmCOM );       // zavrsi prijem
               return iop->sts;
            }
         }
      }
      else
      {
         Sleep( 10 );      // ako nije bilo nicega, ohani malo
      }
   }
   while( !tmCOM->exp ); 
   
   return ERR_TIM;               // istekao timout
}
