#include "stdafx.h"

#include "Ioascii.h"

/***********************************************************************/
/*  IOASCII je modul za rad sa konfiguracijom                          */
/*  Sluzi za iscitavanje datoteke i pravljenje RealTime baze podataka  */
/***********************************************************************/

SYS_DESCRIPT  OnSYS;
RTU_DESCRIPT *OnRTU, total;

typedef struct
{
    int   type;
    char *name;
} PAIR_DEF;

KEYWRD TypesDef[] =
{
    "AnaIn",     PVT_AI, 
    "AnaOut",    PVT_AO, 
    "Counter",   PVT_CNT,
    "Digital",   PVT_DIG,
    "Object",    PVT_OBJ,
    "AnyType",   PVT_ANY,
    NULL,        0      
};

int GetTypeID( char* name )
{
    for( int i=0; TypesDef[i].key_word; i++ )
    {
        if( !strcmp( name, TypesDef[i].key_word ) )
        {
            return TypesDef[i].key_code;
        }
    }
    return -1;
}

char* GetTypeName( int tip )
{
    for( int i=0; TypesDef[i].key_word; i++ )
    {
        if( TypesDef[i].key_code == tip )
            return TypesDef[i].key_word;
    }
    return "?????????";
}

// pretrazivanje tabele sa kljucnim recima
int FindKey(char *line, KEYWRD *keyword_table)
{
    char keyword[40] = "";
    sscanf( line, "%s", keyword );
    int i = 0;
    while( keyword_table[i].key_word != NULL  )
    {
        if( !strcmp(keyword, keyword_table[i].key_word) )
            return keyword_table[i].key_code;               // vrati kod asociran kljucnoj reci koju si nasao u tabeli  
        else
            i++;
    }
    return -1;
}

char* FindKeyWord( int key_code, KEYWRD *key_table )
{
    for( int i=0; key_table[i].key_word; i++ )
    {
        if( key_table[i].key_code == key_code )
            return key_table[i].key_word;
    }
    return "?????????";
}


// definicija case strukture za obradu ucitanih CFG izraza
enum s_codes
{
    INVALID_KEYWORD = 0,     
    S_ACQTIM,      S_AIN_NO,      S_ALARM_HIGH,  S_ALARM_LOW,  S_ANAIN,      S_ANAOUT,    S_AOUT_NO,   S_AUTOCLR,
    S_BAUD_RATE,   S_COLOR,       S_CHANEL,      S_CLAMP_HIGH, S_CLAMP_LOW,  S_CLRHOUR,   S_CLRMIN,    S_CMD_LIST,
    S_COMMAND,     S_CMD_NO,      S_CMD_OFF,     S_CNT_DIV,    S_CNT_MUL,    S_CNT_NO,    S_COUNTER,                 
    S_DATA_LEN,    S_DB_EU,       S_DIGIN_NO,    S_DIGITAL,    S_DIGOUT_NO,  S_DIGDEF,    
    S_END,         S_EL_CODE,     S_EL_HIGH,     S_EL_LOW,     S_EGU_CODE,   S_EGU_HIGH,   S_EGU_LOW,   S_EGU,   
    S_HWA,         S_HOST_NAME,   S_IN_LIST,     S_IN_OFF,     S_INFO,
    S_MAX_RATE,    S_NAME,        S_NO_IN,       S_NO_OUT,     S_NO_PARAM,         
    S_OUT_LIST,    S_OUT_OFF,     S_OBJ_CTG,     S_OBJECT,     S_OBJDEF,
    S_PARAM_LIST,  S_PARAM_OFF,   S_PARITY,      S_PORT,       S_PROTOCOL,                
    S_RTU,         S_RTU_LIST,    S_RTU_NAME,    S_RTU_ADR,    S_RAW_HIGH,   S_RAW_LOW,    S_REP_TO,    S_ROUTE,
    S_SYSTEM,      S_STATION,     S_STATE,       S_STOPBITS,   S_STS_LIST,   S_STS_NO,     S_STS_OFF,   S_SYS_NAME,
    S_TIMOUT,      S_TYPE,        S_TCPORT,      S_VIRTUAL,    S_WARN_HIGH,  S_WARN_LOW
};

KEYWRD RegionTable[] =
{
    "OnEnd",    0,
    "SYSTEM",   PVT_SYS,
    "STATION",  PVT_STN,
    "CHANNEL",  PVT_CHA,
    "RTU",      PVT_RTU,
    "ANAIN",    PVT_AI, 
    "ANAOUT",   PVT_AO, 
    "COUNTER",  PVT_CNT,
    "DIGITAL",  PVT_DIG,
    "OBJECT",   PVT_OBJ,
    "UNIT",     CAT_EGU,
    "STATE",    CAT_STS,
    "COMMAND",  CAT_CMD,
    "DIGDEF",   CAT_DIG,
    "OBJDEF",   CAT_OBJ,
    NULL,       0      
};

KEYWRD CaseTable[] =
{
    "INVALID_KEYWORD", INVALID_KEYWORD,
    "S_ACQTIM",    S_ACQTIM,      "S_AIN_NO",   S_AIN_NO,    "S_ALARM_HIGH",S_ALARM_HIGH, "S_ALARM_LOW", S_ALARM_LOW,  "S_ANAIN",    S_ANAIN,
    "S_ANAOUT",    S_ANAOUT,      "S_AOUT_NO",  S_AOUT_NO,   "S_AUTOCLR",   S_AUTOCLR,   
    "S_BAUD_RATE", S_BAUD_RATE,   "S_COLOR",    S_COLOR,     "S_CHANEL",    S_CHANEL,     "S_CLAMP_HIGH",S_CLAMP_HIGH, "S_CLAMP_LOW",S_CLAMP_LOW,
    "S_CLRHOUR",   S_CLRHOUR,     "S_CLRMIN",   S_CLRMIN,    "S_CMD_LIST",  S_CMD_LIST,  
    "S_COMMAND",   S_COMMAND,     "S_CMD_NO",   S_CMD_NO,    "S_CMD_OFF",   S_CMD_OFF,    "S_CNT_DIV",   S_CNT_DIV,    "S_CNT_MUL",  S_CNT_MUL,
    "S_CNT_NO",    S_CNT_NO,      "S_COUNTER",  S_COUNTER,                                
    "S_DATA_LEN",  S_DATA_LEN,    "S_DB_EU",    S_DB_EU,     "S_DIGIN_NO",  S_DIGIN_NO,   "S_DIGITAL",   S_DIGITAL,    "S_DIGOUT_NO",S_DIGOUT_NO,
    "S_DIGDEF",    S_DIGDEF,                       
    "S_END",       S_END,         "S_EL_CODE",  S_EL_CODE,   "S_EL_HIGH",   S_EL_HIGH,    "S_EL_LOW",    S_EL_LOW,     "S_EGU_CODE", S_EGU_CODE,
    "S_EGU_HIGH",  S_EGU_HIGH,    "S_EGU_LOW",  S_EGU_LOW,   "S_EGU",       S_EGU,       
    "S_HWA",       S_HWA,         "S_HOST_NAME",S_HOST_NAME, "S_IN_LIST",   S_IN_LIST,    "S_IN_OFF",    S_IN_OFF,     "S_INFO",     S_INFO,        
    "S_MAX_RATE",  S_MAX_RATE,    "S_NAME",     S_NAME,      "S_NO_IN",     S_NO_IN,      "S_NO_OUT",    S_NO_OUT,     "S_NO_PARAM", S_NO_PARAM,            
    "S_OUT_LIST",  S_OUT_LIST,    "S_OUT_OFF",  S_OUT_OFF,   "S_OBJ_CTG",   S_OBJ_CTG,    "S_OBJECT",    S_OBJECT,     "S_OBJDEF",   S_OBJDEF,      
    "S_PARAM_LIST",S_PARAM_LIST,  "S_PARAM_OFF",S_PARAM_OFF, "S_PARITY",    S_PARITY,     "S_PORT",      S_PORT,       "S_PROTOCOL", S_PROTOCOL,                  
    "S_RTU",       S_RTU,         "S_RTU_LIST", S_RTU_LIST,  "S_RTU_NAME",  S_RTU_NAME,   "S_RTU_ADR",   S_RTU_ADR,    "S_RAW_HIGH", S_RAW_HIGH,
    "S_RAW_LOW",   S_RAW_LOW,     "S_REP_TO",   S_REP_TO,    "S_ROUTE",     S_ROUTE,     
    "S_SYSTEM",    S_SYSTEM,      "S_STATION",  S_STATION,   "S_STATE",     S_STATE,      "S_STOPBITS",  S_STOPBITS,   "S_STS_LIST", S_STS_LIST,
    "S_STS_NO",    S_STS_NO,      "S_STS_OFF",  S_STS_OFF,   "S_SYS_NAME",  S_SYS_NAME,  
    "S_TIMOUT",    S_TIMOUT,      "S_TYPE",     S_TYPE,      "S_TCPORT",    S_TCPORT,     "S_VIRTUAL",   S_VIRTUAL,    "S_WARN_HIGH",S_WARN_HIGH,
    "S_WARN_LOW",  S_WARN_LOW,    
    NULL,          0      
};

static char buffer[256];
static int pass_cnt, line_num;
static short rcode[2];
#define ERR_READLINE -2

void ShowCfgError( void )
{
    // ispisi gresku
    CString Str;
    char* Region = FindKeyWord( rcode[0], RegionTable );
    char* Case = FindKeyWord( rcode[1], CaseTable );
    Str.Format( "Error %d.%d, reading %s, case %s at pass %d!\nLine %d \n%s", rcode[0], rcode[1], Region, Case, pass_cnt, line_num, buffer );
    MessageBox( NULL, Str, "dScada CfgRead Error", MB_ICONERROR | MB_OK);
}

int ReadLineCfg( FILE *fcfg, KEYWRD keywrd[], int ignore_error )
{
    while( true)
    {
        // iscitavanje linije po linije iz datoteke
        if( !fgets (buffer, 256, fcfg) )
            break;
        // uvecaj brojac ucitanih linija
        line_num++;
        // da li je linija prazna
        if( strlen(buffer) < 4 )
            continue;
        // potrazi kljucnu rec
        int key_code = FindKey(buffer, keywrd);
        if( key_code >= 0 )
        {
            rcode[1] = key_code;
            return key_code;
        }
        else
        {
            // prekini ako nisi nasao kljucnu rec
            if( !ignore_error )
                break;
        }
    }
    rcode[1] = INVALID_KEYWORD;
    return ERR_READLINE;
}


int raise_2( int power )
{
    return( power?(2<<(power-1)):1 );
}

int find_string( char** str_array, char* str )
{
    for( int i=0; str_array[i]; i++ )
    {
        if( !strcmp(str_array[i], str) )
            return i;
    }
    return -1;
}

/*--------------------------------------------------------------------------*/
/* Funkcija konvertuje string u short                                       */
/*--------------------------------------------------------------------------*/
int sget_short( char* buff, short *svalue )
{
    int j, ret;
    short i;

    if((j=sscanf (buff, "%*s %hd", &i )) && j!=EOF)
    {
        *svalue = i;
        ret = 0;
    }
    else
        ret = -1;

    return ret;
}

/*--------------------------------------------------------------------------*/
/* Funkcij konvertuje string u long                                        */
/*--------------------------------------------------------------------------*/
int sget_long( char* buff, long *lvalue )
{
    int j, ret;
    long i;

    if((j=sscanf (buff, "%*s %ld", &i )) && j!=EOF)
    {
        *lvalue = i;
        ret = 0;
    }
    else
        ret = -1;

    return ret;
}

/*--------------------------------------------------------------------------*/
/* Funkcija konvertuje string u unsigned long                               */
/*--------------------------------------------------------------------------*/
int sget_ulong( char* buff, unsigned long *ulvalue )
{
    int j, ret;
    unsigned long i;

    if((j=sscanf (buff, "%*s %lu", &i )) && j!=EOF)
    {
        *ulvalue = i;
        ret = 0;
    }
    else
        ret = -1;

    return ret;
}

/*--------------------------------------------------------------------------*/
/* Funkcija konvertuje string u float                                       */
/*--------------------------------------------------------------------------*/
int sget_float( char* buff, float *fvalue )
{
    int j, ret;
    float f;

    if((j=sscanf (buff, "%*s %f", &f )) && j!=EOF)
    {
        *fvalue = f;
        ret = 0;
    }
    else
        ret = -1;

    return ret;
}

/*--------------------------------------------------------------------------*/
/* Funkcija konvertuje string u jedno od stanja ON ili OFF                  */
/*--------------------------------------------------------------------------*/
int sget_on_off( char* buff, short* state )
{
    int  j, ret;
    char tmpbuf [40];

    if((j=sscanf (buff, "%*s %s", tmpbuf))&&j!=EOF)
    {
        if( !strcmp(_strupr(tmpbuf), "ON") )
        {
            *state = 1;
            ret = 0;
        }
        else if( !strcmp(_strupr(tmpbuf), "OFF") )
        {
            *state = 0;
            ret = 0;
        }
        else
            ret = -1;
    }
    else
        ret = -1;

    return ret;
}

/*--------------------------------------------------------------------------*/
/* Kopira prvih len karaktera iz prvog u drugi string                       */
/*--------------------------------------------------------------------------*/
int sget_alltext( char* buff, char* text, int len )
{
    int  j, ret;
    char tmpbuf [128]="";

    if((j=sscanf (buff, "%*s %[^\v\f\n\r]", tmpbuf)) && j!=EOF)
    {
        strncpy( text, tmpbuf, len );
        text[len-1] = 0;
        while( text[j=(strlen(text)-1)] == ' ' ) text[j]=0;
        ret = 0;
    }
    else
        ret = -1;

    return ret;
}

int sget_string( char* buff, char* text, int len )
{
    int  j, ret;
    char tmpbuf [128]="";

    if((j=sscanf (buff, "%*s %s", tmpbuf)) && j!=EOF)
    {
        strncpy( text, tmpbuf, len );
        text[len-1] = 0;
        while( text[j=(strlen(text)-1)] == ' ' ) text[j]=0;
        ret = 0;
    }
    else
        ret = -1;

    return ret;
}


/*--------------------------------------------------------------------------*/
/* Ucitava deo konfiguracione datoteke koja pocinje kljucnom reci [SYSTEM]  */
/* a zavrsava se sa [END] u strukturu SYSTEM                                */
/*--------------------------------------------------------------------------*/
int in_system( FILE* file, int pass )
{
    int ret = 0, repeat = true;

    if( pass == 0 )
    {
        KEYWRD sys_keywrd[] =
        {
            "SysName",      S_SYS_NAME,
            "[STOP]",       S_END,
            NULL,           0
        };

        while( repeat )
        {
            switch( ReadLineCfg(file, sys_keywrd, 0) )
            {
            case ERR_READLINE:
                return -1;
            case S_SYS_NAME:
                ret = sget_alltext( buffer, rtdb.sys_name, sizeof(rtdb.sys_name) );
                break;
            case S_END:
                repeat = false;
                break;
            default:
                break;
            }
            if( ret )
                repeat = false;
        }
    }

    return ret;
}

