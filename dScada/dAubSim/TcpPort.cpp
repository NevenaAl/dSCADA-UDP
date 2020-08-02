#include "stdafx.h"

extern TIMER *tmCOM;

typedef struct
{
   SOCKET socklsn;
   SOCKET sockfd;
   SOCKADDR_IN server;
} CONNECTION;

static CONNECTION Connection[RTU_CHN*MAX_PORTS];

/*----------------------------------------------------*/
/* Otvara TCP Client ili Server port                  */
/*----------------------------------------------------*/
int OpenTcpPort( CHANET *chp )
{
   if( chp->fix.comm.ip.server )
   {                                                                       // TCP server
      // napravi i stavi u listen stanje sockete za konekciju
      //    server name = localhost (INADDR_ANY)
      //    server port = rtdb.chanet->tcport + komunikaciona adresa RTUa
      for( int i=0; i < chp->fix.rtu_n; i++ )
      {
         int rtu = chp->fix.rtu_lst[i];
         CONNECTION *cnp = Connection + rtu;
         // Stvaramo listen uticnicu 
         cnp->socklsn = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
         if( SOCKET_ERROR == cnp->socklsn )
         {
            PutLogMessage( "Create socket problem! Error code: %d", WSAGetLastError());
            return NOK;
         }
         // Set the socket I/O mode: iMode = 0/1, blocking /non-blocking; 
         int iMode = 1;
         if( ioctlsocket( cnp->socklsn, FIONBIO, (u_long FAR*)&iMode ) )
         {
            PutLogMessage( "Set socket IO mode problem! Error code: %d", WSAGetLastError());
            closesocket( cnp->socklsn );
            return NOK;
         }
         // identifikacija lokalne masine, priprema za bind
         SOCKADDR_IN server;
	      memset(&server, 0, sizeof(server) );
         server.sin_family      = AF_INET;
         server.sin_addr.s_addr = htonl( INADDR_ANY );
         //server.sin_port        = htons( chp->fix.comm.ip.locport );
         server.sin_port        = htons( rtdb.rtut[rtu].fix.hport );

         if( bind(cnp->socklsn, (sockaddr*)&server, sizeof(server)) )
         {
            PutLogMessage( "Bind socket problem! Error code: %d", WSAGetLastError());
            closesocket( cnp->socklsn );
            return NOK;
         }
         if( listen(cnp->socklsn, 1) == SOCKET_ERROR )
         {
            PutLogMessage( "Listen socket problem! Error code: %d", WSAGetLastError());
            closesocket( cnp->socklsn );
            return NOK;
         }
      }
   }
   else
   {                                                                    // TCP client
      // pripremi konekciju na TCP servere (RTU stanice)
      // host je definisan preko RTU parametara: 
      for( int i=0; i < chp->fix.rtu_n; i++ )
      {
         int rtu = chp->fix.rtu_lst[i];
         RTUT *rtup = rtdb.rtut + rtu;
         struct hostent *hp;
         SOCKADDR_IN *serverp = &Connection[rtu].server;
         memset( serverp, 0, sizeof(SOCKADDR_IN) );
         serverp->sin_family = AF_INET;
         if( (hp = gethostbyname(rtup->fix.hname)) == NULL )
         {
            PutLogMessage( "Unknown host! Error code: %d", WSAGetLastError());
            return NOK;
         }
         memcpy( &serverp->sin_addr.s_addr, hp->h_addr, hp->h_length );
         serverp->sin_port = htons( rtup->fix.hport );
      }
   }

   return OK;
}


int ConnectTcpPort( int rtu )
{
   int iMode = 1;
   fd_set wfds;
   struct timeval timeout = { 1, 0 };  // u sec
   int ret, err;

   RTUT *rtup = rtdb.rtut + rtu;
   CHANET *chp = rtdb.chanet + rtup->fix.channel;
   CONNECTION *cnp = Connection + rtu;

   if( chp->fix.comm.ip.server )
   {                                         // TCP server
      // proveri sa select je li accept pending
      wfds.fd_count = 1;
      wfds.fd_array[0] = cnp->socklsn;
      if( (ret = select(1, &wfds, NULL, NULL, &timeout)) == 1 )
      {
         SOCKET client = accept( cnp->socklsn, NULL, NULL );
         if( client != INVALID_SOCKET )
         {
            cnp->sockfd = client;
            rtup->var.connected = 1;
            return OK;
         }
      }
   }
   else
   {
      // napravi socket
      cnp->sockfd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
      if( INVALID_SOCKET == cnp->sockfd )
      {
         PutLogMessage( "Create socket problem! Error code: %d", WSAGetLastError());
         return NOK;
      }
      // Set the socket I/O mode: iMode = 0/1, blocking /non-blocking; 
      if( ioctlsocket( cnp->sockfd, FIONBIO, (u_long FAR*)&iMode ) )
      {
         PutLogMessage( "Set socket IO mode problem! Error code: %d", WSAGetLastError());
         closesocket( cnp->sockfd );
         return NOK;
      }
      // pokusaj konekciju
      ret = connect( cnp->sockfd, (SOCKADDR*) &cnp->server, sizeof(SOCKADDR_IN) );
      if( ret == 0 )
      {
         rtup->var.connected = 1;
         return OK;     // uspela iz prve
      }
      else
      {
         // vidi da li ce za 1 sec konekcija uspeti
         if( (ret == SOCKET_ERROR) && ((err=WSAGetLastError()) == WSAEWOULDBLOCK) )
         {
            // proveri sa select je li connect zavrsen
            wfds.fd_count = 1;
            wfds.fd_array[0] = cnp->sockfd;
            if( (ret = select(1, NULL, &wfds, NULL, &timeout)) == 1 )
            {
               rtup->var.connected = 1;
               return OK;
            }
         }
         closesocket( cnp->sockfd );
      }
   }
   // nije uspelo povezivanje sa RTU
   return NOK;
}

