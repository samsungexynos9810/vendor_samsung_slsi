/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef SIT_RIL_GPS_DEF_H
#define SIT_RIL_GPS_DEF_H


typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long long  u64;

typedef signed char         s8;
typedef signed short        s16;
typedef signed int          s32;
typedef signed long long    s64;

#pragma pack(1)
#define PACKED_STRUCT struct
#define PACKED_UNION  union
//#define PACKED_UNION __packed union


/*-----------------------------------------------------------------------*/
/* Defines                          */
/*-----------------------------------------------------------------------*/
#define SIT_CPMO_LOC_CLIENT_ID_LENGTH               82
#define SIT_CPMO_MLCNUM_LENGTH                      82
#define SIT_SAT_INFO_SIZE                           15

#define SIT_DECIPHER_LENGTH                         7
#define SIT_GPS_TOW_ASSIST_LENGTH                   12
#define SIT_GPS_APDU_LENGTH                         63

#define SIT_SATELLITE_ID_LENGTH                     16
#define SIT_GPS_ELEMENT_LENGTH                      16
#define SIT_NAVIGATION_SATELLITE_INFO_LENGTH        16
#define SIT_ALMANAC_SATELLITE_INFO_LENGTH           64

#define SIT_LPP_CAPABILITIES_A_GNSS                             0x01
#define SIT_LPP_CAPABILITIES_EPDU                               0x02
#define SIT_LPP_ABORT                                           0x01
#define SIT_LPP_ERROR                                           0x02
#define SIT_ASSIST_DATA_NO_SUPPORT_SERVER                       0x01
#define SIT_ASSIST_DATA_SUPPROT_BUT_NO_AVAIL_SERVER             0x02
#define SIT_ASSIST_DATA_PARTLY_NO_SUPPORT_NO_AVAIL_SERVER       0x03
#define SIT_NOT_ENOUGH_SATELLITES                               0x04
#define SIT_ASSIST_DATA_MISSING                                 0x05
#define SIT_NOT_ALL_REQ_MEASURE_POSSIBLE                        0x06
#define SIT_LPP_MAX_SV_CNT                                      16
#define SIT_BM_KLOBUCHAR_MODEL                                  0x01
#define SIT_BM_NEQUICKMODEL                                     0x02
#define SIT_LPP_MAX_AGNSS_CNT                                   0x02

#define SIT_SUPL_OTDOA_REQ_CAP                      0x01
#define SIT_SUPL_OTDOA_PROV_ASSIST                  0x02
#define SIT_SUPL_OTDOA_REQ_LOC                      0x04
#define SIT_SUPL_ECID_REQ_CAP                       0x08
#define SIT_SUPL_ECID_REQ_LOC                       0x10
#define SIT_SUPL_OTDOA_TRIGGER_REQ_ASSIST           0x20

#define SIT_SUPL_OTDOA_PROV_CAP                     0x01
#define SIT_SUPL_OTDOA_REQ_ASSIST                   0x02
#define SIT_SUPL_OTDOA_PROV_LOC                     0x04
#define SIT_SUPL_ECID_PROV_CAP                      0x08
#define SIT_SUPL_ECID_PROV_LOC                      0x10

#define SIT_LTE_ECID_REQ_CAP    0x01
#define SIT_LTE_ECID_REQ_LOC    0x02

#define SIT_MAX_MRL_LIST        8

#define SIT_BM_NOT_USED    0x00
#define SIT_BM_RSRP        0x01
#define SIT_BM_RSRQ        0x02
#define SIT_BM_TA          0x04
#define SIT_BM_MRL_LIST    0x08
#define SIT_BM_EARFCN      0x10


typedef u8 sitGpsApReleaseGpsNoti;
typedef u8 sitGpsResetAsstDataNoti;


/*-----------------------------------------------------------------------*/
/* Enumerations                          */
/*-----------------------------------------------------------------------*/
typedef enum
{
    SIT_WCDMA_FREQ_INFO_FDD = 0x01,
    SIT_WCDMA_FREQ_INFO_TDD = 0x02,
}sitGpsWcdmaFreqInfoFlag;

typedef enum
{
    SIT_WCDMA_CELLMEAS_RESULT_FDD = 0x01,
    SIT_WCDMA_CELLMEAS_RESULT_TDD = 0x02,
}sitGpsWcdmaCellMeasResultFlag;

typedef enum
{
    SIT_ECID_BM_NW_MEAS_RESULT = 0x01,
    SIT_ECID_BM_TA = 0x02,
}sitGpsGsmCellInfoBitMask;

typedef enum
{
    SIT_BM_FREQUENCY_INFO = 0x01,
    SIT_BM_PRIMARY_SCRAMBLING_CODE = 0x02,
    SIT_BM_MEASURED_RESULTS_LIST = 0x04,
    SIT_BM_CELL_PARAMETERS_ID = 0x08,
    SIT_BM_TIMING_ADVANCE = 0x10,
}sitGpsWcdmaCellInfoBitMask;

typedef enum
{
    SIT_WCDMA_ECID_PROV_CAP = 0x01,
    SIT_WCDMA_ECID_PROV_LOC = 0x02,
}sitGpsWcdmaEcidProvFlag;

typedef enum
{
    SIT_WCDMA_ECID_REQ_CAP = 0x01,
    SIT_WCDMA_ECID_REQ_LOC = 0x02,

}sitGpsGetRRLPEcidFlag;

typedef enum
{
    SIT_GSM_CELL_INFO_TYPE = 0x01,
    SIT_WCDMA_CELL_INFO_TYPE
}sitGpsRrlpEcidCellInfoType;

/* GPS Assistance Data Flags */
typedef enum
{
    SIT_GPS_ASSISTANCE_NONE = 0x00000000,
    SIT_GPS_ASSISTANCE_ALMANAC = 0x00000001,
    SIT_GPS_ASSISTANCE_UTC_MODEL = 0x00000002,
    SIT_GPS_ASSISTANCE_IONO_MODEL = 0x00000004,
    SIT_GPS_ASSISTANCE_NAVI_MODEL = 0x00000008,
    SIT_GPS_ASSISTANCE_DGPS_CORRECTION = 0x00000010,
    SIT_GPS_ASSISTANCE_REF_LOCATION = 0x00000020,
    SIT_GPS_ASSISTANCE_REF_TIME = 0x00000040,
    SIT_GPS_ASSISTANCE_ACQUISITION_ASSISTANCE = 0x00000080,
    SIT_GPS_ASSISTANCE_REAL_TIME_INTEGRITY = 0x00000100,
}sitGpsAssistanceDataFlag;

typedef enum
{
    SIT_BM_N_CGI_INFO = 0x01,
    SIT_BM_N_RSRP = 0x02,
    SIT_BM_N_RSRQ = 0x04,
    SIT_BM_N_EARFCN = 0x08,
}sitGpsEutraInfoBitMask;

/* MOLR Type */
typedef enum
{
    SIT_GPS_MOLR_INVALID,
    SIT_GPS_MOLR_LOCATION_ESTIMATE,
    SIT_GPS_MOLR_ASSISTANCE_DATA,
    SIT_GPS_MOLR_DECIPHERING_KEY
} sitGpsMolrType;

/* Response Time Type */
typedef enum
{
    SIT_GPS_RESPONSE_TIME_INVALID,
    SIT_GPS_RESPONSE_TIME_LOW_DELAY,
    SIT_GPS_RESPONSE_TIME_DELAY_TOLERANT
} sitGpsResponseTime;

/* Location Method Type */
typedef enum
{
    SIT_GPS_LOCATION_METHOD_INVALID,
    SIT_GPS_LOCATION_METHOD_MSBASED_EOTD,
    SIT_GPS_LOCATION_METHOD_MSASSISTED_EOTD,
    SIT_GPS_LOCATION_METHOD_ASSISTED_GPS
} sitGpsLocationMethodType;

/* GAD Shape Type */
typedef enum
{
    SIT_GPS_SHAPE_ELLIPSOIDPOINT,
    SIT_GPS_SHAPE_ELLIPSOIDPOINT_WITH_UNCERTAINTY_CIRCLE,
    SIT_GPS_SHAPE_ELLIPSOIDPOINT_WITH_UNCERTAINTY_ELLIPSE,
    SIT_GPS_SHAPE_POLYGON,
    SIT_GPS_SHAPE_ELLIPSOIDPOINT_WITH_ALTITUDE,
    SIT_GPS_SHAPE_ELLIPSOIDPOINT_WITH_ALTITUDE_AND_UNCERTAINTYELI,
    SIT_GPS_SHAPE_ELLIPSOIDPOINT_ARC
} sitGpsGadShapeType;

/* GPS Carrier Type*/
typedef enum
{
    SIT_GPS_CARRIER_INVALID,
    SIT_GPS_CARRIER_GSM,
    SIT_GPS_CARRIER_UMTS
} sitGpsCarrierType;

/*Utran Sfn Uncertainty*/
typedef enum
{
    SIT_GPS_UTRAN_SFN_UNCERTAINTY_INVALID,
    SIT_GPS_UTRAN_SFN_UNCERTAINTY_LESSTHAN_10,
    SIT_GPS_UTRAN_SFN_UNCERTAINTY_MORETHAN_10
} sitGpsSfnUncType;

/* UTRAN Choice mode Type*/
typedef enum
{
    SIT_GPS_UTRAN_CHOICE_INVALID,
    SIT_GPS_UTRAN_CHOICE_FDD,
    SIT_GPS_UTRAN_CHOICE_TDD
} sitGpsUtrnChoiceModeType;


/* DGPS Status Type*/
typedef enum
{
    SIT_GPS_DGPS_INVALID,
    SIT_GPS_DGPS_UDRE_SCALE_1_0,
    SIT_GPS_DGPS_UDRE_SCALE_0_75,
    SIT_GPS_DGPS_UDRE_SCALE_0_5,
    SIT_GPS_DGPS_UDRE_SCALE_0_3,
    SIT_GPS_DGPS_UDRE_SCALE_0_2,
    SIT_GPS_DGPS_UDRE_SCALE_0_1,
    SIT_GPS_DGPS_NO_DATA
} sitGpsDgpsStatusType;

/* Navigation Satellite Stauts Type*/
typedef enum
{
    SIT_GPS_NAVIGATION_MODEL_NEW_SATELLITE_NEW_NAVIGATION,
    SIT_GPS_NAVIGATION_MODEL_EXIST_SATELLITE_EXIST_NAVIGATION,
    SIT_GPS_NAVIGATION_MODEL_EXIST_SATELLITE_NEW_NAVIGATION,
    SIT_GPS_NAVIGATION_MODEL_RESERVED
} sitGpsNavigationSatStatusType;

/* Method Type*/
typedef enum
{
    SIT_GPS_METHODTYPE_INVALID,
    SIT_GPS_METHODTYPE_MS_ASSISTED,
    SIT_GPS_METHODTYPE_MS_BASED,
    SIT_GPS_METHODTYPE_MS_BASED_PREF,
    SIT_GPS_METHODTYPE_MS_ASSISTED_PREF
} sitGpsMethodType;

/* Use Multiple Sets Type*/
typedef enum
{
    SIT_GPS_MULTIPLESETS_INVALID,
    SIT_GPS_MULTIPLESETS_MULTIPLESETS,
    SIT_GPS_MULTIPLESETS_ONESET
} sitGpsUseMultiSetsType;

/* Environment Char Type*/
typedef enum
{
    SIT_GPS_ENVIRONMENT_INVALID,
    SIT_GPS_ENVIRONMENT_BAD_AREA,
    SIT_GPS_ENVIRONMENT_NOT_BAD_AREA,
    SIT_GPS_ENVIRONMENT_MIXED_AREA
} sitGpsEnvCharType;

/* Cell Timing Wanted Type*/
typedef enum
{
    SIT_GPS_CELLTIMING_INVALID,
    SIT_GPS_CELLTIMING_WANTED,
    SIT_GPS_CELLTIMING_NOT_WANTED
} sitGpsCellTimingWntType;

/* Additional Assist Request Type*/
typedef enum
{
    SIT_GPS_ADDITIONAL_ASSISREQ_INVALID,
    SIT_GPS_ADDITIONAL_ASSISREQ_REQ,
    SIT_GPS_ADDITIONAL_ASSISREQ_NOT_REQ
} sitGpsAddAssitReqType;

typedef enum
{
    SIT_GPS_MSR_MULTIPATH_NOT_MEASURED,
    SIT_GPS_MSR_PMULTIPATH_LOW,
    SIT_GPS_MSR_MULTIPATH_MEDIUM,
    SIT_GPS_MSR_MULTIPATH_HIGH
}sitMultiPathType;

/* MTLR Notification Type*/
typedef enum
{
    SIT_GPS_NO_NOTIFY_NO_VERIFY,
    SIT_GPS_USER_NOTIFY_ONLY,     /* Location Notification Allowed */
    SIT_GPS_USER_NOTIFY_VERIFY_ALLOW_NO_RESP, /* notify and verify, if no response, Location Notification Allowed */
    SIT_GPS_USER_NOTIFY_VERIFY_NOT_ALLOW_NO_RESP, /* notify and verify, if no response, Location Notification Not Allowed */
    SIT_GPS_PRIVACY_NEEDED,
    SIT_GPS_PRIVACY_OVERRIDE
} sitGpsMtlrNotify;

/* location estimate Type*/
typedef enum
{
    SIT_GPS_ESTIMATE_TYPE_CURRENT_LOCATION,    ///< Current Location
    SIT_GPS_ESTIMATE_TYPE_CURRENT_OR_LAST_KNOWN_LOCATION,    ///< Current or Last Known Location
    SIT_GPS_ESTIMATE_TYPE_INITIAL_LOCATION,        ///< Initial Location
    SIT_GPS_ESTIMATE_TYPE_ACTIVATE_DEFERRED_LOC,    ///< Activate Deferred Location
    SIT_GPS_ESTIMATE_TYPE_CANCEL_DEFERRED_LOC,    ///< Cancel Deferred Location
    SIT_GPS_ESTIMATE_TYPE_NOTIFY_VERIFY_ONLY,        ///< Notification and Verification Only
    SIT_GPS_ESTIMATE_TYPE_INVALID
} sitGpsLocEstimate;

/* ID Format Type */
typedef enum
{
    SIT_GPS_FORMAT_IND_LOGICAL_NAME,
    SIT_GPS_FORMAT_IND_EMAIL_ADDRESS,
    SIT_GPS_FORMAT_IND_MSISDN,
    SIT_GPS_FORMAT_IND_URL,
    SIT_GPS_FORMAT_IND_SIPURL,
    SIT_GPS_FORMAT_IND_IMS_PUBLIC_ID
} sitGpsFormatInd;

/* Verify Response Type */
typedef enum
{
    SIT_GPS_VERIFY_RSP_TYPE_DENIED,
    SIT_GPS_VERIFY_RSP_TYPE_GRANTED,
    SIT_GPS_VERIFY_RSP_TYPE_INVALID
} sitGpsVerifyRsp;

typedef enum
{
    SIT_AGNSS_CAPABILITY_TYPE_NOT_USED,
    SIT_AGNSS_CAPABILITY_TYPE_BM_SUPPORT_LIST,
    SIT_AGNSS_CAPABILITY_TYPE_BM_ASSIST_SUPPORT,
    SIT_AGNSS_CAPABILITY_TYPE_BM_LOCATION_COORDINATE_TYPE = 0x04,
    SIT_AGNSS_CAPABILITY_TYPE_BM_VELOCITY_TYPE = 0x08
} sitAgnssCapabilityBitMask;

typedef enum
{
    SIT_AGNSS_HORIZENTAL_VELOCITY = 0x01,
    SIT_AGNSS_HORIZENTAL_WITH_VERTICAL_VELOCITY = 0x02,
    SIT_AGNSS_HORIZENTAL_VELOCITY_WITH_UNCERTAINTY = 0x04,
    SIT_AGNSS_HORIZENTAL_WITH_VERTICAL_VELOCITY_AND_UNC = 0x08,
} sitAgnssVelocityType;

typedef enum
{
    SIT_AGNSS_ELEMENT_TYPE_NOT_USED,
    SIT_AGNSS_ELEMENT_TYPE_BM_SBAS_IDS,
    SIT_AGNSS_ELEMENT_TYPE_BM_FTA_MEASURE_SUPPORT,
}sitGnssElementTypeBitMask;

typedef enum
{
    SIT_FTA_MEAS_SUPP_CELLTIME_TYPE_EUTRA,
    SIT_FTA_MEAS_SUPP_CELLTIME_TYPE_UTRA = 0x02,
    SIT_FTA_MEAS_SUPP_CELLTIME_TYPE_GSM = 0x04,
}sitFtaMeasSuppTypeCellTime;

typedef enum
{
    SIT_FTA_MEAS_SUPP_TYPE_NOT_USED,
    SIT_FTA_MEAS_SUPP_TYPE_BM_CELL_TIME,
    SIT_FTA_MEAS_SUPP_TYPE_BM_POSITION_MODE,
}sitFtaMeasSuppTypeMask;

typedef enum
{
    SIT_GNSS_SBAS_ID_TYPE_WASS=0x00,
    SIT_GNSS_SBAS_ID_TYPE_EGNOS=0x01,
    SIT_GNSS_SBAS_ID_TYPE_MSAS=0x02,
    SIT_GNSS_SBAS_ID_TYPE_GAGAN=0x04,
    SIT_GNSS_SBAS_ID_TYPE_RESERVED=0x10
}sitGnssSbasIdType;

typedef enum
{
    SIT_GNSS_POSITION_MODE_STANDALONE = 0x01,
    SIT_GNSS_POSITION_MODE_UE_BASED = 0x02,
    SIT_GNSS_POSITION_MODE_UE_ASSISTED = 0x04,
    SIT_GNSS_POSITION_MODE_RESERVED
}sitGnssPositionMode;

typedef enum
{
    SIT_GNSS_LPP_ERROR_NONE=0x00,
    SIT_GNSS_LPP_ERROR_CMN=0x01,
    SIT_GNSS_LPP_ERROR_AGNSS=0x02,
    SIT_GNSS_LPP_ERROR_OTDOA=0x04,
    SIT_GNSS_LPP_ERROR_ECID=0x08,
    SIT_GNSS_LPP_ERROR_EPDU=0x10,
}sitLppErrorSetFlagType;

typedef enum
{
    SIT_GNSS_LPP_LOC_TYPE_NONE=0x00,
    SIT_GNSS_LPP_LOC_TYPE_CMN=0x01,
    SIT_GNSS_LPP_LOC_TYPE_AGNSS=0x02,
    SIT_GNSS_LPP_LOC_TYPE_OTDOA=0x04,
    SIT_GNSS_LPP_LOC_TYPE_ECID=0x08,
    SIT_GNSS_LPP_LOC_TYPE_EPDU=0x10,
}sitLppReqLocInfoIndType;

typedef enum
{
    SIT_GNSS_SRV_ERROR_CAUSE = 0x01,
    SIT_GNSS_DEV_ERROR_CAUSE,
}sitAGnssErrorType;

typedef enum
{
    SIT_GNSS_CAUSE_NOT_USED = 0x00,
    SIT_GNSS_CAUSE_BM_FTA_MEASURE_NOT_POSSIBLE = 0x01,
    SIT_GNSS_CAUSE_BM_ADR_MEASUREMENTS_NOT_POSSIBLE = 0x02,
    SIT_GNSS_CAUSE_BM_MULTI_FREQUENCY_MEASURE_NOT_POSSIBLE = 0x04
}sitTargetDeviceErrorCausesBitMask;

typedef enum
{
    SIT_GNSS_DEVICE_ERROR_CAUSE_UNDEFINED = 0x01,
    SIT_GNSS_DEVICE_ERROR_CAUSE_NOT_ENOUGH_SATELLITE,
    SIT_GNSS_DEVICE_ERROR_CAUSE_ASSIST_DATA_MISSING,
    SIT_GNSS_DEVICE_ERROR_CAUSE_NOT_ALL_REQUESTED_MEAS_POSSIBLE
}sitTargetDeviceErrorCause;

typedef enum
{
    SIT_GNSS_IE_REQUEST_LOC_INFO_TYPE_NOT_USED =0x00,
    SIT_GNSS_IE_REQUEST_LOC_INFO_TYPE_BM_TRIGGERED_REPORTING =0x01,
    SIT_GNSS_IE_REQUEST_LOC_INFO_TYPE_PERIODICAL_REPORTING =0x02,
    SIT_GNSS_IE_REQUEST_LOC_INFO_TYPEBM_ADDITIONAL_INFORMATION =0x04,
    SIT_GNSS_IE_REQUEST_LOC_INFO_TYPE_BM_QOS =0x08,
    SIT_GNSS_IE_REQUEST_LOC_INFO_TYPE_BM_ENVIRONMENT=0x10,
    SIT_GNSS_IE_REQUEST_LOC_INFO_TYPE_BM_LOCATION_COORDINATE_TYPES=0x20,
    SIT_GNSS_IE_REQUEST_LOC_INFO_TYPEE_BM_VELOCITY_TYPES =0x40
}sitCmnIeReqLocInfoType;

typedef enum
{
    SIT_GNSS_LOC_INFO_TYPE_LOC_ESTIMATE_REQUIRED=0x01,
    SIT_GNSS_LOC_INFO_TYPE_LOC_MEAS_REQUIRED,
    SIT_GNSS_LOC_INFO_TYPE_LOC_ESTIMATE_PREFERRED,
    SIT_GNSS_LOC_INFO_TYPE_LOC_MEAS_PREFERRED,
}sitLocInfoType;

typedef enum
{
    SIT_GNSS_REP_AMOUNT_RA_1=0x01,
    SIT_GNSS_REP_AMOUNT_RA_2,
    SIT_GNSS_REP_AMOUNT_RA_4,
    SIT_GNSS_REP_AMOUNT_RA_8,
    SIT_GNSS_REP_AMOUNT_RA_16,
    SIT_GNSS_REP_AMOUNT_RA_32,
    SIT_GNSS_REP_AMOUNT_RA_64,
    SIT_GNSS_REP_AMOUNT_RA_INFINITY,
}sitRepAmount;