/*--------------------------------------------------------------------------*/
/* Ucitava deo konfiguracione datoteke koja pocinje kljucnom reci [EGU]*/
/* a zavrsava se sa [END] u katalog jedinica                */
/*--------------------------------------------------------------------------*/
int in_egunit( FILE* file, int pass )
{
    int ret = 0, repeat = true;

    if( pass == 0 )
    {
        // samo povecaj broj jedinica u OnSYS
        OnSYS.egu++;
    }
    else if( pass == 1 )
    {
        char  tvalue[40];
        KEYWRD egu_keywrd[] =
        {
            "Unit",     S_NAME,
            "Color",    S_COLOR,
            "[STOP]",   S_END,
            NULL,       0
        };

        EguDef* egu_p = rtdb.EGUnit + OnSYS.egu++;

        while( repeat )
        {
            switch( ReadLineCfg(file, egu_keywrd, 0) )
            {
            case ERR_READLINE:
                return -1;
            case S_NAME:
                ret = sget_string( buffer, egu_p->egu, sizeof(egu_p->egu));
                break;
            case S_COLOR:
                if( (ret = sget_string(buffer, tvalue, sizeof(tvalue))) == -1 )
                    break;
                if( (ret = find_string(ColorNames, tvalue)) != -1 )
                {
                    egu_p->color = ret;
                    ret = 0;
                }
                break;
            case S_END:
                repeat = false;
                break;
            default:
                break;
            }
            if( ret )
                repeat = false;
        }
    }

    return ret;
}

/*--------------------------------------------------------------------------*/
/* Ucitava deo konfiguracione datoteke koja pocinje kljucnom reci [STANJA]  */
/* a zavrsava se sa [END] u globalnu strukturu rtdb.DigState                       */
/*--------------------------------------------------------------------------*/
int in_state( FILE* file, int pass )
{
    int ret = 0, repeat = true;

    if( pass == 0 )
    {
        // samo povecaj broj stanja u OnSYS
        OnSYS.sts++;
    }
    else if( pass == 1 )
    {
        char  tvalue[40];
        KEYWRD sts_keywrd[] =
        {
            "State",    S_NAME,
            "Color",    S_COLOR,
            "[STOP]",   S_END,
            NULL,       0
        };

        StateDef* stadef_p = rtdb.DigState + OnSYS.sts++;

        while( repeat )
        {
            switch( ReadLineCfg(file, sts_keywrd, 0) )
            {
            case ERR_READLINE:
                return -1;
            case S_NAME:
                ret = sget_string( buffer, stadef_p->state, sizeof(stadef_p->state) );
                break;
            case S_COLOR:
                if( (ret = sget_string(buffer, tvalue, sizeof(tvalue))) ==-1 )
                    break;
                if( (ret = find_string(ColorNames, tvalue)) != -1 )
                {
                    stadef_p->color = ret;
                    ret = 0;
                }
                break;
            case S_END:
                repeat = false;
                break;
            default:
                break;
            }
            if( ret )
                repeat = false;
        }
    }

    return ret;
}


/*--------------------------------------------------------------------------*/
/* Ucitava deo konfiguracione datoteke koja pocinje kljucnom reci [KOMANDE] */
/* a zavrsava se sa [END] u globalnu strukturu rtdb.DigCmd                      */
/*--------------------------------------------------------------------------*/
int in_command( FILE* file, int pass )
{
    int ret = 0, repeat = true;

    if( pass == 0 )
    {
        // samo povecaj broj stanja u OnSYS
        OnSYS.cmd++;
    }
    else if( pass == 1 )
    {
        char  tvalue[40];
        KEYWRD cmd_keywrd[] =
        {
            "Command",  S_NAME,
            "Color",    S_COLOR,
            "[STOP]",   S_END,
            NULL,       0
        };

        CommandDef* cmd_p = rtdb.DigCmd + OnSYS.cmd++;

        while( repeat )
        {
            switch( ReadLineCfg(file, cmd_keywrd, 0) )
            {
            case ERR_READLINE:
                return -1;
            case S_NAME:
                ret = sget_string( buffer, cmd_p->command, sizeof(cmd_p->command) );
                break;
            case S_COLOR:
                if( (ret = sget_string(buffer, tvalue, sizeof(tvalue))) == -1 )
                    break;
                if( (ret = find_string(ColorNames, tvalue)) != -1 )
                {
                    cmd_p->color = ret;
                    ret = 0;
                }
                break;
            case S_END:
                repeat = false;
                break;
            default:
                break;
            }
            if( ret )
                repeat = false;
        }
    }

    return ret;
}

/*--------------------------------------------------------------------------*/
/* Ucitava deo konfiguracione datoteke koja pocinje kljucnom reci [TIPOVI]  */
/* a zavrsava se sa [END] u globalnu strukturu DigDev                       */
/*--------------------------------------------------------------------------*/
int in_digdef( FILE* file, int pass )
{
    int ret = 0, repeat = true;

    if( pass == 0 )
    {
        // samo povecaj broj TipDig u OnSYS
        OnSYS.digd++;
    }
    else if( pass == 1 )
    {
        int   i, sts_no, cmd_no, sts_off, cmd_off;
        char  tvalue[40];
        KEYWRD tip_keywrd[] =
        {
            "Type",     S_NAME,
            "NoDigIn",  S_NO_IN,
            "NoDigOut", S_NO_OUT,
            "StsList",  S_STS_LIST,
            "State",    S_STS_OFF,
            "CmdList",  S_CMD_LIST,
            "Command",  S_CMD_OFF,
            "[STOP]",   S_END,
            NULL,       0
        };

        DigDevDef* ddef_p = rtdb.DigDev + OnSYS.digd++;

        while( repeat )
        {
            switch( ReadLineCfg(file, tip_keywrd, 0) )
            {
            case ERR_READLINE:
                return -1;
            case S_NAME:
                ret = sget_string( buffer, ddef_p->type, sizeof(ddef_p->type));
                break;
            case S_NO_IN:
                ret = sget_short( buffer, &ddef_p->noIn );
                break;
            case S_NO_OUT:
                ret = sget_short( buffer, &ddef_p->noOut );
                break;
            case S_STS_LIST:
                sts_off = 0;
                sts_no = raise_2( ddef_p->noIn );
                break;
            case S_CMD_LIST:
                cmd_off = 0;
                cmd_no = raise_2( ddef_p->noOut );
                break;
            case S_STS_OFF:
                if( sts_off < sts_no )
                {
                    ret = -1;
                    if( sscanf(buffer, "%*s %s", tvalue) == 1 )
                    {
                        for( i=0; i < rtdb.no_state; i++ )
                        {
                            if( !strcmp( tvalue, rtdb.DigState[i].state ) )
                            {
                                ddef_p->states[sts_off++] = i;
                                ret = 0;
                                break;
                            }
                        }
                    }
                }
                break;
            case S_CMD_OFF:
                if( cmd_off < cmd_no )
                {
                    ret = -1;
                    if( sscanf(buffer, "%*s %s ", tvalue) == 1 )
                    {
                        for( i=0; i<rtdb.no_command; i++ )
                        {
                            if( !strcmp( tvalue, rtdb.DigCmd[i].command ) )
                            {
                                ddef_p->commands[cmd_off++] = i;
                                ret = 0;
                                break;
                            }
                        }
                    }
                }
                break;
            case S_END:
                repeat = false;
                break;
            default:
                break;
            }
            if( ret ) 
                repeat = false;
        }
    }

    return ret;
}


int in_objdef( FILE* file, int pass )
{
    int ret = 0, repeat = true;

    if( pass == 0 )
    {
        // samo povecaj broj tipova objekata u OnSYS
        OnSYS.objd++;
    }
    else if( pass == 1 )
    {
        short svalue;
        int   i, elem_off, param_off;
        char  tvalue[40], tvalue1[40];
        char* klasa[] = { "DigObj", "AnaObj", NULL };
        char* catgry[] = { "Real", "Virtual", NULL };
        KEYWRD objdef_keywrd[] =
        {
            "Type",        S_NAME,
            "Class",       S_TYPE,
            "Category",    S_OBJ_CTG,
            "NoElem",      S_STS_NO,
            "ElemList",    S_STS_LIST,
            "Element",     S_STS_OFF,
            "NoParam",     S_NO_PARAM,
            "ParamList",   S_PARAM_LIST,
            "Parameter",   S_PARAM_OFF,
            "[STOP]",      S_END,
            NULL,          0
        };

        ObjectDef* objdef_p = rtdb.ObjDev+ OnSYS.objd++;

        while( repeat )
        {
            switch( ReadLineCfg(file, objdef_keywrd, 0) )
            {
            case ERR_READLINE:
                return -1;
            case S_NAME:
                ret = sget_string( buffer, objdef_p->type, sizeof(objdef_p->type) );
                break;
            case S_TYPE:
                if( (ret = sget_string( buffer, tvalue, sizeof(tvalue)) ) == -1 )
                    break;
                if( (ret = find_string(klasa, tvalue)) != -1 )
                {
                    objdef_p->klasa = (SCHAR) ret;
                    ret = 0;
                }
                break;
            case S_OBJ_CTG:
                if( (ret = sget_string( buffer, tvalue, sizeof(tvalue)) ) == -1 )
                    break;
                if( (ret = find_string(catgry, tvalue)) != -1 )
                {
                    objdef_p->category = (SCHAR) ret;
                    ret = 0;
                }
                break;
            case S_STS_NO:
                if( !(ret = sget_short(buffer, &svalue)) )
                {
                    if( svalue <= MAX_ELEM )
                        objdef_p->noElems = svalue;
                    else
                        ret = -1;
                }
                break;
            case S_STS_LIST:
                elem_off = 0;
                break;
            case S_STS_OFF:
                if( elem_off < objdef_p->noElems )
                {
                    ret = -1;
                    if( sscanf(buffer, "%*s %s %*s %s", &tvalue, &tvalue1) == 2 )
                    {
                        strcpy( objdef_p->elementDescription[elem_off], tvalue );
                        if( (i = GetTypeID(tvalue1)) >= 0 )
                        {
                            objdef_p->elementType[elem_off] = i;
                            elem_off++;
                            ret = 0;
                        }
                    }
                }
                break;
            case S_NO_PARAM:
                if( !(ret = sget_short(buffer, &svalue)) )
                {
                    if( svalue <= MAX_PARAM )
                        objdef_p->noParams = svalue;
                    else
                        ret = -1;
                }
                break;
            case S_PARAM_LIST:
                param_off = 0;
                break;
            case S_PARAM_OFF:
                if( param_off < objdef_p->noParams )
                {
                    ret = -1;
                    if( sscanf(buffer, "%*s %s", &tvalue) == 1 )
                    {
                        strcpy( objdef_p->parameterDescription[param_off], tvalue );
                        param_off++;
                        ret = 0;
                    }
                }
                break;
            case S_END:
                repeat = false;
                break;
            default:
                break;
            }
            if( ret )
                repeat = false;
        }
    }

    return ret;
}

static char* dataType[] = { "TANDEM", "REPBUS", "MODBUS", "USERTYPE2", "USERTYPE3", 
    "USERTYPE4", "USERTYPE5", "USERTYPE6", "USERTYPE7", "USERTYPE8", NULL };
static char* baudrate[] = { "110", "150", "300", "600", "1200", "2400", "4800", "9600", NULL };
static char* parity[] = { "None", "Odd", "Even", NULL};
static char* ipside[] = { "Client", "Server", NULL };
static char* port[] = { "COM", "UDP", "TCP", NULL };

int in_channel( FILE* file, int pass )
{
    int ret = 0, repeat = true; 

    if( pass == 0 )
    {
        OnSYS.chn++;            // prebroj kanale
    }
    else if( pass == 1 )
    {
        short svalue;
        char  tvalue[40];
        KEYWRD chan_keywrd[] =
        {
            "Protocol",    S_PROTOCOL,
            "TimeOut",     S_TIMOUT,
            "Port",        S_PORT,
            "ipSide",      S_TYPE,
            "ipPort",      S_TCPORT,
            "BaudRate",    S_BAUD_RATE,
            "DataLen",     S_DATA_LEN,
            "StopBits",    S_STOPBITS,
            "Parity",      S_PARITY,
            "Name",        S_NAME,
            "Info",        S_INFO,
            "[STOP]",      S_END,
            NULL,          0
        };

        CHANET* chan_p = rtdb.chanet + OnSYS.chn++;

        while( repeat )
        {
            switch( ReadLineCfg(file, chan_keywrd, 0) )
            {
            case ERR_READLINE:
                return -1;
            case S_PORT:
                if( (ret=sget_string( buffer, tvalue, sizeof(tvalue)) ) == -1 )
                    break;
                svalue = atoi( tvalue + 3 );
                if( (svalue < 1) || (svalue > 8) )
                {
                    ret = -1;
                }
                else
                {
                    // namesti indeks porta
                    chan_p->fix.port = (SCHAR) (svalue - 1);
                    // skrati string i nadji type
                    tvalue[3] = 0;
                    if( (ret = find_string(port, tvalue)) != -1 )
                    {
                        chan_p->fix.type = (SCHAR)ret;
                        ret = 0;
                    }
                }
                break;
            case S_TCPORT:
                if(!(ret = sget_short(buffer, &svalue)))
                    chan_p->fix.comm.ip.locport = svalue;
                break;
            case S_TYPE:                                 // Klijent ili Server
                if( (ret = sget_string(buffer, tvalue, sizeof(tvalue))) == -1 )
                    break;
                if( (ret = find_string(ipside, tvalue)) != -1 )
                {
                    chan_p->fix.comm.ip.server = (ret)?true:false;
                    ret = 0;
                }
                break;
            case S_BAUD_RATE:
                if( (ret = sget_string(buffer, tvalue, sizeof(tvalue))) == -1 )
                    break;
                if( (ret = find_string(baudrate, tvalue)) != -1 )
                {
                    //chan_p->fix.comm.uart.baud = (SCHAR)ret;
                    chan_p->fix.comm.uart.baud = (short) atoi(tvalue);
                    ret = 0;
                }
                break;
            case S_PARITY:
                if( (ret = sget_string(buffer, tvalue, sizeof(tvalue))) == -1 )
                    break;
                if( (ret = find_string(parity, tvalue)) != -1 )
                {
                    chan_p->fix.comm.uart.parity = (SCHAR)ret;
                    ret = 0;
                }
                break;
            case S_STOPBITS:
                if( !(ret = sget_short(buffer, &svalue)) )
                    chan_p->fix.comm.uart.stop_bits = (SCHAR)(svalue-1);
                break;
            case S_DATA_LEN:
                if( !(ret = sget_short(buffer, &svalue)) )
                    chan_p->fix.comm.uart.lenght = (SCHAR)svalue;
                    //chan_p->fix.comm.uart.lenght = 8;
                break;
            case S_TIMOUT:
                if( !(ret = sget_short(buffer, &svalue)) )
                    chan_p->fix.timout = svalue;
                break;
            case S_NAME:
                ret = sget_string( buffer, chan_p->fix.name, sizeof(chan_p->fix.name) );
                break;
            case S_INFO:
                ret = sget_alltext( buffer, chan_p->fix.info, sizeof(chan_p->fix.info) );
                break;
            case S_PROTOCOL:
                if( (ret = sget_string(buffer, tvalue, sizeof(tvalue))) == -1 )
                    break;
                if( (ret = find_string(dataType, tvalue)) != -1 )
                {
                    // zapamti tip protokola 
                    chan_p->fix.protocol = ret;
                    ret = 0;
                }
                break;
            case S_END:
                repeat = false;
                chan_p->var.val = 1;
                put_bit(chan_p->opr.status, OPR_ACT, 1);
                break;
            default:
                break;
            }
            if( ret ) repeat = false;
        }
    }

    return ret;
}

