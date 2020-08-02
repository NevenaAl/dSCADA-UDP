#include "stdafx.h"
#include "uthash.h"

// hash table level
#define HASH_LEVEL 2            // 1 ili 2

//  hash funkcije: 
//  HASH_JEN     Jenkins (default)
//  HASH_BER     Bernstein
//  HASH_SAX     Shift-Add-Xor
//  HASH_OAT     One-at-a-time
//  HASH_FNV     Fowler/Noll/Vo
//  HASH_SFH     Paul Hsieh

//#define HASH_FUNCTION HASH_BER


#if HASH_LEVEL == 1

typedef struct hash_item 
{
    char name[NAME_LEN*2];             /* key */
    PVID pvid;      
    UT_hash_handle hh;         /* makes this structure hashable */
} HASH_ITEM;

struct hash_item *hvars = NULL;

void hash_add_var( PVID *pvid, char *rtu, char *name) 
{
    HASH_ITEM *hi = NULL;

    char kname[20];             // key name
    strcpy(kname, rtu);
    strcat(kname, ":");
    strcat(kname, name);

    // check the name
    //HASH_FIND_STR(hvars, kname, hi);    /* kname already in the hash? */
    //if( hi == NULL ) 
    {
        hi = (HASH_ITEM *)malloc( sizeof(HASH_ITEM) );
        if( hi == NULL )
            _Abort();
        strcpy( hi->name, kname );
        memcpy( &hi->pvid, pvid, sizeof(PVID) );
        HASH_ADD_STR( hvars, name, hi);/* name: name of key field */
    }
}

HASH_ITEM *hash_find_var( char *rtu, char *name )
{
    HASH_ITEM *s;

    char kname[20];             // key name
    strcpy(kname, rtu);
    strcat(kname, ":");
    strcat(kname, name);
    // potrazi ga
    HASH_FIND_STR(hvars, kname, s);    /* find by kname */
    return s;
}

void delete_var(HASH_ITEM *var)
{
    HASH_DEL( hvars, var);  /* user: pointer to deletee */
    free( var );
}

void delete_all( void ) 
{
    HASH_ITEM *cur_var, *tmp;

    HASH_ITER( hh, hvars, cur_var, tmp )
    {
        HASH_DEL(hvars, cur_var);        /* delete it (users advances to next) */
        free(cur_var);                  /* free it */
    }
}

int count_all( void ) 
{
    return HASH_COUNT( hvars );
}

void InitHashTable( void )
{
    for( int r=0; r < rtdb.no_rtu; r++ )
    {
        int i;
        RTUT *rtup = rtdb.rtut + r;
        for( i=0; i < rtup->fix.ainfut.total; i++ )
        {
            ANAIN *aip = rtup->fix.ainfut.ain_t + i;
            hash_add_var( &aip->fix.mID, rtup->fix.name, aip->fix.name );
        }
        for( i=0; i < rtup->fix.aoutfut.total; i++ )
        {
            ANAOUT *aop = rtup->fix.aoutfut.aout_t + i;
            hash_add_var( &aop->fix.mID, rtup->fix.name, aop->fix.name );
        }
        for( i=0; i< rtup->fix.cntfut.total; i++ )
        {
            COUNTER *cntp = rtup->fix.cntfut.cnt_t + i;
            hash_add_var( &cntp->fix.mID, rtup->fix.name, cntp->fix.name );
        }
        for( i=0; i< rtup->fix.digfut.total; i++ )
        {
            DIGITAL *digp = rtup->fix.digfut.dig_t + i;
            hash_add_var( &digp->fix.mID, rtup->fix.name, digp->fix.name );
        }
        for( i=0; i< rtup->fix.objfut.total; i++ )
        {
            OBJECT *objp = rtup->fix.objfut.obj_t + i;
            hash_add_var( &objp->fix.mID, rtup->fix.name, objp->fix.name );
        }
    }

}

#elif HASH_LEVEL == 2

typedef struct hash_item 
{
    char name[NAME_LEN];             /* key */
    PVID pvid;      
    UT_hash_handle hh;         /* makes this structure hashable */
    struct hash_item *sub;
} HASH_ITEM;

HASH_ITEM *hvars = NULL;


HASH_ITEM *hash_add_rtu( PVID *pvid, char *rtu ) 
{
    HASH_ITEM *hi = NULL;
    hi = (HASH_ITEM *)calloc( 1, sizeof(HASH_ITEM) );
    if( hi == NULL )
        _Abort();
    strcpy( hi->name, rtu );
    hi->sub = NULL;
    memcpy( &hi->pvid, pvid, sizeof(PVID) );
    HASH_ADD_STR( hvars, name, hi );                /* name: name of key field */
    return hi;
}

