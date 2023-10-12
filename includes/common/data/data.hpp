#pragma once

#include "common/protocols/protocol_list.hpp"
#include "common/packets/packet_parser_list.hpp"

// struct DataTag {};

// struct PacketsData : public DataTag
// {
//     struct  raw_tag {};
//     struct  messages_tag {};
// };

// struct ProtocolData : public DataTag
// {
//     struct  raw_tag {};
//     struct  tcp_tag {};
//     struct  udp_tag {};
// };

// template<class T>
// class Data
// {
// };


// template<Protocol P>
// class Data<P>
// {
// };

// template<>
// class Data<Protocol::TCP>
// {
// }

// template<>
// class Data<Protocol::UDP>
// {
// }


// template<Packets P>
// class Data<P>
// {
// };

// template<>
// class Data<Packets::RAW>
// {
// };

// template<>
// class Data<Packets::MESSAGES>
// {
// };


template<class ...T>
class Data : public T...
{
};



// class ProtocolTCPData
// {
//     public:
//         int a;
// };

// class PacketsRawData
// {
//     public:
//         int b;
// };


// template<Protocol P>
// class ProtocolData
// {
// };

// template<>
// class ProtocolData<Protocol::TCP> : public Data<ProtocolTCPData, PacketsRawData>
// {
//     public: 
//         ProtocolData()
//         {
//             this->a = 1;
//             this->b = 2;
//         }
// };



// class MyClientData
// {
//     public:
//         int d;
//         int e;
// };


// template<Protocol P, class D>
// class SClient : public Data<ProtocolData<P>, D>
// {
// };

// template<>
// class SClient<Protocol::TCP, MyClientData> : public Data<ProtocolData<Protocol::TCP>, MyClientData>
// {
//     public:
//         SClient()
//         {
//             this->a = 3;
//             this->b = 3;
//             this->d = 3;
//             this->e = 3;
//         }
// };

// ~

// template<class ...T>
// class ClientData : public T...
// {};



// template<class ...T>
// class CustomData : public ClientData< Data<PacketsData::messages_tag>, Data<PacketsData::raw_tag>, T... >
// {
//     CustomData()
//     {
//         this->a = 1;
//         this->b = 2;
//     }
// };
