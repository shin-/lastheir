#include "EngineException.h"


EngineException::EngineException(LPCWSTR what) {
    _what = what;
}

EngineException::~EngineException(void) {}

LPCWSTR EngineException::what() {
    return _what;
}
