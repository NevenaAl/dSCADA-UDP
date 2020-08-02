#define _Abort() _abort(__FILE__,__LINE__)

#define veto() SetPriority(THREAD_PRIORITY_HIGHEST)
#define permit(priority) SetThreadPriority(GetCurrentThread(),priority)
#define set_priority(priority) permit(priority)

#if !defined( TYPES_DEFINED )
typedef unsigned char  BYTE;
typedef signed char    SCHAR;
typedef unsigned short WORD;
typedef unsigned long  DWORD;
#define TYPES_DEFINED
#endif

/* makroi za postavljanje i brisanje bit - indikatora */
#define set(FlagW,Flag) ( (FlagW) |= (Flag) )
#define reset(FlagW,Flag) ( (FlagW) &= ~(Flag) )
#define get_bit(FlagW,Flag) (((unsigned)((FlagW)&(Flag))==(unsigned)(Flag))?1:0)
#define put_bit(FlagW,Flag,Val) ((Val)?(set( FlagW, Flag )):(reset( FlagW, Flag )))
#define not(FlagW,Flag) ( (FlagW) ~= Flag )
#define xor(FlagW,Flag) ( (FlagW) ^= Flag )

#define OK 		  0					// OK status  
#define NOK		 -1					// NOK status 

/*******************************/
/*      Definicija TIMER-a     */
/*******************************/
#define INFO_TIMER 20

#define MSEC   1
#define TSEC   102
#define SEC    1024
#define MIN    61440
#define HOUR   3686400

typedef struct timer
{
   WORD     act:1;            // aktivan ili ne
   WORD     exp:1;            // istekao ili ne
   WORD     susp:1;           // privremeni suspend
   int      value;            // trenutna vrednost
   int      time_set;         // postavljena vrednost
   int      time_left;        // preostalo vreme
   int      time_ponder;      // vremenska jedinica
   char     info[INFO_TIMER]; // opis timera (cist komentar) 
   struct   timer *next;      // sledeci
   struct   timer *prev;      // prethodni
}  TIMER;

/**************************************/
/*      Rukovanje IO porukama         */
/**************************************/
#define MSG_LEN      256		// duzina pojedinacne poruke u MPOR
#define MBUF_LEN		500 		// broj MPOR-a u MSGBUF

#define END_LIST	0				// na kraj liste 
#define TOP_LIST	1				// na vrh liste 

#define  ERR_SND	1           // greska pri slanju
#define  ERR_RCV	2           // greska pri prijemu
#define  ERR_TIM	3           // greska timout
#define  ERR_CRC	4           // greska CRC
#define  ERR_LEN	5           // greska u duzini poruke
#define  ERR_CONN	6           // greska pri uspostavi konekcije

typedef struct _iorb          // IO Request Block
{
   BYTE     io_request;       // tip zahteva - SEND, RECV, ...
   BYTE     request_on;       // u toku je izvrsenje zahteva
   BYTE     chn;              // kanal kojem se pristupa
   short    rtu;              // logicki broj odredisne tacke
   BYTE     id;               // id poruke
   BYTE     sts;              // status komunikacije
   SCHAR    maxrep;           // max broj zanavljanja poruke
   BYTE     request_type;     // vrsta upita
   int      slen;             // duzina poruke za slanje
   int      no_snd;           // broj poslatih znakova
   BYTE     sbuf[MSG_LEN];    // predajni bafer
   BYTE     reply_type;       // vrsta odgovora
   int      rlen;             // duzina odgovora PK
   int      no_rcv;           // broj primljenih znakova
   BYTE     rbuf[MSG_LEN];    // prijemni bafer
}	IORB;

typedef struct _msgid         // Message ID
{
   BYTE chn;                // neophodan za broadcast poruke, kad je rtu -1
   short rtu;
   BYTE type;
   BYTE id;
} MSGID;

typedef struct _iobuffer      // IO Buffer
{
   HANDLE mutex;              // Mutex za sinhronizaciju pristupa
   short	 head;               // vrh liste 
   short	 tail;               // sa njega se salje 
   short	 count;              // broj spremnih za slanje 
   IORB	 iorb[MBUF_LEN];
} 	IOBUF;

/*******************************************/
/* Tip zahteva preko komunikacionog kanala */
/*******************************************/
#define SEND_RECV    1 
#define RECV_SEND    2
#define SEND         3
#define RECV         4
#define CONNECT      5
#define DISCONNECT   6

// pregled prioriteta kako ih Windows postavlja
//#define THREAD_PRIORITY_TIME_CRITICAL   15
//#define THREAD_PRIORITY_HIGHEST         2
//#define THREAD_PRIORITY_ABOVE_NORMAL    1
//#define THREAD_PRIORITY_NORMAL          0
//#define THREAD_PRIORITY_BELOW_NORMAL    -1
//#define THREAD_PRIORITY_LOWEST          -2
//#define THREAD_PRIORITY_IDLE            -15

enum _stations 
{ 
   STA_AUB=1, STA_APS         // dAUB, AppServer, ... 
};




/*********************/
/* extern definicije */
/*********************/
extern int sys_timer_period;
extern int TimeMS, timeS;
extern HANDLE periodicProcessingTimer, smCOM, smACQ, smPOUT, LogerMTX;

extern bool ServerShutdown;

void wputs( int i, char *format, ... );
void lputs( char *format, ... );
void _abort( char* file, int line );

/**************/
/* stdafx.cpp */
/**************/
void refresh_WD( void );
void disable_WD( void );
void enable_WD( void );
int SetPriority( int priority );
void bsplit( void *dst, BYTE **src, int size );

/**********************/
/* Rukovanje timerima */
/**********************/
void OpenTimerList( int update_period );
void CloseTimerList( void );
TIMER* GetTimer( char *opis );
void DelTimer( TIMER *tp );
void StartTimer( TIMER *timer, int T, int ponder );
void RestartTimer( TIMER *timer );
void StopTimer( TIMER *timer );
void SuspendTimer( TIMER *timer );
void ResumeTimer( TIMER *timer );
int TimeLeftTimer( TIMER *timer, int ponder );
void UpdateTimers( void );

/*************************/
/* Rukovanje ComBuferima */
/*************************/
void OpenIORbuf( void );
void ClearIORbuf( void );       

void Send( MSGID *msg, BYTE *msgbuf, int mlen, int kako=END_LIST );
void Receive( MSGID *msg, int kako=END_LIST );
void SendRawMsg( MSGID *msg, BYTE *msgbuf, int mlen, int kako=END_LIST );

/*********************************/
/* Izvrsava zahtev sa iobuf[tail] */
/*********************************/
void ExecuteIOReq( void );

// zapis u LOG
void PutLogMessage( char *format, ... );

int ReadConfigFile( void );
