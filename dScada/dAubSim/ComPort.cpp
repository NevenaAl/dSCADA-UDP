#include "stdafx.h"

//**********************************************************************************
// direktorijum podrzanih tipova komunikacionih portova

COMPORT ports[PORTS_NUM] = 
{
    { UART, OpenUartPort, NULL, NULL, CloseUartPort, putUartMsg, getUartMsg },
    { UDP,  OpenUdpPort, NULL, NULL, CloseUdpPort, putUdpMsg, getUdpMsg },
    { TCP,  OpenTcpPort, ConnectTcpPort, DisconnectTcpPort, CloseTcpPort, putTcpMsg, getTcpMsg }
};

TIMER *tmCOM;

//**********************************************************************************

// usluzna funkcija za ispis Tx/Rx poruka
void print_msg( int row, IORB *iop )
{
    char buff[1024], *buffptr;
    BYTE *msg;
    int len;

    if( row == 1 )
    {                          // send
        msg = iop->sbuf;
        len = iop->no_snd;
    }
    else
    {                          // recv
        msg = iop->rbuf;
        len = iop->no_rcv;
    }

    // ispisi celu poruku
    sprintf( buff, "[%s] %s %3d:", ctime2str(), err_str[iop->sts], len );
    buffptr = buff + strlen(buff);
    for( int i=0; i <len; i++ )
    {
        sprintf( buffptr, " %02X", msg[i] );
        buffptr += 3;
    }

    int protokol = rtdb.chanet[iop->chn].fix.protocol;
    // if( iop->chn, rtu, protokol == xx )        // da filtriram zapis u log
    if( iop->chn == 22 )        // da filtriram zapis u log
    {
        // zapis komunikacije u LogFile uz preskakanje vremena slanja/prijema
        if( row == 1 )
            PutLogMessage( "Tx %s %s", RTU_protokoli[protokol]->get_msg_name(iop->request_type), buff+11 );       // po slanju poruke
        else
            PutLogMessage( "Rx %s %s", RTU_protokoli[protokol]->get_msg_name(iop->reply_type), buff+11 );         // po prijemu poruke
    }

    // ispis na ekran sa vremenom slanja/prijema
    if( strlen(buff) > 103 )
        memcpy( buff+103, ">>", 3 );
    wputs( row, buff );
}

/*------------------------------------------------------------------*/
/*  Funkcije za rad sa komunikacionim portom                              */
/*------------------------------------------------------------------*/
int OpenComPort( CHANET *chp )
{
    // namesti pport i proveri ima li sve f-je
    COMPORT *pport = ports + chp->fix.type;
    if( !pport )
        return NOK;
    else if( !pport->open_com_port || !pport->close_com_port  || !pport->put_msg  || !pport->get_msg )
        return NOK;

    // otvori com port
    return pport->open_com_port( chp );
}

int ConnectComPort( int rtu )
{
    CHANET *chp = rtdb.chanet + rtdb.rtut[rtu].fix.channel;
    COMPORT *pport = ports + chp->fix.type;
    if( pport->connect_com_port )
        return pport->connect_com_port( rtu );
    // ako nema connect funkcije, port je "povezan"
    rtdb.rtut[rtu].var.connected = 1;
    return OK;
}

void DisconnectComPort( int rtu )
{
    CHANET *chp = rtdb.chanet + rtdb.rtut[rtu].fix.channel;
    COMPORT *pport = ports + chp->fix.type;
    if( pport->connect_com_port )
        pport->disconnect_com_port( rtu );
    else
        rtdb.rtut[rtu].var.connected = 0;   // ako nema connect funkcije, port je "odvezan"
}


void CloseComPort( CHANET *chp )
{
    COMPORT *pport = ports + chp->fix.type;
    if( pport )
        pport->close_com_port(chp);
}

int putMsg( IORB *iop )
{
    CHANET *chp = rtdb.chanet + iop->chn;
    COMPORT *pport = ports + chp->fix.type;
    iop->no_snd = 0;   
    iop->sts = pport->put_msg(iop);
    print_msg( 1, iop );
    return iop->sts;
}

// prima na duzinu (rlen>0) 
int getMsg( IORB *iop )
{
    CHANET *chp = rtdb.chanet + iop->chn;
    COMPORT *pport = ports + chp->fix.type;
    iop->no_rcv = 0;
    iop->sts = pport->get_msg( iop );
    // ne ispisuj poruku ako je RECV i timout
    if( (iop->io_request != RECV) || (iop->sts != ERR_TIM) )
    {
        print_msg( 2, iop );
    }
    return iop->sts;
}

static bool OpenComPorts_Flag = false;
void OpenComPorts( void )
{
    if( !OpenComPorts_Flag )
    {
        // napravi timera za rcv_timout
        tmCOM = GetTimer( "tmCOM" );

        for( int i=0; i < rtdb.no_channel; i++ )
        {
            CHANET *chp = rtdb.chanet + i;
            if( OpenComPort(chp) )
                _Abort();
        }
        OpenComPorts_Flag = true;
    }
}

void CloseComPorts( void )
{
    if( OpenComPorts_Flag )
    {
        for( int i=0; i <rtdb.no_channel; i++ )
        {
            CloseComPort( rtdb.chanet + i );
        }
        DelTimer( tmCOM );
        OpenComPorts_Flag = false;
    }
}

