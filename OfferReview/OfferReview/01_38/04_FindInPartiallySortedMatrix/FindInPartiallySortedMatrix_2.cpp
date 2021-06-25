//
//  FindInPartiallySortedMatrix_2.cpp
//  OfferReview
//
//  Created by CHM on 2021/1/29.
//  Copyright © 2021 CHM. All rights reserved.
//

#include "FindInPartiallySortedMatrix_2.hpp"

bool FindInPartiallySortedMatrix_2::find(int* matrix, int rows, int columns, int number) {
    bool found = false;
    
    if (matrix != nullptr && rows > 0 && columns > 0) {
        int row = 0;
        int column = columns - 1;
        
        while (row < rows && column >= 0) {
            int current = matrix[row * columns + column];
            if (current == number) {
                found = true;
                break;
            } else if (current > number) {
                --column;
            } else {
                ++row;
            }
        }
    }
    
    return found;
}
