//
// Created by Ivan Marochkin on 12.03.2021.
//

#include <NSServer.h>


int main() {
    NSServer s(4);
    s.start(3129);

    return 0;
}