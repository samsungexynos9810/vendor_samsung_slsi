#ifndef EWLTECONNECTOR_H
#define EWLTECONNECTOR_H

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup EWLTECON Expway Lte Modem Connector interface.
 *
 * @{
 *
 * Expway LTE Modem Connector API
 *
 * This interface exposes structures and functions to integrate the LTE Modem Connector.
 * This file defines the API between the Middleware and the MBMS Modem Service (MS)
 * \n\n
 */

/**
 * Value returned when the modem do not have network time information.
 * @see EwLTECON_TimeCallback_t
 */
#define EWLTECON_NO_NETWORK_TIME_AVAILABLE ((uint64_t)(-1))

/**
 * Enumeration of response status
 */
typedef enum
{
    /** Operation not supported. */
    EWLTECON_NOT_SUPPORTED = -2,

    /** Operation failed due to an error. */
    EWLTECON_ERROR = -1,

    /** Operation is success. */
    EWLTECON_SUCCESS =  0,

} EwLTECON_Status_t;

/**
 * Enumeration of modem status.
 * @see EwLTECON_ModemStatusCallback_t
 */
typedef enum
{
	/** Device is ready
	Device is accepting commands, such as enabling eMBMS service.
	Typically, this status is used at initialization, or when device is back
	from #EWLTECON_MS_DEVICE_OFF. */
    EWLTECON_MS_READY,

	/** Device is ready, and eMBMS service is enable.
	Typically this status is used after enabling procedure is SUCCESS */
    EWLTECON_MS_EMBMS_ENABLE,

	/** Device is ready, and eMBMS service is disable and could be enabled
	Typically this status is used after disabling procedure is SUCCESS */
    EWLTECON_MS_EMBMS_DISABLE,

	/** Device is not ready.
	All commands will fail until the status changes back to #EWLTECON_MS_READY.
    The status may change to #EWLTECON_MS_READY due to a reason outside the scope of this API.
	Typically this status is used in case of flight mode or modem booting or modem disconnected. */
    EWLTECON_MS_DEVICE_OFF,

} EwLTECON_ModemStatus_t;

/**
 * Enumeration of LTE Coverage State.
 *
 * @see EwLTECON_CoverageCallback_t
 */
typedef enum
{
    /** LTE off, or dropped and searching */
    EWLTECON_NO_COVERAGE,

    /** LTE unicast available, but no eMBMS */
    EWLTECON_UNICAST_COVERAGE,

    /** LTE unicast and eMBMS available */
    EWLTECON_FULL_COVERAGE,

} EwLTECON_CoverageState_t;

/**
 * Enumeration of TMGI status.
 *
 * @see EwLTECON_SessionListUpdatedCallback_t
 */
typedef enum
{
    /** TMGI state is ACTIVATED */
    EWLTECON_TMGI_ACTIVATED,

    /** TMGI state is AVAILABLE */
    EWLTECON_TMGI_AVAILABLE,

    /** TMGI state is Out Of Service */
    EWLTECON_TMGI_OOS,

} EwLTECON_TMGIStatus_t;

/**
 * Enumeration of MBMS Session Control Type.
 *
 * @see EwLTECON_SessionControlCallback_t
 */
typedef enum
{
    /** Control to activate Session */
    EWLTECON_SESSION_ACTIVATE,

    /** Control to deactivate Session     */
    EWLTECON_SESSION_DEACTIVATE,

} EwLTECON_ControlType_t;

/**
 * Enumeration of signal information types
 *
 * @see EwLTECON_SignalInformation_t
 */
typedef enum
{
    EWLTECON_SI_BSSI,   /**< Broadcast Signal Strength Indicator (in dBm). int32Val value   */
    EWLTECON_SI_RSSI,   /**< Received Signal Strength Indicator (in dBm). int32Val value    */
    EWLTECON_SI_SNR,    /**< Signal to Noise Ratio (in dB). int32Val value                  */
    EWLTECON_SI_BLER,   /**< Block Error Rate (in percentage). doubleVal value              */
    EWLTECON_SI_RSRP,   /**< Reference Signal Receive Power (in dBm). int32Val value        */
    EWLTECON_SI_RSRQ,   /**< Reference Signal Receive Quality (in dB). int32Val value       */
    EWLTECON_SI_COUNT__ /**< (enumeration item count)                                       */
} EwLTECON_SignalInformationType_t;

/**
 * Type definition for Signal information structure
 *
 * The signal information structure is composed of a signal type and an union field.
 * According to the signal type eSignalType (see #EwLTECON_SignalInformationType_t),
 * the user can access the right value of the union.
 *
 * @see EwLTECON_SignalInformationCallback_t
 */
typedef struct
{
    EwLTECON_SignalInformationType_t eSignalType;   /**< Type of provided modem signal */
    union
    {
        int32_t int32Val;   /**< Signed 32 bit   */
        uint32_t uint32Val; /**< Unsigned 32 bit */
        int32_t int64Val;   /**< Signed 64 bit   */
        uint64_t uint64Val; /**< Unsigned 64 bit */
        float floatVal;     /**< Float           */
        double doubleVal;   /**< Double          */
    } value;
} EwLTECON_SignalInformation_t;

/**
 * Type definition for TMGI.
 *
 * The TMGI (Temporary Mobile Group Identity) is the key identifier of a session.
 * The TMGI is encoded as unsigned integer including octets 3 to 8 of the TMGI 
 * definition in 3GPP "TS 24.008" (chapter 10.5.6.13).
 * Refer to 3GPP "TS 26.346" chapter 7.3.2.7 for how the conversion to a integer 
 * is done.
 */
typedef uint64_t EwLTECON_TMGI_t;

/**
 * Type definition for SAI, the Service Area Identity. 
 *
 * @see EwLTECON_AvailablityInfo_t
 */
typedef uint32_t EwLTECON_SAI_t;

/**
 * Type definition for Frequency encoded as a LTE EARFCN value.
 *
 * Although the EARFCN is on 16 bits today (0-65535)
 * we define it over 32 bits here as there are discussion to extend
 * it in future LTE release. The API is then future proof.
 */
typedef uint32_t EwLTECON_EARFCN_t;

/**
 * Type definition of information to provide when ativate/deactivate session (TMGI)
 *
 * @see EwLTECON_ActivateSession(), EwLTECON_DeactivateSession(), EwLTECON_SwitchSession()
 */
typedef struct
{
    /** The TMGI of the session to activate or deactivate. */
    EwLTECON_TMGI_t uTMGI;

    /** The number of SAI in which the session is available.
    Size of pSAIList array. */
    uint32_t uSAICount;

    /** The array of SAI in which the session is available. */
    EwLTECON_SAI_t *pSAIList;

    /** The number of frequency in which the session is available.
    Size of pFreqList array. */
    uint32_t uFreqCount;

    /** The array of frequencies in which the session is available. */
    EwLTECON_EARFCN_t *pFreqList;

}EwLTECON_AvailablityInfo_t;

/**
 * Type definition for timing to provide to modem for managing hysteresis delay.
 *
 * @see EwLTECON_ConfigureHysteresisDelays()
 */
typedef struct
{
    /** Hysteresis delay applied when a TMGI disappears from
    the list of available sessions.
    In milliseconds */
    uint32_t uSessionOutDelayMs;

    /** Hysteresis delay applied when coverage status changes to #EWLTECON_FULL_COVERAGE
    In milliseconds */
    uint32_t uMbmsInDelayMs;

    /** Hysteresis delay applied when coverage status changes from #EWLTECON_FULL_COVERAGE 
    to any other coverage state
    In milliseconds */
    uint32_t uMbmsOutDelayMs;

}EwLTECON_HysteresisDelays_t;

/**
 * Handle on a LTE modem connector.
 *
 * @see EwLTECON_Initialize()
 */
typedef void* EwLTECON_Handle_t;


/**
 * Prototype for callback function that receives modem status events
 * and the modem interface name.
 *
 * This callback will be invoked first at initialization time.
 *
 * Then, this callback will be invoked as a response to EwLTECON_EnableService() call
 * or EwLTECON_DisableService() call and also whenever the modem state change due to
 * an external reason.
 *
 * The modem interface information MUST be set when the status is #EWLTECON_MS_EMBMS_ENABLE.
 * In other modem status it is optional.
 *
 * @param[in] pUserData         User data passed at initialization time.
 * @param[in] eState            Modem state.
 * @param[in] pInterfaceName    The modem interface name, or NULL if this information is not available.
 * @param[in] nInterfaceIndex   The modem interface index, or 0 if this information is not available.
 */
typedef void EwLTECON_ModemStatusCallback_t(void *pUserData,
                                            EwLTECON_ModemStatus_t eState,
                                            char *pInterfaceName,
                                            int nInterfaceIndex);

/**
 * Prototype for callback function that receives coverage information events
 *
 * This callback will be invoked first after the modem status is #EWLTECON_MS_EMBMS_ENABLE.
 *
 * Then, this callback will be invoked whenever the coverage state changes due to
 * an external reason.
 *
 * After the modem status is no longer #EWLTECON_MS_EMBMS_ENABLE, this callback will not be invoked.
 *
 * @param[in] pUserData     User data passed at initialization time.
 * @param[in] eState        Coverage state.
 */
typedef void EwLTECON_CoverageCallback_t(void *pUserData,
                                         EwLTECON_CoverageState_t eState);

/**
 * Prototype for callback function that receives network information
 *
 * This callback will be invoked first after the modem status is #EWLTECON_MS_EMBMS_ENABLE.
 *
 * Then, this callback will be invoked whenever the network information changes due to
 * an external reason.
 *
 * After the modem status is no longer #EWLTECON_MS_EMBMS_ENABLE, this callback will not be invoked.
 *
 * @param[in] pUserData     User data passed at initialization time.
 * @param[in] iMcc          The MCC (Mobile Country Code) value.
 * @param[in] iMnc          The MNC (Mobile Network Code) value.
 * @param[in] iCellid       The Cell identifier.
 */
typedef void EwLTECON_NetworkInformationCallback_t(void *pUserData,
                                                   int32_t iMcc,
                                                   int32_t iMnc,
                                                   int32_t iCellid);

/**
 * Prototype for callback function that receives the list of Service Area Identifiers.
 *
 * This callback will be invoked first after the modem status is #EWLTECON_MS_EMBMS_ENABLE.
 *
 * Then, this callback will be invoked whenever the network information changes due to
 * mobility (eMBMS area change) or change in broadcasted content.
 *
 * After the modem status is no longer #EWLTECON_MS_EMBMS_ENABLE, this callback will not be invoked.
 *
 * @param[in] pUserData       User data passed at initialization time.
 * @param[in] uNbCampedSAI    Number of elements in pCampedSAI array.
 * @param[in] pCampedSAI      Array of camped SAIs (intra SAIs).
 *                            Array of SAIs on the frequencies on which the device is camped to.
 * @param[in] uNbGroup        Number of elements in pNbSAIperGroup array.
 * @param[in] pNbSAIperGroup  Array of number of available SAIs per group.
 *                            A Group is a set of SAIs on which TMGI's can be activated at same time.
 * @param[in] uNbAvailableSAI Number of elements in pAvailableSAI array.
 *                            This is equal to sum of the values of pNbSAIperGroup.
 * @param[in] pAvailableSAI   Array of all available SAIs per group aligned to the values of pNbSAIperGroup.
 */
typedef void EwLTECON_ServiceAreaListUpdatedCallback_t(void *pUserData,
                                                       uint32_t uNbCampedSAI,
                                                       EwLTECON_SAI_t *pCampedSAI,
                                                       uint32_t uNbGroup,
                                                       uint32_t *pNbSAIperGroup,
                                                       uint32_t uNbAvailableSAI,
                                                       EwLTECON_SAI_t *pAvailableSAI);


/**
 * Prototype for callback function that receives the list of eMBMS sessions.
 *
 * When notifiying the list of #EWLTECON_TMGI_AVAILABLE sessions:
 * This callback will be invoked first after the modem status is #EWLTECON_MS_EMBMS_ENABLE.
 * Then, this callback will be invoked whenever the list of available sesssions
 * changes due to mobility (eMBMS area change) or change in broadcasted content.
 *
 * When notifiying the list of #EWLTECON_TMGI_ACTIVATED sessions:
 * This callback will be invoked whenever the list of activated sesssions
 * changes due to mobility (eMBMS area change) or change in broadcasted content
 * or successful call to EwLTECON_ActivateSession() or EwLTECON_DeactivateSession()
 * or EwLTECON_SwitchSession()
 *
 * When notifiying the list of #EWLTECON_TMGI_OOS sessions:
 * This callback will be invoked whenever the list of out of service sesssions
 * changes due change in broadcasted content.
 *
 * After the modem status is no longer #EWLTECON_MS_EMBMS_ENABLE, this callback will not be invoked.
 *
 * @param[in] pUserData     User data passed at initialization time.
 * @param[in] eTMGIStatus   TMGI Status for the whole list.
 * @param[in] uNbTMGI       Number of element in the paTMGI array
 * @param[in] pTMGI         Pointer on TMGIs list array.
 */
typedef void EwLTECON_SessionListUpdatedCallback_t(void *pUserData,
                                                   EwLTECON_TMGIStatus_t eTMGIStatus,
                                                   uint32_t uNbTMGI,
                                                   EwLTECON_TMGI_t *pTMGI);

/**
 * Prototype for callback function that receives the result of TMGI control service.
 *
 * This callback will be invoked as a response to EwLTECON_ActivateSession() or 
 * EwLTECON_DeactivateSession() or EwLTECON_SwitchSession() calls to eMBMS service.
 *
 * @param[in] pUserData     User data passed at initialization time.
 * @param[in] eControlType  The session control type.
 * @param[in] eStatus       The Result of TMGI control action.
 * @param[in] uTMGI         The TMGI.
 */
typedef void EwLTECON_SessionControlCallback_t(void *pUserData,
                                               EwLTECON_ControlType_t eControlType,
                                               EwLTECON_Status_t eStatus,
                                               EwLTECON_TMGI_t uTMGI );

/**
 * Prototype for callback function that receives signal level information events.
 *
 * This callback will be invoked first after the modem status is #EWLTECON_MS_EMBMS_ENABLE.
 *
 * Then, this callback will be invoked whenever the signal information changes.
 *
 * After the modem status is no longer #EWLTECON_MS_EMBMS_ENABLE, this callback will not be invoked.
 *
 * @param[in] pUserData         User data passed at initialization time.
 * @param[in] uNbSignalInfo     The number of element in paSignalInfo array.
 * @param[in] paSignalInfo      The available signal information array.
 */
typedef void EwLTECON_SignalInformationCallback_t(void *pUserData,
                                                  uint32_t uNbSignalInfo,
                                                  EwLTECON_SignalInformation_t *paSignalInfo);

/**
 * Prototype for callback function that receives the eMBMS time in microseconds
 * since 1970/01/01 according on air time information (SIB16).
 *
 * This callback will be invoked first after the modem status is #EWLTECON_MS_EMBMS_ENABLE.
 *
 * Then, this callback will be invoked as a response to a successful EwLTECON_GetTime() call.
 *
 * @param[in] pUserData User data passed at initialization time.
 * @param[in] uTime     The eMBMS time in microseconds.
 *                      The value #EWLTECON_NO_NETWORK_TIME_AVAILABLE is returned
 *                      When the network time is not available.
 */
typedef void EwLTECON_TimeCallback_t(void *pUserData,
                                     uint64_t uTime);

/**
 * Callback configuration for LTE Connector.
 *
 * IMPORTANT: The LTE Connector implementation must call the callback functions
 *            without any mutex / lock held. Also, the LTE Connector implementation
 *            must not wait for callback function end.
 *
 * @see EwLTECON_Initialize()
 */
typedef struct
{
    EwLTECON_ModemStatusCallback_t              *pModemStatusCB;                /**< Modem status callback              */
    EwLTECON_CoverageCallback_t                 *pCoverageCB;                   /**< Coverage state callback            */
    EwLTECON_NetworkInformationCallback_t       *pNetworkInformationCB;         /**< Network information Callback       */
    EwLTECON_ServiceAreaListUpdatedCallback_t   *pServiceAreaListUpdatedCB;     /**< SAIs list update Callback.          */
    EwLTECON_SessionListUpdatedCallback_t       *pSessionListUpdatedCB;         /**< Session list updated callback      */
    EwLTECON_SessionControlCallback_t           *pSessionControlCB;             /**< Session list updated callback      */
    EwLTECON_SignalInformationCallback_t        *pSignalInformationCB;          /**< Signal information callback        */
    EwLTECON_TimeCallback_t                     *pTimeCB;                       /**< eMBMS Time callback                */

} EwLTECON_Callbacks_t;

/**
 * Configuration for LTE modem Connector.
 *
 * @see EwLTECON_Initialize()
 */
typedef struct
{
    /** Remote modem network interface address.
    May be NULL. */
    const char *szRemoteModemNetworkInterface;

    /** Remote modem network interface port number. */
    uint32_t uRemoteModemNetworkInterfacePort;

    /** Callbacks functions. */
    EwLTECON_Callbacks_t stCallbacks;

    /** Caller userdata */
    void *pCallbacksUserData;

} EwLTECON_Configuration_t;

/**
 * Initialize the LTE Connector Handle and register all Callbacks
 *
 * @param[out] pHandle  Handle of the LTE Connector (NULL if fails).
 * @param[in]  pConfig  Configuration parameters.
 *
 * @return
 *    - #EWLTECON_SUCCESS
 *          Action taken by eMBMS service is successful.
 *          The modem status at initialization time shall be notified.
 *    - #EWLTECON_ERROR
 *          Action taken by eMBMS service has failed.
 *
 * @see EwLTECON_ModemStatusCallback_t
 */
EwLTECON_Status_t EwLTECON_Initialize(EwLTECON_Handle_t *pHandle,
                                      const EwLTECON_Configuration_t *pConfig);

/**
 * Destroy the LTE Connector Handle and deregister all Callbacks.
 *
 * @param[in] hHandle The LTE Connector Handle.
 *
 * @return
 *    - #EWLTECON_SUCCESS
 *          Action taken by eMBMS service is successful.
 *    - #EWLTECON_ERROR
 *          Action taken by eMBMS service has failed.
 */
EwLTECON_Status_t EwLTECON_Destroy(EwLTECON_Handle_t hHandle);

/**
 * Enable the eMBMS service.
 *
 * @param[in] hHandle The LTE Connector Handle.
 *
 * @return
 *    - #EWLTECON_SUCCESS
 *          Action taken by eMBMS service is accepted. The result of the action
 *          is notified in a callback.
 *    - #EWLTECON_ERROR
 *          Action taken by eMBMS service has failed and is not accepted.
 *
 * @see EwLTECON_ModemStatusCallback_t
 */
EwLTECON_Status_t EwLTECON_EnableService(EwLTECON_Handle_t hHandle);

/**
 * Disable the eMBMS device.
 *
 * @param[in] hHandle The LTE Connector Handle.
 *
 * @return
 *    - #EWLTECON_SUCCESS
 *          Action taken by eMBMS service is accepted. The result of the action
 *          is notified in a callback.
 *    - #EWLTECON_ERROR
 *          Action taken by eMBMS service has failed and is not accepted.
 *
 * @see EwLTECON_ModemStatusCallback_t
 */
EwLTECON_Status_t EwLTECON_DisableService(EwLTECON_Handle_t hHandle);

/**
 * Activate an MBMS Session.
 *
 * @param[in] hHandle   The LTE Connector Handle.
 * @param[in] uPriority The preemption priority of the MBMS Session.
 *                      It can take a value from 0 (Lowest Priority) to 3 (Highest Priority).
 *                      Any value greater than 3 will be considered as 3 and any value
 *                      less than 0 will be considered as 0.
 * @param[in] pInfo     Availability information about the given MBMS Session
 *
 * @return
 *    - #EWLTECON_SUCCESS
 *          Action taken by eMBMS service is accepted. The result of the action
 *          is notified in a callback.
 *    - #EWLTECON_ERROR
 *          Action taken by eMBMS service has failed and is not accepted.
 *
 * @see EwLTECON_SessionControlCallback_t
 * @see EwLTECON_SessionListUpdatedCallback_t
 */
EwLTECON_Status_t EwLTECON_ActivateSession(EwLTECON_Handle_t hHandle,
                                           uint8_t uPriority,
                                           EwLTECON_AvailablityInfo_t *pInfo);

/**
 * Deactivate an MBMS Session.
 *
 * @param[in] hHandle   The LTE Connector Handle.
 * @param[in] pInfo     Availability information about the given MBMS Session
 *
 * @return
 *    - #EWLTECON_SUCCESS
 *          Action taken by eMBMS service is accepted. The result of the action
 *          is notified in a callback.
 *    - #EWLTECON_ERROR
 *          Action taken by eMBMS service has failed and is not accepted.
 *
 * @see EwLTECON_SessionControlCallback_t
 * @see EwLTECON_SessionListUpdatedCallback_t
 */
EwLTECON_Status_t EwLTECON_DeactivateSession(EwLTECON_Handle_t hHandle,
                                             EwLTECON_AvailablityInfo_t *pInfo);

/**
 * Switch Session used to activate a MBMS Session and deactivate another MBMS Session in the same call.
 *
 * @param[in] hHandle    The LTE Connector Handle.
 * @param[in] uPriority  The preemption priority of TMGI.
 *                       It can take a value from 0 (Lowest Priority) to 3 (Highest Priority).
 *                       Any value greater than 3 will be considered as 3 and any value
 *                       less than 0 will be considered as 0.
 * @param[in] pActInfo   Availability information about the MBMS Session to activate
 * @param[in] pDeactInfo Availability information about the MBMS Session to deactivate
 *
 * @return
 *    - #EWLTECON_SUCCESS
 *          Action taken by eMBMS service is accepted. The result of the action
 *          is notified in a callback.
 *    - #EWLTECON_ERROR
 *          Action taken by eMBMS service has failed and is not accepted.
 *
 * @see EwLTECON_SessionControlCallback_t
 * @see EwLTECON_SessionListUpdatedCallback_t
 */
EwLTECON_Status_t EwLTECON_SwitchSession(EwLTECON_Handle_t hHandle,
                                         uint8_t uPriority,
                                         EwLTECON_AvailablityInfo_t *pActInfo,
                                         EwLTECON_AvailablityInfo_t *pDeactInfo);

/**
 * Request the eMBMS time in microseconds since 1970/01/01 according on
 * air time information (SIB16).
 *
 * @param[in] hHandle The LTE Connector Handle.

  * @return
 *    - #EWLTECON_SUCCESS
 *          Action taken by eMBMS service is accepted. The result of the action
 *          is notified in a callback.
 *    - #EWLTECON_ERROR
 *          Action taken by eMBMS service has failed and is not accepted.
 *    - #EWLTECON_NOT_SUPPORTED
 *          Action taken by eMBMS service has failed due to unsupported request
 **/
EwLTECON_Status_t EwLTECON_GetTime(EwLTECON_Handle_t hHandle);

/**
 * Configure the modem hysteresis delays.
 *
 * @param[in] hHandle           The LTE Connector Handle.
 * @param[in] pHysteresisDelays The Pointer to the hysteresis configuration.
 *
 * @return
 *    - #EWLTECON_SUCCESS
 *          Action taken by eMBMS service is successful.
 *    - #EWLTECON_ERROR
 *          Action taken by eMBMS service has failed.
 *    - #EWLTECON_NOT_SUPPORTED
 *          Action taken by eMBMS service has failed due to unsupported request
 */
EwLTECON_Status_t EwLTECON_ConfigureHysteresisDelays(EwLTECON_Handle_t hHandle,
                                                     EwLTECON_HysteresisDelays_t *pHysteresisDelays);

/**
 * Configure the watchdog timeout delays for the modem reconnection.
 *
 * @param[in] hHandle           The LTE Connector Handle.
 * @param[in] uWatchdogDelayMs  Watchdog delay in milliseconds.
 *
 * @return
 *    - #EWLTECON_SUCCESS
 *          Action taken by eMBMS service is successful.
 *    - #EWLTECON_ERROR
 *          Action taken by eMBMS service has failed.
 *    - #EWLTECON_NOT_SUPPORTED
 *          Action taken by eMBMS service has failed due to unsupported request
 */
EwLTECON_Status_t EwLTECON_ConfigureWatchdogDelay(EwLTECON_Handle_t hHandle,
                                                  uint32_t uWatchdogDelayMs);

/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* EWLTECONNECTOR_H */
