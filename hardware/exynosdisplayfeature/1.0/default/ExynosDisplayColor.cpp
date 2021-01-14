#define LOG_TAG "ExynosDisplayColor-Hal"
#include "ExynosDisplayUtils.h"
#include "ExynosDisplayColor.h"

#include <utils/Log.h>
#include <cmath>

using std::string;
using std::vector;

int colorSetRgbGain(double r, double g, double b)
{
    int gc[195] = {0,};

    ALOGD("colorSetRgbGain(): r=%f, g=%f, b=%f", (r * 255.0f), (g * 255.0f), (b * 255.0f));

    for (int i = 0; i < 65; i++) {
        gc[i] = (int)round((double)(r * 255.0f) / 64 * i);
        gc[i + 65] = (int)round((double)(g * 255.0f) / 64 * i);
        gc[i + 130] = (int)round((double)(b * 255.0f) / 64 * i);
    }

    string stream;
    stream.clear();

    for (int i = 0; i < 195; i++) {
        stream.append(std::to_string(gc[i]));
        stream.append(",");
    }
    stream.pop_back();

    /*ALOGD("%s\n", stream.c_str());*/

    setGamma(stream.c_str());
    stream.clear();

    return 0;
}
