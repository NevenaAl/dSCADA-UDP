
#include "stdafx.h"
#include "modbus.h"
#include "math.h"

// Implementira ModBus komunikaciju sa slave uredjajima 

static MODBUS_CFG mdb_cfg[1024];          // MAX 1024 RTUa u sistemu

static int ReadAddCfg( int rtu )
{
    FILE *sf;
    char buff[MAX_PATH];
    int no_msg=0, i=0, ret = 0;

    MODBUS_CARD *mcp;                   // Modbus Card Pointer
    char reg_type[16], str_pio[16];
    int pio_type, broj_pio, adresa, byte_order, dec_point;
    int base_pio[PIO_NUM];

    // odredi pocetne vrednosti za base_pio
    for( i = PIO_AI; i < PIO_NUM; i++ )
        base_pio[i] = 0;

    sprintf_s( buff, sizeof(buff), "%s\\%s.txt", StnCfg.SysFolder, rtdb.rtut[rtu].fix.name );
    if( (sf=fopen(buff, "rb")) == NULL )
        return( -1 );

    while( !feof(sf) )
    {
        if( !fgets(buff, sizeof(buff), sf) )
        {
            ret = -1;
            break;
        }

        if( !no_msg )
        {
            // NO_CARDS  7  
            char string[16];
            sscanf( buff, "%s %d", string, &no_msg );
            if( !strcmp(string, "NO_CARD") && no_msg )
            {
                // uzmi memoriju za pio module
                mcp = (MODBUS_CARD*)calloc( no_msg, sizeof(MODBUS_CARD) );
                i = 0;
            }
        }
        else
        {
            reg_type[0] = str_pio[0] = 0;
            broj_pio = adresa = byte_order = dec_point = 0;
            sscanf( buff, "%s %s %d %d %d %d", reg_type, str_pio, &broj_pio, &adresa, &dec_point, &byte_order );
            // proveri sta si ucitao
            if( !strcmp(str_pio, "") )
                continue;

            if( !strcmp(str_pio, "AI") )
                pio_type = PIO_AI;
            else if( !strcmp(str_pio, "AO") )
                pio_type = PIO_AO;
            else if( !strcmp(str_pio, "CNT") )
                pio_type = PIO_CNT;
            else if( !strcmp(str_pio, "DI") )
                pio_type = PIO_DI;
            else if( !strcmp(str_pio, "DO") )
                pio_type = PIO_DO;
            else
            {
                ret = -1;
                goto kraj;
            }

            // definicija nacina obrade ucitanih podataka
            mcp[i].pio_type = pio_type;
            mcp[i].no_pio  = broj_pio;
            mcp[i].base_hwa = base_pio[pio_type];
            // pomeri adresu za sledeci blok
            base_pio[pio_type] += broj_pio;

            // pocetna adresa registarskog bloka ili adresa registra
            mcp[i].adr_reg = adresa;

            // broj pozicija za koje se pomera decimalna tacka nad INT/LONG podatkom
            mcp[i].dec_point  = dec_point;

            // obrada po tipu registra
            if( !strcmp(reg_type, "DO_REG") )
            {
                // definicija registra
                mcp[i].reg_type = DO_REG;      // Coils u originalu
                mcp[i].reg_len = 2;           // duzina [byte]
                mcp[i].byte_order = 21;       // Hi, Low
                mcp[i].rd_code = READ_COILS;  // Read Coils (DO status)
                mcp[i].wr_code = FORCE_COIL;  // Force Single Coil
                mcp[i].no_reg  = broj_pio;    // duzina u bitima
                // definicija obrade
                mcp[i].pio_type = PIO_DO;      // bez obzira na CFG
            }
            else if( !strcmp(reg_type, "DI_REG") )
            {
                // definicija registra
                mcp[i].reg_type = DI_REG;         // Input Status u originalu
                mcp[i].reg_len = 2;              // duzina [byte]
                mcp[i].byte_order = 21;          // Hi, Low
                mcp[i].rd_code = 2;              // Read Input Status (DI status)
                mcp[i].wr_code = -1;             // nema pisanja
                mcp[i].no_reg  = broj_pio;       // duzina u bitima
                // definicija obrade             
                mcp[i].pio_type = PIO_DI;         // bez obzira na CFG
            }
            else if( !strcmp(reg_type, "IN_REG") )
            {
                // definicija registra
                mcp[i].reg_type = IN_REG;         // Input Register u originalu
                mcp[i].reg_len = 2;              // duzina [byte]
                mcp[i].byte_order = 21;          // Hi, Low
                mcp[i].rd_code = 4;              // Read Input Regs
                mcp[i].wr_code = -1;             // nema pisanja
                mcp[i].no_reg  = broj_pio;       // broj registara
                // definicija obrade             
                mcp[i].pio_type = PIO_AI;         // bez obzira na CFG
            }

            else if( !strcmp(reg_type, "HR_INT") )
            {
                // definicija registra
                mcp[i].reg_type = HR_INT;         // Holding Register u originalu
                mcp[i].reg_len = 2;              // duzina [byte] - 16 bita
                mcp[i].byte_order = 21;          // Hi, Low
                mcp[i].rd_code = 3;              // Read Holding Regs
                mcp[i].wr_code = SET_HOLDREGS;   // Preset Multiple HoldRegs
                mcp[i].no_reg  = broj_pio;       // broj registara
                //if( pio_type == PIO_DI )
                //{
                //   mcp[i].pio_type = PIO_DI;         // bez obzira na CFG
                //   mcp[i].no_reg = broj_pio /16;    // broj registara sa DI statusima
                //   if( broj_pio % 16 )
                //      mcp[i].no_reg += 1;           // uvecaj po potrebi (van modula 16)
                //}
            }
            else if( !strcmp(reg_type, "HR_LONG") )
            {
                // definicija registra
                mcp[i].reg_type = HR_LONG;        // Holding Register duple duzine
                mcp[i].reg_len = 2;              // duzina [byte] - 32 bita
                mcp[i].byte_order = 4321;        // Hi, Low
                mcp[i].rd_code = 3;              // Read Holding Regs
                mcp[i].wr_code = SET_HOLDREGS;
                mcp[i].no_reg  = broj_pio * 2;   // broj registara
            }
            else if( !strcmp(reg_type, "HR_FLT") )
            {
                // definicija registra
                mcp[i].reg_type = HR_FLT;         // Holding Register u originalu
                mcp[i].reg_len = 2;              // duzina [byte] - 2 registra po 16 bita sa dve potrosene adrese
                mcp[i].byte_order = 4321;        // Hi, Low
                mcp[i].rd_code = 3;              // Read Holding Regs
                mcp[i].wr_code = SET_HOLDREGS;   // Preset Multiple HoldRegs
                mcp[i].no_reg  = broj_pio * 2;   // broj registara
            }
            else
            {
                wputs( 5, "InitModbusCard %s: Illegal register type = %s", rtdb.rtut[rtu].fix.name, reg_type );
                free ( mcp );
                ret = -1;
                goto kraj;
            }

            // prethodno smo namestili default vrednosti
            switch( byte_order )
            {
            case 0:
                break;                           // podrazumevana vrednost
            case 12:
            case 21:
            case 1234:
            case 4321:
            case 2143:
                mcp[i].byte_order = byte_order;  // prema zahtevu
                break;
            default:
                wputs( 5, "InitModbusCard %s: Neispravan BYTE_ORDER=%d ", rtdb.rtut[rtu].fix.name, byte_order );
                free ( mcp );
                return -1;
            }

            // pomeri indeks
            i++;

            if( i == no_msg )
            {
                mdb_cfg[rtu].no_cards = no_msg;
                mdb_cfg[rtu].mdb_cards = mcp;
                goto kraj;
            }
        }
    }

kraj:
    fclose( sf );
    return ret;

}


