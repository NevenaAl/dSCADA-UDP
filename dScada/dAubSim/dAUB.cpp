#include "StdAfx.h"

/***************************************************************
*                                                              *
*         Funkcija za inicijalizaciju RT baze podataka         *
*                                                              *
***************************************************************/

int initAUB( void )
{
    int ret;
    char CfgPath[MAX_PATH];

    // oznaci tip stanice
    StnCfg.StaType = STA_AUB;

    // Inicijalizuj listu timera
    OpenTimerList( 10 );

    // ucitaj imena cfg fajlova
    if( ReadConfigFile() )
    {
        AfxMessageBox( "Greska u citanju dConfig !" );
        return NOK;
    }

    // Konfiguracija procesa
    sprintf( CfgPath, "%s\\%s", StnCfg.SysFolder, StnCfg.AubCfg );
    if( (ret = ReadAsciiConfiguration(CfgPath)) )
    {
        CString Str;
        Str.Format( "Greska %d u citanju konfiguracije", ret );
        AfxMessageBox( Str );
        return NOK;
    }

    // set hash table
    InitHashTable();

    return OK;
}

/***************************************************************
*                                                              *
*         Funkcija za deinicijalizaciju RT baze podataka       *
*                                                              *
***************************************************************/

void deinitAUB( void )
{
    CloseTimerList();
    // vrati svu memoriju
    FreeHashTable();
    FreeConfigurationAllocations();
}

DWORD WINAPI PeriodicProcessingTask( LPVOID pData )
{
    // time in seconds
    timeS = get_seconds(); // reconsider to create it as local variable
    // Loop untill shutdown is issued.
    while(!ServerShutdown)
    {
        DWORD sret = WaitForSingleObject( periodicProcessingTimer, 200 );
        // If periodicProcessingTimer is released and server
        // is not in shutdown mode - execute periodic processing.
        if( (sret == WAIT_OBJECT_0) && !ServerShutdown )
        {
            timeS = get_seconds();        // vreme u sekundama

            ReleaseSemaphore( smACQ, 1, NULL );    // otpusta akvizicioni task

            // vremenske AUP funkcije
            // Check if commands sent are successfully executed
            CommandInspector();
            // Executed implemented control procedures
            AutoProcedure();
            // Execute industry-specific object processing
            ObjectsProcessor();
        }
    }
    return 0;
}

DWORD WINAPI CommunicationTask( LPVOID pData )
{
    OpenIORbuf();
    OpenComPorts();

    while( !ServerShutdown )
    {
        DWORD sret = WaitForSingleObject( smCOM, 100 );
        if( ServerShutdown )
            continue;

        if( (sret==WAIT_OBJECT_0) || (sret==WAIT_TIMEOUT) )
        {
            ExecuteIOReq();            // izvrsi zahtevani comm zahtev
        }
    }
    // definitivno terminiraj komunikaciju
    ClearIORbuf();       
    CloseComPorts();
    return 0;
}

DWORD WINAPI AcquisitionTask( LPVOID pData )
{
    while( !ServerShutdown )
    {
        DWORD sret = WaitForSingleObject( smACQ, 200 );
        if( sret == WAIT_TIMEOUT )
            continue;

        int latchS = timeS;     // zapamti vreme ulaska u funkciju
        for( int rtu=0; rtu < rtdb.no_rtu; rtu++ )
        {
            RTUT *rtup = rtdb.rtut + rtu;
            // ocitaj trenutno stanje ulaza/izlaza, ako je RTU aktivan i nije virtualan
            if( !rtup->var.virt && get_bit(rtup->opr.status, OPR_ACT) )
            {
                if( !(latchS % rtup->opr.acq_period) )             // ako je vreme za to
                {
                    int protokol = rtdb.chanet[rtup->fix.channel].fix.protocol;
                    RTU_protokoli[protokol]->acquisit_rtu( rtu );
                }
            }
        }
    }
    return 0;
}

DWORD WINAPI PipeTask( LPVOID pData )
{
    OpenPipe();         // napravi cevi
    while( !ServerShutdown )
    {
        // Sacekaj klijenta da se javi
        WaitForMessage();                 // konekcijom ili poslatom komandom
		
    }
    ClosePipe();     // zatvori cevi
    return 0;
}

DWORD WINAPI RefreshTask( LPVOID pData )
{
    while( !ServerShutdown )
    {
        // posalji sve promene ka klijentu
        RefreshClient();
        Sleep( 200 );
    }	
    DisconnectClient();
    return 0;
}