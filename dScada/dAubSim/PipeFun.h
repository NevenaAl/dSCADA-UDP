int OpenPipe( void );
void ClosePipe( void );
void DisconnectClient( void );
void ConnectClient(void);
int SendToPipe( char *msg );
int WaitForMessage( void );
void RefreshClient( void );
void InitClient( void );
int PutRtuSts( char *ime, int val );

