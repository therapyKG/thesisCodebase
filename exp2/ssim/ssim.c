#include "ssim.h"

uint16_t getLuminenceScore(iMat img){
    float res = 0.0;
    for(unsigned int i = 0; i < ROW; i ++){
        for(unsigned int j = 0; j < COL; j ++){
            res += img[i][j]/(ROW * COL);
        }
    }
    return (uint16_t)res;
}

uint16_t getContrastScore(iMat img, uint16_t luminence){
    float res = 0.0;
    for(unsigned int i = 0; i < ROW; i ++){
        for(unsigned int j = 0; j < COL; j ++){
            res += SQ(img[i][j] - luminence)/(ROW * COL - 1);
        }
    }
    return (uint16_t)res;
}

uint16_t SSIM(iMat img1, iMat img2){
    uint16_t L1 = getLuminenceScore(img1);
    uint16_t L2 = getLuminenceScore(img2);

    uint16_t C1 = getContrastScore(img1, L1);
    uint16_t C2 = getContrastScore(img2, L2);

    float C12 = 0.0;
    for(unsigned int i = 0; i < ROW; i ++){
        for(unsigned int j = 0; j < COL; j ++){
            C12 += (img1[i][j] - L1) * (img2[i][j] - L2)/(N1 - 1);
        }
    }
    uint16_t C12_i = (uint16_t)C12;
    uint16_t denom = (SQ(L1) + SQ(L2) + C_1) * (C1 + C2 + C_2);
    uint16_t numer = (2 * L1 * L2 + C_1) * (2 * C12_i + C_2);
    //optional: modify alpha, beta, gamma and use them here for actual SSIM formula
    return (uint16_t)((float)(numer/denom)*scalar);
}

void putTestImg(iMat img){
    for(unsigned int j = 0; j < ROW; j ++){
        for(unsigned int k = 0; k < COL; k ++){
            img[j][k] = k;
        }
    }
}
