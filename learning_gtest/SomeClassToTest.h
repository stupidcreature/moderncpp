#ifndef MODERNCPP_SOMECLASSTOTEST_H
#define MODERNCPP_SOMECLASSTOTEST_H

#include <stdint.h>

class SomeClassToTest {
public:
    SomeClassToTest() = default;
    virtual ~SomeClassToTest() = default;

    int64_t AddTwoNumbers(int64_t a, int64_t b) {
        return a+b;
    }

private:
public:
    int64_t some_internal_state_ = 0;

};


#endif //MODERNCPP_SOMECLASSTOTEST_H
