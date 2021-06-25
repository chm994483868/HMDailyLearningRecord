//
//  Array.hpp
//  OfferReview
//
//  Created by HM C on 2020/7/30.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef Array_hpp
#define Array_hpp

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <exception>

namespace Partition {

void printArray(std::string desc, int nums[], int count);
void swap(int* num1, int* num2);
int randomInRange(int min, int max);

int partition(int data[], int length, int start, int end);

}

#endif /* Array_hpp */
