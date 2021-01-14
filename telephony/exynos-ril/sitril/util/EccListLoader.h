#ifndef __ECCLISTLOADER_H__
#define __ECCLISTLOADER_H__

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <telephony/ril.h>
#include "callreqdata.h"
#include "sitdef.h"

#define EMERGENCY_CALL_ROUTING_NORMAL_URN     "urn:emergencynumber:routing:2"

class EccListLoader {
private:
    void getAttrValue(xmlNode* a_node, const xmlChar* attrName, char* store);
    void parser_ecc_entries(xmlDocPtr doc, xmlNode* a_node, const char* plmn, const char* mcc,
                            const char* mnc, EmcInfoList *emcInfoList);
    void reloadEccList(const char* plmn, EmcInfoList *emcInfoList);
    void reloadEccListByIccid(const char* iccId, EmcInfoList *emcInfoList);
public:
    void getEccList(const char *emergencyId, EmcInfoList *emergencyNumberList);
};
#endif