void hash_add_var( HASH_ITEM *rtu, PVID *pvid, char *name ) 
{
    HASH_ITEM *hi = NULL;
    hi = (HASH_ITEM *)calloc( 1, sizeof(HASH_ITEM) );
    if( hi == NULL )
        _Abort();
    strcpy( hi->name, name );
    hi->sub = NULL;
    memcpy( &hi->pvid, pvid, sizeof(PVID) );
    HASH_ADD_STR( rtu->sub, name, hi);                 /* name: name of key field */
}

HASH_ITEM *hash_find_rtu( char *rtu )
{
    HASH_ITEM *r;
    // nadji RTU
    HASH_FIND_STR(hvars, rtu, r);
    return r;
}

HASH_ITEM *hash_find_var( char *rtu, char *name )
{
    HASH_ITEM *r, *s;
    // nadji RTU prvo
    HASH_FIND_STR(hvars, rtu, r);
    if( r != NULL )
    {
        // potom po imeni promenljive
        HASH_FIND_STR(r->sub, name, s);
    }
    return s;
}

void delete_var(HASH_ITEM *var)
{
    HASH_DEL( hvars, var);  /* user: pointer to deletee */
    free( var );
}

void delete_all( void ) 
{
    /* clean up both hash tables */
    HASH_ITEM *cur_rtu, *tmp_rtu;
    HASH_ITER( hh, hvars, cur_rtu, tmp_rtu )
    {
        HASH_ITEM *cur_var, *tmp_var;
        HASH_ITER( hh, hvars->sub, cur_var, tmp_var )
        {
            HASH_DEL(hvars->sub, cur_var);          /* delete it (users advances to next) */
            free(cur_var);                          /* free it */
        }
        HASH_DEL(hvars, cur_rtu);                   /* delete it (users advances to next) */
        free(cur_rtu);                              /* free it */
    }
}

int count_all( void ) 
{
    int count = 0;
    /* count all variables in sub hash tables */
    HASH_ITEM *cur_rtu, *tmp_rtu;
    HASH_ITER( hh, hvars, cur_rtu, tmp_rtu )
    {
        count += HASH_COUNT( hvars->sub );
    }
    return count;
}

void InitHashTable( void )
{
    for( int r=0; r < rtdb.no_rtu; r++ )
    {
        int i;
        RTUT *rtup = rtdb.rtut + r;
        HASH_ITEM *hrtu = hash_add_rtu( &rtup->fix.mID, rtup->fix.name );

        for( i=0; i < rtup->fix.ainfut.total; i++ )
        {
            ANAIN *aip = rtup->fix.ainfut.ain_t + i;
            hash_add_var( hrtu, &aip->fix.mID, aip->fix.name );
        }
        for( i=0; i < rtup->fix.aoutfut.total; i++ )
        {
            ANAOUT *aop = rtup->fix.aoutfut.aout_t + i;
            hash_add_var( hrtu, &aop->fix.mID, aop->fix.name );
        }
        for( i=0; i< rtup->fix.cntfut.total; i++ )
        {
            COUNTER *cntp = rtup->fix.cntfut.cnt_t + i;
            hash_add_var( hrtu, &cntp->fix.mID, cntp->fix.name );
        }
        for( i=0; i< rtup->fix.digfut.total; i++ )
        {
            DIGITAL *digp = rtup->fix.digfut.dig_t + i;
            hash_add_var( hrtu, &digp->fix.mID, digp->fix.name );
        }
        for( i=0; i< rtup->fix.objfut.total; i++ )
        {
            OBJECT *objp = rtup->fix.objfut.obj_t + i;
            hash_add_var( hrtu, &objp->fix.mID, objp->fix.name );
        }
    }
}


PVID *find_rtu( char *rtu )
{
    PVID *rtuID = NULL;
    // nadji RTU u hash tabeli
    HASH_ITEM *r = hash_find_rtu( rtu );
    if( r != NULL )
    {
        // vrati njegov PVID
        rtuID = &r->pvid;
    }
    return rtuID;
}

PVID *find_var( char *rtu, char *name )
{
    PVID *varID = NULL;
    // nadji RTU u hash tabeli
    HASH_ITEM *s = hash_find_var( rtu, name );
    if( s != NULL )
    {
        // vrati PVID SCADA variable
        varID = &s->pvid;
    }
    return varID;
}


#endif



void FreeHashTable( void )
{
    delete_all();
}