int mdb_get_short( BYTE *tp, int byte_order )
{
    short *sp;
    char pc[2];

    switch( byte_order )
    {
    case 12:
        pc[0] = tp[0];
        pc[1] = tp[1];
        break;
    case 21:
        pc[0] = tp[1];
        pc[1] = tp[0];
        break;
    default:
        return 0;
    }
    sp = (short *) pc;
    return *sp;
}

int mdb_get_long( BYTE *tp, int byte_order )
{
    int   *iop;
    char pc[4];

    switch( byte_order )
    {
    case 1234:
        iop = (int *) tp;            // bez konverzije
        break;
    case 4321:
        pc[0] = tp[3];
        pc[1] = tp[2];
        pc[2] = tp[1];
        pc[3] = tp[0];
        iop = (int *) pc;
        break;
    case 2143:
        pc[0] = tp[1];
        pc[1] = tp[0];
        pc[2] = tp[3];
        pc[3] = tp[2];
        iop = (int *) pc;
        break;
    default:
        return 0;
    }
    return *iop;
}


float mdb_get_float( BYTE *tp, int byte_order )
{
    float *fp;
    char pc[4];

    switch( byte_order )
    {
    case 1234:
        fp = (float *) tp;            // bez konverzije
        break;
    case 4321:
        pc[0] = tp[3];
        pc[1] = tp[2];
        pc[2] = tp[1];
        pc[3] = tp[0];
        fp = (float *) pc;
        break;
    case 2143:
        pc[0] = tp[1];
        pc[1] = tp[0];
        pc[2] = tp[3];
        pc[3] = tp[2];
        fp = (float *) pc;
        break;
    default:
        return 0.0;
    }
    return *fp;
}

// iz primljene poruke izdvaja jedan modbus_card registar
int getRegData( char* rtuname, MODBUS_CARD *mcp, BYTE **tp, float *egu_value, int *raw_value )
{
    switch( mcp->reg_type )
    {
    case IN_REG:         // binarne vrednosti tipa 12 (Hi, Low)
    case HR_INT:
        *raw_value = mdb_get_short( *tp, mcp->byte_order );
        if( !mcp->dec_point )
        {  // pravi INT16
            *raw_value &= 0x0FFF;  // svedi na 12 A/D bita
            *egu_value= 0;
        }
        else
        {  // to je u stvari float broj
            *egu_value= (float)( *raw_value / pow(10.0, mcp->dec_point) );
            *raw_value = -1;
        }
        *tp += 2;                         // pomeri tp za duzinu registra
        break;
    case HR_LONG:
        *raw_value = mdb_get_long( *tp, mcp->byte_order );
        if( mcp->dec_point )
        {
            *egu_value= (float) mdb_get_long( *tp, mcp->byte_order );
            *egu_value/= (float) pow(10.0, mcp->dec_point);
            *raw_value = -1;
        }
        *tp += 4;
        break;
    case HR_FLT:
        *egu_value= mdb_get_float( *tp, mcp->byte_order );
        *raw_value = -1;
        *tp += 4;
        break;
    default:
        wputs( 5, "getRegData %s: Illegal type of Modbus register = %d !", rtuname, mcp->reg_type );
        return -1;
    }
    return 0;
}


void mdb_put_short( BYTE *sp, BYTE *buf, int byte_order )
{
    switch( byte_order )
    {
    case 12:
        buf[0] = sp[0];
        buf[1] = sp[1];
        break;
    case 21:
        buf[0] = sp[1];
        buf[1] = sp[0];
        break;
    }
}

