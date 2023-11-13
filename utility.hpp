
#include <cmath>

int nearestPowerOfTwo_32bit(int n) {
    int x {n};
    x -= 1;
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    x += 1;
    return x;
}
