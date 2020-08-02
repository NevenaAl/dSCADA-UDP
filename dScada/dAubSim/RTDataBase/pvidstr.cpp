/************************************************************************
*           pvidstr.cpp je modul za rad sa PVID strukturom              *
************************************************************************/

#include "StdAfx.h"

/************************************************************************
*  Funkcija get_PVID vraca pokazivac na PVID strukturu procesne         *
*  promenljive na imena (tekstualnog kljuca)                            *
************************************************************************/

PVID* get_PVID( char *rtu_name, char* pvar_name, bool hash_find )
{
    if( !hash_find)
    {
        int i, j;
        for( i=0; i < rtdb.no_rtu; i++ )
        {
            if( !strcmp( rtdb.rtut[i].fix.name, rtu_name ) )
            {
                // nasli smo RTU
                RTUT *rtup = rtdb.rtut + i;
                if( pvar_name == NULL)
                {
                    // vrati PVID samog RTUa
                    return &rtup->fix.mID;
                }
                // nastavi pretragu do trazene SCADA promenljive
                for( j=0; j < rtup->fix.ainfut.total; j++ )
                {
                    if( !strcmp(pvar_name, rtup->fix.ainfut.ain_t[j].fix.name) )
                        return &rtup->fix.ainfut.ain_t[j].fix.mID;
                }
                for( j=0; j < rtup->fix.aoutfut.total; j++ )
                {
                    if( !strcmp(pvar_name, rtup->fix.aoutfut.aout_t[j].fix.name) )
                        return &rtup->fix.aoutfut.aout_t[j].fix.mID;
                }
                for( j=0; j < rtup->fix.cntfut.total; j++ )
                {
                    if( !strcmp(pvar_name, rtup->fix.cntfut.cnt_t[j].fix.name) )
                        return &rtup->fix.cntfut.cnt_t[j].fix.mID;
                }
                for( j=0; j < rtup->fix.digfut.total; j++ )
                {
                    if( !strcmp(pvar_name, rtup->fix.digfut.dig_t[j].fix.name) )
                        return &rtup->fix.digfut.dig_t[j].fix.mID;
                }
                for( j=0; j<rtup->fix.objfut.total; j++ )
                {
                    if( !strcmp(pvar_name, rtup->fix.objfut.obj_t[j].fix.name) )
                        return &rtup->fix.objfut.obj_t[j].fix.mID;
                }
            }
        }
    }
    else
    {
        PVID *pvID = NULL;
        if( pvar_name == NULL)
        {
            // trazi samo RTU
            pvID = find_rtu( rtu_name );
        }
        else
        {
            // trazi promenljivu
            pvID = find_var( rtu_name, pvar_name );
        }
        if( pvID != NULL )
            return pvID;
    }

    // zabelezi pogresnu referencu, ali nemoj prekinuti rad
    if( pvar_name == NULL)
    {
        PutLogMessage( "REF_ERR: RTU NAME=%s", rtu_name );
    }
    else
    {
        PutLogMessage( "REF_ERR: PVAR NAME=%s:%s", rtu_name, pvar_name );
    }
    return NULL;
}

/************************************************************************
*  Funkcija get_pvp vraca pokazivac na strukturu procesne promenljive,  *
*  ili na njen VAR, TMP ili FIX deo                                     *
************************************************************************/

