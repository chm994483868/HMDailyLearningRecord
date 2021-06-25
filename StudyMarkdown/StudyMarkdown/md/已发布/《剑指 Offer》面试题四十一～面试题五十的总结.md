# 《剑指 Offer》面试题四十一～面试题五十的总结

> &emsp;上一篇是 31～40 题，本篇是 41～50 题。⛽️⛽️

## 面试题 41:数据流中的中位数
&emsp;题目：如何得到一个数据流中的中位数？如果从数据流中读出奇数个数值，那么中位数就是所有数值排序之后位于中间的数值。如果从数据流中读出偶数个数值，那么中位数就是所有数值排序之后中间两个数的平均值。
```c++
template <typename T>
class DynamicArray {
public:
    // 要保证最大堆中的所有数据都要小于最小堆中的数据。
    // 当数据的总数目是偶数时，按照前面的分配规则会把新数据插入最小堆。
    
    // 如果此时这个新的数据比最大堆中的一些数据要小，那该怎么办呢？⬅️⬅️
    // 可以先把这个新的数据插入最大堆，接着把最大堆中最大的数字拿出来插入最小堆。
    // 由于最终插入最小堆的数字是原最大堆中的最大的数字，这样就保证了最小堆中所有数字都大于最大堆中的数字。
    
    void insert(T num) {
        // 判断当前总数目是奇数个还是偶数个
        if (((min.size() + max.size()) & 1) == 0) {
            // 当前是偶数个数据，把新数据插入最小堆
            
            // 如果最大堆有值，且新插入的值小于最大堆的最大值
            if (max.size() > 0 && num < max[0]) {
                // num 入堆
                max.push_back(num);
                // 调整堆化
                push_heap(max.begin(), max.end(), less<T>());
                
                // num 取出，最大堆的最大值
                num = max[0];
                
                // 把最大值出堆
                pop_heap(max.begin(), max.end(), less<T>());
                max.pop_back();
            }
            
            // 把 num 插入最小堆
            min.push_back(num);
            // 堆化
            push_heap(min.begin(), min.end(), greater<T>());
        } else {
            // 同上
            if (min.size() > 0 && min[0] < num) {
                min.push_back(num);
                push_heap(min.begin(), min.end(), greater<T>());
                
                num = min[0];
                
                pop_heap(min.begin(), min.end(), greater<T>());
                min.pop_back();
            }
            
            max.push_back(num);
            push_heap(max.begin(), max.end(), less<T>());
        }
    }
    
    T getMedian() {
        // 总数目个数
        int size = min.size() + max.size();
        
        if (size == 0) {
            throw exception(); // no numbers are available
        }
        
        T median = 0;
        if ((size & 1) == 1) {
            // 如果总数目是奇数，则中位数是最小堆或者最大堆的最值
            median = min[0];
        } else {
            // 如果总数目是偶数，就是最小堆和最大堆的最值的和的一半
            median = (min[0] + max[0]) / 2;
        }
        
        return median;
    }
    
private:
    // 数据在逻辑排列中，max 在左边 min 在右边
    // 根据准备的两个指针，一个指向左边的最大值，一个指向右边部分的最小值
    // 在数据的总数目是偶数时把新数据插入最小堆，否则插入最大堆。
    
    // min 和 max 的数据个数相差不能超过 1，
    // 即要么数目相等，要么相差 1 个元素
    
    // min 作为最小堆使用
    vector<T> min;
    // max 作为最大堆使用
    vector<T> max;
};
```
## 面试题 42:连续子数组的最大和
&emsp;题目：输入一个整型数组，数组里有正数也有负数。数组中一个或连续的多个整数组成一个子数组。求所有子数组的和的最大值。要求时间复杂度为 O(n)。
```c++
namespace GreatestSumOfSubarrays {
static bool g_InvalidInput = false;
int findGreatestSumOfSubArray(int* pData, int nLength);
}

// g_InvalidInput 表示入参是否无效，
// 如输入的数组参数为空指针、数组长度小于等于 0 的情况，此时我们让函数返回什么数字呢？
// 如果返回 0，那么又怎么区分子数组的和的最大值是 0 和无效的输入这两种情况呢？
// 因此定义一个全局变量来标记是否输入参数无效。

int GreatestSumOfSubarrays::findGreatestSumOfSubArray(int* pData, int nLength) {
    if (pData == nullptr || nLength <= 0) {
        g_InvalidInput = true;
        return 0;
    }
    
    g_InvalidInput = false;
    
    // 记录当前的和
    int nCurSum = 0;
    
    // 用于记录出现过的最大的和（这里由最大的负数开始）
    int nGreatestSum = 0x80000000;
    
    // 遍历开始
    for (unsigned int i = 0; i < nLength; ++i) {
        
        // 首先判断当前的和是否小于 0，如果小于 0 则前面的几个连续数字的和可以直接抛弃了，
        // 并直接从当前的 pData[i] 开始从新记录 nCurSum 的值
        if (nCurSum <= 0) {
            nCurSum = pData[i];
        } else {
            // 如果 nCurSum 大于 0，则继续累加当前的 pData[i]
            nCurSum += pData[i];
        }
        
        // 判断当前的和是否是出现过的最大和 nGreatestSum
        if (nCurSum > nGreatestSum) {
            nGreatestSum = nCurSum;
        }
    }
    
    // 返回最大的和
    return nGreatestSum;
}
```
## 43:从1到n整数中1出现的次数
&emsp;题目：输入一个整数 n，求从 1 到 n 这 n 个整数的十进制表示中1出现的次数。例如输入 12，从 1 到 12 这些整数中包含 1 的数字有 1，10，11 和 12，1 一共出现了 5 次。
```c++
namespace NumberOf1 {
int numberOf1(unsigned int n);
int numberOf1Between1AndN_Solution1(unsigned int n);

int numberOf1(const char* strN);
int powerBase10(unsigned int n);
int numberOf1Between1AndN_Solution2(int n);
}

int NumberOf1::numberOf1(unsigned int n) {
    int number = 0;
    
    // 从个位开始，判断 1 出现的次数
    while (n) {
        
        // 取模判断个位数字是否是 1
        if (n % 10 == 1) {
            ++number;
        }
        
        // 取商前进，撇弃个位
        n /= 10;
    }
    
    return number;
}

// O(nlogn) 时间复杂度的解法
int NumberOf1::numberOf1Between1AndN_Solution1(unsigned int n) {
    int number = 0;
    // 从 1 遍历 到 n，然后统计这 n 个数字中 1 出现的次数
    for (unsigned int i = 1; i <= n; ++i) {
        // 累加数字中 1 出现的次数
        number += numberOf1(i);
    }
    
    // 返回总次数
    return number;
}

// 找规律求 1 出现的次数
int NumberOf1::numberOf1(const char* strN) {
    // strN 是一个字符指针，判断入参是否合规
    if (strN == nullptr || *strN < '0' || *strN > '9' || *strN == '\0') {
        return 0;
    }
    
    // 取出首字符的数字表示的值
    int first = *strN - '0';
    // 取得字符串长度，（不包含 '\0'）
    unsigned int length = static_cast<unsigned int>(strlen(strN));
    
    // 如果仅一个数字且是数字 0，那肯定返回 0
    if (length == 1 && first == 0) {
        return 0;
    }
    
    // 如果仅一个数字且是大于 0 的数字，那么包含数字 1 的数只有 1
    if (length == 1 && first > 0) {
        return 1;
    }
    
    int numFirstDigit = 0;
    
    // 1 出现在最高位时分两种情况：
    // 1): 1 出现在最高位，且最高位大于 1
    // 2): 1 出现在最高位，且最高位仅为 1
    if (first > 1) {
        // 万位是 2 （21345），在 10000~19999 这 10000 个数字的万位中，一共出现了 10^4 次
        numFirstDigit = powerBase10(length - 1);
    } else if (first == 1) {
        
        // atoi (表示 ascii to integer) 是把字符串转换成整型数的一个函数。
        // int atoi(const char *nptr) 函数会扫描参数 nptr字符串，
        // 会跳过前面的空白字符（例如空格，tab缩进）等。
        // 如果 nptr 不能转换成 int 或者 nptr 为空字符串，那么将返回 0 [1]。
        // 特别注意，该函数要求被转换的字符串是按十进制数理解的。
        // atoi输入的字符串对应数字存在大小限制（与 int 类型大小有关），若其过大可能报错-1。
        
        // 万位是 1（12345），在 10000~12345，出现的次数则是 12345 - 10000 + 1
        numFirstDigit = atoi(strN + 1) + 1;
    }
    
    // numOtherDigits 是 1346~21345 除第一位之外的数位中的数目
    int numOtherDigits = first * (length - 1) * powerBase10(length - 2);
    
    // numRecursive 是 1~1345 中的数目
    int numRecursive = numberOf1(strN + 1);
    
    return numFirstDigit + numOtherDigits + numRecursive;
}

// 10 的次方
int NumberOf1::powerBase10(unsigned int n) {
    int result = 1;
    for (unsigned int i = 0; i < n; ++i) {
        result *= 10;
    }
    return result;
}

int NumberOf1::numberOf1Between1AndN_Solution2(int n) {
    if (n <= 0) {
        return 0;
    }
    
    // 用于输入数字字符串
    char strN[50];
    sprintf(strN, "%d", n);
    
    return numberOf1(strN);
}
```
## 44:数字序列中某一位的数字
&emsp;题目：数字以 0123456789101112131415⋯ 的格式序列化到一个字符序列中。在这个序列中，第 5 位（从 0 开始计数）是 5，第 13 位是 1，第 19 位是 4，等等。请写一个函数求任意位对应的数字。
```c++
namespace DigitsInSequence {
int countOfIntegers(int digits);
int digitAtIndex(int index, int digits);
int beginNumber(int digits);

int digitAtIndex(int index);
}

// 取得 digits 位的数字总共有多少个
// 例如：输入 2 返回两位数（10～99）的个数是 90
// 输入 3 返回三位数（100～999）的个数是 900
int DigitsInSequence::countOfIntegers(int digits) {
    if (digits == 1) {
        return 10;
    }
    
    int count = (int)pow(10, digits - 1);
    return 9 * count;
}

// 当我们知道要找的那一位数字位于某 m 位数之中后，可以用如下函数找出那一位数字
// 如第 881 是某个三位数中的一位，由于 881 = 270 * 3 + 1，
// 意味着第 881 位是从 100 开始的第 270 个数字即 370 的中间一位，也就是 7
int DigitsInSequence::digitAtIndex(int index, int digits) {
    // 对应上面的情况：
    // beginNumber(digits) 是 100
    // index / digits 是 270
    // index % digits 是 1
    // digits - index % digits 表示要找的这个数字在 digits 位数中从右边是下标第几位，
    // 如 370 中 7 从右边开始算是下标为 1 的数字
    int number = beginNumber(digits) + index / digits;
    int indexFromRight = digits - index % digits;
    for (int i = 1; i < indexFromRight; ++i) {
        // 依次缩小，撇掉低位
        number /= 10;
    }
    
    // 取模求末位的数字
    return number % 10;
}

// 取得 digits 位的数的第一个数字，例如，第一个两位数是 10，第一个三位数是 100
int DigitsInSequence::beginNumber(int digits) {
    // 第一个一位数是 0，其它的情况下都是 10 的 digits - 1 次方，
    // 两位 10^1
    // 三位 10^2
    // 四位 10^3
    if (digits == 1) {
        return 0;
    }
    
    return (int)pow(10, digits - 1);
}

int DigitsInSequence::digitAtIndex(int index) {
    // 如果下标小于 0 则返回 -1
    if (index < 0) {
        return -1;
    }
    
    int digits = 1;
    while (true) {
        // 统计不同位数数字的总数，从 1 位数开始，
        // 如 1 位数的个数是 10，2 位数的个数是 90，3 位数的个数 900 总共可包含 900 + 90 + 10
        int number = countOfIntegers(digits);
        
        // 这里是确定 index 具体是位于哪个位数的数字字中，例如 1001 是三位数中的一个数字
        if (index < number * digits) {
            // 找到具体的那一位是数字是啥
            return digitAtIndex(index, digits);
        }
        
        // 从 1 位数的的个数开始减去前面的数字
        index -= number * digits;
        
        // digits 表示数字位数，从 1 开始一直累加
        ++digits;
    }
    
    // 未找到则返回 -1
    return -1;
}
```
## 面试题 45:把数组排成最小的数
&emsp;题目：输入一个正整数数组，把数组里所有数字拼接起来排成一个数，打印能拼接出的所有数字中最小的一个。例如输入数组 {3, 32, 321}，则打印出这 3 个数字能排成的最小数字 321323。
```c++
namespace SortArrayForMinNumber {
int compare(const void* strNumber1, const void* strNumber2);
// int 型整数用十进制表示最多只有 10 位
const int g_MaxNumberLength = 10;
char* g_StrCombine1 = new char[g_MaxNumberLength * 2 + 1];
char* g_StrCombine2 = new char[g_MaxNumberLength * 2 + 1];
void PrintMinNumber(const int* numbers, int length);
}

void SortArrayForMinNumber::PrintMinNumber(const int* numbers, int length) {
    if (numbers == nullptr || length <= 0) {
        return;
    }
    
    // 元素是 char** 的数组
    char** strNumbers = (char**)(new int[length]);
    // 把 numbers 中的内容放入 strNumbers 中
    for (int i = 0; i < length; ++i) {
        strNumbers[i] = new char[g_MaxNumberLength + 1];
        sprintf(strNumbers[i], "%d", numbers[i]);
    }
    
    // 根据 compare 函数排序，如 mn < nm，则 m 就排在 n 前面，
    qsort(strNumbers, length, sizeof(char*), compare);
    
    // 打印 strNumbers，即排好序的数字
    for (int i = 0; i < length; ++i) {
        printf("%s", strNumbers[i]);
    }
    printf("\n");
    
    // 释放内存
    for (int i = 0; i < length; ++i) {
        delete [] strNumbers[i];
    }
    
    delete [] strNumbers;
}

// 如果[strNumber1][strNumber2] > [strNumber2][strNumber1], 返回值大于0
// 如果[strNumber1][strNumber2] = [strNumber2][strNumber1], 返回值等于0
// 如果[strNumber1][strNumber2] < [strNumber2][strNumber1], 返回值小于0
int SortArrayForMinNumber::compare(const void* strNumber1, const void* strNumber2) {
    // [strNumber1][strNumber2]
    strcpy(g_StrCombine1, *(const char**)g_StrCombine1);
    strcat(g_StrCombine1, *(const char**)g_StrCombine2);
    
    // [strNumber2][strNumber1]
    strcpy(g_StrCombine2, *(const char**)g_StrCombine2);
    strcat(g_StrCombine2, *(const char**)g_StrCombine1);
    
    return strcmp(g_StrCombine1, g_StrCombine2);
}
```
## 面试题 46:把数字翻译成字符串
&emsp;题目：给定一个数字，我们按照如下规则把它翻译为字符串：0 翻译成 "a"，1 翻译成 "b"，⋯⋯，11 翻译成 "l"，⋯⋯，25 翻译成 "z"。一个数字可能有多个翻译。例如 12258 有 5 种不同的翻译，它们分别是 "bccfi"、"bwfi"、"bczi"、"mcfi" 和 "mzi"。请编程实现一个函数用来计算一个数字有多少种不同的翻译方法。
```c++
namespace TranslateNumbersToStrings {
int getTranslationCount(const string& number);
int getTranslationCount(int number);
}

// f(i) = f(i+1) + g(i, i + 1) * f(i + 2) g(i, i + 1) 的值为 0 或者 为 1
// 这里如每个数字只能翻译为一个字符，例如 666 那就仅有一种翻译方式就是 fff，当前后两个数字能连在一起且值在 [0, 25] 范围内时，
// 就会出现不同的翻译方式。如 123 可以 1 2 3 abc 也可以 1 23 a x 也可以 12 3 l c。
int TranslateNumbersToStrings::getTranslationCount(const string& number) {
    // string 长度
    unsigned long length = number.length();
    
    // int 数组
    int* counts = new int[length];
    
    // count 用来统计总共有多少种不同的翻译方法
    int count = 0;
    for (unsigned long i = length - 1; i >= 0; --i) {
        count = 0;
        
        // 从倒数数字开始，先取前一个数字有多少种转换方式
        if (i < length - 1) {
            count = counts[i + 1];
        } else {
            count = 1;
        }
        
        // 主要进行判断相邻的两个数字是否能一起转换
        if (i < length - 1) {
            // number[i] 字符转数字
            int digit1 = number[i] - '0';
            // number[i + 1] 字符转数字
            int digit2 = number[i + 1] - '0';
            
            // 把两个数字拼在一起，
            // 如果在 [0, 25] 的范围内则两者可以合并转化为一个字符
            int converted = digit1 * 10 + digit2;
            
            // 如果在 [0, 25] 的范围内，则可以多一种转化方式
            if (converted >= 10 && converted <= 25) {
                if (i < length - 2) {
                    count += counts[i + 2];
                } else {
                    // 加 1
                    count += 1;
                }
            }
        }

        // 赋值，用数组 counts 记录数字能翻译的不同方式的数量
        counts[i] = count;
    }

    // 从 length - 1 开始到 0，counts[0] 中记录的是最大的不同的转换方式
    count = counts[0];
    
    // 释放内存
    delete [] counts;

    return count;
}

int TranslateNumbersToStrings::getTranslationCount(int number) {
    if (number < 0) {
        return 0;
    }
    
    // 数字转为字符串
    string numberString = to_string(number);
    
    return getTranslationCount(numberString);
}
```
## 面试题 47:礼物的最大价值
&emsp;题目：在一个 m×n 的棋盘的每一格都放有一个礼物，每个礼物都有一定的价值（价值大于 0）。你可以从棋盘的左上角开始拿格子里的礼物，并每次向左或者向下移动一格直到到达棋盘的右下角。给定一个棋盘及其上面的礼物，请计算你最多能拿到多少价值的礼物？
```c++
namespace MaxValueOfGifts {
int getMaxValue_solution1(const int* values, int rows, int cols);
int getMaxValue_solution2(const int* values, int rows, int cols);
}

int MaxValueOfGifts::getMaxValue_solution1(const int* values, int rows, int cols) {
    // 入参判断
    if (values == nullptr || rows <= 0 || cols <= 0) {
        return 0;
    }
    
    // 准备一个 rows * cols 的矩阵记录到达每个坐标时能拿到的礼物的最大值
    int** maxValues = new int*[rows];
    unsigned int i = 0;
    for (; i < rows; ++i) {
        maxValues[i] = new int[cols];
    }
    
    // 遍历棋盘
    for (i = 0; i < rows; ++i) {
        for (unsigned int j = 0; j < cols; ++j) {
            
            int up = 0;
            int left = 0;
            
            // 上面坐标点的最大值
            if (i > 0) {
                up = maxValues[i - 1][j];
            }
            
            // 左边一个坐标点d
            if (j > 0) {
                left = maxValues[i][j - 1];
            }
            
            // 记录当前最大值
            maxValues[i][j] = std::max(up, left) + values[i * cols + j];
        }
    }
    
    // 右下角的最大值
    int maxValue = maxValues[rows - 1][cols - 1];
    
    // 释放内存
    for (i = 0; i < rows; ++i) {
        delete [] maxValues[i];
    }
    delete [] maxValues;

    // 返回最大值
    return maxValue;
}

int MaxValueOfGifts::getMaxValue_solution2(const int* values, int rows, int cols) {
    // 入参判断
    if (values == nullptr || rows <= 0 || cols <= 0) {
        return 0;
    }
    
    // 仅需要一个一维数组足够保存最大值
    int* maxValues = new int[cols];
    
    // 遍历棋盘
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            
            int left = 0;
            int up = 0;
            
            // 上面的最大值
            if (i > 0) {
                up = maxValues[j];
            }
            
            // 左面的最大值
            if (j > 0) {
                left = maxValues[j - 1];
            }
            
            // 记录当前最大值
            maxValues[j] = std::max(left, up) + values[i * cols + j];
        }
    }

    // 右下角的最大值
    int maxValue = maxValues[cols - 1];
    // 释放内存
    delete [] maxValues;
    // 返回最大值
    return maxValue;
}
```
## 48:最长不含重复字符的子字符串
&emsp;题目：请从字符串中找出一个最长的不包含重复字符的子字符串，计算该最长子字符串的长度。假设字符串中只包含从 'a' 到 'z' 的字符。
```c++
namespace LongestSubstringWithoutDup {
// 方法一：蛮力法
bool hasDuplication(const std::string& str, int position[]);
int longestSubstringWithoutDuplication_1(const std::string& str);
//// 方法二：动态规划
int longestSubstringWithoutDuplication_2(const std::string& str);    
}

// 判断字符是否重复出现
bool LongestSubstringWithoutDup::hasDuplication(const std::string& str, int position[]) {
    for (int i = 0; i < 26; ++i) {
        position[i] = -1;
    }
    
    for (int i = 0; i < str.length(); ++i) {
        int indexInPosition = str[i] - 'a';
        if (position[indexInPosition] >= 0) {
            return true;
        }
        
        position[indexInPosition] = indexInPosition;
    }
    
    return false;
}

int LongestSubstringWithoutDup::longestSubstringWithoutDuplication_1(const std::string& str) {
    int longest = 0;
    int* position = new int[26];
    for (int start = 0; start < str.length(); ++start) {
        for (int end = start; end < str.length(); ++end) {
            int count = end - start + 1;
            const std::string& substring = str.substr(start, count);
            if (!hasDuplication(substring, position)) {
                if (count > longest) {
                    longest = count;
                } else {
                    break;
                }
            }
        }
    }
    
    delete [] position;
    return longest;
}

int LongestSubstringWithoutDup::longestSubstringWithoutDuplication_2(const std::string& str) {
    
    // 记录当前不重复字符串长度
    int curLength = 0;
    
    // 记录最大长度
    int maxLength = 0;
    
    // 准备一个长度是 26 的数组，标记 a 到 z 这 26 个字符上次出现的位置
    int* position = new int[26];
    unsigned int i = 0;
    // 数组元素全部置为 -1
    for (; i < 26; ++i) {
        position[i] = -1;
    }
    
    // 遍历字符串
    for (i = 0; i < str.length(); ++i) {
        
        // 首先从 position 数组中找出该字符上次出现的位置（str[i] - 'a' 作为下标）
        int prevIndex = position[str[i] - 'a'];
        
        // 如果还没出现过或者距离超过了当前 curLength，则 curLength 自增
        if (prevIndex < 0 || i - prevIndex > curLength) {
            ++curLength;
        } else {
            // 否则就是重复出现了，curLength 的值要变小了
            
            // 更新 maxLength
            if (curLength > maxLength) {
                maxLength = curLength;
            }
            
            // 更新当前长度 curLength
            curLength = i - prevIndex;
        }
        
        // 记录字符的出现的位置
        position[str[i] - 'a'] = i;
    }
    
    // 是否更新 maxLength
    if (curLength > maxLength) {
        maxLength = curLength;
    }
    
    // 释放内存
    delete [] position;
    // 返回 maxLength
    return maxLength;
}
```
## 面试题 49:丑数
&emsp;题目：我们把只包含因子2、3和5的数称作丑数（Ugly Number）。求按从小到大的顺序的第1500个丑数。例如6、8都是丑数，但14不是，因为它包含因子7。习惯上我们把1当做第一个丑数。
```c++
namespace UglyNumber {
bool IsUgly(int number);
int GetUglyNumber_Solution1(int index);
int Min(int number1, int number2, int number3);
int GetUglyNumber_Solution2(int index);
}

// 判断一个数字是否是丑数
bool UglyNumber::IsUgly(int number) {
    // 任意丑数对 2/3/5 其中一个取模必是 0，
    // 然后连续取商的话最后必是 1，即最后只需要判断 number 是否是 1 即可。
    while (number % 2 == 0) {
        number /= 2;
    }
    while (number % 3 == 0) {
        number /= 3;
    }
    while (number % 5 == 0) {
        number /= 5;
    }
    
    return (number == 1) ? true: false;
}

// 从 0 开始遍历每一个整数，判断该整数是否是整数，然后记录下是第几个丑数，直到第 index 个丑数
int UglyNumber::GetUglyNumber_Solution1(int index) {
    if (index <= 0) {
        return 0;
    }
    
    int number = 0;
    int uglyFound = 0;
    while (uglyFound < index) {
        ++number;
        
        if (IsUgly(number)) {
            ++uglyFound;
        }
    }
    
    return number;
}

// 求三个数字中最小的数字
int UglyNumber::Min(int number1, int number2, int number3) {
    int min = (number1 < number2) ? number1: number2;
    min = (min < number3) ? min: number3;
    
    return min;
}

// 准备一个 index 长度的数组，按从小到大顺序记录每个丑数，直到 index。
int UglyNumber::GetUglyNumber_Solution2(int index) {
    if (index <= 0) {
        return 0;
    }
    
    // 准备一个 index 长度的数组记录丑数
    int *pUglyNumbers = new int[index];
    // 第一个丑数从 1 开始
    pUglyNumbers[0] = 1;
    // 记录当前是第几个丑数
    int nextUglyIndex = 1;
    
    // 三个指针分别记录当前大于已有丑数乘以 2 3 5 后的最小值
    int* pMultiply2 = pUglyNumbers;
    int* pMultiply3 = pUglyNumbers;
    int* pMultiply5 = pUglyNumbers;
    
    // 循环直到第 index 个丑数
    while (nextUglyIndex < index) {
        // 当前包含 2 3 5 因子的最小丑数
        int min = Min(*pMultiply2 * 2, *pMultiply3 * 3, *pMultiply5 * 5);
        pUglyNumbers[nextUglyIndex] = min;
        
        // 2 => [2, 4, 8, 16, 32, 64, ...]
        // 3 => [3, 6, 9, 12, 15, 18, ...]
        // 5 => [5, 10, 15, 20, 25, 30, ...]
        // 如上，每次从以上三个组找大于当前丑数的最小丑数
        
        // [1, 2, 3, 4, 5, 6, 8, 9, 10, 12, 15, 16, 18, 20, ...]
        // 更新 3 个指针
        while (*pMultiply2 * 2 <= pUglyNumbers[nextUglyIndex]) {
            ++pMultiply2;
        }
        while (*pMultiply3 * 3 <= pUglyNumbers[nextUglyIndex]) {
            ++pMultiply3;
        }
        while (*pMultiply5 * 5 <= pUglyNumbers[nextUglyIndex]) {
            ++pMultiply5;
        }
        
        // 自增 1，直到 index
        ++nextUglyIndex;
    }
    
    // 要找到丑数
    int ugly = pUglyNumbers[nextUglyIndex - 1];
    
    // 释放内存
    delete [] pUglyNumbers;
    
    return ugly;
}
```
## 50:(一)字符串中第一个只出现一次的字符
&emsp;题目：在字符串中找出第一个只出现一次的字符。如输入 "abaccdeff"，则输出 'b'。
```c++
namespace FirstNotRepeatingChar {
char firstNotRepeatingChar(const char* pString);
}

char FirstNotRepeatingChar::firstNotRepeatingChar(const char* pString) {
    if (pString == nullptr) {
        return '\0';
    }
    
    // 准备一个长度是 256 的 int 数组，每个元素初始值为 0。
    // 数组下标对应字符的 ASCII 码 ，每个值则对应该字符出现的次数。
    const int tableSize = 256;
    unsigned int hashTable[tableSize];
    for (unsigned int i = 0; i < tableSize; ++i) {
        hashTable[i] = 0;
    }
    
    // 遍历字符串，记录每个字符出现的次数
    const char* pHashKey = pString;
    while (*(pHashKey) != '\0') {
        hashTable[*(pHashKey++)]++;
    }
    
    // 从 pString 遍历每个字符出现的次数，找到第一个 1 时直接返回即可
    pHashKey = pString;
    while (*pHashKey != '\0') {
        if (hashTable[*pHashKey] == 1) {
            return *pHashKey;
        }
        
        pHashKey++;
    }
    
    return '\0';
}
```
## 50:(二)字符流中第一个只出现一次的字符
&emsp;题目：请实现一个函数用来找出字符流中第一个只出现一次的字符。例如，当从字符流中只读出前两个字符 "go" 时，第一个只出现一次的字符是 'g'。当从该字符流中读出前六个字符 "google" 时，第一个只出现一次的字符是 'l'。
```c++
class CharStatistics {
public:
    // 构造函数
    CharStatistics() : index(0) {
        // 初始化 occurrence 数组，-1 表示字符还没还没有找到
        // occurrence 下标表示字符的 ASCII 码，值表示出现的位置
        for (int i = 0; i < 256; ++i) {
            occurrence[i] = -1;
        }
    }
    
    // 把字符流中字符的 index 记录到 occurrence 中
    void Insert(char ch) {
        if (occurrence[ch] == -1) {
            // 第一次出现 occurrence[ch] 赋值为出现的位置
            occurrence[ch] = index;
        } else if (occurrence[ch] >= 0) {
            // -2 表示该字符已经出现过，重复了
            occurrence[ch] = -2;
        }
        
        index++;
    }
    
    // 第一个只出现一次的字符
    char firstAppearingOnce() {
        char ch = '\0';
        
        // minIndex 值初始为 int 类型数值的最大值
        int minIndex = numeric_limits<int>::max();
        
        // 遍历 occurrence 找到第一个出现一次的字符，该字符的 occurrence[i] 值是它在字符流中的位置
        for (int i = 0; i < 256; ++i) {
            if (occurrence[i] >= 0 && occurrence[i] < minIndex) {
                ch = (char)i;
                minIndex = occurrence[i];
            }
        }
        
        return ch;
    }
    
private:
    // occurrence[i]: A character with ASCII value i; 字符的 ASCII 码
    // occurrence[i] = -1: The character has not found; 未找到
    // occurrence[i] = -2: The character has been found for mutlple times 重复
    // occurrence[i] >= 0: The character has been found only once 出现一次
    int occurrence[256];
    int index;
};
```
## 完结撒花🎉🎉，感谢陪伴！
