//
// Created by neapu on 2025/12/11.
//

#include "Helper.h"
extern "C"{
#include <libavutil/error.h>
}

namespace codec {
std::string Helper::getFFmpegErrorString(int errNum)
{
    char errBuf[AV_ERROR_MAX_STRING_SIZE] = {0};
    av_strerror(errNum, errBuf, sizeof(errBuf));
    return std::string(errBuf);
}
} // namespace codec