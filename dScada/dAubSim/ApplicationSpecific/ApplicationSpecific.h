/**
*  @file   ApplicationSpecific.h
*
*  @brief  Function prototypes for application specific support.
*
*/

#pragma once

/**
* @brief     Restores last command.
*
* @param     digp [in]  -  pointer to digital device last command
*                          should be restored
*
* @return    void.
*
*/
void RestoreLastCommand( DIGITAL *digp );

/**
* @brief     Checks if spontaneous change happened.
*
* @param     digp [in]  - pointer to digital device spontaneous change
*                         should be checked
*
*/
void CheckSpontaneousChange( DIGITAL *digp );

/**
* @brief     Validates command before execution.
*
* @param     digp [in]  - Pointer to digital device command should be
*                         validated for.
*
*            cmd [in]   - Command that should be validate.
*
* @return    OK for successful validation, NOK otherwise.
*
*/
int ValidateDigitalCommand( DIGITAL *digp, SCHAR cmd);

/**
* @brief     Checks if requested command was executed.
*
* @param     digp [in]  - pointer to digital device command execution
*                         should be checked.
*
* @return    OK if command succeeded, NOK otherwise.
*
*/
int CheckCommandExecution(DIGITAL *digp);

/**
* @brief     Executes object processing after command execution inspection
*            inspection and control procedures. This command is called
*            for each defined object.
*
* @param     digp [in]  -  pointer to digital device spontaneous change
*                          should be checked
*
* @return    true if command succeeded, false otherwise
*
*/
bool ProcessObject( OBJECT *obj );

/**
* @brief     Checks if requested object command was executed.
*
* @param     obj  [in]    - Pointer to object to validate.
*
*            command [in] - Command to validate.
*
* @return    true if command succeeded, false otherwise
*
*/
bool ValidateObjectCommand( OBJECT *obj, SCHAR command );

/**
* @brief     Validates object state.
*
* @param     obj  [in]    - Pointer to object to validate.
*
*            status [in] -  Command that should be validate.
*
* @return    true if validation passed, false otherwise.
*
*/
bool ValidateObjectState( OBJECT *obj, SCHAR status );

/**
* @brief     Sets initial device commands state.
*
* @return    OK if command succeeded, NOK otherwise.
*
*/
int SetCommandInitialState( void );

/**
* @brief     Control procedure called after commands are inspected.
*
*/
void AutoProcedure( void );
