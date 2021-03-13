//
// Created by Ivan Marochkin on 12.03.2021.
//

#include <NSClient.h>
#include <iostream>


int main(int ac, char **av) {
    if (ac < 2) {
        std::cerr << "Usage: ./run_client [path_to_book]\n";
        exit(EXIT_FAILURE);
    }

    NSClientParams p{
        .inputPath = av[1],
        .outputPath = "./2.txt",
        .serverAddress = "127.0.0.1",
        .serverPort = "3129",
        .bufSize = 1024
    };

    NSClient c(p);

    c.start();

    return 0;
}