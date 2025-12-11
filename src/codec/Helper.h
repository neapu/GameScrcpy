//
// Created by neapu on 2025/12/11.
//

#pragma once
#include <string>

namespace codec {

class Helper {
public:
    static std::string getFFmpegErrorString(int errNum);
};

} // namespace codec
