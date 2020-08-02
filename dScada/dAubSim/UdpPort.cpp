#include "stdafx.h"

extern TIMER *tmCOM;

static SOCKET sockfd[MAX_PORTS];

/*----------------------------------------------------*/
/* Otvara UDP klijent/serevr port.                    */
/*----------------------------------------------------*/
int OpenUdpPort( CHANET *chp )
{
   /*---------------------------------*/
   /* Stvaranje uticnice              */
   /* IP familija protokola (PF_INET) */
   /* UDP (SOCK_DGRAM)                */
   /* IPPROTO_UDP                     */
   /*---------------------------------*/
   sockfd[chp->fix.port] = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP );
   if( INVALID_SOCKET == sockfd[chp->fix.port] )
   {
      PutLogMessage( "Create socket problem! Error code: %d", WSAGetLastError());
      return NOK;
   }

   if( chp->fix.comm.ip.server )
   {
      SOCKADDR_IN server;   
      // Fill in structure fields for binding to local host
	   memset(&server, 0, sizeof(server) );
      server.sin_family      = AF_INET;
      // identifikacija lokalne masine, priprema za bind
      server.sin_addr.s_addr = htonl( INADDR_ANY );
      server.sin_port        = htons( chp->fix.comm.ip.locport );
      // Bind it
      if( bind(sockfd[chp->fix.port], (sockaddr*)&server, sizeof(server)) )
         return NOK;
   }

   // Set the socket I/O mode: blocking mode
   // If iMode = 0/1, blocking /non-blocking; 
   int iMode = 1;
   if( ioctlsocket( sockfd[chp->fix.port], FIONBIO, (u_long FAR*)&iMode ) )
   {
      PutLogMessage( "Set socket IO mode problem! Error code: %d", WSAGetLastError());
   }

   return OK;
}

/*------------------*/
/* Zatvara UDP port */
/*------------------*/
int CloseUdpPort( CHANET *chp )
{
   closesocket( sockfd[chp->fix.port] );
   return OK;
}
/*--------------------------------------------------------*/
/* Salje porku preko UDP protokola iz IORB-a              */
/*--------------------------------------------------------*/
int putUdpMsg( IORB *iop )
{
   CHANET *chp = rtdb.chanet + iop->chn;
   RTUT *rtup = rtdb.rtut + iop->rtu;

   struct hostent *hp;
   SOCKADDR_IN txAddr;
   memset( &txAddr, 0, sizeof(txAddr) );
   txAddr.sin_family = AF_INET;
   // odredi ip adresu odredista preko opisa RTU uredjaja
   if( (hp = gethostbyname(rtup->fix.hname))==0 )
   {
      PutLogMessage( "Pogresan ili nepoznat host! Kod greske: %d", WSAGetLastError());
      return ERR_SND;
   }
   /*------------------------------------------------------------------*/
   /* Kopiramo IP adresu u sockaddr koji se vec nalazi                 */
   /* u mreznom redosledu okteta (network byte order)                  */
   /*------------------------------------------------------------------*/
   memcpy( &txAddr.sin_addr.s_addr, hp->h_addr, hp->h_length );
   // i dodajemo port
   txAddr.sin_port = htons( rtup->fix.hport );
   /*------------------------*/
   /* Saljemo poruku serveru */
   /*------------------------*/
   iop->no_snd = sendto( sockfd[chp->fix.port], (char*) iop->sbuf, iop->slen, 0, (SOCKADDR*) &txAddr, sizeof(txAddr) );
   if( iop->no_snd != iop->slen )
      return ERR_SND;
   else
      return OK;
}

/*------------------------------------------------------------*/
// Prima poruku preko UDP protokola i kopira sadrzaj u  IORB  */
/*------------------------------------------------------------*/
int getUdpMsg( IORB *iop )
{
   char inBuffer[512];
   SOCKADDR_IN rxAddr;
   short cli_port;
   int rcnt, alen = sizeof(rxAddr);
   u_long bcnt;

   CHANET *chp = rtdb.chanet + iop->chn;
   
   // pokreni timer na prijem
   if( iop->io_request != RECV )
      StartTimer( tmCOM, chp->fix.timout, SEC );
   else
      StartTimer( tmCOM, 10, MSEC );

   do
   {
      // proveri da li je sta stiglo i ucitaj
      ioctlsocket( sockfd[chp->fix.port], FIONREAD, (u_long FAR*)&bcnt ); 
      rcnt = 0;
      if( bcnt > 0 )
      {
         memset( &rxAddr, 0, alen );
         rcnt = recvfrom( sockfd[chp->fix.port], inBuffer, sizeof(inBuffer), 0, (sockaddr*)&rxAddr, &alen );
         cli_port = ntohs (rxAddr.sin_port );
      }
      if( rcnt > 0 )
      {
         for( int i=0; i < rcnt; i++ )
         {
            if( RTU_protokoli[chp->fix.protocol]->get_char(inBuffer[i], iop) )
            { 
               StopTimer( tmCOM );          // zavrsi prijem
               return iop->sts;
            }
         }
      }
      else
      {
         Sleep( 2 );         // da malo uspori petlju
      }
   }
   while( !tmCOM->exp );
   
   return ERR_TIM;            // istekao timout
}