typedef enum
{
    SIT_GNSS_REP_INTERVAL_NO_REP=0x01,
    SIT_GNSS_REP_INTERVAL_RI_0_25,
    SIT_GNSS_REP_INTERVAL_RI_0_5,
    SIT_GNSS_REP_INTERVAL_RI_1,
    SIT_GNSS_REP_INTERVAL_RI_2,
    SIT_GNSS_REP_INTERVAL_RI_4,
    SIT_GNSS_REP_INTERVAL_RI_8,
    SIT_GNSS_REP_INTERVAL_RI_16,
    SIT_GNSS_REP_INTERVAL_RI_32,
    SIT_GNSS_REP_INTERVAL_RI_64,
}sitRepInterval;

typedef enum
{
    SIT_GNSS_LPP_ASSISTANCE_NONE=0x00,
    SIT_GNSS_LPP_ASSISTANCE_CMN=0x01,
    SIT_GNSS_LPP_ASSISTANCE_A_GNSS=0x02,
    SIT_GNSS_LPP_ASSISTANCE_OTDOA=0x04,
    SIT_GNSS_LPP_ASSISTANCE_EPDU=0x08
}sitGnssAssistDataFlag;

typedef enum
{
    SIT_GNSS_ONLY_RETURN_INFO_REQ=0x01,
    SIT_GNSS_MAY_RETURN_ADDITIONAL_INFO,
}sitGnssAddInfoType;

typedef enum
{
    SIT_GNSS_ENV_TYPE_BAD_AREA=0x01,
    SIT_GNSS_ENV_TYPE_NOT_BAD_AREA,
    SIT_GNSS_ENV_TYPE_MIXED_AREA,
}sitEnvType;

typedef enum
{
    SIT_GNSS_BM_QOS_TYPE_HORIZONTAL_ACCURACY=0x01,
    SIT_GNSS_BM_QOS_TYPE_VERTICAL_ACCURACY=0X02,
    SIT_GNSS_BM_QOS_TYPE_RESPONSE_TIME=0x04,
}sitBmQosTypeBitmask;

typedef enum
{
    NOT_USED,
    BM_SBAS_ID,
    BM_TIME_MODELS_SUPPORT,
    BM_DIFF_CORRECT_SUPPORT = 0x04,
    BM_NAV_MODEL_SUPPORT = 0x08,
    BM_RTI_SUPPORT = 0x10,
    BM_DATA_BIT_ASSIST_SUPPORT = 0x20,
    BM_ACQ_ASSIST_SUPPORT = 0x40,
    BM_ALM_SUPPORT = 0x80,
    BM_UTC_MODEL_SUPPORT = 0x100,
    BM_AUXILIARY_INFO_SUPPORT = 0x200
}sitGnssGenAssistDataSuppElementType;

typedef enum
{
    SIT_GNSS_ID_TYPE_GPS,
    SIT_GNSS_ID_TYPE_SBASS,
    SIT_GNSS_ID_TYPE_QZSS,
    SIT_GNSS_ID_TYPE_GALILEO,
    SIT_GNSS_ID_TYPE_GLONASS,
    SIT_GNSS_ID_TYPE_RESERVED
}sitGnssIdType;

typedef enum
{
    SIT_NAVMODEL_SUPPORT_NOT_USED,
    SIT_NAVMODEL_SUPPORT_BM_CLOCK_MODEL,
    SIT_NAVMODEL_SUPPORT_BM_ORBIT_MODEL
}sitGnssNavigationModelSupportType;

typedef enum
{
    SIT_GNSS_NAVI_CLOCK_MODEL1 = 0x00,
    SIT_GNSS_NAVI_CLOCK_MODEL2 = 0x01,
    SIT_GNSS_NAVI_CLOCK_MODEL3 = 0x02,
    SIT_GNSS_NAVI_CLOCK_MODEL4 = 0x04,
    SIT_GNSS_NAVI_CLOCK_MODEL5 = 0x08
}sitGnssNavigationModelClockModelType;

typedef enum
{
    SIT_GNSS_NAVI_ORBIT_MODEL1 = 0x00,
    SIT_GNSS_NAVI_ORBIT_MODEL2 = 0x01,
    SIT_GNSS_NAVI_ORBIT_MODEL3 = 0x02,
    SIT_GNSS_NAVI_ORBIT_MODEL4 = 0x04,
    SIT_GNSS_NAVI_ORBIT_MODEL5 = 0x08
}sitGnssNavigationModelOrbitModelType;

typedef enum
{
    SIT_ACQ_ASSIST_SUPPORT_NOT_USED = 0x00,
    SIT_ACQ_ASSIST_SUPPORT_BM_CONFIDENCE_SUPPORT = 0x01,
    SIT_ACQ_ASSIST_SUPPORT_BM_DOPPLER_UNC_EXT_SUPPORT = 0x02,
}sitGnssAcquisitionAssistanceSupportType;

typedef enum
{
    SIT_ALMANAC_SUPPORT_NOT_USED = 0x00,
    SIT_ALMANAC_SUPPORT_BM_ALM_MODEL = 0x01,
}sitGnssAlmanacSupportType;

typedef enum
{
    SIT_GNSS_ALMANAC_MODEL1 = 0x01,
    SIT_GNSS_ALMANAC_MODEL2 = 0x02,
    SIT_GNSS_ALMANAC_MODEL3 = 0x04,
    SIT_GNSS_ALMANAC_MODEL4 = 0x08,
    SIT_GNSS_ALMANAC_MODEL5 = 0x10,
    SIT_GNSS_ALMANAC_MODEL6 = 0x20
}sitGnssAlmanacSuppotAlModel;

typedef enum
{
     SIT_GNSS_UTC_MODEL1 = 0x01,
     SIT_GNSS_UTC_MODEL2 = 0x02,
     SIT_GNSS_UTC_MODEL3 = 0x04,
     SIT_GNSS_UTC_MODEL4 = 0x08,
}sitGnssUtcSupportModelType;

typedef enum
{
    SIT_COMMON_ASSIST_DATA_SUPP_NOT_USED = 0x00,
    SIT_COMMON_ASSIST_DATA_SUPP_BM_REF_TIME = 0x01,
    SIT_COMMON_ASSIST_DATA_SUPP_BM_REF_LOCATION = 0x02,
    SIT_COMMON_ASSIST_DATA_SUPP_BM_IONO_MODEL = 0x04,
    SIT_COMMON_ASSIST_DATA_SUPP_BM_EARTH_ORIENTATION_PARAMETERS = 0x08
}sitGnssCommAssistDataSuppElementType;

typedef enum
{
    SIT_GNSS_ID_BITMAP_TYPE_NOT_ADDRESSED = 0x00,
    SIT_GNSS_ID_BITMAP_TYPE_GPS = 0x01,
    SIT_GNSS_ID_BITMAP_TYPE_SBAS = 0x02,
    SIT_GNSS_ID_BITMAP_TYPE_QZSS = 0x04,
    SIT_GNSS_ID_BITMAP_TYPE_GALILEO = 0x08,
    SIT_GNSS_ID_BITMAP_TYPE_GLONASS = 0x10
}sitGnssIdBitmapType;

typedef enum
{
    SIT_GNSS_LOC_ERROR_TYPE_UNDEFINED,
    SIT_GNSS_LOC_ERROR_TYPE_REQ_METHOD_NOT_SUPPORTED,
    SIT_GNSS_LOC_ERROR_TYPE_POS_METHOD_FAILURE,
    SIT_GNSS_LOC_ERROR_TYPE_PERODIC_LOC_MEAS_NOT_AVAILABLE,
    SIT_GNSS_LOC_ERROR_TYPE_NO_LOC_INFO,
}sitLocationErrorType;

typedef enum
{
    ALM_SET = 0x01,
    NAV_ALM = 0x02,
    REDUCED_ALM = 0x04,
    MIDI_ALM = 0x08,
    GLONASS_ALM = 0x10,
    SBAS_ALM = 0x20
}sitGnssAlmanacElementType;

typedef enum
{
    SIT_GNSS_SAT_MEAS_ELEM_TYPE_NOT_USED=0x00,
    SIT_GNSS_SAT_MEAS_ELEM_TYPE_BM_CARRIER_QUALITY_IND=0x01,
    SIT_GNSS_SAT_MEAS_ELEM_TYPE_BM_INTEGER_CODE_PHASE=0x02,
    SIT_GNSS_SAT_MEAS_ELEM_TYPE_BM_DOPPLER=0x04,
    SIT_GNSS_SAT_MEAS_ELEM_TYPE_BM_ADR=0x08,
}sitGnssSatMeasElementTypeMask;

typedef enum
{
    UDRE_SCALE_FACTOR_1,//Scale Factor = 1.0
    UDRE_SCALE_FACTOR_2,// UDRE Scale Factor = 0.75
    UDRE_SCALE_FACTOR_3,// UDRE Scale Factor = 0.5
    UDRE_SCALE_FACTOR_4,// UDRE Scale Factor = 0.3
    UDRE_SCALE_FACTOR_5,// UDRE Scale Factor = 0.2
    UDRE_SCALE_FACTOR_6,// UDRE Scale Factor = 0.1
    UDRE_SCALE_FACTOR_7,// Reference Station Transmission Not Monitored
    UDRE_SCALE_FACTOR_8,// Data is invalid - disregard
}sitGnssstatusHealth;

typedef enum
{
    SIT_GNSS_PROV_LOC_INFO_TYPE_NOT_USED =0x00,
    SIT_GNSS_PROV_LOC_INFO_TYPE_BM_SIGNAL_MEASURE_INFO=0x01,
    SIT_GNSS_PROV_LOC_INFO_TYPE_BM_LOCATION_INFO=0x02,
    SIT_GNSS_PROV_LOC_INFO_TYPE_BM_ERROR=0x04,
}sitGnssProvLocInfoBitmask;

typedef enum
{
    SIT_GNSS_LOC_COORD_TYPE_NOT_USED    =0x00,
    SIT_GNSS_LOC_COORD_TYPE_ELLIPSOID_POINT    =0x01,
    SIT_GNSS_LOC_COORD_TYPE_ELLIPSOID_POINT_WITH_UNCERTAINTY_CIRCLE    =0x02,
    SIT_GNSS_LOC_COORD_TYPE_ELLIPSOID_POINT_WITH_UNCERTAINTY_ELLIPSE    =0x04,
    SIT_GNSS_LOC_COORD_TYPE_POLYGON    =0x08,
    SIT_GNSS_LOC_COORD_TYPE_ELLIPSOID_POINT_WITH_ALTITUDE    =0x10,
    SIT_GNSS_LOC_COORD_TYPE_ELLIPSOID_POINT_WITH_ALTITUDE_AND_UNCERTAINTY_ELLIPSOID    =0x20,
    SIT_GNSS_LOC_COORD_TYPE_ELLIPSOID_ARC    =0x40,
}sitGnsslocCoordinateType;

typedef enum
{
    SIT_GNSS_MEAS_REF_TIME_TYPE_NOT_USED=0x00,
    SIT_GNSS_MEAS_REF_TIME_TYPE_BM_GNSS_TOD_FRAC=0x01,
    SIT_GNSS_MEAS_REF_TIME_TYPE_BM_GNSS_TOD_UNC=0x02,
    SIT_GNSS_MEAS_REF_TIME_TYPE_BM_NETWORK_TIME=0x04,
}sitGnssMeasRefTimeMask;

typedef enum
{
    SIT_UTC_MODEL_TYPE_M1 = 0x01,
    SIT_UTC_MODEL_TYPE_M2 = 0x02,
    SIT_UTC_MODEL_TYPE_M3 = 0x04,
    SIT_UTC_MODEL_TYPE_M4 = 0x08
}sitGnssUtcModelType;

typedef enum
{
    SIT_LOC_SERVER_ERR_CAUSE_UNDEFINED = 0x01,
    SIT_UNDELIVERED_ASSIST_DATA_NOT_SUPPORTED_BY_SERVER,
    SIT_UNDELIVERED_ASSIST_DATA_SUPPORTED_CURRENTLY_NOT_AVAILABLE,
    SIT_UNDELIVERED_ASSIST_DATA_PARTLY_NOT_SUPPORTED_NOT_AVAILABLE_BY_SERVER
}sitGNSSLocationServerErrorCausesType;

typedef enum
{
    SIT_ASSIST_DATA_NOT_USED = 0x00,
    SIT_CMN_ASSIST_DATA = 0x01,
    SIT_GPS_GENERIC_ASSIST_DATA = 0x02,
    SIT_GLONASS_GENERIC_ASSIST_DATA = 0x04,
    SIT_MISC_GENERIC_ASSIST_DATA = 0x08,
}sitGnssAssistDataType;

/*-----------------------------------------------------------------------*/
/*  Structure used in Main                  */
/*-----------------------------------------------------------------------*/
typedef PACKED_STRUCT SitMoLocQos_t
{
    u32                        flag;
    u8                        HoriAccu;
    u8                        VertiCordi;
    u8                        VertiAccu;
    u8                        RspTime; //sit_gps_response_time
}SitMoLocQos;


typedef PACKED_STRUCT sitSatInfo_t
{
    u8                        SatId; //Satellite PRN
    u8                        SatIode; //Ephemeris IODE
}sitSatInfo;


typedef PACKED_STRUCT sitExtEphChk_t
{
    u8                        WeekBegin;
    u8                        WeekEnd;
    u8                        TowBegin;
    u8                        TowEnd;
}sitExtEphChk;


typedef PACKED_STRUCT sitGpsMoAssData_t
{
    u32                        flag;
    u16                        GpsWeek;
    u8                        GpsToe; //GPS Time id week for eph/alm data
    u8                        NumOfSat; //Number of satellite
    u8                        ToeLimit; //GPS Toe Limit
    sitSatInfo                SatInfo[SIT_SAT_INFO_SIZE];
    u8                         ExtenEmp;
    sitExtEphChk            ExtEmpChk;
}sitGpsMoAssData;


typedef PACKED_STRUCT sitEllipsoidPoint_t
{
    s32                Latitude;
    s32                Longitude;
}sitEllipsoidPoint;


typedef PACKED_STRUCT sitUncCircle_t
{
    sitEllipsoidPoint    Points;
    u8                UncCircle;
}sitUncCircle;


typedef PACKED_STRUCT sitUncEllipse_t
{
    sitEllipsoidPoint    Points;
    u8                SemiMajorAxis;
    u8                SemiMinorAxis;
    u8                OrientationCircle;
    u8                Confidence;
}sitUncEllipse;


typedef PACKED_STRUCT sitAltUncEllipse_t
{
    sitEllipsoidPoint    Points;
    s16                Altitude;
    u8                SemiMajorAxis;
    u8                SemiMinorAxis;
    u8                OrientationCircle;
    u8                UncAltitude;
    u8                Confidence;

}sitAltUncEllipse;


typedef PACKED_STRUCT sitEllipsoidArc_t
{
    sitEllipsoidPoint    Points;
    u16                InnerRadius;//inner radius of Ellipsoid Arc
    u8                UncRadius;
    u8                OffsetAngle;
    u8                IncludeAngle;
    u8                Confidence;
}sitEllipsoidArc;


typedef PACKED_STRUCT sitEllipsoidAlt_t
{
    sitEllipsoidPoint    Points;
    s16                Altitude;//Altitude in meter
}sitEllipsoidAlt;


typedef PACKED_STRUCT sitPolygon_t
{
    u8                NumPoint;//Number of point in Polygon
    sitEllipsoidPoint    Points[15];
}sitPolygon;


typedef PACKED_STRUCT sitLocInfo_t
{
    u8                ShapeType; //sitGpsGadShapeType
    sitUncCircle        PointUncCircle; //
    sitUncEllipse        PointUncEllipse; //
    sitAltUncEllipse    PointAltUncEllipsee; //
    sitEllipsoidArc    EllipsoidArc; //
    sitEllipsoidPoint    EllipsoidPoint; //
    sitEllipsoidAlt        EllipsoidAlt; //
    sitPolygon        Polygon; //
}sitLocInfo;


typedef PACKED_STRUCT sitDeciperKey_t
{
    u8                flag;
    u8                CurrDeciperKet[SIT_DECIPHER_LENGTH]; //
    u8                NexteciperKet[SIT_DECIPHER_LENGTH];
}sitDeciperKey;


typedef PACKED_STRUCT sitGsmTime_t
{
    u8    ValidFlag;
    u16    BcchCarrier;
    u16    BSIC;
    u32    FrameNum;
    u16    TimeSlot;
    u16    BitNum;
}sitGsmTime;


typedef PACKED_STRUCT sitGpsRefTime_t
{
    u8    ValidFlag;
    u32  CellFrame;
    u8    ChoiceMode; //sitGpsUtrnChoiceModeType
    u32    FddScremCode;//FDD primary scrembling code;
    u32    TddCellPAramID;//TDD cell Parameter ID
    u32    Sfn;
}sitGpsRefTime;


typedef PACKED_STRUCT sitGpsUnc_t
{
    u8    ValidFlag;
    u32  GpsTimeUnc;//GPS time uncertainty;
}sitGpsUnc;


typedef PACKED_STRUCT sitDriftRateType_t
{
    u8    ValidFlag;
    s32  DriftRate;
}sitDriftRateType;


typedef PACKED_STRUCT sitUtranTime_t
{
    sitGpsRefTime    GpsRefTime;
    sitGpsUnc        GpsUnc;//GPS Uncertainty
    u8                SfnUnc;//sitGpsSfnUncType
    sitDriftRateType    DriftRateType;
}sitUtranTime;


typedef PACKED_UNION sitNetTimeInfo_t
{
    sitGsmTime  GsmTime;
    sitUtranTime    UtranTime;
}sitNetTimeInfo;


typedef PACKED_STRUCT sitGpsTowAssistType_t
{
    u16        satID;
    u16        tlmWord;
    u8        antiSpoofFlag;
    u8        alertFlag;
    u8        tmlReservedBits;
}sitGpsTowAssistType;


typedef PACKED_STRUCT sitRefTime_t
{
    u32                GpsTow;//GPS timeof week
    u32                GpsWeek;//GPS Time of week
    u8                NumSat;//Number of settelite
    sitNetTimeInfo    NetTimeInfo;
    sitGpsTowAssistType GpsTowAssist[SIT_GPS_TOW_ASSIST_LENGTH];
}sitRefTime;


typedef PACKED_STRUCT sitRefLoc_t
{
    u8                ShapeType;//Shape Type
    u8                HemiSpare;//HemiSpare
    u16                Altitude;//Number of setteliteu8
    u32                Latitude;//Number of settelite
    s32                Longitude;//Number of settelite
    u8                DirOfAlti;//Direction of Altitude
    u8                SemiMajorUncer;//Semi MAjor uncertainty
    u8                SemiMinorUncer;//Semi Minor uncertainty
    u8                MajorAxis;//Number of settelite
    u8                AltitudeUncer;//AltitudeUncertainty
    u8                Confidence;
}sitRefLoc;


typedef PACKED_STRUCT sitSatSeq_t
{
    u8    SatId;//Settelite ID
    u16    IODE;//sitGpsDgpsStatusType
    u8    UDRE;
    s16    pseudoRangeCorr;//pseudo Range Correction
    s16    RangeRateCorr;// Range Rate Correction
}sitSatSeq;


typedef PACKED_STRUCT sitDgpsCorrection_t
{
    u32    GpsTow;
    u8    Status;//sitGpsDgpsStatusType
    u32    NumSat;
    sitSatSeq    SatSeq[SIT_NAVIGATION_SATELLITE_INFO_LENGTH];//satellite sequence
}sitDgpsCorrection;


typedef PACKED_STRUCT sitNaviSubFrameRsvType_t
{
    u32    RSV1;
    u32    RSV2;
    u32    RSV3;
    u32    RSV4;
}sitNaviSubFrameRsvType;


typedef PACKED_STRUCT sitNaviEpheType_t
{
    u8    ephemCodeOnL2;
    u8    ephemUra;
    u8    ephemSvHealth;
    u16    ephemIodc;
    u8    ephemL2PFlag;
    sitNaviSubFrameRsvType NaviSubFrameRsv;
    s8    ephemTgd;
    u16    ephemToc;
    s8 ephemAf2;
    s16 ephemAf1;
    s32 ephemAf0;
    s16 ephemCrs;
    s16 ephemDeltaN;
    s32 ephemM0;
    s16 ephemCuc;
    u32 ephemE;
    s16 ephemCus;
    u32 ephemAPowrHalf;
    u16 ephemToe;
    s8 ephemFitFlag;
    u8 ephemAoda;
    s16 ephemCic;
    s32 ephemOmegaA0;
    s16 ephemCis;
    s32 ephemI0;
    s16 ephemCrc;
    s32 ephemW;
    s32 ephemOmegaADot;
    s16 ephemIDot;
}sitNaviEpheType;


typedef PACKED_STRUCT sitSatInfoType_t
{
    u8    SatId;//Settelite ID
    u8    NaviSatStatus;//sitGpsNavigationSatStatusType
    sitNaviEpheType    NaviEhpe;
}sitSatInfoType;


typedef PACKED_STRUCT sitNaviModel_t
{
    u32                   NumSat;    //Number of satelite
    sitSatInfoType    NaviSatInfo[SIT_NAVIGATION_SATELLITE_INFO_LENGTH];
}sitNaviModel;


typedef PACKED_STRUCT sitIonoModel_t
{
    s8            ALFA0;
    s8            ALFA1;
    s8            ALFA2;
    s8            ALFA3;
    s8            BETA0;
    s8            BETA1;
    s8            BETA2;
    s8            BETA3;
}sitIonoModel;


typedef PACKED_STRUCT sitUtcModel_t
{
    s32    UTC_A1;
    s32    UTC_A0;
    u8    UTC_TOT;
    u8    UTC_WNT;
    s8    UTC_DELTA_TLS;
    u8    UTC_WNLSF;
    s8    UTC_DN;
    s8    UTC_DELTA_TLSF;
}sitUtcModel;


