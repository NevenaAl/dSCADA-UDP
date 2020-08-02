/**
*  @file    Common.h
*
*  @brief   This file contains complete RTDB and dSCADA model structure.
*
*/
#ifndef COMMON_INCLUDED
#define COMMON_INCLUDED

// Set all structures to 1-byte alligned
#pragma pack(1)

// Maximum length of SCADA point name
#define NAME_LEN	15
// Maximum length of SCADA point description
#define INFO_LEN	30

/**
* @struct ProcessVariableID
*
* @brief  Represents unique ID for process variables. Each
*         process variable is uniquely defined by its type,
*         RTU it belongs to.
*
*/
typedef struct _ProcessVariableID 
{
    BYTE     type;          // Tip Procesne velicine
    short    rtu;           // Pripadnost PK - RTU (PLC)
    short    sqn;           // Sekvenca
} PVID;

// definicija tipova procesnih promenljivih (PV Types)
enum PvidType
{
    PVT_ANY = 0,      // proizvoljan tip
    PVT_SYS,          // SYSTEM koren svih velicina
    PVT_STN,          // stanica (STATION)
    PVT_CHA,          // CHANET tablica kom. kanala
    PVT_RTU,          // RTUT, tablica RTU-a
    PVT_AI,           // analogni ulaz (ANAIN)
    PVT_AO,           // analogni izlaz (ANAOUT)
    PVT_CNT,          // brojacki ulaz (COUNTER)
    PVT_DIG,          // digitalni uredjaj (DIGITAL)
    PVT_OBJ,          // objekat (OBJECT)
    CAT_EGU,          // EGU jedinice
    CAT_STS,          // digitalno stanje
    CAT_CMD,          // digitalna komanda
    CAT_DIG,          // definicija digitalnog uredjaja
    CAT_OBJ           // definicija objekta
};

// konstante za pristup delovima PVAR struktura
#define ALL_PART  0           // PVAR struktura kao celine
#define VAR_PART  1           // var deo PVAR strukture
#define OPR_PART  2           // operaterski deo PVAR strukture
#define FIX_PART  4           // fiksni deo PVAR strukture

// ofseti za pristup chg flagu
#define CHG_VAR   0
#define CHG_OPR   1

// definicija tipa promene
#define CHG_FINAL 1           // zavrsena promena koja se prenosi do klijenta
#define CHG_SIMUL 3           // simulirana promena koja se mora jos obraditi

/************************************************************************/
//               Definicija konstanti i struktura za kataloge            
/************************************************************************/

// katalog jedinica
typedef struct _egu
{
    char  egu[NAME_LEN];
    short color;
    PVID  mID;
} EguDef;

// katalog stanja
typedef struct _state
{
    char        state[NAME_LEN];
    short       color;
    PVID        mID;
} StateDef;

// katalog komandi
typedef struct _command
{
    char        command[NAME_LEN];
    short       color;
    PVID        mID;
} CommandDef;

// katalog digitalnih uredjaja (tipova)
typedef struct _digdev
{
    char        type[NAME_LEN];
    short       noIn;
    short       noOut;
    SCHAR       states[16];
    SCHAR       commands[16];
    PVID        mID;
} DigDevDef;

// katalog objekata (tipova)

// Maksimalan broj elemenata / parametara jednog objekta
#define MAX_ELEM    30
#define MAX_PARAM   30

// klasa objekta (digitalni ili analogni)
#define DIG_OBJ   0
#define ANA_OBJ   1

// kategorija objekta (realan ili virtualni)
#define REAL_OBJ   0
#define VIRT_OBJ   1

typedef struct _objdef
{
    char        type[NAME_LEN];
    SCHAR       klasa;                 // digitalan ili analogan
    SCHAR       category;              // realan ili virtualni
    short       noElems;
    char        elementDescription[MAX_ELEM][NAME_LEN];
    char        elementType[MAX_ELEM];
    short       noParams;
    char        parameterDescription[MAX_PARAM][NAME_LEN];
    PVID        mID;
} ObjectDef;


