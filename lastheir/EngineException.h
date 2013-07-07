#pragma once
#include <exception>
#include <Windows.h>

class EngineException : public std::exception {
public:
    EngineException(LPCWSTR what);
    ~EngineException(void);
    LPCWSTR what();
private:
    LPCWSTR _what;
};

