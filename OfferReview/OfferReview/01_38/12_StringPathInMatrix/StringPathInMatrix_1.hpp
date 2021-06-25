//
//  StringPathInMatrix_1.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/2.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef StringPathInMatrix_1_hpp
#define StringPathInMatrix_1_hpp

#include <stdio.h>
#include <string>

namespace StringPathInMatrix_1 {

// 12：矩阵中的路径
// 题目：请设计一个函数，用来判断在一个矩阵中是否存在一条包含某字符串所有
// 字符的路径。路径可以从矩阵中任意一格开始，每一步可以在矩阵中向左、右、
// 上、下移动一格。如果一条路径经过了矩阵的某一格，那么该路径不能再次进入
// 该格子。例如在下面的3×4的矩阵中包含一条字符串“bfce”的路径（路径中的字
// 母用下划线标出）。但矩阵中不包含字符串“abfb”的路径，因为字符串的第一个
// 字符b占据了矩阵中的第一行第二个格子之后，路径不能再次进入这个格子。
// A B T G
// C F C S
// J D E H

bool hasPathCore(const char* matrix, int rows, int cols, int row, int col, const char* str, int& pathLength, bool* visited);
bool hasPath(const char* matrix, int rows, int cols, const char* str);

// 测试代码
void Test(const char* testName, const char* matrix, int rows, int cols, const char* str, bool expected);
//ABTG
//CFCS
//JDEH

//BFCE
void Test1();
//ABCE
//SFCS
//ADEE

//SEE
void Test2();
//ABTG
//CFCS
//JDEH

//ABFB
void Test3();
//ABCEHJIG
//SFCSLOPQ
//ADEEMNOE
//ADIDEJFM
//VCEIFGGS

//SLHECCEIDEJFGGFIE
void Test4();
//ABCEHJIG
//SFCSLOPQ
//ADEEMNOE
//ADIDEJFM
//VCEIFGGS

//SGGFIECVAASABCEHJIGQEM
void Test5();
//ABCEHJIG
//SFCSLOPQ
//ADEEMNOE
//ADIDEJFM
//VCEIFGGS

//SGGFIECVAASABCEEJIGOEM
void Test6();
//ABCEHJIG
//SFCSLOPQ
//ADEEMNOE
//ADIDEJFM
//VCEIFGGS

//SGGFIECVAASABCEHJIGQEMS
void Test7();
//AAAA
//AAAA
//AAAA

//AAAAAAAAAAAA
void Test8();
//AAAA
//AAAA
//AAAA

//AAAAAAAAAAAAA
void Test9();
//A

//A
void Test10();
//A

//B
void Test11();
void Test12();

void Test();

}

#endif /* StringPathInMatrix_1_hpp */
