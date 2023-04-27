#include "ssim.h"

int getLuminenceScore(iMat img){
    float res = 0.0;
    for(unsigned int i = 0; i < ROW; i ++){
        for(unsigned int j = 0; j < COL; j ++){
            res += img[i][j]/(ROW * COL);
        }
    }
    return res;
}

int getContrastScore(iMat img, int luminence){
    int res = 0;
    for(unsigned int i = 0; i < ROW; i ++){
        for(unsigned int j = 0; j < COL; j ++){
            res += SQ(img[i][j] - luminence)/(ROW * COL - 1);
        }
    }
    return res;
}

int SSIM(iMat img1, iMat img2){
    int L1 = getLuminenceScore(img1);
    int L2 = getLuminenceScore(img2);

    int C1 = getContrastScore(img1, L1);
    int C2 = getContrastScore(img2, L2);

    int C12 = 0;
    for(unsigned int i = 0; i < ROW; i ++){
        for(unsigned int j = 0; j < COL; j ++){
            C12 += (img1[i][j] - L1) * (img2[i][j] - L2)/(N1 - 1);
        }

    }
    
    int denom = (SQ(L1) + SQ(L2) + C_1) * (C1 + C2 + C_2);
    int numer = (2.0 * L1 * L2 + C_1) * (2.0 * C12 + C_2);
    //optional: modify alpha, beta, gamma and use them here for actual SSIM formula
    return (int)((float)(numer/denom)*scalar);
}

void putTestImg(iMat img){
    for(unsigned int j = 0; j < ROW; j ++){
        for(unsigned int k = 0; k < COL; k ++){
            img[j][k] = 9*k;
        }
    }
}
