#pragma once

#include "common/protocols/protocol.hpp"

template<>
class ProtocolParser<Protocol::RAW> : public ProtocolParserBase
{
    public:
        ProtocolParserBase::EvalResult  eval(const char *data_strip, const size_t strip_size) override
        {
            (void)strip_size;
            (void)data_strip;
            return ProtocolParserBase::EvalResult::COMPLETE;
        }

        void*                           parse(const char *data_strip, const size_t strip_size) override
        {
            std::string *data = new std::string(data_strip, strip_size);
            return (reinterpret_cast<void*>(data));
        }
};

