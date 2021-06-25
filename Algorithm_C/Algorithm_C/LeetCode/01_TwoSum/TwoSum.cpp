//
//  TwoSum.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/8/5.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "TwoSum.hpp"

vector<int> TwoSum::twoSum(vector<int>& nums, int target) {
    map<int, int> a;
    vector<int> b(2, -1);
    for (int i = 0; i < nums.size(); ++i) {
        if (a.count(target - nums[i]) > 0) {
            b[0] = a[target - nums[i]];
            b[1] = i;
        }
        
        a.insert(map<int, int>::value_type(nums[i], i));
    }
    
    return b;
}

void mapPractice() {
    std::map<int, string> personnel;
    typedef map<int, string> UDT_MAP_INT_STRING;
    UDT_MAP_INT_STRING enumMap;
    
    // 定义一个 map 对象
    map<int, string> mapStudent;
    // 第一种 用 insert 函数插入 pair
    mapStudent.insert(pair<int, string>(000, "student_zero"));
    // 第二种 用 insert 函数插入 value_type 数据
    mapStudent.insert(map<int, string>::value_type(001, "student_one"));
    // 第三种 用 array 方式插入
    mapStudent[123] = "student_first";
    mapStudent[456] = "student_second";
    
    // 构造定义，返回一个 pair 对象
    pair<map<int, string>::iterator, bool> Insert_Pair;
    Insert_Pair = mapStudent.insert(map<int, string>::value_type(001, "student_one"));
    if (!Insert_Pair.second) {
        printf("Error insert new element");
    }
    
    map<int, string>::iterator iter = mapStudent.find(123);
    if (iter != mapStudent.end()) {
        
        // 删除元素
        // 用关键字删除
        mapStudent.erase(123);
        // 用迭代器删除
        mapStudent.erase(mapStudent.begin(), mapStudent.end());
        // map 大小
        unsigned long nSize = mapStudent.size();
        printf("%ld", nSize);
    }
}
