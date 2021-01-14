#include "EccListLoader.h"
#include "rillog.h"
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>

#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_NET, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_NET, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_NET, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_NET, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

#define DEFAULT_ECC_FILE      "vendor/etc/database/EccTable_DEFAULT.xml"
#define ECC_FILE_LOCATION     "vendor/etc/database/"
#define URN_VALUE             "urn:emergencynumber:value:"
#define MCCMNC_VALUE          "MccmncValue"
#define MCCMNC_RETAIL         "#R"
#define ENTITY_NAME           "EccEntry"
#define ENTITY_ATTR_NUMBER    "Number"
#define ENTITY_ATTR_VALUE     "Value"
#define ENTITY_ATTR_CATEGORY  "Category"
#define ENTITY_ATTR_CONDITION "Condition"

inline bool exists (char *filename) {
  struct stat buffer;
  return (stat(filename, &buffer) == 0);
}

inline bool hasCondition(int conditions, int condition)
{
    return (conditions & condition) == condition;
}

void EccListLoader::getAttrValue(xmlNode* a_node, const xmlChar* attrName, char* store) {
    struct _xmlAttr* pAtt = a_node->properties;
    xmlChar *p_prop;
    while(pAtt != NULL) {
        if (!xmlStrcmp(pAtt->name, attrName)) {
            // RilLogI("EccListLoader.name: %s \n", pAtt-> name);
            // RilLogI("EccListLoader.value:%s\n", xmlGetProp(a_node, pAtt->name));
            p_prop = xmlGetProp(a_node, pAtt->name);
            strcpy(store, (char*)p_prop);
            xmlFree(p_prop);
            return;
        }
        pAtt = pAtt->next;
    }
}

void EccListLoader::parser_ecc_entries(xmlDocPtr doc, xmlNode* a_node, const char* plmn, const char* mcc,
                                       const char* mnc, EmcInfoList *emcInfoList)
{
    int countEccList = 0;
    xmlNode *cur_node = NULL;
    xmlNode *entry_node = NULL;
    cur_node = a_node->children;
    EmcInfo* emcInfoArray = emcInfoList->getEmcInfoList();
    xmlChar *p_plmn;
    xmlChar *p_mccmnc_retail;
    while(cur_node != NULL) {
        if(cur_node->type == XML_ELEMENT_NODE && (!xmlStrcmp(cur_node->name,(const xmlChar*)MCCMNC_VALUE))) {
            // RilLogI("EccListLoader.Element: %s\n", cur_node->name);
            // RilLogI("EccListLoader.cur_node->properties->name:%s\n", cur_node->properties->name);
            // RilLogI("EccListLoader.cur_node->properties->value:%s\n", xmlGetProp(cur_node, cur_node->properties->name));
            // RilLogI("mcc:%s, mnc:%s\n", mcc, mnc);
            p_plmn = xmlGetProp(cur_node, cur_node->properties->name);
            p_mccmnc_retail = xmlGetProp(cur_node, cur_node->properties->name);
            if (!xmlStrcmp(p_plmn, (const xmlChar*)plmn) ||
                !xmlStrcmp(p_mccmnc_retail, (const xmlChar*)MCCMNC_RETAIL)) {
                xmlFree(p_plmn);
                xmlFree(p_mccmnc_retail);

                entry_node = cur_node->children;
                while(entry_node != NULL) {
                    if (cur_node->type == XML_ELEMENT_NODE && (!xmlStrcmp(entry_node->name,(const xmlChar*)ENTITY_NAME))) {
                        char number[10] = {0,};
                        char value[10] = {0,};
                        char urn_value[MAX_URN_LEN] = {0,};
                        char cate[10] = {0,};
                        char con[10] = {0,};
                        getAttrValue(entry_node, (const xmlChar*)ENTITY_ATTR_NUMBER, number);
                        getAttrValue(entry_node, (const xmlChar*)ENTITY_ATTR_VALUE, value);
                        getAttrValue(entry_node, (const xmlChar*)ENTITY_ATTR_CATEGORY, cate);
                        getAttrValue(entry_node, (const xmlChar*)ENTITY_ATTR_CONDITION, con);
                        EmcInfo *emcInfo = &emcInfoArray[countEccList++];
                        if (strlen(value) > 0) {
                            snprintf(urn_value, strlen(URN_VALUE) + strlen(value) + 1,
                                     "%s%s", URN_VALUE, value);
                        }
                        emcInfo->update(mcc ,mnc, number, strlen(number), urn_value, strlen(urn_value),
                         atoi(cate), (int)strtol(con, NULL, 16), RIL_EMERGENCY_NUMBER_SOURCE_MODEM_CONFIG);
                        if (hasCondition((int)strtol(con, NULL, 16), SIT_EMERGENCY_CONDITION_UI_ONLY)) {
                            emcInfo->updateUrn(EMERGENCY_CALL_ROUTING_NORMAL_URN,
                                               strlen(EMERGENCY_CALL_ROUTING_NORMAL_URN));
                        }
                    }
                    entry_node = entry_node->next;
                }
                emcInfoList->m_count = countEccList;
                return;
            }
            xmlFree(p_plmn);
            xmlFree(p_mccmnc_retail);
        }
        cur_node = cur_node->next;
    }
    emcInfoList->m_count = 0;
}