// definicija procesnih promenljivih

/************* zajednicko define parce za STS polje u opr delu ***************/
#define OPR_ACT         0x01           // aktivno merenje
#define OPR_MAN_VAL     0x02           // vrednost je rucno postavio operater
#define OPR_EVENT_INH   0x04           // Zabrana generisanja dogadjaja
#define OPR_CMD_INH     0x08           // Zabrana izvrsenja SVIH komandi
#define OPR_MCMD_INH    0x10           // Zabrana izvrsenja RUCNIH komandi
//#define OPR_HISTORY     0x20           // indikator zapisa u istorijat


/******************* struktura za opis STANICE *****************************/

#define MAX_STN      10                // max broj replikacionih stanica
#define MAX_RTUCHN   64                // max broj RTU kanala, ciji se podaci distribuiraju

typedef struct station_var
{
#define STA_VAL      0x01
#define STA_SCAN1    0x02
    WORD  status;      
} StationVar;

typedef struct station_opr
{
    WORD  status;
    // lista RepTo RTU stanica
    short no_repto;
    short stn_repto[MAX_STN];
    // lista RepFrom RTU stanica
    short no_repfrom;
    short stn_repfrom[MAX_STN];
} StationOpr;

typedef struct station_fix
{
    short station_rtu;                 // rtu koji reprezentuje stanicu
    // lista RTU kanala
    short no_rtuchn;                   // broj rtu kanala
    short rtuchn[MAX_RTUCHN];          // lista rtu kanala na stanici
    BYTE  route[MAX_STN];              // routing table per node
    PVID  mID;
    char  name[NAME_LEN];
    char  info[INFO_LEN];     // opis
} StationFix;

typedef struct _station
{
    BYTE       chg[2];                 // change flag for dClient refresh
    BYTE       dChg[MAX_STN][2];       // change flag for data distribution
    StationVar var;
    StationOpr opr;
    StationFix fix;
}  STATION;


/****************** Struktura za kanal ************************/

#define RTU_CHN          256           // max broj RTU na jednom kanalu

typedef union _comprm
{
    struct
    {                          // Za serijski kanal
        short baud;                    // kod brzine
        SCHAR parity;                  // kod parnosti
        SCHAR stop_bits;               // kod broja stop-bita
        SCHAR lenght;                  // kod duzine karaktera
    } uart;
    struct
    {                          // Za TCP/UDP kanal
        bool server;                   // Client ili Server
        short locport;                 // ako je server
    } ip;
} COMPRM;

struct chanet_var
{
    WORD  val         : 1;             // indikator ispravnosti kanala
    WORD  clr_tpor    : 1;             // zahtev za brisanje tpor-a za kanala
};

struct chanet_opr
{
    WORD  status;
    short owner;                       // station that owns this channel
};

struct chanet_fix
{
    SCHAR protocol;                    // tip podataka koji se razmenjuju: RAW, PROCESSED, USERTYPE1, USERTYPE2, ...
    SCHAR type;                        // tip kanala: UART/UDP/TCP
    SCHAR port;                        // logicka adresa porta
    COMPRM comm;                       // COMM port parametri
    short timout;                      // COMM timeout brojac (sec)
    // lista RTU
    short rtu_n;                       // broj rtu-a na kanalu
    short rtu_lst[RTU_CHN];            // lista lrn RTUa na kanalu
    // opsti podaci
    PVID mID;
    char name[NAME_LEN];            // ime kanala
    char info[INFO_LEN];     // puno ime kanala
};


typedef struct chanet_var CHANET_VAR;
typedef struct chanet_opr CHANET_OPR;
typedef struct chanet_fix CHANET_FIX;

typedef struct _chanet
{
    CHANET_VAR var;
    CHANET_OPR opr;
    CHANET_FIX fix;
}  CHANET;


/****************** Strukture za analogne ulaze *************************/