void mdb_put_long( BYTE *sp, BYTE *buf, int byte_order )
{
    switch( byte_order )
    {
    case 1234:
        memcpy( buf, sp, sizeof(int) );  // bez konverzije
        break;
    case 4321:
        buf[0] = sp[3];
        buf[1] = sp[2];
        buf[2] = sp[1];
        buf[3] = sp[0];
        break;
    case 2143:
        buf[0] = sp[1];
        buf[1] = sp[0];
        buf[2] = sp[3];
        buf[3] = sp[2];
        break;
    }
}

void mdb_put_float( BYTE *sp, BYTE *buf, int byte_order )
{
    switch( byte_order )
    {
    case 1234:
        memcpy( buf, sp, sizeof(float) );  // bez konverzije
        break;
    case 4321:
        buf[0] = sp[3];
        buf[1] = sp[2];
        buf[2] = sp[1];
        buf[3] = sp[0];
        break;
    case 2143:
        buf[0] = sp[1];
        buf[1] = sp[0];
        buf[2] = sp[3];
        buf[3] = sp[2];
        break;
    }
}

// u poruku za slanje zapisuje jedan modbus_card registar
int setRegData( BYTE* buf, MODBUS_CARD *mcp, float egu_value, int raw_value )
{
    short sint;
    int lint, ret=0;
    switch( mcp->reg_type )
    {
    case IN_REG:         // binarne vrednosti tipa 12 (Hi, Low)
    case HR_INT:
        if( mcp->dec_point )
            sint = (short)( egu_value* pow(10.0,mcp->dec_point) );
        else
            sint = raw_value;
        mdb_put_short( (BYTE*)&sint, buf, mcp->byte_order );
        ret = 2;
        break;
    case HR_LONG:
        lint = raw_value;
        mdb_put_long( (BYTE*)&lint, buf, mcp->byte_order );
        ret = 4;
        break;
    case HR_FLT:        // float vrednosti
        mdb_put_float( (BYTE*)&egu_value, buf, mcp->byte_order );
        ret = 4;
        break;
    default:
        wputs( 5, "setRegData: Illegal type of Modbus register = %d !", mcp->reg_type );
        break;
    }
    return ret;
}

// nalazi Modbus adresu i-tog clana modbus kartice
int getRegAdr( MODBUS_CARD *mcp, int ofset )
{
    int adresa;

    switch( mcp->reg_type )
    {
    case DO_REG:
    case DI_REG:
    case IN_REG:         // binarne vrednosti tipa 12 (Hi, Low)
    case HR_INT:
        adresa = mcp->adr_reg + ofset;
        break;
    case HR_LONG:
    case HR_FLT:
        adresa = mcp->adr_reg + 2*ofset;
        break;
    default:
        wputs( 5, "getRegAdr: Illegal type of Modbus register = %d !", mcp->reg_type );
        adresa = -1;
        break;
    }
    return adresa;
}


