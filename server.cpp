//
// Created by Ivan Marochkin on 12.03.2021.
//

#include <NSServer.h>


int main() {
    NSServerParams p{
        .numThreads = 4,
        .port = 3129,
        .maxConnections = 1024,
    };

    NSServer s(p);
    s.start();

    return 0;
}