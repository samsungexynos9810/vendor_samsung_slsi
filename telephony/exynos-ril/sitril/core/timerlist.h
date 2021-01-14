/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <sys/time.h>
#include <list>

using namespace std;

class Message;

 typedef struct {
     struct timeval timeout;
     Message *msg;
 } TimerEvent;

 // sorted list
 class TimerList {
 private:
    list<TimerEvent> mList;
    unsigned int mMaxTimeout;

 public:
    TimerList();

 public:
     TimerEvent front();
     TimerEvent pop();
     void put(Message *msg, unsigned int millis);
     void erase(Message *msg);
     int size() { return mList.size(); }
 public:
     static int compare(struct timeval &l, struct timeval &r);
     static void add(struct timeval &l, struct timeval &r);
     static void add(struct timeval& tv, unsigned int millis);
     static void sub(struct timeval &l, struct timeval &r);
     static struct timeval getNow();
     static struct timeval getNow(unsigned int elapse);
 };