/* Obrada primljene MODBUS poruke u RTU formatu */
void ProcessModbusResponse( IORB *iop )
{
    BYTE *mp, *tp;
    int i, raw_value, reg_broj;
    float egu_value;
    WORD reg_adr;  

    MODBUS_CARD *mcp = mdb_cfg[iop->rtu].mdb_cards + iop->id;
    RTUT *rtup = rtdb.rtut + iop->rtu;
    int mdb_tcp = (rtdb.chanet[iop->chn].fix.type == TCP)?1:0;

    mp = iop->rbuf;           /* primljeni odgovor */
    if( mdb_tcp )
        mp = mp + 6;

    switch( iop->reply_type )
    {
    case READ_COILS  :
        tp = mp + 3;                    // pointer iza SlaveAdr, FuncKoda i byte_counta
        for( i=0; i < mcp->no_pio; i++ )
        {  // sacuvaj u raw data tabeli
            BYTE reg8 = tp[i/8];
            SetDigitalOutputValue( iop->rtu, mcp->base_hwa+i, reg8 & (1<<(i%8)) );
        }
        if( rtup->fix.digfut.total )
        {
            for( i=0; i < mcp->no_pio; i++ )
            {  // obradi primljene podatke
                short sqn = rtup->fix.digfut.do_lt[mcp->base_hwa + i];
                if( sqn != -1 )
                {
                    // uradi obradu pridruzenog dig uredjaja
                    DIGITAL *digp = rtup->fix.digfut.dig_t + sqn;
                    if( get_bit(digp->opr.status, OPR_ACT) )
                    {
                        ProcessDigitalOutput( digp );
                    }
                }
            }
        }
#ifdef SEQUENTIAL
        for( i=0; i < rtup->fix.digfut.total; i++ )    // obrada sirovog stanja DI
        {
            DIGITAL *digp = rtup->fix.digfut.dig_t+i;
            if( !get_bit(digp->opr.status, OPR_ACT) || get_bit(digp->opr.status, OPR_MAN_VAL) )
                continue;
            if( digp->fix.no_out )
            {
                bool uradi_obradu = false;
                for( j=0; j < digp->fix.no_out; j++ )
                {
                    if( (digp->fix.hw_out[j] >= mcp->base_hwa) && (digp->fix.hw_out[j] <= mcp->base_hwa + mcp->no_pio) )
                    {
                        uradi_obradu = true;
                        break;
                    }
                }
                if( uradi_obradu )
                    ProcessDigitalOutput( digp );
            }
        }
#endif
        break;

    case READ_INPUTS  :
        tp = mp + 3;                    // pointer iza SlaveAdr, FuncKoda i byte_counta
        for( i=0; i < mcp->no_pio; i++ )
        {  // sacuvaj u raw data tabeli
            BYTE reg8 = tp[i/8];
            PutRawDigitalInput( iop->rtu, mcp->base_hwa+i, reg8 & (1<<(i%8)) );
        }
        for( i=0; i < mcp->no_pio; i++ )
        {  // obradi primljene podatke
            short sqn = rtup->fix.digfut.di_lt[mcp->base_hwa + i];
            if( sqn != -1 )
            {
                // uradi obradu pridruzenog dig uredjaja
                DIGITAL *digp = rtup->fix.digfut.dig_t + sqn;
                if( get_bit(digp->opr.status, OPR_ACT) && !get_bit(digp->opr.status, OPR_MAN_VAL) )
                {
                    ProcessDigitalInput( digp );
                }
            }
        }
#ifdef SEQUENTIAL
        for( i=0; i < rtup->fix.digfut.total; i++ )    // obrada sirovog stanja DI
        {
            DIGITAL *digp = rtup->fix.digfut.dig_t+i;
            if( !get_bit(digp->opr.status, OPR_ACT) || get_bit(digp->opr.status, OPR_MAN_VAL) )
                continue;
            if( digp->fix.no_in )
            {
                bool uradi_obradu = false;
                for( j=0; j < digp->fix.no_in; j++ )
                {
                    if( (digp->fix.hw_in[j] >= mcp->base_hwa) && (digp->fix.hw_in[j] <= mcp->base_hwa + mcp->no_pio) )
                    {
                        uradi_obradu = true;
                        break;
                    }
                }
                if( uradi_obradu )
                    ProcessDigitalInput( digp );
            }
        }
#endif
        break;

    case READ_HOLDREGS:
    case READ_INREGS  :	  
        tp = mp + 3;                    // pointer iza SlaveAdr, FuncKoda i byte_counta
        switch( mcp->pio_type )
        {
        case PIO_AI:
            {  // pocetna vrednost rawdat pointera
                WORD *rawdat = rtup->fix.ainfut.rawdat + mcp->base_hwa;
                for( i=0; i < mcp->no_pio; i++ )
                {  
                    // uzmi podatak
                    if( !getRegData(rtup->fix.name, mcp, &tp, &egu_value, &raw_value) )
                    {
                        // pronadji ANAIN sa istom adresom i uradi sekundarnu obradu
                        short sqn = rtup->fix.ainfut.ain_lt[mcp->base_hwa + i];
                        if( sqn != -1 )
                        {
                            ANAIN *aip = rtup->fix.ainfut.ain_t + sqn;
                            if( get_bit(aip->opr.status, OPR_ACT) && !get_bit(aip->opr.status, OPR_MAN_VAL) )
                            {
                                ProcessAnalogInput( aip, raw_value, egu_value );
                                rawdat[i] = aip->var.raw_value;
                            }
                        }
#ifdef SEQUENTIAL
                        for( j=0; j < rtup->fix.ainfut.total; j++ )
                        {
                            ANAIN *aip = rtup->fix.ainfut.ain_t + j;
                            if( !get_bit(aip->opr.status, OPR_ACT) || get_bit(aip->opr.status, OPR_MAN_VAL) )
                                continue;
                            if( aip->fix.hwa == mcp->base_hwa + i )
                            {
                                ProcessAnalogInput( aip, raw_value, egu_value);
                                rawdat[i] = aip->var.raw_value;
                                break;
                            }
                        }
#endif
                    }
                }
            }
            break;

        case PIO_AO:
            {
                // pocetna vrednost rawdat pointera
                WORD *rawdat = rtup->fix.aoutfut.rawdat + mcp->base_hwa;
                for( i=0; i < mcp->no_pio; i++ )
                {  
                    // uzmi podatak
                    if( !getRegData(rtup->fix.name, mcp, &tp, &egu_value, &raw_value) )
                    {
                        short sqn = rtup->fix.aoutfut.aout_lt[mcp->base_hwa + i];
                        if( sqn != -1 )
                        {
                            // pronadji ANAOUT sa istom adresom i uradi sekundarnu obradu
                            ANAOUT *aop = rtup->fix.aoutfut.aout_t + sqn;
                            if( get_bit(aop->opr.status, OPR_ACT) && !get_bit(aop->opr.status, OPR_MAN_VAL) )
                            {
                                ProcessAnalogOutput( aop, raw_value, egu_value );
                                rawdat[i] = aop->var.raw_value;
                            }
                        }
#ifdef SEQUENTIAL
                        for( j=0; j < rtup->fix.aoutfut.total; j++ )
                        {
                            ANAOUT *aop = rtup->fix.aoutfut.aout_t + j;
                            if( !get_bit(aop->opr.status, OPR_ACT) || get_bit(aop->opr.status, OPR_MAN_VAL) )
                                continue;
                            if( aop->fix.hwa == mcp->base_hwa + i )
                            {
                                //put_bit_validnosti( &aop->fix.mID, 1 );
                                ProcessAnalogOutput( aop, raw_value, egu_value);
                                rawdat[i] = aop->var.raw_value;
                                break;
                            }
                        }
#endif
                    }
                }
            }
            break;
        case PIO_CNT:
            {
                // pocetna vrednost rawdat pointera
                long *rawdat = rtup->fix.cntfut.rawdat + mcp->base_hwa;
                for( i=0; i < mcp->no_pio; i++ )
                {  
                    // uzmi podatak
                    if( !getRegData(rtup->fix.name, mcp, &tp, &egu_value, &raw_value) )
                    {
                        short sqn = rtup->fix.cntfut.cnt_lt[mcp->base_hwa + i];
                        if( sqn != -1 )
                        {
                            // pronadji COUNTER sa istom adresom i uradi sekundarnu obradu
                            COUNTER *cntp = rtup->fix.cntfut.cnt_t + sqn;
                            if( get_bit(cntp->opr.status, OPR_ACT) && !get_bit(cntp->opr.status, OPR_MAN_VAL) )
                            {
                                ProcessCounter( cntp, raw_value, 0 );
                                rawdat[i] = cntp->var.raw_value;
                            }
                        }
#ifdef SEQUENTIAL
                        for( j=0; j < rtup->fix.cntfut.total; j++ )
                        {
                            COUNTER *cntp = rtup->fix.cntfut.cnt_t + j;
                            if( !get_bit(cntp->opr.status, OPR_ACT) || get_bit(cntp->opr.status, OPR_MAN_VAL) )
                                continue;
                            if( cntp->fix.hwa == mcp->base_hwa + i )
                            {
                                ProcessCounter( cntp, raw_value, 0 );
                                rawdat[i] = cntp->var.raw_value;
                                break;
                            }
                        }
#endif
                    }
                }
            }
            break;

        case PIO_DI:
        case PIO_DO:
            break;
        }
        break;

    case FORCE_COIL  :            // uspesno slanje digitalne commands
    case FORCE_COILS :
        break;

    case SET_HOLDREG :            // uspesno slanje analogne commands (AO, CNT)
    case SET_HOLDREGS:
        {
            tp = mp + 2;               // pointer iza SlaveAdr i FuncKoda; sledi adresa i broj_reg
            reg_adr = (tp[0]<<8) + tp[1];  
            reg_broj = (tp[2]<<8) + tp[3];
            reg_broj /= mcp->reg_len;           // svedi na broj registara iza jedne PIO tacke
            switch( mcp->pio_type )
            {
            case PIO_AO:
                break;
            case PIO_CNT:
                {
                    // pocetna vrednost rawdat pointera
                    long *rawdat = rtup->fix.cntfut.rawdat + mcp->base_hwa;
                    for( int k=0; k < reg_broj; k++ )
                    {
                        for( i=0; i < mcp->no_pio; i++ )
                        {
                            // nadji offset registra nad kojim je izdata komanda
                            if( reg_adr + k*mcp->reg_len == mcp->adr_reg + i*mcp->reg_len )
                            {
                                short sqn = rtup->fix.cntfut.cnt_lt[mcp->base_hwa + i];
                                if( sqn != -1 )
                                {
                                    // obrisi COUNTER
                                    COUNTER *cntp = rtup->fix.cntfut.cnt_t + sqn;
                                    OnClearCounterDone( cntp );
                                    rawdat[i] = 0;
                                    break;
                                }
#ifdef SEQUENTIAL
                                for( j=0; j < rtup->fix.cntfut.total; j++ )
                                {
                                    COUNTER *cntp = rtup->fix.cntfut.cnt_t + j;
                                    if( cntp->fix.hwa == mcp->base_hwa + i )
                                    {
                                        OnClearCounterDone( cntp );
                                        rawdat[i] = 0;
                                        return;
                                    }
                                }
#endif
                            }
                        }
                    }
                }
                break;
            }
            break;
        }
    }
}