void *get_pvp( PVID *pvid, int part )
{
    void *pvp = NULL;

    if( pvid->rtu < rtdb.no_rtu )
    {
        RTUT *rtup = rtdb.rtut + pvid->rtu;             // rtu index je OK
        switch( pvid->type )
        {

        case PVT_SYS:
            pvp = &rtdb;
            break;

        case PVT_RTU:
            if( pvid->sqn < rtdb.no_rtu )
            {
                if( !same_pvid( &rtup->fix.mID, pvid) )
                    break;
                switch( part )
                {
                case VAR_PART:
                    pvp = &((rtdb.rtut+pvid->sqn)->var);
                    break;
                case OPR_PART:
                    pvp = &((rtdb.rtut+pvid->sqn)->opr);
                    break;
                case FIX_PART:
                    pvp = &((rtdb.rtut+pvid->sqn)->fix);
                    break;
                case ALL_PART:
                default:
                    pvp = rtdb.rtut+pvid->sqn;
                    break;
                }
            }
            break;

        case PVT_AI:
            if( pvid->sqn < rtup->fix.ainfut.total )
            {
				ANAIN *aip = rtup->fix.ainfut.ain_t + pvid->sqn;
                if( !same_pvid( &aip->fix.mID, pvid) )
                    break;
                switch( part )
                {
                case VAR_PART:
                    pvp = &(aip->var);
                    break;
                case OPR_PART:
                    pvp = &(aip->opr);
                    break;
                case FIX_PART:
                    pvp = &(aip->fix);
                    break;
                case ALL_PART:
                default:
                    pvp = aip;
                    break;
                }
            }
            break;

        case PVT_CNT:
            if( pvid->sqn < rtup->fix.cntfut.total )
            {
                if( !same_pvid(&rtup->fix.cntfut.cnt_t[pvid->sqn].fix.mID, pvid) )
                    break;
                switch( part )
                {
                case VAR_PART:
                    pvp = &((rtup->fix.cntfut.cnt_t + pvid->sqn)->var);
                    break;
                case OPR_PART:
                    pvp = &((rtup->fix.cntfut.cnt_t + pvid->sqn)->opr);
                    break;
                case FIX_PART:
                    pvp = &((rtup->fix.cntfut.cnt_t + pvid->sqn)->fix);
                    break;
                case ALL_PART:
                default:
                    pvp = rtup->fix.cntfut.cnt_t + pvid->sqn;
                    break;
                }
            }
            break;

        case PVT_DIG:
            if( pvid->sqn < rtup->fix.digfut.total )
            {
                if( !same_pvid(&rtup->fix.digfut.dig_t[pvid->sqn].fix.mID, pvid) )
                    break;
                switch( part )
                {
                case VAR_PART:
                    pvp = &((rtup->fix.digfut.dig_t + pvid->sqn)->var);
                    break;
                case OPR_PART:
                    pvp = &((rtup->fix.digfut.dig_t + pvid->sqn)->opr);
                    break;
                case FIX_PART:
                    pvp = &((rtup->fix.digfut.dig_t + pvid->sqn)->fix);
                    break;
                case ALL_PART:
                default:
                    pvp = rtup->fix.digfut.dig_t + pvid->sqn;
                    break;
                }
            }
            break;

        case PVT_AO:
            if( pvid->sqn < rtup->fix.aoutfut.total )
            {
                if( !same_pvid(&rtup->fix.aoutfut.aout_t[pvid->sqn].fix.mID, pvid) )
                    break;
                switch( part )
                {
                case VAR_PART:
                    pvp = &((rtup->fix.aoutfut.aout_t + pvid->sqn)->var);
                    break;
                case OPR_PART:
                    pvp = &((rtup->fix.aoutfut.aout_t + pvid->sqn)->opr);
                    break;
                case FIX_PART:
                    pvp = &((rtup->fix.aoutfut.aout_t + pvid->sqn)->fix);
                    break;
                case ALL_PART:
                default:
                    pvp = rtup->fix.aoutfut.aout_t + pvid->sqn;
                    break;
                }
            }
            break;

        case PVT_OBJ:
            if( pvid->sqn < rtup->fix.objfut.total )
            {
                if( !same_pvid(&rtup->fix.objfut.obj_t[pvid->sqn].fix.mID, pvid) )
                    break;
                switch( part )
                {
                case VAR_PART:
                    pvp = &((rtup->fix.objfut.obj_t + pvid->sqn)->var);
                    break;
                case OPR_PART:
                    pvp = &((rtup->fix.objfut.obj_t + pvid->sqn)->opr);
                    break;
                case FIX_PART:
                    pvp = &((rtup->fix.objfut.obj_t + pvid->sqn)->fix);
                    break;
                case ALL_PART:
                default:
                    pvp = rtup->fix.objfut.obj_t + pvid->sqn;
                    break;
                }
                break;
            }
        default:
            break;
        }
    }

    // kriticna greska - neresena PVID referenca!
    if( pvp == NULL )
    {
        PutLogMessage( "PVREF_ERR: PVID=%d:%d:%d", pvid->type, pvid->rtu, pvid->sqn );
        _Abort();
    }

    return( pvp );
}

