# 《剑指 Offer》面试题十一～面试题二十的总结

> &emsp;上一篇是 1～10 题，本篇是 11～20 题。⛽️⛽️

## 面试题 11:旋转数组的最小数字
&emsp;题目：把一个数组最开始的若干个元素搬到数组的末尾，我们称之为数组的旋转。输入一个递增排序的数组的一个旋转，输出旋转数组的最小元素。例如数组 {3, 4, 5, 1, 2} 为 {1, 2, 3, 4, 5} 的一个旋转，该数组的最小值为 1 。
```c++
namespace MinNumberInRotatedArray {
// 开局相关题目：

// 返回 [start, end] 区间内的一个随机数
unsigned int randomInRange(unsigned int start, unsigned int end);

// data 在逻辑上分为两个部分，前一半都是小于某个值后一半都是大于某个值
int partition(int data[], int length, int start, int end);

// 用于交换数组中的两个位置的元素
void swap(int* num1, int* num2);

// 快速排序
void quickSort(int data[], int length, int start, int end);

// 员工年龄排序（计数排序）
void sortAges(int ages[], int length);

int minInorder(int* numbers, int index1, int index2);
int min(int* numbers, int length);
}

// 生成 [start, end] 区间内的一个随机数
unsigned int MinNumberInRotatedArray::randomInRange(unsigned int start, unsigned int end) {
    // 生成 [start, end] 区间内的一个随机数
    int rand = (random() % (end - start + 1)) + start;
    
    return rand;
}

int MinNumberInRotatedArray::partition(int data[], int length, int start, int end) {
    if (data == nullptr || length <= 0 || start < 0 || end >= length) {
        throw std::exception(); // 参数错误
    }
    
    // 生成 [start, end] 区间内的一个随机数
    int index = randomInRange(start, end);
    
    // 把 data 的 index 和 end 处的值做交换 （目的是先把这个分界值放在数组的末尾处）
    swap(&data[index], &data[end]);
    
    // small 是 start 减 1，例如: start 是 0 时，那么 small 是 -1
    int small = start - 1;
    
    // 从 start 到 end 之前一位 的循环遍历
    for (index = start; index < end; ++index) {
        // 这里使用 data[end] 与数组前面的每一位进行比较 
        if (data[index] < data[end]) {
            // 只要是比 data[end] 小的话，small 就 +1
            ++small;
            
            if (small != index) {
                // 如果本次遇到一个比 data[end] 小的值，且 small 和 index 不等，
                // 表示目前 small 处的值大于 data[end]，而 data[index] 小于 data[end]，则 small 与 index 处的值进行交换
                swap(&data[small], &data[index]);
            }
        }
    }
    
    // 最后 small 要前进一步
    ++small;
    
    // 如果此时没有到 end 处，表示目前 small 处的值大于 end 的值，进行交换
    if (small != end) {
        swap(&data[small], &data[end]);
    }
    
    // 返回值是 small，它正是一个分界，左边的值都小于 data[small]，右边的值都大于 data[small]
    return small;
}

// 根据指针进行交换值
void MinNumberInRotatedArray::swap(int* num1, int* num2) {
    int temp = *num1;
    *num1 = *num2;
    *num2 = temp;
}

// 快速排序（还是挖坑 + 分治法好理解）
void MinNumberInRotatedArray::quickSort(int data[], int length, int start, int end) {
    // 递归结束的条件
    if (start == end) {
        return;
    }
    
    // index 正是一个分界，左边的值都小于 data[index]，右边的值都大于 data[index]
    int index = partition(data, length, start, end);
    
    // 递归 左半部分进行排序
    if (index > start) {
        quickSort(data, length, start, index - 1);
    }
    
    // 递归 右半部分进行排序
    if (index < end) {
        quickSort(data, length, index + 1, end);
    }
}

// 员工年龄排序（计数排序）
void MinNumberInRotatedArray::sortAges(int ages[], int length) {
    if (ages == nullptr || length <= 0) {
        return;
    }
    
    // 假设员工的年龄是 [0, 99] 岁
    const int oldestAge = 99;
    
    // 准备一个长度是 100 的数组，用来记录每个年龄的员工的人数
    int timesOfAge[oldestAge + 1];
    int i = 0;
    // 数组初始化为 0 
    for (; i <= oldestAge; ++i) {
        timesOfAge[i] = 0;
    }
    
    // 统计每个年龄的人数
    for (i = 0; i < length; ++i) {
        int age = ages[i];
        
        // 年龄超过 [0, 99] 的范围
        if (age < 0 || age > oldestAge) {
            throw std::exception();
        }
        
        // 统计（timesOfAge 的每个下标就对应一个年龄）
        ++timesOfAge[age];
    }
    
    // 从低到高依次取出每个年龄的人数，然后把每个年龄的连续放在 ages 内   
    int index = 0;
    for (i = 0; i <= oldestAge; ++i) {
        // count 代表年龄是 i 的人数
        int count = timesOfAge[i];
        
        // 然后 ages 数组内连续 count 个位置都是 i 
        while (count > 0) {
            ages[index] = i;
            ++index;
            --count;
        }
    }
}

int MinNumberInRotatedArray::minInorder(int* numbers, int index1, int index2) {
    int result = numbers[index1];
    for (int i = index1 + 1; i <= index2; ++i) {
        if (result > numbers[i]) {
            result = numbers[i];
        }
    }
    
    return result;
}

int MinNumberInRotatedArray::min(int* numbers, int length) {
    if (numbers == nullptr || length <= 0) {
        throw std::exception(); // 参数错误
    }
    
    // 记录数组首位和末位
    int index1 = 0;
    int index2 = length - 1;
    
    // indexMid 初始为 index1（这里对应一种特殊情况，例如数组旋转的若干元素的若干是 0，即没有进行旋转，即 numbers[index1] 就是最小的元素了）
    int indexMid = index1;
    
    //（常规状态下发生旋转后 numbers[0] 是大于等于 numbers[length - 1] 的，如果不是的话表明数组没有进行旋转，可直接跳过 while 循环，执行下面的 return numbers[indexMid]，返回第 0 个元素）
    while (numbers[index1] >= numbers[index2]) {
    
        // index2 和 index1 相邻，表示找到了最小元素
        
        // 这里可以用 [1, 2] 旋转后是 [2, 1]
        // [1, 2, 3] 旋转后 [2, 3, 1] 来理解 "index2 - index1 == 1" 是结束的条件。
        
        if (index2 - index1 == 1) {
            // 放在 indexMid 
            indexMid = index2;
            break;
        }
        
        // 找到 index1 和 index2 的中间值
        indexMid = ((index2 - index1) >> 1) + index1;
        
        //（这里也是一种特殊情况，假如三个位置的值完全相同，则不能使用类似二分查找的思想进行查找，只能从前到后遍历数组找到最小的值）
        if (numbers[index1] == numbers[index2] && numbers[indexMid] == numbers[index1]) {
            return minInorder(numbers, index1, index2);
        }
        
        if (numbers[indexMid] >= numbers[index1]) {
            // 表示最小值在右半部分
            index1 = indexMid;
        } else if (numbers[indexMid] <= numbers[index2]) {
            // 表示最小值在左半部分
            index2 = indexMid;
        }
    }
    
    return numbers[indexMid];
}
```
## 面试题 12:矩阵中的路径
&emsp;题目：请设计一个函数，用来判断在一个矩阵中是否存在一条包含某字符串所有字符的路径。路径可以从矩阵中任意一格开始，每一步可以在矩阵中向左、右、上、下移动一格。如果一条路径经过了矩阵的某一格，那么该路径不能再次进入该格子。例如在下面的 3×4 的矩阵中包含一条字符串 “bfce” 的路径（路径中的字母用下划线标出）。但矩阵中不包含字符串 “abfb” 的路径，因为字符串的第一个字符 b 占据了矩阵中的第一行第二个格子之后，路径不能再次进入这个格子。
> &emsp;A ~B~ T G
>
> &emsp;C ~F~ ~C~ S
>
> &emsp;J D ~E~ H
```c++
namespace StringPathInMatrix {
bool hasPathCore(const char* matrix, int rows, int cols, int row, int col, const char* str, int& pathLength, bool* visited);
bool hasPath(const char* matrix, int rows, int cols, const char* str);
}

bool StringPathInMatrix::hasPathCore(const char* matrix, int rows, int cols, int row, int col, const char* str, int& pathLength, bool* visited) {
    if (str[pathLength] == '\0') {
        // 如果目前前进到 str 的末尾了，则表示在矩阵中找到 str 的完整路径了，返回 true
        return true;
    }
    
    bool hasPath = false;
    
    // 判断 row 是否在 [0, rows) 范围内，判断 col 是否在 [0, cols) 区间内，判断矩阵点是否等于 str 的一个点，判断该点没有被经过
    if (row >= 0 && row < rows && col >= 0 && col < cols && matrix[row * cols + col] == str[pathLength] && !visited[row * cols + col]) {
        
        // 标记前进一步
        ++pathLength;
        // 标记该点已经经过了
        visited[row * cols + col] = true;
        
        // 上面表示找到了路径中的一个点符合，然后下面在四个方向上查找下一个节点
        
        // 判断 row 和 col 的四个方向是否包含路径上的下一个字符
        hasPath = hasPathCore(matrix, rows, cols, row - 1, col, str, pathLength, visited) || hasPathCore(matrix, rows, cols, row, col - 1, str, pathLength, visited) || hasPathCore(matrix, rows, cols, row + 1, col, str, pathLength, visited) || hasPathCore(matrix, rows, cols, row, col + 1, str, pathLength, visited);
        
        // 不包含的话进行回溯
        if (!hasPath) {
            --pathLength;
            visited[row * cols + col] = false;
        }
    }
    
    return hasPath;
}

bool StringPathInMatrix::hasPath(const char* matrix, int rows, int cols, const char* str) {
    // 起始条件判断
    if (matrix == nullptr || rows < 1 || cols < 1 || str == nullptr) {
        return false;
    }
    
    // 准备一个 visited 数组，用来标记矩阵中的点是否已经走过了，不能重复经过。
    bool* visited = new bool[rows * cols];
    // 初始把每个元素都置为 0
    memset(visited, 0, rows * cols);
    
    // 记录目前前进到 str 的哪个字符了
    int pathLength = 0;

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
        
            // 双层循环从 (0, 0) 处开始判断进行，这里的双层循环只是为了找一个起点而已，核心回溯法都在下面的 hasPathCore 函数内部
            if (hasPathCore(matrix, rows, cols, row, col, str, pathLength, visited)) {
                return true;
            }
            
        }
    }
    
    // 释放 visited 内存空间
    delete [] visited;
    
    return false;
}
```
## 面试题 13:机器人的运动范围
&emsp;题目：地上有一个 m 行 n 列的方格。一个机器人从坐标 (0, 0) 的格子开始移动，它每一次可以向左、右、上、下移动一格，但不能进入行坐标和列坐标的数位之和大于 k 的格子。例如，当 k 为 18 时，机器人能够进入方格 (35, 37)，因为 3+5+3+7=18。但它不能进入方格 (35, 38)，因为 3+5+3+8=19。请问该机器人能够到达多少个格子？
```c++
namespace RobotMove {
int movingCoungCore(int threshold, int rows, int cols, int row, int col, bool* visited);
bool check(int threshold, int rows, int cols, int row, int col, bool* visited);
int getDigitSum(int number);
int movingCount(int threshold, int rows, int cols);
}

int RobotMove::movingCoungCore(int threshold, int rows, int cols, int row, int col, bool* visited) {
    int count = 0;
    
    if (check(threshold, rows, cols, row, col, visited)) {
        visited[row * cols + col] = true;
        
        // 统计四个方向
        count = 1 + movingCoungCore(threshold, rows, cols, row - 1, col, visited) + movingCoungCore(threshold, rows, cols, row, col - 1, visited) + movingCoungCore(threshold, rows, cols, row + 1, col, visited) + movingCoungCore(threshold, rows, cols, row, col + 1, visited);
        
    }
    
    return count;
}

// 判断机器人是否可进入这个指定的坐标
bool RobotMove::check(int threshold, int rows, int cols, int row, int col, bool* visited) {
    if (row >= 0 && row < rows && col >= 0 && col < cols && getDigitSum(row) + getDigitSum(col) <= threshold && !visited[row * cols + col]) {
        // 1. row 在 [0, rows) 内，col 在 [0, cols) 内
        // 2. row 和 col 的所有数字之和小于等于 threshold
        // 3. 该坐标从没有进入过
        
        // 返回 true 
        
        return true;
    } else {
        return false;
    }
}

// 计算入参 number 中的所有数字之和
int RobotMove::getDigitSum(int number) {
    int sum = 0;
    
    // 从低位开始依次统计所有数字的和
    while (number > 0) {
        sum += number % 10;
        number /= 10;
    }
    
    return sum;
}

int RobotMove::movingCount(int threshold, int rows, int cols) {
    // 判断入参是否合规
    if (threshold < 0 || rows <= 0 || cols <= 0) {
        return 0;
    }
    
    // 准备一个长度是 rows * cols 的数组，用来标记坐标是否被经过过
    bool* visited = new bool[rows * cols];
    // 数组每个元素初始为 false 
    for (int i = 0; i < rows * cols; ++i) {
        visited[i] = false;
    }
    
    // 从 (0, 0) 开始
    int count = movingCoungCore(threshold, rows, cols, 0, 0, visited);
    
    // 释放 visited 内存空间
    delete [] visited;
    
    // 返回 count
    return count;
}
```
## 面试题 14:剪绳子
&emsp;题目：给你一根长度为 n 绳子，请把绳子剪成 m 段（ m、n 都是整数，n>1 并且 m≥1）。每段的绳子的长度记为 k[0]、k[1]、⋯⋯、k[m]。k[0] * k[1] * ⋯ *k[m] 可能的最大乘积是多少？例如当绳子的长度是 8 时，我们把它剪成长度分别为 2、3、3 的三段，此时得到最大的乘积 18。
```c++
namespace CuttingRope {
// 动态规划
int maxProductAfterCutting_solution1(int length);
// 贪婪算法
int maxProductAfterCutting_solution2(int length);
}

// 条件1: 绳子长度大于 1，（ n > 1 ）
// 条件2: 至少要减 1 刀，（ m >= 1 ）

// 动态规划
int CuttingRope::maxProductAfterCutting_solution1(int length) {
    // 绳子长度小于 2 不符合题目要求，返回 0
    if (length < 2) {
        return 0;
    }
    
    // 绳子长度是 2 时只能剪成两个 1，乘积是 1
    if (length == 2) {
        return 1;
    }
    
    // 绳子长度是 3 时，剪成 1 和 2，乘积最大
    if (length == 3) {
        return 2;
    }
    
    // 准备一个 length + 1 长度的数组，用于记录不同长度的绳子能剪出的乘积的最大值
    int* products = new int[length + 1];
    
    // 绳子长度为 0，1，2，3 时对应的乘积最大值
    products[0] = 0;
    products[1] = 1;
    products[2] = 2;
    products[3] = 3;
    
    int max = 0;
    // i 表示绳子长度，从 4 开始
    for (int i = 4; i <= length; ++i) {
        max = 0;
        
        // j 表示绳子被剪的段数，
        // 这里 j 的界限只需要到 i / 2，因为 j + (i - j) = i ,
        // products[j] * products[i - j] 中，i 和 i - j 从 j = i / 2 以后就重合了
        
        for (int j = 1; j <= i / 2; ++j) {
            
            // 这里从下到上，根据前面的值统计绳子长度变长以后能出现的乘积的最大值
            int product = products[j] * products[i - j];
            
            // 记录最大乘积
            if (max < product) {
                max = product;
            }
            
            // 这里是 i，即统计绳子长度从 4 往后随着长度的增加每个长度的最大乘积保存在 products 数组对应的下标位置
            products[i] = max;
        }
    }
    
    // 最后从 products 数组中，取出 length 的最大乘积
    max = products[length];
    
    // 释放 products 内存空间
    delete [] products;
    
    // 返回 max
    return max;
}

// 贪婪算法，（尽可能多的减出长度是 3 的段，）
int CuttingRope::maxProductAfterCutting_solution2(int length) {
    if (length < 2) {
        return 0;
    }
    if (length == 2) {
        return 1;
    }
    if (length == 3) {
        return 2;
    }
    
    // 尽可能多地减去长度为 3 的绳子段
    int timesOf3 = length / 3;
    
    // 当绳子最后剩下的长度为 4 的时候，不能再剪去长度为 3 的绳子段。
    // 此时更好的办法是把绳子剪成长度为 2 的两段，因为 2 * 2 > 3 * 1.
    if (length - timesOf3 * 3 == 1) {
        // 此时表示绳子长度可减为一个长度是 4 的段和另外 n 个长度是 3 的段。
        
        // 这里把长度是 3 的段减少 1
        timesOf3 -= 1;
    }
    
    // 如果最后可减出一段长度为 4，则这里 timesOf2 会等于 2。
    // 其它情况最后一段长度是 0 1 2 时，timesOf2 则分别是 0 0 1
    
    int timesOf2 = (length - timesOf3 * 3) / 2;
    
    // 然后分别求 3 和 2 的次方的乘积
    return (int) (pow(3, timesOf3)) * (int) (pow(2, timesOf2));
}
```
## 面试题 15:二进制中1的个数
&emsp;题目：请实现一个函数，输入一个整数，输出该数二进制表示中 1 的个数。例如把 9 表示成二进制是 1001，有 2 位是 1。因此如果输入 9，该函数输出 2。
```c++
namespace NumberOf1InBinary {
// 相关题目:
// 用一条语句判断一个整数是不是 2 的整数次方。
// 一个整数如果是 2 的整数次方，那么它的二进制表示中有且只有一位是 1，而其它所有位都是 0。
// ((n - 1) & n) == 0? true: false;

// 输入两个整数 m 和 n，计算需要改变 m 的二进制表示中的多少位才能得到 n。
// 分两步，第一步求这两个数的异或；第二步统计异或结果中 1 的位数。

int numberOf1_Solution1(int n);
int numberOf1_Solution2(int n);
}

// 用 1 分别向左移动 32 位，每次和 n 做与操作，统计 n 的二进制表示中每一位是否是 1
int NumberOf1InBinary::numberOf1_Solution1(int n) {
    int count = 0;
    unsigned int flag = 1;
    
    // 这里 flag 要向右移动 32 位才能结束 while 循环
    
    while (flag) {
        
        // 1 每次向左移动一次就和 n 做一次与操作，判断 n 的该位是否是 1
        if (n & flag) {
            // 统计次数
            ++count;
        }
        
        // 左移
        flag = flag << 1;
    }
    
    // 返回 count 
    return count;
}

int NumberOf1InBinary::numberOf1_Solution2(int n) {
    int count = 0;
    
    // 这里很巧妙，while 循环的次数就是 n 二进制表示中 1 的个数
    while (n) {
        ++count;
        
        // n 减 1 后再和 n 做 与 操作，后面的 1 全部被去掉了
        n = (n - 1) & n;
    }
    
    // 返回 count
    return count;
}
```
## 面试题 16:数值的整数次方
&emsp;题目：实现函数 double Power(double base, int exponent)，求 base 的 exponent 次方。不得使用库函数，同时不需要考虑大数问题。
```c++
namespace CPower {
static bool g_InvalidInput = false;
bool equal(double num1, double num2);
double powerWithUnsignedExponent(double base, unsigned int exponent);
double power(double base, int exponent);
}

// 判断两个 double 值是否相等 ([-0.0000001, 0.0000001])
bool CPower::equal(double num1, double num2) {
    // [-0.0000001, 0.0000001]
    if ((num1 - num2 > -0.0000001) && (num1 - num2 < 0.0000001)) {
        return true;
    } else {
        return false;
    }
}

// 计算数字的正数次幂
double CPower::powerWithUnsignedExponent(double base, unsigned int exponent) {
//    if (exponent == 0) {
//        return 1;
//    }
//
//    if (exponent == 1) {
//        return base;
//    }
//
//    double result = 1.0;

      // 重复 exponent 次乘积运算
//    for (unsigned int i = 1; i <= exponent; ++i) {
//        result *= base;
//    }

//
//    return result;
    
    // 如果幂是 0，则返回 1，任何数的 0 次幂都是 1
    if (exponent == 0) {
        return 1;
    }
    
    // 如果幂是 1，则直接返回 base
    if (exponent == 1) {
        return base;
    }
    
    // 递归
    // 幂值从一半开始即可，例如要算 base 的 6 次幂，只需要知道 base 的 3 次幂，base 的 3 次幂乘以 base 的 3 次幂即得 6 次幂。
    // 
    double result = powerWithUnsignedExponent(base, exponent >> 1);
    result *= result;

    // 当幂值是奇数时要多做一次 *base
    if ((exponent & 0x1) == 1) {
        result *= base;
    }

    return result;
}

double CPower::power(double base, int exponent) {
    // 全局变量表示参数输入是否有效
    g_InvalidInput = false;
    
    // 如果 base 是 0 且 幂值是小于 0，则输入标记输入无效，且返回 0.0
    if (equal(base, 0.0) && exponent < 0) {
        g_InvalidInput = true;
        return 0.0;
    }
    
    // 计算幂值的绝对值
    unsigned int absExponent = (unsigned int)exponent;
    if (exponent < 0) {
        absExponent = (unsigned int)(-exponent);
    }
    
    // 计算幂值为正数时 base 的幂
    double result = powerWithUnsignedExponent(base, absExponent);
    
    // 如果幂为负数，则取倒数
    if (exponent < 0) {
        result = 1.0 / result;
    }
    
    return result;
}
```
## 面试题 17:打印 1 到最大的 n 位数
&emsp;题目：输入数字 n，按顺序打印出从 1 最大的 n 位十进制数。比如输入 3，则打印出 1、2、3 一直到最大的 3 位数即 999。
```c++
namespace Print1ToMaxOfNDigits {
void printNumber(char* number);
bool increment(char* number);
void print1ToMaxOfNDigits_1(int n);
void print1ToMaxOfNDigitsRecursively(char* number, int length, int index);
void print1ToMaxOfNDigits_2(int n);
}

// 考虑到大数问题，数字使用字符串来表示

// 打印数字，这里从第一个非 0 字符开始打印
void Print1ToMaxOfNDigits::printNumber(char* number) {
    // 标记找到第一个非 0 字符
    bool isBeginning0 = true;
    unsigned long nLength = strlen(number);
    
    unsigned long i = 0;
    for (; i < nLength; ++i) {
        if (isBeginning0 && number[i] != '0') {
            isBeginning0 = false;
        }
        
        // 打印
        if (!isBeginning0) {
            printf("%c", number[i]);
        }
    }
    
    printf("\t");
}

// 字符串数值每次加 1，返回值表示是否打印到了最大的 n 位数
bool Print1ToMaxOfNDigits::increment(char* number) {
    // isOverflow 表示是否是最大的 n 位数溢出了
    bool isOverflow = false;
    
    // nTakeOver 表示进位，例如个位数字从 9 增加 1 到 10 了，则要向十位进 1
    //（由于我们是做加法，所以 nTakeOver 只可能为 1 或 0，即有进位时是 1， 无进位时是 0）
    int nTakeOver = 0;
    
    // 字符串长度（不包括 '\0'）
    unsigned long nLength = strlen(number);
    unsigned long i = nLength - 1;
    
    // "001" "002" "003" ... "010" "011" ... "100" "101" "102" ... "999" 
    // 字符串末尾表示十进制的个位
    
    for (; i >= 0; --i) {
        // number[i] - '0' 把字符转化为整数
        // 如果不是个位，则根据进位求和
        int sum = number[i] - '0' + nTakeOver;
        
        // 如果 i 是个位则自增 1
        if (i == nLength - 1) {
            ++sum;
        }
        
        // 大于 10，表示要发生进位了
        if (sum >= 10) {
            // 如果是 0 位求和大于等于 10，表示目前要发生进位了，例如 "999" + 1
            if (i == 0) {
                // 标记要发生溢出了
                isOverflow = true;
                
            } else {
                // 其它情况下，减去 10，得到该位的数值
                sum -= 10;
                
                // 表示进位
                nTakeOver = 1;
                
                // '0' + sum 把数字转为字符
                number[i] = '0' + sum;
            }
        } else {
            // 没有发生进位，正常 +1 后转为字符
            number[i] = '0' + sum;
            
            break;
        }
    }
    
    // 返回 isOverflow，表示目前是否到了最大 n 位数
    return isOverflow;
}

void Print1ToMaxOfNDigits::print1ToMaxOfNDigits_1(int n) {
    // 入参判断
    if (n <= 0) {
        return;
    }
    
    // 准备一个 n+1 长度的字符串表示数字
    char* number = new char[n + 1];
    // 前 n 位置为 0，最后一位放空字符，表示字符串结尾
    memset(number, '0', n);
    number[n] = '\0';
    
    // 循环打印数字
    while (!increment(number)) {
        printNumber(number);
    }
    
    // 释放 number 的内存空间
    delete [] number;
}

// 排列，递归打印（每一位的数字是 0 ～ 9 的任一个数字）
void Print1ToMaxOfNDigits::print1ToMaxOfNDigitsRecursively(char* number, int length, int index) {
    if (index == length - 1) {
        printNumber(number);
        return;
    }
    
    for (int i = 0; i < 10; ++i) {
        number[index + 1] = i + '0';
        print1ToMaxOfNDigitsRecursively(number, length, index + 1);
    }
}

void Print1ToMaxOfNDigits::print1ToMaxOfNDigits_2(int n) {
    if (n <= 0) {
        return;
    }
    
    char* number = new char[n + 1];
    number[n] = '\0';
    
    for (int i = 0; i < 10; ++i) {
        number[0] = i + '0';
        print1ToMaxOfNDigitsRecursively(number, n, 0);
    }
}
```
## 18:(一)在 O(1) 时间删除链表结点
&emsp;题目：给定单向链表的头指针和一个结点指针，定义一个函数在O(1)时间删除该结点。
```c++
namespace DeleteNodeInList {
void deleteNode(ListNode** pListHead, ListNode* pToBeDeleted);
}

// 核心思想是使用要删除节点的 m_pNext 来覆盖要删除的节点，
// 如果要删除的节点是最后一个节点则只能从头遍历链表到最后一个节点前面来删除这最后一个节点，
// 还有一种情况如果要链表仅有头节点，且删除的就是这个头节点的话，需要把入参 *pListHead 指向 nullptr。

void DeleteNodeInList::deleteNode(ListNode** pListHead, ListNode* pToBeDeleted) {
    // 入参判断，pListHead 是头节点指针的指针（因为当要删除的节点是头节点时，要进行置为 nullptr 的操作）
    if (pListHead == nullptr || *pListHead == nullptr || pToBeDeleted == nullptr) {
        return;
    }
    
    if (pToBeDeleted->m_pNext != nullptr) {
        // 1. 如果要删除的节点的 m_pNext 不为 nullptr，则 m_pNext 节点向前移动一步，覆盖要删除的节点
        
        // pNext 是待删除节点的 m_pNext 节点
        ListNode* pNext = pToBeDeleted->m_pNext;
        
        // 把 pNext 的 m_nValue 赋值给待删除节点 pToBeDeleted 的 m_nValue
        pToBeDeleted->m_nValue = pNext->m_nValue;
        // 把 pNext 的 m_pNext 赋值给待删除节点 pToBeDeleted 的 m_pNext
        pToBeDeleted->m_pNext = pNext->m_pNext;
        
        // 释放 pNext 的内存空间
        delete pNext;
        pNext = nullptr;
    } else if (*pListHead == pToBeDeleted) {
        // 2. 链接仅有一个头节点，且待删除的节点就是这个头节点
        
        // 直接释放 pToBeDeleted 的内存空间
        delete pToBeDeleted;
        pToBeDeleted = nullptr;
        
        // 把 *pListHead 指向 nullptr
        *pListHead = nullptr;
    } else {
        // 3. 待删除的节点是尾节点，只能从头遍历链表找到待删除节点的前一个节点 
        ListNode* pNode = *pListHead;
        while (pNode->m_pNext != pToBeDeleted) {
            pNode = pNode->m_pNext;
        }
        
        // m_pNext 指向 nullptr
        pNode->m_pNext = nullptr;
        
        // 释放空间
        delete pToBeDeleted;
        pToBeDeleted = nullptr;
    }
}
```
## 18:(二)删除链表中重复的结点
&emsp;题目：在一个排序的链表中，如何删除重复的结点？
```c++
namespace DeleteDuplicatedNode {
void deleteDuplication(ListNode** pHead);
}

void DeleteDuplicatedNode::deleteDuplication(ListNode** pHead) {
    // 入参判断
    if (pHead == nullptr || *pHead == nullptr) {
        return;
    }
    
    // pPreNode 主要用于表示重复节点的前一个节点
    ListNode* pPreNode = nullptr;
    // 头节点
    ListNode* pNode = *pHead;
    
    while (pNode != nullptr) {
        ListNode* pNext = pNode->m_pNext;
        
        // needDelete 用于表示是否发现了重复的节点需要删除
        bool needDelete = false;
        
        // 如果节点的 m_nValue 和该节点的下个节点的 m_nValue 相等，则表示发现了重复的节点，要进行删除
        if (pNext != nullptr && pNode->m_nValue == pNext->m_nValue) {
            needDelete = true;
        } 
        
        if (!needDelete) {
            // 如果相邻的两个节点没有发生重复则可以稳定前进一个节点了。
            
            // 更新 pPreNode
            pPreNode = pNode;
            // pNode 前进到下一个节点
            pNode = pNode->m_pNext;
        } else {
            // 发现了值重复的节点，需要进行删除
            
            // 记录值，除了相邻的两个节点的重复外，可能连续的几个节点值都是重复的，都要进行删除
            int value = pNode->m_nValue;
            // pToBeDelNode 用于记录待删除的节点
            ListNode* pToBeDelNode = pNode;
            
            // while 循环，删除重复的节点
            while (pToBeDelNode != nullptr && pToBeDelNode->m_nValue == value) {
                pNext = pToBeDelNode->m_pNext;
                
                // 删除节点
                delete pToBeDelNode;
                pToBeDelNode = nullptr;
                
                pToBeDelNode = pNext;
            }
            
            // 这里如果 pPreNode 依然是 nullptr，表示头节点就发生了重复，头节点要被删除了
            if (pPreNode == nullptr) {
                // 更新头节点
                *pHead = pNext;
            } else {
                // 更新 pPreNode 的下个节点指向 pNext
                pPreNode->m_pNext = pNext;
            }
            
            // 更新 pNode 继续进行循环，看链表后面是否还有重复的节点需要删除
            pNode = pNext;
        }
    }
}
```
## 面试题 19:正则表达式匹配
&emsp;题目：请实现一个函数用来匹配包含 '.' 和 '*' 的正则表达式。模式中的字符 '.' 表示任意一个字符，而 '*' 表示它前面的字符可以出现任意次（含 0 次）。在本题中，匹配是指字符串的所有字符匹配整个模式。例如，字符串 "aaa" 与模式 "a.a" 和 "ab*ac*a" 匹配，但与 "aa.a" 及 "ab*a" 均不匹配。
```c++
namespace RegularExpressions {
bool matchCore(const char* str, const char* pattern);
bool match(const char* str, const char* pattern);
}

bool RegularExpressions::matchCore(const char* str, const char* pattern) {
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
        if (*str == *pattern ||(*pattern == '.' && *str != '\0')) {
            // 1): 忽略模式字符串的前两个字符
            // 2): 忽略匹配字符串第一个字符（因为 * 前面的字符串可出现若干次（包括零次））
            // 3): 忽略匹配字符串第一个字符和忽略模式字符串的前两个字符
            return matchCore(str, pattern + 2) || matchCore(str + 1, pattern) || matchCore(str + 1, pattern + 2);
        } else {
            // 忽略模式字符串的前两个字符
            return matchCore(str, pattern + 2);
        }
    } else {
        if (*str == *pattern ||(*pattern == '.' && *str != '\0')) {
            // 第一个字符已经匹配完毕，双方同时前进一步
            return matchCore(str + 1, pattern + 1);
        }
    }
    
    return false;
}

bool RegularExpressions::match(const char* str, const char* pattern) {
    // 入参判断
    if (str == nullptr || pattern == nullptr) {
        return false;
    }
    
    // 核心判断函数
    return matchCore(str, pattern);
}
```
## 面试题 20:表示数值的字符串
&emsp;题目：请实现一个函数用来判断字符串是否表示数值（包括整数和小数）。例如，字符串“+100”、“5e2”、“-123”、“3.1416”及“-1E-16”都表示数值，但“12e”、“1a3.14”、“1.2.3”、“+-5”及“12e+5.4”都不是。
```c++
namespace NumericStrings {
// 每次指针往前扫描遇到不符合的条件则停止扫描，并不是一次扫描完毕...
// 在最后的 return 里面自会判断，是否到达了字符串末尾
// 如果没有到达字符串末尾，则不符合...
bool scanUnsignedInteger(const char** str);
bool scanInteger(const char** str);

bool isNumeric(const char* str);
}

// 判断无符号整数，从字符串起点开始，都是 [0, 9] 中的任意字符
bool NumericStrings::scanUnsignedInteger(const char** str) {
    const char* before = *str;
    while (**str != '\0' && **str >= '0' && **str <= '9') {
        ++(*str);
    }
    
    // 这里是 *str > before，表示 str 至少前进一步才会返回 true
    return *str > before;
}

// 判断有符号整数，即前面包含 + / - 或者都不包含
bool NumericStrings::scanInteger(const char** str) {
    if (**str == '+' || **str == '-') {
        // 如果 str 起点是 +/- 则前进一步
        ++(*str);
    }
    
    // 判断后面是无符号整数
    return scanUnsignedInteger(str);
}

bool NumericStrings::isNumeric(const char* str) {
    if (str == nullptr) {
        return false;
    }
    
    // 1. 判断前面的整数部分可以是 +/- 开头，或者直接省略了符号，
    //    numeric 也可能是 false，比如是从小数点开始的数字就没有整数部分: .10
    bool numeric = scanInteger(&str);
    
    // 2. 到了小数点部分，必须不能包含 +/- 所以使用 scanUnsignedInteger 函数判断，
    //    由于小数点前面的整数部分可有可无，所以后面用的或（ || numeric ）
    if (*str == '.') {
        ++str;
        numeric = scanUnsignedInteger(&str) || numeric;
    }
    
    // 3. 到了指数部分，类似整数部分可以包含 +/- 或者不包含，
    //    后面用的 && numeric，因为在数字表示中前面必须有内容才能出现指数部分
    if (*str == 'e' || *str == 'E') {
        ++str;
        numeric = scanInteger(&str) && numeric;
    }
    
    // 4. numeric 为真，并且字符串到了末尾，才表示该字符串确实表示的是一个数值
    return numeric && *str == '\0';
}
```
## 完结撒花🎉🎉，感谢陪伴！
