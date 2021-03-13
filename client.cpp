//
// Created by Ivan Marochkin on 12.03.2021.
//

#include <NSClient.h>


int main() {
    NSClientParams p{
        .inputPath = "./t",
        .outputPath = "./2.txt",
        .serverAddress = "127.0.0.1",
        .serverPort = "3130",
        .bufSize = 1024
    };

    NSClient c(p);

    c.start();

    return 0;
}