/*--------------------------------------------------------------------------*/
/* Ucitava deo konfiguracione datoteke koja pocinje kljucnom reci [RTU]     */
/* a zavrsava se sa [END] u globalnu strukturu rtdb.rtut, koja  povezuje    */
/* sve procesne velicine                                                    */
/*--------------------------------------------------------------------------*/
int in_rtu( FILE* file, int pass )
{
    int ret = 0, repeat = true;

    if( (pass == 0) || (pass == 3) )
    {
        OnSYS.rtu++;            // prebroj RTU kontrolere
        return ret;
    }

    short svalue;
    int   i;
    char  tvalue[40];
    KEYWRD rtu_keywrd[] =
    {
        "Virtual",    S_VIRTUAL,
        "Channel",    S_CHANEL,
        "CommAddr",   S_RTU_ADR,
        "Period",     S_ACQTIM,
        "NoAin",      S_AIN_NO,
        "NoAout",     S_AOUT_NO,
        "NoDin",      S_DIGIN_NO,
        "NoDout",     S_DIGOUT_NO,
        "NoCnt",      S_CNT_NO,
        "Name",       S_NAME,
        "Info",       S_INFO,
        "HostName",   S_HOST_NAME,
        "HostPort",   S_TCPORT,
        "[STOP]",     S_END,
        NULL,         0
    };

    if( OnSYS.rtu >= rtdb.no_rtu )
        return -1;

    RTUT* rtu_p = rtdb.rtut + OnSYS.rtu++;

    while( repeat )
    {
        if( pass == 1 )
        {                                                  // drugi prolaz
            switch( ReadLineCfg(file, rtu_keywrd, 0) )
            {
            case ERR_READLINE:
                return -1;
            case S_VIRTUAL:
                if( !(ret = sget_on_off(buffer, &svalue)) )
                    rtu_p->var.virt = svalue;
                break;
            case S_RTU_ADR:
                if( !(ret = sget_short(buffer, &svalue)) )
                    rtu_p->fix.com_adr = (BYTE) svalue;
                break;
            case S_ACQTIM:
                if( !(ret = sget_short(buffer, &svalue)) )
                    rtu_p->opr.acq_period = (BYTE) svalue;
                break;
            case S_DIGIN_NO:
                ret = sget_short( buffer, &(rtu_p->fix.no_din) );
                break;
            case S_DIGOUT_NO:
                ret = sget_short( buffer, &(rtu_p->fix.no_dout) );
                break;
            case S_AIN_NO:
                ret = sget_short( buffer, &(rtu_p->fix.no_ain) );
                break;
            case S_AOUT_NO:
                ret = sget_short( buffer, &(rtu_p->fix.no_aout) );
                break;
            case S_CNT_NO:
                ret = sget_short( buffer, &(rtu_p->fix.no_cnt) );
                break;
            case S_NAME:
                ret = sget_string( buffer, rtu_p->fix.name, sizeof(rtu_p->fix.name) );
                break;
            case S_INFO:
                ret = sget_alltext( buffer, rtu_p->fix.info, sizeof(rtu_p->fix.info) );
                break;
            case S_HOST_NAME:
                ret = sget_string( buffer, rtu_p->fix.hname, sizeof(rtu_p->fix.hname) );
                break;
            case S_TCPORT:
                if( !(ret = sget_short(buffer, &svalue)) )
                    rtu_p->fix.hport = svalue;
                break;
            case S_END:
                repeat = false;
                // pocetna inicijalizacija
                rtu_p->var.first_scan = 1;
                //rtu_p->var.val = 1;
                //rtu_p->var.err = 1;
                put_bit( rtu_p->opr.status, OPR_ACT, 1 );
                break;
            default:
                break;
            }
        }
        else if( pass == 2 )
        {                                         // treci prolaz
            switch( ReadLineCfg(file, rtu_keywrd, 0) )
            {
            case ERR_READLINE:
                return -1;
            case S_CHANEL:
                if( (ret = sget_string(buffer, tvalue, sizeof(tvalue))) == -1 )
                    break;
                for( i=0; rtdb.no_channel; i++ )
                {
                    if( !strcmp( tvalue, rtdb.chanet[i].fix.name ) )
                    {
                        // nasao si svoj kanal
                        rtu_p->fix.channel = (BYTE) i;
                        // upisi se kod njega
                        CHANET *chp = rtdb.chanet + i;
                        chp->fix.rtu_lst[chp->fix.rtu_n++] = rtu_p->fix.mID.sqn;
                        break;
                    }
                    if( i == rtdb.no_channel )
                        ret = -1;
                }
                break;
            case S_END:
                repeat = false;
                break;
            default:
                break;
            }
        }
        if( ret ) 
            repeat = false;
    }

    return ret;
}


/*--------------------------------------------------------------------------*/
/* Ucitava deo konfiguracione datoteke koja pocinje kljucnom reci [ANAIN]   */
/* a zavrsava se sa [END] u strukturu rtdb.rtut                                 */
/*--------------------------------------------------------------------------*/
int in_anain( FILE* file, int pass )
{
    if( (pass == 0) || (pass == 3) )    // u prvom i zadnjem prolazu ne radis nista
        return 0;

    int   i, ret = 0, repeat = true;
    char  tvalue[40];
    KEYWRD ain_keywrd[] =
    {
        "EGUnit",      S_EGU_CODE,
        "EguBandLow",  S_EGU_LOW,
        "EguBandHigh", S_EGU_HIGH,
        "AlarmLow",    S_ALARM_LOW,
        "AlarmHigh",   S_ALARM_HIGH,
        "WarnLow",     S_WARN_LOW,
        "WarnHigh",    S_WARN_HIGH,
        "DeadBand",    S_DB_EU,
        "ElUnit",      S_EL_CODE,
        "ElBandLow",   S_EL_LOW,
        "ElBandHigh",  S_EL_HIGH,
        "RawBandLow",  S_RAW_LOW,
        "RawBandHigh", S_RAW_HIGH,
        "hwAddr",      S_HWA,
        "RTU",         S_RTU_NAME,
        "Name",        S_NAME,
        "Info",        S_INFO,
        "[STOP]",      S_END,
        NULL,          0
    };

    ANAIN ain;
    memset( &ain, 0, sizeof(ANAIN) );

    while( repeat )
    {
        if( pass == 1 )
        {
            switch( ReadLineCfg(file, ain_keywrd, 0) )
            {
            case ERR_READLINE:
                return -1;
            case S_RTU_NAME:
                if( (ret = sget_string( buffer, tvalue, NAME_LEN )) == -1 )
                    break;
                for( i=0; i<rtdb.no_rtu; i++ )
                {
                    if( !strcmp( rtdb.rtut[i].fix.name, tvalue ) )
                    {
                        OnRTU[i].ain++;
                        break;
                    }
                }
                if( i == rtdb.no_rtu )
                    ret = -1;
                else
                    ret = 0;
                break;
            case S_END:
                repeat = false;
                break;
            default:
                break;
            }
        }
        else if( pass == 2 )
        {
            switch( ReadLineCfg(file, ain_keywrd, 0) )
            {
            case ERR_READLINE:
                return -1;
            case S_ALARM_LOW:
                ret = sget_float( buffer, &(ain.opr.alarm_lim[0]) );
                break;
            case S_ALARM_HIGH:
                ret = sget_float( buffer, &(ain.opr.alarm_lim[1]) );
                break;
            case S_WARN_LOW:
                ret = sget_float( buffer, &(ain.opr.warn_lim[0]) );
                break;
            case S_WARN_HIGH:
                ret = sget_float( buffer, &(ain.opr.warn_lim[1]) );
                break;
            case S_HWA:
                ret = sget_short( buffer, &(ain.fix.hwa) );
                break;
            case S_EGU_LOW:
                ret = sget_float( buffer, &(ain.opr.egu_range[0]) );
                break;
            case S_EGU_HIGH:
                ret = sget_float( buffer, &(ain.opr.egu_range[1]) );
                break;
            case S_EL_LOW:
                ret = sget_float( buffer, &(ain.fix.el_range[0]) );
                break;
            case S_EL_HIGH:
                ret = sget_float( buffer, &(ain.fix.el_range[1]) );
                break;
            case S_RAW_LOW:
                ret = sget_short( buffer, (short*)&(ain.fix.raw_range[0]) );
                break;
            case S_RAW_HIGH:
                ret = sget_short( buffer, (short*)&(ain.fix.raw_range[1]) );
                break;
            case S_DB_EU:
                ret = sget_float( buffer, &(ain.opr.deadband) );
                break;
            case S_EGU_CODE:
                ret = -1;
                if( !sget_string(buffer, tvalue, NAME_LEN) )
                {
                    for( i=0; i<rtdb.no_egu; i++ )
                    {
                        if( !strcmp(rtdb.EGUnit[i].egu, tvalue) )
                        {
                            ain.fix.egu_code = i;
                            ret = 0;
                            break;
                        }
                    }
                }
                break;
            case S_EL_CODE:
                ret = -1;
                if( !sget_string(buffer, tvalue, NAME_LEN) )
                {
                    for( i=0; i<rtdb.no_egu; i++ )
                    {
                        if( !strcmp(rtdb.EGUnit[i].egu, tvalue) )
                        {
                            ain.fix.el_code = i;
                            ret = 0;
                            break;
                        }
                    }
                }
                break;
            case S_RTU_NAME:
                ret = -1;
                if( !sget_string(buffer, tvalue, NAME_LEN) )
                {
                    for( i=0; i<rtdb.no_rtu; i++ )
                    {
                        if( !strcmp(rtdb.rtut[i].fix.name, tvalue) )
                        {
                            ain.fix.mID.type = PVT_AI;
                            ain.fix.mID.rtu = i;
                            ain.fix.mID.sqn = OnRTU[i].ain++;
                            ret = 0;
                            break;
                        }
                    }
                }
                break;
            case S_NAME:
                ret = sget_string( buffer, ain.fix.name, sizeof(ain.fix.name) );
                break;
            case S_INFO:
                ret = sget_alltext( buffer, ain.fix.info, sizeof(ain.fix.info) );
                break;
            case S_END:
                // pocetna inicijalizacija
                ain.var.status = 0;
                put_bit( ain.var.status, AI_VAL, 1 );
                put_bit( ain.var.status, AI_SCAN1, 1 );
                put_bit( ain.opr.status, OPR_ACT, 1 );
                // kopiraj gde treba
                memcpy( &(rtdb.rtut[ain.fix.mID.rtu].fix.ainfut.ain_t[ain.fix.mID.sqn]), &ain, sizeof(ANAIN) );
                repeat = false;
                break;
            default:
                break;
            }
        }
        if( ret )
            repeat = false;
    }
    return ret;
}


/*--------------------------------------------------------------------------*/
/* Ucitava deo konfiguracione datoteke koja pocinje kljucnom reci [ANAOUT]  */
/* a zavrsava se sa [END] u strukturu rtdb.rtut                                 */
/*--------------------------------------------------------------------------*/
int in_anaout( FILE* file, int pass )
{
    if( (pass == 0) || (pass == 3) )                     // u prvom i zadnjem prolazu ne radis nista
        return 0;

    int   i, ret = 0, repeat = true;
    char  tvalue[40];
    KEYWRD aout_keywrd[] =
    {
        "EGUnit",         S_EGU_CODE,
        "EguBandLow",     S_EGU_LOW,
        "EguBandHigh",    S_EGU_HIGH,
        "AlarmLow",       S_ALARM_LOW,
        "AlarmHigh",      S_ALARM_HIGH,
        "ClampBandLow",   S_CLAMP_LOW,
        "ClampBandHigh",  S_CLAMP_HIGH,
        "MaxRate",        S_MAX_RATE,
        "DeadBand",       S_DB_EU,
        "ElUnit",         S_EL_CODE,
        "ElBandLow",      S_EL_LOW,
        "ElBandHigh",     S_EL_HIGH,
        "RawBandLow",     S_RAW_LOW,
        "RawBandHigh",    S_RAW_HIGH,
        "hwAddr",         S_HWA,
        "RTU",            S_RTU_NAME,
        "Name",           S_NAME,
        "Info",           S_INFO,
        "[STOP]",         S_END,
        NULL,             0
    };

    ANAOUT aout;
    memset( &aout, 0, sizeof(ANAOUT) );

    while( repeat )
    {
        if( pass == 1 )
        {
            switch( ReadLineCfg(file, aout_keywrd, 0) )
            {
            case ERR_READLINE:
                return -1;
            case S_RTU_NAME:
                if( (ret = sget_string(buffer, tvalue, NAME_LEN)) == -1 )
                    break;
                for( i=0; i<rtdb.no_rtu; i++ )
                {
                    if( !strcmp( rtdb.rtut[i].fix.name, tvalue ) )
                    {
                        OnRTU[i].aout++;
                        break;
                    }
                }
                if( i == rtdb.no_rtu )
                    ret = -1;
                break;
            case S_END:
                repeat = false;
                break;
            default:
                break;
            }
        }
        else if( pass == 2 )
        {
            switch( ReadLineCfg(file, aout_keywrd, 0) )
            {
            case ERR_READLINE:
                return -1;
            case S_ALARM_LOW:
                ret = sget_float( buffer, &(aout.opr.alarm_lim[0]) );
                break;
            case S_ALARM_HIGH:
                ret = sget_float( buffer, &(aout.opr.alarm_lim[1]) );
                break;
            case S_HWA:
                ret = sget_short( buffer, &(aout.fix.hwa) );
                break;
            case S_EGU_LOW:
                ret = sget_float( buffer, &(aout.opr.egu_range[0]) );
                break;
            case S_EGU_HIGH:
                ret = sget_float( buffer, &(aout.opr.egu_range[1]) );
                break;
            case S_EL_LOW:
                ret = sget_float( buffer, &(aout.fix.el_range[0]) );
                break;
            case S_EL_HIGH:
                ret = sget_float( buffer, &(aout.fix.el_range[1]) );
                break;
            case S_RAW_LOW:
                ret = sget_short( buffer, (short*)&(aout.fix.raw_range[0]) );
                break;
            case S_RAW_HIGH:
                ret = sget_short( buffer, (short*)&(aout.fix.raw_range[1]) );
                break;
            case S_CLAMP_LOW:
                ret = sget_float( buffer, &(aout.opr.clamp_lim[0]) );
                break;
            case S_CLAMP_HIGH:
                ret = sget_float( buffer, &(aout.opr.clamp_lim[1]) );
                break;
            case S_MAX_RATE:
                ret = sget_float( buffer, &(aout.opr.max_rate) );
                break;
            case S_DB_EU:
                ret = sget_float( buffer, &(aout.opr.deadband) );
                break;
            case S_EGU_CODE:
                ret = -1;
                if( !sget_string(buffer, tvalue, NAME_LEN) )
                {
                    for( i=0; i<rtdb.no_egu; i++ )
                    {
                        if( !strcmp( rtdb.EGUnit[i].egu, tvalue ) )
                        {
                            aout.fix.egu_code = i;
                            ret = 0;
                            break;
                        }
                    }
                }
                break;
            case S_EL_CODE:
                ret = -1;
                if( !sget_string(buffer, tvalue, NAME_LEN) )
                {
                    for( i=0; i<rtdb.no_egu; i++ )
                    {
                        if( !strcmp(rtdb.EGUnit[i].egu, tvalue) )
                        {
                            aout.fix.el_code = i;
                            ret = 0;
                            break;
                        }
                    }
                }
                break;
            case S_RTU_NAME:
                ret = -1;
                if( !sget_string(buffer, tvalue, NAME_LEN) )
                {
                    for( i=0; i<rtdb.no_rtu; i++ )
                    {
                        if( !strcmp(rtdb.rtut[i].fix.name, tvalue) )
                        {
                            aout.fix.mID.type = PVT_AO;
                            aout.fix.mID.rtu = i;
                            aout.fix.mID.sqn = OnRTU[i].aout++;
                            ret = 0;
                            break;
                        }
                    }
                }
                break;
            case S_NAME:
                ret = sget_string( buffer, aout.fix.name, sizeof(aout.fix.name) );
                break;
            case S_INFO:
                ret = sget_alltext( buffer, aout.fix.info, sizeof(aout.fix.info) );
                break;
            case S_END:
                // pocetna inicijalizacija
                aout.var.status = 0;
                put_bit( aout.var.status, AO_VAL, 1 );
                put_bit( aout.var.status, AO_SCAN1, 1 );
                put_bit( aout.opr.status, OPR_ACT, 1 );
                // kopiraj gde treba
                memcpy( &(rtdb.rtut[aout.fix.mID.rtu].fix.aoutfut.aout_t[aout.fix.mID.sqn]), &aout, sizeof(ANAOUT) );
                repeat = false;
                break;
            default:
                break;
            }
        }
        if( ret )
            repeat = false;
    }
    return ret;
}