struct anain_var
{
#define AI_VAL       0x0001            // ispravno merenje
#define AI_ORL       0x0002            // ORL je odbroj van odb_opsega
#define AI_HI_ALARM  0x0004            // indikator gornjeg alarma
#define AI_LO_ALARM  0x0008            // indikator donjeg alarma
#define AI_HI_WARN   0x0010            // indikator gornjeg upozorenja
#define AI_LO_WARN   0x0020            // indikator donjeg upozorenja
#define AI_SCAN1     0x0040            // prvo ocitavanje ulaza
#define AI_FORCE     0x0080            // obavezna obrada ulaza

    WORD  status;
    WORD  raw_value;                   // vrednost odbroja A/D
    float egu_value;                   // konvertovana (EU) vrednost merenja
    long  timestamp;                   // Vreme promene
};

struct anain_opr
{
    WORD  status;
    float man_value;                   // vrednost koju je rucno postavio operater
    float egu_range[2];                // opseg fizicke (merene) velicine
    float alarm_lim[2];                // granice alarma u EU
    float warn_lim[2];                 // granice upozorenja u EU
    float deadband;                    // "mrtvi opseg" u EU
};

struct anain_fix
{
    short hwa;                         // fizicka adresa merne tacke
    SCHAR egu_code;                    // EU - fizicka mera procesne velicine ofset u katalogu JM
    SCHAR el_code;                     // EL - mera elektricnog signala senzora ofset u katalogu JM
    float el_range[2];               // elektricni opseg senzora
    WORD  raw_range[2];                // mera fiz i el opsega u A/D odbrojima
    PVID  mID;                         // ID iste strukture
    PVID  objID;                       // ID nadredjenog objekta
    char  name[NAME_LEN];           // simbolicko ime merne tacke
    char  info[INFO_LEN];    // opis merne tacke
};

typedef struct anain_var ANAIN_VAR;
typedef struct anain_opr ANAIN_OPR;
typedef struct anain_fix ANAIN_FIX;

typedef struct anain
{
    BYTE        chg[2];                // indikator promene
    BYTE        dChg[MAX_STN][2];      // change flag for data distribution
    ANAIN_VAR   var;
    ANAIN_OPR   opr;
    ANAIN_FIX   fix;
}  ANAIN;

/******************** Strukture za analogne izlaze **********************/

struct anaout_var
{
#define AO_VAL       0x01              // ispravno merenje
#define AO_HI_ALARM  0x02              // indikator gornjeg alarma
#define AO_LO_ALARM  0x04              // indikator donjeg alarma
#define AO_CMD_REQ   0x08              // u toku je izvrsenje commands
#define AO_SCAN1     0x10              //  prvi prolaz
#define AO_FORCE     0x20              // obavezna obrada

    WORD status;
    WORD  raw_value;                   // u odbrojima
    float egu_value;                   // poslednja (trenutna) vrednost EU
    long  timestamp;                   // Vreme promene
    float req_value;                   // zahtevana vrednost EU
};

struct anaout_opr
{
    WORD  status;
    float man_value;                   // vrednost koju je rucno postavio operater
    float alarm_lim[2];                // granice alarma u EU
    float egu_range[2];                // fizicki opseg
    float clamp_lim[2];                // dozvoljene granice u EU zbog mogucih kvarova
    float max_rate;                    // dozvoljena brzina promene u EU
    float deadband;                    // "mrtvi opseg" u EU
};

struct anaout_fix
{
    short hwa;
    SCHAR egu_code;                    // EU - fizicka mera procesne velicine
    SCHAR el_code;                     // EL - elektricna mera izlaznog signala
    float el_range[2];                 // elektricni opseg izlaza
    WORD  raw_range[2];                // mera fiz i el opsega u D/A odbrojima
    PVID  mID;                         // ID iste strukture
    PVID  objID;                       // ID nadredjenog objekta
    char  name[NAME_LEN];           // simbolicko ime AOUT-a
    char  info[INFO_LEN];    // opis AOUT-a
};

