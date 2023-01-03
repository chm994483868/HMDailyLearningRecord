//
//  LeftRotateString.hpp
//  OfferReview
//
//  Created by HM C on 2020/11/11.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef LeftRotateString_hpp
#define LeftRotateString_hpp

#include <stdio.h>
#include <cstdio>
#include <string>

namespace LeftRotateString {

// 面试题58（二）：左旋转字符串
// 题目：字符串的左旋转操作是把字符串前面的若干个字符转移到字符串的尾部。
// 请定义一个函数实现字符串左旋转操作的功能。比如输入字符串 "abcdefg" 和数
// 字 2，该函数将返回左旋转 2 位得到的结果 "cdefgab"。

void reverse(char* pBegin, char* pEnd);
char* leftRotateString(char* pStr, int n);

}
#endif /* LeftRotateString_hpp */