////////////////////////////////////////////////////////////////////
//   Tabela za racunanje CRC znaka Modbus RTU protokola
////////////////////////////////////////////////////////////////////
static BYTE CRC_High[256]=
{
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40
};

static BYTE CRC_Low[256]=
{
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
    0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
    0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
    0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,
    0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
    0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
    0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,
    0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
    0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,
    0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
    0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,
    0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
    0x40
} ;

static WORD make_CRC( BYTE *por, int duz)    // MODBUS RTU CRC racunica
{
    WORD i;
    BYTE CRCh=0xFF, CRCl=0xFF;
    while( duz-- )
    {
        i    = CRCh ^ *por++;
        CRCh = CRCl ^ CRC_High[i];
        CRCl = CRC_Low[i];
    }
    i = (CRCh<<8) | CRCl;
    return i;
}


static int init_RTU( int rtu )
{	
    return ReadAddCfg( rtu );     
}

static void free_RTU( int rtu )
{	
    free( mdb_cfg[rtu].mdb_cards );
}

static char* get_prot_name( void )
{
    return( "MODBUS" );
}

/* Pribavljanje imena poruke */
static char* get_msg_name( BYTE type )
{
    switch( type )       /* tip_poruke */
    {
    case READ_COILS  :     return( "READ_COILS    " );
    case READ_INPUTS :     return( "READ_INPUTS   " );
    case READ_HOLDREGS:    return( "READ_HOLDREGS " );
    case READ_INREGS  :    return( "READ_INREGS   " );
    case FORCE_COIL  :     return( "FORCE_COIL    " );
    case FORCE_COILS :     return( "FORCE_COILS   " );
    case SET_HOLDREG :     return( "SET_HOLDREG   " );
    case SET_HOLDREGS:     return( "SET_HOLDREGS  " );
    default :              return( "NEPOZNAT TIP  " );
    }
}


/* Prozivka RTU-a */
static void acquisit_rtu ( int rtu )
{
    MSGID msg;
    MODBUS_CARD *mcp = mdb_cfg[rtu].mdb_cards;

    // popuni MSGID gde i kome saljes
    msg.rtu = rtu;

    // regularna modbus prozivka
    for( int i=0; i < mdb_cfg[rtu].no_cards; i++ )
    {
        if( mcp[i].rd_code != -1 )
        {
            // pripremi slanje poruke
            msg.type = mcp[i].rd_code;
            msg.id  = i;                            // broj bloka
            Send( &msg, NULL, 0, END_LIST );
        }
    }
}

