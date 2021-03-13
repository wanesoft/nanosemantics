//
// Created by Ivan Marochkin on 13.03.2021.
//

#ifndef NSCLIENT_H
#define NSCLIENT_H

#include <string>


struct NSClientParams {
    std::string inputPath;
    std::string outputPath;
    std::string serverAddress;
    std::string serverPort;
    int bufSize;
};

class NSClient {
public:
    NSClient() = delete;
    explicit NSClient(NSClientParams &params);
    ~NSClient();

    int start();

private:
    int create_socket_and_connect();

    NSClientParams _params;
    int _generalSocket;
};


#endif //NSCLIENT_H
