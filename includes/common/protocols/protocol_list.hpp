#pragma once

// enum class Protocol
// {
//     RAW,
//     TCP,
//     UDP
// };

struct Protocols
{
    struct Transport
    {
        struct TCP {};
        struct UDP {};
    };

    struct Presentation
    {
        struct TLS {};
    };

    struct Application
    {
        struct MESSAGES {};
    };
};