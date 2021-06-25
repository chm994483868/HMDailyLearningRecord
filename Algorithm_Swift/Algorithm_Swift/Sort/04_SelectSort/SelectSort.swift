//
//  SelectSort.swift
//  Algorithm_Swift
//
//  Created by CHM on 2020/8/4.
//  Copyright © 2020 CHM. All rights reserved.
//

import Foundation

class SelectSort {
    
    private static func selectSort<T>(_ array: inout Array<T>) where T: Comparable {
        guard !array.isEmpty else {
            return
        }
        let count = array.count
        for i in 0..<count {
            var minIndex = i
            for j in i + 1..<count {
                if array[j] < array[minIndex] {
                    minIndex = j
                }
            }
            array.swapAt(i, minIndex)
        }
    }
    
    private static func test<T>(_ testName: String, _ array: inout Array<T>) where T: Comparable {
        print("\(testName) begins:")
        print(array)
        selectSort(&array)
        print(array)
    }
    
    // 随机
    private static func test1() {
        var array = [4, 6, 1, 2, 9, 10, 20, 1, 1]
        test("test1", &array);
    }
    
    // 升序
    private static func test2() {
        var array = [1, 2, 3, 4, 5, 6, 7, 8]
        test("test2", &array);
    }
    
    // 降序
    private static func test3() {
        var array = [5, 4, 3, 2, 1]
        test("test3", &array);
    }
    
    // 有负数
    private static func test4() {
        var array = [-4, 6, -2, 9, 10, -20, 1, -1]
        test("test4", &array);
    }
    
    // 一个元素
    private static func test5() {
        var array = [4, 2]
        test("test5", &array);
    }
    
    static func Test() {
        test1()
        test2()
        test3()
        test4()
        test5()
    }
}
