#include "StdAfx.h"
#include "Simulator/Simulator.h"


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

/*-----------------------------------------------------------------------*/
/*                      TASK1                                            */
/*       Vremenske funkcije AUP koda                                     */
/*-----------------------------------------------------------------------*/

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



/*-----------------------------------------------------------------------*/
/*                      TASK2                                            */
/*             Simulacija promena u postrojenju                          */
/*-----------------------------------------------------------------------*/

DWORD WINAPI SimulationTask( LPVOID pData )
{
    SetSimulation();

    while( !ServerShutdown )
    {
        DWORD sret = WaitForSingleObject( smACQ, 200 );
        if( sret == WAIT_TIMEOUT )
            continue;

        RunSimulation( 1 );

        for( int rtu=0; rtu < rtdb.no_rtu; rtu++ )
        {
            RTUT *rtup = rtdb.rtut + rtu;
            if( !(timeS % rtup->opr.acq_period) )             // ako je vreme za to
            {
                SimulatorRTU( rtu );
                if( rtup->var.first_scan )
                {
                    rtup->var.first_scan = 0;
                    rtup->var.val = 1;
                    rtup->chg = CHG_FINAL;
                }
            }
        }

    }
    return 0;
}

/*-----------------------------------------------------------------------*/
/*                      TASK4                                            */
/*       Rukovanje named-pipeom : connect & receive                      */
/*-----------------------------------------------------------------------*/
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

/*-----------------------------------------------------------------------*/
/*                      TASK5                                            */
/*             Osvezavanje klijenta, send to pipe                        */
/*-----------------------------------------------------------------------*/
DWORD WINAPI RefreshTask( LPVOID pData )
{
    while( !ServerShutdown )
    {
        // posalji sve promene ka klijentu
        RefreshClient();
        Sleep( 200 );
    }	
    return 0;
}