
#include "server/server_client.hpp"

    
#ifdef ENABLE_TLS
ServerClient::ServerClient(ServerEndpoint& accepted_ep, const int socket, const InetAddress& addr_info, SSL* ssl)
    : InetAddress(addr_info), TcpSocket(socket, addr_info.getAddressFamily()), PacketManager(), _ssl_connection(ssl), _useTLS(ssl != nullptr), _accept_done(false), _endpoint(accepted_ep)
    {} 
#else
ServerClient::ServerClient(ServerEndpoint& accepted_ep, const int socket, const InetAddress& addr_info)
    : InetAddress(addr_info), TcpSocket(socket, addr_info.getAddressFamily()), _endpoint(accepted_ep)
    {} 
#endif

ServerClient::ServerClient(const ServerClient& copy)
: InetAddress(copy), TcpSocket(copy), _endpoint(copy._endpoint)
{
}

ServerEndpoint& ServerClient::getEndpoint()
{
    return (this->_endpoint);
}


#ifdef ENABLE_TLS
SSL    *ServerClient::getSSL() const
{
    return this->_ssl_connection;
}

bool    ServerClient::useTLS() const
{
    return this->_useTLS;
}

// todo 
std::string ServerClient::getCertificate()
{
    if (!this->_useTLS)
        return "";

    X509 *cert;
    cert = SSL_get_peer_certificate(this->_ssl_connection);
    if ( cert == NULL )
        return "No certificate";
    
    // char *line;
    // printf("Server certificates:\n");
    // line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
    // printf("Subject: %s\n", line);
    // free(line);
    // line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
    // printf("Issuer: %s\n", line);
    // free(line);
    // X509_free(cert);
    char *line;
    line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
    std::string certificate = std::string(line, std::strlen(line));
    free(line);
    X509_free(cert);
    return (certificate);
}

#endif