typedef PACKED_STRUCT sitAimSatInfo_t
{
    s8    dataId;
    u8    satId;
    u16    almanacE;
    u8    almanacToa;
    s16    almanacKsii;
    s16    almanacOemgaDot;
    u8    almanacSvHealth;
    u32    almanacAPowerHalf;
    s32    almanacOmega0;
    s32    almanacW;
    s32    almanacM0;
    s16    almanacAf0;
    s16    almanacAf1;
}sitAimSatInfo;


typedef PACKED_STRUCT sitAlmanac_t
{
    u8                    AlmWna;
    u32                    NumSat;
    sitAimSatInfo AlmSatInfo[SIT_ALMANAC_SATELLITE_INFO_LENGTH];
}sitAlmanac;


typedef PACKED_STRUCT sitAcqUtranTimeType_t
{
    sitGpsRefTime    GsmTime;
    sitGpsUnc           GpsUnc;
}sitAcqUtranTimeType;


typedef PACKED_UNION sitAcqTimInfo_t
{
    sitGsmTime                   GpsRefTime;
    sitAcqUtranTimeType   GpsAcqUtranTime;
}sitAcqTimInfo;


typedef PACKED_STRUCT sitAcqSatInfo_t
{
    u8    SatelliteID;
    s16    Dopple0value;
    u8    Doppler1;
    u8    DopplerUncertainty;
    u16    CodePhase;
    u8    IntCodePhase;
    u8    GPSbitNumber;
    u8    CodePhaseSearchWindow;
    u8    Azimuth;
    u8    Elevation;
}sitAcqSatInfo;


typedef PACKED_STRUCT sitAcqAssist_t
{
    u32    GpsTow;
    sitAcqTimInfo    AcqTimeInfo;
    u32                 NumSat;//Number of satelite
    sitAcqSatInfo    AcqSatInfo[SIT_SATELLITE_ID_LENGTH];
}sitAcqAssist;


typedef PACKED_STRUCT sitRealTimeInt_t
{
    u8    SatId[SIT_SATELLITE_ID_LENGTH];
    u8     NumSat;
}sitRealTimeInt;


typedef PACKED_STRUCT sitAccType_t
{
    u32                        Flag;
    u8                        HoriAccu; //Hotizental Accuracy
    u8                        VertiAccu;//Vertical Accuracy
} sitAccType;


typedef PACKED_STRUCT sitMeasLocType_t
{
    u32            GpsTow; //0x00 Sucess 0x01 Fail
    u16            GpsWeek;
    u8            FixType;//0x01 2d Fix, 0x02 3D fix
    sitLocInfo    MLocInfo;
} sitMeasLocType;


typedef PACKED_STRUCT sitGpsElemType_t
{
    u8        SatelliteID;
    u8        Cn0;
    s16        Doppler;
    u16        WholeChips;
    u16        FracChips;
    u8         MultiPath; //sitMultiPathType
    u8        PseuRangeRmsError;
} sitGpsElemType;


typedef PACKED_STRUCT sitGpsMeaType_t
{
    u32                GpsTow; //0x00 Sucess 0x01 Fail
    u16                GpsWeek;
    u8                NumSat;//Number of seetelite
    sitGpsElemType    GpsEleType[SIT_GPS_ELEMENT_LENGTH];
} sitGpsMeaType;


/* Location Type */
typedef PACKED_STRUCT sitLocType_t
{
    u16         LocEvent;
    u8        LocEtsiType;//sitGpsLocEstimate
} sitLocType;


/* DCS String  Type */
typedef PACKED_STRUCT sitStringType_t
{
    u32     LocEvent;
    u8    ApduLength[SIT_GPS_APDU_LENGTH];
} sitStringType;


/* Client Name Type */
typedef PACKED_STRUCT sitClientName_t
{
    u8                          dcs;
    sitStringType    StringType;
    u8                ftmIndicator;//sitGpsFormatInd
} sitClientName;


/* Code Word Type */
typedef PACKED_STRUCT sitCodeWord_t
{
    u8                 dcs;
    sitStringType  str;
} sitCodeWord;


/*=================================================================================

   SUB_CMD(1) : SIT_GPS_EXT_RADIO_SIG      0x21 GPS Extended Radio Signal Message

==================================================================================*/

typedef PACKED_STRUCT sitGpsRadioSigInfoRsp_t
{
    u16                        Afrcn;
    u8                        Bsic;
    u8                        RxLevel;
    u8                        TimeAdvance;
}sitGpsRadioSigInfoRsp;


/*=================================================================================

   SUB_CMD(1) : SIT_GPS_CP_MO_LOCATION      0x22 GPS Control Plane MO Location Request Message

==================================================================================*/

/*Molr Set Request Type */
typedef PACKED_STRUCT  sitGpsCpMoLocSet_t
{
    u8                        MolrType; //sitGpsMolrType
    u8                        LocMethod; //sitGpsLocationMethodType
    SitMoLocQos                Qos;
    u8                        ClientId[SIT_CPMO_LOC_CLIENT_ID_LENGTH];
    u8                        MlcNum[SIT_CPMO_MLCNUM_LENGTH];
    sitGpsMoAssData            AssData; //
    u8                        GadShape;
    u8                        SvcTypeId;
    u8                        PseudInd;    //pseudonym indicator
}sitGpsCpMoLocSet;

/*Molr Notification Responce  DESCRIPTION : GPS Control Plane MO Location Message*/

typedef PACKED_STRUCT sitGpsCpMoLocNtf_t
{
    sitLocInfo        LocInfo;
    u8                NoLoc; // No LOcation
    sitDeciperKey    DeciKey;
}sitGpsCpMoLocNtf;



/*=================================================================================

   SUB_CMD(1) : RCM_GPS_ASSIST_DATA      0x23 GPS Assist Data Message  => Notification Type

==================================================================================*/


typedef PACKED_STRUCT sitGpsAssDataNtf_t
{
    u32                        flag;
    sitRefTime                RefTime;
    sitRefLoc                RefLoc;
    sitDgpsCorrection            DgpsCorrection;
    sitNaviModel                NaviModel;
    sitIonoModel                IonoModel;
    sitUtcModel                UtcModel;
    sitAlmanac                Almanac;
    sitAcqAssist                AcqAssist;
    sitRealTimeInt            RealTimeInt;//Real Time Integrity;

}sitGpsAssDataNtf;


/*=================================================================================

   SUB_CMD(1) :     0x25 GPS Measure Position Message

==================================================================================*/

/*Measure Position Indication Message*/

typedef PACKED_STRUCT sitGpsMeasPosInd_t
{
    u8                        MethodType; //sitGpsMethodType
    sitAccType                Accuracy;//sitGpsMtlrNotify
    u8                        RspTime;
    u8                        UseMultiSet;//sitGpsUseMultiSetsType
    u8                        EnvChar;//sitGpsEnvCharType
    u8                        CallTimingWant;//sitGpsCellTimingWntType
    u8                         AddAssiReq;//sitGpsAddAssitReqType
} sitGpsMeasPosInd;

/*Measure Position Confirm Message    DESCRIPTION : GPS Data Connection Confirm*/

typedef PACKED_STRUCT sitGpsMeasPosCnf_t
{
    u8                        Result; //0x00 Sucess 0x01 Fail
    u32                                 ResType;
    sitGpsMeaType            GpsMeasure;
    sitMeasLocType            LocInfo;//
    sitGpsMoAssData            MAssisData;//
    sitGpsRefTime            GpsRefTime;//
} sitGpsMeasPosCnf;





/*  EXTENDED CELL INFO */
typedef PACKED_STRUCT sitNMRelement_t
{
    u16                    arfcn;
    u8                    bsic;
    u8                    rxlev;
}sitNMRelement;

typedef PACKED_STRUCT sitGsmNwMeasResults_t
{
   sitNMRelement            nMR[15];//INTEGER(1..128)
   u8                        sizeOfnMR;//INTEGER(1..15)
}sitGsmNwMeasResults;

typedef PACKED_STRUCT sitTimingAdvance_t
{
    u16                    ta;//INTEGER(0..8191)
    u8                    taResolution;
/*
0x01 : res10chip
0x02 : res05chip
0x03 : res0125chip
If missing, resolution is 0.125 chips
*/
    u8                    chipRate;
/*
0x01 : tdd128
0x02 : tdd384
0x03 : tdd768
If missing, chip rate is 1.28 Mchip/s
*/
}sitWcdmaTimingAdvance;

typedef PACKED_STRUCT sitWcdmaCellMeasResultsFdd_t
{
    u16                    primaryCpichInfo;
    u8                    cpichECN0;
    u8                    cpichRSCP;
    u8                    pathLoss;
}sitWcdmaCellMeasResultsFdd;

typedef PACKED_STRUCT sitWcdmaCellMeasResultsTdd_t
{
    u8                                   cellParametersId;
    u8                                   proposedtgsn;
    u8                                   primaryCcpchRSCP;
    u8                                   pathLoss;
    u8                                   timeSlotiscplist[14];
    u8                                   sizeofTimeSlotiscplist;
}sitWcdmaCellMeasResultsTdd;

typedef PACKED_STRUCT sitWcdmaFrequencyinfo_t
{
// SIT_WCDMA_FREQ_INFO_FDD     0x01
// SIT_WCDMA_FREQ_INFO_TDD     0x02
   u8                                    modeSpecificInfo;
   u16                                  uarfcnUl;//FDD
   u16                                  uarfcnDl;//FDD
   u16                                  uarfcnNt;//TDD
}sitWcdmaFrequencyinfo;

typedef PACKED_STRUCT sitWcdmaCellMeasuredResults_t
{
// SIT_WCDMA_CELLMEAS_RESULT_FDD   0x01
// SIT_WCDMA_CELLMEAS_RESULT_TDD   0x02
    u8                                   modeSpecificInfo;
    u32                                 cellIdentity;
    PACKED_UNION{
        sitWcdmaCellMeasResultsFdd   fdd;
        sitWcdmaCellMeasResultsTdd   tdd;
    }sitSpecificCellMeasInfo;
}sitWcdmaCellMeasuredResults;

typedef PACKED_STRUCT sitWcdmaMeasResult_t
{
    sitWcdmaFrequencyinfo        frequencyInfo;
    u8                            utraCarrierRSSI;
    sitWcdmaCellMeasuredResults    cellMeasuredResultsList[16];
    u8                            sizeofCellmeasResultsList;// 1....16
}sitWcdmaMeasResults;

typedef PACKED_STRUCT sitGsmCellInformation_t
{
// SIT_ECID_BM_NW_MEAS_RESULT        0x01
// SIT_ECID_BM_TA                    0x02
   u8                        bitMask;
   u16                    refMCC;//INTEGER(0..999)
   u16                    refMNC;//INTEGER(0..999)
   u16                    refLAC;//INTEGER(0..65535)
   u16                    refCI;//INTEGER(0..65535)
   sitGsmNwMeasResults    nwMeasResult;
   u8                        ta;
}sitGsmCellInformation;

typedef PACKED_STRUCT sitWcdmaCellInformation_t
{
// SIT_BM_FREQUENCY_INFO                0x01
// SIT_BM_PRIMARY_SCRAMBLING_CODE    0x02
// SIT_BM_MEASURED_RESULTS_LIST        0x04
// SIT_BM_CELL_PARAMETERS_ID            0x08
// SIT_BM_TIMING_ADVANCE                0x10
   u8                                bitMask;//INTEGER(1..128)
   u16                            refMcc;
   u16                            refMnc;
   u32                            refUc;
   sitWcdmaFrequencyinfo            frequencyInfo;
   u16                            primaryScramblingCode;//INTEGER(0..511)
   sitWcdmaMeasResults            measuredResultsList[8];
   u8                                sizeOfMeasResultList;
   u8                                cellParametersId;//INTEGER(0..127), --Not applicable for FDD
   sitWcdmaTimingAdvance            timingAdvance;//Not applicable for FDD
}sitWcdmaCellInformation;


/*=================================================================================

   SUB_CMD(1) :   0x26 GPS MTLR Notification Message

==================================================================================*/
/*Notification*/

typedef PACKED_STRUCT sitGpsMtlrInd_t
{
    u8                        ReqId;
    u8                        NotiType;//sitGpsMtlrNotify
    sitLocType                LocType;
    u8                        ClientId[SIT_CPMO_LOC_CLIENT_ID_LENGTH];
    sitClientName            ClientName;
    sitClientName            ReqestorId;
    sitCodeWord                CodeWord;
    u8                         ServiceType;
}sitGpsMtlrInd;

/*Confirmation*/
typedef PACKED_STRUCT sitGpsMtlrCnf_t
{
    u8                        ReqId;
    u8                        ResType;//sitGpsVerifyRsp
}sitGpsMtlrCnf;

typedef PACKED_STRUCT
{
  u8                                     enable;
} sitGpsFrequAidingSetReq;

typedef PACKED_STRUCT
{
  u8                        lock_status;
  u8                                     afc_update;
} sitGpsFreqAidingNoti;

typedef PACKED_STRUCT sitLppHeader_t
{
    u32            Sid;
}sitLppHeader;

/* SUPL UDP PORT LISTENING*/
#define  SIT_GPS_MAX_SUPL_MSG_LENGTH     256

/*SUPL NI READY SET */
typedef PACKED_STRUCT  sitGpsSuplNiReadySet_t
{
    u16                        Port; //The desired port number to be set. e.g. 7275
}sitGpsSuplNiReadySet;

/*SUPL NI READY NTF */
typedef PACKED_STRUCT  sitGpsSuplNiReadyNtf_t
{
    u8                        Result; //result - 0x00 FALSE, 0x01 TRUE
}sitGpsSuplNiReadyNtf;

/*Confirmation*/
typedef PACKED_STRUCT sitGpsSuplNiMsgCnf_t
{
    u8        Tid; //The desired Transaction ID
    u8        Result; //0x00 Re-Send (Refer to TID), 0x01 ABORT "Please do not send any more", 0x02 DONE
}sitGpsSuplNiMsgCnf;

typedef PACKED_STRUCT sitGpsSuplNiMsgInd_t
{
    u8                        Tid; //The desired Transaction ID [0x1 ~ 0xFF]
    u16                        sizeOfMsg; //The size of SUPL NI message
    u8                         suplMsg[SIT_GPS_MAX_SUPL_MSG_LENGTH]; //SUPL Network-Initiated Message
} sitGpsSuplNiMsgInd;

#define  SIT_GPS_MAX_SUPL_PRS_RAW_DATA   610

/*SUPL LPP PRS INFO SET */
typedef PACKED_STRUCT  sitGpsSuplLppPrsInfoSet_t
{
    sitLppHeader        lpp_hdr;
    u8                 Flag; /* For Set, 0x01 : Request capabilities, 0x08 : Provide assistance data, 0x10 : Request location information */
    /* For NTF, 0x02 : Provide capabilities, 0x04 : Request assistance data, 0x20 : Provide location info */
    u8                PrsInfo[SIT_GPS_MAX_SUPL_PRS_RAW_DATA]; //PRS RAW DATA
    u16                SizeOfPrs;  //INTEGER(1..512)
    u8                ResponseTime; //INTEGER(1 ~128)
}sitGpsSuplLppPrsInfoSet;

typedef PACKED_STRUCT  sitGpsSuplLppPrsInfoNtf_t
{
    sitLppHeader        lpp_hdr;
    u8                 Flag;
    u8                PrsInfo[SIT_GPS_MAX_SUPL_PRS_RAW_DATA]; //PRS RAW DATA
    u16                SizeOfPrs;  //INTEGER(1..512)

}sitGpsSuplLppPrsInfoNtf;

typedef PACKED_STRUCT sitGpsCapabilityAck_t
{
    u8                        ackReq;
    u8                        ackInd;
}sitGpsCapabilityAck;

typedef PACKED_STRUCT SitTranscationIdType_t
{
    u8                        tidInit;
    u8                        tidNum;
}SitTranscationIdType;

typedef PACKED_STRUCT sitLppCommonDataType_t
{
    SitTranscationIdType        tid;
    u8                        endTid;
    u32                    sessionID;
    u8                        seqNum;
    sitGpsCapabilityAck        ack;
}sitLppCommonDataType;

typedef PACKED_STRUCT sitGnssSignalIdType_t
{
    u8 signalID;
}sitGnssSignalIdType;

typedef PACKED_STRUCT sitGnssAccessTypes_t
{
    u8                    accessTypes; //0 -eutra , 1-utra, 2 - gsm
}sitGnssAccessTypes;

typedef PACKED_STRUCT sitGnssSbasIds_t
{
     u8                            bitMask;    //0x00 : NOT_USED, 0x01 :BM_SBAS_IDS
     u8                            sbasIDs;//sitGnssSbasIdType
}sitGnssSbasIds;

typedef PACKED_STRUCT sitGnssSbasId_t
{
     u8                            sbasIDs;//sitGnssSbasIdType
}sitGnssSbasId;

typedef PACKED_STRUCT sitGnssIdBitmap_t
{
    u16            gnssIds;//sitGnssIdBitmapType
}sitGnssIdBitmap;

typedef PACKED_STRUCT sitGnssDifferentialCorrectionsSupport_t
{
    sitGnssSignalIdType    gnssSignalID;
    u8                    validityTimeSupport; // 00 - False, 0x01 - True
}sitGnssDifferentialCorrectionsSupport;

typedef PACKED_STRUCT sitGnssNavigationModelSupport_t
{
     u8                 bitMask;//sitGnssNavigationModelSupportType
     u8                clockModel;//sitGnssNavigationModelClockModelType
     u8                     orbitModel;//sitGnssNavigationModelOrbitModelType
}sitGnssNavigationModelSupport;

typedef PACKED_STRUCT sitGnssAcquisitionAssistanceSupport_t
{
    u8                                    bitMask;//sitGnssAcquisitionAssistanceSupportType
    u8                                    confidenceSupport;
    u8                                    dopplerUncExtSupport;
}sitGnssAcquisitionAssistanceSupport;

typedef PACKED_STRUCT sitGnssAlmanacSupport_t
{
    u8            bitMask;//sitGnssAlmanacSupportType
    u8            almModel;//sitGnssAlmanacSuppotAlModel
}sitGnssAlmanacSupport;

typedef PACKED_STRUCT sitGnssUtcModelSupport_t
{
    u8                            bitMask;//0x00 : NOT_USED, 0x01 : BM_UTC_MODEL
    u8                            utcModel;//sitGnssUtcSupportModelType
}sitGnssUtcModelSupport;

typedef PACKED_STRUCT sitGnssReferenceTimeSupport_t
{
    u8                            bitMask; //0x00 : NOT _USED 0x01 : BM_FTA
    sitGnssIdBitmap                gnssSystemTime;
    sitGnssAccessTypes            Fta;
}sitGnssReferenceTimeSupport;

typedef PACKED_STRUCT sitGnssIonosphericModelSupport_t
{
    u8                        ionoModel; //0x00 : kloburchar, 0x01 : neQuick
}sitGnssIonosphericModelSupport;

typedef PACKED_STRUCT sitGnssId_t
{
    u8 gnssID;//sitGnssIdType
}sitGnssId;

typedef PACKED_STRUCT sitGnssGenericAssistDataSupportElement_t
{
    u16                                    bitMask;//sitGnssGenAssistDataSuppElementType
    sitGnssId                            gnssID;
    sitGnssSbasId                        sbasID;
    //sitGnssTimeModelListSupport            timeModelsSupport; //Future Use
    sitGnssDifferentialCorrectionsSupport    diffCorrectSupport;
    sitGnssNavigationModelSupport             navModelSupport;
    //sitGnssRealTimeIntegritySupport        rti_Support;
    //sitGnssDataBitAssistanceSupport        DataBitAssistSupport;
    sitGnssAcquisitionAssistanceSupport        acqAssistSupport;
    sitGnssAlmanacSupport                alm_Support;
    sitGnssUtcModelSupport                 utcModelSupport;
    //sitGnssAuxiliaryInformationSupport        auxiliaryInfoSupport;
}sitGnssGenericAssistDataSupportElement;

typedef PACKED_STRUCT sitGnssCommonAssistanceDataSupport_t
{
    u8                                        bitMask;//sitGnssCommAssistDataSuppElementType
    sitGnssReferenceTimeSupport                refTime;
    //sitGnssReferenceLocationSupport            refLocation;//future use
    sitGnssIonosphericModelSupport                ionoModel;
    //sitGnssEarthOrientationParametersSupport    earthOrientationParameters;//future use
}sitGnssCommonAssistanceDataSupport;

typedef PACKED_STRUCT sitAssistanceDataSupportList_t
{
     sitGnssCommonAssistanceDataSupport        cmnAssistDataSupport;
     sitGnssGenericAssistDataSupportElement    genAssistDataSupport[SIT_LPP_MAX_SV_CNT];
     u8                                        sizeOfGenAssistDataSupport; // integer 1..16
}sitAssistanceDataSupportList;

typedef PACKED_STRUCT sitLocationCoordinateTypes_t
{
     u8                        ellipsoidPoint;
     u8                        ellipsoidPointWithUncertaintyCircle;
     u8                        ellipsoidPointWithUncertaintyEllipse;
     u8                        polygon;
     u8                        ellipsoidPointWithAltitude;
     u8                        ellipsoidPointWithAltitudeAndUncertaintyEllipsoid;
     u8                        ellipsoidArc;
}sitLocationCoordinateTypes;

typedef PACKED_STRUCT sitGnssFtaMeasureSupportType_t
{
    u8                cellTime;//sitFtaMeasSuppTypeCellTime
    u8                positionModes;//sitGnssPositionMode
}sitGnssFtaMeasureSupportType;

typedef PACKED_STRUCT sitAgnssSupportElement_t
{
    u8                            bitMask;//sitGnssElementTypeBitMask
    sitGnssId                    gnssID;
    sitGnssSbasId                sbasIDs;
    u8                            positionMode;//sitGnssPositionMode
    sitGnssSignalIdType            signalIDs;
    sitGnssFtaMeasureSupportType ftaMeasureSupport;
    u8                            adrSupport;
    u8                            velocityMeasure;//0x00 : False 0x01 : True
}sitAgnssSupportElement;

