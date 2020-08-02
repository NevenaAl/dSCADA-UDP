/************************************************************************
*           pvidstr.cpp je modul za rad sa PVID strukturom              *
************************************************************************/

#ifndef PVIDSTR_H
#define PVIDSTR_H

/************************************************************************
*  Funkcija get_pvid_ID vraca pokazivac na PVID strukturu procesne      *
*  promenljive na osnovu imena RTU i njenog imena                            *
************************************************************************/
PVID* get_PVID( char *rtu_name, char* pvar_name, bool hash_find=true );

/************************************************************************
*  Funkcija pid_pok vraca pokazivac na strukturu procesne promenljive,  *
*  ili na njen VAR, TMP ili FIX deo                                     *
************************************************************************/
void *get_pvp( PVID *pvid, int part=ALL_PART );

bool same_pvid( PVID *pvid1, PVID *pvid2 );
char *get_name( PVID  *pvid );
void put_chg( PVID *pvid, int chgtype, int val );
int get_chg( PVID *pvid, int chgtype );


WORD get_status( PVID* pvidp, int part );
long get_timestamp( PVID *pvid );
char *get_status_string( PVID* pvidp );
int put_pvid_status( PVID* pvidp, int part, WORD flag, int val );

PVID *getObjElmID( PVID *objID, int elem_i );
void *getObjElm( PVID *objID, int elem_i );
float getObjPrm( PVID *objID, int prm_i );
int getObjType( PVID *objID );
int getObjClass( PVID *objID );
int getObjCatg( PVID *objID );

SCHAR getDigSts( PVID *pvid );
SCHAR getDigCmd( PVID *pvid );
float getEguVal( PVID *pvid );
int getCmd( char *reqval );
int getSts( char *reqval );


void save_var( PVID *pvid, BYTE *mp );
void save_opr( PVID *pvid, BYTE *mp );

#endif