// uporedjuje dva PVIDa i vraca true ako jesu isti
bool same_pvid( PVID *pvid1, PVID *pvid2 )
{
    if( !memcmp(pvid1, pvid2, sizeof(PVID)) )
        return(true);
    else
        return(false);
}

char *get_name( PVID  *pvid )
{
    int rtu = pvid->rtu;

    switch( pvid->type )
    {
    case PVT_CHA:
        if( pvid->sqn >= rtdb.no_channel ) 
            return NULL;
        else 
            return rtdb.chanet[pvid->sqn].fix.name;
    case PVT_RTU:
        if( pvid->sqn >= rtdb.no_rtu ) 
            return NULL;
        else 
            return rtdb.rtut[pvid->sqn].fix.name;
        break;
    case PVT_AI:
        if( rtu >= rtdb.no_rtu || pvid->sqn >= rtdb.rtut[rtu].fix.ainfut.total )
            return NULL;
        else 
            return rtdb.rtut[rtu].fix.ainfut.ain_t[pvid->sqn].fix.name;
    case PVT_CNT:
        if( rtu >= rtdb.no_rtu || pvid->sqn >= rtdb.rtut[rtu].fix.cntfut.total )
            return NULL;
        else return 
            rtdb.rtut[rtu].fix.cntfut.cnt_t[pvid->sqn].fix.name;
    case PVT_DIG:
        if( rtu >= rtdb.no_rtu || pvid->sqn >= rtdb.rtut[rtu].fix.digfut.total )
            return NULL;
        else 
            return rtdb.rtut[rtu].fix.digfut.dig_t[pvid->sqn].fix.name;
    case PVT_AO:
        if( rtu >= rtdb.no_rtu || pvid->sqn >= rtdb.rtut[rtu].fix.aoutfut.total )
            return NULL;
        else 
            return rtdb.rtut[rtu].fix.aoutfut.aout_t[pvid->sqn].fix.name;
    case PVT_OBJ:
        if( rtu >= rtdb.no_rtu || pvid->sqn >= rtdb.rtut[rtu].fix.objfut.total )
            return NULL;
        else 
            return rtdb.rtut[rtu].fix.objfut.obj_t[pvid->sqn].fix.name;
    default:
        return NULL;
    }
}

void put_chg( PVID *pvid, int chgtype, int value )
{
    void *pvp = get_pvp( pvid );
    switch( pvid->type )
    {
    case PVT_AI:   ((ANAIN*)pvp)->chg[chgtype]   = value; break;
    case PVT_AO:   ((ANAOUT*)pvp)->chg[chgtype]  = value; break;
    case PVT_CNT:  ((COUNTER*)pvp)->chg[chgtype] = value; break;
    case PVT_DIG:  ((DIGITAL*)pvp)->chg[chgtype] = value; break;
    case PVT_OBJ:  ((OBJECT*)pvp)->chg[chgtype]  = value; break;
    }
}

int get_chg( PVID *pvid, int chgtype )
{
    int rchg = -1;
    void *pvp = get_pvp( pvid );
    switch( pvid->type )
    {
    case PVT_AI:   rchg = ((ANAIN*)pvp)->chg[chgtype];   break;
    case PVT_AO:   rchg = ((ANAOUT*)pvp)->chg[chgtype];  break;
    case PVT_CNT:  rchg = ((COUNTER*)pvp)->chg[chgtype]; break;
    case PVT_DIG:  rchg = ((DIGITAL*)pvp)->chg[chgtype]; break;
    case PVT_OBJ:  rchg = ((OBJECT*)pvp)->chg[chgtype];  break;
    }
    return rchg;
}


