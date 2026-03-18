#pragma once
#define WIN32_LEAN_AND_MEAN  
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#pragma comment(lib, "Ws2_32.lib")

class NetworkManager {
public:
    static NetworkManager& Instance();

    bool Init();      
    void Cleanup();   
    bool Connect(const wchar_t* ip, int port); 
    void Disconnect();

    bool SendJson(const std::string& type, const std::string& payload); 
    bool PollMessages(std::string& outType, std::string& outPayload); 

private:
    NetworkManager();
    SOCKET m_socket;
    std::string m_recvBuffer;
};