/* Slanje poruke */
static int send_msg( IORB *iop, BYTE *msgbuf, int mlen )
{
    MODBUS_CARD *mcp;
    BYTE *mp, *iobuf;

    if( iop->id < mdb_cfg[iop->rtu].no_cards )
        mcp = mdb_cfg[iop->rtu].mdb_cards + iop->id;     // blok
    else
    {
        PutLocEvent( NULL, "LOS MCP_INDEX: ID=%d %s", iop->id, get_msg_name(iop->request_type) );
        return -1;
    }

    int mdb_tcp = (rtdb.chanet[iop->chn].fix.type == TCP)?1:0;

    // iskopiraj i popuni osnovne podatke
    iop->io_request = SEND_RECV;
    iop->reply_type = iop->request_type;             // pocetna vrednost, moze ga promeniti exception

    iop->maxrep  = 2;

    // namesti buf pointere
    iobuf = msgbuf;
    mp = iop->sbuf;
    /* napravi poruku */
    int len = 0;

    if( mdb_tcp )
    {
        mp[len++] = 0;			            // Transaction Identifier (first byte)
        mp[len++] = 1;			            // Transaction Identifier (second byte)
        mp[len++] = 0;			            // Protocol Identifier		(first byte)
        mp[len++] = 0;			            // Protocol Identifier		(second byte)
        // ostalo len poruke za slanje
        len += 2;				            // Length Field (2 bytes)
    }
    // nastavljamo 
    mp[len++] = rtdb.rtut[iop->rtu].fix.com_adr;        // Slave/Unit adresa
    mp[len++] = iop->request_type;                      // Function
    if( iobuf )
    {                                         // ovo su commands
        if( len + mlen > MSG_LEN -2 )
        {
            PutLocEvent( NULL, "%s: %s", "PREDUGA PORUKA", get_msg_name(iop->request_type) );
            return -2;
        }
        memcpy( mp+len, iobuf, mlen );
        len += mlen;
    }
    else
    {                                         //ovo su citanja
        mp[len++] = mcp->adr_reg >> 8;         // Starting Address Hi
        mp[len++] = mcp->adr_reg & 0xFF;       // Starting Address Lo
        mp[len++] = mcp->no_reg >> 8;          // No. of Regs Hi
        mp[len++] = mcp->no_reg & 0xFF;        // No. of Regs  Lo
    }

    if( !mdb_tcp )
    {
        // sracunaj LRC znak i napuni ga u sbuf
        WORD crc = make_CRC( mp, len);
        mp[len++] = crc >> 8;                     // CRC High
        mp[len++] = crc & 0xFF;                   // CRC Low
    }
    else
    {
        mp[4] = (len-6) >> 8;
        mp[5] = (len-6) & 0xFF;
    }
    // ovo dalje vazi za sve
    iop->slen = len;                     // namesti duzinu upita

    // namesti duzinu odgovora
    if( mdb_tcp )
    {
        iop->rlen = 6;                    // 6 bajta headera
    }
    else
    {
        // UART i UDP
        switch( iop->request_type )
        {
        case READ_COILS:
        case READ_INPUTS:	   
            iop->rlen = 5 + (mcp->no_reg/8);		   //(mcp->no_reg/16)*2         
            if( mcp->no_reg % 8 )							   //( mcp->no_reg % 16 )
                iop->rlen += 1;			                  // 2         
            break;
        case READ_HOLDREGS:
        case READ_INREGS:		 
            iop->rlen = 5 + mcp->no_reg*mcp->reg_len;
            break;
        case FORCE_COIL:
        case FORCE_COILS:
        case SET_HOLDREG:	   
            iop->rlen = len; // odgovor je istovetan upitu + CRC
            break;
        case SET_HOLDREGS:
            iop->rlen = 8; // odgovor je istovetan upitu + CRC
            break;
        default:
            PutLocEvent( NULL, "Nepoznata MODBUS poruka!" );
            return -3;
        }
    }

    return 0;
}

/* Slanje BROADCAST poruke */
static int send_bcst_msg( IORB *iop, BYTE *msgbuf, int mlen )
{
    return 0;
}