enum  _status_codes
{
    // OPR statusi
    OPRSTS_ACTIVE, OPRSTS_INACTIVE, OPRSTS_EV_INH, OPRSTS_CMD_INH, OPRSTS_MCMD_INH, OPRSTS_MAN_VAL,
    // VAR statusi  
    VARSTS_NORMAL, VARSTS_INVALID, VARSTS_LOW_ALARM, VARSTS_HI_ALARM, VARSTS_ALARM, VARSTS_ORL,
    VARSTS_LOW_WARN, VARSTS_HI_WARN, VARSTS_NO_COUNTING, VARSTS_LOW_CLAMP, VARSTS_HI_CLAMP 
};


char *tbl_status[] =
{
    "Active",  "!Active",  "EvntInh",  "CmdInh",  "MCmdInh",  
#ifndef _dSIM
    "Manual",
#else
    "Fail",
#endif
    "Normal", "!Valid", "LoLow", "HiHigh", "Alarm", "ORL", "Low", "High", "Stall", "LoClmp", "HiClmp"
};

int put_pvid_status( PVID* pvidp, int part, WORD flag, int value )
{
    void *pvp;
    int ret = 0;

    if( part == OPR_PART )
    {
        pvp = get_pvp( pvidp, OPR_PART );
        switch( pvidp->type )
        {
        case PVT_AI:   put_bit( ((ANAIN_OPR*)pvp)->status,   flag, value ); break;
        case PVT_AO:   put_bit( ((ANAOUT_OPR*)pvp)->status,  flag, value ); break;
        case PVT_CNT:  put_bit( ((COUNTER_OPR*)pvp)->status, flag, value ); break;
        case PVT_DIG:  put_bit( ((DIGITAL_OPR*)pvp)->status, flag, value ); break;
        case PVT_OBJ:  put_bit( ((OBJECT_OPR*)pvp)->status,  flag, value ); break;
        }
    }
    else if( part == VAR_PART )
    {
        pvp = get_pvp( pvidp, VAR_PART );
        switch( pvidp->type )
        {
        case PVT_AI:   put_bit( ((ANAIN_VAR*)pvp)->status,   flag, value ); break;
        case PVT_AO:   put_bit( ((ANAOUT_VAR*)pvp)->status,  flag, value ); break;
        case PVT_CNT:  put_bit( ((COUNTER_VAR*)pvp)->status, flag, value ); break;
        case PVT_DIG:  put_bit( ((DIGITAL_VAR*)pvp)->status, flag, value ); break;
        case PVT_OBJ:  put_bit( ((OBJECT_VAR*)pvp)->status,  flag, value ); break;
        }
    }
    else
    {
        ret = -1;
    }
    return ret;
}

WORD get_status( PVID* pvidp, int part  )
{
    void* pvp = get_pvp( pvidp );
    WORD sts;
    switch( pvidp->type )
    {
    case PVT_AI:
        if( part == VAR_PART )
            sts = ((ANAIN*)pvp)->var.status;
        else
            sts = ((ANAIN*)pvp)->opr.status;
        break;
    case PVT_AO:
        if( part == VAR_PART )
            sts = ((ANAOUT*)pvp)->var.status;
        else
            sts = ((ANAOUT*)pvp)->opr.status;
        break;
    case PVT_CNT:
        if( part == VAR_PART )
            sts = ((COUNTER*)pvp)->var.status;
        else
            sts = ((COUNTER*)pvp)->opr.status;
        break;
    case PVT_DIG:
        if( part == VAR_PART )
            sts = ((DIGITAL*)pvp)->var.status;
        else
            sts = ((DIGITAL*)pvp)->opr.status;
        break;
    case PVT_OBJ:
        if( part == VAR_PART )
            sts = ((OBJECT*)pvp)->var.status;
        else
            sts = ((OBJECT*)pvp)->opr.status;
        break;
    case PVT_RTU:
        sts = 0;
        break; 
    default:
        _Abort();
    }
    return sts;
}