#ifndef NULL
void FindHashTable( void )
{
    HASH_ITEM *hvar;

    for( int r=0; r < rtdb.no_rtu; r++ )
    {
        int i;
        RTUT *rtup = rtdb.rtut + r;
        for( i=0; i < rtup->fix.ainfut.total; i++ )
        {
            ANAIN *aip = rtup->fix.ainfut.ain_t + i;
            hvar = hash_find_var( rtup->fix.name, aip->fix.name );
            if( hvar == NULL )
                _Abort();
        }
        for( i=0; i < rtup->fix.aoutfut.total; i++ )
        {
            ANAOUT *aop = rtup->fix.aoutfut.aout_t + i;
            hvar = hash_find_var( rtup->fix.name, aop->fix.name );
            if( hvar == NULL )
                _Abort();
        }
        for( i=0; i< rtup->fix.cntfut.total; i++ )
        {
            COUNTER *cntp = rtup->fix.cntfut.cnt_t + i;
            hvar = hash_find_var( rtup->fix.name, cntp->fix.name );
            if( hvar == NULL )
                _Abort();
        }
        for( i=0; i< rtup->fix.digfut.total; i++ )
        {
            DIGITAL *digp = rtup->fix.digfut.dig_t + i;
            hvar = hash_find_var( rtup->fix.name, digp->fix.name );
            if( hvar == NULL )
                _Abort();
        }
        for( i=0; i< rtup->fix.objfut.total; i++ )
        {
            OBJECT *objp = rtup->fix.objfut.obj_t + i;
            hvar = hash_find_var( rtup->fix.name, objp->fix.name );
            if( hvar == NULL )
                _Abort();
        }
    }
}

void FindSequntial( void )
{
    PVID *pvid;
    for( int r=0; r < rtdb.no_rtu; r++ )
    {
        int i;
        RTUT *rtup = rtdb.rtut + r;
        for( i=0; i < rtup->fix.ainfut.total; i++ )
        {
            ANAIN *aip = rtup->fix.ainfut.ain_t + i;
            pvid = get_PVID( rtup->fix.name, aip->fix.name );
            if( pvid == NULL )
                _Abort();
        }
        for( i=0; i < rtup->fix.aoutfut.total; i++ )
        {
            ANAOUT *aop = rtup->fix.aoutfut.aout_t + i;
            pvid = get_PVID( rtup->fix.name, aop->fix.name );
            if( pvid == NULL )
                _Abort();
        }
        for( i=0; i< rtup->fix.cntfut.total; i++ )
        {
            COUNTER *cntp = rtup->fix.cntfut.cnt_t + i;
            pvid = get_PVID( rtup->fix.name, cntp->fix.name );
            if( pvid == NULL )
                _Abort();
        }
        for( i=0; i< rtup->fix.digfut.total; i++ )
        {
            DIGITAL *digp = rtup->fix.digfut.dig_t + i;
            pvid = get_PVID( rtup->fix.name, digp->fix.name );
            if( pvid == NULL )
                _Abort();
        }
        for( i=0; i< rtup->fix.objfut.total; i++ )
        {
            OBJECT *objp = rtup->fix.objfut.obj_t + i;
            pvid = get_PVID( rtup->fix.name, objp->fix.name );
            if( pvid == NULL )
                _Abort();
        }
    }
}


#define TIMER_VALUE 100000
void HashTest( void )
{
    TIMER *tmTEST = GetTimer( "tmTEST" );

    //int sz = sizeof(UT_hash_handle);
    //sz = sizeof(HASH_ITEM);

    // set hash table
    StartTimer( tmTEST, TIMER_VALUE, MSEC );
    InitHashTable();
    SuspendTimer( tmTEST );
    int t1 = TIMER_VALUE - TimeLeftTimer( tmTEST, MSEC );

    ResumeTimer( tmTEST );
    FindHashTable();
    SuspendTimer( tmTEST );
    int t2 = TIMER_VALUE - TimeLeftTimer( tmTEST, MSEC );
    StopTimer( tmTEST );

    int num_hvars = count_all(); 
    PutEvent( NULL, "Hash %d-level: num=%d t1=%d, t2=%d msec", HASH_LEVEL, num_hvars, t1, t2-t1 );

    // sekvencijalna pretraga
    //StartTimer( tmTEST, TIMER_VALUE, MSEC );
    //FindSequntial();
    //StopTimer( tmTEST );
    //int t3 = TIMER_VALUE - TimeLeftTimer( tmTEST, MSEC );
    //PutEvent( NULL, "Find Sequntial t3=%d msec", t3 );

    DelTimer(tmTEST);
}

#endif

