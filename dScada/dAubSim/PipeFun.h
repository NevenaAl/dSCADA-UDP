int OpenPipe( void );
void ClosePipe( void );
void DisconnectClient( void );
int SendToPipe( char *msg );
int WaitForClient( void );
void WaitForCmd( void );
void RefreshClient( void );
void InitClient( void );
int PutRtuSts( char *ime, int val );