int get_priority_status( PVID* pvidp, int part )
{
    int sts;
    WORD status = get_status( pvidp, part );

    if( part == OPR_PART )
    {
        // prioritet OPR statusa, opadajuci
        // !OPR_ACT, OPR_MAN_VAL, OPR_CMD_INH, OPR_MCMD_INH, OPR_EVENT_INH   
        sts = OPRSTS_ACTIVE;             // default status
        if( !(status & OPR_ACT) )
            sts = OPRSTS_INACTIVE;
        else if( status & OPR_MAN_VAL )
            sts = OPRSTS_MAN_VAL;
        else if( status & OPR_CMD_INH )
            sts = OPRSTS_CMD_INH;
        else if( status & OPR_MCMD_INH )
            sts = OPRSTS_MCMD_INH;
        else if( status & OPR_EVENT_INH )
            sts = OPRSTS_EV_INH;
    }
    else if( part == VAR_PART )
    {
        // prioriteti VAR statusa zavise od tipa procesne promenljive

        sts = VARSTS_NORMAL;       // default status
        switch( pvidp->type )
        {
        case PVT_AI:
            if( !(status & AI_VAL) )
                sts = VARSTS_INVALID;
            else if( status & AI_ORL )
                sts = VARSTS_ORL;
            else if( status & AI_LO_ALARM )
                sts = VARSTS_LOW_ALARM;
            else if( status & AI_HI_ALARM )
                sts = VARSTS_HI_ALARM;
            else if( status & AI_LO_WARN )
                sts = VARSTS_LOW_WARN;
            else if( status & AI_HI_WARN )
                sts = VARSTS_HI_WARN;
            break;
        case PVT_AO:
            if( !(status & AO_VAL) )
                sts = VARSTS_INVALID;
            else if( status & AO_LO_ALARM )
                sts = VARSTS_LOW_ALARM;
            else if( status & AO_HI_ALARM )
                sts = VARSTS_HI_ALARM;
            break;
        case PVT_CNT:
            if( !(status & CNT_VAL) )
                sts = VARSTS_INVALID;
            else if( status & CNT_STALLED )
                sts = VARSTS_NO_COUNTING;
            break;
        case PVT_DIG:
            if( !(status & DIG_VAL))
                sts = VARSTS_INVALID;
            else if(  status & DIG_ALARM )
                sts = VARSTS_ALARM;
            break;
        case PVT_OBJ:
            if( !(status & OBJ_VAL) )
                sts = VARSTS_INVALID;
            else if( status & OBJ_ALARM )
                sts = VARSTS_ALARM;
            break;
        }
    }

    return sts;
}

char *get_status_string( PVID* pvidp )
{
    // prvo proveri OPR status kao najprioritetniji
    int opr_sts = get_priority_status( pvidp, OPR_PART );
    if( (opr_sts == OPRSTS_INACTIVE) || (opr_sts == OPRSTS_MAN_VAL ) )
        return tbl_status[opr_sts];
    // vrati VAR status za aktivnu i non-manual promenljivu
    int sts_var = get_priority_status( pvidp, VAR_PART );
    return tbl_status[sts_var];
}



// rukovanje elementima objekta
PVID *getObjElmID( PVID *objID, int elem_i )
{
    OBJECT *obj = (OBJECT*) get_pvp( objID );
    return &obj->fix.element[elem_i];
}

void *getObjElm( PVID *objID, int elem_i )
{
    OBJECT *obj = (OBJECT*) get_pvp( objID );
    return get_pvp( &obj->fix.element[elem_i]);
}

float getObjPrm( PVID *objID, int prm_i )
{
    OBJECT *obj = (OBJECT*) get_pvp( objID );
    return obj->opr.param[prm_i];
}

int getObjType( PVID *objID )
{
    OBJECT *obj = (OBJECT *)get_pvp( objID );
    return obj->fix.type;
}

int getObjClass( PVID *objID )
{
    OBJECT *obj = (OBJECT *)get_pvp( objID );
    return obj->fix.obj_class;
}

int getObjCatg( PVID *objID )
{
    OBJECT *obj = (OBJECT *)get_pvp( objID );
    return obj->fix.category;
}


// vrati index digitalne komande ako postoji u katalogu DigCmd
int getCmd( char *reqval )
{
    for( int i=0; i<rtdb.no_command; i++ )
    {
        if( !strcmp(rtdb.DigCmd[i].command, reqval) )
            return i;
    }
    return -1;
}

// vrati index digitalnog stanja ako postoji u katalogu DigState
int getSts( char *reqval )
{
    for( int i=0; i<rtdb.no_state; i++ )
    {
        if( !strcmp(rtdb.DigState[i].state, reqval) )
            return i;
    }
    return -1;
}

