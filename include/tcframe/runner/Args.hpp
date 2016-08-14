#pragma once

#include <string>

#include "tcframe/util.hpp"

using std::string;

namespace tcframe {

struct Args {
    friend class ArgsParser;

public:
    enum class Command {
        GEN,
        SUBMIT
    };

private:
    Command command_;
    optional<string> solution_;
    optional<string> tcDir_;
    optional<int> timeLimit_;
    optional<int> memoryLimit_;
    optional<unsigned> seed_;

public:
    Command command() const {
        return command_;
    }

    const optional<string>& solution() const {
        return solution_;
    }

    const optional<string>& tcDir() const {
        return tcDir_;
    }

    const optional<int>& timeLimit() const {
        return timeLimit_;
    }

    const optional<int>& memoryLimit() const {
        return memoryLimit_;
    }

    const optional<unsigned>& seed() const {
        return seed_;
    }
};

}
