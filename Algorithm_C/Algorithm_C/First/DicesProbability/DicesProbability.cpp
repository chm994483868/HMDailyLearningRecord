//
//  DicesProbability.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/26.
//  Copyright Â© 2020 CHM. All rights reserved.
//

#include "DicesProbability.hpp"

int g_maxValue = 6;

void probability(int number, int* pProbabilities);
void probability(int original, int current, int sum, int* pProbabilities);

void printProbability_Solution1(int number) {
    // 1. å¦æð²ä¸ªæ°å°äº 1ï¼å¯ä»¥ç´æ¥è¿åäº
    if (number < 1) {
        return;
    }
    
    // 2. number ä¸ªð²ï¼ä¸æ¬¡åºç°çæå¤§åæ¯ð²æ¯ä¸ªé¢é½æ¯ 6ï¼å³ number * 6
    int maxSum = number * g_maxValue;
    // 3. åå¤ä¸ä¸ªææåçå¯è½åºç°çæ¬¡æ°å¤æ°ç»
    // æ°ç»é¿åº¦æ¯ææåå¯è½åºç°çæåµï¼æå°æ¶æ¯ð²æ¯ä¸ªé¢é½æ¯ 1ï¼å³ number * 1
    // æä»¥æå°çååæå¤§çåä¹é´å·®è·ï¼maxSum - number, å³ææå¯è½åºç°çç»æå³: maxSum - number + 1
    // å³ä¸é¢æ°ç»éæ¯ä¸ªåç´ å¯¹åºä¸ä¸ªç¹æ°ä¹ååºç°çæ¬¡æ°
    int* pProbabilities = new int[maxSum - number + 1];
    // 4. æææåç´ ç½®ä¸º 0
    for (int i = number; i <= maxSum; ++i) {
        pProbabilities[i - number] = 0;
    }
    
    // 8. è¿ä¸ªå½æ°å°åºæ¯ä»ä¹ææ
    probability(number, pProbabilities);
    
    // 5. number ä¸ªð²æä¸å»ï¼ææå¯è½åºç°çæåµå³ 6 ç number æ¬¡æ¹ä¸ª
    int total = pow((double)g_maxValue, number);
    // 6. æå°ææåçå¼åºç°çæ¬¡æ°ä¸æ»æåµçæ¯ç
    for (int i = number; i <= maxSum; ++i) {
        double ratio = (double)pProbabilities[i - number] / total;
        printf("%d: %e\n", i, ratio);
    }
    // 7. éæ¾åå­
    delete [] pProbabilities;
}

// åè®¾ number = 5 maxSum = 30 minSum = 6 30 - 5 + 1 = 26
void probability(int number, int* pProbabilities) {
    // i ä» 1 å° 6 çä¸ä¸ªå¾ªç¯
    for (int i = 1; i <= g_maxValue; ++i) {
        // i = 1 æ¶: 5 5 1 int[26]
        // i = 2 æ¶: 5 5 2 int[26]
        // ...
        // i = 6 æ¶: 5 5 6 int[26]
        probability(number, number, i, pProbabilities);
    }
}

// è¿ä¸ªéå½å½æ°åæ¯å¹²å¥ç
// åè®¾ç¬¬ä¸è½®è¿æ¥:
// original = 5 current = 5 sum = 1 int[26]
// current != 1 è¿å¥ else éé¢ç 6 æ¬¡å¾ªç¯

// i = 1 æ¶: original = 5 current = 4 sum = 2 int[26] è¿å¥ä¸ä¸ªéå½:
// original = 5 current = 4 sum = 2 int[26]
//
// original = 5 current = 5 sum = 1 int[26] æ åº
void probability(int original, int current, int sum, int* pProbabilities) {
    if (current == 1) {
        ++pProbabilities[sum - original];
    } else {
        for (int i = 1; i <= g_maxValue; ++i) {
            probability(original, current - 1, i + sum, pProbabilities);
        }
    }
}

// 5 5 1
// 5 4 2
// 5 3 3
// 5 2 4
// 5 1 5

// 1 2 3 4 5 6
// 1 2 3 4 5 6

// n = 2 çæåµ f(k) = f(k-1) + f(k-2) + f(k-3) + f(k-4) + f(k-5) + f(k-6)
// k = 7 æ¶: f(7) = f(6) + f(5) + f(4) + f(3) + f(2) + f(1)
//
// 2 3 4 5 6 7
// 3 4 5 6 7 8
// 4 5 6 7 8 9
// 5 6 7 8 9 10
// 6 7 8 9 10 11
// 7 8 9 10 11 12

// 2 3 3
