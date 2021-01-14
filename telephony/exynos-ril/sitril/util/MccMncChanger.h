#ifndef __MCCMNCCHANGERLOADER_H__
#define __MCCMNCCHANGERLOADER_H__

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <telephony/ril.h>
#include "sitdef.h"

class MccMncChanger {
private:
    bool getAttrValue(xmlNode* a_node, const xmlChar* attrName, char* store);
    bool parserMccMncEntries(xmlDocPtr doc, xmlNode* a_node, const char* pMccMnc, char* pChangedMccMnc);
    bool loadMccMncList(const char* mccmnc, char* changedMccMnc);
public:
    bool checkMccMncToChange(const char* mccmnc, char* changedMccMnc);
};
#endif
