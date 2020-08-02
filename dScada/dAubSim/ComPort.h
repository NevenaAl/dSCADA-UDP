// podrzani comm portovi
enum _port_types
{
  UART = 0, UDP, TCP, PORTS_NUM
};

// podrzano maksimalno 8 portova jednog tipa
#define MAX_PORTS 8

// struktura komunikacionog porta
typedef struct comport
{
   int port_type;                               // UART, UDP, TCP
   int (*open_com_port)( CHANET *chp );
   int (*connect_com_port)( int rtu );
   void (*disconnect_com_port)( int rtu );
   int (*close_com_port)( CHANET *chp );
   int (*put_msg)( IORB *msg );
   int (*get_msg)( IORB *msg );
} COMPORT;

extern char err_str[7][20];

// metode svakog od podrzanih portova
int OpenUartPort( CHANET *chp );
int CloseUartPort( CHANET *chp );
int putUartMsg( IORB *msg );
int getUartMsg( IORB *msg );

int OpenUdpPort( CHANET *chp );
int CloseUdpPort( CHANET *chp );
int putUdpMsg( IORB *msg );
int getUdpMsg( IORB *msg );

int OpenTcpPort( CHANET *chp );
int ConnectTcpPort( int rtu );
void DisconnectTcpPort( int rtu );
int CloseTcpPort( CHANET *chp );
int putTcpMsg( IORB *msg );
int getTcpMsg( IORB *msg );


/***********************************/
/* Rukovanje komunikacionim portom */
/***********************************/
void OpenComPorts( void );
int ConnectComPort( int rtu );
void DisconnectComPort( int rtu );
void CloseComPorts( void );
int putMsg( IORB *msg );
int getMsg( IORB *msg );

// standardne funkcije za rukovanje komunikacionim statusom RTUa
void on_rtu_ok( RTUT *rtup );
void on_rtu_err( RTUT *rtup );
