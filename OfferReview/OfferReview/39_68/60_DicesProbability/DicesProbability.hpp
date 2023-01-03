//
//  DicesProbability.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/11.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef DicesProbability_hpp
#define DicesProbability_hpp

#include <stdio.h>
#include <math.h>

namespace DicesProbability {

int g_maxValue = 6;

void Probability(int number, int* pProbabilities);
void Probability(int original, int current, int sum, int* pProbabilities);

void PrintProbability_Solution1(int number) {
    if(number < 1)
        return;
 
    int maxSum = number * g_maxValue;
    int* pProbabilities = new int[maxSum - number + 1];
    for(int i = number; i <= maxSum; ++i)
        pProbabilities[i - number] = 0;
 
    Probability(number, pProbabilities);
 
    int total = pow((double)g_maxValue, number);
    for(int i = number; i <= maxSum; ++i) {
        double ratio = (double)pProbabilities[i - number] / total;
        printf("%d: %e\n", i, ratio);
    }
 
    delete[] pProbabilities;
}
 
void Probability(int number, int* pProbabilities) {
    for(int i = 1; i <= g_maxValue; ++i)
        Probability(number, number, i, pProbabilities);
}
 
void Probability(int original, int current, int sum, int* pProbabilities) {
    if(current == 1) {
        pProbabilities[sum - original]++;
    } else {
        for(int i = 1; i <= g_maxValue; ++i) {
            Probability(original, current - 1, i + sum, pProbabilities);
        }
    }
}

}

#endif /* DicesProbability_hpp */