typedef struct anaout_var ANAOUT_VAR;
typedef struct anaout_opr ANAOUT_OPR;
typedef struct anaout_fix ANAOUT_FIX;

typedef struct anaout
{
    BYTE        chg[2];                // indikator promene 
    BYTE        dChg[MAX_STN][2];      // change flag for data distribution
    ANAOUT_VAR  var;
    ANAOUT_OPR  opr;
    ANAOUT_FIX  fix;
}  ANAOUT;

/************************* Strukture za brojace ***************************/
struct counter_var
{
#define CNT_VAL      0x01
#define CNT_STALLED  0x02              // Brojac ne broji
#define CNT_IN_CLEAR 0x04              // Da li je brisanje u toku
#define CNT_SCAN1    0x10
#define CNT_FORCE    0x20

    WORD  status;
    long  raw_value;                   // binarna vrednost brojaca counts
    float egu_value;                   // izracunata vrednost raw_value*const_br
    long  timestamp;                   // Vreme promene
    float frozen_value;                // vrednost u trenutku brisanja
    long  lastclear;                   // Vreme brisanja brojaca
};

struct counter_opr
{
#define OPR_AUTOCLR   0x100            // Brojac se automatski dnevno brise

    WORD  status;
    float man_value;                   // vrednost koju je rucno postavio operater
    float mul_const;                   // multiplikativna konstanta brojaca
    float div_const;                   // deliteljska konstanta brojaca
    BYTE  clrhour;                     // Dnevno vreme brisanja hh:mm
    BYTE  clrmin;                      //                            
    long  watch_period;
    float deadband;
};

struct counter_fix
{
    short hwa;                         // fizicka adresa brojaca
    BYTE  egu_code;                    // egu mere
    PVID  mID;                         // ID iste strukture
    PVID  objID;                       // ID nadredjenog objekta
    char  name[NAME_LEN];           // simbolicko ime brojaca
    char  info[INFO_LEN];    // puno ime brojaca
};

typedef struct counter_var COUNTER_VAR;
typedef struct counter_opr COUNTER_OPR;
typedef struct counter_fix COUNTER_FIX;

typedef struct counter
{
    BYTE        chg[2];                // indikator promene 
    BYTE        dChg[MAX_STN][2];      // change flag for data distribution
    COUNTER_VAR var;
    COUNTER_OPR opr;
    COUNTER_FIX fix;
}  COUNTER;

/******************* Strukture za digitalne uredjaje *********************/

struct digital_var
{
#define DIG_VAL         0x0001
#define DIG_CMD_REQ     0x0002
#define DIG_ALARM       0x0004
#define DIG_IN_SCAN1    0x0100
#define DIG_OUT_SCAN1   0x0200
#define DIG_FORCE       0x0400
#define DIG_IN_SCAN2    0x0800

    WORD  status;
    SCHAR state;                       // stanje dig. ulaza
    SCHAR command;                     // stanje dig. izlaza
    long  timestamp;                   // Vreme promene
    SCHAR cmd_requested;               // Zadnja komanda koja je izdata
    long  cmd_timestamp;               // Vreme zadnje commands
    short cmd_timer;                   // tekuce vreme izvrsenja commands (sec)
};

struct digital_opr
{
    WORD status;
    SCHAR oper_in;                     // stanje koji je rucno postavio operater
};

struct digital_fix
{
    int   type;                        // tip digitalnog uredjaja - offset u katalogu tipova
    short no_in;                       // broj ulaza
    short no_out;                      // broj izlaza
    SCHAR states[16];                  // moguca stanja ulaza ofseti iz kataloga stanja redosledno
    SCHAR commands[16];                // moguci tipovi ofseti iz kataloga komadni komandi
    short hw_in[4];                    // fizicke adrese ulaza Hardwerske
    short hw_out[4];                   // fizicke adrese komandi Hardwerske
    short cmd_timout;                  // vreme za izvrsenje commands (sec)
    PVID  mID;                         // ID iste strukture
    PVID  objID;                       // ID nadredjenog objekta
    char  name[NAME_LEN];           // ime digitalnog uredjaja
    char  info[INFO_LEN];    // opis digitalnog uredjaja
};

