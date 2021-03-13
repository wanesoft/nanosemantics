//
// Created by Ivan Marochkin on 13.03.2021.
//

#include <NSClient.h>
#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>


NSClient::NSClient(NSClientParams &params) : _params(params) {

}

NSClient::~NSClient() {

}

int NSClient::start() {
    _generalSocket = create_socket_and_connect();
    if (_generalSocket < 0) {
        exit(EXIT_FAILURE);
    }

    int readFd = -1;
    if (_params.inputPath.empty()) {
        readFd = 0;
    } else {
        readFd = open(_params.inputPath.c_str(), O_RDONLY);
        if (readFd < 0) {
            std::cerr << "Failed to open file " << _params.inputPath << ": " << strerror(errno) << "\n";
            return -1;
        }
    }

    bool run = true;
    char buf[_params.bufSize];
    char recvBuf[_params.bufSize];
    std::vector<char> result;
    int gCheck = 0;

    while (run) {
        int readRes = read(readFd, buf, _params.bufSize);
        if (readRes < 0) {
            // std::cerr << "Error while reading: " << strerror(errno) << '\n';
        } else if (readRes == 0) {
            shutdown(_generalSocket, SHUT_WR);
            close(readFd);
            // assert(0);
        } else {
            int resSend = send(_generalSocket, buf, readRes, MSG_DONTWAIT);
            gCheck += resSend;
            if (resSend < 0) {
                std::cerr << "Error while sending: " << strerror(errno) << '\n';
            }
        }
        int resRecv = recv(_generalSocket, recvBuf, _params.bufSize, MSG_DONTWAIT);
        if (resRecv < 0) {
            if (errno == EAGAIN) {
                ;
            } else {
                run = false;
            }
            // std::cerr << "Error while receiving: " << strerror(errno) << '\n';
        } else if (resRecv == 0) {
            run = false;
        } else {
            result.reserve(resRecv + result.size());
            std::copy(recvBuf, recvBuf + resRecv, std::back_inserter(result));
        }
    }

    write(1, result.data(), result.size());
    write(1, "\n", 1);

    std::cerr << "i send " << gCheck << '\n';

    return 0;
}

int NSClient::create_socket_and_connect() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Failed to create socket: " << strerror(errno) << "\n";
        return -1;
    }

    struct sockaddr_in addr;
    socklen_t addrLen = sizeof(addr);
    memset(&addr, 0, addrLen);
    int port = atoi(_params.serverPort.c_str());
    if (port <= 0) {
        std::cerr << "Failed to parse server port: " << strerror(errno) << "\n";
        return -1;
    }
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    int resInetPton = inet_pton(AF_INET, _params.serverAddress.c_str(), &(addr.sin_addr));
    if (resInetPton <= 0) {
        std::cerr << "Failed to parse server address: " << strerror(errno) << "\n";
        return -1;
    }

    std::clog << "Trying to connect to " << _params.serverAddress << ':' << _params.serverPort << '\n';
    int resConnect = connect(sock, (struct sockaddr *)&addr, addrLen);
    if (resConnect < 0) {
        std::cerr << "Failed to connect: " << strerror(errno) << "\n";
        return -1;
    }

    return sock;
}
