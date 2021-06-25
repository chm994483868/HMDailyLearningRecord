//
//  BubbleSort.swift
//  Algorithm_Swift
//
//  Created by CHM on 2020/8/4.
//  Copyright © 2020 CHM. All rights reserved.
//

import Foundation

class BubbleSort {
    private static func bubbleSort<T>(_ array: inout Array<T>) where T: Comparable {
        guard !array.isEmpty else { return }
        
        let count = array.count
        var k = count - 1
        for _ in 0..<count-1 {
            var noExchange = true
            var minIndex = 0
            for j in 0..<k {
                if array[j] > array[j + 1] {
                    array.swapAt(j, j + 1)
                    noExchange = false // 记录本次循环是否发生过交换，如果没有，则表示当前数组已经是有序的了
                    minIndex = j // 用于记录本次循环发生最后一次交换时的最小下标
                }
            }
            if noExchange { break }
            k = minIndex
        }
    }
    
    private static func test<T>(_ testName: String, _ array: inout Array<T>) where T: Comparable {
        print("\(testName) begins:")
        print(array)
        bubbleSort(&array)
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
