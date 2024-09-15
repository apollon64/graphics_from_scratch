#ifndef MISC_H
#define MISC_H

// frand is a random number generator
/**
 * @brief frand is a random number generator
 *  
 * @return float 0 <= frand() < 1
 *  
 */
float frand();
int EndsWith(const char *str, const char *suffix);
//float maxf(float a, float b) { return a > b ? a : b; }
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#endif
