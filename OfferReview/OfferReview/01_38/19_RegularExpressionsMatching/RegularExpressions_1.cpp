//
//  RegularExpressions_1.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/4.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "RegularExpressions_1.hpp"

bool RegularExpressions_1::matchCore(const char* str, const char* pattern) {
    if (*str == '\0' && *pattern == '\0') {
        // 如果匹配字符串和模式字符串都到了末尾，则返回 true
        return true;
    }
    
    if (*str != '\0' && *pattern == '\0') {
        // 如果匹配字符串还没到末尾而模式字符串到了末尾，则返回 false
        return false;
    }
    
    if (*(pattern + 1) == '*') {
        // 如果模式字符串第二个字符是 *
        if (*str == *pattern || (*pattern == '.' && *str != '\0')) {
            return matchCore(str, pattern + 2) || matchCore(str + 1, pattern) || matchCore(str + 1, pattern + 2);
        } else {
            return matchCore(str, pattern + 2);
        }
    } else {
        if (*str == *pattern || (*pattern == '.' && *str != '\0')) {
            return matchCore(str + 1, pattern + 1);
        }
    }
    
    return false;
}

bool RegularExpressions_1::match(const char* str, const char* pattern) {
    if (str == nullptr || pattern == nullptr) {
        return false;
    }
    
    return matchCore(str, pattern);
}

// 测试代码
void RegularExpressions_1::Test(const char* testName, const char* string, const char* pattern, bool expected) {
    if (testName != nullptr)
        printf("%s begins: ", testName);

    if (match(string, pattern) == expected)
        printf("Passed.\n");
    else
        printf("FAILED.\n");
}

void RegularExpressions_1::Test() {
    Test("Test01", "", "", true);
    Test("Test02", "", ".*", true);
    Test("Test03", "", ".", false);
    Test("Test04", "", "c*", true);
    Test("Test05", "a", ".*", true);
    Test("Test06", "a", "a.", false);
    Test("Test07", "a", "", false);
    Test("Test08", "a", ".", true);
    Test("Test09", "a", "ab*", true);
    Test("Test10", "a", "ab*a", false);
    Test("Test11", "aa", "aa", true);
    Test("Test12", "aa", "a*", true);
    Test("Test13", "aa", ".*", true);
    Test("Test14", "aa", ".", false);
    Test("Test15", "ab", ".*", true);
    Test("Test16", "ab", ".*", true);
    Test("Test17", "aaa", "aa*", true);
    Test("Test18", "aaa", "aa.a", false);
    Test("Test19", "aaa", "a.a", true);
    Test("Test20", "aaa", ".a", false);
    Test("Test21", "aaa", "a*a", true);
    Test("Test22", "aaa", "ab*a", false);
    Test("Test23", "aaa", "ab*ac*a", true);
    Test("Test24", "aaa", "ab*a*c*a", true);
    Test("Test25", "aaa", ".*", true);
    Test("Test26", "aab", "c*a*b", true);
    Test("Test27", "aaca", "ab*a*c*a", true);
    Test("Test28", "aaba", "ab*a*c*a", false);
    Test("Test29", "bbbba", ".*a*a", true);
    Test("Test30", "bcbbabab", ".*a*a", false);
}

//class Solution {
//public:
//    bool isMatch(string s, string p) {
//        if (s.length() == 0 && p.length() == 0) {
//            return true;
//        }
//
//        if (s.length() > 0 && p.length() == 0) {
//            return false;
//        }
//
//        if (p[1] == '*') {
//            if (s[0] == p[0] || (p[0] == '.' && s.length() != 0)) {
//                return isMatch(s, p.substr(2)) || isMatch(s.substr(1), p) || isMatch(s.substr(1), p.substr(2));
//            } else {
//                return isMatch(s, p.substr(2));
//            }
//        } else {
//            if (s[0] == p[0] || (p[0] == '.' && s.length() != 0)) {
//                return isMatch(s.substr(1), p.substr(1));
//            }
//        }
//
//        return false;
//    }
//};

//class Solution {
//public:
//    bool isMatch(string s, string p) {
//        if (p.empty()) {
//            return s.empty();
//        }
//
//        if (p[1] == '*') {
////            return isMatch(s, p.substr(2)) || (!s.empty() && (s[0] == p[0] || p[0] == '.')) && isMatch(s.substr(1), p);
//            return isMatch(s, p.substr(2)) || ((!s.empty() && (s[0] == p[0] || p[0] == '.')) && isMatch(s.substr(1), p));
//        } else {
//            return !s.empty() && (s[0] == p[0] || p[0] == '.') && (isMatch(s.substr(1), p.substr(1)));
//        }
//    }
//};
