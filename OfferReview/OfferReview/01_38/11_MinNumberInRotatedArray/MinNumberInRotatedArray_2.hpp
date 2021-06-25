//
//  MinNumberInRotatedArray_2.hpp
//  OfferReview
//
//  Created by CHM on 2021/2/22.
//  Copyright © 2021 CHM. All rights reserved.
//

#ifndef MinNumberInRotatedArray_2_hpp
#define MinNumberInRotatedArray_2_hpp

#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include <vector>

using namespace std;

namespace MinNumberInRotatedArray_2 {

unsigned int randomInRange(unsigned int start, unsigned int end);
int partition(int data[], int length, int start, int end);
void swap(int* num1, int* num2);
void quickSort(int data[], int length, int start, int end);
void sortAges(int ages[], int length);

int minInorder(int* numbers, int index1, int index2);
int min(int* numbers, int length);

}

#endif /* MinNumberInRotatedArray_2_hpp */
