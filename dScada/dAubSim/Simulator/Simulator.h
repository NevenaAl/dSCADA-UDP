// iz processing.cpp
void analog_input_alarm(ANAIN *ai);
bool counter_input_alarm( COUNTER *cp, bool chg );

void CheckSpontaneousChange( DIGITAL *digp );
void RestoreLastCommand( DIGITAL *digp );

// iz SimLib.cpp
#define SET_ERROR 1
void ssetPVID( PVID *pvidp, float egu_value, SCHAR stanje, int set_error = 0 );
void ssetPVAR( char *rtu_name, char* pvar_name, float egu_value, SCHAR stanje, int set_error = 0 );
float frand( float value, float range_min, float range_max );
int irand( int value, int range_min, int range_max );

// iz Simulator.cpp
void SimulatorRTU( int rtu );
void SetSimulation( void );
void RunSimulation( int Tsim );

// iz SimRandom.cpp
void randAI( ANAIN *ainp );
void randCNT( COUNTER *cntp );
void randDigDev( DIGITAL *digp );

// iz SimPlant.cpp
void SetPlantSimulator( void );
void SetPlantDigDev( DIGITAL *digp );
void RunPlantSimulator( int Tsim );
void MakeCmdResponse( DIGITAL *digp );