typedef PACKED_STRUCT sitAGnssCapability_t
{
    u8                            bitMask;//sitAgnssCapabilityBitMask
    sitAgnssSupportElement        supportList[SIT_LPP_MAX_SV_CNT];
    u8                            sizeOfSupportList; //Integer (1..16)
    sitAssistanceDataSupportList    assistDataSupportList;
    sitLocationCoordinateTypes     locationCoordinateTypes;
    u8                            velocityType;//sitAgnssVelocityType
}sitAGnssCapability;

typedef PACKED_STRUCT sitGnssTargetDeviceErrorCausesType_t
{
    u8            bitMask;//sitTargetDeviceErrorCausesBitMask
    u8            cause;//sitTargetDeviceErrorCause
/*ftaMeasurementsNotPossible
adrMeasurementsNotPossible
multiFrequencyMeasurementsNotPossible
*/
}sitGnssTargetDeviceErrorCausesType;

typedef PACKED_STRUCT sitGNSSLocationServerErrorCauses_t
{
    sitGNSSLocationServerErrorCausesType             cause;
}sitGNSSLocationServerErrorCauses;

typedef PACKED_STRUCT sitAGnssError_t
{
    u8        errorType;//sitAGnssErrorType
    sitGNSSLocationServerErrorCauses         locationServerErrorCauses;
    /*
    0x01 : undefined
    0x02: undeliveredAssistanceDataIsNotSupportedByServer
    0x03 : undeliveredAssistanceDataIsSupportedButCurrentlyNotAvailableByServer
    0x04 : undeliveredAssistanceDataIsPartlyNotSupportedAndPartlyNotAvailableByServer
    */
    sitGnssTargetDeviceErrorCausesType    targetDeviceErrorCauses;
}sitAGnssError;

typedef PACKED_STRUCT sitGnssRefTimeREQ_t
{
    u8                        bitMask;    //0x00 : NOT_USED. 0x01 : BM_TOW 0x02 : BM_NOTI_OF_LEAP_SEC
    sitGnssId                timeReqPrefList[8];
    u8                        sizeOfTimeReqPrefList;
    u8                        tow;
    u8                        notiOfLeapSec;
}sitGnssRefTimeREQ;

typedef PACKED_STRUCT sitGnssIonoModelReq_t
{
    u8                        bitMask;//0x0000 : NOT_USED. 0x0001:KLOBUCHAR_MODEL 0x0002:NEQUICKMODEL
    u8                        klobucharModel; //BIT STRING (SIZE(2))
    u8                        nequickModel;//NULL
}sitGnssIonoModelReq;

typedef PACKED_STRUCT sitGnssCommonAssistDataREQ_t
{
    u8                                    bitMask;    //sitGnssCommAssistDataSuppElementType
    sitGnssRefTimeREQ                     refTime;
    //sitGnssRefLocREQ                     refLocation; //future use
    sitGnssIonoModelReq                    ionoModel;
    //sitGnssEarthOrientationParametersREQ     earthOrientation; //future use
}sitGnssCommonAssistDataREQ;

typedef PACKED_STRUCT sitGnssSatListRelatedDataElementtype_t
{
    u8                        bitMask;//0x00 : NOT_USED. 0x01 : BM_CLOCK_MODEL_ID 0x02 : BM_ORBIT_MODEL_ID
    u8                        svID;
    u16                        iod;
    u8                        clockModelID;//INTEGER (1..8)
    u8                        orbitModelID;//INTEGER (1..8)
}sitGnssSatListRelatedDataElementtype;

typedef PACKED_STRUCT sitGnssStoredNaviListInfoType_t
{
    u8                            bitMask;//0x00 : NOT_USED. 0x01 : BM_SAT_LIST_RELATED_DATA_LIST
    u16                            weekOrDay;//INTEGER (0..4095)
    u8                            toe;//INTEGER (0..255)
    u8                            toeLimit;//INTEGER (0..15)
    sitGnssSatListRelatedDataElementtype    satListRelatedDataList[32];
    u8                                    sizeOfSatListRelatedDataList ; //Integer (1..32)
}sitGnssStoredNaviListInfoType;

typedef PACKED_STRUCT sitGnssReqNavListInfoType_t
{
    u8                            bitMask;//enum needed
    /*0x00 : NOT_USED.
    0x01 : BM_CLOCK_MODEL_ID_PREF_LIST
    0x02 : BM_ORBIT_MODEL_ID_PREF_LIST
    0x04 : BM_ADD_NAV_PARAM_REQ*/
    u8                            svReqList[8];//as per lpp-NS interface strucut
    u8                            clockModelIDPrefList[8];//(SIZE (1..8)) OF INTEGER (1..8)
    u8                            sizeOfClockModelIDPrefList;//Integer (1..8)
    u8                            orbitModelIDPrefList[8];
    u8                            sizeOforbitModelIDPrefList;//Integer (1..8)
    u8                            addNavparamReq;
}sitGnssReqNavListInfoType;

typedef PACKED_STRUCT sitGnssToIdType_t
{
    u8 toIds;
 /*ID_UNKNOWN = 0x00,
 GPS  = 0x01,
 GALILEO = 0x02,
 QZSS = 0x03,
 GLONASS = 0x04*/
}sitGnssToIdType;

typedef PACKED_STRUCT sitGnssTimeModelElementReq_t
{
    sitGnssToIdType            toIds;
    u8                        deltaT;
}sitGnssTimeModelElementReq;

typedef PACKED_STRUCT sitGnssDifferentialCorrectionsREQ_t
{
    sitGnssSignalIdType            signalsReq;
    u8                            validityTimeReq;
}sitGnssDifferentialCorrectionsREQ;

typedef PACKED_STRUCT sitGnssNavigationModelREQ_t
{
    u8                            modelReqType;//0x01 : STORED_NAV_LIST  0x02 : REQ_NAV_LIST
    PACKED_UNION{
        sitGnssStoredNaviListInfoType    storedNavListInfo;
        sitGnssReqNavListInfoType        reqNavListInfo;
        }sitGnssNavListInfo;
}sitGnssNavigationModelREQ;

typedef PACKED_STRUCT sitGnssDataBitAssistanceREQ_t
{
    u8                        bitMask; //0x00 : NOT_USED. 0x01 : BM_TOD_FRAC  0x02 : BM_DATA_BITS_REQ
    u16                        tod;//INTEGER (0..3599)
    u16                        todFrac;//INTEGER (0..999), Scale factor 1 millisecond.
    u8                        dataBitInterval;//INTEGER (0..15)
    sitGnssSignalIdType        signalIDs;
    u8                        dataBitsReq[64];//Refer to Satellite-Id table ####
    u8                        sizeOfDataBitsReq ;//Integer (1.. 64)
}sitGnssDataBitAssistanceREQ;

typedef PACKED_STRUCT sitGnssAcquisitionAssistanceREQ_t
{
    u8                        signalID;//INTEGER (0 .. 7)
}sitGnssAcquisitionAssistanceREQ;

typedef PACKED_STRUCT sitGnssAlmanacREQ_t
{
    u8                     bitMask;//0x01 : BM_MODEL_ID
    u8                    modelID;
    /*
    0 : Not Used
    1 : GALILEO
    2 : GPS
    2 : QZSS
    5 : GLONASS
    6 : SBAS
    */
}sitGnssAlmanacREQ;

typedef PACKED_STRUCT sitGnssUTCModelREQ_t
{
    u8                     bitMask;//0x01 : BM_MODEL_ID
    u8                    modelID;
    /*
    0 : Not Used
    1 : GALILEO
    2 : GPS
    2 : QZSS
    5 : GLONASS
    6 : SBAS
    */
}sitGnssUTCModelREQ;

typedef PACKED_STRUCT sitGnssGenericAssistDataREQ_t
{
    u16                                bitMask;    //sitGnssGenAssistDataSuppElementType
    sitGnssId                        gnssID;
    u8                                        sbasIds;
    sitGnssTimeModelElementReq        timeModels;
    //u8                            sizeOfTimeModels;// 1 to 15 //future use
    sitGnssDifferentialCorrectionsREQ     diffCorrections;
    sitGnssNavigationModelREQ         navModel;
    //sitGnssRealTimeIntegrityREQ        rti; //future use
    sitGnssDataBitAssistanceREQ        dataBitAssistance;
    sitGnssAcquisitionAssistanceREQ     acqAssistance;
    sitGnssAlmanacREQ                alm;
    sitGnssUTCModelREQ                 utc;
    //sitGnssAuxiliaryInformationREQ         auxiliaryInfo; //future
}sitGnssGenericAssistDataREQ;

typedef PACKED_STRUCT sitAGnssDataReq_t
{
    u8                            bitMask;//0x00:NOT _USED 0x01:BM_CMN_ASSIST 0x02:BM_GEN_ASSIST
    sitGnssCommonAssistDataREQ     cmnAssist;
    sitGnssGenericAssistDataREQ     genAssist[SIT_LPP_MAX_AGNSS_CNT];
    u8                            sizeOfGenAssist; //Integer (1..2)
}sitAGnssDataReq;

typedef PACKED_STRUCT sitGnssTowAssistElement_t
{
    u8                            satelliteID;//INTEGER (1..64)
    u16                            tlmWord;//INTEGER (0..16383)
    u8                            antiSpoof;//INTEGER (0..1)
    u8                            alert;//INTEGER (0..1)
    u8                            tlmRsvdBits;//INTEGER (0..3)
}sitGnssTowAssistElement;

typedef PACKED_STRUCT sitGnssSystemTime_t
{
    u8                            bitMask;
    /*0x00 : NOT_USED.
    0x01 : BM_TIME_OF_DAY_FRAC_MSEC
    0x02 : BM_NOTI_OF_LEAP_SEC
    0x04 : BM_TOW_ASSIST*/
    sitGnssId                    timeID;
    u16                            dayNumber;//INTEGER (0..32767)
    u32                            timeOfDay; //INTEGER (0..86399)
    u16                            timeOfDayFrac_msec;//INTEGER (0..999)
    u8                            notiOfLeapSec;//BIT STRING (SIZE(2))
    sitGnssTowAssistElement        towAssist[64];
    u8                            sizeOfTowAssist;//Integer (1..64)
}sitGnssSystemTime;

typedef PACKED_STRUCT sitGnssPlmnIdentity_t
{
    u8                mcc[3];//INTEGER (0..9)
    u8                mnc[3];//INTEGER (0..9)
    u8                sizeOfMcc;//INTEGER (0..9)
    u8                sizeOfMnc;//INTEGER (0..9)
}sitGnssPlmnIdentity;

typedef PACKED_STRUCT sitGnssCellIDIdentity_t
{
    u8                        cidType;//0x01 : EUTRA, 0x02 : UTRA
    PACKED_UNION
    {
        u32                        eutra;//BIT STRING (SIZE (28))
        u32                        utra;//BIT STRING (SIZE (32))
    }u;
}sitGnssCellIDIdentity;

typedef PACKED_STRUCT sitGnssCellGlobalIdEutraAndUtra_t
{
    sitGnssPlmnIdentity            plmnIdentity;
    sitGnssCellIDIdentity            cellIdentity;
}sitGnssCellGlobalIdEutraAndUtra;

typedef PACKED_STRUCT sitGnssCellIdEutra_t
{
    u8                                    bitMask;// 1 -FDD_TYPE, 2 -TDD_TYPE
    u16                                    physCellId;//INTEGER(0..503)
    sitGnssCellGlobalIdEutraAndUtra        cellGlobalIdEUTRA;
    u16                                    earfcn;//INTEGER (0.. 65535)
}sitGnssCellIdEutra;


typedef PACKED_STRUCT sitGnssPrimaryCPICHInfo_t
{
    u16                    primaryCPICHInfo;//INTEGER(0..511)
}sitGnssPrimaryCPICHInfo;

typedef PACKED_STRUCT sitGnssCellParameters_t
{
    u8                    cellParameters;//INTEGER(0..127)
}sitGnssCellParameters;

typedef PACKED_STRUCT sitGnssUtraMode_t
{
    u8                        modeType;//0x01 : FDD_TYPE,0x02 : TDD_TYPE
    PACKED_UNION
    {
        sitGnssPrimaryCPICHInfo     fdd;
        sitGnssCellParameters     tdd;
    }u;
}sitGnssUtraMode;

typedef PACKED_STRUCT sitGnssCellIdUtra_t
{
    u8                                    bitMask;
    sitGnssUtraMode                        mode;
    sitGnssCellGlobalIdEutraAndUtra        cellGlobalIdUTRA;
    u16                                    uarfcn;//INTEGER(0..4095)
}sitGnssCellIdUtra;

typedef PACKED_STRUCT sitGnssCellGlobalIdGERAN_t
{
    sitGnssPlmnIdentity            plmnIdentity;
    u16                            locationAreaCode;//BIT STRING(SIZE(16))
    u16                            cellIdentity;//BIT STRING(SIZE(16))
}sitGnssCellGlobalIdGERAN;

typedef PACKED_STRUCT sitGnssReferenceFrame_t
{
    u8                            bitMask;
    u16                            referenceFN;//INTEGER(0..65535)
    u8                            referenceFNMSB;//INTEGER(0..63)
}sitGnssReferenceFrame;

typedef PACKED_STRUCT  sitGnssCellIdGsm_t
{
    u8                        bitMask;
    /*
    0x00 : NOT_USED.
    0x01 : BM_CELL_GLOBAL_ID
    0x02 : BM_DELTA_GNSS_TOD
    */
    u16                        bcchCarrier;//INTEGER(0..1023)
    u8                        bsic;//
    sitGnssCellGlobalIdGERAN    cellGlobalIdGERAN;
}sitGnssCellIdGsm;

typedef PACKED_UNION{
    sitGnssCellIdEutra                eUTRA;
    sitGnssCellIdUtra                    uTRA;
    sitGnssCellIdGsm                    gSM;
}u_networkTime_t;

typedef PACKED_STRUCT sitGnssCellId_t
{
    u8                        cellIDType;//0x01 : EUTRA, 0x02 : UTRA, 0x04 : GSM
    u_networkTime_t            u_networkTime;
}sitGnssCellId;

typedef PACKED_STRUCT sitGnssNetworkTime_t
{
    u8                bitMask;
    u16                secondsFromFrameStructureStart;
    u32                fractionalSecondsFromFrameStructureStart;
    s8                frameDrift;
    sitGnssCellId    cellID;
}sitGnssNetworkTime;

typedef PACKED_STRUCT sitGnssReferenceTimeForOneCell_t
{
    u8                            bitMask; //0x01 : BM_BS_ALIGN
    sitGnssNetworkTime            networkTime;
    u8                            referenceTimeUnc;//INTEGER (0..127)
    u8                            bsAlign;
}sitGnssReferenceTimeForOneCell;

typedef PACKED_STRUCT sitGnssProvideCommRefTime_t
{
    u8                                bitMask; //0x00 : NOT_USED. 0x01 : BM_REF_TIME_UNC  0x02 : BM_REF_TIME_FOR_CELLS
    sitGnssSystemTime                systemTime;
    u8                                refTimeUnc;//INTEGER (0..127)
    sitGnssReferenceTimeForOneCell    refTimeForCells[SIT_LPP_MAX_SV_CNT];
    u8                                sizeOfRefTimeForCells;//Integer (1.. 16)
}sitGnssProvideCommRefTime;

typedef PACKED_STRUCT sitGnssEllipsoidPointWithAltitudeAndUncertaintyEllipsoid_t
{
    u8            latitudeSign;//0x00 : north,0x01 : south
    u32            degreesLatitude;///* 23 bit field */
    s32            degreesLongitude;// /* 24 bit field */
    u8            altitudeDirection;//0x00 : height, 0x01 : depth
    u16            altitude;///* 15 bit field */
    u8            uncertaintySemiMajor;//INTEGER(0..127)
    u8            uncertaintySemiMinor;//INTEGER(0..127)
    u8            orientationMajorAxis;//INTEGER(0..179)
    u8            uncertaintyAltitude;//INTEGER(0..127)
    u8            confidence;//INTEGER(0..100)
}sitGnssEllipsoidPointWithAltitudeAndUncertaintyEllipsoid;

typedef PACKED_STRUCT sitGnssProvideCommRefLocation_t
{
    sitGnssEllipsoidPointWithAltitudeAndUncertaintyEllipsoid    threeDlocation;
}sitGnssProvideCommRefLocation;

typedef PACKED_STRUCT sitGnssKlobucharModelParameter_t
{
    u8                        dataID;//BITSTRING(SIZE(2))
    s8                        alfa0;//INTEGER(-128..127)
    s8                        alfa1;//INTEGER(-128..127)
    s8                        alfa2;//INTEGER(-128..127)
    s8                        alfa3;//INTEGER(-128..127)
    s8                        beta0;//INTEGER(-128..127)
    s8                        beta1;//INTEGER(-128..127)
    s8                        beta2;//INTEGER(-128..127)
    s8                        beta3;//INTEGER(-128..127)
}sitGnssKlobucharModelParameter;

typedef PACKED_STRUCT sitGnssNeQuickModelParameter_t
{
    u8                    bitMask;
    /*
    0x00 : NOT_USED.
    0x01 : BM_IONO_STORM_FLAG1
    0x02 : BM_IONO_STORM_FLAG2
    0x04 : BM_IONO_STORM_FLAG3
    0x08 : BM_IONO_STORM_FLAG4
    0X10 : BM_IONO_STORM_FLAG5
    */
    u16                    ai0;//INTEGER(0..4095)
    u16                    ai1;//INTEGER(0..4095)
    u16                    ai2; //INTEGER(0..4095)
    u8                    ionoStormFlag1;//INTEGER(0..1)
    u8                    ionoStormFlag2;//INTEGER(0..1)
    u8                    ionoStormFlag3;//INTEGER(0..1)
    u8                    ionoStormFlag4;//INTEGER(0..1)
    u8                    ionoStormFlag5;//INTEGER(0..1)
}sitGnssNeQuickModelParameter;


typedef PACKED_STRUCT sitGnssProvideCommIonosphericModel_t
{
    u8                        bitMask;
    /*
    0x00 : NOT_USED.
    0x01 : BM_KLOBUCHAR_MODEL
    0x02 : BM_NE_QUICK_MODEL
    */
    sitGnssKlobucharModelParameter    klobucharModel;
    sitGnssNeQuickModelParameter        neQuickModel;
}sitGnssProvideCommIonosphericModel;

typedef struct sitGnssProvideCommEarthOrientationParameters_t
{
    u16                        teop;//INTEGER(0..65535)
    s32                        pmX;//INTEGER(-1048576..1048575)
    s16                        pmXdot;//INTEGER(-16384..16383)
    s32                        pmY;//INTEGER(-1048576..1048575)
    s16                        pmYdot;//INTEGER(-16384..16383)
    s32                        deltaUT1;//INTEGER(-1073741824..1073741823)
    s32                        deltaUT1dot;//deltaUT1dot
}sitGnssProvideCommEarthOrientationParameters;

typedef PACKED_STRUCT sitGnssCommonAssistData_t
{
    u8                                        bitMask; //sitGnssCommAssistDataSuppElementType
    sitGnssProvideCommRefTime                refTime;
    sitGnssProvideCommRefLocation            refLocation;
    sitGnssProvideCommIonosphericModel        ionoModel;
    sitGnssProvideCommEarthOrientationParameters    earthOrientationParameters;
}sitGnssCommonAssistData;

typedef PACKED_STRUCT sitGnssTimeModelElement_t
{
    u8                        bitMask;
    /*
    0x00 : NOT_USED.
    0x01 : BM_TA1
    0x02 : BM_TA2
    0x04 : BM_WEEK_NUMBER
    0x08 : BM_DELTA_T
    */
    u16                        gnssTimeModelRefTime;
    s32                        tA0;
    s16                        tA1;
    s8                        tA2;
    sitGnssToIdType            gnssToID;
    u16                        weekNumber;
    s8                        deltaT;
}sitGnssTimeModelElement;

typedef PACKED_STRUCT sitGnssSVIdType_t
{
    u8                        satelliteId;//INTEGER (0..63)
}sitGnssSVIdType;

typedef PACKED_STRUCT sitGnssCorrectionsElement_t
{
    u8                        bitMask;
    /*
    0x00 : NOT_USED.
    0x01 : BM_UDRE_GROWTH_RATE
    0x02 : BM_UDRE_VALIDITY_TIME
    */
    sitGnssSVIdType            svID      ;
    u16                        iod;//BIT STRING (SIZE(11))
    u8                        udre;//
    /*
    0x00 : UDRE   1.0 m
    0x01: 1.0 m < UDRE   4.0 m
    0x02: 4.0 m < UDRE ? 8.0 m
    0x03: 8.0 m < UDRE
    */
    s16                        pseudoRangeCor;//INTEGER (-2047..2047)
    s8                        rangeRateCor;////INTEGER (-127..127)
    u8                        udreGrowthRate;
    /*
    0x00 : 1.5
    0x01 : 2
    0x02 : 4
    0x03 : 6
    0x04 : 8
    0x05 : 10
    0x06 : 12
    0x07 : 16
    */
    u8                    udreValidityTime;
    /*
    0x00 : 20
    0x01 : 40
    0x02 : 80
    0x03 : 160
    0x04 : 320
    0x05 : 640
    0x06 : 1280
    0x07 : 2560
    */
}sitGnssCorrectionsElement;

typedef PACKED_STRUCT sitDGnssSgnTypeElement_t
{
    sitGnssSignalIdType            signalID;
    u8                                 statusHealth;//sitGnssstatusHealth
    sitGnssCorrectionsElement          dgnssSatList[SIT_LPP_MAX_SV_CNT];
    u8                            sizeOfDgnssSatList;//Integer (1..16)
}sitDGnssSgnTypeElement;

typedef PACKED_STRUCT sitGnssDifferentialCorrections_t
{
    u16                    refTime;
    sitDGnssSgnTypeElement     sgnTypeList;
}sitGnssDifferentialCorrections;