/*------------------*/
/* Prekida TCP vezu */
/*------------------*/
void DisconnectTcpPort( int rtu )
{
   CONNECTION *cnp = Connection + rtu;
   closesocket( cnp->sockfd );
   rtdb.rtut[rtu].var.connected = 0;
}


/*------------------*/
/* Zatvara TCP port */
/*------------------*/
int CloseTcpPort( CHANET *chp )
{
   for( int i=0; i<chp->fix.rtu_n; i++ )
   {
      CONNECTION *cnp = Connection + chp->fix.rtu_lst[i];
      closesocket( cnp->sockfd );
      if( chp->fix.comm.ip.server)
         closesocket( cnp->socklsn );
   }
   return OK;
}
/*--------------------------------------------------------*/
/* Salje serveru poruku iz IORB-a preko TCP protokola     */
/*--------------------------------------------------------*/
int putTcpMsg( IORB *iop )
{
   CONNECTION *cnp = Connection + iop->rtu;
   iop->no_snd = send( cnp->sockfd, (char*) iop->sbuf, iop->slen, 0 );
   if( iop->no_snd != iop->slen )
   {
      rtdb.rtut[iop->rtu].var.connected = 0;
      closesocket( cnp->sockfd );
      return ERR_SND;
   }
   return OK;
}

/*------------------------------------------------------------*/
// Prima poruku preko TCP protokola i kopira sadrzaj u  IORB  */
/*------------------------------------------------------------*/
int getTcpMsg( IORB *iop )
{
   char inBuffer[512];
   u_long bcnt;
   int rcnt;
   
   CHANET *chp = rtdb.chanet + iop->chn;
   CONNECTION *cnp = Connection + iop->rtu;

   // pokreni timer na prijem
   if( iop->io_request != RECV )
      StartTimer( tmCOM, chp->fix.timout, SEC );
   else
      StartTimer( tmCOM, 10, MSEC );
   do
   {
      int rsts = ioctlsocket( cnp->sockfd, FIONREAD, (u_long FAR*)&bcnt );
      // proveri da li je javio neku gresku
      if( rsts == SOCKET_ERROR )
      {
         //  prekid veze
         closesocket( cnp->sockfd );
         rtdb.rtut[iop->rtu].var.connected = 0;
         StopTimer( tmCOM );
         return ERR_RCV;
      }
         
      if( bcnt > 0 )
      {
         rcnt = recv( cnp->sockfd, inBuffer, sizeof(inBuffer), 0 );
         if( rcnt <= 0 )
         {
            //  prekid veze
            closesocket( cnp->sockfd );
            rtdb.rtut[iop->rtu].var.connected = 0;
            StopTimer( tmCOM );
            return ERR_RCV;
         }
         else
         {		 
            for( int i=0; i < rcnt; i++ )
            {
               if( RTU_protokoli[chp->fix.protocol]->get_char(inBuffer[i], iop) )
               { 
                  StopTimer( tmCOM );                // zavrsi prijem
                  return iop->sts;
               }
            }
         }
      }
      else
      {
         if( !chp->fix.comm.ip.server )
         {
            Sleep( 2 );         // da malo uspori petlju
         }
         else
         {                       // TCP server
            fd_set wfds;
            int ret;
            struct timeval timeout = { 0, 50 };             // u msec
            // proveri sa select je li close pending
            wfds.fd_count = 1;
            wfds.fd_array[0] = cnp->socklsn;
            if( (ret = select(1, &wfds, NULL, NULL, &timeout)) == 1 )
            {
               closesocket( cnp->sockfd );
               rtdb.rtut[iop->rtu].var.connected = 0;
               StopTimer( tmCOM );
               return ERR_RCV;
            }
         }
      }
   }
   while( !tmCOM->exp );
   
   return ERR_TIM;            // istekao timout
}
