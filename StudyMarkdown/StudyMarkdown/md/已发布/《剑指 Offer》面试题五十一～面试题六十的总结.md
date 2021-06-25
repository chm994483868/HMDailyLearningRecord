# 《剑指 Offer》面试题五十一～面试题六十的总结

> &emsp;上一篇是 41～50 题，本篇是 51～60 题。⛽️⛽️

## 面试题 51:数组中的逆序对
&emsp;题目：在数组中的两个数字如果前面一个数字大于后面的数字，则这两个数字组成一个逆序对。输入一个数组，求出这个数组中的逆序对的总数。
```c++
namespace InversePairs {
int InversePairsCore(int* data, int* copy, int start, int end);
int InversePairs(int* data, int length);
}

// 归并排序，合并数组时，如 [1, 3, 5] 和 [2, 4] 合并，
// 3 > 2 即 3 后面的所有数字和 2 都能构成逆序对。
// 归并排序中的摘录：
// if (a[i] <= a[j]) {
//    temp[k++] = a[i++];
// } else {
//    temp[k++] = a[j++];
//    
//    // a[j] 和 前面每一个数都能组成逆序数对 ⬅️⬅️
//    // 这里 a[i] 大于 a[j]，因为 a[i] 后面的数字都是大于 a[i] 的，它们都可以和 a[j] 构成逆序对
//    g_nCount += m - i + 1;
// }

int InversePairs::InversePairs(int* data, int length) {
    // 入参判断
    if (data == nullptr || length <= 0) {
        return 0;
    }
    
    // 把 data 数组中的数据复制到 copy 数组中
    // copy 作为归并排序的辅助数组
    int* copy = new int[length];
    for (int i = 0; i < length; ++i) {
        copy[i] = data[i];
    }
    
    // 核心函数
    int count = InversePairsCore(data, copy, 0, length - 1);
    
    // 释放 copy 的内存
    delete [] copy;
    
    return count;
}

int InversePairs::InversePairsCore(int* data, int* copy, int start, int end) {
    if (start == end) {
        copy[start] = data[start];
        return 0;
    }
    
    // start 与中间值的距离
    int length = (end - start) / 2;
    // 递归拆分数据
    int left = InversePairsCore(copy, data, start, start + length);
    int right = InversePairsCore(copy, data, start + length + 1, end);
    
    // i 初始化为前半段最后一个数字的下标
    int i = start + length;
    // j 初始化为后半段最后一个数字的下标
    int j = end;
    
    int indexCopy = end;
    int count = 0;
    
    // 合并数组
    while (i >= start && j >= start + length + 1) {
        if (data[i] > data[j]) {
            copy[indexCopy--] = data[i--];
            
            // 在这里统计逆序对数，如果 data[i] > data[j]，
            // 则 data[i] 能和 data[j] 后的所有数据组成逆序对，
            // j 后面有 j - (start + length) 个数字
            count += j - start - length;
        } else {
            copy[indexCopy--] = data[j--];
        }
    }
    
    for (; i >= start; --i) {
        copy[indexCopy--] = data[i];
    }
    
    for (; j >= start + length + 1; --j) {
        copy[indexCopy--] = data[j];
    }
    
    return left + right + count;
}
```
## 面试题 52:两个链表的第一个公共结点
&emsp;题目：输入两个链表，找出它们的第一个公共结点。
```c++
namespace FirstCommonNodesInLists {
unsigned int getListLength(ListNode* pHead);
ListNode* findFirstCommonNode(ListNode* pHead1, ListNode* pHead2);
}

// 如果是单链表，那么两个链表相交后后面的所有节点就是共用的了。
// 先分别计算两个链表的长度，然后准备两个指针从两个链表头节点开始向后遍历，
// 首先指向长链表头节点的指针前进长的链表长出来的部分，然后两个指针一起前进，直到两个指针指向相同时，
// 即是两个链表的第一个交点
ListNode* FirstCommonNodesInLists::findFirstCommonNode(ListNode* pHead1, ListNode* pHead2) {
    // 得到两个链表的长度
    unsigned int nLength1 = getListLength(pHead1);
    unsigned int nLength2 = getListLength(pHead2);
    int nLengthDif = nLength1 - nLength2;
    
    // 指向两个链表头节点的指针
    ListNode* pListHeadLong = pHead1;
    ListNode* pListHeadShort = pHead2;
    
    // 确保 pListHeadLong 指向长链表头节点，
    // pListHeadShort 指向短链表头节点
    if (nLength2 > nLength1) {
        pListHeadLong = pHead2;
        pListHeadShort = pHead1;
        nLengthDif = nLength2 - nLength1;
    }
    
    // 先在长链表上走几步，再同时在两个链表上遍历
    for (int i = 0; i < nLengthDif; ++i) {
        pListHeadLong = pListHeadLong->m_pNext;
    }
    
    while ((pListHeadLong != nullptr) && (pListHeadShort != nullptr) && (pListHeadLong != pListHeadShort)) {
        pListHeadLong = pListHeadLong->m_pNext;
        pListHeadShort = pListHeadShort->m_pNext;
    }
    
    // 得到第一个公共节点
    ListNode* pFirstCommonNode = pListHeadLong;
    
    return pFirstCommonNode;
}

// 统计链表节点个数
unsigned int FirstCommonNodesInLists::getListLength(ListNode* pHead) {
    unsigned int nLength = 0;
    ListNode* pNode = pHead;
    while(pNode != nullptr) {
        ++nLength;
        pNode = pNode->m_pNext;
    }
    
    return nLength;
}
```
## 53:(一)数字在排序数组中出现的次数
&emsp;题目：统计一个数字在排序数组中出现的次数。例如输入排序数组 {1, 2, 3, 3, 3, 3, 4, 5} 和数字 3，由于 3 在这个数组中出现了 4 次，因此输出 4。
```c++
namespace NumberOfK {
int getFirstK(const int* data, int length, int k, int start, int end);
int getLastK(const int* data, int length, int k, int start, int end);
int getNumberOfK(const int* data, int length, int k);
}

// 找到数组中第一个 k 的下标，如果数组中不存在 k，返回 -1
int NumberOfK::getFirstK(const int* data, int length, int k, int start, int end) {
    if (start > end) {
        return -1;
    }
    
    // 数组中间下标
    int middleIndex = (start + end) / 2;
    // 数组中间值
    int middleData = data[middleIndex];
    
    if (middleData == k) {
        // 如果中间值正是 k
        if ((middleIndex > 0 && data[middleIndex - 1] != k) || middleIndex == 0) {
            // 如果中间下标大于 0，且中间下标前面一个下标的值不是 k，或者中间下标是 0，表示当前的 k 已经是数组的第一个 k
            return middleIndex;
        } else {
            // 否则当前 k 不是第一个 k，第一个 k 在左边，所以更新 end
            end = middleIndex - 1;
        }
    } else if (middleData > k) {
        // 如果中间值大于 k，表示第一个 k 在左边，同样更新 end
        end = middleIndex - 1;
    } else {
        // 如果小于 k，则表示第一个 k 在右边，则更新 start
        start = middleIndex + 1;
    }
    
    // 递归查找第一个 k
    return getFirstK(data, length, k, start, end);
}

// 同上
// 找到数组中最后一个 k 的下标。如果数组中不存在 k，返回 -1
int NumberOfK::getLastK(const int* data, int length, int k, int start, int end) {
    if (start > end) {
        return -1;
    }
    
    int middleIndex = (start + end) / 2;
    int middleData = data[middleIndex];
    
    if (middleData == k) {
        if ((middleIndex < length - 1 && data[middleIndex + 1] != k) || middleIndex == length - 1) {
            return middleIndex;
        } else {
            start = middleIndex + 1;
        }
    } else if (middleData < k) {
        start = middleIndex + 1;
    } else {
        end = middleIndex - 1;
    }
    
    return getLastK(data, length, k, start, end);
}

int NumberOfK::getNumberOfK(const int* data, int length, int k) {
    int number = 0;
    
    if (data != nullptr && length > 0) {
        // 找到第一个 k 的位置，找到最后一个 k 的位置
        int first = getFirstK(data, length, k, 0, length - 1);
        int last = getLastK(data, length, k, 0, length - 1);
        
        // 两者的距离，即是 k 重复的次数
        if (first > - 1 && last > -1) {
            number = last - first + 1;
        }
    }
    
    // 返回重复次数
    return number;
}
```
## 53:(二)0 到 n-1 中缺失的数字
&emsp;题目：一个长度为 n-1 的递增排序数组中的所有数字都是唯一的，并且每个数字都在范围 0 到 n-1 之内。在范围 0 到 n-1 的 n 个数字中有且只有一个数字不在该数组中，请找出这个数字。
```c++
namespace MissingNumber {
int getMissingNumber(const int* numbers, int length);
}

// 只有一个数字不在该数组中，即从该数字开始数字和下标和值是不相等的了，
// 如: [0, 1, 2, 4, 5, 6] 4 位于下标 3 处，则自 3 下标以后，下标值和数组值都不再相等了。
int MissingNumber::getMissingNumber(const int* numbers, int length) {
    if (numbers == nullptr || length <= 0) {
        return -1;
    }
    
    int left = 0;
    int right = length - 1;
    
    while (left <= right) {
        // 中间下标
        int middle = (right + left) >> 1;
        
        if (numbers[middle] != middle) {
            // 如果中间下标值和数组值不相等
            if (middle == 0 || numbers[middle - 1] == middle - 1) {
                // 如果中间下标是 0 下标，或者 middle - 1 下标等于 numbers[middle - 1]，
                // 即表示 middle 便是第一个和自己下标不相等的值，直接返回
                return middle;
            }
            
            // 如果不是的话，表示第一个与自己下标不等的值在左边，缩小 right
            right = middle - 1;
        } else {
            // 否则在右边，增大 left
            left = middle + 1;
        }
    }
    
    // 一直到最右边，则返回 length
    if (left == length) {
        return length;
    }
    
    // 无效的输入，比如数组不是按要求排序的，
    // 或者有数字不在 0 到 n-1 范围之内
    return -1;
}
```
## 53:(三)数组中数值和下标相等的元素
&emsp;题目：假设一个单调递增的数组里的每个元素都是整数并且是唯一的。请编程实现一个函数找出数组中任意一个数值等于其下标的元素。例如，在数组 {-3, -1, 1, 3, 5} 中，数字 3 和它的下标相等。
```c++
namespace IntegerIdenticalToIndex {
int getNumberSameAsIndex(const int* numbers, int length);
}

// 上面一题是找第一个值和下标不等的值，这一题则是找第一个值和下标相等的值
int IntegerIdenticalToIndex::getNumberSameAsIndex(const int* numbers, int length) {
    // 入参判断
    if (numbers == nullptr || length <= 0) {
        return -1;
    }
    
    int left = 0;
    int right = length - 1;
    
    while (left <= right) {
        // 中间下标
        int middle = left + ((right - left) >> 1);
        
        // 如果中间值相等，则直接返回
        if (numbers[middle] == middle) {
            return middle;
        }
        
        // 如果中间值大于 middle，则表示在左边，缩小 right
        if (numbers[middle] > middle) {
            right = middle - 1;
        } else {
            // 如果大于，则表示在右边，增大 left
            left = middle + 1;
        }
    }
    
    return -1;
}
```
## 面试题 54:二叉搜索树的第 k 个结点
&emsp;题目：给定一棵二叉搜索树，请找出其中的第 k 大的结点。
```c++
namespace KthNodeInBST {
const BinaryTreeNode* kthNodeCore(const BinaryTreeNode* pRoot, unsigned int& k);
const BinaryTreeNode* kthNode(const BinaryTreeNode* pRoot, unsigned int k);
}

// 后序遍历二叉搜索树，既是从小到大递增的有序序列
const BinaryTreeNode* KthNodeInBST::kthNode(const BinaryTreeNode* pRoot, unsigned int k) {
    if (pRoot == nullptr || k <= 0) {
        return nullptr;
    }
    
    return kthNodeCore(pRoot, k);
}

const BinaryTreeNode* KthNodeInBST::kthNodeCore(const BinaryTreeNode* pRoot, unsigned int& k) {
    const BinaryTreeNode* target = nullptr;
    
    // 递归左子树或者左节点
    if (pRoot->m_pLeft != nullptr) {
        target = kthNodeCore(pRoot->m_pLeft, k);
    }
    
    // k 每次减 1
    if (target == nullptr) {
        // 当 k 减到 1 后的节点，就是第 k 个节点
        if (k == 1) {
            target = pRoot;
            
            // 疑问：不用在这里写个 return 而结束递归吗？
        }
        
        k--;
    }
    
    // 递归右子树或者右节点
    if (target == nullptr && pRoot->m_pRight != nullptr) {
        target = kthNodeCore(pRoot->m_pRight, k);
    }
    
    // 返回 target
    return target;
}
```
## 55:(一)二叉树的深度
&emsp;题目：输入一棵二叉树的根结点，求该树的深度。从根结点到叶结点依次经过的结点（含根、叶结点）形成树的一条路径，最长路径的长度为树的深度。
```c++
namespace TreeDepth {
int treeDepth(const BinaryTreeNode* pRoot);
}

int TreeDepth::treeDepth(const BinaryTreeNode* pRoot) {
    if (pRoot == nullptr) {
        return 0;
    }
    
    // 递归左右子树
    int nLeft = treeDepth(pRoot->m_pLeft);
    int nRight = treeDepth(pRoot->m_pRight);
    
    return (nLeft > nRight) ? (nLeft + 1) : (nRight + 1);
}
```
## 55:(二)平衡二叉树
&emsp;题目：输入一棵二叉树的根结点，判断该树是不是平衡二叉树。如果某二叉树中任意结点的左右子树的深度相差不超过1，那么它就是一棵平衡二叉树。
```c++
namespace BalancedBinaryTree {
int treeDepth(const BinaryTreeNode* pRoot);
bool isBalanced_Solution1(const BinaryTreeNode* pRoot);

bool isBalanced(const BinaryTreeNode* pRoot, int* pDepth);
bool isBalanced_Solution2(const BinaryTreeNode* pRoot);
}

int BalancedBinaryTree::treeDepth(const BinaryTreeNode* pRoot) {
    if (pRoot == nullptr) {
        return 0;
    }
    
    int nLeft = treeDepth(pRoot->m_pLeft);
    int nRight = treeDepth(pRoot->m_pRight);
    
    return (nLeft > nRight) ? (nLeft + 1): (nRight + 1);
}

// 平衡二叉树：任意结点的左右子树的深度相差不超过 1
// 方法 1，从根节点开始，会判断以每个节点为根节点的二叉树是否是平衡二叉树，存在大量的重复计算
bool BalancedBinaryTree::isBalanced_Solution1(const BinaryTreeNode* pRoot) {
    if (pRoot == nullptr) {
        return true;
    }
    
    int left = treeDepth(pRoot->m_pLeft);
    int right = treeDepth(pRoot->m_pRight);
    
    int diff = left - right;
    
    // 如果超过 1 则不是平衡二叉树
    if (diff > 1 || diff < -1) {
        return false;
    }
    
    return isBalanced_Solution1(pRoot->m_pLeft) && isBalanced_Solution1(pRoot->m_pRight);
}

bool BalancedBinaryTree::isBalanced_Solution2(const BinaryTreeNode* pRoot) {
    int depth = 0;
    return isBalanced(pRoot, &depth);
}

// 用后序遍历的方式遍历二叉树的每个节点，在遍历到一个节点之前就已经遍历了它的左右子树，
// 在遍历每个节点的时候记录它的深度（某一节点的深度等于它到叶节点的路径的长度），
// 可以一边遍历一边判断每个节点是不是平衡的
bool BalancedBinaryTree::isBalanced(const BinaryTreeNode* pRoot, int* pDepth) {
    if (pRoot == nullptr) {
        *pDepth = 0;
        return true;
    }
    
    int left, right;
    
    if (isBalanced(pRoot->m_pLeft, &left) && isBalanced(pRoot->m_pRight, &right)) {
        int diff = left - right;
        
        if (diff <= 1 && diff >= -1) {
            *pDepth = 1 + (left > right? left: right);
            return true;
        }
    }
    
    return false;
}
```
## 56:(一)数组中只出现一次的两个数字
&emsp;题目：一个整型数组里除了两个数字之外，其他的数字都出现了两次。请写程序找出这两个只出现一次的数字。要求时间复杂度是 O(n)，空间复杂度是 O(1)。
```c++
namespace NumbersAppearOnce {
unsigned int findFirstBitIs1(int num);
bool isBit1(int num, unsigned int indexBit);
void findNumsAppearOnce(int data[], int length, int* num1, int* num2);
}

// 判断数字 num 的第 indexBit 位是不是 1
bool NumbersAppearOnce::isBit1(int num, unsigned int indexBit) {
    num = num >> indexBit;
    return (num & 1);
}

// 找到 num 从右边数起第一个是 1 的位
unsigned int NumbersAppearOnce::findFirstBitIs1(int num) {
    // 从 0 开始，记录 num 右移的次数
    int indexBit = 0;
    
    // num 每次右移 1 位，直到第一个 1 被移到二进制表示的最右边
    while (((num & 1) == 0) && (indexBit < 8 * sizeof(int))) {
        num = num >> 1;
        ++indexBit;
    }
    
    return indexBit;
}

void NumbersAppearOnce::findNumsAppearOnce(int data[], int length, int* num1, int* num2) {
    // 入参判断，数组不为 nullptr，元素个数至少 2 个
    if (data == nullptr || length < 2) {
        return;
    }
    
    // 全数组的元素做异或操作
    int resultExclusiveOR = 0;
    for (int i = 0; i < length; ++i) {
        resultExclusiveOR ^= data[i];
    }
    
    // 找到异或结果中从右边起第一个是 1 的位
    unsigned int indexOf1 = findFirstBitIs1(resultExclusiveOR);
    
    *num1 = *num2 = 0;
    
    // 数组分为两个组，把 indexOf1 位是 1 的一组，不是 1 的一组，
    // 然后两组数再做异或，最后 *num1 和 *num2 就是要找的两个只出现一次的数字
    for (int j = 0; j < length; ++j) {
        if (isBit1(data[j], indexOf1)) {
            *num1 ^= data[j];
        } else {
            *num2 ^= data[j];
        }
    }
}
```
## 56:(二)数组中唯一只出现一次的数字
&emsp;题目：在一个数组中除了一个数字只出现一次之外，其他数字都出现了三次。请找出那个只出现一次的数字。
```c++
namespace NumberAppearingOnce {
int findNumberAppearingOnce(int numbers[], int length);
}

int NumberAppearingOnce::findNumberAppearingOnce(int numbers[], int length) {
    // 入参判断
    if (numbers == nullptr || length <= 0) {
        throw new std::exception();
    }
    
    // 准备一个长度是 32 的 int 数组，把数组中每个数字的二进制位加起来，
    // 因为其它数字都出现了 3 次，所以相加后，每个二进制位是 3 的倍数，或者不是 3 的倍数，
    // 如果某位是 3 的倍数，表示数组中只出现一次的数字的该位是 0，否则是 1
    int bitSum[32] = {0};
    
    for (int i = 0; i < length; ++i) {
        
        // 把每个数字的二进制表示对应的每位相加
        int bitMask = 1;
        for (int j = 31; j >= 0; --j) {
            int bit = numbers[i] & bitMask;
            
            // 加 1
            if (bit != 0) {
                bitSum[j] += 1;
            }
            
            // 依次统计每位
            bitMask = bitMask << 1;
        }
    }
    
    // 统计每个是 1 的位
    int result = 0;
    for (int i = 0; i < 32; ++i) {
        result = result << 1;
        result += bitSum[i] % 3;
    }
    
    return result;
}
```
## 57:(一)和为 s 的两个数字
&emsp;题目：输入一个递增排序的数组和一个数字 s，在数组中查找两个数，使得它们的和正好是 s。如果有多对数字的和等于 s，输出任意一对即可。
```c++
namespace TwoNumbersWithSum {
bool findNumbersWithSum(int data[], int length, int sum, int* num1, int* num2);
}

// 准备两个指针，一个从数组头开始，一个从数组尾开始，计算两个指针的和，
// 如果和小于 sum，则左边指针前移，如果和大于 sum，则右边指针后移。
bool TwoNumbersWithSum::findNumbersWithSum(int data[], int length, int sum, int* num1, int* num2) {
    bool found = false;
    if (length < 1 || num1 == nullptr || num2 == nullptr) {
        return found;
    }
    
    // 左边
    int ahead = length - 1;
    // 右边
    int behind = 0;
    
    while (ahead > behind) {
        // 求和
        long long curSum = data[ahead] + data[behind];
        
        if (curSum == sum) {
            // 如果和等于 sum，用 num1 和 num2 记录
            *num1 = data[behind];
            *num2 = data[ahead];
            
            // 标记找到
            found = true;
            
            // 已经找到，则跳出循环
            break;
        } else if (curSum > sum) {
            // 和太大了，左移
            --ahead;
        } else {
            // 和太小了，右移
            ++behind;
        }
    }
    
    return found;
}
```
## 57:(二)和为 s 的连续正数序列
&emsp;题目：输入一个正数 s，打印出所有和为 s 的连续正数序列（至少含有两个数）。例如输入 15，由于 1+2+3+4+5 = 4+5+6 = 7+8 = 15，所以结果打印出 3 个连续序列 1〜5、4〜6 和 7〜8。
```c++
namespace ContinuousSquenceWithSum {
void printContinuousSequence(int small, int big);
void findContinuousSequence(int sum);
}

// [1, 2, 3, 4, ...]
void ContinuousSquenceWithSum::findContinuousSequence(int sum) {
    // 连续正数，两个起步，和至少是 1 + 2 = 3
    if (sum < 3) {
        return;
    }
    
    // 分别从正数 1 和 2 开始
    int small = 1;
    int big = 2;
    
    // 最大到 middle
    int middle = (1 + sum) / 2;
    
    // 当前的和
    int curSum = small + big;
    
    while (small < middle) {
        if (curSum == sum) {
            printContinuousSequence(small, big);
        }
        
        // 如果 curSum 大于要求的和，把 small 减去
        while (curSum > sum && small < middle) {
            
            // curSum 减去 small，然后 small 自增
            curSum -= small;
            small++;
            
            if (curSum == sum) {
                printContinuousSequence(small, big);
            }
        }
        
        // 增大 big
        big++;
        curSum += big;
    }
}

// 打印 small 到 bit 的连续正数
void ContinuousSquenceWithSum::printContinuousSequence(int small, int big) {
    for (int i = small; i <= big; ++i) {
        printf("%d ", i);
    }
    
    printf("\n");
}
```
## 58:(一)翻转单词顺序
&emsp;题目：输入一个英文句子，翻转句子中单词的顺序，但单词内字符的顺序不变。为简单起见，标点符号和普通字母一样处理。例如输入字符串 "I am a student. "，则输出 "student. a am I"。
```c++
namespace ReverseWordsInSentence {
void reverse(char* pBegin, char* pEnd);
char* reverseSentence(char* pData);
}

// 翻转字符串，准备两个指针，一个从字符串头部开始，一个从尾部开始，
// 左边指针递增，右边指针递减，交换两个指针指向的字符串
void ReverseWordsInSentence::reverse(char* pBegin, char* pEnd) {
    if (pBegin == nullptr || pEnd == nullptr) {
        return;
    }
    
    while (pBegin < pEnd) {
        // 交换字符
        char temp = *pBegin;
        *pBegin = *pEnd;
        *pEnd = temp;
        
        // 左边指针递增
        ++pBegin;
        // 右边指针递减
        --pEnd;
    }
}

char* ReverseWordsInSentence::reverseSentence(char* pData) {
    // 判空
    if (pData == nullptr) {
        return nullptr;
    }
    
    // 准备两个指针 pBegin 和 pEnd，分别指向开头和末尾
    char* pBegin = pData;
    char* pEnd = pData;
    while (*pEnd != '\0') {
        ++pEnd;
    }
    --pEnd;
    
    // 翻转整个句子
    reverse(pBegin, pEnd);
    
    // 翻转句子中的每个单词
    //（三个指针都是从头开始）
    pBegin = pEnd = pData;
    
    while (*pBegin != '\0') {
        // 首先是 pBegin 指向字符串开头，
        // pEnd 从头往前走，直到遇到空格
        if (*pBegin == ' ') {
            // 空格时，两者都进行前移
            ++pBegin;
            ++pEnd;
        } else if (*pEnd == ' ' || *pEnd == '\0') {
            // *pEnd 是一个空格时，表示 pEnd 到了一个单词的末尾，
            // [pBegin, --pEnd] 刚好是一个单词的区间，翻转一个单词
            reverse(pBegin, --pEnd);
            
            // 更新 pBegin 到下一个单词的开头
            pBegin = ++pEnd;
        } else {
            // pEnd 正常前进，直到遇到空格
            ++pEnd;
        }
    }
    
    return pData;
}
```
## 58:(二)左旋转字符串
&emsp;题目：字符串的左旋转操作是把字符串前面的若干个字符转移到字符串的尾部。请定义一个函数实现字符串左旋转操作的功能。比如输入字符串 "abcdefg" 和数字  2，该函数将返回左旋转2位得到的结果 "cdefgab"。
```c++
namespace LeftRotateString {
char* leftRotateString(char* pStr, int n);
}

// 三步完成，还是使用前面的字符串翻转函数，
// 这里是先翻转字符串前面 n 个字符，
// 再翻转字符串的后面部分，
// 最后翻转整个字符串，

char* LeftRotateString::leftRotateString(char* pStr, int n) {
    if (pStr != nullptr) {
        
        // 获取字符串长度
        int nLength = static_cast<int>(strlen(pStr));
        
        if (nLength > 0 && n > 0 && n < nLength) {
            
            // 把字符串在逻辑上分为两个部分:
            // 字符串 1 [pStr, pStr + n - 1]
            char* pFirstStart = pStr;
            char* pFirstEnd = pStr + n - 1;
            
            // 字符串 2 [pStr + n, pStr + nLength - 1]
            char* pSecondStart = pStr + n;
            char* pSecondEnd = pStr + nLength - 1;
            
            // 翻转字符串的前面 n 个字符
            reverse(pFirstStart, pFirstEnd);
            // 翻转字符串的后面部分
            reverse(pSecondStart, pSecondEnd);
            // 翻转整个字符串
            reverse(pFirstStart, pSecondEnd);
        }
    }
    
    return pStr;
}
```
## 59:(一)滑动窗口的最大值
&emsp;题目：给定一个数组和滑动窗口的大小，请找出所有滑动窗口里的最大值。例如，如果输入数组 {2, 3, 4, 2, 6, 2, 5, 1} 及滑动窗口的大小 3，那么一共存在 6 个滑动窗口，它们的最大值分别为 {4, 4, 6, 6, 6, 5}。
```c++
namespace MaxInSlidingWindow {
vector<int> maxInWindows(const vector<int>& num, unsigned int size);
}

vector<int> MaxInSlidingWindow::maxInWindows(const vector<int>& num, unsigned int size) {
    // 滑动窗口
    vector<int> maxInWindows;
    
    if (num.size() >= size && size >= 1) {
        
        // 两端开口的队列 index，用来记录 num 中可能作为最大值的各元素的下标
        deque<int> index;
        for (unsigned int i = 0; i < size; ++i) {
            // num[i] 大于 num[index.back()] 表示前面的数字都不可能成为滑动窗口的最大值，把它们移除
            while (!index.empty() && num[i] >= num[index.back()]) {
                index.pop_back();
            }
            
            // 入队
            index.push_back(i);
        }
        
        for (unsigned int i = size; i < num.size(); ++i) {
            
            // 滑动窗口的最大值每次都是位于 index.front() 头部
            maxInWindows.push_back(num[index.front()]);
            
            // 去尾
            while (!index.empty() && num[i] >= num[index.back()]) {
                index.pop_back();
            }
            
            // 去头
            
            // index.front() <= (int)(i - size)
            // 表示队列头部的数字下标 和 i 的距离大于等于滑动窗口的大小，
            // 该位置的数字已经滑出滑动窗口了，需要把它出队
            if (!index.empty() && index.front() <= (int)(i - size)) {
                index.pop_front();
            }
            
            // 入队
            index.push_back(i);
        }
        
        maxInWindows.push_back(num[index.front()]);
    }
    
    return maxInWindows;
}
```
## 59:(二)队列的最大值
&emsp;题目：给定一个数组和滑动窗口的大小，请找出所有滑动窗口里的最大值。例如，如果输入数组 {2, 3, 4, 2, 6, 2, 5, 1} 及滑动窗口的大小 3，那么一共存在 6 个滑动窗口，它们的最大值分别为 {4, 4, 6, 6, 6, 5}。
```c++
template<typename T>
class QueueWithMax {
public:
    QueueWithMax() : currentIndex(0) { }
    
    void push_back(T number) {
        // 如果新来的数据大于 maximums 尾部的数据，表示新的数据才会成为滑动窗口的最大值
        while (!maximums.empty() && number >= maximums.back().number) {
            maximums.pop_back();
        }
        
        // 构建 InternalData 局部变量
        InternalData internalData = { number, currentIndex };
        
        // 入队
        data.push_back(internalData);
        maximums.push_back(internalData);
        
        // 自增
        ++currentIndex;
    }
    
    void pop_front() {
        if (maximums.empty()) {
            throw new exception();
        }
        
        // 出队
        if (maximums.front().index == data.front().index) {
            maximums.pop_front();
        }
        
        // 出队
        data.pop_front();
    }
    
    T max() const {
        if (maximums.empty()) {
            throw new exception();
        }
        
        // maximums 队列头部记录的是滑动窗口的最大值
        return maximums.front().number;
    }

private:
    // InternalData 用来记录数据和其下标
    struct InternalData {
        T number;
        int index;
    };
    
    // data 保存原始数据
    deque<InternalData> data;
    // maximums 保存滑动窗口最大值
    deque<InternalData> maximums;
    // currentIndex 记录当前是第几个数字
    int currentIndex;
};
```
## 面试题 60: n 个骰子的点数
&emsp;题目：把 n 个骰子扔在地上，所有骰子朝上一面的点数之和为 s。输入 n，打印出 s 的所有可能的值出现的概率。
```c++
namespace DicesProbability {

int g_maxValue = 6;

void Probability(int number, int* pProbabilities);
void Probability(int original, int current, int sum, int* pProbabilities);

void PrintProbability_Solution1(int number) {
    if(number < 1)
        return;
 
    int maxSum = number * g_maxValue;
    int* pProbabilities = new int[maxSum - number + 1];
    for(int i = number; i <= maxSum; ++i)
        pProbabilities[i - number] = 0;
 
    Probability(number, pProbabilities);
 
    int total = pow((double)g_maxValue, number);
    for(int i = number; i <= maxSum; ++i) {
        double ratio = (double)pProbabilities[i - number] / total;
        printf("%d: %e\n", i, ratio);
    }
 
    delete[] pProbabilities;
}
 
void Probability(int number, int* pProbabilities) {
    for(int i = 1; i <= g_maxValue; ++i)
        Probability(number, number, i, pProbabilities);
}
 
void Probability(int original, int current, int sum, int* pProbabilities) {
    if(current == 1) {
        pProbabilities[sum - original]++;
    } else {
        for(int i = 1; i <= g_maxValue; ++i) {
            Probability(original, current - 1, i + sum, pProbabilities);
        }
    }
}
}
```
## 完结撒花🎉🎉，感谢陪伴！

## 参考链接
**参考链接:🔗**
+ [白话经典算法系列之九 从归并排序到数列的逆序数对（微软笔试题）](https://blog.csdn.net/MoreWindows/article/details/8029996)
