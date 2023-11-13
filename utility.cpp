#include "utility.hpp"

int nearestPowerOfTwo_32bit(int n) {
    n -= 1;
    n |= (n >> 1);
    n |= (n >> 2);
    n |= (n >> 4);
    n |= (n >> 8);
    n |= (n >> 16);
    n += 1;
    return n;
}
