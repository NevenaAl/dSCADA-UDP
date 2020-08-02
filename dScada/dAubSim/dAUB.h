#ifndef DAUB_HEADER_FILE
#define DAUB_HEADER_FILE

/**
* @brief:    Task that periodically processes acquisited variables:
*            - are commands executed successfully,
*            - automatic procedures,
*            - objects processing
*
* @param:    pData [in] - not used but is neede by CreateThread().
*
* @return:   When finished - zero.
*
*/
DWORD WINAPI PeriodicProcessingTask( LPVOID pData );
/**
* @brief:    Task that handles requests and responses with
*            devices on the field.
*
* @param:    pData [in] - not used but is neede by CreateThread().
*
* @return:   When finished - zero.
*
*/
DWORD WINAPI CommunicationTask( LPVOID pData );
/**
* @brief:    Task responsible for state synchronization between
*            this boxes partner.
*
* @param:    pData [in] - not used but is neede by CreateThread().
*
* @return:   When finished - zero.
*
*/
DWORD WINAPI AcquisitionTask( LPVOID pData );

/**
* @brief:    Task handles client requests like commanding.
*
* @param:    pData [in] - not used but is neede by CreateThread().
*
* @return:   When finished - zero.
*
*/
DWORD WINAPI PipeTask( LPVOID pData );
/**
* @brief:    RefreshTask
*
* @param:    pData [in] - not used but is neede by CreateThread().
*
* @return:   When finished - zero.
*
*/
DWORD WINAPI RefreshTask( LPVOID pData );

#endif