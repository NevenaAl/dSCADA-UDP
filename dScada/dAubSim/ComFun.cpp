
#include "stdafx.h"


static IOBUF iobuf;
char err_str[7][20] = { " OK ", "eSnd", "eRcv", "eTim", "eCrc", "eLen", "eCon" };

void OpenIORbuf( void )
{
    iobuf.count = iobuf.head = iobuf.tail = 0;
    iobuf.mutex = CreateMutex( NULL, FALSE, NULL );
}

/*---------------------------*/
/* Pribavlja IORB iz buffera */
/*---------------------------*/
IORB *GetIORbuf( int kako )   
{
    short i;
    IORB *iop;                 // IO requests block pointer

    while( iobuf.count >= MBUF_LEN )
        Sleep( 10 );

    WaitForSingleObject( iobuf.mutex, INFINITE );
    if( kako == END_LIST )
    {
        i = iobuf.head;
        iobuf.head = (i + 1) % MBUF_LEN;
    }
    else
    {
        if( iobuf.tail )
            iobuf.tail --;
        else
            iobuf.tail = MBUF_LEN - 1;
        i = iobuf.tail;
    }
    iop = iobuf.iorb + i;

    return( iop );
}

/*-------------------------------*/
/* Inicira izvrsenje IO zahteva */
/*-------------------------------*/
void PutIORbuf( IORB *iop )     
{
    iobuf.count++;
    ReleaseMutex( iobuf.mutex );
    ReleaseSemaphore( smCOM, 1, NULL );          // rasporedi ComTask
}

/*----------------------*/
/* Brise izdate tpor[k] */
/*----------------------*/
void ClearIORbuf( void )       
{
    WaitForSingleObject( iobuf.mutex, INFINITE );
    iobuf.count = iobuf.head = iobuf.tail = 0;
    ReleaseMutex( iobuf.mutex );
}

// preuziva IORB iz buffera i kopira na zadatu adresu
int TakeIORbuf( IORB *iop )
{
    if( !iobuf.count )                                 // ovo ne sme da se desi
        _Abort();

    WaitForSingleObject( iobuf.mutex, INFINITE );

    short t = iobuf.tail;                           // io buff pointer
    // iskopiraj zahtev
    memcpy( iop, iobuf.iorb + t, sizeof(IORB) );

    // proveri da li dati zahtev moze da se izvrsi (protocol specific)
    int protokol = rtdb.chanet[iop->chn].fix.protocol;
    int ret = RTU_protokoli[protokol]->pre_release( iop );
    if( ret == OK )
    {
        // azuriraj tail i smanji count
        iobuf.tail = (t + 1) % MBUF_LEN;
        iobuf.count --;
    }

    ReleaseMutex( iobuf.mutex );
    return ret;
}

void on_rtu_ok( RTUT *rtup )
{
    // ako je ovo bio prvi uspesan prolaz, izvesti klijenta
    if( rtup->var.first_scan )
    {
        rtup->var.first_scan = 0;
        rtup->chg = 1;
    }

    if( !rtup->var.val )
    {
        // azuriraj brojac kom. gresaka
        if( rtup->var.err )
        {
            rtup->var.err--;
        }
        else
        {
            rtup->var.val = 1;
            rtup->chg = 1;
            PutEvent( NULL, "RTU %s in scan!", rtup->fix.name );
        }
        wputs( 3, "%s: err = %d, val = %d", rtup->fix.name, rtup->var.err, rtup->var.val );
    }

}

void on_rtu_err( RTUT *rtup )
{
    if( rtup->var.val )
    {
        rtup->var.err++;
        if ( rtup->var.err >= 7 )
        {
            rtup->var.val = 0;
            rtup->chg = 1;
            PutEvent( NULL, "RTU %s out of scan!", rtup->fix.name );
        }
        wputs( 3, "%s: err = %d, val = %d", rtup->fix.name, rtup->var.err, rtup->var.val );
    }
}


/************************************************************************/
/* Provera/prihvat primljenih podataka i poziv funkcije za dalju obradu */
/************************************************************************/
void ProcessRxMsg( IORB *iop )
{
    int protokol = rtdb.chanet[iop->chn].fix.protocol;
    RTU_protokoli[protokol]->proccess_msg( iop );
}

void check_sts( IORB *iop )
{
    // ovo je ocekivan dogadjaj (nema unsolicited odgovora)
    if( (iop->io_request == RECV) && (iop->sts == ERR_TIM) )
        return;

    // odredi status RTU-a
    int protokol = rtdb.chanet[iop->chn].fix.protocol;
    if( iop->sts == OK )
    {
        RTU_protokoli[protokol]->on_msg_ok( iop );
    }
    else
    {
        RTUT *rtup = rtdb.rtut + iop->rtu;
        RTU_protokoli[protokol]->on_msg_err( iop );
        // ispisi gresku
        PutLocEvent( NULL, "%s %s %s", rtup->fix.name, err_str[iop->sts], RTU_protokoli[protokol]->get_msg_name(iop->request_type) );
    }
}