/*--------------------------------------------------------------------------*/
/* Ucitava deo konfiguracione datoteke koja pocinje kljucnom reci [COUNTER] */
/* a zavrsava se sa [END] u strukturu rtdb.rtut                                 */
/*--------------------------------------------------------------------------*/
int in_counter( FILE* file, int pass )
{
    if( (pass == 0) || (pass == 3) )                     // u prvom i zadnjem prolazu ne radis nista
        return 0;

    int   i, ret = 0, repeat = true;
    short svalue;
    char  tvalue[40];

    KEYWRD cnt_keywrd[] =
    {
        "EGUnit",         S_EGU_CODE,
        "MulConst",       S_CNT_MUL,
        "DivConst",       S_CNT_DIV,
        "DeadBand",       S_DB_EU,
        "WatchPeriod",    S_TIMOUT,
        "AutoClear",      S_AUTOCLR,
        "ClearHour",      S_CLRHOUR,
        "ClearMinute",    S_CLRMIN,
        "hwAddr",         S_HWA,
        "RTU",            S_RTU_NAME,
        "Name",           S_NAME,
        "Info",           S_INFO,
        "[STOP]",         S_END,
        NULL,             0
    };

    COUNTER cnt;
    memset( &cnt, 0, sizeof(COUNTER) );

    while( repeat )
    {
        if( pass == 1 )
        {
            switch( ReadLineCfg(file, cnt_keywrd, 0) )
            {
            case ERR_READLINE:
                return -1;
            case S_RTU_NAME:
                ret = -1;
                if( !sget_string(buffer, tvalue, NAME_LEN) )
                {
                    for( i=0; i<rtdb.no_rtu; i++ )
                    {
                        if( !strcmp(rtdb.rtut[i].fix.name, tvalue) )
                        {
                            OnRTU[i].cnt++;
                            ret = 0; 
                            break;
                        }
                    }
                }
                break;
            case S_END:
                repeat = false;
                break;
            default:
                break;
            }
        }
        else if( pass == 2 )
        {
            switch( ReadLineCfg(file, cnt_keywrd, 0) )
            {
            case ERR_READLINE:
                return -1;
            case S_AUTOCLR:
                if( !(ret = sget_on_off(buffer, &svalue)) )
                    put_bit( cnt.opr.status, OPR_AUTOCLR, svalue );
                break;
            case S_CLRHOUR:
                if( !(ret = sget_short(buffer, &svalue)) )
                    cnt.opr.clrhour = (BYTE)svalue;
                break;
            case S_CLRMIN:
                if( !(ret = sget_short(buffer, &svalue)) )
                    cnt.opr.clrmin = (BYTE)svalue;
                break;
            case S_DB_EU:
                ret = sget_float( buffer, &(cnt.opr.deadband) );
                break;
            case S_TIMOUT:
                ret = sget_long( buffer, &(cnt.opr.watch_period) );
                break;
            case S_CNT_MUL:
                ret = sget_float( buffer, &(cnt.opr.mul_const) );
                break;
            case S_CNT_DIV:
                ret = sget_float( buffer, &(cnt.opr.div_const) );
                break;
            case S_HWA:
                ret = sget_short( buffer, &(cnt.fix.hwa) );
                break;
            case S_EGU_CODE:
                ret = -1;
                if( !sget_string( buffer, tvalue, NAME_LEN ) )
                {
                    for( i=0; i<rtdb.no_egu; i++ )
                    {
                        if( !strcmp( rtdb.EGUnit[i].egu, tvalue ) )
                        {
                            cnt.fix.egu_code = (SCHAR)i;
                            ret = 0;
                            break;
                        }
                    }
                }
                break;
            case S_RTU_NAME:
                ret = -1;
                if( !sget_string(buffer, tvalue, NAME_LEN) )
                {
                    for( i=0; i<rtdb.no_rtu; i++ )
                    {
                        if( !strcmp(rtdb.rtut[i].fix.name, tvalue) )
                        {
                            cnt.fix.mID.type = PVT_CNT;
                            cnt.fix.mID.rtu = i;
                            cnt.fix.mID.sqn = OnRTU[i].cnt++;
                            ret = 0;
                            break;
                        }
                    }
                }
                break;
            case S_NAME:
                ret = sget_string( buffer, cnt.fix.name, sizeof(cnt.fix.name) );
                break;
            case S_INFO:
                ret = sget_alltext( buffer, cnt.fix.info, sizeof(cnt.fix.info) );
                break;
            case S_END:
                // pocetna inicijalizacija
                cnt.var.status = 0;
                put_bit( cnt.var.status, CNT_VAL, 1 );
                put_bit( cnt.var.status, CNT_SCAN1, 1 );
                put_bit( cnt.opr.status, OPR_ACT, 1 );
                // kopiraj gde treba
                memcpy( &(rtdb.rtut[cnt.fix.mID.rtu].fix.cntfut.cnt_t[cnt.fix.mID.sqn]), &cnt, sizeof(COUNTER) );
                repeat = false;
                break;
            default:
                break;
            }
        }
        if( ret )
            repeat = false;
    }
    return ret;
}


/*--------------------------------------------------------------------------*/
/* Ucitava deo konfiguracione datoteke koja pocinje kljucnom reci [DIGITAL] */
/* a zavrsava se sa [END] u strukturu rtdb.rtut                             */
/*--------------------------------------------------------------------------*/
int in_digital( FILE* file, int pass )
{
    if( (pass == 0) || (pass == 3) )                     // u prvom i zadnjem prolazu ne radis nista
        return 0;

    int   i, ret=0, repeat=true;
    char  tvalue[40];
    int   in_off, out_off;
    KEYWRD dig_keywrd[] =
    {
        "Type",        S_TYPE,
        "InputList",   S_IN_LIST,
        "Input",       S_IN_OFF,
        "OutputList",  S_OUT_LIST,
        "Output",      S_OUT_OFF,
        "CmdTimOut",   S_TIMOUT,
        "RTU",         S_RTU_NAME,
        "Name",        S_NAME,
        "Info",        S_INFO,
        "[STOP]",      S_END,
        NULL,          0
    };

    DIGITAL dig;
    memset( &dig, 0, sizeof(DIGITAL) );

    while( repeat )
    {
        if( pass == 1 )
        {
            switch( ReadLineCfg(file, dig_keywrd, 0) )
            {
            case ERR_READLINE:
                return -1;
            case S_RTU_NAME:
                ret = -1;
                if( !sget_string(buffer, tvalue, NAME_LEN) )
                {
                    for( i=0; i<rtdb.no_rtu; i++ )
                    {
                        if( !strcmp( rtdb.rtut[i].fix.name, tvalue ) )
                        {
                            OnRTU[i].dig++;
                            ret = 0;
                            break;
                        }
                    }
                }
                break;
            case S_END:
                repeat = false;
                break;
            default:
                break;
            }
        }
        else if( pass == 2 )
        {
            switch( ReadLineCfg(file, dig_keywrd, 0) )
            {
            case ERR_READLINE:
                return -1;
            case S_TYPE:
                ret = -1;
                if( !sget_string(buffer, tvalue, NAME_LEN) )
                {
                    for( i=0; i<rtdb.no_digdev; i++ )
                    {
                        DigDevDef *tdd = rtdb.DigDev + i;
                        if( !strcmp(tdd->type, tvalue) )
                        {
                            dig.fix.type = i;
                            // iskopiraj definiciju uredjaja iz kataloga
                            dig.fix.no_in = tdd->noIn;
                            dig.fix.no_out = tdd->noOut;
                            memcpy( dig.fix.states, tdd->states, sizeof(tdd->states) );
                            memcpy( dig.fix.commands, tdd->commands, sizeof(tdd->commands) );
                            ret = 0;
                            break;
                        }
                    }
                }
                break;
            case S_IN_LIST:
                in_off = 0;
                break;
            case S_IN_OFF:
                ret = sget_short( buffer, &(dig.fix.hw_in[in_off++]) );
                break;
            case S_OUT_LIST:
                out_off = 0;
                break;
            case S_OUT_OFF:
                ret = sget_short( buffer, &(dig.fix.hw_out[out_off++]) );
                break;
            case S_TIMOUT:
                ret = sget_short( buffer, &(dig.fix.cmd_timout) );
                break;
            case S_RTU_NAME:
                ret = -1;
                if( !sget_string(buffer, tvalue, NAME_LEN) )
                {
                    for( i=0; i<rtdb.no_rtu; i++ )
                    {
                        if( !strcmp(rtdb.rtut[i].fix.name, tvalue) )
                        {
                            dig.fix.mID.type = PVT_DIG;
                            dig.fix.mID.rtu = i;
                            dig.fix.mID.sqn = OnRTU[i].dig++;
                            ret = 0;
                            break;
                        }
                    }
                }
                break;
            case S_NAME:
                ret = sget_string( buffer, dig.fix.name, sizeof(dig.fix.name) );
                break;
            case S_INFO:
                ret = sget_alltext( buffer, dig.fix.info, sizeof(dig.fix.info) );
                break;
            case S_END:
                // pocetna inicijalizacija
                dig.var.status = 0;
                put_bit( dig.var.status, DIG_VAL, 1 );
                put_bit( dig.opr.status, OPR_ACT, 1 );
                // pocetno stanje
                if( dig.fix.no_in )
                {
                    put_bit( dig.var.status, DIG_IN_SCAN1, 1 );
                    dig.var.state = 0;
                }
                else
                    dig.var.state = dig.fix.states[0];
                // pocetna komanda
                if( dig.fix.no_out )
                {
                    put_bit( dig.var.status, DIG_OUT_SCAN1, 1 );
                    dig.var.command = 0;
                }
                else
                    dig.var.command = dig.fix.commands[0];
                // kopiraj gde treba
                memcpy( &(rtdb.rtut[dig.fix.mID.rtu].fix.digfut.dig_t[dig.fix.mID.sqn]), &dig, sizeof(DIGITAL) );
                repeat = false;
                break;
            default:
                break;
            }
        }
        if( ret )
            repeat = false;
    }

    return ret;
}