typedef struct digital_var DIGITAL_VAR;
typedef struct digital_opr DIGITAL_OPR;
typedef struct digital_fix DIGITAL_FIX;

typedef struct digital
{
    BYTE        chg[2];                // indikator promene
    BYTE        dChg[MAX_STN][2];      // change flag for data distribution
    DIGITAL_VAR var;
    DIGITAL_OPR opr;
    DIGITAL_FIX fix;
}  DIGITAL;


/********************** Strukture za objekte ********************************/
#define N_INT     3                    // Duzina radne zone int-ova 
#define N_FLT     3                    // Duzina radne zone float-a 

struct object_var
{
#define OBJ_VAL      0x01
#define OBJ_ALARM    0x02
#define OBJ_BAD      0x04              // neispravni parametri objekta
#define OBJ_CMD_REQ  0x08

    WORD     status;    
    float    egu_value;                // EU vrednost objekta
    SCHAR    state;                    // stanje objekta
    SCHAR    command;                  // komanda nad objektom
    float    req_value;                // zahtevana vrednost
    long     timestamp;                // Vreme promene
    short    ceo[N_INT];               // radna zona int-ova
    float    flt[N_FLT];               // radna zona float-a
};

struct object_opr
{
    WORD     status;
    float    man_value;                // analogna vrednost koju je rucno postavio operater
    SCHAR    oper_in;                  // stanje koji je rucno postavio operater
    float    param[MAX_PARAM];         // vrednosti parametara
};

struct object_fix
{
    SCHAR    type;                     // tip objekta
    SCHAR    obj_class;                // digitalni ili kontinualni
    SCHAR    category;                 // realan ili virtualni
    SCHAR    egu_code;                 // ofset jedinice mere u vektoru
    short    no_elem;                  // broj elemenata
    short    no_param;                 // broj parametara
    PVID     element[MAX_ELEM];        // elementi objekta
    PVID     mID;                      // pvid iste strukture
    PVID     objID;                    // ID nadredjenog objekta
    char     name[NAME_LEN];        // ime merne tacke
    char     info[INFO_LEN]; // opis merne tacke
};

typedef struct object_var OBJECT_VAR;
typedef struct object_opr OBJECT_OPR;
typedef struct object_fix OBJECT_FIX;

typedef struct object
{
    BYTE        chg[2];                // indikator promene
    BYTE        dChg[MAX_STN][2];      // change flag for data distribution
    OBJECT_VAR  var;
    OBJECT_OPR  opr;
    OBJECT_FIX  fix;
    void        *extMem;               // dodatna memorija
    short       extMemLen;             // Duzina dodatne memorijske zone
} OBJECT;


/******************** Strukture za RTUT promenljivu  **********************/


/******************* Zaglavlje analognih ulaza na RTU-u ******************/
typedef struct anafut
{
    short    total;                    // ukupno analognih tabela
    ANAIN    *ain_t;                   // ukazivac na vektor ANALOG-a
    WORD     *rawdat;                  // ukazivac na RAWDAT buffer (odbroji)
    short    *ain_lt;                  // ANAIN link table
}  AINFUT;


/****************** Zaglavlje analognih izlaza na RTU-u ******************/

typedef struct aoutfut
{
    short    total;                    // ukupno aout tabela
    ANAOUT   *aout_t;                  // vektor ANA_OUT
    WORD     *rawdat;                  // RAW DATA
    short    *aout_lt;                 // ANAOUT link table
}  AOUTFUT;


/************************* Zaglavlje brojaca ***************************/

typedef struct cntfut
{
    short    total;                    // ukupno count tabela
    COUNTER  *cnt_t;                   // ukazivac na prvu strukturu u lancu tipa COUNT
    long     *rawdat;                  // pokazivac na deo RAWDAT-a u kome su smesteni odbroji
    short    *cnt_lt;                  // COUNTER link table
}  CNTFUT;


