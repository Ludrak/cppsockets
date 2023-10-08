#pragma once

#include "common/protocols/protocol.hpp"

template<>
class ProtocolParser<Protocol::RAW> : public ProtocolParserBase
{
    public:
        ProtocolParserBase::EvalResult  eval(const char *data_strip, const size_t strip_size) override;
        void*                           parse(const char *data_strip, const size_t strip_size) override;
};