void do_execute( IORB *iop )
{
    RTUT *rtup = rtdb.rtut + iop->rtu;
    // imamo li konekciju na remote stranu?
    if( !rtup->var.connected )              // samo za TCP moguce
    {
        ConnectComPort( iop->rtu );
    }

    if( rtup->var.connected )               // imamo konekciju
    {
        // konacno izvrsi ono sto je zahtevano
        for( int i=0; i< iop->maxrep; i++ )
        {
            iop->sts = OK;                            // pocetak je OK
            switch( iop->io_request )
            {
            case SEND_RECV:
                if( putMsg(iop) == OK )
                {
                    do
                    {
                        if( getMsg(iop) == OK )
                            ProcessRxMsg( iop );
                        else
                            iop->request_on = 0;
                        check_sts( iop );
                    } 
                    while( iop->request_on );
                }
                break;
            case RECV_SEND:
                if( getMsg(iop) == OK )
                {
                    ProcessRxMsg( iop );
                    putMsg( iop );
                }
                break;
            case SEND:
                putMsg( iop );
                break;
            case RECV:
                while( getMsg(iop) == OK )
                {
                    ProcessRxMsg( iop );
                    check_sts( iop );
                }
                break;
            case DISCONNECT:
                DisconnectComPort( iop->rtu );
                break;
            }
            if( iop->sts == OK )
                break;               // Uspesno izvrsen IO zahtev
        }
    }
    else
        iop->sts = ERR_CONN;

    check_sts( iop );

    return;
}


/*----------------------------------------*/
/*    Izvrsava zahtev sa iobuf[tail]      */
/*----------------------------------------*/
void ExecuteIOReq( void )
{
    IORB iorb;                    
    // ako ima izdatih zahteva, preuzmi IORB koji treba izvrsiti
    if( iobuf.count && (TakeIORbuf(&iorb) == OK)  )    // ima li izvrsivih zahteva ?
    {
        //DWORD timestamp = GetTickCount();
        do_execute( &iorb );
    }
    else
    {
        // ako nema sta da se izvrsi, odradi prijem i/ili vremenske kontrole
        for( int i=0; i < rtdb.no_rtu; i++ )
        {
            RTUT *rtup = rtdb.rtut + i;
            memset( &iorb, 0, sizeof(IORB) );
            // namesti rtu, kanal i broj zanavljanja
            iorb.rtu = i;
            int protokol = rtdb.chanet[rtup->fix.channel].fix.protocol;
            if( RTU_protokoli[protokol]->pre_release(&iorb) )
            {
                do_execute( &iorb );
            }
        }
    }
    return;
}

void Send( MSGID *msg, BYTE *msgbuf, int mlen, int kako )
{
    IORB iorb;
    int ret;

    // set all zero za pocetak
    memset( &iorb, 0, sizeof(IORB) );

    // iskopiraj sve iz msg 
    iorb.request_type = msg->type;
    iorb.rtu = msg->rtu;
    iorb.id = msg->id;
    if( iorb.rtu < 0 )
        iorb.chn = msg->chn;
    else
        iorb.chn = rtdb.rtut[iorb.rtu].fix.channel;

    int protokol = rtdb.chanet[iorb.chn].fix.protocol;

    if( msg->rtu >= 0 )
    {                    // normalno slanje poruke ka RTU stanici
        ret = RTU_protokoli[protokol]->send_msg( &iorb, msgbuf, mlen );
    }
    else
    {                    // broadcast slanje poruke na kanal
        ret = RTU_protokoli[protokol]->send_bcst_msg( &iorb, msgbuf, mlen );
    }

    if( ret == 0 )
    {
        IORB *iop = GetIORbuf( kako );
        memcpy( iop, &iorb, sizeof(IORB) );
        PutIORbuf( iop );
    }
}

void Receive( MSGID *msg, int kako )
{
    IORB iorb;

    iorb.chn = rtdb.rtut[msg->rtu].fix.channel;
    iorb.rtu = msg->rtu;
    iorb.id = msg->id;

    int protokol = rtdb.chanet[iorb.chn].fix.protocol;

    int ret = RTU_protokoli[protokol]->recv_msg( &iorb, msg );

    if( ret == 0 )
    {
        IORB *iop = GetIORbuf( kako );
        memcpy( iop, &iorb, sizeof(IORB) );
        PutIORbuf( iop );
    }
}

void SendRawMsg( MSGID *msg, BYTE *msgbuf, int mlen, int kako )
{
    IORB iorb;
    if( mlen > 0 )
    {
        // iskopiraj i popuni osnovne podatke
        iorb.io_request = SEND;
        iorb.chn = rtdb.rtut[msg->rtu].fix.channel;
        iorb.rtu = msg->rtu;
        iorb.id = msg->id;

        iorb.maxrep = 1;

        iorb.request_type = msg->type;
        // iskopiraj poruku za slanje
        memcpy( iorb.sbuf, msgbuf, mlen );
        iorb.slen = mlen;

        iorb.reply_type = -1;
        iorb.rlen = 0;

        IORB *iop = GetIORbuf( kako );
        memcpy( iop, &iorb, sizeof(IORB) );
        PutIORbuf( iop );
    }
}

int get_io_count()
{
    return iobuf.count;
}