typedef PACKED_STRUCT sitGnssStandardClockModelElement_t
{
    u8        bitMask;
    /*
    0x00 : NOT_USED.
    0x01 : BM_STAN_CLOCK_TGD
    0x02 : BM_STAN_MODEL_ID
    */
    u16        stanClockToc;//INTEGER(0..16383)
    s16    stanClockAF2;//INTEGER(-2048..2047)
    s32     stanClockAF1;//INTEGER(-131072..131071)
    s32    stanClockAF0;//INTEGER(-134217728..134217727)
    s16    stanClockTgd;//INTEGER(-512..511)
    u8    stanModelID; //0x0 : I/Nav, 0x1 : F/Nav
}sitGnssStandardClockModelElement;

typedef PACKED_STRUCT sitGnssNavClockModel_t
{
    u16    navToc;//    2    INTEGER(0..37799)
    s8    navaf2;//    1    INTEGER(-128..127)
    s16     navaf1;//    2    INTEGER(-32768..32767)
    s32    navaf0;//    3    INTEGER(-2097152..2097151)
    s8    navTgd;//    1    INTEGER(-128..127)
}sitGnssNavClockModel;

typedef PACKED_STRUCT sitGnssCNAVClockModel_t
{
    u8            bitMask;
    /*
    0x00 : NOT_USED.
    0x01 : BM_CNAV_ISCL1_CP
    0x02 : BM_CNAV_ISCL1_CD
    0x04 : BM_CNAV_ISCL1_CA
    0x08 : BM_CNAV_ISCL2_C
    0x10 : BM_CNAV_ISCL5_I5
    0x20 : BM_CNAV_ISCL5_Q5
    */
    u16            cnavToc;
    u16            cnavTop;
    s8            cnavURA0;//INTEGER(-16..15)
    u8            cnavURA1;//
    u8            cnavURA2;//    1    INTEGER(0..7)
    s16            cnavAf2;//    2    INTEGER(-512..511)
    s32            cnavAf1;//    3    INTEGER(-524288..524287)
    s32            cnavAf0;//    INTEGER(-33554432..33554431)
    s16            cnavTgd;//    2    INTEGER(-4096..4095)
    s16            cnavISCl1cp;//    2    INTEGER(-4096..4095)
    s16            cnavISCl1cd;//    2    INTEGER(-4096..4095)
    s16            cnavISCl1ca;//    2    INTEGER(-4096..4095)
    s16            cnavISCl2c;///    2    INTEGER(-4096..4095)
    s16            cnavISCl5i5;//    2    INTEGER(-4096..4095)
    s16            cnavISCl5q5;//    2    INTEGER(-4096..4095)
}sitGnssCNAVClockModel;

typedef PACKED_STRUCT sitGLONASSClockModel_t
{
    u8            bitMask;//0x00 : NOT_USED.0x01 : BM_GLO_DELTA_TAU
    s32            gloTau;//INTEGER(-2097152..2097151)
    s16            gloGamma;//INTEGER(-1024..1023)
    s8            gloDeltaTau;//INTEGER(-16..15)
}sitGLONASSClockModel;

typedef PACKED_STRUCT sitSBASClockModel_t
{
    u16        sbasTo;//    2    INTEGER(0..5399)
    s16        sbasAgfo;//    2    INTEGER(-2048..2047)
    s8        sbasAgf1;//    1    INTEGER(-128..127)
}sitSBASClockModel;

typedef PACKED_STRUCT sitGnssClockModel_t
{
    u8                        clockModelType;
    /*
    0x01 : STAND_CLK_MODEL
    0x02 : NAV_CLK_MODEL
    0x04 : CNAV_CLK_MODEL
    0x08 : GLONASS_CLK_MODEL
    0x10 : SBAS_CLK_MODEL
    */
    PACKED_UNION {
    sitGnssStandardClockModelElement    stdClockModelElement[SIT_LPP_MAX_AGNSS_CNT];
    sitGnssNavClockModel            navClockModel;
    sitGnssCNAVClockModel         cnavClockModel;
    sitGLONASSClockModel         glonassClockModel;
    sitSBASClockModel             sbasClockModel;
    }u;
    u8                            sizeOfStandardClockModelList;
}sitGnssClockModel;

typedef PACKED_STRUCT sitGnssNavModelKeplerianSet_t
{
    u16    keplerToe;//    2    INTEGER(0..16383)
    s32    keplerW;//    4    INTEGER(-2147483648..2147483647)
    s16    keplerDeltaN;//    2    INTEGER(-32768..32767)
    s32    keplerM0;//    4    INTEGER(-2147483648..2147483647)
    s32    keplerOmegaDot;//    3    INTEGER(-8388608..8388607)
    u32    keplerE;//    4    INTEGER(0..4294967295)
    s16    keplerIDot;//    2    INTEGER(-8192..8191)
    u32    keplerAPowerHalf;//    3    INTEGER(0..4294967295)
    s32    keplerI0;//    4    INTEGER(-2147483648..2147483647)
    s32    keplerOmega0;//    4    INTEGER(-2147483648..2147483647)
    s16    keplerCrs;//    2    INTEGER(-32768..32767)
    s16    keplerCis;//   2    INTEGER(-32768..32767)
    s16    keplerCus;//    2    INTEGER(-32768..32767)
    s16    keplerCrc;//    2    INTEGER(-32768..32767)
    s16    keplerCic;//   2    INTEGER(-32768..32767)
    s16    keplerCuc;//    2    INTEGER(-32768..32767)
}sitGnssNavModelKeplerianSet;

typedef PACKED_STRUCT sitGnssephemSF1Rsvd_t
{
    u32        reserved1;//INTEGER(0..8388607)
    u32        reserved2;//INTEGER(0..16777215)
    u32        reserved3;//INTEGER(0..16777215)
    u16        reserved4;//INTEGER(0..65535)
}sitGnssephemSF1Rsvd;

typedef PACKED_STRUCT sitGnssAddNAVparam_t
{
    u8            ephemCodeOnL2;//INTEGER(0..3)
    u8            ephemL2Pflag;//INTEGER(0..31)
    sitGnssephemSF1Rsvd        ephemSF1Rsvd;
    u32                ephemAODA;//INTEGER(0..31)
}sitGnssAddNAVparam;

typedef PACKED_STRUCT sitGnssNavModelNavKeplerianSet_t
{
    u8    bitMask;//0x01 : ADD_NAV_PARAM
    u8    navURA    ;// 1    INTEGER(0..15)
    u8    navFitFlag;//    1    INTEGER(0..1)
    u16    navToe;//    2    INTEGER(0..37799)
    s32    navOmega;//    4    INTEGER(-2147483648..2147483647)
    s16    navDeltaN;//    2    INTEGER(-32768..32767)
    s32    navM0;//    4    INTEGER(-2147483648..2147483647)
    s32    navOmegaADot;//    3    INTEGER(-8388608..8388607)
    u32    navE;//    3    INTEGER(0..4294967295)
    s16    navIDot;//    2    INTEGER(-8192..8191)
    u32    navAPowerHalf;//    3    INTEGER(0..4294967295)
    s32    navI0;//    4    INTEGER(-2147483648..2147483647)
    s32    navOmegaA0;//    4    INTEGER(-2147483648..2147483647)
    s16    navCrs;//    2    INTEGER(-32768..32767)
    s16    navCis;//    2    INTEGER(-32768..32767)
    s16    navCus;//    2    INTEGER(-32768..32767)
    s16    navCrc;//    2    INTEGER(-32768..32767)
    s16    navCic;//    2    INTEGER(-32768..32767)
    s16    navCuc;//    2    INTEGER(-32768..32767)
    sitGnssAddNAVparam    addNAVparam;
}sitGnssNavModelNavKeplerianSet;

typedef PACKED_STRUCT sitGnssNavModelCNAVKeplerianSet_t
{
    u16    cnavTop;//    2    INTEGER(0..2015)
    s8    cnavURAindex;//    1    INTEGER(-16..15)
    s32    cnavDeltaA;//    4    INTEGER(-33554432..33554431)
    s32    cnavAdot    ; //  //4    INTEGER(-16777216..16777215)
    s32    cnavDeltaNo  ;//    3    INTEGER(-65536..65535)
    s32    cnavDeltaNoDot;//    3    INTEGER(-4194304..4194303)
    s64    cnavMo;//    5    INTEGER(-4294967296..4294967295)
    u64    cnavE;//    5    INTEGER(0..8589934591)
    s64    cnavOmega;//    5    INTEGER(-4294967296..4294967295)
    s64    cnavOMEGA0;//    5    INTEGER(-4294967296..4294967295)
    s32    cnavDeltaOmegaDot;//    3    INTEGER(-65536..65535)
    s64    cnavIo;//    5    INTEGER(-4294967296..4294967295)
    s16    cnavIoDot;//    2    INTEGER(-16384..16383)
    s16    cnavCis;//    2    INTEGER(-32768..32767)
    s16    cnavCic;//    2    INTEGER(-32768..32767)
    s32    cnavCrs;//    3    INTEGER(-8388608..8388607)
    s32    cnavCrc;//    3    INTEGER(-8388608..8388607)
    s32    cnavCus;//    3    INTEGER(-1048576..1048575)
    s32    cnavCuc;//    3    INTEGER(-1048576..1048575)
}sitGnssNavModelCNAVKeplerianSet;

typedef PACKED_STRUCT sitGnssNavModelGLONASSECEF_t
{
    u8    gloEn;//    1    INTEGER(0..31)
    u8    gloP1;//    1    BITSTRING(SIZE(2))
    u8    gloP2;//    1    0x00 : False,0x01 : True
    u8    gloM;//    1    INTEGER(0..3)
    s32    gloX;//    4    INTEGER(-67108864..67108863)
    s32    gloXdot;//    3    INTEGER(-8388608..8388607)
    s8    gloXdotdot;//    1    INTEGER(-16..15)
    s32    gloY;//    4    INTEGER(-67108864..67108863)
    s32    gloYdot;//    3    INTEGER(-8388608..8388607)
    s8    gloYdotdot;//    1    INTEGER(-16..15)
    s32    gloZ;//    3    INTEGER(-67108864..67108863)
    s32    gloZdot;//    3    INTEGER(-8388608..8388607)
    s8    gloZdotdot;//    1    INTEGER(-16..15)
}sitGnssNavModelGLONASSECEF;

typedef PACKED_STRUCT sitGnssNavModelSBASECEF_t
{
    u8    bitMask;//0x01 : BM_SBAS_TO
    u16    sbasTo;//    2    INTEGER(0..5399)
    u8    sbasAccuracy;//    1    BITSTRING(SIZE(4))
    s32    sbasXg;//    4    INTEGER(-536870912..536870911)
    s32    sbasYg;//    4    INTEGER(-536870912..536870911)
    s32    sbasZg;//    4    INTEGER(-16777216..16777215)
    s32    sbasXgDot;    //    3    INTEGER(-65536..65535)
    s32    sbasYgDot;//    3    INTEGER(-65536..65535)
    s32    sbasZgDot;//    3    INTEGER(-131072..131071)
    s16    sbasXgDotDot;//    2    INTEGER(-512..511)
    s16    sbagYgDotDot    ;//    2    INTEGER(-512..511)
    s16    sbasZgDotDot    ;//   2    INTEGER(-512..511)
}sitGnssNavModelSBASECEF;

#define KELP_SET        0x01
#define NAV_KEPL_SET    0x02
#define CNAV_KEPL_SET   0x04
#define GLONASS_ECEF    0x08
#define SBAS_ECEF       0x10

typedef PACKED_STRUCT sitGnssOrbitModel_t
{
    u8                            orbitModelType;
    /*0x01 : KELP_SET
    0x02 : NAV_KEPL_SET
    0x04 : CNAV_KEPL_SET
    0x08 : GLONASS_ECEF
    0x10 : SBAS_ECEF
    */
    PACKED_UNION{
    sitGnssNavModelKeplerianSet        keplerianSet;
    sitGnssNavModelNavKeplerianSet    navKeplerianSet;
    sitGnssNavModelCNAVKeplerianSet    cnavKeplerianSet;//
    sitGnssNavModelGLONASSECEF        glonassECEF;//
    sitGnssNavModelSBASECEF         sbasECEF;
    }u;
}sitGnssOrbitModel;

typedef PACKED_STRUCT sitGnssNavModelSatelliteElement_t
{
    sitGnssSVIdType            svID;
    u8                        svHealth;//BIT STRING (SIZE(8))
    u16                        iod;//BIT STRING (SIZE(11))
    sitGnssClockModel        gnssClockModel;
    sitGnssOrbitModel            gnssOrbitModel;
}sitGnssNavModelSatelliteElement;

typedef PACKED_STRUCT sitGnssNavigationModel_t
{
    u8                                nonBroadcastIndFlag;//INTEGER (0..1)
    sitGnssNavModelSatelliteElement    gnssSatelliteList[SIT_LPP_MAX_SV_CNT];
    u8                                sizeOfGnssSatelliteList;//INTEGER (1..2)
}sitGnssNavigationModel;

typedef PACKED_STRUCT sitGnssBadSignalElement_t
{
    u8                    bitMask;//0x00 : NOT_USED., 0x01 : BM_BAD_SIGNAL_ID
    sitGnssSVIdType        badSVID;
    sitGnssSignalIdType    badSignalID;
}sitGnssBadSignalElement;

typedef PACKED_STRUCT sitGnssDataBitsSgnElement_t
{
    sitGnssSignalIdType        signalType;
    u8                        dataBits[128];
}sitGnssDataBitsSgnElement;

typedef PACKED_STRUCT sitGnssDataBitSatElement_t
{
    sitGnssSVIdType            svID;
    sitGnssDataBitsSgnElement     dataBitsSgnList[8];
    u8                    sizeOfDataBitsSgnList;
}sitGnssDataBitSatElement;

typedef PACKED_STRUCT sitGnssDataBitAssistance_t
{
    u8                    bitMask;
    u16                    gnssTod;//INTEGER(0..3599)
    u16                    gnssTodFrac;//INTEGER(0..999)
    sitGnssDataBitSatElement        gnssDataBitsSatList[SIT_LPP_MAX_SV_CNT];
    u8                        sizeOfGnssDataBitsSatList;//Integer(1...16)
}sitGnssDataBitAssistance;

typedef PACKED_STRUCT sitGnssAcquisitionAssistElement_t
{
    u8                        bitMask;
    sitGnssSVIdType            svId;
    s16                        doppler0;//    2    INTEGER(-2048..2047)
    u8                        doppler1;//    1    INTEGER(0..63)
    u8                        dopplerUncertainty;//    1    INTEGER(0..4)
    u16                        codePhase;///    2    INTEGER(0..1022)
    u8                        intCodePhase;//    1    INTEGER(0..127)
    u8                        codePhaseSearchWindow;//    1    INTEGER(0..31)
    u16                        azimuth;//    2    INTEGER(0..511)
    u8                        elevation;//    1;    INTEGER(0..127)
    u8                        codePhase1023;
    u8                        dopplerUncertaintyExt_r10;// 1
    /*0x00 : d60
    0x01 : d80
    0x02 : d100
    0x03 : d120
    0x04 : noInformation
    */
}sitGnssAcquisitionAssistElement;

typedef PACKED_STRUCT sitGnssAcquisitionAssistance_t
{
    sitGnssSignalIdType        signalID;
    sitGnssAcquisitionAssistElement        acqAssistList[SIT_LPP_MAX_SV_CNT];
    u8                                sizeOfAcqAssistList;//Integer (1.. 2)
    u8                                Confidence;//INTEGER(0..100)
}sitGnssAcquisitionAssistance;

typedef PACKED_STRUCT sitGnssAlmanacKeplerianSet_t
{
    sitGnssSVIdType            svID;
    u16    kepAlmanacE;//    2    INTEGER(0..2047)
    s16    kepAlmanacDeltaI;//    2    INTEGER(-1024..1023)
    s16    kepAlmanacOmegaDot;//    2    INTEGER(-1024..1023)
    u8    kepSVHealth;//    1    INTEGER(0..15)
    s32    kepAlmanacAPowerHalf;//    3    INTEGER(-65536..65535)
    s16    kepAlmanacOmega0;//    2    INTEGER(-32768..32767)
    s16    kepAlmanacW;//    2    INTEGER(-32768..32767)
    s16    kepAlmanacM0;//    2    INTEGER(-32768..32767)
    s16    kepAlmanacAF0;//    2    INTEGER(-8192..8191)
    s16    kepAlmanacAF1;//    2    INTEGER(-1024..1023)
}sitGnssAlmanacKeplerianSet;

typedef PACKED_STRUCT sitGnssAlmanacNavKeplerianSet_t
{
    sitGnssSVIdType            svID;
    u16    navAlmE;//    2    INTEGER(0..65535)
    s16    navAlmDeltaI;//    2    INTEGER(-32768..32767)
    s16    navAlmOMEGADOT ;//      2      INTEGER(-32768..32767)
    u8    navAlmSVHealth;//    1    INTEGER(0..255)
    u32    navAlmSqrtA;//    3    INTEGER(0..16777215)
    s32    navAlmOMEGAo;//    3    INTEGER(-8388608..8388607)
    s32    navAlmOmega;//    3    INTEGER(-8388608..8388607)
    s32    navAlmMo;//    3    INTEGER(-8388608..8388607)
    s16    navAlmaf0;//    2    INTEGER(-1024..1023)
    s16    navAlmaf1;//    2    INTEGER(-1024..1023)
}sitGnssAlmanacNavKeplerianSet;

typedef PACKED_STRUCT sitGnssAlmanacReducedKeplerianSet_t
{
    sitGnssSVIdType            svID;
    s8    redAlmDeltaA;//    1    INTEGER(-128..127)
    s8    redAlmOmega0;//    1    INTEGER(-64..63)
    s8    redAlmPhi0;//    1    INTEGER(-64..63)
    u8    redAlmL1Health;//    1    0x00 : false, 0x01 : true
    u8    redAlmL2Health;//    1    0x00 : false, 0x01 : true
u8    redAlmL5Health;//    1    0x00 : false, 0x01 : true
}sitGnssAlmanacReducedKeplerianSet;

typedef PACKED_STRUCT sitGnssAlmanacMidiAlmanacSet_t
{
    sitGnssSVIdType            svID;
    u16    midiAlmE;//    2    INTEGER(0..2047)
    s16    midiAlmDeltaI;//    2    INTEGER(-1024..1023)
    s16    midiAlmOmegaDot;//    2    INTEGER(-1024..1023)
    u32    midiAlmSqrtA;//    3    INTEGER(0..131071)
    s16    midiAlmOmega0;//    2    INTEGER(-32768..32767)
    s16    midiAlmOmega;//    2    INTEGER(-32768..32767)
    s16    midiAlmMo;//    2    INTEGER(-32768..32767)
    s16    midiAlmaf0;//    2    INTEGER(-1024..1023)
    s16    midiAlmaf1;//    2    INTEGER(-512..511)
    u8    midiAlmL1Health;//    1    0x00 : false, 0x01 : true
    u8    midiAlmL2Health;//    1    0x00 : false, 0x01 : true
    u8    midiAlmL5Health;//    1    0x00 : false, 0x01 : true
}sitGnssAlmanacMidiAlmanacSet;

typedef PACKED_STRUCT sitGnssAlmanacGlonassAlmanacSet_t
{
    u8    bitMask;//    1    0x00 : NOT_USED, 0x01 : BM_GLOALM_MA
    u16    gloAlmNA;//    2    INTEGER(1..1461)
    u8    gloAlmnA;//      1    INTEGER(1..24)
    u8    gloAlmHA;  //    1    INTEGER(0..31)
    s32    gloAlmLambdaA;//    3    INTEGER(-1048576..1048575)
    u32    gloAlmtlambdaA;//    3    INTEGER(0..2097151)
    s32    gloAlmDeltaIa;//    3    INTEGER(-131072..131071)
    s32    gloAlmDeltaTA;//    3    INTEGER(-2097152..2097151)
    s8    gloAlmDeltaTdotA;//    1    INTEGER(-64..63)
    u16    gloAlmEpsilonA;//    2    INTEGER(0..32767)
    s16    gloAlmOmegaA;///    2    INTEGER(-32768..32767)
    s16    gloAlmTauA;//    2    INTEGER(-512..511)
    u8    gloAlmCA;//    1    INTEGER(0..1)
    u8    gloAlmMA;//    1    BITSTRING(SIZE(2))
}sitGnssAlmanacGlonassAlmanacSet;

typedef PACKED_STRUCT sitGnssAlmanacEcefsbasAlmanacSet_t
{
    u8    sbasAlmDataID;//    1    INTEGER(0..3)
    sitGnssSVIdType        svID;
    u8    sbasAlmHealth;//    1    BITSTRING(SIZE(8))
    s16    sbasAlmXg;//    2    INTEGER(-16384..16383)
    s16    sbasAlmYg;//    2    INTEGER(-16384..16383)
    s16    sbasAlmZg;//    2    INTEGER(-256..255)
    s8    sbasAlmXgdot;//    1    INTEGER(-4..3)
    s8    sbasAlmYgDot;//    1    INTEGER(-4..3)
    s8    sbasAlmZgDot;//    1    INTEGER(-8..7)
    u16    sbasAlmTo;//    2    INTEGER(0..2047)
}sitGnssAlmanacEcefsbasAlmanacSet;

typedef PACKED_STRUCT sitGnssAlmanacElement_t
{
    u8                        almType;//sitGnssAlmanacElementType
    PACKED_UNION {
    sitGnssAlmanacKeplerianSet     keplerianAlmanacSet;
    sitGnssAlmanacNavKeplerianSet     keplerianNavAlmanac;
    sitGnssAlmanacReducedKeplerianSet        keplerianReducedAlmanac;
    sitGnssAlmanacMidiAlmanacSet            keplerianMidiAlmanac;
    sitGnssAlmanacGlonassAlmanacSet         keplerianGLONASS;
    sitGnssAlmanacEcefsbasAlmanacSet     ecefSbasAlmanac;
    }u;
}sitGnssAlmanacElement;

