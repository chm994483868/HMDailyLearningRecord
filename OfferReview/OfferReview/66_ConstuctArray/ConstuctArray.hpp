//
//  ConstuctArray.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/11.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef ConstuctArray_hpp
#define ConstuctArray_hpp

#include <stdio.h>
#include <cstdio>
#include <vector>

using namespace std;

namespace ConstuctArray {

// 面试题66：构建乘积数组
// 题目：给定一个数组A[0, 1, ⋯, n-1]，请构建一个数组B[0, 1, ⋯, n-1]，其
// 中B中的元素B[i] =A[0]×A[1]×⋯ ×A[i-1]×A[i+1]×⋯×A[n-1]。不能使用除法。

void buildProductionArray(const vector<double>& input, vector<double>& output);

}
#endif /* ConstuctArray_hpp */