/* Slanje commands */
static void send_command( PVID *pvidp, float cmd )
{
    MSGID msg;
    BYTE buf[60];

    // popuni MSGID gde i kome saljes
    msg.rtu = pvidp->rtu;

    MODBUS_CARD *mcp = mdb_cfg[msg.rtu].mdb_cards;

    switch( pvidp->type )
    {
    case PVT_AO:
        {
            ANAOUT *aop = (ANAOUT*)get_pvp( pvidp );
            // nadji pravu karticu
            for( int i=0; i < mdb_cfg[msg.rtu].no_cards; i++ )
            {
                if( mcp[i].pio_type != PIO_AO )
                    continue;
                if( (aop->fix.hwa >= mcp[i].base_hwa) && (aop->fix.hwa < mcp[i].base_hwa + mcp[i].no_pio ) )
                {
                    // nasao sam pravu karticu, posalji komandu
                    if( mcp[i].wr_code == SET_HOLDREGS )
                    {
                        // pripremi slanje poruke
                        int ofset = aop->fix.hwa - mcp[i].base_hwa;
                        int adr_reg = getRegAdr( mcp+i, ofset );
                        int len = 0;
                        buf[len++] = adr_reg >> 8;             // Starting Address Hi
                        buf[len++] = adr_reg & 0xFF;           // Starting Address Lo
                        buf[len++] = 0;                        // no_pnt Hi
                        buf[len++] = mcp->reg_len/2;           // no_pnt Lo - uvek 1 ili 2
                        int blen = setRegData( buf+len+1, mcp+i, 0, (int)cmd );
                        if( blen )
                        {
                            buf[len++] = blen;                  // Byte_count
                            len += blen;
                            msg.type = (BYTE) mcp[i].wr_code;
                            msg.id  = i;                        // mcp index
                            Send( &msg, buf, len, END_LIST );
                        }
                    }
                }
            }
        }
        break;
    case PVT_DIG:
        {
            int i, j, rbkom;
            DIGITAL *digp = (DIGITAL*)get_pvp( pvidp );

            rbkom = GetRawCommand( digp, (SCHAR) cmd );
            if( rbkom == -1 )          // ovo ne bi smelo da se desi
                return;

            msg.type = FORCE_COIL;
            for( i=0; i < mdb_cfg[msg.rtu].no_cards; i++ )
            {
                if( mcp[i].pio_type != PIO_DO )
                    continue;
                if( (digp->fix.hw_out[0] >= mcp[i].base_hwa) && (digp->fix.hw_out[0] < mcp[i].base_hwa + mcp[i].no_pio ) )
                {              // nasli smo karticu
                    if( mcp[i].wr_code == FORCE_COIL )
                    {
                        msg.id  = i;
                        BYTE cmdmsk = 1 << digp->fix.no_out;
                        for( j=0; j < digp->fix.no_out; j++ )
                        {
                            // pripremi slanje poruke
                            int adr_reg = getRegAdr( mcp+i, digp->fix.hw_out[j] - mcp[i].base_hwa );
                            int len = 0;
                            buf[len++] = adr_reg >> 8;             // Starting Address Hi
                            buf[len++] = adr_reg & 0xFF;           // Starting Address Lo

                            // sa leva na desno po bitima, sto odgovara porastu hwa
                            cmdmsk = cmdmsk >> 1;
                            if( rbkom & cmdmsk )
                                buf[len++] = 0xFF;             // ON
                            else
                                buf[len++] = 0;                // OFF
                            buf[len++] = 0;
                            Send( &msg, buf, len, END_LIST );
                        }
                    }
                    break;         
                }
            }
        }
        break;
    case PVT_CNT:
        {                       /* Brisanje brojaca  */
            COUNTER *cp = (COUNTER*)get_pvp( pvidp );
            // nadji pravu karticu
            for( int i=0; i < mdb_cfg[msg.rtu].no_cards; i++ )
            {
                if( mcp[i].pio_type != PIO_CNT )
                    continue;
                if( (cp->fix.hwa >= mcp[i].base_hwa) && (cp->fix.hwa < mcp[i].base_hwa + mcp[i].no_pio ) )
                {
                    // nasao sam pravu karticu, posalji komandu
                    if( mcp[i].wr_code == SET_HOLDREGS )
                    {
                        // pripremi slanje poruke
                        int ofset = cp->fix.hwa - mcp[i].base_hwa;
                        int adr_reg = getRegAdr( mcp+i, ofset );
                        int len = 0;			           
                        buf[len++] = adr_reg >> 8;             // Starting Address Hi
                        buf[len++] = adr_reg & 0xFF;           // Starting Address Lo
                        buf[len++] = 0;                        // no_pnt Hi
                        buf[len++] = mcp->reg_len;             // no_pnt Lo - uvek 1 ili 2
                        int blen = setRegData( buf+len+1, mcp+i, 0, 0 );
                        if( blen )
                        {
                            buf[len++] = blen;                  // Byte_count
                            len += blen;
                            msg.type = (BYTE) mcp[i].wr_code;
                            msg.id  = i;                        // mcp index
                            Send( &msg, buf, len, END_LIST );
                        }
                    }
                }
            }
        }
        break;
    }
}

// Modbus ne podrzava unsolicited transfer
static int recv_msg( IORB *iop, MSGID *msg )
{
    iop->io_request = RECV;
    iop->reply_type = msg->type;
    iop->maxrep  = 1;
    iop->rlen = -1;
    return 0;
}

/* Provera CRC nakon kompletnog prijema - poziva se ako nije MODBUS_TCP */
static void check_msg( IORB* iop )
{
    iop->rlen -= 2;                // duzina INFO dela poruke bez CRC
    WORD crc = (iop->rbuf[iop->rlen]<<8) + iop->rbuf[iop->rlen+1];
    if( crc != make_CRC(iop->rbuf, iop->rlen) )
    {
        iop->sts = ERR_CRC;                    // neispravan prijem
    }
}

// Prijem karaktera iz kom.stream-a ili prekidne rutine
int get_char_tcp(BYTE rb, IORB *iop)
{
    if( iop->no_rcv >= MSG_LEN )
    {
        iop->sts = ERR_LEN;
        return 1;                              // kraj prijema
    }

    iop->rbuf[iop->no_rcv] = rb;             // akumuliraj poruku      
    switch( iop->no_rcv++ )
    {
    case 3:
        // provera zaglavlja
        if( 0x00010000 != mdb_get_long(iop->rbuf, 4321) )
        {
            iop->no_rcv = 0;
        }
        break;
    case 5:
        // redefinisi rlen - duzinu poruke za prijem
        iop->rlen = 6 + mdb_get_short( iop->rbuf+4, 21);
        break;
    case 6:
        // provera UnitID
        if( rb != iop->sbuf[6] )               // da li odgovora Slave adresi	  
        {
            iop->no_rcv = 0;
        }
        break;
    case 7:
        // proveri tip odgovora
        if( rb & 0x80 )                        // exception kod ?
        {
            iop->reply_type = rb;
        }
        else if( rb != iop->reply_type )
        {
            iop->no_rcv = 0;
        }
        break;
    default:
        if( iop->no_rcv == iop->rlen )
            return 1;                           // kraj prijema
        break;
    }

    return 0;
}

