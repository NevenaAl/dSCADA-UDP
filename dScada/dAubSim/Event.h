// slanje eventa
void PutEvent( PVID *ppvid, char *format, ... );
void PutStaEvent( int dsId, PVID *ppvid, char *format, ... );
void PutLocEvent( PVID *ppvid, char *format, ... );

void FireEvent( int whereto, char *emsg );

// definicija odredista eventa
// 0 .. N = dsId           // tandem + sqn odedisne stanice : PutStaEvent
#define REP_EVNT   -1      // tandem + RepTo stanice : PutEvent
#define LOC_EVNT   -2      // lokalni event : PutLocEvent

// oznacava primljeni event, koji se ne zapisuju i ne distribuira
#define RCV_EVNT   -3      // tandem + RepTo stanice : PutEvent

