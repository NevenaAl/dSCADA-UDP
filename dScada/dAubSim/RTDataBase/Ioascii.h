/**
 *  @file:   Ioascii.h
 *
 *  @brief:  Contains function declaration for configuration
 *           initialization.
 *
 */
#ifndef IOASCII_H
#define IOASCII_H

typedef struct
{
   char* key_word;
   int   key_code;
} KEYWRD;

/**
* @brief:    Reads ASCII configuration and initializes internal
*            model.
*
* @param:    configurationFile
*
* @return:   0 for success, otherwise appropriate error code.
*
*/
int ReadAsciiConfiguration(char* configurationFile);

/**
* @brief:    Initializes global counting variables.
*
*/
void InitializeConfiguration(void);
/**
* @brief:    Frees internal model allocations.
*
*/
void FreeConfigurationAllocations(void);

/**
* @brief:    Finds provided key and returns it's position.
*
* @param:    buffer [in] - Ascii buffer to scan for keyword.
*
* @param:    keywor_table [in]    - keyword table to loop trough.
*
* @return:   Id of found key or -1 if key was not found.
*
*/
int FindKey(char *buffer, KEYWRD *keywor_table);

int MultCfg( int mult_factor );
int WrASCIICfg( char* cfg_ASCII );

#endif