typedef PACKED_STRUCT sitGnssAlmanac_t
{
    u8            bitMask;
    /*
    0x00 : NOT_USED.
    0x01 : BM_WEEK_NUMBER
    0x02 : BM_TOA
    0x04 : BM_IODA
    */
    u8    weekNumber;//    1    INTEGER(0..255)
    u8    toa;//    1    INTEGER(0..255)
    u8    ioda;//    1    INTEGER(0..3)
    u8    completeAlmanacProvided;//    1    0x00 : false,0x01 : true
    sitGnssAlmanacElement    almanacList[32];
    u8    sizeOfAlmanacList;//    1    Integer(12)
}sitGnssAlmanac;


typedef PACKED_STRUCT sitGnssUtcModelSet1_t
{
    s32    gnssUtcA1;//    3    INTEGER(-8388608..8388607)
    s32    gnssUtcA0;//    4    INTEGER(-2147483648..2147483647)
    u8    gnssUtcTot;//    1    INTEGER(0..255)
    u8    gnssUtcWNt;//    1    INTEGER(0..255)
    s8    gnssUtcDeltaTls;//    1    INTEGER(-128..127)
    u8    gnssUtcWNlsf;//    1    INTEGER(0..255)
    s8    gnssUtcDN;//    1    INTEGER(-128..127)
    s8    gnssUtcDeltaTlsf;//    1    INTEGER(-128..127)
}sitGnssUtcModelSet1;

typedef PACKED_STRUCT sitGnssUtcModelSet2_t
{
    s16    utcA0;//    2    INTEGER(-32768..32767)
    s16    utcA1;//    2    INTEGER(-4096..4095)
    s8    utcA2;//    1    INTEGER(-64..63)
    s8    utcDeltaTls;//    1    INTEGER(-128..127)
    u16    utcTot;//    2    INTEGER(0..65535)
    u16    utcWNot;//    2    INTEGER(0..8191)
    u8    utcWNlsf;//    1    INTEGER(0..255)
    u8    utcDN;//    1    BITSTRING(SIZE(4))
    s8    utcDeltaTlsf;//    1    INTEGER(-128..127)
}sitGnssUtcModelSet2;

typedef PACKED_STRUCT sitGnssUtcModelSet3_t
{
    u8    bitMask;
    /*    0x00 : NOT_USED.
    0x01 : BM_B1
    0x02 : BM_B2
    0x04 : BM_KP
    */
    u16    nA;//    2    INTEGER(1..1461)
    s32    tauC;//    4    INTEGER(-2147483648..2147483647)
    s16    b1;//    2    INTEGER(-1024..1023)
    s16    b2;//    2    INTEGER(-512..511)
    u8    kp;//    1    BITSTRING(SIZE(2))
}sitGnssUtcModelSet3;

typedef PACKED_STRUCT sitGnssUtcModelSet4_t
{
    s32    utcA1wnt;//    3    INTEGER(-8388608..8388607)
    s32    utcA0wnt;//    4    INTEGER(-2147483648..2147483647)
    u8    utcTot;//    1    INTEGER(0..255)
    u8    utcWNt;//    1    INTEGER(0..255)
    s8    utcDeltaTls;//    1    INTEGER(-128..127)
    u8    utcWNlsf;//    1    INTEGER(0..255)
    s8    utcDN;//    1    INTEGER(-128..127)
    s8    utcDeltaTlsf;//    1    INTEGER(-128..127)
    u8    utcStandardID;//    1    INTEGER(0..7)
}sitGnssUtcModelSet4;

typedef PACKED_STRUCT sitGnssUtcModel_t
{
    u8                            utcType;//sitGnssUtcModelType
    PACKED_UNION{
    sitGnssUtcModelSet1            utcModel1;
    sitGnssUtcModelSet2            utcModel2;
    sitGnssUtcModelSet3            utcModel3;
    sitGnssUtcModelSet4            utcModel4;
    }u;
}sitGnssUtcModel;

typedef PACKED_STRUCT sitGnssIdGlonassSatElement_t
{
    u8                bitMask;//0x00 : NOT_USED., 0x01 : BM_CHANNEL_NUMBER
    sitGnssSVIdType            svID;
    sitGnssSignalIdType        signalsAvailable;
    s8                    channelNumber;//INTEGER (-7..13)
}sitGnssIdGlonassSatElement;

typedef PACKED_STRUCT sitGNSSIDGPSSatElement_t
{
    sitGnssSVIdType            svID;
    sitGnssSignalIdType        signalsAvailable;
}sitGNSSIDGPSSatElement;

typedef PACKED_STRUCT sitGnssAuxiliaryInfo_t
{
    u8                                    auxInfoType;
    PACKED_UNION {
    PACKED_STRUCT {
    sitGNSSIDGPSSatElement                 idGps[SIT_LPP_MAX_SV_CNT];
    u8                                    sizeOfIdGps;//Integer (1..16)
    }sitGpsList;
    PACKED_STRUCT {
    sitGnssIdGlonassSatElement             idGlonass[SIT_LPP_MAX_SV_CNT];
    u8                                sizeOfIdGlonass;//Integer (1..16)
    }sitGlonassList;
    }u;
}sitGnssAuxiliaryInfo;

typedef PACKED_STRUCT sitGnssEUTRA_t
{
    u8                                    bitMask;// 1 -FDD_TYPE, 2 -TDD_TYPE
    u16                                    physCellId;//INTEGER(0..503)
    sitGnssCellGlobalIdEutraAndUtra        cellGlobalIdEUTRA;
    u16                                    systemFrameNumber;//INTEGER (0.. 65535)
}sitGnssEutra;

typedef PACKED_STRUCT sitGnssUTRA_t
{
    u8                                    bitMask;
    sitGnssUtraMode                        mode;
    sitGnssCellGlobalIdEutraAndUtra        cellGlobalIdUTRA;
    u16                                    referenceSystemFrameNumber;//INTEGER(0..4095)
}sitGnssUtra;

typedef PACKED_STRUCT  sitGnssGSM_t
{
    u8                        bitMask;
    /*
    0x00 : NOT_USED.
    0x01 : BM_CELL_GLOBAL_ID
    0x02 : BM_DELTA_GNSS_TOD
    */
    u16                        bcchCarrier;//INTEGER(0..1023)
    u8                        bsic;//
    sitGnssCellGlobalIdGERAN    cellGlobalId;
    sitGnssReferenceFrame    referenceFrame;
    u16                        deltaGNSS_TOD;
}sitGnssGsm;

typedef PACKED_STRUCT sitGnssNwTimeType_t
{
    u8  networkTimeType;//0x01 : EUTRA 0x02 : UTRA 0x04 : GSM
    PACKED_UNION{
    sitGnssEutra                eUTRA;
    sitGnssUtra                uTRA;
    sitGnssGsm                gSM;
    }networkTime;
}sitGnssNwTimeType;

typedef PACKED_STRUCT sitGnssMeasRefTimeType_t
{
    u8                bitmask;//sitGnssMeasRefTimeMask
    u32                 gnssTODMsec;//(0..3599999)
    u16                 gnssTODFrac; //(0..3999)
    u16                 gnssTODUnc;//(0..127)
    sitGnssId                  gnssTimeID;//sitGnssIdType
    sitGnssNwTimeType networkTime;
}sitGnssMeasRefTimeType;

typedef PACKED_STRUCT
{
    sitGnssMeasRefTimeType         measurementReferenceTime;
    sitGnssIdBitmap                 agnss_List;
}sitGnssLocationInfoType;

typedef PACKED_STRUCT
{
    u8                    bitMask;//sitGnssSatMeasElementTypeMask
    sitGnssSVIdType         svID;
    u8                     cNo;//(0..63)
    u8                         mpathDet;//0x00 : notMeasured 0x01 : low 0x02 : medium 0x03 : high
    u8                         carrierQualityInd;//(0-3)
    u32                     codePhase;//(0..2097151)
    u16                     integerCodePhase;//(0..127)
    u16                     codePhaseRMSError;//(0-63)
    s16                     doppler;//(-32768..32767)
    u32                     adr;//(0..33554431)
}sitGnssSatMeasElementType;

typedef PACKED_STRUCT sitGnssSgnMeasElementType_t
{
    u8             bitmask; //0x00 : NOT_USED. 0x01 : BM_CODE_PHASE_AMBIGUITY
    sitGnssSignalIdType    signalID;
    u16 codePhaseAmbiguity;//INTEGER(0..127)
    sitGnssSatMeasElementType SatMeasList[SIT_SATELLITE_ID_LENGTH];
    u8 sizeOfMeasurementList;
}sitGnssSgnMeasElementType;

typedef PACKED_STRUCT
{
    sitGnssId gnssID;
    sitGnssSgnMeasElementType gnssSgnMeasList;
    u8                                   sizeOfMeasurementList;
}sitGnssMeasForOneGNSSType;

typedef PACKED_STRUCT sitGnssSignalMeasInfoType_t
{
    sitGnssMeasRefTimeType measureRefTime;
    sitGnssMeasForOneGNSSType measurementList[SIT_LPP_MAX_AGNSS_CNT];
    u8 sizeOfMeasurementList; //(1..2)
}sitGnssSignalMeasInfoType;

typedef PACKED_STRUCT sitGnssGenericAssistData_t
{
    u16                            bitMask;//sitGnssGenAssistDataSuppElementType
    sitGnssId                    gnss_ID;
    sitGnssSbasId                sbas_ID;
    sitGnssTimeModelElement        timeModels;
    //u8                        sizeOfTimeTimeModels; // Future Use
    sitGnssDifferentialCorrections    diffCorrections;
    sitGnssNavigationModel        navModel;
    sitGnssBadSignalElement        rti[8];
    u8                            sizeOfRti;//Integer (1..8)
    //sitGnssDataBitAssistance        dataBitAssistance; //Future Use
    sitGnssAcquisitionAssistance    acqAssistance;
    sitGnssAlmanac                almanac;
    sitGnssUtcModel                utcModel;
    sitGnssAuxiliaryInfo            auxiliaryInfo;
}sitGnssGenericAssistData;

typedef PACKED_STRUCT sitGnssEllipsoidPointType_t
{
    u8         latSign;//0x00 : north 0x01 : south
    u32         degreesLat;//(0..8388607),
    s32         degreesLong;//(-8388608..8388607)
}sitGnssEllipsoidPointType;

typedef PACKED_STRUCT sitGnssEllipsoidPointWithUncertaintyCircleType_t
{
    u8             latSign;//0x00 : north 0x01 : south
    u32             degreesLat;
    s32             degreesLong;
    u8             uncertainty; //(0..100)
}sitGnssEllipsoidPointWithUncertaintyCircleType;

typedef PACKED_STRUCT sitGnssEllipsoidPointWithUncertaintyEllipseType_t
{
    u8             latSign;
    u32             degreesLat;
    s32             degreesLong;
    u8             uncertainty; //(0..127)
    u8             uncertaintySemiMaj; //(0..127)
    u8             uncertaintySemiMin; //(0..127)
    u8             orientationMajorAxis; //(0..179)
    u8             confidence; //(0..100)
}sitGnssEllipsoidPointWithUncertaintyEllipseType;

typedef PACKED_STRUCT sitGnssPolygonType_t
{
    u8     latSign;
    u32 degreesLat;
    s32 degreesLong;

}sitGnssPolygonType;

typedef PACKED_STRUCT sitGnssEllipsoidPointWithAltitudeType_t
{
    u8         latSign;
    u32         degreesLat;
    s32         degreesLong;
    u8         altiDirection;//0x00 : heigtht 0x01: depth
    u16         alti;    //(0..32767)
}sitGnssEllipsoidPointWithAltitudeType;

typedef PACKED_STRUCT sitGnssEllipsoidPointWithAltitudeAndUncertaintyEllipsoidType_t
{
    u8         latSign;
    u32         degreesLat;
    s32         degreesLong;
    u8         altiDirection;
    u16         alti;
    u8         uncertaintySemiMajor;
    u8         uncertaintySemiMinor;
    u8         orientationMajorAxis;
    u8         uncertaintyAltitude;
    u8         confidence;
}sitGnssEllipsoidPointWithAltitudeAndUncertaintyEllipsoidType;

typedef PACKED_STRUCT sitGnssEllipsoidArcType_t
{
    u8             latSign;
    u32             degreesLat;
    s32             degreesLong;
    u16             innerRadius; //(0..65535)
    u8             uncertaintyRadius; //(0..127)
    u8             offsetAngle; //(0..179)
    u8             includedAngle; //(0..179)
    u8             confidence;
}sitGnssEllipsoidArcType;

typedef PACKED_STRUCT sitGnssLocationCoordinatesType_t
{
    u8                              locCoordinateType;//sitGnsslocCoordinateType
    PACKED_UNION {
    sitGnssEllipsoidPointType          ellipsoidPoint;
    sitGnssEllipsoidPointWithUncertaintyCircleType         ellipsoidPointWithUncertaintyCircle;
    sitGnssEllipsoidPointWithUncertaintyEllipseType     ellipsoidPointWithUncertaintyEllipse;
    sitGnssPolygonType                             polygon;
    sitGnssEllipsoidPointWithAltitudeType             ellipsoidPointWithAltitude;
    sitGnssEllipsoidPointWithAltitudeAndUncertaintyEllipsoidType     ellipsoidPointWithAltitudeAndUncertaintyEllipsoid;
    sitGnssEllipsoidArcType ellipsoidArc;
    }u;
}sitGnssLocationCoordinatesType;

typedef PACKED_STRUCT sitGnssHorizontalVelocityType_t
{
    u16 bearing;//(0..359)
    u16 horiSpeed; //(0..2047)
}sitGnssHorizontalVelocityType;

typedef PACKED_STRUCT sitGnssHorizontalWithVerticalVelocityType_t
{
    u16 bearing;//(0..359)
    u16 horiSpeed; //(0..2047)
    u8 vertiDirection; //0x00 : upward 0x01 : downward
    u8 vertiSpeed;//(0..255)
}sitGnssHorizontalWithVerticalVelocityType;

typedef PACKED_STRUCT sitGnssHorizontalVelocityWithUncertaintyType_t
{
    u16 bearing;//(0..359)
    u16 horiSpeed; //(0..2047)
    u8 uncertaintySpeed;//(0..255)
}sitGnssHorizontalVelocityWithUncertaintyType;

typedef PACKED_STRUCT sitGnssHorizontalWithVerticalVelocityAndUncertainty_t
{
    u16 bearing;//(0..359)
    u16 horiSpeed; //(0..2047)
    u8 vertiDirection; //0x00 : upward 0x01 : downward
    u8 vertiSpeed;//(0..255)
    u8 horiUncertaintySpeed;
    u8 vertiUncertaintySpeed;
}sitGnssHorizontalWithVerticalVelocityAndUncertainty;

typedef PACKED_STRUCT sitLocationError_t
{
    u8 locFailCause;//sitLocationErrorType
}sitLocationError;

typedef PACKED_STRUCT sitGnssVelocityType_t
{
    u8                     velocityType;
    /*0x01 : HorizontalVelocity
    0x02 : HorizontalWithVerticalVelocity
         0x04 : HorizontalVelocityWithUncertainty
          0x08 : HorizontalWithVerticalVelocityAndUncertainty
          */
    PACKED_UNION {
    sitGnssHorizontalVelocityType                     horiVelo;
    sitGnssHorizontalWithVerticalVelocityType         horiWithVertiVelo;
    sitGnssHorizontalVelocityWithUncertaintyType         horiVeloWithUncertainty;
    sitGnssHorizontalWithVerticalVelocityAndUncertainty     horiWithVertiVeloAndUncertainty;
    }u;
}sitGnssVelocityType;

typedef PACKED_STRUCT sitAGnssProvideData_t
{
    u8                            assistsType;//sitGnssAssistDataType
    /*
    0x00 : NOT_USED.
    0x01 : CMN_ASSIST_DATA
    0x02 : GPS_GENERIC_ASSIST_DATA
    0x04 : GLONASS_GENERIC_ASSIST_DATA
    0x08 : MISC_GENERIC_ASSIST_DATA
    */
    PACKED_UNION{
        sitGnssCommonAssistData         cmnAssistData;
        sitGnssGenericAssistData         gpsGenericAssistData;
        sitGnssGenericAssistData        glonassGenericAssistData;
        sitGnssGenericAssistData        miscGenericAssistData;
    }u;
}sitAGnssProvideAssistData;

typedef PACKED_STRUCT sitGnssCmnIEProvLocInfo_t
{
    u8 bitMask;
    /*
    0x01 : BM_LOCATION_ESTIMATE
    0x02 : BM_VELOCITY_ESTIMATE
    0x04 : BM_LOCATION_ERROR
    */
    sitGnssLocationCoordinatesType         locationEstimate;
    sitGnssVelocityType                     velocityEstimate;
    sitLocationError                         locationError;
}sitGnssCmnIEProvLocInfo;

typedef PACKED_STRUCT sitGnssProvLocInfoType_t
{
    u8                                bitMask;//sitGnssProvLocInfoBitmask
    sitGnssSignalMeasInfoType         signalMeasurementInformation;
    sitGnssLocationInfoType         locationInformation;
}sitGnssProvLocInfoType;

typedef PACKED_STRUCT sitGnssVertiAccuracyType_t
{
    u8 accuracy;
    u8 confidence;
}sitGnssVertiAccuracyType;

typedef PACKED_STRUCT sitGnssRespTimeType_t
{
    u8     RespTime;//(1-128)
}sitGnssRespTimeType;

typedef PACKED_UNION sitGnssVelocityTypes_t
{
    u8 horizontalVelocity;
    u8 horizontalWithVerticalVelocity;
    u8 horizontalVelocityWithUncertainty;
       u8 horizontalWithVerticalVelocityAndUncertainty;
}sitGnssVelocityTypes;

typedef PACKED_STRUCT sitLocGnssInfoType_t
{
    u8 LocationInformationType;//sitLocInfoType
}sitGnssLocInfoType;

typedef PACKED_STRUCT sitRepDuration_t
{
    u8         reportingDuration;//(0-255)
}sitRepDuration;

typedef PACKED_STRUCT sitGnssHoriAccuracyType_t{
    u8    accuracy;
    u8    confidence;
}sitGnssHoriAccuracyType;

typedef PACKED_STRUCT sitGnssEnvironmentType_t
{
    u8        Environment;
    /*
    0x01: badArea,
    0x02: notBadArea,
    0x03: mixedArea
    */
}sitGnssEnvironmentType;

typedef PACKED_STRUCT sitGnssQosType_t
{
    u8                             bitMask;//sitBmQosTypeBitmask
    sitGnssHoriAccuracyType         horizontalAccuracy;
    u8                             verticalCoordinateRequest;//0x00 : false  0x01 : true
    sitGnssVertiAccuracyType         verticalAccuracy;
    sitGnssRespTimeType             responseTime;
    u8                             velocityRequest;//0x00 : false 0x01 : true
}sitGnssQosType;

typedef PACKED_STRUCT sitGnssTriggeredRepCriteria_t
{
    u8                 cellChange;//0x00 : false 0x01 : true
    sitRepDuration     reportingDuration;

}sitGnssTriggeredRepCriteria;

typedef PACKED_STRUCT sitGnssPeriodicRepCriteria_t
{
     u8         reportingAmount;//sitRepAmount
     u8         reportingInterval;//sitRepInterval

}sitGnssPeriodicRepCriteria;

typedef PACKED_STRUCT sitGnssCmnIEReqLocInfo_t
{
    u8                             bitmask;//sitCmnIeReqLocInfoType
    sitGnssLocInfoType             locInfoType;
    sitGnssTriggeredRepCriteria     TriggeredReporting;
    sitGnssPeriodicRepCriteria         PeriodicalReporting;
    u8                             AdditionalInfo;//sitGnssAddInfoType
    sitGnssQosType                 GnssQosType;
    u8                             environment;//sitEnvType
    sitLocationCoordinateTypes         locationCoordinateTypes;
    u8                             velocityType;
}sitGnssCmnIEReqLocInfo;

typedef PACKED_STRUCT sitGnssPosInstType_t
{
    sitGnssIdBitmap     gnssMethods;
    u8                 ftaMeasReq;//0x00 : False  0x01 : True  Indicates whether the target device is requested to report GNSS-network time association or not
    u8                 adrMeasReq;//0x00 : False 0x01 : True  indicates whether the target device is requested to include ADR measurements in GNSS-MeasurementList IE or not
    u8                 multiFreqMeasReq;//0x00 : False 0x01 : Indicates whether the target device is requested to report measurements on multiple supported GNSS signal types in GNSS-MeasurementList IE or not.
    u8                 assistanceAvailability;//0x00 : False 0x01 : True Indicates whether the target device may request additional GNSS assistance data from the server or not
}sitGnssPosInstType;

typedef  PACKED_STRUCT sitGnssReqLocInfoType_t
{
    sitGnssPosInstType     positioningInstructions;
}sitGnssReqLocInfoType;


/*=================================================================================

   SUB_CMD(1) :  0x49 AGNSS REFERENCE LOCATION INFORMATION NTF

==================================================================================*/

//Notification
typedef PACKED_STRUCT
{
    u8    TAC[2];
    u16 PSC;
    u32    CID;
}sitGnssRefLocInfoNoti;

/*=================================================================================

   SUB_CMD(1) :       0x51 AGNSS Request AGNSS Capability

==================================================================================*/

//Indication
typedef PACKED_STRUCT sitGnssCapabilityInd_t
{
    sitLppHeader    lpp_hdr;
    u8            cmn;    //0x00 : Not Supported, 0x01 : Supported
    u8            agnss;    //0x00 : Not Supported, 0x01 : Supported
    u8            epdu_id;  //0x01 = OMA LPP extensions
    u16           size_of_epdu; //max : 1024
    u8            epdu[1024];   //EPDU data
} sitGnssCapabilityInd;

//CNF
typedef PACKED_STRUCT sitGnssCapabilityMessageCnf_t
{
    sitLppHeader            lpp_hdr;
    u8                      flag;
    sitAGnssCapability      agnss;
    u8                      epdu_id;    //0x01 = OMA LPP extensions
    u16                     size_of_epdu; //max : 1024
    u8                      epdu[1024]; //EPDU data
}sitGnssCapabilityMessageCnf;