void EccListLoader::reloadEccList(const char* plmn, EmcInfoList *emcInfoList) {
    char filename[256];
    char mcc[10] ={0,};
    char mnc[10] ={0,};

    strncpy(mcc, plmn, MAX_MCC_LEN);
    strncpy(mnc, plmn + MAX_MCC_LEN, MAX_MNC_LEN);
    if (mnc[MAX_MNC_LEN - 1] == '#') {
        mnc[MAX_MNC_LEN - 1] = 0;
    }

    sprintf(filename, ECC_FILE_LOCATION"EccTable_%s.xml", mcc);
    RilLogI("EccListLoader.filename: %s, mcc: %s, mnc: %s", filename, mcc, mnc);

    if (!exists(filename)) {
      sprintf(filename, DEFAULT_ECC_FILE);
      RilLogI("EccListLoader. use default ecc list filename: %s", filename);
    }

    xmlDoc *doc;
    xmlNode *root_element;

    doc = xmlReadFile(filename, NULL, 0 );
    if (doc == NULL) {
        RilLogI("EccListLoader.error: could not parse file %s\n", filename);
        return;
    }
    /*Get the root element node */
    root_element = xmlDocGetRootElement(doc);

    parser_ecc_entries(doc, root_element, plmn, mcc, mnc, emcInfoList);

    xmlFreeDoc(doc);
    // xmlCleanupParser();
}

void EccListLoader::reloadEccListByIccid(const char* iccId, EmcInfoList *emcInfoList) {
    char filename[256];

    sprintf(filename, ECC_FILE_LOCATION"EccTable_ICCID.xml");

    xmlDoc *doc;
    xmlNode *root_element;

    doc = xmlReadFile(filename, NULL, 0 );
    if (doc == NULL) {
        RilLogI("EccListLoader.error: could not parse file %s\n", filename);
        return;
    }
    /*Get the root element node */
    root_element = xmlDocGetRootElement(doc);

    RilLogI("EccListLoader.reloadEccListByIccid: iccId=%s", iccId);
    parser_ecc_entries(doc, root_element, iccId, iccId, iccId, emcInfoList);

    xmlFreeDoc(doc);
    // xmlCleanupParser();
}

void EccListLoader::getEccList(const char *emergencyId, EmcInfoList *emcInfoList) {
    if (emergencyId == NULL) {
        RilLogI("EccListLoader.getEccList: emergencyId is NULL");
        return;
    }

    int lenPrefixIccId = strlen("ICCID_");
    RilLogI("EccListLoader.getEccList: emergencyId=%s", emergencyId);
    if (strncmp(emergencyId, "ICCID_", lenPrefixIccId) == 0) {
        reloadEccListByIccid(emergencyId + lenPrefixIccId, emcInfoList);
    } else {
        reloadEccList(emergencyId, emcInfoList);
    }
}