/******************* Zaglavlje digitalnih uredjaje *********************/

typedef struct digfut
{
    short    total;                    // ukupno digitalnih tabela
    DIGITAL  *dig_t;                   // ukazivac na prvu strukturu u lancu tipa DIGITAL
    BYTE     *rawdat_in;               // ukazivac na deo RAWDAT-a u kome su smesteni digitalni ulazi
    BYTE     *rawdat_out;              // ukazivac na deo RAWDAT-a u kome su smesteni digitalni izlazi
    short    *di_lt;                   // DIG_IN link table
    short    *do_lt;                   // DIG_OUT link table
}  DIGFUT;

/*********************** Zaglavlje objekata *****************************/

typedef struct objfut
{
    short    total;                    // ukupno objekata na RTU
    OBJECT   *obj_t;                   // ukazivac na prvu strukturu u lancu objekata
}  OBJFUT;


/******************* struktura za opis RTU *****************************/

struct rtut_var
{
    WORD  val         : 1;             // indikator ispravnosti RTU-a
    WORD  first_scan  : 1;             // prva prozivka RTU
    WORD  virt        : 1;             // Da li je lazni RTU
    WORD  connected   : 1;             // indikator uspostavljanja veze kanala (TCP)
    short err;                         // brojac tekucih kom. gresaka RTU-a
    long  timestamp;                   // Vreme zadnje promene (uspesne akvizicije)
};

struct rtut_opr
{
    WORD  status;      
    BYTE  acq_period;                  // period akvizicije za sve pI/O
    short owner;                       // station that owns this channel
    //int   history_t;                   // period zapisa statistike
};

struct rtut_fix
{
    char  hname[INFO_LEN];   // ime racunara (host name)
    short hport;                       // TCP/UDP port racunara (host port)
    BYTE  com_adr;                     // komunikaciona adresa RTU-a
    BYTE  channel;                     // broj kanala na kome je RTU implementiran

    short  no_din;                     // broj digitalnih ulaza
    short  no_dout;                    // broj digitalnih izlaza
    short  no_ain;                     // broj analognih ulaza
    short  no_aout;                    // broj analognih izlaza
    short  no_cnt;                     // broj brojaca

    DIGFUT   digfut;                   // za digitalne uredjaje
    AINFUT   ainfut;                   // za analogne ulaze
    AOUTFUT  aoutfut;                  // za analogne izlaze
    CNTFUT   cntfut;                   // za brojace
    OBJFUT   objfut;                   // za objekte

    PVID  mID;
    char  name[NAME_LEN];
    char  info[INFO_LEN];    // opis
};

typedef struct rtut_var RTUT_VAR;
typedef struct rtut_opr RTUT_OPR;
typedef struct rtut_fix RTUT_FIX;

typedef struct _rtut
{
    BYTE     chg;
    BYTE     dChg[MAX_STN];            // change flag for data distribution
    RTUT_VAR var;
    RTUT_OPR opr;
    RTUT_FIX fix;
}  RTUT;


/******************* Implementacija industrijskih protokola *************************/

enum tipovi_protokola
{
    TANDEM_DATA=0,
    REPBUS_DATA,
    MODBUS_DATA,                       // Modbus protokol
    USERTYPE2_DATA,
    USERTYPE3_DATA,
    USERTYPE4_DATA,
    USERTYPE5_DATA,
    USERTYPE6_DATA,
    USERTYPE7_DATA,
    USERTYPE8_DATA,
    PROTOCOL_TYPE_NO
};

