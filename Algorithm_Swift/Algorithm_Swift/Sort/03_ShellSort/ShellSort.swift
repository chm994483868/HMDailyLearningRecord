//
//  ShellSort.swift
//  Algorithm_Swift
//
//  Created by CHM on 2020/8/4.
//  Copyright © 2020 CHM. All rights reserved.
//

import Foundation

class ShellSort {
    private static func shellSort<T>(_ array: inout Array<T>) where T: Comparable {
        guard !array.isEmpty else {
            return
        }
        let count = array.count
        var gap = count / 2;
        
//        while gap > 0 {
//            for i in gap..<count {
//                for j in stride(from: i - gap, through: 0, by: -gap) {
//                    if array[j] > array[j + gap] {
//                        array.swapAt(j, j + gap)
//                    } else {
//                        break
//                    }
//                }
//            }
//            gap /= 2
//        }
        
        while gap > 0 {
            for i in 0..<gap {
                var j = i + gap
                while j < count {
                    var k = j - gap
                    while k >= 0 && array[k] > array[k + gap] {
                        array.swapAt(k, k + gap)
                        k -= gap
                    }
                    j += gap
                }
            }
            gap /= 2
        }
    }
    
    private static func test<T>(_ testName: String, _ array: inout Array<T>) where T: Comparable {
        print("\(testName) begins:")
        print(array)
        shellSort(&array)
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