/*=================================================================================

   SUB_CMD(1) :      0x52 AGNSS Provide AGNSS Capability Info

==================================================================================*/
//SET
typedef PACKED_STRUCT  sitGnssProvideCapabilityMsgSet_t
{
    sitLppHeader                lpp_hdr;
    u8                                        flag;
    sitAGnssCapability        agnss;
}sitGnssProvideCapabilityMsgSet;

/*=================================================================================

   SUB_CMD(1) :       0x53 Request AGNSS assist data

==================================================================================*/
//SET
typedef PACKED_STRUCT sitGnssAssistDataMsgSetReq_t
{
    sitLppHeader            lpp_hdr;
    u8                      flag;   //sitGnssAssistDataFlag
    sitAGnssDataReq         agnss;
    u8                      epdu_id;    //0x01 = OMA LPP extensions
    u16                     size_of_epdu; //max : 1024
    u8                      epdu[1024]; //EPDU data
}sitGnssAssistDataMsgSetReq;

//Indication
typedef PACKED_STRUCT sitGnssAssistDataMsgInd_t
{
    sitLppHeader                    lpp_hdr;
    u8                              flag; //sitGnssAssistDataFlag
    sitAGnssProvideAssistData       agnss;
    u8                              epdu_id;    //0x01 = OMA LPP extensions
    u16                             size_of_epdu; //max : 1024
    u8                              epdu[1024]; //EPDU data
}sitGnssAssistDataMsgInd;

/*=================================================================================

   SUB_CMD(1) :       0x54 AGNSS Provide Assist Data info

==================================================================================*/
//Ind
typedef PACKED_STRUCT sitGnssProvideAssistDataMsgInd_t
{
    sitLppHeader                    lpp_hdr;
    u8                              flag; //sitGnssAssistDataFlag
    sitAGnssProvideAssistData       agnss;
    u8                              epdu_id;    //0x01 = OMA LPP extensions
    u16                             size_of_epdu; //max : 1024
    u8                              epdu[1024]; //EPDU data
}sitGnssProvideAssistDataMsgInd;

/*=================================================================================

   SUB_CMD(1) :     0x55 AGNSS Request Location Information Message

==================================================================================*/
/*Indication*/
 typedef PACKED_STRUCT sitGnssReqLocInfoMsgInd_t
 {
    sitLppHeader                    lpp_hdr;
    u8                              flag;//sitLppReqLocInfoIndType
    sitGnssCmnIEReqLocInfo          CMN;
    sitGnssReqLocInfoType           A_GNSS;
    u8                              epdu_id;    //0x01 = OMA LPP extensions
    u16                             size_of_epdu; //max : 1024
    u8                              epdu[1024]; //EPDU data
}sitGnssReqLocInfoMsgInd;

/*CNF*/
 typedef PACKED_STRUCT sitGnssReqLocInfoMsgCnf_t
 {
    sitLppHeader                    lpp_hdr;
    u8                              flag;//sitLppReqLocInfoIndType
    sitGnssCmnIEProvLocInfo         CMN;
    sitGnssProvLocInfoType          aGNSS;
    u8                              epdu_id;    //0x01 = OMA LPP extensions
    u16                             size_of_epdu; //max : 1024
    u8                              epdu[1024]; //EPDU data
}sitGnssReqLocInfoMsgCnf;

/*=================================================================================

   SUB_CMD(1) :       0x56 AGNSS Provide Location Information Message

==================================================================================*/
/*Notification*/ //CMD TYPE :0X03
typedef PACKED_STRUCT sitGnssProvLocInfoMsgNoti_t
{
    sitLppHeader                    lpp_hdr;
    u8                              flag;//sitLppErrorSetFlagType
    sitGnssCmnIEProvLocInfo         CMN;
    sitGnssProvLocInfoType          aGNSS;
    u8                              epdu_id;    //0x01 = OMA LPP extensions
    u16                             size_of_epdu; //max : 1024
    u8                              epdu[1024]; //EPDU data
}sitGnssProvLocInfoMsgNoti;

/*=================================================================================

   SUB_CMD(1) :     0x57 AGNSS ERROR

==================================================================================*/

typedef PACKED_STRUCT
{
    sitLppHeader            lpp_hdr;
    u8                      flag;
    u8                      cause;
#define SIT_LPP_ERR_INVALIED_SESSION_ID             0x01
#define SIT_LPP_ERR_UNEXPECTED_MSG                  0x02
#define SIT_LPP_ERR_NOT_ALL_REQ_MEASURE_POSSIBLE    0x03
#define SIT_LPP_ERR_AUTH_NET_FAILURE                0x04
#define SIT_LPP_ERR_MEASURE_TIME_EXPIRED            0x05
#define SIT_LPP_ERR_NOT_ENOUGH_SV                   0x06
#define SIT_LPP_ERR_AA_DATA_MISSING                 0x07
    u8                      epdu_id;    //0x01 = OMA LPP extensions
    u16                     size_of_epdu; //max : 1024
    u8                      epdu[1024]; //EPDU data
}sitGnssLppErrorSet;

typedef PACKED_STRUCT
{
    sitLppHeader            lpp_hdr;
    u8                      flag;
    u8                      casue;
#define SIT_LPP_INVALIED_SESSION_ID                                             0x01
#define SIT_LPP_UNEXPECTED_MSG                                                  0x02
#define SIT_LPP_ASSIST_DATA_PARTLY_NO_SUPPORT_NO_AVAIL_SERVER                   0x03
#define SIT_LPP_TIME_EXPIRED                                                    0x04
#define SIT_LPP_POSITION_METHOD_FAILURE                                         0x05
#define SIT_LPP_NOT_ALL_REQ_MEASURE_POSSIBLE                                    0x06
#define SIT_LPP_SERVER_DISCONNECTED                                             0x07
#define SIT_LPP_UNDELIVERED_ASSIST_DATA_IS_NOT_SUPPORTED_BY_SERVER              0x08
#define SIT_LPP_UNDELIVERED_ASSIST_DATA_IS_SUPPORTED_BUT_AVAILABLE_BY_SERVER    0x09
    u8                      epdu_id;    //0x01 = OMA LPP extensions
    u16                     size_of_epdu; //max : 1024
    u8                      epdu[1024]; //EPDU data
}sitGnssLppErrorNoti;


/*=================================================================================

   SUB_CMD(1) :     0x64 CELL INFO MSG

==================================================================================*/
typedef PACKED_STRUCT sitCellInfoNoti_t
{
    u8                        flag;
// SIT_WCDMA_ECID_PROV_CAP            0x01
// SIT_WCDMA_ECID_PROV_LOC            0x02

    u8                        ecidSupported;
    u8                        cellInfoType;
// SIT_GSM_CELL_INFO        0x01
// SIT_WCDMA_CELL_INFO    0x02
    PACKED_UNION sitCellInfo_t
    {
        sitGsmCellInformation            gsmCellInfo;
        sitWcdmaCellInformation        wcdmaCellInfo;
    }sitCellInfo;
}sitCellInfoNoti;

typedef PACKED_STRUCT sitCellInfoSetReq_t
{
    u8                        flag;
 #define SIT_WCDMA_ECID_REQ_CAP            0x01
 #define SIT_WCDMA_ECID_REQ_LOC            0x02
    u8                        ResponseTime;//INTEGER(1..128)
}sitCellInfoSetReq;


/*=================================================================================

   SUB_CMD(1) :  0x63 ENHANCED LTE CELL INFO_MSG

==================================================================================*/

typedef PACKED_STRUCT sitCgiInfo_t
{
    u16 mcc;
    u16 mnc;
    u32 cellId;
    u8 Tac[2];
}sitCgiInfo;

//MRL_List : neighbourCell
typedef PACKED_STRUCT sitMrlEutraInfo_t
{
    u8 bitMask;
    u16 physCellId;
    sitCgiInfo tCgiInfo;
    u8 rsrp;
    u8 rsrq;
    u16 earfcn;
}sitMrlEutraInfo;

typedef PACKED_STRUCT sitEnahncedLTECellInfoNoti_t
{
   u8    Flag;
   u8 BitMask;
   u8 EcidSupported;
   sitCgiInfo tCgiInfo; //Serving Cell info
   u16 PhysCellId;
   u8 Rsrp;
   u8 Rsrq;
   u16 Ta;
   u16 Earfcn;
   sitMrlEutraInfo atMrlEutraInfo[SIT_MAX_MRL_LIST]; //neighbour Cell Info
   u8 sizeOfMRL;
}sitEnhancedLTECellInfoNoti;

typedef PACKED_STRUCT sitEnahancedLTECellInfoSetReq_t
{
    u8    Flag; //0x01 : LTE_ECID_REQ_CAP, 0x02 : LTE _ECID_REQ_LOC
    u8    ResponseTime;//INTEGER(1..128), If SUPL_ECID_REQ_LOC present, it has a value.
}sitEnahancedLTECellInfoSetReq;
#pragma pack()


/* CDMA */
#pragma pack(1)

#define MAX_SV_NUM     (32)
#define SIT_SATELLITE_ID_LENGTH                        16

/*-----------------------------------------------------------------------*/
/* Enumerations                          */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/*  Structure used in Main                  */
/*-----------------------------------------------------------------------*/

/*=================================================================================

   SIT_IND_3GPP_SEND_GANSS_ASSIT_DATA(RCM ID = 0x0C15)

==================================================================================*/
typedef struct
{
    u8      bitMask;
    /*
    0x01 : BM_GANSS_DAY
    0x02 : BM_GANSS_REF_TOD_UNCERTAINTY
    0x04 : BM_GANSS_REF_TIME_ID
    */
    u16     gnssDay;    //Integer(0..8191)
    u16     gnssTod;    //Integer(0..86399)
    u8      gnssTodUncertainity;    //Integer(0..127)
    u8      gnssTimeId; //Integer(0..7)
}sitGnssRefTimeInfo;

typedef struct
{
    u8      bitMask;
    /*0x01 : BM_GANSS_FRAME_DRIFT*/
    u16     bcchCarrier; //Integer(0..1023)
    u8      bsic;   //Integer(0..63)
    u32     frameNumber;    //Integer(0..2097151)
    u8      timeSlot;   //Integer(0..7)
    u8      bitNumber;  //Integer(0..156)
    s8      frameDrift; //Integer(-64..63)
}sitGnssTodGsmTimeAssoc;

typedef struct
{
    u64  CellFrame;
    u8  ChoiceMask; //0x01 : FDD_SCMB_CODE,0x02 : TDD_CELL_PRAM_ID
    u32 FddScremCode;//FDD primary scrembling code;
    u32 TddCellPAramID;//TDD cell Parameter ID
    u32 Sfn;
}sitUtranGnssRefTime;

typedef struct
{
    u32 gnssTod;
    u8  gnssTimeId;
}sitGnssRefTimeOnly;

typedef struct
{
    u8      bitMask;
    /*
    0x01 : BM_GANSS_TOD_GSM_TIME_ASSOC
    0x02 : BM_UTRAN_GANSS_REFERENCE_TIME
    */
    sitGnssRefTimeInfo         refTimeInfo;
    sitGnssTodGsmTimeAssoc     todGsmTimeAssoc;
    sitUtranGnssRefTime        utranGnssRefTime;
    sitGnssRefTimeOnly         refTimeOnly;
}sitGnssRefTime;

typedef struct
{
    u16     ai0; //Integer(0..4095)
    u16     ai1; //Integer(0..4095)
    u16     ai2; //Integer(0..4095)
}sitGnssIonoSphereModel;

typedef struct
{
    u8                  ionoStormFlag1;//INTEGER(0..1)
    u8                  ionoStormFlag2;//INTEGER(0..1)
    u8                  ionoStormFlag3;//INTEGER(0..1)
    u8                  ionoStormFlag4;//INTEGER(0..1)
    u8                  ionoStormFlag5;//INTEGER(0..1)
}sitGnssIonoStormFlags;

typedef struct
{
    u8 bitMask; //0x01 : BM_GANSS_IONOSTORM_FLAGS

    sitGnssIonoSphereModel ionoSphereModel;
    sitGnssIonoStormFlags      ionoStormFlags;
}sitGnssIonoModel;

typedef struct
{
    u8 dataId;
    sitIonoModel ionoModel;
}sitGnssAddIonoModel;

typedef struct sitGnssComAssistData_t
{
    u8                              bitMask;
    /*
    0x01 : BM_GANSS_REF_TIME
    0x02 : BM_GANSS_REF_LOCATION
    0x04 : BM_GANSS_IONO_MODEL
    0x08 : BM_GANSS_ADD_IONO_MODEL
    0x10 : BM_GANSS_EARTH_ORIENT_PARAM
    */
    sitGnssRefTime             refTime;
    sitRefLoc                  refLocation;
    sitGnssIonoModel           ionoModel;
    sitGnssAddIonoModel            addIonoModel;
    sitGnssProvideCommEarthOrientationParameters   earthOrientationParameters;
}sitGnssComAssistData;

typedef struct
{
    u8                                   bitMask;
    /*
      #define BM_GANSS_TA1                    0x01
      #define BM_GANSS_TA2                    0x02
      #define BM_GANSS_WEEK_NUMBER            0x04
    */
    u16                                  ganssTimeModelRefTime;
    s32                                   tA0;
    s32                                   tA1;
    s8                                    tA2;
    u8                                   gnssToid;
    u16                                 weekNumber;
} sitGnssTimeModel;

typedef struct sitWcdmaGnssNavClockModel_t
{
    u16 navToc;//   2   INTEGER(0..37799)
    s8  navaf2;//   1   INTEGER(-128..127)
    s16     navaf1;//   2   INTEGER(-32768..32767)
    s32 navaf0;//   3   INTEGER(-2097152..2097151)
    s16 navTgd;//   1   INTEGER(-128..127)
}sitWcdmaGnssNavClockModel;

typedef struct sitWcdmaGnssCNAVClockModel_t
{
    u8          bitMask;
    /*
      0x00 : NOT_USED.
      0x01 : BM_CNAV_ISCL1_CP
      0x02 : BM_CNAV_ISCL1_CD
      0x04 : BM_CNAV_ISCL1_CA
      0x08 : BM_CNAV_ISCL2_C
      0x10 : BM_CNAV_ISCL5_I5
      0x20 : BM_CNAV_ISCL5_Q5
    */
    u16         cnavToc;
    u16         cnavTop;
    s8          cnavURA0;//INTEGER(-16..15)
    s8          cnavURA1;//
    s8          cnavURA2;// 1   INTEGER(0..7)
    s16         cnavAf2;//  2   INTEGER(-512..511)
    s16         cnavAf1;//  3   INTEGER(-524288..524287)
    s32         cnavAf0;//  INTEGER(-33554432..33554431)
    s16         cnavTgd;//  2   INTEGER(-4096..4095)
    s16         cnavISCl1cp;//  2   INTEGER(-4096..4095)
    s16         cnavISCl1cd;//  2   INTEGER(-4096..4095)
    s16         cnavISCl1ca;//  2   INTEGER(-4096..4095)
    s16         cnavISCl2c;///  2   INTEGER(-4096..4095)
    s16         cnavISCl5i5;//  2   INTEGER(-4096..4095)
    s16         cnavISCl5q5;//  2   INTEGER(-4096..4095)
}sitWcdmaGnssCNAVClockModel;

typedef struct sitWcdmaGnssClockModel_t
{
    u8                      clockModelType;
    /*
      0x01 : STANDARD_CLOCK_MODEL_ELEM
      0x02 : NAV_CLOCK_MODEL
      0x03 : CNAV_CLOCK_MODEL
      0x04 : GLONASS_CLOCK_MODEL
    */
    PACKED_UNION {
        sitGnssStandardClockModelElement   stdClockModelElement[2];
        sitWcdmaGnssNavClockModel          navClockModel;
        sitWcdmaGnssCNAVClockModel         cnavClockModel;
        sitGLONASSClockModel       glonassClockModel;
    }u;
}sitWcdmaGnssClockModel;

typedef struct sitWcdmaGnssNavModelNavKeplerianSet
{
    u8  navURA  ;// 1   INTEGER(0..15)
    u8  navFitFlag;//   1   INTEGER(0..1)
    u16 navToe;//   2   INTEGER(0..37799)
    s32 navOmega;// 4   INTEGER(-2147483648..2147483647)
    s16 navDeltaN;//    2   INTEGER(-32768..32767)
    s32 navM0;//    4   INTEGER(-2147483648..2147483647)
    s32 navOmegaADot;// 3   INTEGER(-8388608..8388607)
    u32 navE;// 3   INTEGER(0..4294967295)
    s16 navIDot;//  2   INTEGER(-8192..8191)
    u32 navAPowerHalf;//    3   INTEGER(0..4294967295)
    s32 navI0;//    4   INTEGER(-2147483648..2147483647)
    s32 navOmegaA0;//   4   INTEGER(-2147483648..2147483647)
    s16 navCrs;//   2   INTEGER(-32768..32767)
    s16 navCis;//   2   INTEGER(-32768..32767)
    s16 navCus;//   2   INTEGER(-32768..32767)
    s16 navCrc;//   2   INTEGER(-32768..32767)
    s16 navCic;//   2   INTEGER(-32768..32767)
    s16 navCuc;//   2   INTEGER(-32768..32767)
}sitWcdmaGnssNavModelNavKeplerianSet;

typedef struct sitWcdmaGnssNavModelCNAVKeplerianSet_t
{
    u16 cnavTop;//  2   INTEGER(0..2015)
    s8  cnavURAindex;// 1   INTEGER(-16..15)
    s32 cnavDeltaA;//   4   INTEGER(-33554432..33554431)
    s32 cnavAdot    ; //  //4   INTEGER(-16777216..16777215)
    s32 cnavDeltaNo  ;//    3   INTEGER(-65536..65535)
    s32 cnavDeltaNoDot;//   3   INTEGER(-4194304..4194303)
    s32 cnavMo;//   5   INTEGER(-4294967296..4294967295)
    u64 cnavE;//    5   INTEGER(0..8589934591)
    s64 cnavOmega;//    5   INTEGER(-4294967296..4294967295)
    s64 cnavOMEGA0;//   5   INTEGER(-4294967296..4294967295)
    s32 cnavDeltaOmegaDot;//    3   INTEGER(-65536..65535)
    s64 cnavIo;//   5   INTEGER(-4294967296..4294967295)
    s16 cnavIoDot;//    2   INTEGER(-16384..16383)
    s16 cnavCis;//  2   INTEGER(-32768..32767)
    s16 cnavCic;//  2   INTEGER(-32768..32767)
    s16 cnavCrs;//  3   INTEGER(-8388608..8388607)
    s16 cnavCrc;//  3   INTEGER(-8388608..8388607)
    s16 cnavCus;//  3   INTEGER(-1048576..1048575)
    s16 cnavCuc;//  3   INTEGER(-1048576..1048575)
}sitWcdmaGnssNavModelCNAVKeplerianSet;

typedef struct sitWcdmaGnssOrbitModel_t
{
    u8                          orbitModelType;
    //0x01 : NAV_MODEL_KEPLERIAN_SET
    //0x02 : NAV_MODEL_NAV_KEPLERIAN_SET
    //0x03 : NAV_MODEL_CNAV_KEPLERIAN_SET
    //0x04 : NAV_MODEL_GLONASS_ECEF
    PACKED_UNION{
        sitGnssNavModelKeplerianSet        keplerianSet;
        sitWcdmaGnssNavModelNavKeplerianSet    navKeplerianSet;
        sitWcdmaGnssNavModelCNAVKeplerianSet   cnavKeplerianSet;//
        sitGnssNavModelGLONASSECEF     glonassECEF;//
    }u;
}sitWcdmaGnssOrbitModel;

typedef struct sitGnssNavModelSatElement_t
{
    u8 bitMask; //0x01 : BM_SV_HEALTH_MSB,0x02 : BM_IOD_MSB
    u8                      svID;
    u8                      svHealth;//BIT STRING (SIZE(8))
    u16                     iod;//BIT STRING (SIZE(11))
    sitWcdmaGnssClockModel gnssClockModel;
    sitWcdmaGnssOrbitModel gnssOrbitModel;
    u8                      svHealthMsb;
    u8                      iodMsb;
}sitGnssNavModelSatElement;

typedef struct sitGnssNavModel_t
{
    u8                              nonBroadcastIndFlag;//INTEGER (0..1)
    sitGnssNavModelSatElement      gnssSatelliteList[16];
    u8                              sizeOfGnssSatelliteList;//INTEGER (1..2)
}sitGnssNavModel;

typedef struct sitAdditionalDopplerField_t
{
    u8  doppler1;   //Integer(0..63)
    u8  dopplerUncertainty; //Integer(0..4)
}sitAdditionalDopplerField;

typedef struct sitAdditionalAngleField_t
{
    u8  azimuth;    //Integer(0..31)
    u8  elevation;  //Integer(0..4)
}sitAdditionalAngleField;

typedef struct sitGnssRefMeasElem_t
{
    u8                      bitMask;
    u8                      svId;
    s16                     doppler0;
    sitAdditionalDopplerField  additionalDoppler;
    u16                     codePhase;
    u8                      intCodePhase;
    u8                      codePhaseSearchWindow;
    sitAdditionalAngleField    additionalAngle;
}sitGnssRefMeasElem;

typedef struct sitGnssRefMeasAssist_t
{
    u8                          signalID;
    sitGnssRefMeasElem         gnssRefMeasAssistList[16];
    u8                          sizeOfgnssRefMeasAssistList;
}sitGnssRefMeasAssist;

typedef struct sitGnssGenericAssistantData_t
{
    u16                         bitMask;
    u8                          gnss_ID;
    sitGnssTimeModel           gnssTimeModel;
    //sitGnssDiffCorrections       diffCorrections;
    sitGnssNavModel            navModel;
    sitGnssBadSignalElement    rti[8];
    u8                          sizeOfRti;
    sitGnssRefMeasAssist       gnssRefMeasAssist;
    sitUtcModel                utcModel;
}sitGnssGenericAssistantData;