typedef struct rtu_procedure
{
    int protocol_type;
    // Inicijalizacija stanice
    int (*init_RTU)       ( int rtu );
    // Inicijalizacija stanice
    void (*free_RTU)      ( int rtu );
    // Ime protokola
    char* (*get_prot_name) ( void );
    // Pribavljanje imena poruke
    char* (*get_msg_name) ( BYTE type );
    // Prozivka RTU-a
    void (*acquisit_rtu)  ( int rtu );
    // Slanje poruke
    int (*send_msg)       ( IORB *iop, BYTE *iobuf, int mlen );
    // Slanje broadcast poruke
    int (*send_bcst_msg)  ( IORB *iop, BYTE *iobuf, int mlen );
    // Slanje commands
    void (*send_command)  ( PVID *pvidp, float cmd );
    // Prijem poruke (unsolicited)
    int (*recv_msg)       ( IORB *iop, MSGID *msg );
    // Prijem karaktera iz prekidne rutine
    int (*get_char)       ( BYTE rb, IORB *iop );
    // Obrada primljene poruke
    void (*proccess_msg)  ( IORB *iop );
    // Poziva se prilikom uspesnog izvrsenja IO zahteva
    void (*on_msg_ok)     ( IORB* iop );
    // Poziva se prilikom neuspesnog izvrsenja IO zahteva
    void (*on_msg_err)    ( IORB* iop );
    // Poziva se pre izvrsenja IO zahteva
    int (*pre_release)    ( IORB* iop );
    // Poziva se po uspostavi veze
    void (*on_connect)    ( IORB* iop );
    // Poziva se po prekidu veze
    void (*on_disconnect) ( IORB* iop );
} RTU_PROCEDURE;

enum
{
    PIO_AI=0,      /* analogni ulaz */
    PIO_AO,        /* analogni izlaz */
    PIO_CNT,       /* brojacki ulaz */
    PIO_DI,        /* digitalni ulaz */
    PIO_DO,        /* digitalni izlaz */
    PIO_NUM        /* broj tipova pIO */
};

// struktura za ukupan broj pojedinih elemenata u konfiguraciji
typedef struct _system
{
    char           sys_name[26];       // Ime sistema ( opis )
    // Katalozi
    short          no_egu;             // Broj egu jedinica u katalogu
    EguDef        *EGUnit;
    short          no_state;           // Broj stanja u katalogu
    StateDef 	   *DigState;
    short          no_command;         // Broj komandi u katalogu
    CommandDef	   *DigCmd;
    short          no_digdev;          // Broj tipova dig. ur. u katalogu
    DigDevDef 	   *DigDev;
    short          no_objdev;          // Broj tipova obj. u katalogu
    ObjectDef     *ObjDev;
    // SCADA procesne promenljive (real-time podaci)
    short          no_channel;         // ukupan broj svih kanala
    CHANET         *chanet;
    short          no_rtu;             // ukupan broj RTUa
    RTUT           *rtut;
    short          no_station;         // ukupan broj STATION-a
    STATION        *station;
}  SYSTEM;


typedef struct
{
    int  sts;         // states
    int  cmd;         // commands
    int  digd;        // digdefs
    int  egu;         // egus
    int  objd;        // objdefs
    int  chn;         // channels
    int  rtu;         // RTUs
    int  stn;         // stations
} SYS_DESCRIPT;

typedef struct
{
    int  ain;         // AnaIns
    int  aout;        // AnaOuts
    int  cnt;         // Counters
    int  dig;         // Digital devices
    int  obj;         // Objects
    int  all;         // all together
} RTU_DESCRIPT;

typedef struct _sta_cfg
{
    char SysFolder[128];        // sa njega cita konfiguraciju
    char StaFolder[128];        // na njega pise log i evente
    char PlcCfg[128];           // ime PlcSim cfg datoteke
    char AubCfg[128];           // ime AubSim cfg datoteke
    char SvgGrp[128];           // ime SVG grafickog fajla

    char StaName[20];
    short MyStaId;
    //STATION *MyStaPnt;
    int  StaType;
    //char TndName[20];
    //int  StaIdx, PeerIndx;
    //bool TndCfg;
    //BYTE TndSts;
} STATION_CFG;


#endif
