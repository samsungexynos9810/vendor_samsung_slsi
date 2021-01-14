/*
 * nvdef.h
 *
 *  Created on: 2015-3-4
 *      Author: yemz
 */

#ifndef NVDEFINE_H_
#define NVDEFINE_H_

#include <stddef.h>

typedef struct nv_item_id_tag{
    unsigned long nv_base;
    unsigned long nv_offset;
}nv_item_id_t;
typedef struct nv_item_define_tag{
    nv_item_id_t nv_id;
    unsigned long nv_length;
    char* nv_name;
} nv_item_t;

nv_item_t nv_item_table[] ={
    /*ZONE_IMEI*/
    {{0,0},            15,        "IMEI"},
    /*ZONE_BD_ADDR*/
    {{15,0},        6,        "BD_ADDR"},
    /*ZONE_WIFI_ADDR*/
    {{21,0},        6,        "WIFI_ADDR"},
    /*ZONE_MMI_RECORD*/
    {{27,0},        4,        "MMI_RECORD"},
    /*ZONE_TRACE_BASE*/
    //PCBA Info
    {{31,0},        12,        "REF_PCBA"},
    {{31,12},        15,        "SN"},  //SN include 6 nv items as follow.
    /*{{31,12},        4,        "SHORT_CODE"},
    {{31,16},        2,        "ICS"},
    {{31,18},        1,        "SITE_FAC_PCBA"},
    {{31,19},        1,        "LINE_FAC_PCBA"},
    {{31,20},        3,        "DATE_PROD_PCBA"},
    {{31,23},        4,        "SN_PCBA"},*/
    //Handset Info
    {{31,27},        12,        "INDUS_REF_HANDSET"},
    {{31,39},        2,        "INFO_PTM"},
    {{31,41},        1,        "SITE_FAC_HANDSET"},
    {{31,42},        1,        "LINE_FAC_HANDSET"},
    {{31,43},        3,        "DATE_PROD_HANDSET"},
    {{31,46},        4,        "SN_HANDSET"},
    //Mini Info
    {{31,50},        3,        "INFO_PTS_MINI"},
    {{31,53},        20,        "INFO_NAME_MINI"},
    {{31,73},        20,        "INFO_TECH_MINI"},
    //Goldgen Sample
    {{31,93},        1,        "INFO_GOLDGEN_FLAG"},
    {{31,94},        3,        "INFO_GOLDGEN_DATE"},
    //HDTB(Reworked PCBA download)
    {{31,97},        3,        "INFO_ID_BAIE_HDTB"},
    {{31,100},        3,        "INFO_DATE_PASS_HDTB"},
    //PT1 Test
    {{31,103},        3,        "INFO_PROD_BAIE_PARA_SYS"},
    {{31,106},        1,        "INFO_STATUS_PARA_SYS"},
    {{31,107},        1,        "INFO_NBRE_PASS_PARA_SYS"},
    {{31,108},        3,        "INFO_DATE_PASS_PARA_SYS"},
    //PT2 Test
    {{31,111},        3,        "INFO_PROD_BAIE_PARA_SYS_2"},
    {{31,114},        1,        "INFO_STATUS_PARA_SYS_2"},
    {{31,115},        1,        "INFO_NBRE_PASS_PARA_SYS_2"},
    {{31,116},        3,        "INFO_DATE_PASS_PARA_SYS_2"},
    //PT3 Test
    {{31,119},        3,        "INFO_PROD_BAIE_PARA_SYS_3"},
    {{31,122},        1,        "INFO_STATUS_PARA_SYS_3"},
    {{31,123},        1,        "INFO_NBRE_PASS_PARA_SYS_3"},
    {{31,124},        3,        "INFO_DATE_PASS_PARA_SYS_3"},
    //Bluetooth Test(PFT3)
    {{31,127},        3,        "INFO_PROD_BAIE_BW"},
    {{31,130},        1,        "INFO_STATUS_BW"},
    {{31,131},        1,        "INFO_NBRE_PASS_BW"},
    {{31,132},        3,        "INFO_DATE_BAIE_BW"},
    //WIFI Test
    {{31,135},        3,        "INFO_PROD_BAIE_GPS"},
    {{31,138},        1,        "INFO_STATUS_GPS"},
    {{31,139},        1,        "INFO_NBRE_PASS_GPS"},
    {{31,140},        3,        "INFO_DATE_BAIE_GPS"},
    //MMI Test
    {{31,143},        1,        "INFO_STATUS_MMI_TEST"},
    //Final Test(Antenna Test)
    {{31,144},        3,        "INFO_PROD_BAIE_FINAL"},
    {{31,147},        1,        "INFO_STATUS_FINAL"},
    {{31,148},        1,        "INFO_NBRE_PASS_FINAL"},
    {{31,149},        3,        "INFO_DATE_BAIE_FINAL"},
    //Final Test 2(3G Antenna Test)
    {{31,152},        3,        "INFO_PROD_BAIE_FINAL_2"},
    {{31,155},        1,        "INFO_STATUS_FINAL_2"},
    {{31,156},        1,        "INFO_NBRE_PASS_FINAL_2"},
    {{31,157},        3,        "INFO_DATE_BAIE_FINAL_2"},
    //HDT(CU perso download)
    {{31,160},        3,        "INFO_ID_BAIE_HDT"},
    {{31,163},        3,        "INFO_DATE_PASS_HDT"},
    //CU SW info
    {{31,166},        20,        "INFO_COMM_REF"},
    {{31,186},        3,        "INFO_PTS_APPLI"},
    {{31,189},        20,        "INFO_NAME_APPLI"},
    {{31,209},        20,        "INFO_NAME_PERSO_1"},
    {{31,229},        20,        "INFO_NAME_PERSO_2"},
    {{31,249},        20,        "INFO_NAME_PERSO_3"},
    {{31,269},        20,        "INFO_NAME_PERSO_4"},
    //PT4
    {{31,289},        3,        "INFO_PROD_BAIE_PARA_SYS_4"},
    {{31,292},        1,        "INFO_STATUS_PARA_SYS_4"},
    {{31,293},        1,        "INFO_NBRE_PASS_PARA_SYS_4"},
    {{31,294},        3,        "INFO_DATE_PASS_PARA_SYS_4"},
    {{31,297},        84,        "INFO_SPARE_REGION"},
    //P Sensor Calibration Param
    {{31,381},        40,        "PSENSOR"},
    //WiFi SSID
    {{31,421},        32,        "INFO_SSID"},
    //Password
    {{31,463},        15,        "INFO_Password"},
    //Tablet SN
    {{31,468},        15,        "INFO_Tablet_SN"},
    //PPE flag
    {{31,483},        1,        "PPE_ FLAG"},
    //KPH flag
    {{31,484},        1,        "KPH_FLAG"},
    {{31,485},        27,        "INFO_SPARE_REGION"},
    /*ZONE_EXTENZONE_BASE*/
    //CDMA2000 Info
    {{543,0},        16,        "AKEY"},
    {{543,16},        4,        "AKEYCHECKSUM"},
    {{543,20},        6,        "SPC"},
    {{543,26},        6,        "OTKSL"},
    //IMEI_2
    {{543,32},        15,        "IMEI_2"},
    //IMEI_3
    {{543,47},        15,        "IMEI_3"},
    //IMEI_4
    {{543,62},        15,        "IMEI_4"},
    //EVDO_PRI Parameter(HDT)
    {{543,77},        1,        "SYNC_FLAG"},
    {{543,78},        12,        "VAILD_MASK"},
    {{543,90},        9,        "MIN1"},
    {{543,99},        5,        "MIN2"},
    {{543,104},        3,        "MCC"},
    {{543,107},        2,        "IMSI_11_12"},
    {{543,109},        11,        "NUMBER_I"},
    {{543,120},        17,        "NUMBER_PCS"},
    {{543,137},        97,        "MIP"},
    {{543,234},        128,    "SIP"},
    {{543,362},        65,        "AN"},
    {{543,427},        48,        "INFO_SPARE_REGION"},

    {{0,0},            0,        NULL}
};

