//
//  MergeSort.swift
//  Algorithm_Swift
//
//  Created by CHM on 2020/8/4.
//  Copyright © 2020 CHM. All rights reserved.
//

import Foundation

class MergeSort {
    private static func mergeSort<T>(_ array: inout Array<T>) where T: Comparable {
        guard !array.isEmpty else {
            return
        }
        
        mergeSortCore(&array, 0, array.count - 1)
    }
    
    private static func mergeSortCore<T>(_ array: inout Array<T>, _ first: Int, _ last: Int) where T: Comparable {
        guard first < last else {
            return
        }
        
        let mid = ((last - first) / 2) + first;
        
        mergeSortCore(&array, first, mid)
        mergeSortCore(&array, mid + 1, last)
        
        mergeArray(&array, first, mid, last)
    }
    
    private static func mergeArray<T>(_ array: inout Array<T>, _ first: Int, _ mid: Int, _ last: Int) where T: Comparable {
        var i = first, j = mid + 1
        let m = mid, n = last
        var k = 0
        
        var temp = Array<T>()
        while i <= m && j <= n {
            if array[i] <= array[j] {
                temp.append(array[i])
                i += 1
                k += 1
            } else {
                temp.append(array[j])
                j += 1
                k += 1
            }
        }
        
        while i <= m {
            temp.append(array[i])
            k += 1
            i += 1
        }
        
        while j <= n {
            temp.append(array[j])
            k += 1
            j += 1
        }
        
        for i in 0..<k {
            array[first + i] = temp[i]
        }
    }
    
    private static func test<T>(_ testName: String, _ array: inout Array<T>) where T: Comparable {
        print("\(testName) begins:")
        print(array)
        mergeSort(&array)
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
