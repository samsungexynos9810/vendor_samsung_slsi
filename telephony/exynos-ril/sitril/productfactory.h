/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef    __PRODUCT_FACTORY_H__
#define    __PRODUCT_FACTORY_H__

#include "rildef.h"

class RilContext;
class RilParser;
class ServiceMgr;
class IoChannel;

class ProductFactory {
protected:

    /**
     * constructor
     */
protected:
    ProductFactory() {}
public:
    virtual ~ProductFactory() {}

    /**
     * interface
     */
public:
    virtual RilParser *GetRequestParser(RilContext *pRilContext)=0;
    virtual ServiceMgr *GetServiceManager(RilContext *pRilContext)=0;
    virtual IoChannel *GetIoChannel(RilContext *pRilContext, const char *name)=0;

public:
    static ProductFactory *GetDefaultProductFactory(RilContext *pRilContext);
};

#endif // __PRODUCT_FACTORY_H__
