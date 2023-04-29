#include "DSPLib/include/DSPLib.h"
#include <stdlib.h>
#include <math.h>

#define A 1
#define BETA 1
#define GAMMA 1

#define ROW 28
#define COL 28
#define N1 784

#define SQ(x) (x * x)
#define C_1 7.0
#define C_2 59.0
#define C_3 29.0

#define scalar 10000

typedef uint16_t iMat[ROW][COL];

uint16_t getLuminenceScore(iMat img);

uint16_t getContrastScore(iMat img, uint16_t luminence);

uint16_t SSIM(iMat img1, iMat img2);

void putTestImg(iMat img);
