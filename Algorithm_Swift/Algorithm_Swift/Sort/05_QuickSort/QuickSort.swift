//
//  QuickSort.swift
//  Algorithm_Swift
//
//  Created by CHM on 2020/8/4.
//  Copyright © 2020 CHM. All rights reserved.
//

import Foundation

class QuickSort {
    
    private static func quickSort<T>(_ array: inout Array<T>) where T: Comparable {
        guard !array.isEmpty else {
            return
        }
        
        quickSortCore(&array, 0, array.count - 1)
    }
    
    private static func quickSortCore<T>(_ array: inout Array<T>, _ l: Int, _ r: Int) where T: Comparable {
        guard l < r else {
            return
        }
        
        var i = l, j = r
        let x = array[l];
        
        while i < j {
            while i < j && array[j] >= x {
                j -= 1
            }
            if i < j {
                array[i] = array[j]
                i += 1
            }
            
            while i < j && array[i] < x {
                i += 1
            }
            if i < j {
                array[j] = array[i]
                j -= 1
            }
        }
        
        array[i] = x
        quickSortCore(&array, l, i - 1)
        quickSortCore(&array, i + 1, r)
    }
    
    private static func test<T>(_ testName: String, _ array: inout Array<T>) where T: Comparable {
        print("\(testName) begins:")
        print(array)
        quickSort(&array)
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
