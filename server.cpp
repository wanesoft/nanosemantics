//
// Created by Ivan Marochkin on 12.03.2021.
//

#include <iostream>
#include <fstream>
#include <csignal>
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

NSServer *gP = nullptr;

void signal_handler(int signal) {
    (void)signal;
    std::clog << "Signal " << signal << " call\n";
    gP->stop();
}


std::vector<uint8_t> get_last_word(std::vector<uint8_t> &vector, int resRecv) {
    if (vector.empty()) {
        return {};
    }

    auto counter = 0;
    auto it = (vector.begin() + resRecv - 2);
    for ( ; it != vector.begin() - 1; --it) {
        if (*it == ' ') {
            break;
        }
        ++counter;
    }

    if (counter == 0) {
        return {};
    }

    std::vector<uint8_t> ret(0, counter);
    it = (vector.begin() + resRecv - 2);
    for ( ; it != vector.begin() - 1; --it) {
        if (*it == ' ') {
            break;
        }
        if (*it == 0) {
            assert(0);
        }
        ret.emplace_back(*it);
    }

    std::reverse(ret.begin(), ret.end());
    return ret;
}

void printVec(std::vector<uint8_t> &vector) {
    for (auto &cur : vector) {
        std::clog << (char)cur;
    }
    std::clog << '\n';
}

int main(int ac, char **av) {

//    uint8_t buf[] = "chapter 1 chapter 2 c hapter 3 chapter 4 chapter 5 chapter 6 chapter 7 chapter 8 chapter 9 chapter 10 chapter 11 chapter 12 chapt";
//    std::vector<uint8_t> v(buf, buf + sizeof(buf));
//
//    auto res = get_last_word(v, v.size());
//    printVec(res);
//
//    return 0;

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

    signal(SIGINT, signal_handler);

    NSServer s(p);
    gP = &s;
    s.start();

    return 0;
}