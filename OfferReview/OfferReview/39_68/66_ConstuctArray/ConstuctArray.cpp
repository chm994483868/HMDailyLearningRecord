//
//  ConstuctArray.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/11.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "ConstuctArray.hpp"

void ConstuctArray::buildProductionArray(const vector<double>& input, vector<double>& output) {
    unsigned long length1 = input.size();
    unsigned long length2 = output.size();
    
    if (length1 == length2 && length2 > 1) {
        output[0] = 1;
        for (int i = 1; i < length1; ++i) {
            output[i] = output[i - 1] * input[i - 1];
        }
        
        double temp = 1;
        for (unsigned long i = length1 - 2; i >= 0; --i) {
            temp *= input[i + 1];
            output[i] *= temp;
        }
    }
}