// nije MODBUS_TCP
int get_char_rtu( BYTE rb, IORB *iop )
{
    if( iop->no_rcv >= MSG_LEN )             // provera duzine
    {
        iop->sts = ERR_LEN;
        return 1;                              // kraj prijema
    }

    // ako je prvi znak
    if( iop->no_rcv == 0 )
    {
        if( rb == iop->sbuf[0] )               //  i odgovora Slave adresi
            iop->rbuf[iop->no_rcv++] = rb;		// pocni akumulaciju poruke u rbuf	  
    }
    else
    {
        // normalan prijem do rlen
        iop->rbuf[iop->no_rcv++] = rb;        // akumuliraj poruku

        if( (iop->no_rcv == 2) )              // proveri tip odgovora
        {
            // da li je primljen exception code      
            if( rb & 0x80 )         
            {
                iop->rlen = 5;
                iop->reply_type = rb;
            }
            else if( rb != iop->reply_type )    // ako kod poruke nije isti onom u upitu - greska u sekvenci poruka
            {
                iop->no_rcv = 0;
                return 0;
            }
        }
        else if( iop->no_rcv == iop->rlen )
        {
            check_msg( iop );                   // proveri CRC
            return 1;                           // kraj prijema
        }
    }
    return 0;                                 // nastavi prijem
}

static int get_char( BYTE rb, IORB *iop )
{
    int ret;
    if( rtdb.chanet[iop->chn].fix.type == TCP )
        ret = get_char_tcp( rb, iop );
    else
        ret = get_char_rtu( rb, iop );
    return ret;
}

/* Obrada primljene poruke */
static void proccess_msg( IORB *iop )
{
    char buf[128];

    switch( iop->reply_type )
    {
    case READ_COILS  :
    case READ_INPUTS  :
    case READ_HOLDREGS:
    case READ_INREGS  :
    case FORCE_COIL  :
    case FORCE_COILS :
    case SET_HOLDREG :
    case SET_HOLDREGS:
        ProcessModbusResponse( iop );
        break;

    default:
        if( iop->reply_type & 0x80 )
        {
            int reply_type = iop->reply_type & 0x7F; 
            sprintf( buf, "REXCEPTION CODE %d: RTU=%s MSG=%2d:%s", iop->rbuf[2], rtdb.rtut[iop->rtu].fix.name, reply_type, get_msg_name(reply_type) );
            PutEvent( NULL, buf );
        }
        else
        {
            sprintf( buf, "RxMdbPorErr %2d:%s", iop->reply_type, get_msg_name(iop->reply_type) );
            PutEvent( NULL, buf );
        }
        break;
    }
}

// po svakom uspesnom izvrsenju IO zahteva
static void on_msg_ok( IORB* iop )
{
    on_rtu_ok( rtdb.rtut + iop->rtu );
}

// po svakom neuspesnom izvrsenju IO zahteva
static void on_msg_err( IORB* iop )
{
    on_rtu_err( rtdb.rtut + iop->rtu );
}

static int pre_release( IORB* iop )
{
    return OK;
}

static void on_connect( IORB* iop )
{
}

// po prekidu veze (po diskonekciji)
static void on_disconnect( IORB* iop )
{
}


RTU_PROCEDURE MODBUS_protokol=
{
    MODBUS_DATA,
    init_RTU,
    free_RTU,
    get_prot_name,
    get_msg_name,
    acquisit_rtu,
    send_msg,
    send_bcst_msg,
    send_command,
    recv_msg,
    get_char,
    proccess_msg,
    on_msg_ok,
    on_msg_err,
    pre_release,
    on_connect,
    on_disconnect
};



#ifndef TRUE

int ReadAddCfg( int rtu )

    int mdb_get_short( BYTE *tp, int byte_order )
    int mdb_get_long( BYTE *tp, int byte_order )
    float mdb_get_float( BYTE *tp, int byte_order )
    // iz primljene poruke izdvaja jedan modbus_card registar
    int getRegData( BYTE* ime, MODBUS_CARD *mcp, BYTE **tp, float *egu_value, int *raw_value )

    void mdb_put_short( BYTE *sp, BYTE *buf, int byte_order )
    void mdb_put_long( BYTE *sp, BYTE *buf, int byte_order )
    void mdb_put_float( BYTE *sp, BYTE *buf, int byte_order )
    // u poruku za slanje zapisuje jedan modbus_card registar
    int setRegData( BYTE* buf, MODBUS_CARD *mcp, float egu_value, int raw_value )

    // nalazi Modbus adresu i-tog clana modbus kartice
    int getRegAdr( MODBUS_CARD *mcp, int ofset )

    /* Obrada primljene MODBUS poruke u RTU formatu */
    void ProcessModbusResponse( IORB *iop )

    static WORD make_CRC( BYTE *por, int duz)    // MODBUS RTU CRC racunica
    /* Provera CRC nakon kompletnog prijema - poziva se ako nije MODBUS_TCP */
    void check_msg( IORB* iop )

    // Prijem karaktera iz kom.stream-a ili prekidne rutine
    int get_char( BYTE rb, IORB *iop )
    int get_char_tcp(BYTE rb, IORB *iop)
    int get_char_rtu( BYTE rb, IORB *iop )

    // Modbus ne podrzava unsolicited transfer
    int recv_msg( IORB *iop, MSGID *msg )

    /* Pribavljanje imena poruke */
    char* get_msg_name( BYTE type )

    /* Slanje poruke */
    int send_msg( IORB *iop, BYTE *msgbuf, int mlen )
    /* Slanje BROADCAST poruke */
    int send_bcst_msg( IORB *iop, BYTE *msgbuf, int mlen )

    /* Slanje komande */
    void send_command( PVID *pvidp, float cmd )

    /* Prozivka RTU-a */
    void acquisit_rtu ( int rtu )

    /* Obrada primljene poruke */
    void proccess_msg( IORB *iop )

    /* Brisanje brojaca  */
    int clear_counter( PVID* pvidp, int clr_type )

    void on_msg_err( IORB* iop )
    void on_msg_ok( IORB* iop )

    char* get_prot_name( void )

    int init_RTU( int rtu )

#endif
