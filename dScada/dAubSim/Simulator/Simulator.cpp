#include "stdafx.h"

#include "Simulator.h"

static bool SimRandom = true;


void SetSimulation( void )
{
    // pozovi rand zbog simulacije
    srand( (unsigned)time(NULL) );

    // odredjuje da li se radi slucajna simulacija ili simulacija postrojenja
    SimRandom = false;

    // postavi pocetno stanje za simulaciju
    for( int rtu=0; rtu<rtdb.no_rtu; rtu++ )
    {
        RTUT *rtup = rtdb.rtut + rtu;
        for( int i=0; i<rtup->fix.ainfut.total; i++ )
        {
            ANAIN *ainp = rtup->fix.ainfut.ain_t + i;
            ainp->var.egu_value= (ainp->opr.egu_range[1] - ainp->opr.egu_range[0]) / 2 ;
            ainp->chg[CHG_VAR] = CHG_SIMUL;
        }

        for( int i=0; i<rtup->fix.cntfut.total; i++ )
        {
            COUNTER *cntp = rtup->fix.cntfut.cnt_t + i;
            cntp->var.egu_value= 0;
            cntp->chg[CHG_VAR] = CHG_SIMUL;
        }  

        for( int i=0; i<rtup->fix.digfut.total; i++ )
        {
            DIGITAL* digp = rtup->fix.digfut.dig_t + i;
            // default postavka
            digp->var.state = digp->fix.states[0];
            digp->var.command = digp->fix.commands[0];
            put_bit( digp->var.status, DIG_OUT_SCAN1, 0 );           // jer u dSim nicemu ne sluzi
            if( !SimRandom )
            {
                // postavlja pocetno stanje postrojenja
                SetPlantDigDev( digp );
            }
            digp->chg[CHG_VAR] = CHG_SIMUL;
        }
    }

    if( !SimRandom )
    {
        SetPlantSimulator();
    }
}

// Simulacija realnog postrojenja
void RunSimulation( int Tsim )
{
    if( !SimRandom )
    {
        // simuliraj ispravno ponasanje postrojenja
        RunPlantSimulator( Tsim );
    }
    else
    {
        // simuliraj slucajne promene
        for( int rtu=0; rtu<rtdb.no_rtu; rtu++ )
        {
            RTUT *rtup = rtdb.rtut + rtu;
            for( int i=0; i<rtup->fix.ainfut.total; i++ )
            {
                ANAIN *ainp = rtdb.rtut[rtu].fix.ainfut.ain_t + i;
                if( (ainp->opr.status & OPR_ACT) && !(ainp->opr.status & OPR_MAN_VAL) )
                    randAI( ainp );
            }

            for( int i=0; i<rtup->fix.cntfut.total; i++ )
            {
                COUNTER *cntp = rtup->fix.cntfut.cnt_t + i;
                if( (cntp->opr.status & OPR_ACT) && !(cntp->opr.status & OPR_MAN_VAL) )
                    randCNT( cntp );
            }  

            for( int i=0; i<rtup->fix.digfut.total; i++ )
            {
                DIGITAL* digp = rtup->fix.digfut.dig_t + i;
                if( (digp->opr.status & OPR_ACT) && !(digp->opr.status & OPR_MAN_VAL) )
                    randDigDev( digp );
            }
        }
    }
}

void CheckDIgital( DIGITAL *digp )
{
    if( get_bit(digp->var.status, DIG_IN_SCAN1) )
    {
        put_bit( digp->var.status, DIG_IN_SCAN1, 0 );
        if( digp->fix.no_out )
            RestoreLastCommand( digp );              // vidi sta bi mogla biti zadnja komanda
    }
    CheckSpontaneousChange( digp );            // Provera Spontane Promene */
}

void SimulatorRTU( int rtu )
{
    int i;
    RTUT* rtup = rtdb.rtut + rtu;

    for( i=0; i<rtup->fix.ainfut.total; i++ )
    {
        ANAIN *ainp = rtup->fix.ainfut.ain_t + i;
        if( ainp->chg[CHG_VAR] == CHG_SIMUL || get_bit(ainp->var.status, AI_SCAN1) )
        {
            analog_input_alarm( ainp );               // samo po promeni AI ili prvom scanu
            put_bit( ainp->var.status, AI_SCAN1, 0 );
            ainp->var.timestamp = timeS;              // omoguci osvezavanje dClienta
            ainp->chg[CHG_VAR] = CHG_FINAL;
        }
    }

    for( i=0; i<rtup->fix.cntfut.total; i++ )
    {
        COUNTER *cntp = rtup->fix.cntfut.cnt_t + i;
        if( get_bit(cntp->var.status, CNT_SCAN1) )
        {
            cntp->chg[CHG_VAR] = CHG_SIMUL;
            put_bit( cntp->var.status, CNT_SCAN1, 0 );
        }
        bool chg = (cntp->chg[CHG_VAR])?true:false;                 // proveri CNT svaki put
        chg = counter_input_alarm( cntp, chg );                 // proveri CNT svaki put
        if( cntp->chg[CHG_VAR] || chg )
        {
            cntp->var.timestamp = timeS;
            cntp->chg[CHG_VAR] = CHG_FINAL;                    // omoguci osvezavanje dClienta
        }
    }  

    for( i=0; i<rtup->fix.digfut.total; i++ )
    {
        DIGITAL* digp = rtup->fix.digfut.dig_t + i;
        if( (digp->var.status & DIG_CMD_REQ) && !(digp->opr.status & OPR_MAN_VAL) )
        {
            MakeCmdResponse( digp );               // simuliraj odziv na digitalnu komandu
        }
        if( digp->var.status & (DIG_IN_SCAN1|DIG_IN_SCAN2) )
        {
            put_bit( digp->var.status, DIG_IN_SCAN2, 0 );
            digp->chg[CHG_VAR] = CHG_SIMUL;                 // obezbedi obradu ako je prvi scan ili obrisan otkaz
        }
        if( digp->fix.no_in && (digp->chg[CHG_VAR]==CHG_SIMUL) && !get_bit(digp->var.status, DIG_CMD_REQ) )
        {
            CheckDIgital( digp );                  // obradi spontane promene
        }
        if( digp->chg[CHG_VAR] )
        {
            digp->var.timestamp = timeS;
            digp->chg[CHG_VAR] = CHG_FINAL;                 // omoguci osvezavanje dClienta
        }
    }
}

