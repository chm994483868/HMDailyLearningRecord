# 《剑指 Offer》面试题六十一～面试题六十八的总结

> &emsp;上一篇是 51～60 题，本篇是 61～68 题。⛽️⛽️

## 面试题 61:扑克牌的顺子
&emsp;题目：从扑克牌中随机抽 5 张牌，判断是不是一个顺子，即这 5 张牌是不是连续的。2〜10 为数字本身，A 为 1，J 为 11，Q 为 12，K 为 13，而大、小王可以看成任意数字。
```c++
namespace ContinousCards {
int compare(const void* arg1, const void* arg2);
bool isContinuous(int* numbers, int length);
}

// 给快速排序比较大小规则用的函数
int ContinousCards::compare(const void* arg1, const void* arg2) {
    return *(int*)arg1 - *(int*)arg2;
}

bool ContinousCards::isContinuous(int* numbers, int length) {
    if (numbers == nullptr || length < 1) {
        return false;
    }
    
    // 数组快速排序
    qsort(numbers, length, sizeof(int), compare);
    
    int numberOfZero = 0;
    int numberOfGap = 0;
    
    // 统计数组中 0 的个数
    for (int i = 0; i < length && numbers[i] == 0; ++i) {
        ++numberOfZero;
    }
    
    // 统计数组中的间隔数目
    
    // 因为数组已经是排过序的了，所以数组中如果有 0 的话一定是位于数组前面的。
    
    // 所以这里 numbers[small] 是数组中第一个非零的数字，
    // numbers[big] 则是第二个数字，
    // 即 numbers[small] 和 numbers[big] 是一前一后两个数字
    int small = numberOfZero;
    int big = small + 1;
    
    while (big < length) {
        // 两个数相等，有对子，不可能是顺子
        if (numbers[small] == numbers[big]) {
            return false;
        }
        
        // 统计已排序数组相邻两个数字之间的距离
        numberOfGap += numbers[big] - numbers[small] - 1;
        
        // small 和 big 都前进一步
        small = big;
        big++;
    }
    
    // 如果 0 的个数小于间隔的数目，则无法构成顺子，否则可以构成顺子
    return (numberOfGap > numberOfZero) ? false: true;
}
```
## 面试题 62:圆圈中最后剩下的数字
&emsp;题目：0, 1, ⋯, n-1 这 n 个数字排成一个圆圈，从数字 0 开始每次从这个圆圈里删除第 m 个数字。求出这个圆圈里剩下的最后一个数字。
```c++
namespace LastNumberInCircle {
int lastRemaining_Solution1(unsigned int n, unsigned int m);
int lastRemaining_Solution2(unsigned int n, unsigned int m);
}

// 使用链表表示圆圈
int LastNumberInCircle::lastRemaining_Solution1(unsigned int n, unsigned int m) {
    // 入参判断
    if (n < 1 || m < 1) {
        return -1;
    }
    
    // 把 [0, n - 1] 这 n 个数加入链表中
    unsigned int i = 0;
    list<int> numbers;
    for (i = 0; i < n; ++i) {
        numbers.push_back(i);
    }
    
    //迭代器
    list<int>::iterator current = numbers.begin();
    
    // while 循环，直到 numbers 剩最后一个节点
    while (numbers.size() > 1) {
        
        for (int i = 1; i < m; ++i) {
            ++current;
            
            // 当遍历到 numbers 结尾后，调转迭代器到链表头部继续遍历
            if (current == numbers.end()) {
                current = numbers.begin();
            }
        }
        
        // next 记录下一个节点
        list<int>::iterator next = ++current;
        
        // 判断 next 是否是链表结尾，如果是则需要转到链表头部
        if (next == numbers.end()) {
            next = numbers.begin();
        }
        
        // current 自减一步，（对应 next 的 ++）
        --current;
        
        // current 擦除
        numbers.erase(current);
        
        // 更新 current
        current = next;
    }
    
    // 返回 current
    return *(current);
}

// 数学的魅力
int LastNumberInCircle::lastRemaining_Solution2(unsigned int n, unsigned int m) {
    if (n < 1 || m < 1) {
        return -1;
    }
    
    int last = 0;
    for (int i = 2; i <= n; ++i) {
        last = (last + m) % i;
    }
    
    return last;
}
```
## 面试题 63:股票的最大利润
&emsp;题目：假设把某股票的价格按照时间先后顺序存储在数组中，请问买卖交易该股票可能获得的利润是多少？例如一只股票在某些时间节点的价格为 {9, 11, 8, 5, 7, 12, 16, 14}。如果我们能在价格为 5 的时候买入并在价格为 16 时卖出，则能收获最大的利润 11。
```c++
namespace MaximalProfit {
int maxDiff(const int* numbers, unsigned length);
}

int MaximalProfit::maxDiff(const int* numbers, unsigned length) {
    // 入参判断，至少有一个买入价格和一个卖出价格
    if (numbers == nullptr || length < 2) {
        return 0;
    }
    
    // 最小值从 0 下标元素开始
    int min = numbers[0];
    // 首先记录 numbers[1] 和 numbers[0] 的差价
    int maxDiff = numbers[1] - min;
    
    for (int i = 2; i < length; ++i) {
        
        // 用于记录买入最小值
        if (numbers[i - 1] < min) {
            min = numbers[i - 1];
        }
        
        // 记录当前价格和最小值的差价
        int currentDiff = numbers[i] - min;
        
        // 判断 maxDiff 记录最大差价
        if (currentDiff > maxDiff) {
            maxDiff = currentDiff;
        }
    }
    
    // 返回最大差价
    return maxDiff;
}
```
## 面试题 64:求 1+2+⋯+n
&emsp;题目：求 1+2+⋯+n，要求不能使用乘除法、for、while、if、else、switch、case 等关键字及条件判断语句（A?B:C）。
```c++
namespace Accumulate {
// 方法一: 利用构造函数求解
class Temp {
public:
    Temp() { ++N; Sum += N; }

    static void Reset() { N = 0; Sum = 0; }
    static unsigned int GetSum() { return Sum; }

private:
    static unsigned int N;
    static unsigned int Sum;
};

unsigned int Temp::N = 0;
unsigned int Temp::Sum = 0;

unsigned int Sum_Solution1(unsigned int n) {
    Temp::Reset();

    Temp *a = new Temp[n];
    delete [] a;
    a = NULL;

    return Temp::GetSum();
}

// 方法二: 利用虚函数求解
class A;
A* Array[2];

class A {
public:
    virtual unsigned int Sum (unsigned int n) {
        return 0;
    }
};

class B: public A {
public:
    virtual unsigned int Sum (unsigned int n) {
        return Array[!!n]->Sum(n-1) + n;
    }
};

int Sum_Solution2(int n) {
    A a;
    B b;
    Array[0] = &a;
    Array[1] = &b;

    int value = Array[1]->Sum(n);

    return value;
}

// 方法三: 利用函数指针求解
typedef unsigned int (*fun)(unsigned int);

unsigned int Solution3_Teminator(unsigned int n) {
    return 0;
}

unsigned int Sum_Solution3(unsigned int n) {
    static fun f[2] = {Solution3_Teminator, Sum_Solution3};
    return n + f[!!n](n - 1);
}

// 方法四: 利用模版类型求解
template <unsigned int n> struct Sum_Solution4 {
    enum Value { N = Sum_Solution4<n - 1>::N + n};
};

template <> struct Sum_Solution4<1> {
    enum Value { N = 1};
};

template <> struct Sum_Solution4<0> {
    enum Value { N = 0};
};
}
```
## 面试题 65:不用加减乘除做加法
&emsp;题目：写一个函数，求两个整数之和，要求在函数体内不得使用＋、－、×、÷ 四则运算符号。
```c++
namespace AddTwoNumbers {
int add(int num1, int num2);
}

// 用位运算进行加法操作
int AddTwoNumbers::add(int num1, int num2) {
    int sum, carry;
    do {
        sum = num1 ^ num2; // 异或结果可表示二进制表示逐位相加，但是不做进位
        carry = (num1 & num2) << 1; // carry 则表示要进行进位的位
        
        // 第三步，如果 carray 是 0 的话，不需要进行进位，即 sum 即是 num1 + num2 的结果，
        // 但是如果 carray 不是 0，则要进行 sum 和 carry 相加求和才能最后得 num1 + num2 的结果
        
        // while 条件正是判断是否发生了进位，如果发生了进位则持续进行以位操作所代替的加法操作
        num1 = sum;
        num2 = carry;
    } while (num2 != 0);
    
    return num1;
}
```
## 面试题 66:构建乘积数组
&emsp;题目：给定一个数组 A[0, 1, ⋯, n-1]，请构建一个数组 B[0, 1, ⋯, n-1]，其中 B 中的元素 B[i] =A[0]×A[1]×⋯ ×A[i-1]×A[i+1]×⋯×A[n-1]。不能使用除法。
```c++
void ConstuctArray::buildProductionArray(const vector<double>& input, vector<double>& output) {
    unsigned long length1 = input.size();
    unsigned long length2 = output.size();
    
    if (length1 == length2 && length2 > 1) {
        output[0] = 1;
        for (int i = 1; i < length1; ++i) {
            output[i] = output[i - 1] * input[i - 1];
        }
        
        double temp = 1;
        for (unsigned long i = length1 - 2; i >= 0; --i) {
            temp *= input[i + 1];
            output[i] *= temp;
        }
    }
}
```
## 面试题 67:把字符串转换成整数
&emsp;题目：请你写一个函数 StrToInt，实现把字符串转换成整数这个功能。当然，不能使用 atoi 或者其他类似的库函数。
```c++
namespace StringToInt {
enum States { kValid = 0, kInvalid};
int g_nStatus = kValid;

long long strToIntCore(const char* str, bool minus);
int strToInt(const char* str);
}

long long StringToInt::strToIntCore(const char* digit, bool minus) {
    long long num = 0;
    
    while (*digit != '\0') {
        if (*digit >= '0' && *digit <= '9') {
            
            // 正数或者负数
            int flag = minus ? -1: 1;
            num = num * 10 + flag * (*digit - '0');
            
            // 溢出时的情况
            if ((!minus && num > 0x7FFFFFFF) || (minus && num < (signed int)0x80000000)) {
                num = 0;
                break;
            }
            
            // 指针移动
            digit++;
        } else {
            // 如果有 0 ～ 9 之外的字符则直接 break
            num = 0;
            break;
        }
    }
    
    // digit 没有移动到空字符表示字符串中有非数字字符
    if (*digit == '\0') {
        g_nStatus = kValid;
    }
    
    return num;
}

int StringToInt::strToInt(const char* str) {
    // g_nStatus 标记 str 是否能转化为整数
    g_nStatus = kInvalid;
    
    long long num = 0;
    if (str != nullptr && *str != '\0') {
        
        // minus 表示是负数还是正数
        bool minus = false;
        
        // 前进一步，撇开前面的正号和负号
        if (*str == '+') {
            ++str;
        } else if (*str == '-') {
            ++str;
            
            // 表示当前数字是负数
            minus = true;
        }
        
        // *str 不是空字符，进行字符串到数字的转换
        if (*str != '\0') {
            num = strToIntCore(str, minus);
        }
    }
    
    return (int)num;
}
```
## 面试题 68:树中两个结点的最低公共祖先
&emsp;题目：输入两个树结点，求它们的最低公共祖先。
```c++
bool GetNodePath(const TreeNode* pRoot, const TreeNode* pNode, list<const TreeNode*>& path)
{
    if(pRoot == pNode)
        return true;
 
    path.push_back(pRoot);
 
    bool found = false;

    vector<TreeNode*>::const_iterator i = pRoot->m_vChildren.begin();
    while(!found && i < pRoot->m_vChildren.end())
    {
        found = GetNodePath(*i, pNode, path);
        ++i;
    }
 
    if(!found)
        path.pop_back();
 
    return found;
}

const TreeNode* GetLastCommonNode
(
    const list<const TreeNode*>& path1, 
    const list<const TreeNode*>& path2
)
{
    list<const TreeNode*>::const_iterator iterator1 = path1.begin();
    list<const TreeNode*>::const_iterator iterator2 = path2.begin();
    
    const TreeNode* pLast = nullptr;
 
    while(iterator1 != path1.end() && iterator2 != path2.end())
    {
        if(*iterator1 == *iterator2)
            pLast = *iterator1;
 
        iterator1++;
        iterator2++;
    }
 
    return pLast;
}

const TreeNode* GetLastCommonParent(const TreeNode* pRoot, const TreeNode* pNode1, const TreeNode* pNode2)
{
    if(pRoot == nullptr || pNode1 == nullptr || pNode2 == nullptr)
        return nullptr;
 
    list<const TreeNode*> path1;
    GetNodePath(pRoot, pNode1, path1);
 
    list<const TreeNode*> path2;
    GetNodePath(pRoot, pNode2, path2);
 
    return GetLastCommonNode(path1, path2);
}
```
## 完结撒花🎉🎉，感谢陪伴！

## 参考链接
**参考链接:🔗**
+ [白话经典算法系列之九 从归并排序到数列的逆序数对（微软笔试题）](https://blog.csdn.net/MoreWindows/article/details/8029996)

