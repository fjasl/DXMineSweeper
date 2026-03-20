#include "NetworkManager.h"

NetworkManager& NetworkManager::Instance() {
    static NetworkManager instance;
    return instance;
}

NetworkManager::NetworkManager() : m_socket(INVALID_SOCKET) {}

bool NetworkManager::Init() {
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
}

void NetworkManager::Cleanup() {
    Disconnect();
    WSACleanup();
}

bool NetworkManager::Connect(const wchar_t* ip, int port) {
    Disconnect();
    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_socket == INVALID_SOCKET) return false;

    char ipA[64];
    WideCharToMultiByte(CP_ACP, 0, ip, -1, ipA, sizeof(ipA), NULL, NULL);

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons((u_short)port);
    inet_pton(AF_INET, ipA, &addr.sin_addr);

    if (connect(m_socket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        if (WSAGetLastError() != WSAEWOULDBLOCK) {
            Disconnect();
            return false;
        }
    }

    u_long mode = 1; // 设为非阻塞
    ioctlsocket(m_socket, FIONBIO, &mode);
    return true;
}

void NetworkManager::Disconnect() {
    if (m_socket != INVALID_SOCKET) {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
    m_recvBuffer.clear();
}

bool NetworkManager::SendJson(const std::string& type, const std::string& payload) {
    if (m_socket == INVALID_SOCKET) return false;
    std::string msg = "{\"type\":\"" + type + "\",\"payload\":\"" + payload + "\"}\n";
    return send(m_socket, msg.c_str(), (int)msg.length(), 0) != SOCKET_ERROR;
}

bool NetworkManager::PollMessages(std::string& outType, std::string& outPayload) {
    if (m_socket == INVALID_SOCKET) return false;
    char buf[1024];
    int bytes = recv(m_socket, buf, sizeof(buf) - 1, 0);
    if (bytes > 0) {
        buf[bytes] = '\0';
        m_recvBuffer += buf;
    }
    else if (bytes == 0) {
        Disconnect(); return false;
    }

    size_t pos = m_recvBuffer.find('\n');
    if (pos != std::string::npos) {
        std::string line = m_recvBuffer.substr(0, pos);
        m_recvBuffer.erase(0, pos + 1);

        auto Extract = [&](const std::string& key) -> std::string {
            std::string search = "\"" + key + "\":\"";
            size_t start = line.find(search);
            if (start == std::string::npos) return "";
            start += search.length();
            size_t end = line.find("\"", start);
            if (end == std::string::npos) return "";
            return line.substr(start, end - start);
            };
        outType = Extract("type");
        outPayload = Extract("payload");
        return true;
    }
    return false;
}
