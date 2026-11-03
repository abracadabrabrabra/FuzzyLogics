#include "ipc.h"
#include <cassert>
#include <cstring>
#include <iostream>

static void test_create_and_access() {
    auto shared = ipc::SharedSession::create("TestSession", 1024);
    assert(shared.data() != nullptr);
    assert(shared.size() >= 1024);

    std::memcpy(shared.data(), "hello", 6);

    assert(std::memcmp(shared.data(), "hello", 6) == 0);
}

static void test_lock_unlock() {
    auto shared = ipc::SharedSession::create("TestLock", 64);
    shared.lock();
    std::memcpy(shared.data(), "abc", 4);
    shared.unlock();
    assert(std::memcmp(shared.data(), "abc", 4) == 0);
}

static void test_open_in_same_process() {
    auto creator = ipc::SharedSession::create("TestOpen", 128);
    auto opener = ipc::SharedSession::open("TestOpen");
    assert(opener.data() != nullptr);

    std::memcpy(creator.data(), "xyz", 4);
    assert(std::memcmp(opener.data(), "xyz", 4) == 0);
}

int main() {
    test_create_and_access();
    test_lock_unlock();
    test_open_in_same_process();
    std::cout << "All ipc tests passed\n";
    return 0;
}