//
//  RegularExpressions_1.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/4.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef RegularExpressions_1_hpp
#define RegularExpressions_1_hpp

#include <stdio.h>
#include <string>

using namespace std;

namespace RegularExpressions_1 {

// 19：正则表达式匹配
// 题目：请实现一个函数用来匹配包含'.'和'*'的正则表达式。模式中的字符'.'
// 表示任意一个字符，而'*'表示它前面的字符可以出现任意次（含0次）。在本题
// 中，匹配是指字符串的所有字符匹配整个模式。例如，字符串"aaa"与模式"a.a"
// 和"ab*ac*a"匹配，但与"aa.a"及"ab*a"均不匹配。

bool matchCore(const char* str, const char* pattern);
bool match(const char* str, const char* pattern);

// 测试代码
void Test(const char* testName, const char* string, const char* pattern, bool expected);

void Test();

}

#endif /* RegularExpressions_1_hpp */