//*******************************************
//ZONE define
#define ZONE_IMEI                     0
#define ZONE_BD_ADDR                 15
#define ZONE_WIFI_ADDR                 21
#define ZONE_MMI_RECORD                27
#define ZONE_TRACE_BASE             31
#define ZONE_EXTENZONE_BASE         543

//*******************************************
//nv item address in trace zone
//*PCBA Info
#define REF_PCBA                    0
#define SHORT_CODE                    12
#define ICS                            16
#define SITE_FAC_PCBA                18
#define LINE_FAC_PCBA                19
#define DATE_PROD_PCBA                20
#define SN_PCBA                        23
//*Handset Info
#define INDUS_REF_HANDSET            27
#define INFO_PTM                    39
#define SITE_FAC_ HANDSET            41
#define LINE_FAC_ HANDSET            42
#define DATE_PROD_HANDSET            43
#define SN_HANDSET                    46
//*Mini Info
#define INFO_PTS_MINI                50
#define INFO_NAME_MINI                53
#define INFO_TECH_MINI                73
//*Golden Sample
#define INFO_GOLDEN_FLAG            93
#define INFO_GOLDEN_DATE            94
//*HDTB (Reworked PCBA download)
#define INFO_ID_BAIE_HDTB            97
#define INFO_DATE_PASS_HDTB            100
//*PT1 Test
#define INFO_PROD_BAIE_PARA_SYS        103
#define INFO_STATUS_PARA_SYS        106
#define INFO_NBRE_PASS_PARA_SYS        107
#define INFO_DATE_PASS_PARA_SYS        108
//*PT2 Test
#define INFO_PROD_BAIE_PARA_SYS_2    111
#define INFO_STATUS_PARA_SYS_2        114
#define INFO_NBRE_PASS_PARA_SYS_2    115
#define INFO_DATE_PASS_PARA_SYS_2    116
//*PT3 Test
#define INFO_PROD_BAIE_PARA_SYS_3    119
#define INFO_STATUS_PARA_SYS_3        122
#define INFO_NBRE_PASS_PARA_SYS_3    123
#define INFO_DATE_PASS_PARA_SYS_3    124
//*Bluetooth Test(PFT3)
#define INFO_PROD_BAIE_BW            127
#define INFO_STATUS_BW                130
#define INFO_NBRE_PASS_BW            131
#define INFO_DATE_BAIE_BW            132
//*WIFI Test
#define INFO_PROD_BAIE_GPS            135
#define INFO_STATUS_GPS                138
#define INFO_NBRE_PASS_GPS            139
#define INFO_DATE_BAIE_GPS            140
//*MMI Test
#define INFO_STATUS_MMI_TEST        143
//*Final Test (Antenna Test)
#define INFO_PROD_BAIE_FINAL        144
#define INFO_STATUS_FINAL            147
#define INFO_NBRE_PASS_FINAL        148
#define INFO_DATE_BAIE_FINAL        149
//*Final Test 2(3G Antenna Test)
#define INFO_PROD_BAIE_FINAL_2        152
#define INFO_STATUS_FINAL_2            155
#define INFO_NBRE_PASS_FINAL_2        156
#define INFO_DATE_BAIE_FINAL_2        157
//*HDT (CU perso download)
#define INFO_ID_BAIE_HDT            160
#define INFO_DATE_PASS_HDT            163
//*CU SW info
#define INFO_COMM_REF                166
#define INFO_PTS_APPLI                186
#define INFO_NAME_APPLI                189
#define INFO_NAME_PERSO1            209
#define INFO_NAME_PERSO2            229
#define INFO_NAME_PERSO3            249
#define INFO_NAME_PERSO4            269
//*PT4
#define INFO_PROD_BAIE_PARA_SYS_4    289
#define INFO_STATUS_PARA_SYS_4        292
#define INFO_NBRE_PASS_PARA_SYS_4    293
#define INFO_DATE_PASS_PARA_SYS_4    294
#define INFO_SPARE_REGION_1            297
//*P Sensor Calibration Param
#define PSENSOR                        381
//*WiFi SSID
#define INFO_SSID                    421
//*Password
#define INFO_Password                453
//*Tablet SN
#define INFO_Tablet_SN                468
//*PPE flag
#define PPE_ FLAG                    483
//*KPH flag
#define KPH_FLAG                    484
#define INFO_SPARE_REGION_2            485

//*******************************************
//nv item address in Extended Zone
//*CDMA2000 Info
#define AKEY                0
#define AKEYCHECKSUM        16
#define SPC                    20
#define OTKSL                26
//*IMEI 2
#define IMEI_2                32
//*IMEI 3
#define IMEI_3                47
//*IMEI 4
#define IMEI_4                62
//*EVDO_PRI Parameter(HDT)
#define SYNC_FLAG            77
#define VAILD_MASK            78
#define MIN1                90
#define MIN2                99
#define MCC                    104
#define IMSI_11_12            107
#define NUMBER_I            109
#define NUMBER_PCS            120
#define MIP                    137
#define SIP                    234
#define AN                    362
#define INFO_SPARE_REGION    427

#endif /* NVDEFINE_H_ */
