int OpenPipe( void );
void ClosePipe( void );
void DisconnectClient( void );
void ConnectClient(void);
int SendToPipe( char *msg );
int WaitForMessage( void );
void RefreshClient( void );
void InitClient( void );
void QueueCheck(void);
void ResendMessage(char* seq_number);
int PutRtuSts( char *ime, int val );