typedef struct sitGnssDetailedAssistData_t
{
    u8      assistDataType;
    /*
    0x01 : GANSS_COMMON_ASSIST_DATA
    0x02 : GPS_GENERIC_ASSIST_DATA
    0x03 : GANSS_GENERIC_ASSIST_DATA
    */
    PACKED_UNION sitGnssAssistData_t
    {
        sitGnssComAssistData                   cmnAssistData;
        sitGpsAssDataNtf                       gpsGenericAssistData;
        sitGnssGenericAssistantData            glonassGenericAssistData;
    }u;
}sitGnssDetailedAssistData;

typedef struct sitAGnssAssistData_t
{
    u8  bitMask;
    /*
    0x00 : NOT_USED.
    0x01 : GANSS_ASSIST_DATA
    0x02 : GANSS_CARRIER_PHASE_MEAS_REQUEST
    0x04 : GGANSS_TOD_GSM_TIME_ASSOC_MEAS_REQUEST
    */
    sitGnssDetailedAssistData      gnssAssistData; //GANSS_ASSIST_DATA
    u8                              gnssCarrierPhaseMeasReq;    //GANSS_CARRIER_PHASE_MEAS_REQUEST
    u8                              gnssTodGsmTimeAssocMeasReq; //GANSS_TOD_GSM_TIME_ASSOC_MEAS_REQUEST
}sitAGnssAssistData;


/*=================================================================================

    SIT_IND_GANSS_MEAS_POS_MSG(RCM ID = 0x0C16)

==================================================================================*/
typedef struct sitAGnssMeasPosInd_t
{
    u8                  MethodType;
    sitAccType         Accuracy;
    u8                  RspTime;
    u8                  UseMultiSet;
    u8                  EnvChar;
    u8                  CellTimingWant;
    u8                  AddAssiReq;
    u8                  velocityRequested;
    u16                 gnssPositionMethod;
    u8                  ganssCarrierPhaseMeasReq;
    u8                  ganssTODGsmTimeAssociationMeasReq;
    u8                  ganssMultiFreqMeasReq;
} sitAGnssMeasPosInd;


/*=================================================================================

    SIT_SET_GANSS_MEAS_POS_RSP(RCM ID = 0x0C17)

==================================================================================*/
/* UTRAN GPS Reference Time Type */
typedef struct sitUtranGpsRefTime_t
{
    u8                        valid;
    u32                      cellFrames;
    u8                        choice_mode;
    u32                      UtranFdd;
    u32                      UtranTdd;
    u32                      sfn;
}sitUtranGpsRefTime;

typedef struct sitGpsMeasCnfType_t
{
    u32 RspType;
    /*
    0x00 : IPC_GPS_MSR_POS_RES_LOCATION,
    0x01 : IPC_GPS_MSR_POS_RES_GPS_MEASUREMENTS,
    0x02 : IPC_GPS_MSR_POS_RES_AID_REQ,
    0x03 : IPC_GPS_MSR_POS_RES_ERROR
    */
    sitGpsMeaType          GpsMeasure;
    sitMeasLocType         LocInfo;
    sitGpsMoAssData        MAssisData;
    sitUtranGpsRefTime     UtranGpsRefTime;
} sitGpsMeasCnfType;

typedef struct sitRefFrame_t
{
    u16                         referenceFN;//INTEGER(0..65535)
    u8                          referenceFNMSB;//INTEGER(0..63)
} sitRefFrame;

typedef struct sitGnssLocInfo_t
{
    u8 bitMask;
    /*
      0x01 BM_REFERENCE_FN
      0x02 BM_REFERENCE_FN_MSB
      0x04 BM_GANSS_TODM
      0x08 BM_GANSS_TOD_FRAC
      0x10 BM_GANSS_TOD_UNCERTAINTY
      0x20 BM_GANSS_TIME_ID
      0x40 STATIONARY INDICATION
    */
    sitRefFrame    refFrame;
    u32 gnssTodModule;
    u16 gnssTodFrac;
    u8  gnssTodUnc;
    u8  gnssTimeId;
    u8 fixType;
    u16 posData;
    u8 stationaryInd;
    sitLocInfo MLocInfo;
} sitGnssLocInfo;

typedef struct sitDataBitAssistExt_t
{
    u8 gnssTod; //Integer(0...59)
    u8 gnssSignal; //BITSTIRNG(SIZE(8))
    u16 gnssDataBitInterval; //Integer(1...1024)
    u8 numSat; //Integer(0...15)
    u8 gnssSatId[15]; //BITSTIRNG(SIZE(6))
    u8 sizeOfGnssSatId; //Integer(0?5)
} sitDataBitAssistExt;

typedef struct sitGnssEphExt_t
{
    u8 gnssEphExtBeginTod; //BITSTIRNG(SIZE(3))
    u8 gnssEphExtBeginDay; //BITSTIRNG(SIZE(5))
    u8 gnssEphExtEndTod; //BITSTIRNG(SIZE(5))
} sitGnssEphExt;

typedef struct sitGnssAddAssistData_t
{
    u8 ClockModelId;
    u8 OrbitModelId;
    u8 AlmanacModelId;
    u8 utcModelId;
} sitGnssAddAssistData;

typedef struct sitSatRelatedData_t
{
    u16 gnssWeek;
    u8 gnssToe;
    u8 nsat;
    u8 tToeLimit;
    u8 gnssSatIds[16];
    u8 sizeOfGnssSatId;
    u8 iod[16];
    u8 sizeOfIod;
} sitSatRelatedData;

typedef struct sitReqGnssAssistDataTypee_t
{
        u32 Flag;
        /*
          0x00000001 : GANSS_ASSIST_REF_TIME
          0x00000002 : GANSS_ASSIST_ REF_LOCATION
          0x00000004 : GANSS_ASSIST_IONO_MODEL
          0x00000008 : GANSS_ASSIST_ADDI_IONO_MODEL_ID_00
          0x00000010 : GANSS_ASSIST_ADDI_IONO_MODEL_ID_11
          0x00000020 : GANSS_ASSIST_EARTH_ORIENTATION
          0x00000040 : GANSS_ASSIST_REAL_TIME_INTEGRITY
          0x00000080 : GANSS_ASSIST_DIFFERENTIAL_CORRECTION
          0x00000100 : GANSS_ASSIST_ALMANAC
          0x00000200 : GANSS_ASSIST_REF_MEASUREMENT_INFO
          0x00000400 : GANSS_ASSIST_NAVIGATION_MODEL
          0x00000800 : GANSS_ASSIST_EPH_EXT
          0x00001000 : GANSS_ASSIST_TIME_MODEL_GNSS_UTC
          0x00002000 : GANSS_ASSIST_TIME_MODEL_GNSS_GNSS
          0x00004000 : GANSS_ASSIST_DATA_BIT_ASSIST
          0x00008000 : GANSS_ASSIST_EPH_EXT_CHECK
          0x00010000 : GANSS_ASSIST_ADD_ASSIST_DATA_CHOICES
          0x00020000 : GANSS_ASSIST_AUXTILIARY_INFO
        */
        u8  gnss_ID;
        u8  diffCorrectionsExt;
        u8  timeModelGnssExt;
        sitDataBitAssistExt  databitAssistExt;
        u8  validityTime;
        sitGnssEphExt  gnssEphExt;
        u8  sbasId;
        sitGnssAddAssistData   addAssistData;
        sitSatRelatedData      satRelatedData;
} sitReqGnssAssistDataType;

typedef struct
{
    u8                      bitMask;//sitGnssSatMeasElementTypeMask
    u8                      svID;
    u8                      cNo;//(0..63)
    u8                      mpathDet;//0x00 : notMeasured 0x01 : low 0x02 : medium 0x03 : high
    u8                      carrierQualityInd;//(0-3)
    u32                     codePhase;//(0..2097151)
    u8                      integerCodePhase;//(0..127)
    u8                      codePhaseRMSError;//(0-63)
    u16                     doppler;//(-32768..32767)
    u32                     adr;//(0..33554431)
}sitWcdmaGnssSatMeasElementType;

typedef struct sitWcdmaGnssSgnMeasElementType_t
{
    u8          bitmask; //0x00 : NOT_USED. 0x01 : BM_CODE_PHASE_AMBIGUITY
    u8          signalID;
    u8 codePhaseAmbiguity;//INTEGER(0..127)
    sitWcdmaGnssSatMeasElementType SatMeasList[SIT_SATELLITE_ID_LENGTH];
    u8 sizeOfMeasurementList;
}sitWcdmaGnssSgnMeasElementType;

typedef struct sitGnssMeasElem_t
{
    u8 bitMask;
    u8 gnssId;
    sitWcdmaGnssSgnMeasElementType gnssSigTypeList;
} sitGnssMeasElem;

typedef struct sitGnssMeasSetElem_t
{
    u8 bitMask;
    /*
      0x01 : BM_REFERENCE_FRAME
      0x02 : BM_GANSS_ELEM_TOD_M
      0x04 : BM_GANSS_ELEM_TOD_UNCERTAINTY
    */
    sitRefFrame    refFrame;
    u32 gnssTod;
    u8 deltaGnssTod;
    u8 gnssTodUnc;
    sitGnssMeasElem gnssMeasElemList;
} sitGnssMeasSetElem;

typedef struct sitGnssMeasInfoType_t
{
        sitGnssMeasSetElem gnssMeasElem;
} sitGnssMeasInfoType;

typedef struct sitGnssMeasCnfType_t
{
        u8 MeasRspType;
        /*
          0x01 : GANSS_ASSIST_INFO
          0x02 : GANSS_LOCATION_INFO
          0x04 : GANSS_MEASURE_INFO
        */
        u32 RspType;
        /*
          0x00 : IPC_GANSS_MSR_POS_RES_LOCATION,
          0x01 : IPC_GANSS_MSR_POS_RES_GANSS_MEASUREMENTS,
          0x02 : IPC_GANSS_MSR_POS_RES_AID_REQ,
          0x03 : IPC_GANSS_MSR_POS_RES_ERROR
        */
        PACKED_UNION {
            sitReqGnssAssistDataType   GnssAssistDataType;
            sitGnssLocInfo             GnssLocInfoType;
            sitGnssMeasInfoType        GnssMeasInfo;
        }u;
} sitGnssMeasCnfType;

typedef struct sitVelocityEstimateType_t
{
    u8      velocityType;
    /*
    0x00 : VELOCITY_NOT_SUPPORTED
    0x01 : HORIZ_VELOCITY
    0x02 : HORIZ_WITH_VERT_VELOCITY
    0x03 : HORIZ_VELOCITY_WITH_UNC
    0x04 : HORIZ_WITH_VERT_VELOCITY_AND_UNC
    */
    PACKED_UNION {
        sitGnssHorizontalVelocityType                  horiVelo;
        sitGnssHorizontalWithVerticalVelocityType      horiWithVertiVelo;
        sitGnssHorizontalVelocityWithUncertaintyType       horiVeloWithUncertainty;
        sitGnssHorizontalWithVerticalVelocityAndUncertainty    horiWithVertiVeloAndUncertainty;
    }u;
}sitVelocityEstimateType;

typedef struct sitGnssMeasPosCnf_t
{
    u8  Result; //0x00 Sucess 0x01 Fail
    u8  bitMask;
    /*
    0x01 : BM_GPS_MEAS_CFRM
    0x02 : BM_GANSS_MEAS_CFRM
    0x04 : BM_ VELOCITY_ESTIMATE
    */
    sitGpsMeasCnfType  GpsMeasConf;
    sitGnssMeasCnfType GnssMeasConf;
    sitVelocityEstimateType    VelocityEstimate;
} sitGnssMeasPosCnf;


/*=================================================================================

    SIT_IND_GANSS_AP_POS_CAP_REQ (RCM ID = 0x0C18)
    SIT_IND_GANSS_AP_POS_CAP_RSP (RCM ID = 0x0C19)

==================================================================================*/
typedef PACKED_STRUCT sitNetAssistedGnss_t
{
    u8 GnssId;
    /*
    0x00 : SBAS
    0x01 : MODERNIZED_GPS
    0x02 : QZSS
    0x03 : GLONASS
    0x04 : BDS
    */
    u8 SbasIds; //BITSTRING(SIZE(8))
    u8 GnssMode;
    /*
    0x00 : NETWORK_BASED
    0x01 : UE_BASED
    0x02 : BOTH
    0x03 : NONE
    */
    u8 GnssSignalId;
    u8 GnssSignalIds; //BITSTRING(SIZE(8))
    u8 GnssTimeCellFrameMsr;    /*0x00 : false, 0x01 : true*/
    u8 GnssCarrierPhaseMsr;     /*0x00 : false, 0x01 : true*/
    u8 NonNaitveAssistChoice;   /*0x00 : false, 0x01 : true*/
} sitNetAssistedGnss;

typedef PACKED_STRUCT sitGnssPosCapCnf_t
{
    u8 StandAloneLocalMethod; /*0x00 : false, 0x01 : true*/
    u8 NetworkAssistedGPS;
    /*
    0x00 : NETWORK_BASED
    0x01 : UE_BASED
    0x02 : BOTH
    0x03 : NONE
    */
    sitNetAssistedGnss sitNetAssistedGnssList[8];
    u8 sizeofGnssList;
    u8 GnssSupportInd;              /*0x00 : false, 0x01 : true*/
    u8 GPSTimeCellFrameMsr;         /*0x00 : false, 0x01 : true*/
    u8 ipdl;                        /*0x00 : false, 0x01 : true*/
    u8 UeAssistedGpsMsrValidity;    /*0x00 : false, 0x01 : true*/
} sitGnssPosCapCnf;

typedef PACKED_STRUCT sitGnssPosCapInd_t
{
    u8 Flag;
    /*
    0x00 Reserved
    0x01 GPS
    0x02 GANSS
    */
} sitGnssPosCapInd;


/*=================================================================================

    SIT_GET_GSM_EXT_INFO_MSG (RCM ID = 0x0C1A)

==================================================================================*/
typedef struct {
    u16 arfcn;  /* ARFCN of GSM network */
    u8 bsic;    /* Base station identity code */
    u8 rxlev;   /* RX signal level */
    u8 ta;      /* Timing Advance */
} sitGetGsmExtInfoMsgType;


/*=================================================================================

    SIT_GPS_CONTROL_PLANE_ENABLE (RCM ID = 0x0C1B)

==================================================================================*/
typedef struct {
    u8 enable;
    /*
    0x00:DISABLE
    0x01:ENABLE
    */
} sitGpsControlPlaneEnableType;


/*=================================================================================

    SIT_GNSS_LPP_PROFILE_SET (RCM ID = 0x0C1C)

==================================================================================*/
typedef struct {
    u8 bitmask;
    /*
    0x00 : Not Support(Disable)
    0x01 : Support A_GNSS
    0x02 : Support OTDOA
    0x04 : Support ECID
    0x08 : Conventional GPS
    */
} sitGnssLppProfileSetReqType;

typedef struct {
    u8 result;
    /*
    0x00 : No need to reset
    0x01 : Need to reset
    */
} sitGnssLppProfileSetRespType;


/*=================================================================================

    SIT_SET_GPS_LOCK_MODE(RCM ID = 0x0C20)

==================================================================================*/
typedef struct {
    u8 gpsLock;
} sitGpsLockModeType;


/*=================================================================================

    SIT_GET_REFERENCE_LOCATION(RCM ID = 0x0C21)

==================================================================================*/
typedef struct
{
    u8  ValidRefLoc;         /* 0: Not Valid, 1: Valid TimeZone only, 2: Valid Time Zone and BS location */
    u32 TimeZoneLat;         /* Ex)Default location is Kansas and (39.164253,-94.544503)*1000000 */
    u32 TimeZoneLong;        /* FYI, Time Zone Range is [-16h ~ +15.5h]*1000000  */
    u16 SID;                 /* System ID and Range [0..32767] */
    u16 NID;                 /* Network ID and and Range [0..65535] */
    u16 BaseID;              /* Base Station ID and Range [0..65535] */
    u32 BaseLat;             /* WGS84 Geodetic Latitude [degrees]*1000000 ,latitude from base last registered on */
    u32 BaseLong;            /* WGS84 Geodetic Longitude[degrees]*1000000 ,Longitude from base last registered on */
} sitGpsGetRefLocationType;


/*=================================================================================

    SIT_IND_CDMA_GPS_POWER_ON(RCM ID = 0x0C22)

==================================================================================*/
typedef struct
{
    u32 FixMode;               /* Unkown:0, MSA:1, MSB:2, MSS:3, and Control Plane:4 */
    u32 FixRateNumFixes;       /* A value of 1 means is interested in only one fix,  */
                               /* A value > 1,multiple fixes with some time in btw the attempts */
    u32 FixRateTimeBeFixes;    /* Time, in seconds, btw position fix attempts.       */
                               /* A default of 30 seconds is used.                   */
    u32 QoSHAccuracy;          /* Horizontal Accuracy, in meters                     */
    u32 QoSVAccuracy;          /* Vertical Accuracy, in meters                       */
    u32 QoSPerformance;        /* Performance response quality in terms of time, in seconds */
} sitIndCdmaGpsPowerOnType;


/*=================================================================================

    SIT_SET_PSEUDO_RANGE_MEASUREMENTS (RCM ID = 0x0C23)

==================================================================================*/
typedef struct
{
    u8  SVID;
    u8  SV_CN0;          /* Satellite C/N0.                 */
    u8  MultiPath_Ind;   /* Pseudorange Multipath Indicator */
    u8  PS_Range_RMS_ER; /* Pseudorange RMS Error           */
    u16 PS_Dopp;         /* Satellite Doppler               */
    u16 SV_Code_Ph_Wh;   /* Satellite code phase - whole chips */
    u16 SV_Code_Ph_Fr;   /* SV Code Phase Fractional Chips  */
} sitCdmaPseudoRangeMeasData;

typedef struct
{
    u32   Meas_TOW;      /* Measurement GPS Time of Week  */
    u8    Meas_TOW_Unc;  /* Meas GPS Time of Week Uncertainty */
    u8    Num_Meas;      /* Number of measurement 0-16    */
    sitCdmaPseudoRangeMeasData  MeasData[MAX_SV_NUM];
} sitCdmaPseudoRangeMeasType;


/*=================================================================================

    SIT_IND_CDMA_SEND_ACQUSITION_ASSIT_DATA(RCM ID = 0x0C24)

==================================================================================*/
#define SIT_CDMA_GPS_MAX_SAT    32

typedef struct
{
    u8 SVID;
    u8 Doppler1;
    u8 Dopp_Win;
    u8 SV_CodePh_int;
    u8 GPS_BitNum;
    u8 SV_CodePh_Win;
    u8 Azimuth;
    u8 Elevation;
    u16 SV_CodePh;
    u16 Doppler0;
} sitCdmaAcqAssistData;

typedef struct
{
    u32 AA_Ref_TOW;
/*=================================================================================

    SIT_IND_LPP_UPDATE_UE_LOC_INFO (RCM ID = 0x0C2B)

==================================================================================*/
    u8 AA_Num;
    u8 DopIncl;
    u8 AddDopIncl;
    u8 Code_ph_incl;
    u8 Az_El_incl;
    sitCdmaAcqAssistData AA_Data[SIT_CDMA_GPS_MAX_SAT];
} sitIndCdmaSendAcqusitionAssitDataType;


/*=================================================================================

    SIT_IND_CDMA_SESSION_CANCELLATION (RCM ID = 0x0C25)

==================================================================================*/
typedef struct
{
//  No data
} sitIndCdmaSessionCancellationType;


/*=================================================================================

    SIT_GET_CDMA_PRECISE_TIME_AIDING_INFO (RCM ID = 0x0C26)

==================================================================================*/
typedef struct
{
    u8 ValidRefTime;        /* Not Available: 0, Available: 1*/
    u8 rat;
    u16 WeekNum;            /* GPS Time of Week [seconds]     */
    u32 Absolute_RMS_Acc;   /* Absolute Pulse RMS Accuracy [microseconds] */
    u64 TOW;
} sitGetCdmaPreciseTimeAidingInfoType;


/*=================================================================================

    SIT_GET_GPS_CDMA_FREQ_AIDING (RCM ID = 0x0C27)

==================================================================================*/
typedef struct
{
    u32 FrequencyDataType;
    /* 00 = Should not be used
     * 01 = Absolute center frequency of the ECLK (Nominal Freq + delta)
     * 02 = Delta from the nominal frequency
     */
    u32 AccuracyDataType;
    /* 00 = Should not be used
     * 01 = In units of PPM
     * 02 = In units of Hz
     */
    u32 OS_time;        /* OS time [milliseconds] */
    s32 Cal_ppb;        /* Clock frequency calibration value [ppb] */
    u64 Cal_RMS_ppb;    /* Frequency calibration RMS [ppb]*/
    u64 Frequency;      /* In units of Hz. For example, 19.6608 MHz => (19.6608 * 1000000) */
    u64 Accuracy;
    /* 0.2 = Share TCXO
     * 0.5 = Do not share TCXO
     */
} sitGetGpsCdmaFreqAidingRespType;


/*=================================================================================

    SIT_IND_GPS_LOCK_MODE (RCM ID = 0x0C28)

==================================================================================*/
typedef struct
{
    u8 result;
} SitIndGpsLockModeType;


/*=================================================================================

    SIT_IND_GPS_START_MDT_LOC (RCM ID = 0x0C29)

==================================================================================*/
typedef struct
{
    u8 Enable;  /* 0x00 : end MDT, 0x01 : start MDT */
} sitIndGpsStartMdtLocType;


/*=================================================================================

    SIT_GPS_RETRIEVE_LOC_INFO (RCM ID = 0x0C2A)

==================================================================================*/
typedef struct
{
    sitGnssLocationCoordinatesType locationCoordinate;
    sitGnssHorizontalVelocityType horizonVelo;
    u32 gnssTODMsec;
} sitGpsRetrieveLocInfoType;


/*=================================================================================

    SIT_IND_LPP_UPDATE_UE_LOC_INFO (RCM ID = 0x0C2B)

==================================================================================*/
typedef struct
{
    sitGnssEllipsoidPointWithAltitudeType ellipsoidPointWithAltitude;
    sitGnssHorizontalVelocityType horiVelo;
    u32 gnssTODMsec;
} sitIndLppUpdateUeLocInfoType;


#pragma pack()


#endif  /* SIT_RIL_GPS_DEF_H */
