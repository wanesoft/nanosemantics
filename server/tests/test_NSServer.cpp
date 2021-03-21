#include <gtest/gtest.h>
#include <NSServer.h>
#include <csignal>

NSServer *gS = nullptr;

void signal_handler(int signal) {
    (void)signal;
    gS->stop();
}

NSServerParams p{ .wordsForSearch = {"love"}, .numThreads = 4, .port = 3129, .maxConnections = 64, .bufSize = 128 };

class NSServerTest : public NSServer, public ::testing::Test {
public:
    NSServerTest() : NSServer(p) {
    }

    static int countFounds(std::string &str) {
        int i = 0, res = 0;
        while (str[i]) {
            if (str[i++] == ';') {
                ++res;
            }
        }
        return res;
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

    std::string expect = "love,3158;love,9521;love,10570;love,30646;love,31552;love,32489;love,41774;love,41784;love,46205;love,69597;love,69987;love,70043;love,70083;love,90768;love,97556;love,183837;love,185107;love,201043;love,201411;love,202130;love,202163;love,206331;love,209449;love,211904;love,227303;love,230764;love,232263;love,232355;love,233842;love,237903;love,237930;love,240674;love,236803;love,237216;love,237285;love,237507;love,242302;love,243132;love,243257;love,253502;love,253863;love,254021;love,259133;love,297201;love,302788;love,304672;love,304700;love,306709;love,318073;love,326492;love,326536;love,331687;love,341765;love,352525;love,352589;love,383451;love,391282;love,406963;love,428624;love,438846;love,443922;love,446895;love,446907;love,468079;love,477066;love,477748;love,490768;love,491966;love,492678;love,520757;love,537506;love,544056;love,559225;love,574879;love,578404;love,581973;love,584167;love,621517;love,632997;love,633764;love,634023;love,634743;love,639199;love,641202;love,643425;love,644438;love,645821;love,647873;love,649683;love,655577;love,657911;";
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

    ASSERT_EQ(countFounds(res), countFounds(expect));

    using namespace std::chrono;
    std::this_thread::sleep_for(100ms);

    stop();
    // todo fix poll wakeup when `popen` using
    t.detach();
}

TEST_F(NSServerTest, hardBookNetworkTest) {
    std::thread t([this]() {
        this->_params.wordsForSearch = {
                "love",
                "war",
                "123456789123456789123456789123456789123456789123456789123456789",
                "chapter"
        };
        auto res = this->start();
        ASSERT_EQ(0, res);
    });

    std::string expect = "love,1;123456789123456789123456789123456789123456789123456789123456789,43;chapter,969;chapter,979;chapter,989;chapter,999;chapter,1009;chapter,1019;chapter,1029;chapter,1039;chapter,1049;chapter,1059;chapter,1070;chapter,1081;chapter,1092;chapter,1103;chapter,1114;chapter,1125;chapter,1136;chapter,1147;chapter,1158;chapter,1169;chapter,1180;chapter,1191;chapter,1202;chapter,1213;chapter,1224;chapter,1235;chapter,1246;chapter,1257;chapter,1268;chapter,1279;chapter,1290;chapter,1301;chapter,1312;chapter,1323;chapter,1334;chapter,1345;chapter,1356;chapter,1367;chapter,1378;chapter,1389;chapter,1400;chapter,1411;chapter,1422;chapter,1433;chapter,1444;chapter,1455;chapter,1466;chapter,1477;chapter,1488;chapter,1499;chapter,1510;chapter,1521;chapter,1532;chapter,1543;chapter,1554;chapter,1565;chapter,1576;chapter,1587;chapter,1598;chapter,1609;chapter,1620;chapter,1631;love,3228;123456789123456789123456789123456789123456789123456789123456789,4088;chapter,4223;chapter,5949;love,9664;chapter,9981;love,10713;chapter,19152;chapter,24887;chapter,29860;love,30789;love,31695;love,32632;love,41917;love,41927;chapter,42348;love,46348;chapter,53012;chapter,63475;love,69740;love,70130;love,70186;love,70226;chapter,72760;chapter,84668;love,90911;chapter,93188;chapter,97020;love,97699;chapter,105840;chapter,112065;chapter,121556;chapter,139747;chapter,146780;chapter,174865;love,183980;chapter,185164;love,185250;chapter,193951;love,201186;love,201554;love,202273;love,202306;chapter,204807;love,206474;love,209592;love,212047;chapter,214476;chapter,223527;love,227446;love,230907;love,232406;love,232498;chapter,233937;love,233985;love,236946;love,237359;love,237428;love,237650;love,238046;love,238073;love,240817;chapter,242200;love,242445;love,243275;love,243400;love,253645;love,254006;love,254164;chapter,254510;love,259276;chapter,261446;chapter,269429;chapter,282631;chapter,289510;love,297344;chapter,297828;love,302931;love,304815;love,304843;chapter,305993;love,306852;chapter,315917;love,318216;love,326635;love,326679;chapter,327520;love,331830;love,341908;chapter,344043;love,352668;love,352732;chapter,355506;chapter,363026;chapter,368778;chapter,377153;love,383594;chapter,385953;love,391425;chapter,398476;war,403705;love,407106;chapter,408828;love,428767;chapter,435265;love,438989;love,444065;love,447038;love,447050;chapter,448293;chapter,458081;love,468222;chapter,474360;love,477209;love,477891;love,490911;love,492109;love,492821;chapter,495802;chapter,507922;chapter,519545;love,520900;chapter,531660;love,537649;chapter,542420;love,544199;chapter,558365;love,559368;chapter,573777;love,575022;love,578547;love,582116;chapter,582261;love,584310;chapter,594703;chapter,609317;chapter,618391;love,621660;chapter,631513;love,633140;love,633907;love,634166;love,634886;love,639342;love,641345;love,643568;chapter,644457;love,644581;love,645964;love,648016;love,649826;chapter,652838;love,655720;love,658054;";
    std::string prog = "cat pri | nc 127.0.0.1 3129";
    std::string res;
    char tt[8192 * 2];
    memset(tt, 0, 8192 * 2);
    FILE *df = popen(prog.data(), "r");
    while (fscanf(df, "%s", (char *)&tt) > 0) {
        res += tt;
        memset(tt, 0, 8192 * 2);
    }
    fflush(df);
    pclose(df);

    ASSERT_EQ(countFounds(res), countFounds(expect));

    using namespace std::chrono;
    std::this_thread::sleep_for(100ms);

    stop();
    // todo fix poll wakeup when `popen` using
    t.detach();
}

TEST_F(NSServerTest, threadHardBookNetworkTest) {
    std::thread t([this]() {
        this->_params.port = 3133;
        this->_params.numThreads = 4;
        this->_params.maxConnections = 1024;
        this->_params.bufSize = 8192;
        this->_params.wordsForSearch = {
                "love",
                "war",
                "123456789123456789123456789123456789123456789123456789123456789",
                "chapter"
        };
        auto res = this->start();
        ASSERT_EQ(0, res);
    });

    using namespace std::chrono;
    std::this_thread::sleep_for(1000ms);

    static int NUM_WORKERS = 100;
    std::vector<std::thread *> v;
    v.reserve(NUM_WORKERS);
    for (int i = 0; i < NUM_WORKERS; ++ i) {
        v.emplace_back(new std::thread([](){
            std::string expect = "love,1;123456789123456789123456789123456789123456789123456789123456789,43;chapter,969;chapter,979;chapter,989;chapter,999;chapter,1009;chapter,1019;chapter,1029;chapter,1039;chapter,1049;chapter,1059;chapter,1070;chapter,1081;chapter,1092;chapter,1103;chapter,1114;chapter,1125;chapter,1136;chapter,1147;chapter,1158;chapter,1169;chapter,1180;chapter,1191;chapter,1202;chapter,1213;chapter,1224;chapter,1235;chapter,1246;chapter,1257;chapter,1268;chapter,1279;chapter,1290;chapter,1301;chapter,1312;chapter,1323;chapter,1334;chapter,1345;chapter,1356;chapter,1367;chapter,1378;chapter,1389;chapter,1400;chapter,1411;chapter,1422;chapter,1433;chapter,1444;chapter,1455;chapter,1466;chapter,1477;chapter,1488;chapter,1499;chapter,1510;chapter,1521;chapter,1532;chapter,1543;chapter,1554;chapter,1565;chapter,1576;chapter,1587;chapter,1598;chapter,1609;chapter,1620;chapter,1631;love,3228;123456789123456789123456789123456789123456789123456789123456789,4088;chapter,4223;chapter,5949;love,9664;chapter,9981;love,10713;chapter,19152;chapter,24887;chapter,29860;love,30789;love,31695;love,32632;love,41917;love,41927;chapter,42348;love,46348;chapter,53012;chapter,63475;love,69740;love,70130;love,70186;love,70226;chapter,72760;chapter,84668;love,90911;chapter,93188;chapter,97020;love,97699;chapter,105840;chapter,112065;chapter,121556;chapter,139747;chapter,146780;chapter,174865;love,183980;chapter,185164;love,185250;chapter,193951;love,201186;love,201554;love,202273;love,202306;chapter,204807;love,206474;love,209592;love,212047;chapter,214476;chapter,223527;love,227446;love,230907;love,232406;love,232498;chapter,233937;love,233985;love,236946;love,237359;love,237428;love,237650;love,238046;love,238073;love,240817;chapter,242200;love,242445;love,243275;love,243400;love,253645;love,254006;love,254164;chapter,254510;love,259276;chapter,261446;chapter,269429;chapter,282631;chapter,289510;love,297344;chapter,297828;love,302931;love,304815;love,304843;chapter,305993;love,306852;chapter,315917;love,318216;love,326635;love,326679;chapter,327520;love,331830;love,341908;chapter,344043;love,352668;love,352732;chapter,355506;chapter,363026;chapter,368778;chapter,377153;love,383594;chapter,385953;love,391425;chapter,398476;war,403705;love,407106;chapter,408828;love,428767;chapter,435265;love,438989;love,444065;love,447038;love,447050;chapter,448293;chapter,458081;love,468222;chapter,474360;love,477209;love,477891;love,490911;love,492109;love,492821;chapter,495802;chapter,507922;chapter,519545;love,520900;chapter,531660;love,537649;chapter,542420;love,544199;chapter,558365;love,559368;chapter,573777;love,575022;love,578547;love,582116;chapter,582261;love,584310;chapter,594703;chapter,609317;chapter,618391;love,621660;chapter,631513;love,633140;love,633907;love,634166;love,634886;love,639342;love,641345;love,643568;chapter,644457;love,644581;love,645964;love,648016;love,649826;chapter,652838;love,655720;love,658054;";
            std::string prog = "cat pri | nc 127.0.0.1 3133";
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

            ASSERT_EQ(countFounds(res), countFounds(expect));
        }));
    }

    std::this_thread::sleep_for(5000ms);
    stop();
    for (int i = 0; i < NUM_WORKERS; ++ i) {
        v[i]->join();
        delete v[i];
    }

    // todo fix poll wakeup when `popen` using
    t.detach();
}