int in_object( FILE* file, int pass )
{
    if( pass == 0 )                     // u prvom prolazu ne radis nista
        return 0;

    int   i, ret=0, repeat=true, elem_off, param_off;
    char  tvalue[40], tvalue1[40];
    PVID *pvidp;
    KEYWRD obj_keywrd[] =
    {
        "Type",        S_TYPE,
        "EGUnit",      S_EGU_CODE,
        "NoElem",      S_STS_NO,
        "ElemList",    S_STS_LIST,
        "Element",     S_STS_OFF,
        "NoParam",     S_NO_PARAM,
        "ParamList",   S_PARAM_LIST,
        "Parameter",   S_PARAM_OFF,
        "RTU",         S_RTU_NAME,
        "Name",        S_NAME,
        "Info",        S_INFO,
        "[STOP]",      S_END,
        NULL,          0
    };

    OBJECT obj;
    memset( &obj, 0, sizeof( OBJECT ) );

    OBJECT* objp;
    if( pass == 3 )
    {
        int rtu = OnSYS.rtu - 1;
        int sqn = OnRTU[rtu].obj++;
        objp = &rtdb.rtut[rtu].fix.objfut.obj_t[sqn];
    }

    while( repeat )
    {
        if( pass == 1 )
        {
            switch( ReadLineCfg(file, obj_keywrd, 0) )
            {
            case ERR_READLINE:
                return -1;
            case S_RTU_NAME:
                ret = -1;
                if( !sget_string(buffer, tvalue, NAME_LEN) )
                {
                    for( i=0; i<rtdb.no_rtu; i++ )
                    {
                        if( !strcmp( rtdb.rtut[i].fix.name, tvalue ) )
                        {
                            OnRTU[i].obj++;
                            rtdb.rtut[i].fix.objfut.total ++;
                            ret = 0;
                            break;
                        }
                    }
                }
                break;
            case S_END:
                repeat = false;
                break;
            default:
                break;
            }
        }
        else if( pass == 2 )
        {
            switch( ReadLineCfg(file, obj_keywrd, 0) )
            {
            case ERR_READLINE:
                return -1;
            case S_TYPE:
                ret = -1;
                if( !sget_string(buffer, tvalue, NAME_LEN) )
                {
                    for( i=0; i<rtdb.no_objdev; i++ )
                    {
                        if( !strcmp(rtdb.ObjDev[i].type, tvalue) )
                        {
                            obj.fix.type = i;
                            obj.fix.obj_class = rtdb.ObjDev[i].klasa;
                            obj.fix.category = rtdb.ObjDev[i].category;
                            ret = 0;
                            break;
                        }
                    }
                }
                break;
            case S_EGU_CODE:
                ret = -1;
                if( !sget_string(buffer, tvalue, NAME_LEN) )
                {
                    for( i=0; i<rtdb.no_egu; i++ )
                    {
                        if( !strcmp(rtdb.EGUnit[i].egu, tvalue) )
                        {
                            obj.fix.egu_code = i;
                            ret = 0;
                            break;
                        }
                    }
                }
                break;
            case S_STS_NO:
                ret = sget_short( buffer, &obj.fix.no_elem );
                break;
            case S_NO_PARAM:
                ret = sget_short( buffer, &obj.fix.no_param );
                break;
            case S_PARAM_LIST:
                param_off = 0;
                break;
            case S_PARAM_OFF:
                if( param_off < obj.fix.no_param )
                {
                    ret = sget_float( buffer, &(obj.opr.param[param_off++]) );
                }
                break;
            case S_RTU_NAME:
                if( !sget_string(buffer, tvalue, NAME_LEN) )
                {
                    for( i=0; i<rtdb.no_rtu; i++ )
                    {
                        if( !strcmp(rtdb.rtut[i].fix.name, tvalue) )
                        {
                            obj.fix.mID.type = PVT_OBJ;
                            obj.fix.mID.rtu = i;
                            obj.fix.mID.sqn = OnRTU[i].obj++;
                            ret = 0;
                            break;
                        }
                    }
                }
                break;
            case S_NAME:
                ret = sget_string( buffer, obj.fix.name, sizeof(obj.fix.name) );
                break;
            case S_INFO:
                ret = sget_alltext( buffer, obj.fix.info, sizeof(obj.fix.info) );
                break;
            case S_END:
                // pocetna inicijalizacija
                put_bit( obj.var.status, OBJ_VAL, 1 );
                put_bit( obj.opr.status, OPR_ACT, 1 );
                // kopiraj gde treba
                memcpy( &(rtdb.rtut[obj.fix.mID.rtu].fix.objfut.obj_t[obj.fix.mID.sqn]), &obj, sizeof(OBJECT) );
                repeat = false;
                break;
            default:
                break;
            }
        }
        else if( pass == 3 )
        {
            // ucitava elemente objekta
            switch( ReadLineCfg(file, obj_keywrd, 0) )
            {
            case ERR_READLINE:
                return -1;
            case S_STS_LIST:
                elem_off = 0;
                break;
            case S_STS_OFF:
                if( elem_off < objp->fix.no_elem )
                {
                    ret = -1;
                    if( sscanf(buffer, "%*s %s %*s %s", &tvalue, &tvalue1) == 2 )
                    {
                        if( (pvidp = get_PVID(tvalue, tvalue1, false)) )    // da li je to legalan PVID !
                        {
                            objp->fix.element[elem_off++] = *pvidp;
                            ret = 0;
                        }
                    }
                }
                break;
            case S_END:
                repeat = false;
                break;
            }
        }
        if( ret )
            repeat = false;
    }

    return ret;
}


int on_end( int pass )
{
    int ret = 0;
    if( pass == 0 )
    {
        if( OnSYS.stn > MAX_STN )
            return -1;
        rtdb.no_station = OnSYS.stn;
        if( rtdb.no_station )
        {
            rtdb.station = (STATION*)calloc( rtdb.no_station,sizeof(STATION) );
        }
        rtdb.no_egu = OnSYS.egu;
        if( rtdb.no_egu )
        {
            rtdb.EGUnit = (EguDef*)calloc( rtdb.no_egu,sizeof(EguDef) );
        }
        rtdb.no_state = OnSYS.sts;
        if( rtdb.no_state )
        {
            rtdb.DigState = (StateDef*)calloc( rtdb.no_state,sizeof(StateDef) );
        }
        rtdb.no_command = OnSYS.cmd;
        if( rtdb.no_command )
        {
            rtdb.DigCmd = (CommandDef*)calloc( rtdb.no_command,sizeof(CommandDef) );
        }
        rtdb.no_digdev = OnSYS.digd;
        if( rtdb.no_digdev )
        {
            rtdb.DigDev = (DigDevDef*)calloc( rtdb.no_digdev,sizeof(DigDevDef) );
        }
        rtdb.no_objdev = OnSYS.objd;
        if( rtdb.no_objdev )
        {
            rtdb.ObjDev = (ObjectDef*)calloc( rtdb.no_objdev, sizeof(ObjectDef) );
        }
        rtdb.no_channel = OnSYS.chn;
        if(rtdb.no_channel  )
        {
            rtdb.chanet = (CHANET*)calloc( rtdb.no_channel, sizeof(CHANET) );
        }
        rtdb.no_rtu = OnSYS.rtu;
        if( rtdb.no_rtu )
        {
            rtdb.rtut = (RTUT*)calloc( rtdb.no_rtu,sizeof(RTUT) );
        }

        // pocetna inicijalizacija
        for( int i=0; i < rtdb.no_station; i++ )
        {
            STATION *stnp = rtdb.station + i;
            stnp->fix.mID.type = PVT_STN;
            stnp->fix.mID.sqn = i;
            stnp->fix.station_rtu = -1;
        }
        for( int i=0; i < rtdb.no_channel; i++ )
        {
            CHANET *chp = rtdb.chanet + i;
            chp->fix.mID.type = PVT_CHA;
            chp->fix.mID.sqn = i;
            if( rtdb.no_station ) 
                chp->opr.owner = -1;
        }
        for( int i=0; i < rtdb.no_rtu; i++ )
        {
            RTUT *rtup = rtdb.rtut + i;
            rtup->fix.mID.type = PVT_RTU;
            rtup->fix.mID.sqn = rtup->fix.mID.rtu = i;
            if( rtdb.no_station ) 
                rtup->opr.owner = -1;
        }
        // uzmi memoriju za prebrojavanje promenljivih na RTU 
        OnRTU = (RTU_DESCRIPT *)calloc( rtdb.no_rtu,sizeof(RTU_DESCRIPT) );
    }
    else if( pass == 1 )
    {
        // alociraj memoriju za RAW podatke i SCADA promenljive
        for( int i=0; i < rtdb.no_rtu; i++ )
        {
            RTUT *rtu_p = rtdb.rtut + i;
            if( rtu_p->fix.no_ain )          // AI
            {
                rtu_p->fix.ainfut.rawdat = (WORD *)calloc(rtu_p->fix.no_ain, sizeof(WORD));
                rtu_p->fix.ainfut.ain_lt = (short *)calloc(rtu_p->fix.no_ain, sizeof(short));
                if( (rtu_p->fix.ainfut.total = OnRTU[i].ain) )
                {
                    rtu_p->fix.ainfut.ain_t = (ANAIN*)calloc( rtu_p->fix.ainfut.total,sizeof(ANAIN) );
                }
            }
            if( rtu_p->fix.no_aout )         // AO
            {
                rtu_p->fix.aoutfut.rawdat = (WORD *)calloc(rtu_p->fix.no_aout, sizeof(WORD));
                rtu_p->fix.aoutfut.aout_lt = (short *)calloc(rtu_p->fix.no_aout, sizeof(short));
                if( (rtu_p->fix.aoutfut.total = OnRTU[i].aout) )
                {
                    rtu_p->fix.aoutfut.aout_t = (ANAOUT*)calloc( rtu_p->fix.aoutfut.total,sizeof(ANAOUT) );
                }
            }
            if( rtu_p->fix.no_cnt )          // CNT
            {
                rtu_p->fix.cntfut.rawdat = (long *)calloc(rtu_p->fix.no_cnt, sizeof(long));
                rtu_p->fix.cntfut.cnt_lt = (short *)calloc(rtu_p->fix.no_cnt, sizeof(short));
                if( (rtu_p->fix.cntfut.total = OnRTU[i].cnt) )
                {
                    rtu_p->fix.cntfut.cnt_t = (COUNTER*)calloc( rtu_p->fix.cntfut.total,sizeof(COUNTER) );
                }
            }
            if( rtu_p->fix.no_din )          // DI
            {
                int pom = rtu_p->fix.no_din / 8;
                if( rtu_p->fix.no_din % 8 ) ++pom;
                rtu_p->fix.digfut.rawdat_in = (BYTE *)calloc( 1,pom );
                rtu_p->fix.digfut.di_lt = (short *)calloc(rtu_p->fix.no_din, sizeof(short));
            }
            if( rtu_p->fix.no_dout )         // DO
            {
                int pom = rtu_p->fix.no_dout / 8;
                if( rtu_p->fix.no_dout % 8 ) ++pom;
                rtu_p->fix.digfut.rawdat_out = (BYTE *)calloc(1, pom );
                rtu_p->fix.digfut.do_lt = (short *)calloc(rtu_p->fix.no_dout, sizeof(short));
            }            
            if( (rtu_p->fix.digfut.total = OnRTU[i].dig) )             // DIGITAL
            {
                rtu_p->fix.digfut.dig_t = (DIGITAL*)calloc( rtu_p->fix.digfut.total,sizeof(DIGITAL) );
            }
            if( (rtu_p->fix.objfut.total = OnRTU[i].obj) )             // OBJECT
            {
                rtu_p->fix.objfut.obj_t = (OBJECT*)calloc( rtu_p->fix.objfut.total, sizeof(OBJECT) );
            }
        }
    }
    else if( pass == 2 )
    {
        for( int r = 0; r < rtdb.no_rtu; r++ )
        {
            int i, j;
            RTUT *rtup = rtdb.rtut + r;

#ifndef _dSIM
            CHANET *chp = rtdb.chanet + rtup->fix.channel;
            int protokol = chp->fix.protocol;
            if( protokol == RTU_protokoli[protokol]->protocol_type )
            {
                // ucitaj dodatnu IO konfiguraciju za RTU (ako treba)
                ret = RTU_protokoli[protokol]->init_RTU( r );
            }
            else
            {
                // pogresna konfiguracija
                ret = -1;
            }
            if( ret )
                goto end;
#endif

            // namesti link tabele za procesne UI
            // nema PV:-1; ima PV: 0,1,2...
            memset( rtup->fix.ainfut.ain_lt, -1, rtup->fix.no_ain*sizeof(short) );
            for( i=0; i < rtup->fix.ainfut.total; i++ )
            {
                ANAIN *aip = rtup->fix.ainfut.ain_t + i;
                rtup->fix.ainfut.ain_lt[aip->fix.hwa] = aip->fix.mID.sqn;
            }
            memset( rtup->fix.aoutfut.aout_lt, -1, rtup->fix.no_aout*sizeof(short) );
            for( i=0; i < rtup->fix.aoutfut.total; i++ )
            {
                ANAOUT *aop = rtup->fix.aoutfut.aout_t + i;
                rtup->fix.aoutfut.aout_lt[aop->fix.hwa] = aop->fix.mID.sqn;
            }
            memset( rtup->fix.cntfut.cnt_lt, -1, rtup->fix.no_cnt*sizeof(short) );
            for( i=0; i < rtup->fix.cntfut.total; i++ )
            {
                COUNTER *cntp = rtup->fix.cntfut.cnt_t + i;
                rtup->fix.cntfut.cnt_lt[cntp->fix.hwa] = cntp->fix.mID.sqn;
            }
            memset( rtup->fix.digfut.di_lt, -1, rtup->fix.no_din*sizeof(short) );
            memset( rtup->fix.digfut.do_lt, -1, rtup->fix.no_dout*sizeof(short) );
            for( i=0; i < rtup->fix.digfut.total; i++ )
            {
                DIGITAL *digp = rtup->fix.digfut.dig_t + i;
                for( j=0; j<digp->fix.no_in; j++ )
                {
                    if( j == 0 )
                        rtup->fix.digfut.di_lt[digp->fix.hw_in[j]] = digp->fix.mID.sqn;
                    else
                        rtup->fix.digfut.di_lt[digp->fix.hw_in[j]] = -1;
                }
                for( j=0; j<digp->fix.no_out; j++ )
                {
                    if( j == 0 )
                        rtup->fix.digfut.do_lt[digp->fix.hw_out[j]] = digp->fix.mID.sqn;
                    else
                        rtup->fix.digfut.do_lt[digp->fix.hw_out[j]] = -1;
                }
            }
        }
    }
    else if( pass == 3 )
    {
        for( int r = 0; r < rtdb.no_rtu; r++ )
        {
            RTUT *rtup = rtdb.rtut + r;
            // namesti objID na referenciranim varijablama
            // obavezno je zbog ucitavanje objekata u zadnjem prolazu RdAsciiCfg
            for( int i=0; i < rtup->fix.objfut.total; i++ )
            {
                OBJECT *obj = rtup->fix.objfut.obj_t + i;
                put_bit( obj->var.status, OBJ_VAL, 1 );
                if( obj->fix.category == REAL_OBJ )
                {
                    // za realne objekte azuriraj na referenciranim varijablama pripadnost objekta
                    for( int j=0; j < obj->fix.no_elem; j++ )
                    {
                        void *rPVp = get_pvp( &obj->fix.element[j] );        // referencirana varijabla
                        PVID *rObjIDp;                                        // adresa objID u samoj strukturi
                        switch( obj->fix.element[j].type )
                        {
                        case PVT_AI:
                            rObjIDp = &((ANAIN*)rPVp)->fix.objID;
                            break;
                        case PVT_AO:
                            rObjIDp = &((ANAOUT*)rPVp)->fix.objID;
                            break;
                        case PVT_CNT:
                            rObjIDp = &((COUNTER*)rPVp)->fix.objID;
                            break;
                        case PVT_DIG:
                            rObjIDp = &((DIGITAL*)rPVp)->fix.objID;
                            break;
                        case PVT_OBJ:
                            rObjIDp = &((OBJECT*)rPVp)->fix.objID;
                            break;
                        }
                        // iskopiraj PVID objekta na objID u odredisnoj strukturi
                        memcpy( rObjIDp, &obj->fix.mID, sizeof(PVID) );
                    }
                }
            }
        }
    }

    // resetuj brojace referenci
    memset( OnRTU, 0, rtdb.no_rtu*sizeof(RTU_DESCRIPT) );
    memset( &OnSYS, 0, sizeof(SYS_DESCRIPT) );

end:
    return ret;
}


