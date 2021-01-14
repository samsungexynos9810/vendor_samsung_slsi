#ifndef VENDOR_SAMSUNG_SLSI_HARDWARE_EXYNOSDISPLAYFEATURE_V1_0_EXYNOSDISPLAYUTILS_H
#define VENDOR_SAMSUNG_SLSI_HARDWARE_EXYNOSDISPLAYFEATURE_V1_0_EXYNOSDISPLAYUTILS_H

#include <string>
#include <vector>

using std::string;
using std::vector;

int setCgc(const char* stream);
int setGamma(const char *stream);
int setHsc(const char *stream);
int parserXML(const char *docname, vector<string>& stream, const char *mode, const char *item);

#endif  // VENDOR_SAMSUNG_SLSI_HARDWARE_EXYNOSDISPLAYFEATURE_V1_0_EXYNOSDISPLAYUTILS_H