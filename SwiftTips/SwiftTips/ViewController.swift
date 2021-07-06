//
//  ViewController.swift
//  SwiftTips
//
//  Created by HM C on 2021/7/6.
//

import UIKit

class ViewController: UIViewController {

    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view.
        
        view.backgroundColor = .gray
        
//        great("John", on: "Wednesday")
        
//        sumOf(numbers: 42, 597, 12)
//        sumOf()
        
        let numbers = [1, 2, 3]
        let newNumbers = numbers.map { number in
            3 * number
        }
        
        print(numbers)
        print(newNumbers)
        
        print(123456[0])
        
        print(123456[8])
        
        anyCommonElements([1, 3, 2], [3])
        
        var things = [Any]()
        
        let optionalNumber: Int? = 3
        
        things.append(optionalNumber as Any)
        
    }

}

extension ViewController {
    func great(_ person: String, on day: String) -> String {
        return "Hello \(person), today is \(day)."
    }
    
    func sumOf(numbers: Int...) -> Int {
        var sum = 0
        for number in numbers {
            sum += number
        }
        
        return sum
    }
    
    
    func send(job: Int, toPrinter printerName: String) throws -> String {
        if printerName == "Never Has Toner" {
            throw NSError.init()
        }
        
        return "Success"
    }
    
    func anyCommonElements<T: Sequence, U: Sequence>(_ lhs: T, _ rhs: U) -> Bool where T.Iterator.Element: Equatable, T.Iterator.Element == U.Iterator.Element {
        for lhsItem in lhs {
            for rhsItem in rhs {
                if lhsItem == rhsItem {
                    return true
                }
            }
        }
        
        return false
    }
}

extension Int {
    subscript(digitIndex: Int) -> Int {
        var decimalBase = 1
        for _ in 0..<digitIndex {
            decimalBase *= 10
        }
        
        return (self / decimalBase) % 10
    }
}

extension Int {
    enum Kind {
        case negative, zero, positive
    }
    
    var kind: Kind {
        switch self {
        case 0:
            return .zero
        case let x where x > 0:
            return .positive
        default:
            return .negative
        }
    }
}