// vraca stanje digitalnog uredjaja ili objekta
SCHAR getDigSts( PVID *pvid )
{
    SCHAR v;
    void *pvp = get_pvp( pvid );
    switch( pvid->type )
    {
    case PVT_DIG:
        v = ((DIGITAL*)pvp)->var.state;
        break;
    case PVT_OBJ:
        v = ((OBJECT*)pvp)->var.state;
        if( ((OBJECT*)pvp)->fix.obj_class == DIG_OBJ )
            break;
    default:
        _Abort();
    }
    return v;
}

// vraca komandu digitalnog uredjaja ili objekta
SCHAR getDigCmd( PVID *pvid )
{
    SCHAR v;
    void *pvp = get_pvp( pvid );
    switch( pvid->type )
    {
    case PVT_DIG:
        v = ((DIGITAL *)pvp)->var.command;
        break;
    case PVT_OBJ:
        v = ((OBJECT *)pvp)->var.command;
        if( ((OBJECT *)pvp)->fix.obj_class == DIG_OBJ )
            break;
    default:
        _Abort();
    }
    return v;
}

// vraca egu-vrednost kontinualnih procesnih promenljivih
float getEguVal( PVID *pvid )
{
    float v;
    void *pvp = get_pvp( pvid );
    switch( pvid->type )
    {
    case PVT_AI:
        v = ((ANAIN*)pvp)->var.egu_value;
        break;
    case PVT_AO:
        v = ((ANAOUT*)pvp)->var.egu_value;
        break;
    case PVT_CNT:
        v = ((COUNTER*)pvp)->var.egu_value;
        break;
    case PVT_OBJ:
        v=((OBJECT*)pvp)->var.egu_value;
        if( ((OBJECT *)pvp)->fix.obj_class == ANA_OBJ )
            break;
    default:
        _Abort();
    }
    return(v);
}

long get_timestamp( PVID *pvid )
{
    long v;
    void *pvp = get_pvp( pvid );
    switch( pvid->type )
    {
    case PVT_AI:
        v = ((ANAIN*)pvp)->var.timestamp;
        break;
    case PVT_AO:
        v = ((ANAOUT*)pvp)->var.timestamp;
        break;
    case PVT_CNT:
        v = ((COUNTER*)pvp)->var.timestamp;
        break;
    case PVT_DIG:
        v = ((DIGITAL*)pvp)->var.timestamp;
        break;
    case PVT_OBJ:
        v=((OBJECT*)pvp)->var.timestamp;
        break;
    default:
        _Abort();
    }
    return v;
}

// podrska Tandem i Repbus protokolima
void save_var( PVID *pvid, BYTE *mp )
{
    void *pvp = get_pvp( pvid, VAR_PART );
    switch( pvid->type )
    {
    case PVT_AI:   memcpy( pvp, mp, sizeof(ANAIN_VAR) );     break;
    case PVT_AO:   memcpy( pvp, mp, sizeof(ANAOUT_VAR) );    break;
    case PVT_CNT:  memcpy( pvp, mp, sizeof(COUNTER_VAR) );   break;
    case PVT_DIG:  memcpy( pvp, mp, sizeof(DIGITAL_VAR) );   break;
    case PVT_OBJ:  memcpy( pvp, mp, sizeof(OBJECT_VAR) );    break;
    }
}

void save_opr( PVID *pvid, BYTE *mp )
{
    void *pvp = get_pvp( pvid, OPR_PART );
    switch( pvid->type )
    {
    case PVT_AI:   memcpy( pvp, mp, sizeof(ANAIN_OPR) );     break;
    case PVT_AO:   memcpy( pvp, mp, sizeof(ANAOUT_OPR) );    break;
    case PVT_CNT:  memcpy( pvp, mp, sizeof(COUNTER_OPR) );   break;
    case PVT_DIG:  memcpy( pvp, mp, sizeof(DIGITAL_OPR) );   break;
    case PVT_OBJ:  memcpy( pvp, mp, sizeof(OBJECT_OPR) );    break;
    }
}

