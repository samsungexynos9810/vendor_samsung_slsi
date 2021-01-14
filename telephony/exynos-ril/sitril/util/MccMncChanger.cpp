#include "MccMncChanger.h"
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

#define XML_FILE_LOCATION     "vendor/etc/database/"
#define MCCMNC_VALUE          "MccMncValue"
#define ENTITY_NAME           "MccMncEntry"
#define ENTITY_ATTR_NAME      "MccMnc"

inline bool exists (char *filename) {
  struct stat buffer;
  return (stat(filename, &buffer) == 0);
}

bool MccMncChanger::getAttrValue(xmlNode* a_node, const xmlChar* attrName, char* store) {
    struct _xmlAttr* pAtt = a_node->properties;
    xmlChar *p_prop;
    while(pAtt != NULL) {
        if (!xmlStrcmp(pAtt->name, attrName)) {
            RilLogI("MccMncChanger.name: %s \n", pAtt-> name);
            RilLogI("MccMncChanger.value:%s\n", xmlGetProp(a_node, pAtt->name));
            p_prop = xmlGetProp(a_node, pAtt->name);
            strcpy(store, (char*)p_prop);
            xmlFree(p_prop);
                return true;
        }
        pAtt = pAtt->next;
    }
    return false;
}

bool MccMncChanger::parserMccMncEntries(xmlDocPtr doc, xmlNode* a_node, const char* pMccMnc,
        char* pChangedMccMnc)
{
    bool success = false;
    xmlNode *cur_node = NULL;
    xmlNode *entry_node = NULL;
    cur_node = a_node->children;
    while(cur_node != NULL) {
        if(cur_node->type == XML_ELEMENT_NODE
                && (!xmlStrcmp(cur_node->name,(const xmlChar*) MCCMNC_VALUE))) {
            RilLogI("MccMncChanger.Element: %s\n", cur_node->name);
            RilLogI("MccMncChanger.cur_node->properties->name:%s\n", cur_node->properties->name);
            RilLogI("MccMncChanger.cur_node->properties->value:%s\n",
                    xmlGetProp(cur_node, cur_node->properties->name));

            entry_node = cur_node->children;
            while(entry_node != NULL) {
                if (cur_node->type == XML_ELEMENT_NODE
                        && (!xmlStrcmp(entry_node->name,(const xmlChar*) ENTITY_NAME))) {
                    success = getAttrValue(entry_node, (const xmlChar*) ENTITY_ATTR_NAME,
                            pChangedMccMnc);
                    RilLogI("MccMncChanger changedMccMnc:%s\n", pChangedMccMnc);
                }
                entry_node = entry_node->next;
            }
            return success;
        }
        cur_node = cur_node->next;
    }
    return success;
}

bool MccMncChanger::loadMccMncList(const char* mccmnc, char* changedMccMnc) {
    char filename[256];
    bool success = false;

    sprintf(filename, XML_FILE_LOCATION"MccMncTable_%s.xml", mccmnc);
    RilLogI("MccMncChanger.filename: %s, mccmnc: %s", filename, mccmnc);

    if (!exists(filename)) {
        RilLogI("MccMncChanger. filename[%s] dosen't exist", filename);
        return success;
    }

    xmlDoc *doc;
    xmlNode *root_element;

    doc = xmlReadFile(filename, NULL, 0);
    if (doc == NULL) {
        RilLogI("MccMncChanger.error: could not parse file %s\n", filename);
        return success;
    }
    /*Get the root element node */
    root_element = xmlDocGetRootElement(doc);

    success = parserMccMncEntries(doc, root_element, mccmnc, changedMccMnc);

    xmlFreeDoc(doc);
    xmlCleanupParser();

    return success;
}

bool MccMncChanger::checkMccMncToChange(const char *mccmnc, char *changedMccMnc) {
    if (mccmnc == NULL) {
        RilLogI("MccMncChanger.getMccMncList: mccmnc is NULL");
        return false;
    }

    RilLogI("MccMncChanger.getMccMncList: mccmnc=%s", mccmnc);
    return loadMccMncList(mccmnc, changedMccMnc);
}
