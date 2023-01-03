//
//  ReverseWordsInSentence.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/10.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef ReverseWordsInSentence_hpp
#define ReverseWordsInSentence_hpp

#include <stdio.h>
#include <cstdio>
#include <string>

namespace ReverseWordsInSentence {

// 面试题58（一）：翻转单词顺序
// 题目：输入一个英文句子，翻转句子中单词的顺序，但单词内字符的顺序不变。
// 为简单起见，标点符号和普通字母一样处理。例如输入字符串"I am a student. "，
// 则输出"student. a am I"。

void reverse(char* pBegin, char* pEnd);
char* reverseSentence(char* pData);

}

#endif /* ReverseWordsInSentence_hpp */
