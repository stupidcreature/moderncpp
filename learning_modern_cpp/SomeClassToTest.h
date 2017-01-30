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

};


#endif //MODERNCPP_SOMECLASSTOTEST_H