/*--------------------------------------------------------------------------*/
/* Funkcija ucitava konfiguracionu datoteku i podize RT bazu podataka       */
// prolazi tri puta kroz CFG fajl
//    pass 0:  ucitava SYSTEM i prebrojava Kataloge, RTUe i Kanale
//    pass 1:  ucitava Kataloge, RTUe i Kanale, a prebrojava sve PVAR
//    pass 2:  ucitava sve PVAR, ukljucujuci OBJECT-e
//    pass 3:  na kraju podesava OBJ reference
/*--------------------------------------------------------------------------*/

int ReadAsciiConfiguration( char* configurationFile )
{
    int repeat, ret = 0;
    FILE *fcfg;
    // kljucne reci koje se nalaze u konfiguracionoj datoteci
    KEYWRD cfg_keywrd[] =
    {
        "[SYSTEM]",     S_SYSTEM,
        "[EGU]",        S_EGU,
        "[STATE]",      S_STATE,
        "[COMMAND]",    S_COMMAND,
        "[DIGDEF]",     S_DIGDEF,
        "[OBJDEF]",     S_OBJDEF,
        "[CHANNEL]",    S_CHANEL,
        "[RTU]",        S_RTU,
        "[ANAIN]",      S_ANAIN,
        "[ANAOUT]",     S_ANAOUT,
        "[COUNTER]",    S_COUNTER,
        "[DIGITAL]",    S_DIGITAL,
        "[OBJECT]",     S_OBJECT,
        "[STATION]",    S_STATION,
        "[END]",        S_END,
        NULL,           0
    };

    // otvaranje datoteke 
    if( (fcfg=fopen(configurationFile, "rb")) == NULL )
    {
        int error = GetLastError();
        return -1;
    }

    for( pass_cnt=0; !ret && pass_cnt < 4; pass_cnt++ )
    {
        rewind( fcfg );
        line_num = 0;
        repeat = true;
        while( repeat )
        {
            // na osnovu koda kljucne reci "parsira" se konfiguraciona datoteka
            switch( ReadLineCfg(fcfg, cfg_keywrd, 1) )
            {
            case ERR_READLINE:
                rcode[0] = 0;
                ret = ERR_READLINE;
                break;
            case S_SYSTEM:
                rcode[0] = PVT_SYS;
                ret = in_system( fcfg, pass_cnt );
                break;
            case S_EGU:
                rcode[0] = CAT_EGU;
                ret = in_egunit( fcfg, pass_cnt );
                break;
            case S_STATE:
                rcode[0] = CAT_STS;
                ret = in_state( fcfg, pass_cnt );
                break;
            case S_COMMAND:
                rcode[0] = CAT_CMD;
                ret = in_command( fcfg, pass_cnt );
                break;
            case S_DIGDEF:
                rcode[0] = CAT_DIG;
                ret = in_digdef( fcfg, pass_cnt );
                break;
            case S_OBJDEF:
                rcode[0] = CAT_OBJ;
                ret = in_objdef( fcfg, pass_cnt );
                break;
            case S_CHANEL:
                rcode[0] = PVT_CHA;
                ret = in_channel( fcfg, pass_cnt );
                break;
            case S_RTU:
                rcode[0] = PVT_RTU;
                ret = in_rtu( fcfg, pass_cnt );
                break;
            case S_ANAIN:
                rcode[0] = PVT_AI;
                ret = in_anain( fcfg, pass_cnt );
                break;
            case S_ANAOUT:
                rcode[0] = PVT_AO;
                ret = in_anaout( fcfg, pass_cnt );
                break;
            case S_COUNTER:
                rcode[0] = PVT_CNT;
                ret = in_counter( fcfg, pass_cnt );
                break;
            case S_DIGITAL:
                rcode[0] = PVT_DIG;
                ret = in_digital( fcfg, pass_cnt );
                break;
            case S_OBJECT:
                rcode[0] = PVT_OBJ;
                ret = in_object( fcfg, pass_cnt );
                break;
            case S_END:
                rcode[0] = 0;
                repeat = false;
                ret = on_end( pass_cnt );
                break;
            default:
                break;
            }
            // ako ispadne sa greskom, prekini dalji prolazak kroz fajl
            if( ret != OK )
            {
                repeat = false;
                ShowCfgError();
            }
        }
    }
    fclose( fcfg );

    if( OnRTU ) 
    {
        free( (void*)OnRTU );
        OnRTU = NULL;
    }

    // zavrsi inicijalizaciju
    if( ret == OK )
        InitializeConfiguration();

    return ret;
}


void InitializeConfiguration(void)
{
    // ako slucajno sta treba podesiti ...
    for( int r = 0; r < rtdb.no_rtu; r++ )
    {
        RTUT *rtup = rtdb.rtut + r;
        // prebroj ako nista drugo
        total.ain  += rtup->fix.ainfut.total;
        total.aout += rtup->fix.aoutfut.total;
        total.dig  += rtup->fix.digfut.total;
        total.cnt  += rtup->fix.cntfut.total;
        total.obj  +=rtup->fix.objfut.total;
    }
    total.all = total.ain + total.aout + total.dig + total.cnt + total.obj;
}

void FreeConfigurationAllocations(void)
{
    // vrati svu memoriju
    for( int r=0; r < rtdb.no_rtu; r++ )
    {
        RTUT *rtup = rtdb.rtut + r;

#ifndef _dSIM
        // vrati memoriju IO konfiguracije za RTU (ako treba)
        int protokol = rtdb.chanet[rtup->fix.channel].fix.protocol;
        RTU_protokoli[protokol]->free_RTU( r );
#endif

        // ANAIN
        if( rtup->fix.no_ain )
        {
            free( rtup->fix.ainfut.rawdat );
            free( rtup->fix.ainfut.ain_lt);
            if( rtup->fix.ainfut.total )
                free( rtup->fix.ainfut.ain_t);
        }
        // ANAOUT
        if( rtup->fix.no_aout )
        {
            free( rtup->fix.aoutfut.rawdat );
            free( rtup->fix.aoutfut.aout_lt );
            if( rtup->fix.aoutfut.total )
                free( rtup->fix.aoutfut.aout_t );
        }
        // DIGITAL
        if( rtup->fix.no_din )
        {
            free( rtup->fix.digfut.rawdat_in );
            free( rtup->fix.digfut.di_lt );
        }
        if( rtup->fix.no_dout )
        {
            free( rtup->fix.digfut.rawdat_out );
            free( rtup->fix.digfut.do_lt );
        }
        if( rtup->fix.digfut.total )
            free( rtup->fix.digfut.dig_t );
        // COUNTER
        if( rtup->fix.no_cnt )
        {
            free( rtup->fix.cntfut.rawdat );
            free( rtup->fix.cntfut.cnt_lt );
            if(rtup->fix.cntfut.total)
                free( rtup->fix.cntfut.cnt_t );
        }
        //OBJECT
        if( rtup->fix.objfut.total )
            free( rtup->fix.objfut.obj_t );
    }
    // i sve sto je ostalo ...
    if( rtdb.no_channel )
        free( rtdb.chanet );
    if( rtdb.no_rtu )
        free( rtdb.rtut );
    if(rtdb.no_egu)
        free( rtdb.EGUnit );
    if(rtdb.no_state)
        free( rtdb.DigState );
    if(rtdb.no_command)
        free( rtdb.DigCmd );
    if(rtdb.no_digdev)
        free( rtdb.DigDev );
    if( rtdb.no_objdev )
        free( rtdb.ObjDev );
    if( rtdb.no_station )
        free( rtdb.station );
    memset( &rtdb, 0, sizeof(SYSTEM) );

    return;
}



