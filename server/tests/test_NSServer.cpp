#include <gtest/gtest.h>
#include <NSServer.h>
#include <csignal>

NSServer *gS = nullptr;

void signal_handler(int signal) {
    (void)signal;
    gS->stop();
}

NSServerParams p{ .wordsForSearch = {"love"}, .numThreads = 4, .port = 3129, .maxConnections = 64, .bufSize = 8192 };

class NSServerTest : public NSServer, public ::testing::Test {
public:
    NSServerTest() : NSServer(p) {
    }

    void SetUp() override {
        std::signal(SIGUSR1, signal_handler);
        gS = this;
    }
    void TearDown() override {

    }
};

TEST_F(NSServerTest, simpleInitTest) {
    std::thread t([this]() {
        auto res = this->start();
        ASSERT_EQ(0, res);
    });
    std::raise(SIGUSR1);
    t.join();
}


TEST_F(NSServerTest, simpleNetworkTest) {
    std::thread t([this]() {
        auto res = this->start();
        ASSERT_EQ(0, res);
    });

    std::string expect = "love,1;love,6;love,11;";
    std::string prog = "cat t | nc 127.0.0.1 3129";
    std::string res;
    char tt[8192];
    memset(tt, 0, 8192);
    FILE *df = popen(prog.data(), "r");
    while (fscanf(df, "%s", (char *)&tt) > 0) {
        res += tt;
        memset(tt, 0, 8192);
    }
    fflush(df);
    pclose(df);

    ASSERT_EQ(res, expect);

    stop();
    // todo fix poll wakeup when `popen` using
    t.detach();
}

TEST_F(NSServerTest, simpleBookNetworkTest) {
    std::thread t([this]() {
        auto res = this->start();
        ASSERT_EQ(0, res);
    });

    std::string expect = "love,3158;love,9521;love,10570;love,30646;love,31552;love,32489;love,41774;love,41784;love,46205;love,69597;love,69987;love,70043;love,70083;love,90768;love,97556;love,183837;love,185107;love,201043;love,201411;love,202130;love,202163;love,206331;love,209449;love,211904;love,227303;love,230764;love,232263;love,232355;love,237903;love,237930;love,233842;love,240674;love,236803;love,237216;love,237285;love,242302;love,237507;love,243132;love,243257;love,253502;love,253863;love,254021;love,259133;love,297201;love,302788;love,304672;love,304700;love,306709;love,318073;love,326492;love,326536;love,331687;love,341765;love,352525;love,352589;love,383451;love,391282;love,406963;love,428624;love,438846;love,443922;love,446895;love,446907;love,468079;love,477066;love,477748;love,490768;love,491966;love,492678;love,520757;love,537506;love,544056;love,559225;love,574879;love,578404;love,581973;love,584167;love,621517;love,632997;love,633764;love,634023;love,634743;love,639199;love,641202;love,643425;love,644438;love,645821;love,647873;love,649683;love,655577;love,657911;";
    std::string prog = "cat u | nc 127.0.0.1 3129";
    std::string res;
    char tt[8192];
    memset(tt, 0, 8192);
    FILE *df = popen(prog.data(), "r");
    while (fscanf(df, "%s", (char *)&tt) > 0) {
        res += tt;
        memset(tt, 0, 8192);
    }
    fflush(df);
    pclose(df);

    std::clog << res << '\n';

    ASSERT_EQ(res, expect);

    using namespace std::chrono;
    std::this_thread::sleep_for(100ms);

    stop();
    // todo fix poll wakeup when `popen` using
    t.detach();
}