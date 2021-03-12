#include <gtest/gtest.h>
#include <NSThreadPool.h>

static constexpr int TRIES = 10000;

TEST(NSThreadPool, withTimeout) {
    std::atomic_int check{0};
    NSThreadPool tp(4);

    for (int i = 0; i < TRIES; ++i) {
        tp.enqueue_task([&check](){
            ++check;
        });
    }

    sleep(1);
    ASSERT_EQ(TRIES, check);
}

TEST(NSThreadPool, withoutTimeout) {
    std::atomic_int check{0};
    {
        NSThreadPool tp(4);

        for (int i = 0; i < TRIES; ++i) {
            tp.enqueue_task([&check]() {
                ++check;
            });
        }
    }
    ASSERT_EQ(TRIES, check);
}

TEST(NSThreadPool, stopping) {
    std::atomic_int check{0};
    {
        NSThreadPool tp(4);

        for (int i = 0; i < TRIES; ++i) {
            tp.enqueue_task([&check]() {
                usleep(10);
                ++check;
            });
        }
        tp.stop();
    }
    ASSERT_NE(TRIES, check);
}