int MultCfg( int mult_factor )
{
    int i, k, r;
    CHANET *chp;
    RTUT *rtu0, *rtup;
    char text[40];
    RTU_DESCRIPT per_rtu;

    rtu0 = rtdb.rtut + 0;

    // svaki RTU imace ovoliko SCADA promenljivih
    per_rtu.ain = rtu0->fix.no_ain;
    per_rtu.aout = rtu0->fix.no_aout;
    per_rtu.dig = rtu0->fix.no_din;
    per_rtu.cnt = rtu0->fix.no_cnt;
    // ukupna kolicina
    memset( &total, 0, sizeof(RTU_DESCRIPT) );

    // umnozi prvo procesne velicine u okviru postojece CFG sa jednim RTU-om
    if( rtu0->fix.ainfut.total )
    {
        rtu0->fix.ainfut.ain_t = (ANAIN*) realloc( rtu0->fix.ainfut.ain_t, per_rtu.ain*sizeof(ANAIN) );
        ANAIN *ai0 = rtu0->fix.ainfut.ain_t;                                                     // prvi AI
        for( i=1; i < per_rtu.ain; i++ )
        {
            ANAIN *aip = ai0 + i;
            memcpy( aip, ai0, sizeof(ANAIN) );
            aip->fix.hwa = ai0->fix.hwa + i;
            aip->fix.mID.sqn = ai0->fix.mID.sqn + i;
            sprintf( aip->fix.name, "AI-%03d", aip->fix.mID.sqn );
            sprintf( aip->fix.info, "Analog Input %03d", aip->fix.mID.sqn );
            rtu0->fix.ainfut.ain_lt[aip->fix.hwa] = aip->fix.mID.sqn;
        }
        rtu0->fix.ainfut.total = per_rtu.ain;
        total.ain += rtu0->fix.ainfut.total;
    }

    if( rtu0->fix.aoutfut.total )
    {
        rtu0->fix.aoutfut.aout_t = (ANAOUT*) realloc( rtu0->fix.aoutfut.aout_t, per_rtu.aout*sizeof(ANAOUT) );
        ANAOUT *ao0 = rtu0->fix.aoutfut.aout_t;      // prvi AO
        for( i=1; i < per_rtu.aout; i++ )
        {
            ANAOUT *aop = ao0 + i;
            memcpy( aop, ao0, sizeof(ANAOUT) );
            aop->fix.hwa = ao0->fix.hwa + i;
            aop->fix.mID.sqn = ao0->fix.mID.sqn + i;
            sprintf( aop->fix.name, "AO-%03d", aop->fix.mID.sqn );
            sprintf( aop->fix.info, "Analog Output %03d", aop->fix.mID.sqn );
            rtu0->fix.aoutfut.aout_lt[aop->fix.hwa] = aop->fix.mID.sqn;
        }
        rtu0->fix.aoutfut.total = per_rtu.aout;
        total.aout += rtu0->fix.aoutfut.total;
    }

    if( rtu0->fix.digfut.total )
    {
        rtu0->fix.digfut.dig_t = (DIGITAL*) realloc( rtu0->fix.digfut.dig_t, per_rtu.dig*sizeof(DIGITAL) );
        DIGITAL *d0 = rtu0->fix.digfut.dig_t;           // prvi DIG
        // posto imamo samo jedan DigDev
        short ihwa = d0->fix.no_in;          // pocetna adresa za nove ulaze
        short ohwa = d0->fix.no_out;         // pocetna adresa za nove izlaze
        for( i=1; i<per_rtu.dig; i++ )
        {
            DIGITAL *dp = d0 + i;                  // dodati DIG
            memcpy( dp, d0, sizeof(DIGITAL) );
            dp->fix.mID.sqn = d0->fix.mID.sqn + i;
            for(k=0; k<dp->fix.no_in; k++ )
            {
                dp->fix.hw_in[k] = ihwa++;
                if( k == 0 )
                    rtu0->fix.digfut.di_lt[dp->fix.hw_in[k]] = dp->fix.mID.sqn;
                else
                    rtu0->fix.digfut.di_lt[dp->fix.hw_in[k]] = -1;
            }
            for(k=0; k<dp->fix.no_out; k++ )
            {
                dp->fix.hw_out[k] = ohwa++;
                if( k == 0 )
                    rtu0->fix.digfut.do_lt[dp->fix.hw_out[k]] = dp->fix.mID.sqn;
                else
                    rtu0->fix.digfut.do_lt[dp->fix.hw_out[k]] = -1;
            }
            sprintf( (char*)dp->fix.name,  "DD-%03d", dp->fix.mID.sqn );
            sprintf( (char*)dp->fix.info, "DigDevice %03d", dp->fix.mID.sqn );
        }
        rtu0->fix.digfut.total = per_rtu.dig;
        total.dig += rtu0->fix.digfut.total;
    }

    if( rtu0->fix.cntfut.total )
    {
        rtu0->fix.cntfut.cnt_t = (COUNTER*) realloc( rtu0->fix.cntfut.cnt_t, per_rtu.cnt*sizeof(COUNTER) );
        COUNTER *c0 = rtu0->fix.cntfut.cnt_t;           // prvi AI
        for( i=1; i<per_rtu.cnt; i++ )
        {
            COUNTER *cp = c0 + i;                        // dodati AI
            memcpy( cp, c0, sizeof(COUNTER) );
            cp->fix.hwa = c0->fix.hwa + i;
            cp->fix.mID.sqn = c0->fix.mID.sqn + i;
            sprintf( (char*)cp->fix.name, "CNT-%03d", cp->fix.mID.sqn );
            sprintf( (char*)cp->fix.info, "Counter %03d", cp->fix.mID.sqn );
            rtu0->fix.cntfut.cnt_lt[cp->fix.hwa] = cp->fix.mID.sqn;
        }
        rtu0->fix.cntfut.total = per_rtu.cnt;
        total.cnt += rtu0->fix.cntfut.total;
    }
    /*
    if( rtu0->fix.objfut.total )
    {
    // pravi logiku za rad sa promenljivim granicama alarma
    per_rtu.obj = rtu0->fix.ainfut.total / MAX_ELEM;
    rtu0->fix.objfut.obj_t = (OBJECT*) realloc( rtu0->fix.objfut.obj_t, per_rtu.obj*sizeof(OBJECT) );
    OBJECT *o0 = rtu0->fix.objfut.obj_t;            // prvi Objekat
    int aindx = 0;                                  // indeks tekuceg ANAIN elementa
    for( i=0; i<per_rtu.obj; i++ )
    {
    OBJECT *op = o0 + i;                         // dodati Objekat
    if( i != 0 )
    {
    // odradi kopiranje gen info o objektu
    memcpy( op, o0, sizeof(OBJECT) );
    op->fix.mID.sqn = o0->fix.mID.sqn + i;
    sprintf( (char*)op->fix.name, "ALS-%03d", op->fix.mID.sqn );
    sprintf( (char*)op->fix.info, "Alarm set %03d", op->fix.mID.sqn );
    }
    // namesti listu elemenata za svaki objekat
    op->fix.no_elem = MAX_ELEM;
    for( int j=0; j<op->fix.no_elem; j++ )
    {
    // popuni listu elemenata sledecim ANAIN
    op->fix.element[j] = (rtu0->fix.ainfut.ain_t+aindx++)->fix.mID;
    }
    }
    rtu0->fix.objfut.total = per_rtu.obj;
    total.obj += rtu0->fix.objfut.total;
    }
    */

    // sada umnozi mult_factor puta na svaki od konfigurisanih kanala

    // realociraj memoriju za RTUT strukture
    rtdb.no_rtu = mult_factor*rtdb.no_channel;
    rtdb.rtut = (RTUT*) realloc( rtdb.rtut, rtdb.no_rtu*sizeof(RTUT) );
    rtu0 = rtdb.rtut + 0;

    for( r=1; r < rtdb.no_rtu; r++ )
    {
        // iskopiraj celu RTU0 strukturu, potom izmeni sta treba
        rtup = rtu0 + r;
        memcpy( rtup, rtu0, sizeof(RTUT) );
        sprintf( text, "RTU-%03d", r );
        strcpy( rtup->fix.name, text );
        strcpy( rtup->fix.info, text );
        rtup->fix.mID.rtu = r;
        rtup->fix.mID.sqn = r;
        rtup->fix.hport = rtu0->fix.hport + r;
        // alociraj memoriju za RAW podatke i SCADA promenljive
        if( rtup->fix.no_ain )          // AI
        {
            rtup->fix.ainfut.rawdat = (WORD*)calloc(rtup->fix.no_ain, sizeof(WORD));
            if( rtup->fix.ainfut.total )
            {
                rtup->fix.ainfut.ain_t = (ANAIN*)calloc( rtup->fix.ainfut.total, sizeof(ANAIN) );
                memcpy( rtup->fix.ainfut.ain_t, rtu0->fix.ainfut.ain_t, rtu0->fix.ainfut.total*sizeof(ANAIN) );
                for( i=0; i< rtup->fix.ainfut.total; i++ )
                {
                    rtup->fix.ainfut.ain_t[i].fix.mID.rtu = r;
                }
                rtup->fix.ainfut.ain_lt = (short *)calloc(rtup->fix.no_ain, sizeof(short));
                memcpy( rtup->fix.ainfut.ain_lt, rtu0->fix.ainfut.ain_lt, rtup->fix.no_ain*sizeof(short) );
                total.ain += rtup->fix.ainfut.total;
            }
        }
        if( rtup->fix.no_aout )         // AO
        {
            rtup->fix.aoutfut.rawdat = (WORD*)calloc(rtup->fix.no_aout, sizeof(WORD));
            if( rtup->fix.aoutfut.total )
            {
                rtup->fix.aoutfut.aout_t = (ANAOUT*)calloc( rtup->fix.aoutfut.total, sizeof(ANAOUT) );
                memcpy( rtup->fix.aoutfut.aout_t, rtu0->fix.aoutfut.aout_t, rtu0->fix.aoutfut.total*sizeof(ANAOUT) );
                for( i=0; i< rtup->fix.aoutfut.total; i++ )
                {
                    rtup->fix.aoutfut.aout_t[i].fix.mID.rtu = r;
                }
                rtup->fix.aoutfut.aout_lt = (short *)calloc(rtup->fix.no_aout, sizeof(short));
                memcpy( rtup->fix.aoutfut.aout_lt, rtu0->fix.aoutfut.aout_lt, rtup->fix.no_aout*sizeof(short) );
                total.aout += rtup->fix.aoutfut.total;
            }
        }
        if( rtup->fix.no_cnt )          // CNT
        {
            rtup->fix.cntfut.rawdat = (long*)calloc(rtup->fix.no_cnt, sizeof(long));
            if( rtup->fix.cntfut.total )
            {
                rtup->fix.cntfut.cnt_t = (COUNTER*)calloc( rtup->fix.cntfut.total,sizeof(COUNTER) );
                memcpy( rtup->fix.cntfut.cnt_t, rtu0->fix.cntfut.cnt_t, rtu0->fix.cntfut.total*sizeof(COUNTER) );
                for( i=0; i< rtup->fix.cntfut.total; i++ )
                {
                    rtup->fix.cntfut.cnt_t[i].fix.mID.rtu = r;
                }
                rtup->fix.cntfut.cnt_lt = (short *)calloc(rtup->fix.no_cnt, sizeof(short));
                memcpy( rtup->fix.cntfut.cnt_lt, rtu0->fix.cntfut.cnt_lt, rtup->fix.no_cnt*sizeof(short) );
                total.cnt += rtup->fix.cntfut.total;
            }
        }
        if( rtup->fix.no_din )          // DI
        {
            int pom = rtup->fix.no_din / 8;
            if( rtup->fix.no_din % 8 ) pom++;
            rtup->fix.digfut.rawdat_in = (BYTE *)calloc( 1,pom );
        }
        if( rtup->fix.no_dout )         // DO
        {
            int pom = rtup->fix.no_dout / 8;
            if( rtup->fix.no_dout % 8 ) pom++;
            rtup->fix.digfut.rawdat_out = (BYTE *)calloc(1, pom );
        }            
        if( rtup->fix.digfut.total  )             // DIGITAL
        {
            rtup->fix.digfut.dig_t = (DIGITAL*)calloc( rtup->fix.digfut.total, sizeof(DIGITAL) );
            memcpy( rtup->fix.digfut.dig_t, rtu0->fix.digfut.dig_t, rtu0->fix.digfut.total*sizeof(DIGITAL) );
            for( i=0; i< rtup->fix.digfut.total; i++ )
            {
                rtup->fix.digfut.dig_t[i].fix.mID.rtu = r;
            }
            rtup->fix.digfut.di_lt = (short *)calloc(rtup->fix.no_din, sizeof(short));
            memcpy( rtup->fix.digfut.di_lt, rtu0->fix.digfut.di_lt, rtup->fix.no_din*sizeof(short) );
            rtup->fix.digfut.do_lt = (short *)calloc(rtup->fix.no_dout, sizeof(short));
            memcpy( rtup->fix.digfut.do_lt, rtu0->fix.digfut.do_lt, rtup->fix.no_dout*sizeof(short) );
            total.dig += rtup->fix.digfut.total;
        }

        if( rtu0->fix.objfut.total )
        {
            rtup->fix.objfut.obj_t = (OBJECT*)calloc( rtup->fix.objfut.total,sizeof(OBJECT) );
            memcpy( rtup->fix.objfut.obj_t, rtu0->fix.objfut.obj_t, rtu0->fix.objfut.total*sizeof(OBJECT) );
            for( i=0; i< rtup->fix.objfut.total; i++ )
            {
                OBJECT *op = rtup->fix.objfut.obj_t + i;
                op->fix.mID.rtu = r;
                // namesti listu elemenata za svaki objekat
                for( int j=0; j<op->fix.no_elem; j++ )
                {
                    op->fix.element[j].rtu = r;
                }
            }
            total.obj += rtup->fix.objfut.total;
        }
    }

    for( k=0, r=0; k < rtdb.no_channel; k++ )
    {
        chp = rtdb.chanet + k;
        BYTE com_adr = 1;
        chp->fix.rtu_n = 0;
        for( i=0; i<mult_factor; i++, r++ )
        {
            rtup = rtdb.rtut + r;
            rtup->fix.com_adr = com_adr++;           // ostaje u okviru byte-a 0-255
            rtup->fix.channel = k;
            // popuni listu RTU stanica
            chp->fix.rtu_lst[chp->fix.rtu_n++] = rtup->fix.mID.sqn;
#ifndef _dSIM
            if( r )        // preskoci prvi RTU (rtu0)
            {
                // ucitaj dodatnu IO konfiguraciju za RTU (ako treba)
                int ret = RTU_protokoli[chp->fix.protocol]->init_RTU( r );
            }
#endif
        }
    }
    // prebroj SCADA promenljive
    total.all = total.ain + total.aout + total.cnt + total.dig + total.obj;
    return 0;
}


//////////////////////////////////////////////////////////////////////////////

// Zapis konfiguracije

#define KEWRD_LEN 20
#define DEEP_TAB   4

static int deep_level = 0;
void inc_deep_level( void )
{
    deep_level += DEEP_TAB;
}

void dec_deep_level( void )
{
    deep_level -= DEEP_TAB;
}

void fput_text( FILE* file, char *text )
{
    if( fputs(text, file) == EOF )
        _Abort();
}

void fput_newlin( FILE* file )
{
    fput_text( file, "\r\n" );
}

void fput_keywrd( FILE* file, char* keywrd, char* comment )
{
    char buffer[128];
    if( comment )
        sprintf( buffer, "%*s%s   <%s>\r\n", deep_level, "", keywrd, comment);
    else
        sprintf( buffer, "%*s%s\r\n", deep_level, "", keywrd);
    fput_text( file, buffer );
}

void fput_short( FILE* file, char* keywrd, short svalue, char* comment )
{
    char buffer[128];
    if ( comment )
        sprintf( buffer, "%*s  %-*.*s  %hd   <%s>\r\n", deep_level, "", KEWRD_LEN, KEWRD_LEN, keywrd, svalue, comment );
    else
        sprintf( buffer, "%*s  %-*.*s  %hd\r\n", deep_level, "", KEWRD_LEN, KEWRD_LEN, keywrd, svalue );
    fput_text( file, buffer );
}

void fput_long( FILE* file, char* keywrd, long lvalue, char* comment )
{
    char buffer[128];
    if ( comment )
        sprintf( buffer, "%*s  %-*.*s  %ld   <%s>\r\n", deep_level, "", KEWRD_LEN, KEWRD_LEN, keywrd, lvalue, comment );
    else
        sprintf( buffer, "%*s  %-*.*s  %ld\r\n", deep_level, "", KEWRD_LEN, KEWRD_LEN, keywrd, lvalue );
    fput_text( file, buffer );
}

void fput_ulong( FILE* file, char* keywrd, unsigned long ulvalue, char* comment )
{
    char buffer[128];
    if ( comment )
        sprintf( buffer, "%*s  %-*.*s  %lu   <%s>\r\n", deep_level, "", KEWRD_LEN, KEWRD_LEN, keywrd, ulvalue, comment );
    else
        sprintf( buffer, "%*s  %-*.*s  %lu\r\n", deep_level, "", KEWRD_LEN, KEWRD_LEN, keywrd, ulvalue );
    fput_text( file, buffer );
}

void fput_float( FILE* file, char* keywrd, float fvalue, char* comment )
{
    char buffer[128];
    if ( comment )
        sprintf( buffer, "%*s  %-*.*s  %.2f   <%s>\r\n", deep_level, "", KEWRD_LEN, KEWRD_LEN, keywrd, fvalue, comment );
    else
        sprintf( buffer, "%*s  %-*.*s  %.2f\r\n", deep_level, "", KEWRD_LEN, KEWRD_LEN, keywrd, fvalue );
    fput_text( file, buffer );
}

void fput_on_off( FILE* file, char* keywrd, BYTE state )
{
    char buffer[128];
    sprintf( buffer, "%*s  %-*.*s  %s\r\n", deep_level, "", KEWRD_LEN, KEWRD_LEN, keywrd, state?"ON":"OFF" );
    fput_text( file, buffer );
}

void fput_string( FILE* file, char* keywrd, char* text )
{
    char buffer[128];
    sprintf( buffer, "%*s  %-*.*s  %-s\r\n", deep_level, "", KEWRD_LEN, KEWRD_LEN, keywrd, text );
    fput_text( file, buffer );
}

void fput_vstring( FILE* file, char* keywrd, char* format, ... )
{
    char buffer[128];
    va_list argptr;
    int off;

    sprintf( buffer, "%*s  %-*.*s  ", deep_level, "", KEWRD_LEN, KEWRD_LEN, keywrd );

    off = strlen( buffer );
    va_start( argptr, format );
    vsprintf( &(buffer[off]), format,argptr );
    va_end( argptr );

    strcat( buffer, "\r\n" );

    fput_text( file, buffer );
}

void out_system( FILE* file )
{
    fput_keywrd( file, "[SYSTEM]", NULL );
    fput_string( file, "SysName", rtdb.sys_name );
    fput_keywrd( file, "[STOP]", NULL );
    fput_newlin( file );
}

void out_egu( FILE* file, EguDef* egu_p )
{
    fput_keywrd( file, "[EGU]", NULL );
    fput_string( file, "Unit", egu_p->egu );
    fput_string( file, "Color", ColorNames[egu_p->color] );
    fput_keywrd( file, "[STOP]", NULL );
    fput_newlin( file );
}

void out_state( FILE* file, StateDef* sts_p )
{
    fput_keywrd( file, "[STATE]", NULL );
    fput_string( file, "State", sts_p->state );
    fput_string( file, "Color", ColorNames[sts_p->color] );
    fput_keywrd( file, "[STOP]", NULL );
    fput_newlin( file );
}

void out_command( FILE* file, CommandDef* cmd_p )
{
    fput_keywrd( file, "[COMMAND]", NULL );
    fput_string( file, "Command", cmd_p->command );
    fput_string( file, "Color", ColorNames[cmd_p->color] );
    fput_keywrd( file, "[STOP]", NULL );
    fput_newlin( file );
}

void out_digdef( FILE* file, DigDevDef* ddef_p )
{
    int i,j;
    char buff[40];

    fput_keywrd( file, "[DIGDEF]", ddef_p->type );
    fput_string( file, "Type", ddef_p->type );
    fput_short ( file, "NoDigIn", ddef_p->noIn, NULL );
    fput_short ( file, "NoDigOut", ddef_p->noOut, NULL );

    // zapisi stanja
    fput_keywrd( file, "  StsList", NULL );
    j = raise_2( ddef_p->noIn );
    inc_deep_level();
    for( i=0; i < j; i++ )
    {
        sprintf( buff, "< %d >", i );
        fput_vstring ( file, "State", "%s  %-6.6s", rtdb.DigState[ ddef_p->states[i] ].state, buff );
    }
    dec_deep_level();

    // zapisi komande
    fput_keywrd( file, "  CmdList", NULL );
    j = raise_2( ddef_p->noOut );
    inc_deep_level();
    for( i=0; i<j; i++ )
    {
        sprintf( buff, "<%d>", i );
        fput_vstring( file, "Command", "%s  %-6.6s", rtdb.DigCmd[ ddef_p->commands[i] ].command, buff );
    }
    dec_deep_level();

    fput_keywrd( file, "[STOP]", ddef_p->type );
    fput_newlin( file );
}

