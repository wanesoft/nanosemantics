//
// Created by Ivan Marochkin on 12.03.2021.
//

#include <iostream>
#include <fstream>
#include <NSServer.h>
#include "json.hpp"

#define INIT_STR_FROM_CONFIG(_dst, _cfg, _cfg_name)      \
        do {                                             \
            if ((_cfg).count(_cfg_name)) {               \
                std::string _tmp = (_cfg).at(_cfg_name); \
                (_dst) = strdup(_tmp.c_str());           \
            } else {                                     \
                (_dst) = strdup("");                     \
            }                                            \
        } while (0)

#define INIT_FROM_CONFIG(_dst, _cfg, _cfg_name)                       \
        do {                                                          \
            if ((_cfg).count(_cfg_name)) {                            \
                (_dst) = (const decltype(_dst)&)(_cfg).at(_cfg_name); \
            }                                                         \
        } while (0)


int main(int ac, char **av) {
    if (ac < 2) {
        std::clog << "Using ./run_server [path_to_config] (usualy `server.conf`)\n";
        exit(EXIT_FAILURE);
    }

    using namespace nlohmann;
    std::ifstream is(av[1]);
    if (is.fail()) {
        std::clog << "Can't open configuration JSON file: " << av[1] << '\n';
        exit(EXIT_FAILURE);
    }
    json jsonConf = json::parse(is);

    NSServerParams p;

    std::vector<std::string> v = jsonConf.at("wordsForSearch");
    p.wordsForSearch = v;

    INIT_FROM_CONFIG(p.numThreads, jsonConf, "numThreads");
    INIT_FROM_CONFIG(p.port, jsonConf, "port");
    INIT_FROM_CONFIG(p.maxConnections, jsonConf, "maxConnections");
    INIT_FROM_CONFIG(p.bufSize, jsonConf, "bufSize");

    // todo add signal handler

    NSServer s(p);
    s.start();

    return 0;
}