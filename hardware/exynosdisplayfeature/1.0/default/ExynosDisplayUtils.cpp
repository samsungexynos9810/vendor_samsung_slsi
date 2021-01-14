#define LOG_TAG "ExynosDisplayUtils-Hal"
#include "ExynosDisplayUtils.h"
#include <utils/Log.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

using std::string;
using std::vector;

#define SYSFS_CGC_PATH "/sys/class/dqe/dqe/cgc"
#define SYSFS_GAMMA_PATH "/sys/class/dqe/dqe/gamma"
#define SYSFS_HSC_PATH "/sys/class/dqe/dqe/hsc"

int setCgc(const char* stream)
{
    FILE* fp;

    /*ALOGD("setCgc() %s\n", stream);*/

    fp = fopen(SYSFS_CGC_PATH, "w");
    if (fp == NULL) {
        ALOGE("open file error: %s", SYSFS_CGC_PATH);
        return -1;
    }

    if (fputs(stream, fp) < 0) {
        ALOGE("cgc write error");
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}

int setGamma(const char *stream)
{
    FILE* fp;

    /*ALOGD("setGamma() %s\n", stream);*/

    fp = fopen(SYSFS_GAMMA_PATH, "w");
    if (fp == NULL) {
        ALOGE("open file error: %s", SYSFS_GAMMA_PATH);
        return -1;
    }

    if (fputs(stream, fp) < 0) {
        ALOGE("gamma write error");
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}

int setHsc(const char *stream)
{
    FILE* fp;

    /*ALOGD("setHsc() %s\n", stream);*/

    fp = fopen(SYSFS_HSC_PATH, "w");
    if (fp == NULL) {
        ALOGE("open file error: %s", SYSFS_HSC_PATH);
        return -1;
    }

    if (fputs(stream, fp) < 0) {
        ALOGE("hsc write error");
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}

int parserXML(const char *docname, vector<string>& stream, const char *mode, const char *item) {
    xmlDocPtr doc;
    xmlNodePtr cur, node;
    xmlChar *key;

    ALOGD("xml_path = %s, mode = %s, item = %s\n", docname, mode, item);

    doc = xmlParseFile(docname);
    if (doc == NULL) {
        ALOGE("can not parse document\n");
        return -1;
    }

    cur = xmlDocGetRootElement(doc);
    if (cur == NULL) {
        ALOGE("empty document\n");
        xmlFreeDoc(doc);
        return -1;
    }

    if (xmlStrcmp(cur->name, (const xmlChar *)"Calib_Data")) {
        ALOGE("document of the wrong type, root node != Calib_Data\n");
        xmlFreeDoc(doc);
        return -1;
    }

    cur = cur->xmlChildrenNode;

    while (cur != NULL) {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"mode"))) {

            xmlChar *mode_name;

            mode_name = xmlGetProp(cur, (const xmlChar *)"name");

            if ((!xmlStrcmp(mode_name, (const xmlChar *)mode))) {

                node = cur->xmlChildrenNode;

                if ((!xmlStrcmp((const xmlChar *)item, (const xmlChar *)"dqe")))
                {
                    while (node != NULL) {
                        if ((!xmlStrcmp(node->name, (const xmlChar *)"cgc"))) {
                            key = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
                            /*ALOGD("%s dqe cgc: %s\n", mode_name, key);*/
                            stream.push_back((char *)key);
                            xmlFree(key);
                        }

                        if ((!xmlStrcmp(node->name, (const xmlChar *)"gamma"))) {
                            key = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
                            /*ALOGD("%s dqe gamma: %s\n", mode_name, key);*/
                            stream.push_back((char *)key);
                            xmlFree(key);
                        }

                        if ((!xmlStrcmp(node->name, (const xmlChar *)"hsc"))) {
                            key = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
                            /*ALOGD("%s dqe hsc: %s\n", mode_name, key);*/
                            stream.push_back((char *)key);
                            xmlFree(key);
                        }
                        node = node->next;
                    }
                }
                else
                {
                    while (node != NULL) {
                        if ((!xmlStrcmp(node->name, (const xmlChar *)item))) {
                            key = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
                            /*ALOGD("%s %s: %s\n", mode_name, item, key);*/
                            stream.push_back((char *)key);
                            xmlFree(key);
                        }
                        node = node->next;
                    }
                }
            }
            xmlFree(mode_name);
        }
        cur = cur->next;
    }
    xmlFreeDoc(doc);

    if ((int)stream.size() == 0) {
        ALOGE("no data document\n");
        return -1;
    }

    return 0;
}