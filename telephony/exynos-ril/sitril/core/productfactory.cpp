/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

/*
 * sitrilfactory.cpp
 *
 *  Created on: 2014. 6. 14.
 */

#include "productfactory.h"
#include "iochannel.h"
#include "rilparser.h"
#include "servicemgr.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

class DefaultRilProductFactory : public ProductFactory {

public:
    DefaultRilProductFactory() {}
    virtual ~DefaultRilProductFactory() {}

    /**
     * interface
     */
public:
    // @overriding
    virtual RilParser *GetRequestParser(RilContext *pRilContext) {
        if (pRilContext == NULL) {
            return NULL;
        }
        return new RilParser();
    }

    // @overriding
    virtual ServiceMgr *GetServiceManager(RilContext *pRilContext) {
        if (pRilContext == NULL) {
            return NULL;
        }
        return new ServiceMgr(pRilContext);
    }

    // @overriding
    virtual IoChannel *GetIoChannel(RilContext *pRilContext, const char *name) {
        if (pRilContext == NULL) {
            return NULL;
        }
        return new IoChannel(name);
    }
};


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
/**
 * static
 */
static DefaultRilProductFactory defaultRilProductFactory;

ProductFactory *ProductFactory::GetDefaultProductFactory(RilContext *pRilContext)
{
    if (pRilContext == NULL) {
        return NULL;
    }
    return &defaultRilProductFactory;
}
