# 《剑指 Offer》面试题二十一～面试题三十的总结

> &emsp;上一篇是 11～20 题，本篇是 21～30 题。⛽️⛽️

## 21:调整数组顺序使奇数位于偶数前面
&emsp;题目：输入一个整数数组，实现一个函数来调整该数组中数字的顺序，使得所有奇数位于数组的前半部分，所有偶数位于数组的后半部分。
```c++
namespace ReorderArray_1 {
void reorder(int* pData, unsigned int length, bool(*func)(int));
bool isEven(int n);
void reorderOddEven_2(int* pData, unsigned int length);
}

// 这里的 func 函数指针可以抽象出来，此题是针对奇数位于偶数前面，
// 我们可以在函数整体逻辑不变的情况下，做正数位于负数前面，能被 3 整除的数位于不能被 3 整除的数前面等等类似的题目

void ReorderArray::reorder(int* pData, unsigned int length, bool(*func)(int)) {
    // 入参判断
    if (pData == nullptr || length <= 0 || func == nullptr) {
        return;
    }
    
    // 准备两个指针，分别从 pData 头部和尾部开始查找
    int* pBegin = pData;
    int* pEnd = pData + length - 1;
    
    while (pBegin < pEnd) {
        // 从首开始遇到第一个偶数时停下
        while (pBegin < pEnd && (*func)(*pBegin)) {
            ++pBegin;
        }
        
        // 从尾开始遇到第一个奇数时停下
        while (pBegin < pEnd && !(*func)(*pEnd)) {
            --pEnd;
        }
        
        // 交换两个数字
        if (pBegin < pEnd) {
            int temp = *pBegin;
            *pBegin = *pEnd;
            *pEnd = temp;
        }
    }
}

// 如果 n 是奇数则返回 true，如果是偶数则返回 false.
// 奇数二进制表示的最后一位一定是 1
bool ReorderArray::isEven(int n) {
    return n & 0x1; // 返回 true 表示是奇数，返回 false 表示偶数
}

void ReorderArray::reorderOddEven_2(int* pData, unsigned int length) {
    reorder(pData, length, isEven);
}
```
## 面试题 22:链表中倒数第k个结点
&emsp;题目：输入一个链表，输出该链表中倒数第k个结点。为了符合大多数人的习惯，本题从 1 开始计数，即链表的尾结点是倒数第1个结点。例如一个链表有 6 个结点，从头结点开始它们的值依次是 1、2、3、4、5、6。这个链表的倒数第 3 个结点是值为 4 的结点。
```c++
namespace KthNodeFromEnd {
// 求链表的中间节点。如果链表中的节点总数为奇数，则返回中间节点；
// 如果节点总数是偶数，则返回中间两个节点的任意一个。
// 可以定义两个指针，同时从链表的头节点出发，一个指针一次走一步
// 另一个指针一次走两步。当走的快的指针走到链表的末尾时，走的慢的指针正好
// 在链表的中间。

ListNode* findKthToTail(ListNode* pListHead, unsigned int k);
ListNode* findMiddleNode(ListNode* pListHead);
}

// 准备两个指针，首先一个指针从链表头前进 k - 1 步，然后第二个指针也开始从头开始前进，
// 当第一个指针到达链表尾部时，第二个指针就到达了倒数第 k 个节点。

ListNode* KthNodeFromEnd::findKthToTail(ListNode* pListHead, unsigned int k) {
    // 入参判断，头节点不为空且 k 必须大于 0
    //（还有一种特殊的情况下面会判断，链表的总长度小于 k 时，也是返回 nullptr）
    if (pListHead == nullptr || k <= 0) {
        return nullptr;
    }
    
    // pAHead 是第一个指针，
    // pAHead 从头节点开始前进 k - 1 个节点
    ListNode* pAHead = pListHead;
    unsigned int i = 0;
    for (; i < k - 1; ++i) {
        pAHead = pAHead->m_pNext;
        
        // 如 pAHead 为 nullptr，表明链表内节点个数少于 k
        if (pAHead == nullptr) {
            return nullptr;
        }
    }
    
    // pBehind 指针从头节点开始和 pAHead 一起前进，
    // 当 pAHead 到最后一个节点时，pBehind 即指向倒数第 k 个节点
    ListNode* pBehind = pListHead;
    while (pAHead->m_pNext != nullptr) {
        pAHead = pAHead->m_pNext;
        pBehind = pBehind->m_pNext;
    }
    
    return pBehind;
}

// 找到链表的中间节点
ListNode* KthNodeFromEnd::findMiddleNode(ListNode* pListHead) {
    if (pListHead == nullptr) {
        return nullptr;
    }

    ListNode* pFast = pListHead;
    ListNode* pSlow = pListHead;
    
//    while (pFast != nullptr && pFast->m_pNext != nullptr) {
    while (pFast->m_pNext != nullptr && pFast->m_pNext->m_pNext != nullptr) {
        pSlow = pSlow->m_pNext;
        
        pFast = pFast->m_pNext;
        if (pFast != nullptr) {
            pFast = pFast->m_pNext;
        }
    }
    
    return pSlow;
}
```
## 面试题 23:链表中环的入口结点
&emsp;题目：一个链表中包含环，如何找出环的入口结点？
```c++
namespace EntryNodeInListLoop {
ListNode* meetingNode(ListNode* pHead);
ListNode* entryNodeOfLoop(ListNode* phead);
}

// 1. 首先找到链表中环的一个点（两个指针一个快一慢，当快的赶上慢的时候，就是环中的一个节点）
// 2. 计算出环的长度（从环中的一个节点开始计数，当再次走到这个节点时，即可得出环的长度）
// 3. 找到环的入口（两个指针一个指针先前进环的长度，然后另一个指针从链表头节点开始，当两个指针相遇时即是环的入口）

// 找到环中的一个节点
ListNode* EntryNodeInListLoop::meetingNode(ListNode* pHead) {
    if (pHead == nullptr) {
        return nullptr;
    }
    
    // pSlow 初始是头节点的下一个节点（可理解为由头节点前进一步）
    ListNode* pSlow = pHead->m_pNext;
    if (pSlow == nullptr) {
        return nullptr;
    }
    
    // pFast 初始是头节点的下下一个节点（可理解为由头节点前进两步）
    ListNode* pFast = pSlow->m_pNext;
    
    while (pFast != nullptr && pSlow != nullptr) {
    
        // 当 pFast 和 pSlow 相遇时，即找到了链表中的一个节点
        if (pFast == pSlow) {
            return pFast;
        }
        
        // pSlow 一次前进一个节点
        pSlow = pSlow->m_pNext;
        
        // pFast 一次前进两个节点
        pFast = pFast->m_pNext;
        if (pFast != nullptr) {
            pFast = pFast->m_pNext;
        }
    }
    
    // 如果未找到则返回 nullptr
    return nullptr;
}

// 找到包含环的链表中环的入口
ListNode* EntryNodeInListLoop::entryNodeOfLoop(ListNode* pHead) {
    if (pHead == nullptr) {
        return nullptr;
    }
    
    // 找到环中一个节点
    ListNode* pMeetingNode = meetingNode(pHead);
    if (pMeetingNode == nullptr) {
        return nullptr;
    }
    
    // 从👆上面找到的节点开始进行计数，统计环中节点的个数
    unsigned int nNodesOfLoop = 1;
    ListNode* pNode1 = pMeetingNode;
    while (pNode1->m_pNext != pMeetingNode) {
        pNode1 = pNode1->m_pNext;
        ++nNodesOfLoop;
    }
    
    // pNode1 从头节点开始前进 nNodesOfLoop 个节点
    pNode1 = pHead;
    for (unsigned int i = 0; i < nNodesOfLoop; ++i) {
        pNode1 = pNode1->m_pNext;
    }
    
    // pNode2 从头节点开始和 pNode1 一起前进，当两者相遇时，即到了环的入口节点
    ListNode* pNode2 = pHead;
    while (pNode1 != pNode2) {
        pNode1 = pNode1->m_pNext;
        pNode2 = pNode2->m_pNext;
    }
    
    // 返回入口节点
    return pNode1;
}
```
## 面试题 24:反转链表
&emsp;题目：定义一个函数，输入一个链表的头结点，反转该链表并输出反转后链表的头结点。
```c++
namespace ReverseList {
ListNode* reverseList(ListNode* pHead);
}

// 1. 链表反转后的头节点即是没有反转之前的尾节点，谁是尾节点呢，即 m_pNext 是 nullptr 的节点
// 2. 翻转一个节点时记录前一个节点，保证链表不发生断裂

ListNode* ReverseList::reverseList(ListNode* pHead) {
    // pReverseHead 用于记录反转后的头节点
    ListNode* pReverseHead = nullptr;
    
    // pNode 记录当前的头节点
    ListNode* pNode = pHead;
    
    // pPrev 用于反转一个节点时保留 前一个节点 ，防止断裂
    ListNode* pPrev = nullptr;
    
    while (pNode != nullptr) {
        // pNext 用于反转一个节点时保留 下一个节点 ，防止断裂
        ListNode* pNext = pNode->m_pNext;
        
        // 当一个节点的 m_pNext 是 nullptr 时，那么它就是新的头节点
        if (pNext == nullptr) {
            pReverseHead = pNode;
        }
        
        // pNode 表示当前要发生反转的节点
        
        // pNode 的下一个节点指向 pPrev
        pNode->m_pNext = pPrev;
        
        // 更新 pPrev
        pPrev = pNode;
        // 更新 pNode
        pNode = pNext;
    }
    
    return pReverseHead;
}
```
## 面试题 25:合并两个排序的链表
&emsp;题目：输入两个递增排序的链表，合并这两个链表并使新链表中的结点仍然是按照递增排序的。
```c++
namespace MergeSortedLists {
ListNode* merge(ListNode* pHead1, ListNode* pHead2);
}

// 1. 如果一个链表为 nullptr 即合并后的链表就是另外一个节点
// 2. 每次两个链表的头节点谁的值更小，谁就是合并链表的一个新节点，然后递归比较双方的下一个节点 

ListNode* MergeSortedLists::merge(ListNode* pHead1, ListNode* pHead2) {
    // 如果 pHead1 为 nullptr，则直接返回 pHead2
    if (pHead1 == nullptr) {
        return pHead2;
    }
    
    // 如果 pHead2 为 nullptr，则直接返回 pHead1
    if (pHead2 == nullptr) {
        return pHead1;
    }
    
    ListNode* pMergeHead = nullptr;
    if (pHead1->m_nValue < pHead2->m_nValue) {
        // 如果 pHead1 的值更小的话，提出 pHead1 的节点
        pMergeHead = pHead1;
        
        // 然后递归调用，入参是 pHead1->m_pNext 和 pHead2
        pMergeHead->m_pNext = merge(pHead1->m_pNext, pHead2);
    } else {
        // 如果 pHead2 的值更小的话，提出 pHead2 的节点
        pMergeHead = pHead2;
        
        // 然后递归调用，入参是 pHead1 和 pHead2->m_pNext
        pMergeHead->m_pNext = merge(pHead1, pHead2->m_pNext);
    }
    
    return pMergeHead;
}
```
## 面试题 26:树的子结构
&emsp;题目：输入两棵二叉树 A 和 B，判断 B 是不是 A 的子结构。
```c++
namespace SubstructureInTree {
struct BinaryTreeNode {
    double m_dbValue;
    BinaryTreeNode* m_pLeft;
    BinaryTreeNode* m_pRight;
};

bool doesTree1HaveTree2(BinaryTreeNode* pRoot1, BinaryTreeNode* pRoot2);
bool equal(double num1, double num2);
bool hasSubtree(BinaryTreeNode* pRoot1, BinaryTreeNode* pRoot2);
}

// 1. 首先在 A 中找到一个节点的值和 B 的根节点的值相等，然后从此节点开始判断 A 是否包含 B 的所有节点

bool SubstructureInTree::doesTree1HaveTree2(BinaryTreeNode* pRoot1, BinaryTreeNode* pRoot2) {
    // 如果 pRoot2 为 nullptr 了，则表示 B 的所有节点都被包含在 A 中，则返回 true
    if (pRoot2 == nullptr) {
        return true;
    }
    
    // 如果 pRoot1 先为 nullptr 了，则表示 B 还有节点在 A 中没有找到，则返回 false
    if (pRoot1 == nullptr) {
        return false;
    }
    
    // 如果双方有节点的值不相等，则直接返回 false
    if (!equal(pRoot1->m_dbValue, pRoot2->m_dbValue)) {
        return false;
    }
    
    // 递归比较左右子树
    return doesTree1HaveTree2(pRoot1->m_pLeft, pRoot2->m_pLeft) && doesTree1HaveTree2(pRoot1->m_pRight, pRoot2->m_pRight);
}

// double 类型数值判等函数，[-0.0000001, 0.0000001]
bool SubstructureInTree::equal(double num1, double num2) {
    if (num1 - num2 > -0.0000001 && num1 - num2 < 0.0000001) {
        return true;
    } else {
        return false;
    }
}

bool SubstructureInTree::hasSubtree(BinaryTreeNode* pRoot1, BinaryTreeNode* pRoot2) {
    bool result = false;
    
    if (pRoot1 != nullptr && pRoot2 != nullptr) {
        // 如果找到了一个节点的值和 pRoot2 根节点相等，则调用 doesTree1HaveTree2 函数开始比较其它节点
        if (equal(pRoot1->m_dbValue, pRoot2->m_dbValue)) {
            result = doesTree1HaveTree2(pRoot1, pRoot2);
        }
        
        // 在 pRoot1 的左子树中找和 pRoot2 根节点相等的节点
        if (!result) {
            result = hasSubtree(pRoot1->m_pLeft, pRoot2);
        }
        
        // 在 pRoot1 的右子树中找和 pRoot2 根节点相等的节点
        if (!result) {
            result = hasSubtree(pRoot1->m_pRight, pRoot2);
        }
    }
    
    return result;
}
```
## 面试题 27:二叉树的镜像
&emsp;题目：请完成一个函数，输入一个二叉树，该函数输出它的镜像。
```c++
namespace MirrorOfBinaryTree {
void mirrorRecursively(BinaryTreeNode* pRoot);
void mirrorIteratively(BinaryTreeNode* pRoot);
}

// 核心解法是交换非叶子节点的左右节点

// 递归
void MirrorOfBinaryTree::mirrorRecursively(BinaryTreeNode* pRoot) {
    if (pRoot == nullptr) {
        return;
    }
    
    // 交换根节点的左右子节点
    BinaryTreeNode* temp = pRoot->m_pLeft;
    pRoot->m_pLeft = pRoot->m_pRight;
    pRoot->m_pRight = temp;
    
    // 递归调用左子树
    if (pRoot->m_pLeft != nullptr) {
        mirrorRecursively(pRoot->m_pLeft);
    }
    
    // 递归调用右子树
    if (pRoot->m_pRight != nullptr) {
        mirrorRecursively(pRoot->m_pRight);
    }
}

// 迭代
// 使用一个栈从根节点开始，记录交换每个节点的左右子节点
void MirrorOfBinaryTree::mirrorIteratively(BinaryTreeNode* pRoot) {
    if (pRoot == nullptr) {
        return;
    }
    
    // 准备一个栈，并把根节点放入栈中
    std::stack<BinaryTreeNode*> nodes;
    nodes.push(pRoot);
    
    while (!nodes.empty()) {
        // 栈顶节点出栈
        BinaryTreeNode* top = nodes.top();
        nodes.pop();
        
        // 交换栈顶节点的左右子节点
        BinaryTreeNode* temp = top->m_pLeft;
        top->m_pLeft = top->m_pRight;
        top->m_pRight = temp;
        
        // 把左子节点放入栈中
        if (top->m_pLeft != nullptr) {
            nodes.push(top->m_pLeft);
        }
        
        // 把右子节点放入栈中
        if (top->m_pRight != nullptr) {
            nodes.push(top->m_pRight);
        }
    }
}
```
## 面试题 28:对称的二叉树
&emsp;题目：请实现一个函数，用来判断一棵二叉树是不是对称的。如果一棵二叉树和它的镜像一样，那么它是对称的。
```c++
namespace SymmetricalBinaryTree {
bool isSymmetrical(BinaryTreeNode* pRoot1, BinaryTreeNode* pRoot2);
bool isSymmetrical(BinaryTreeNode* pRoot);
}

bool SymmetricalBinaryTree::isSymmetrical(BinaryTreeNode* pRoot1, BinaryTreeNode* pRoot2) {
    // 两者都是 nullptr 时才能返回 true
    if (pRoot1 == nullptr && pRoot2 == nullptr) {
        return true;
    }
    
    // 任有一个节点为 nullptr 则返回 false
    if (pRoot1 == nullptr || pRoot2 == nullptr) {
        return false;
    }
    
    // 如果节点值不同，则返回 false
    if (pRoot1->m_nValue != pRoot2->m_nValue) {
        return false;
    }
    
    // 递归比较左子节点和右子节点
    return isSymmetrical(pRoot1->m_pLeft, pRoot2->m_pRight) && isSymmetrical(pRoot1->m_pRight, pRoot2->m_pLeft);
}

bool SymmetricalBinaryTree::isSymmetrical(BinaryTreeNode* pRoot) {
    return isSymmetrical(pRoot, pRoot);
}
```
## 面试题 29:顺时针打印矩阵
&emsp;题目：输入一个矩阵，按照从外向里以顺时针的顺序依次打印出每一个数字。
```c++
namespace PrintMatrix {
void PrintMatrixInCircle(int** numbers, int colums, int rows, int start);
void printNumber(int number);
void PrintMatrixClockwisely(int** numbers, int colums, int rows);
}

void PrintMatrix::printNumber(int number) {
    printf("%d\t", number);
}

void PrintMatrix::PrintMatrixInCircle(int** numbers, int colums, int rows, int start) {
    int endX = colums - 1 - start;
    int endY = rows - 1 - start;
    
    // 从左到右打印一行（行不变，列增加）
    for (int i = start; i <= endX; ++i) {
        int number = numbers[start][i];
        printNumber(number);
    }
    
    // 从上到下打印一列（列不变，行增加）
    if (start < endY) {
        for (int i = start + 1; i <= endY; ++i) {
            int number = numbers[i][endX];
            printNumber(number);
        }
    }
    
    // 从右到左打印一行（行不变，列减小）
    if (start < endX && start < endY) {
        for (int i = endX - 1; i >= start; --i) {
            int number = numbers[endY][i];
            printNumber(number);
        }
    }
    
    // 从下到上打印一行（列不变，行减小）
    if (start < endX && start < endY - 1) {
        for (int i = endY - 1; i >= start + 1; --i) {
            int number = numbers[i][start];
            printNumber(number);
        }
    }
}

void PrintMatrix::PrintMatrixClockwisely(int** numbers, int colums, int rows) {
    if (numbers == nullptr || colums <= 0 || rows <= 0) {
        return;
    }
    
    int start = 0;
    
    // 列数大于 start 的 2 倍且行数大于 start 的 2 倍则能继续进行
    while (colums > start * 2 && rows > start * 2) {
        PrintMatrixInCircle(numbers, colums, rows, start);
        ++start;
    }
}
```
## 面试题 30:包含min函数的栈
&emsp;题目：定义栈的数据结构，请在该类型中实现一个能够得到栈的最小元素的 min 函数。在该栈中，调用 min、push 及 pop 的时间复杂度都是 O(1)。
```c++
namespace StackWithMin {
// 准备一个辅助栈 m_min 保存栈中的当前的最小值
template <typename T>
class StackWithMin {
public:
    StackWithMin() {}
    virtual ~StackWithMin() {}
    
    // 栈顶
    T& top();
    const T& top() const;
    
    // 入栈
    void push(const T& value);
    // 出栈
    void pop();
    
    // 最小值函数
    const T& min() const;
    
    // 是否为空
    bool empty() const;
    // 栈中元素个数
    size_t size() const;
private:
    // 存放数据栈
    stack<T> m_data;
    // 存放当前最小值的栈
    stack<T> m_min;
};
}

template <typename T>
T& StackWithMin::StackWithMin<T>::top() {
    // 栈顶元素
    return m_data.top();
}

template <typename T>
const T& StackWithMin::StackWithMin<T>::top() const {
    // 栈顶元素
    return m_data.top();
}

// 每次入栈，都比较 value 和当前 m_min 的栈顶元素，如果 value 更小则把 value 放入 m_min 中，
// 如果当前 m_min 栈顶元素更小，则把 m_min 的栈顶元素再次入栈
template <typename T>
void StackWithMin::StackWithMin<T>::push(const T& value) {
    // 数据正常入栈 m_data
    m_data.push(value);
    
    // 如果是第一个元素，或者 value 值更小，则入栈 m_min
    if (m_min.empty() || value < m_min.top()) {
        m_min.push(value);
    } else {
        // 如果 m_min 栈顶元素更小，则再次入栈这个栈顶元素
        m_min.push(m_min.top());
    }
}

template <typename T>
void StackWithMin::StackWithMin<T>::pop() {
    assert(m_data.size() > 0 && m_min.size() > 0);
    
    // 出栈时，双方都进行出栈
    m_data.pop();
    m_min.pop();
}

template <typename T>
const T& StackWithMin::StackWithMin<T>::min() const {
    assert(m_data.size() > 0 && m_min.size() > 0);
    
    // m_min 的栈顶元素一直保持的是当前 m_data 中的最小值
    return m_min.top();
}

template <typename T>
bool StackWithMin::StackWithMin<T>::empty() const {
    // m_data 是否为空
    return m_data.empty();
}

template <typename T>
size_t StackWithMin::StackWithMin<T>::size() const {
    // m_data 元素个数
    return m_data.size();
}
```
## 完结撒花🎉🎉，感谢陪伴！
