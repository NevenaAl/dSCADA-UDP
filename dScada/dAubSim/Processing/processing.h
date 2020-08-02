/**
*  @file  Processing.h
*
*  @brief Declaration of functions for processing of process variables.
*
*/

#ifndef PROCESSING_H
#define PROCESSING_H

#pragma region

/**
* @brief     Processes provided analog input. rawValue is used when point
*            is decimal (integer) and euValue if point is floating point.
*
* @param     analogInput [in/out] - Analog input processing should be done for.
*            rawValue    [in]     - decimal value that should be processed.
*            euValue     [in]     - floating point value that should be processed.
* @return    None.
*
*/
void ProcessAnalogInput(ANAIN *analogInput, int rawValue, float euValue);
/**
* @brief     Function for analog output processing. rawValue is used when point
*            is decimal (integer) and euValue if point is floating point.
*
* @param     analogOutput [in/out] - Analog output processing should be done for.
*            rawValue     [in]     - decimal value that should be processed.
*            euValue      [in]     - floating point value that should be processed.
*
* @return    None.
*
*/
void ProcessAnalogOutput(ANAOUT *analogOutput, WORD rawValue, float euValue);
/**
* @brief     Function calculates next value for analog output taking in
*            consideration analog value maximum change step.
*
* @param     analogOutput [in/out] - Analog output processing should be done for.
*
* @return    Calculated next analog output value.
*
*/
float CalculateAnalogCommand(ANAOUT *analogOutput);

#pragma endregion FUNCTIONS_FOR_PROCESSING_ANALOGS

#pragma region

/**
* @brief     Processes provided counter. rawValue is used when point
*            is decimal (integer) and euValue if point is floating point.
*
* @param     counter  [in/out] - Counter processing should be done for.
*            rawValue [in]     - decimal value that should be processed.
*            euValue  [in]     - floating point value that should be processed.
* @return    None.
*
*/
void ProcessCounter(COUNTER *counter, long rawValue, float euValue);
/**
* @brief     Function executes when counter clear command has successfully
*            executed.
*
* @param     counter  [in/out] - Counter clear was executed for.
*
* @return    None.
*
*/
void OnClearCounterDone(COUNTER* counter);

#pragma endregion FUNCTIONS_FOR_PROCESSING_COUNTERS

#pragma region

/**
* @brief     Function gets raw digital input data for provided rtu
*            at offset address.
*
* @param     rtu [in]       - RTU id digital input is read for.
*            address [in]   - digital input bit offset
*
* @return    Requested digital input value (1 or 0).
*
*/
int GetRawDigitalInput(int rtu, int address);
/**
* @brief     Function sets raw digital input data for provided rtu
*            at offset diAddress.
*
* @param     rtu [in]       - RTU id digital input is set for.
*            diAddress [in] - digital input bit offset
*
* @return    None.
*
*/
void PutRawDigitalInput(int rtu, int diAddress, int di_bit);
/**
* @brief     Function processes digital input.
*
* @param     digitalInput [in] - Digital input to process.
*
* @return    None.
*
*/
void ProcessDigitalInput(DIGITAL *digitalInput);

#pragma endregion FUNCTIONS_FOR_PROCESSING_DIGITAL_INPUTS

#pragma region
/**
* @brief     Gets digital output value for provided RTU and
*            and digital output at provided address.
*
* @param     rtu [in]   - RTU address.
*            address    - DO address.
*
* @return    Digital output value.
*
*/
int GetDigitalOutputValue(int rtu, int address);
/**
* @brief     Sets DO to value at address address for provided
*            RTU.
*
* @param     rtu [in]        - RTU address.
*            address [in]    - DO address.
*            value [in]      - value to set.
*
* @return    Void.
*
*/
void SetDigitalOutputValue(int rtu, int address, int value);
/**
* @brief     Function executes digital output processing.
*
* @param     digital [in]    - Digital device output should be processed for.
*
* @return    Void.
*
*/
void ProcessDigitalOutput(DIGITAL *digital);
/**
* @brief     Returns raw command for provided digital device and command in
*            string format.
*
* @param     digital [in] - Digital device command belongs to.
*            command [in] - command raw value should be found.
*
* @return    Raw command value.
*
*/
int GetRawCommand(DIGITAL *digital, SCHAR command);

#pragma endregion FUNCTIONS_FOR_PROCESSING_DIGITAL_OUTPUTS

#pragma  region

// Following flags represent commanding options.
#define CMD_PUT_EVENT      0x0001         // Generate event for request.
#define CMD_SKIP_PROC      0x0002         // Skip secondary processing.
#define CMD_MAN_REQ        0x0004         // Manually executed command.
/**
* @brief     Executes command for provided pvid.
*
* @param     pvid [in]       - process variable identificator to execute command.
*            value [in]      - value to command.
*            options [in]    - commanding options.
*            origin_stn [in] -
*
* @return    0 for success, -1 otherwise.
*
*/
int PutCommand(PVID *pvid, float value, WORD options, short origin_stn = -1);
/**
* @brief     Clears counter. This functions just wrapps PutCommand().
*
* @param     pvid [in]       - process variable identificator to execute command.
*            options [in]    - commanding options.
*            origin_stn [in] -
*
* @return    0 for success, -1 otherwise.
*
*/
int PutClearCounter(PVID *pvid, WORD cmdopt, short origin_stn = -1);

#pragma endregion FUNCTIONS_FOR_RTU_COMMANDING

#pragma region
/**
* @brief     Function checks if requested commands are executed
*            successfully. It loops trough all devices on each
*            RTU and checks if sent command is executed successfully.
*
* @return    Void.
*
*/
void CommandInspector(void);

#pragma endregion FUNCTIONS_FOR_COMMANDING_VALIDATION

#pragma region

/**
* @brief     Loops trough all objects and calls application
*            specific ProcessObject.
*
* @return    Void.
*
*/
void ObjectsProcessor(void);

/**
* @brief     Sets object value.
*
* @param     object [in]   - Object whose state is to be set.
*            eguValue [in] - Egu value to set.
*            state [in]    - State to set.
*            command [in]  - Command to execute.
*            setchg [in]  - Set the change flag and distribute
*
* @return    true for change done, false otherwise.
*
*/
bool SetObjectValue(OBJECT *obj, float egu_value, SCHAR state, SCHAR command, bool setchg = true);

#pragma endregion FUNCTIONS_FOR_OBJECT_PROCESSING

#pragma region
/**
* @brief     Sets manual value for provided pvid.
*
* @param     pvid [in]  - process variable identificator to execute command.
*            value [in] - velue to set.
*
* @return    Void.
*
*/
void SetManualValue(PVID *pvid, float value);
/**
* @brief     Clears manual flag.
*
* @param     pvid [in]       - process variable identificator for clear flag for.
*
* @return    Void.
*
*/
void ResetManualFlag(PVID *pvid);
/**
* @brief     Executes validation and eventually calls SetManualValue().
*
* @param     pvid [in]       -
*            value [in]      -
*            set_on [in]     -
*            origin_stn [in] -
*
* @return
*
*/
int PutManualValue(PVID *pvid, float value, bool set_on, short origin_stn = -1);
/**
* @brief     Puts tag on pvid.
*
* @param     pvid [in]       - Process variable identificator to tag.
*            tags [in]       - Tags to place in bit-wise mask.
*            origin_stn [in] -
*
* @return
*
*/
int PutTags(PVID *pvid, WORD tags, short origin_stn = -1);

#pragma endregion MANUAL_COMMANDS_FROM_CLIENT

#endif