void out_objdef( FILE* file, ObjectDef* odef_p )
{
    int i;
    char temp[80];
    char* klasa[]={ "DigObj", "AnaObj", NULL };
    char* catgry[]={ "Real", "Virtual", NULL };

    fput_keywrd( file, "[OBJDEF]", odef_p->type );
    fput_string( file, "Type", odef_p->type );
    fput_string( file, "Class", klasa[odef_p->klasa] );
    fput_string( file, "Category", catgry[odef_p->category] );

    fput_short( file, "NoElem", odef_p->noElems, NULL );
    if( odef_p->noElems )
    {
        fput_keywrd( file, "  ElemList", NULL );
        inc_deep_level();
        for( i=0; i<odef_p->noElems; i++ )
        {
            sprintf( temp, "%-20.20s | %-20.20s  < %2d > ", odef_p->elementDescription[i], GetTypeName(odef_p->elementType[i]), i );
            fput_string( file, "Element", temp );
        }
        dec_deep_level();
    }

    fput_short( file, "NoParam", odef_p->noParams, NULL );
    if( odef_p->noParams )
    {
        fput_keywrd( file, "  ParamList", NULL );
        inc_deep_level();
        for( i=0; i<odef_p->noParams; i++ )
        {
            sprintf( temp, "%-20.20s  < %2d >", odef_p->parameterDescription[i], i );
            fput_string( file, "Parameter", temp );
        }
        dec_deep_level();
    }

    fput_keywrd( file, "[STOP]", (char*)odef_p->type );
    fput_newlin( file );
}

void out_channel( FILE* file, CHANET* chan_p )
{
    char temp[80];

    fput_keywrd( file, "[CHANNEL]", chan_p->fix.info );
    fput_string( file, "Protocol", dataType[chan_p->fix.protocol] );
    fput_short( file, "TimeOut", chan_p->fix.timout, NULL );
    sprintf( temp, "%s%d", port[chan_p->fix.type], chan_p->fix.port + 1 );
    fput_string( file, "Port", temp  );
    if( chan_p->fix.type == UART )
    {
        fput_string( file, "BaudRate", baudrate[chan_p->fix.comm.uart.baud] );
        fput_string( file, "Parity", parity[chan_p->fix.comm.uart.parity] );
        fput_short( file, "StopBits", chan_p->fix.comm.uart.stop_bits+1, NULL );
        fput_short( file, "DataLen", chan_p->fix.comm.uart.lenght+5, NULL );
    }
    else
    {
        fput_string( file, "ipSide", ipside[chan_p->fix.comm.ip.server] );
        if( chan_p->fix.comm.ip.server )
            fput_short( file, "ipPort", chan_p->fix.comm.ip.locport, NULL );
    }

    fput_string( file, "Name", chan_p->fix.name );
    fput_string( file, "Info", chan_p->fix.info );
    fput_keywrd( file, "[STOP]", chan_p->fix.info );
    fput_newlin( file );
}

void out_rtu( FILE* file, RTUT* rtu_p )
{
    fput_keywrd( file, "[RTU]", rtu_p->fix.info );
    fput_on_off( file, "Virtual", (BYTE)rtu_p->var.virt );
    fput_string( file, "Channel", rtdb.chanet[rtu_p->fix.channel].fix.name );
    fput_short ( file, "CommAddr", rtu_p->fix.com_adr, NULL );
    fput_short ( file, "Period", rtu_p->opr.acq_period, NULL );
    fput_short ( file, "NoAin", rtu_p->fix.no_ain, NULL );
    fput_short ( file, "NoAout", rtu_p->fix.no_aout, NULL );
    fput_short ( file, "NoDin", rtu_p->fix.no_din, NULL );
    fput_short ( file, "NoDout", rtu_p->fix.no_dout, NULL );
    fput_short ( file, "NoCnt", rtu_p->fix.no_cnt, NULL );
    fput_string( file, "Name", rtu_p->fix.name );
    fput_string( file, "Info", rtu_p->fix.info );
    fput_string( file, "HostName", rtu_p->fix.hname );
    fput_short ( file, "HostPort", rtu_p->fix.hport, NULL );
    fput_keywrd( file, "[STOP]", rtu_p->fix.info );
    fput_newlin( file );
}

void out_anain( FILE* file, ANAIN* ain_p )
{
    char comment[80];
    RTUT *rtup = rtdb.rtut + ain_p->fix.mID.rtu;

    strcpy( comment, rtup->fix.name );
    strcat( comment, " / " );
    strcat( comment, ain_p->fix.name );

    fput_keywrd( file, "[ANAIN]", comment );
    fput_string( file, "EGUnit", rtdb.EGUnit[ain_p->fix.egu_code].egu );
    fput_float ( file, "EguBandLow", ain_p->opr.egu_range[0], NULL );
    fput_float ( file, "EguBandHigh", ain_p->opr.egu_range[1], NULL );
    fput_float ( file, "AlarmLow", ain_p->opr.alarm_lim[0], NULL );
    fput_float ( file, "AlarmHigh", ain_p->opr.alarm_lim[1], NULL );
    fput_float ( file, "WarnLow", ain_p->opr.warn_lim[0], NULL );
    fput_float ( file, "WarnHigh", ain_p->opr.warn_lim[1], NULL );
    fput_float ( file, "DeadBand", ain_p->opr.deadband, NULL );
    fput_string( file, "ElUnit", rtdb.EGUnit[ain_p->fix.el_code].egu );
    fput_float ( file, "ElBandLow", ain_p->fix.el_range[0], NULL );
    fput_float ( file, "ElBandHigh", ain_p->fix.el_range[1], NULL );
    fput_short ( file, "RawBandLow", ain_p->fix.raw_range[0], NULL );
    fput_short ( file, "RawBandHigh", ain_p->fix.raw_range[1], NULL );
    fput_short ( file, "hwAddr", ain_p->fix.hwa, NULL );
    fput_string( file, "RTU", rtup->fix.name );
    fput_string( file, "Name", ain_p->fix.name );
    fput_string( file, "Info", ain_p->fix.info );
    fput_keywrd( file, "[STOP]", comment );
    fput_newlin( file );
}

void out_anaout( FILE* file, ANAOUT* aout_p )
{
    char comment[80];
    RTUT *rtup = rtdb.rtut + aout_p->fix.mID.rtu;

    strcpy( comment, rtup->fix.name );
    strcat( comment, " / " );
    strcat( comment, aout_p->fix.name );

    fput_keywrd( file, "[ANAOUT]", comment );
    fput_string( file, "EGUnit", rtdb.EGUnit[aout_p->fix.egu_code].egu );
    fput_float ( file, "EguBandLow", aout_p->opr.egu_range[0], NULL );
    fput_float ( file, "EguBandHigh", aout_p->opr.egu_range[1], NULL );
    fput_float ( file, "AlarmLow", aout_p->opr.alarm_lim[0], NULL );
    fput_float ( file, "AlarmHigh", aout_p->opr.alarm_lim[1], NULL );
    fput_float ( file, "ClampBandLow", aout_p->opr.clamp_lim[0], NULL );
    fput_float ( file, "ClampBandHigh", aout_p->opr.clamp_lim[1], NULL );
    fput_float ( file, "DeadBand", aout_p->opr.deadband, NULL );
    fput_float ( file, "MaxRate", aout_p->opr.max_rate, NULL );
    fput_string( file, "ElUnit", rtdb.EGUnit[aout_p->fix.el_code].egu );
    fput_float ( file, "ElBandLow", aout_p->fix.el_range[0], NULL );
    fput_float ( file, "ElBandHigh", aout_p->fix.el_range[1], NULL );
    fput_short ( file, "RawBandLow", aout_p->fix.raw_range[0], NULL );
    fput_short ( file, "RawBandHigh", aout_p->fix.raw_range[1], NULL );
    fput_short ( file, "hwAddr", aout_p->fix.hwa, NULL );
    fput_string( file, "RTU", rtup->fix.name );
    fput_string( file, "Name", aout_p->fix.name );
    fput_string( file, "Info", aout_p->fix.info );
    fput_keywrd( file, "[STOP]", comment );
    fput_newlin( file );
}

void out_counter( FILE* file, COUNTER* cnt_p )
{
    char comment[80];
    RTUT *rtup = rtdb.rtut + cnt_p->fix.mID.rtu;

    strcpy( comment, rtup->fix.name );
    strcat( comment, " / " );
    strcat( comment, cnt_p->fix.name );

    fput_keywrd( file, "[COUNTER]", comment );
    fput_string( file, "EGUnit", rtdb.EGUnit[cnt_p->fix.egu_code].egu );
    fput_float ( file, "MulConst",  cnt_p->opr.mul_const, NULL );
    fput_float ( file, "DivConst", cnt_p->opr.div_const, NULL );
    fput_float ( file, "DeadBand", cnt_p->opr.deadband, NULL );
    fput_long  ( file, "WatchPeriod", cnt_p->opr.watch_period, NULL );
    fput_on_off( file, "AutoClear", get_bit(cnt_p->opr.status, OPR_AUTOCLR) );
    fput_short ( file, "ClearHour", cnt_p->opr.clrhour, NULL );
    fput_short ( file, "ClearMinute", cnt_p->opr.clrmin, NULL );
    fput_short ( file, "hwAddr", cnt_p->fix.hwa, NULL );
    fput_string( file, "RTU", rtup->fix.name );
    fput_string( file, "Name", cnt_p->fix.name );
    fput_string( file, "Info", cnt_p->fix.info );
    fput_keywrd( file, "[STOP]", comment );
    fput_newlin( file );
}

void out_digital( FILE* file, DIGITAL* dig_p )
{
    char comment[80];
    RTUT *rtup = rtdb.rtut + dig_p->fix.mID.rtu;

    strcpy( comment, rtup->fix.name );
    strcat( comment, " / " );
    strcat( comment, dig_p->fix.name );

    fput_keywrd( file, "[DIGITAL]", comment );
    fput_string( file, "Type", rtdb.DigDev[dig_p->fix.type].type );
    if( dig_p->fix.no_in )
    {
        fput_keywrd ( file, "  InputList", NULL );
        inc_deep_level();
        for( int i=0; i < dig_p->fix.no_in; i++ )
        {
            fput_vstring( file, "Input", "%d    < %d >", dig_p->fix.hw_in[i], i );
        }
        dec_deep_level();
    }
    if( dig_p->fix.no_out )
    {
        fput_keywrd ( file, "  OutputList", NULL );
        inc_deep_level();
        for( int i=0; i < dig_p->fix.no_out; i++ )
        {
            fput_vstring( file, "Output", "%d    < %d >", dig_p->fix.hw_out[i], i );
        }
        dec_deep_level();
        if( dig_p->fix.no_in )
        {
            fput_short( file, "CmdTimOut", dig_p->fix.cmd_timout, NULL );
        }
    }
    fput_string( file, "RTU", rtup->fix.name );
    fput_string( file, "Name", dig_p->fix.name );
    fput_string( file, "Info", dig_p->fix.info );
    fput_keywrd( file, "[STOP]", comment );
    fput_newlin( file );
}

void out_object( FILE* file, OBJECT* obj_p )
{
    char comment[80], temp[80];

    RTUT *rtup = rtdb.rtut + obj_p->fix.mID.rtu;
    strcpy( comment, rtup->fix.name );
    strcat( comment, " / " );
    strcat( comment, obj_p->fix.name );

    ObjectDef *odp = rtdb.ObjDev + obj_p->fix.type;

    fput_keywrd( file, "[OBJECT]", comment );
    fput_string( file, "Type", odp->type );
    fput_string( file, "EGUnit", rtdb.EGUnit[obj_p->fix.egu_code].egu );

    fput_short ( file, "NoElem", obj_p->fix.no_elem, NULL );
    if( obj_p->fix.no_elem )
    {
        fput_keywrd( file, "  ElemList", NULL );
        inc_deep_level();
        for( int i=0; i < obj_p->fix.no_elem; i++ )
        {
            sprintf( temp, "%s | %s     < %2d  %s >", rtup->fix.name, get_name(&obj_p->fix.element[i]), i, odp->elementDescription[i] );
            fput_string ( file, "Element", temp );
        }
        dec_deep_level();
    }

    fput_short ( file, "NoParam", obj_p->fix.no_param, NULL );
    if( obj_p->fix.no_param )
    {
        fput_keywrd( file, "  ParamList", NULL );
        inc_deep_level();
        for( int i=0; i < obj_p->fix.no_param; i++ )
        {
            sprintf( temp, "%.2f    < %2d %s > ", obj_p->opr.param[i], i, odp->parameterDescription[i] );
            fput_string ( file, "Parameter", temp );
        }
        dec_deep_level();
    }

    fput_string( file, "RTU", rtup->fix.name );
    fput_string( file, "Name", obj_p->fix.name );
    fput_string( file, "Info", obj_p->fix.info );
    fput_keywrd( file, "[STOP]", comment );
    fput_newlin( file );
}

int WrASCIICfg( char* cfg_ASCII )
{
    int i, j, ret=0;
    FILE* fcfg;

    if( (fcfg = fopen(cfg_ASCII, "wb")) == NULL )
    {
        return -1;
    }

    out_system( fcfg );

    for( i=0; i < rtdb.no_egu; i++ )
    {
        out_egu( fcfg, rtdb.EGUnit + i );
    }

    for( i=0; i < rtdb.no_state; i++ )
    {
        out_state( fcfg, rtdb.DigState + i );
    }

    for( i=0; i < rtdb.no_command; i++ )
    {
        out_command( fcfg, rtdb.DigCmd + i);
    }

    for( i=0; i<rtdb.no_digdev; i++ )
    {
        out_digdef( fcfg, rtdb.DigDev + i );
    }

    for( i=0; i<rtdb.no_objdev; i++ )
    {
        out_objdef( fcfg, rtdb.ObjDev + i );
    }

    for( i=0; i<rtdb.no_channel; i++ )
    {
        out_channel( fcfg, rtdb.chanet + i );
    }

    for( i=0; i<rtdb.no_rtu; i++ )
    {
        out_rtu( fcfg, rtdb.rtut + i );
    }

    for( i=0; i<rtdb.no_rtu; i++ )
    {
        AINFUT *ainfut = &rtdb.rtut[i].fix.ainfut;
        for( j=0; j < ainfut->total; j++ )
        {
            out_anain( fcfg, ainfut->ain_t + j );
        }
    }

    for( i=0; i<rtdb.no_rtu; i++ )
    {
        AOUTFUT *aofut = &rtdb.rtut[i].fix.aoutfut;
        for( j=0; j < aofut->total; j++ )
        {
            out_anaout( fcfg, aofut->aout_t + j );
        }
    }

    for( i=0; i<rtdb.no_rtu; i++ )
    {
        CNTFUT *cntfut = &rtdb.rtut[i].fix.cntfut;
        for( j=0; j < cntfut->total; j++ )
        {
            out_counter( fcfg, cntfut->cnt_t + j );
        }
    }

    for( i=0; i<rtdb.no_rtu; i++ )
    {
        DIGFUT *digfut = &rtdb.rtut[i].fix.digfut;
        for( j=0; j < digfut->total; j++ )
        {
            out_digital( fcfg, digfut->dig_t + j );
        }
    }

    for( i=0; i<rtdb.no_rtu; i++ )
    {
        OBJFUT *objfut = &rtdb.rtut[i].fix.objfut;
        for( j=0; j < objfut->total; j++ )
        {
            out_object( fcfg, objfut->obj_t + j );
        }
    }

    fput_keywrd( fcfg, "[END]", rtdb.sys_name );

    //kraj:
    fclose( fcfg );
    return ret;
}
