//
//  MaximalProfit.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/11.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef MaximalProfit_hpp
#define MaximalProfit_hpp

#include <stdio.h>
#include <cstdio>

namespace MaximalProfit {

// 面试题63：股票的最大利润
// 题目：假设把某股票的价格按照时间先后顺序存储在数组中，请问买卖交易该股
// 票可能获得的利润是多少？例如一只股票在某些时间节点的价格为{9, 11, 8, 5,
// 7, 12, 16, 14}。如果我们能在价格为5的时候买入并在价格为16时卖出，则能
// 收获最大的利润11。

int maxDiff(const int* numbers, unsigned length);

}

#endif /* MaximalProfit_hpp */
