# 《剑指 Offer》面试题三十一～面试题四十的总结

> &emsp;上一篇是 21～30 题，本篇是 31～40 题。⛽️⛽️

## 面试题 31:栈的压入、弹出序列
&emsp;题目：输入两个整数序列，第一个序列表示栈的压入顺序，请判断第二个序列是否为该栈的弹出顺序。假设压入栈的所有数字均不相等。例如序列 1、2、3、4、5 是某栈的压栈序列，序列 4、5、3、2、1 是该压栈序列对应的一个弹出序列，但 4、3、5、1、2 就不可能是该压栈序列的弹出序列。
```c++
namespace StackPushPopOrder {
bool isPopOrder(const int* pPush, const int* pPop, int nLength);
}

// 两个序列一个是入栈序列，一个是出栈序列，这里入栈序列中的元素不是一次全部入栈的，
// 而是部分元素入栈了，然后直接又出栈了，随后后面从没入栈过的元素再接着入栈出栈，
// 而出栈序列正是记录这些出栈元素的顺序。

bool StackPushPopOrder::isPopOrder(const int* pPush, const int* pPop, int nLength) {
    bool bPossible = false;
    
    if (pPush != nullptr && pPop != nullptr && nLength > 0) {
        
        // pNextPush 和 pNextPop 分别从头到尾指向入栈和出栈序列的每个元素
        const int* pNextPush = pPush;
        const int* pNextPop = pPop;
        
        // 准备一个栈，把每个入栈序列中的元素入栈再出栈，用来模拟出栈序列的出栈过程
        //（当栈顶元素和 *pNextPop 不等时，要继续从 pNextPush 往后查找，直到找到一个相等的元素入栈）
        stack<int> stackData;
        
        // 循环条件是 pNextPush 指向出栈序列的末尾
        while (pNextPop - pPop < nLength) {
            
            // 把 pPush 序列中的元素入栈，直到找到一个等于 pNextPop 的元素出栈
            while (stackData.empty() || stackData.top() != *pNextPop) {
                
                // 判断 pNextPush 是否到了入栈序列的末尾
                if (pNextPush - pPush == nLength) {
                    break;
                }
                
                // 把 pNextPush 入栈 stackData
                stackData.push(*pNextPush);
                // pNextPush 指针往后移动
                ++pNextPush;
            }
            
            // 如果在 pPush 一直找不到一个和 *pNextPop 相等的元素，则表明这 pPop 不是该栈的弹出序列
            if (stackData.top() != *pNextPop) {
                break;
            }
            
            // 找到了一个弹出的元素
            stackData.pop();
            // 找到了一个弹出元素，pNextPop 也往后移动一位
            ++pNextPop;
        }
        
        // 最后 pNextPop 到了出栈序列的末尾， stackData 中全部元素也都弹出了，则表明 pPop 是弹出序列
        if (stackData.empty() && pNextPop - pPop == nLength) {
            bPossible = true;
        }
    }
    
    // 返回 bPossible
    //（这里可能有一个固定思维，这里是入栈序列不是已经固定的栈，如果是固定的栈，那么它的弹出序列也是唯一的，
    // 这里是入栈的序列，它们可以对应很多种不同的出栈序列，入栈序列不是一次全部入栈的，后入栈的元素可以任意等待栈内的元素是否出栈后它再进行入栈）
    return bPossible;
}
```
## 32:(一)不分行从上往下打印二叉树
&emsp;题目：从上往下打印出二叉树的每个结点，同一层的结点按照从左到右的顺序打印。
```c++
namespace PrintTreeFromTopToBottom {
void printFromTopToBottom(BinaryTreeNode* pRoot);
}

// 每次遍历打印完一个节点后把它的左右子节点入队，直到队列为空
void PrintTreeFromTopToBottom::printFromTopToBottom(BinaryTreeNode* pRoot) {
    if (pRoot == nullptr) {
        return;
    }
    
    // 准备一个队列，并首先把根节点入队列
    deque<BinaryTreeNode*> dequeTreeNode;
    dequeTreeNode.push_back(pRoot);
    
    while (!dequeTreeNode.empty()) {
        // 节点出队列
        BinaryTreeNode* node = dequeTreeNode.front();
        dequeTreeNode.pop_front();
        
        // 打印节点的值
        printf("%d\t", node->m_nValue);
        
        // 如果出队的这个节点有左子节点，则入队列，为下一层的节点打印做准备
        if (node->m_pLeft != nullptr) {
            dequeTreeNode.push_back(node->m_pLeft);
        }
        
        // 如果出队的这个节点有右子节点，则入队列，为下一层的节点打印做准备
        if (node->m_pRight != nullptr) {
            dequeTreeNode.push_back(node->m_pRight);
        }
    }
}
```
## 32:(二)分行从上到下打印二叉树
&emsp;题目：从上到下按层打印二叉树，同一层的结点按从左到右的顺序打印，每一层打印到一行。
```c++
namespace PrintTreesInLines {
void print(BinaryTreeNode* pRoot);
}

void PrintTreesInLines::print(BinaryTreeNode* pRoot) {
    if (pRoot == nullptr) {
        return;
    }
    
    // 准备一个队列，并把根节点放进队列
    deque<BinaryTreeNode*> dequeTreeNode;
    dequeTreeNode.push_back(pRoot);
    
    // nextLevel 用来统计第一行之外的一行中要打印的节点个数
    int nextLevel = 0;
    
    // toBePrinted 表示本行要打印的节点个数，默认值为 1 代指根节点的一行就一个节点
    int toBePrinted = 1;
    
    while (!dequeTreeNode.empty()) {
        
        // 出队列并打印节点
        BinaryTreeNode* node = dequeTreeNode.front();
        printf("%d ", node->m_nValue);
        
        // 如果左节点不为空，则放进队列
        if (node->m_pLeft != nullptr) {
            dequeTreeNode.push_back(node->m_pLeft);
            // 统计节点
            ++nextLevel;
        }
        
        if (node->m_pRight != nullptr) {
            dequeTreeNode.push_back(node->m_pRight);
            // 统计节点
            ++nextLevel;
        }
        
        dequeTreeNode.pop_front();
        
        // 本行要打印的节点个数减 1
        --toBePrinted;
        
        // 如果本行的节点打印完了，则打印换行，并更新 toBePrinted 为下一行要打印的节点个数，
        // nextLevel 则置为 0，继续统计下下一行的节点个数
        if (toBePrinted == 0) {
            printf("\n");
            toBePrinted = nextLevel;
            nextLevel = 0;
        }
    }
}
```
## 32:(三)之字形打印二叉树
&emsp;题目：请实现一个函数按照之字形顺序打印二叉树，即第一行按照从左到右的顺序打印，第二层按照从右到左的顺序打印，第三行再按照从左到右的顺序打印，其他行以此类推。
```c++
namespace PrintTreesInZigzag {
void print(BinaryTreeNode* pRoot);
}

void PrintTreesInZigzag::print(BinaryTreeNode* pRoot) {
    if (pRoot == nullptr) {
        return;
    }
    
    // levels 是一个长度是 2 的 stack<BinaryTreeNode*> 数组，
    // 从上到下每行的节点交替放在 levels[0] 和 levels[1] 中
    stack<BinaryTreeNode*> levels[2];
    
    // current 表示当前正在打印的行的节点所在的 stack<BinaryTreeNode*>
    int current = 0;
    // next 表示下一行节点所在的 stack<BinaryTreeNode*>
    int next = 1;
    
    // 把根节点加入第一个栈
    levels[current].push(pRoot);
    
    // 只要两个栈任一个不为空就表示还有节点需要打印
    while (!levels[0].empty() || !levels[1].empty()) {
        BinaryTreeNode* node = levels[current].top();
        levels[current].pop();
        
        printf("%d ", node->m_nValue);
        
        if (current == 0) {
            // 如果 current 等于 0，则左子节点先入队，右子节点后入队，且队列是 levels[1]
            
            if (node->m_pLeft != nullptr) {
                levels[next].push(node->m_pLeft);
            }
            
            if (node->m_pRight != nullptr) {
                levels[next].push(node->m_pRight);
            }
        } else {
            // 如果 current 等于 1，则右子节点先入队，左子节点后入队，且队列是 levels[0]
            
            if (node->m_pRight != nullptr) {
                levels[next].push(node->m_pRight);
            }
            
            if (node->m_pLeft != nullptr) {
                levels[next].push(node->m_pLeft);
            }
        }
        
        // 如果当前的 stack<BinaryTreeNode*> 为空，表示一行打印完成了，则切到下一行打印
        if (levels[current].empty()) {
            // 每次进入这个 if 表示要开始打印新的一行了，current 和 next 都分别做 1 和 0 的交换
            
            printf("\n");
            current = 1 - current;
            next = 1 - next;
        }
    }
}
```
## 33:二叉搜索树的后序遍历序列
&emsp;题目：输入一个整数数组，判断该数组是不是某二叉搜索树的后序遍历的结果。如果是则返回 true，否则返回 false。假设输入的数组的任意两个数字都互不相同。
```c++
namespace SquenceOfBST {
bool verifySquenceOfBST(int sequence[], int length);
}

// 二叉树搜索树的特点是：所有的左子节点都小于父节点，所有的右子节点都大于父节点
// 二叉树的后序遍历序列是左右根则顺序，则首先可断定的是后序序列的最后一个元素一定是树的根节点
bool SquenceOfBST::verifySquenceOfBST(int sequence[], int length) {
    if (sequence == nullptr || length <= 0) {
        return false;
    }
    
    // 因为是后序序列，所以最后一个元素一定是二叉树的根节点，首先取出根节点的值
    int nRootValue = sequence[length - 1];
    
    // 在后序序列中找到左子树中的根节点的位置的后面一个位置，已知左子树的节点全部小于根节点，
    // 右子树的节点全部大于根节点，所以可通过便利 [0, length-1) 找到左子树和右子树的分界
    unsigned int nLeftIndexEnd = 0;
    for (; nLeftIndexEnd < length - 1; ++nLeftIndexEnd) {
        if (sequence[nLeftIndexEnd] > nRootValue) {
            break;
        }
    }

    // 这里是判断所有右子树的节点都大于根节点，否则直接返回 false
    unsigned int nRightStart = nLeftIndexEnd;
    for (; nRightStart < length - 1; ++nRightStart) {
        if (sequence[nRightStart] < nRootValue) {
            return false;
        }
    }

    // 下面开始递归判断左右子树后序序列是否符合二叉搜索树的规则
    bool bLeft = true;
    // 如果 nLeftIndexEnd 大于 0 表示存在左子树，下面是递归判断左子树是否也是二叉搜索树
    if (nLeftIndexEnd > 0) {
        bLeft = verifySquenceOfBST(sequence, nLeftIndexEnd);
    }

    bool bRight = true;
    // 如果 nLeftIndexEnd 小于 length - 1 表示存在右子树，下面是递归判断右子树是否也是二叉搜索树
    if (nLeftIndexEnd < length - 1) {
        bRight = verifySquenceOfBST(sequence + nLeftIndexEnd, length - nLeftIndexEnd - 1);
    }
    
    // 返回值（左右子树必须全部都是二叉搜索树）
    return bLeft && bRight;
}
```
## 34:二叉树中和为某一值的路径
&emsp;题目：输入一棵二叉树和一个整数，打印出二叉树中结点值的和为输入整数的所有路径。从树的根结点开始往下一直到叶结点所经过的结点形成一条路径。
```c++
namespace PathInTree {
void findPath(BinaryTreeNode* pRoot, int expectedsum, std::vector<int>& path, int& currentSum);
void findPath(BinaryTreeNode* pRoot, int expectedSum);
}

// 从根节点到叶节点的完整路径
void PathInTree::findPath(BinaryTreeNode* pRoot, int expectedsum, std::vector<int>& path, int& currentSum) {
    // currentSum 加根节点的值
    currentSum += pRoot->m_nValue;
    // 把根节点放入 path vector
    path.push_back(pRoot->m_nValue);
    
    // isLeaf 判断是否是根节点
    bool isLeaf = pRoot->m_pLeft == nullptr && pRoot->m_pRight == nullptr;
    
    // 如果找到了完整路径则进行打印
    if (currentSum == expectedsum && isLeaf) {
        std::vector<int>::iterator iter = path.begin();
        for (; iter != path.end() ; ++iter) {
            printf("%d\t", *iter);
        }
        
        printf("\n");
    }
    
    // 在左子树中找路径
    if (pRoot->m_pLeft != nullptr) {
        findPath(pRoot->m_pLeft, expectedsum, path, currentSum);
    }
    
    // 在右子树中找路径
    if (pRoot->m_pRight != nullptr) {
        findPath(pRoot->m_pRight, expectedsum, path, currentSum);
    }
    
    // 如果当前完整路径的和不是 expectedsum，则回退
    currentSum -= pRoot->m_nValue;
    path.pop_back();
}

void PathInTree::findPath(BinaryTreeNode* pRoot, int expectedSum) {
    if (pRoot == nullptr) {
        return;
    }
    
    // 准备一个 vector 用来存放路径上的节点值
    std::vector<int> path;
    // 记录当前路径上节点值的和
    int currentSum = 0;
    // findPath
    findPath(pRoot, expectedSum, path, currentSum);
}
```
## 面试题 35:复杂链表的复制
&emsp;题目：请实现函数 ComplexListNode* Clone(ComplexListNode* pHead)，复制一个复杂链表。在复杂链表中，每个结点除了有一个 m_pNext 指针指向下一个结点外，还有一个 m_pSibling 指向链表中的任意结点或者 nullptr。
```c++
namespace CopyComplexList {
void cloneNodes(ComplexListNode* pHead);
void connectSiblingNodes(ComplexListNode* pHead);
ComplexListNode* reconnectNodes(ComplexListNode* pHead);
ComplexListNode* clone(ComplexListNode* pHead);
}

// 复制节点，把每个复制的节点直接链接在原节点的后面
void CopyComplexList::cloneNodes(ComplexListNode* pHead) {
    ComplexListNode* pNode = pHead;
    while (pNode != nullptr) {
        // 新建节点 m_nValue 是原节点的 m_nValue，m_pSibling 暂时赋 nullptr
        ComplexListNode* pCloned = new ComplexListNode();
        pCloned->m_nValue = pNode->m_nValue;
        pCloned->m_pSibling = nullptr;
        
        // 注意 m_pNext 的赋值，直接链接原节点的下一个节点
        pCloned->m_pNext = pNode->m_pNext;
        
        // 原节点到 m_pNext 指向新复制出的节点
        pNode->m_pNext = pCloned;
        
        // 更新 pNode，继续下个节点的复制
        pNode = pCloned->m_pNext;
    }
}

// 更新 pNode，继续下个节点的复制
void CopyComplexList::connectSiblingNodes(ComplexListNode* pHead) {
    ComplexListNode* pNode = pHead;
    while (pNode != nullptr) {
        if (pNode->m_pSibling != nullptr) {
            // 这里 pNode->m_pNext 就是我们链接在原节点后面的复制的节点，
            // 同样 pNode->m_pSibling 后面紧接着的节点也是 m_pSibling 的复制节点，
            // 所以正是作为复制节点的 m_pSibling
            pNode->m_pNext->m_pSibling = pNode->m_pSibling->m_pNext;
        }
        
        // 更新 pNode 继续进行下个节点的循环
        //（这里之所以不用判断 pNode->m_pNext 是否为 nullptr，是因为每个原节点都进行了复制操作，它的 m_pNext 必不为 nullptr）
        pNode = pNode->m_pNext->m_pNext;
    }
}

// 从复制后的长链表中拆出复制链表
ComplexListNode* CopyComplexList::reconnectNodes(ComplexListNode* pHead) {
    // 用于遍历原链表的临时节点
    ComplexListNode* pNode = pHead;
    
    // 准备的记录复制链表的头节点
    ComplexListNode* pClonedHead = nullptr;
    
    // 用于遍历复制链表的临时节点
    ComplexListNode* pClonedNode = nullptr;
    
    if (pNode != nullptr) {
        
        // 复制链表的头节点就是原链表头节点的 m_pNext
        pClonedHead = pClonedNode = pNode->m_pNext;
        
        // pNode 的 m_pNext 还指向它的原始的下一个节点（即原链表复原）
        pNode->m_pNext = pClonedNode->m_pNext;
        
        // 更新 pNode 到下一个节点
        pNode = pNode->m_pNext;
    }
    
    while (pNode != nullptr) {
        
        // 更新复制节点的 m_pNext 指向 pNode 的 m_pNext
        pClonedNode->m_pNext = pNode->m_pNext;
        
        // 更新 pClonedNode
        pClonedNode = pClonedNode->m_pNext;
        
        // 更新 pNode 的 m_pNext 指向它以前的 m_pNext 节点
        pNode->m_pNext = pClonedNode->m_pNext;
        
        // 更新 pNode
        pNode = pNode->m_pNext;
    }
    
    // 返回复制链表的头节点
    return pClonedHead;
}

ComplexListNode* CopyComplexList::clone(ComplexListNode* pHead) {
    // 复制节点，（复制的节点直接跟在原节点的后面）
    cloneNodes(pHead);
    
    // 链接 m_pSibling 节点
    connectSiblingNodes(pHead);
    
    // 重连，（即拆分原链表和新复制的链表）
    return reconnectNodes(pHead);
}
```
## 面试题 36:二叉搜索树与双向链表
&emsp;题目：输入一棵二叉搜索树，将该二叉搜索树转换成一个排序的双向链表。要求不能创建任何新的结点，只能调整树中结点指针的指向。
```c++
namespace ConvertBinarySearchTree {
void convertNode(BinaryTreeNode* pNode, BinaryTreeNode** pLastNodeInList);
BinaryTreeNode* convert(BinaryTreeNode* pRootOfTree);
}

void ConvertBinarySearchTree::convertNode(BinaryTreeNode* pNode, BinaryTreeNode** pLastNodeInList) {
    if (pNode == nullptr) {
        return;
    }
    
    BinaryTreeNode* pCurrent = pNode;
    
    // pCurrent 记录当前节点，然后沿着 m_pLeft 一直找下去，
    // 到底的节点一定就是值最小的节点，也就是链表的头节点。
    if (pCurrent->m_pLeft != nullptr) {
        // 一直沿着 pCurrent->m_pLeft 递归向下
        convertNode(pCurrent->m_pLeft, pLastNodeInList);
    }
    
    // 这里先把 pCurrent 的 m_pLeft 指向 *pLastNodeInList，
    // 第一次时 *pLastNodeInList 的值是 nullptr，它一直是作为当前链表的尾节点存在的，所以当一个节点进来时，m_pLeft 都是指向它，
    // 然后下面 *pLastNodeInList 的 m_pRight 指向这个当前节点，即把这个进来的节点追加到链表的尾部了。
    pCurrent->m_pLeft = *pLastNodeInList;
    if (*pLastNodeInList != nullptr) {
        (*pLastNodeInList)->m_pRight = pCurrent;
    }
    
    // 然后更新 *pLastNodeInList，往后移动到链表的最后，即更新了当前链表的尾节点。
    *pLastNodeInList = pCurrent;
    
    // 这里是递归每个左子节点的右子树或者右子节点
    if (pCurrent->m_pRight != nullptr) {
        convertNode(pCurrent->m_pRight, pLastNodeInList);
    }
}

BinaryTreeNode* ConvertBinarySearchTree::convert(BinaryTreeNode* pRootOfTree) {
    BinaryTreeNode* pLastNodeInList = nullptr;
    
    // 二叉搜索树转换为双向链表
    convertNode(pRootOfTree, &pLastNodeInList);
    
    // pLastNodeInList 指向双向链表的尾节点，
    // 从尾节点沿着 m_pLeft 遍历到链表头部节点并返回头节点
    BinaryTreeNode* pHeadOfList = pLastNodeInList;
    while (pHeadOfList != nullptr && pHeadOfList->m_pLeft != nullptr) {
        pHeadOfList = pHeadOfList->m_pLeft;
    }
    
    // 返回链表的头节点
    return pHeadOfList;
}
```
## 面试题 37:序列化二叉树
&emsp;题目：请实现两个函数，分别用来序列化和反序列化二叉树。
```c++
namespace SerializeBinaryTrees {
void serialize(const BinaryTreeNode* pRoot, ostream& stream);
bool readStream(istream& stream, int* number);
void deserialize(BinaryTreeNode** pRoot, istream& stream);
}

// 序列化二叉树
void SerializeBinaryTrees::serialize(const BinaryTreeNode* pRoot, ostream& stream) {
    // 如果根节点是 nullptr，则输出一个 $ 并 return，
    // 同时它还是递归结束的的条件
    if (pRoot == nullptr) {
        stream << "$,";
        return;
    }
    
    // 输出节点的值和一个逗号
    stream << pRoot->m_nValue << ',';
    
    // 递归序列化左子树
    serialize(pRoot->m_pLeft, stream);
    
    // 递归序列化右子树
    serialize(pRoot->m_pRight, stream);
}

// readStream 每次从流中读出一个数字或者一个字符 '$'，
// 当从流中读出的是一个数字时，函数返回 true，否则返回 false
bool SerializeBinaryTrees::readStream(istream& stream, int* number) {
    // 流结束
    if(stream.eof())
        return false;
    
    // 长度是 32 的 char 数组
    char buffer[32];
    // 空字符
    buffer[0] = '\0';
    
    char ch;
    // 键盘输入到 ch 中
    stream >> ch;
    int i = 0;
    // 输入逗号表示一个完整的字符输入结束
    while(!stream.eof() && ch != ',') {
        buffer[i++] = ch;
        stream >> ch;
    }
    
    // 当输入 $ 时表示一个 nullptr 节点，否则就是正常的节点的值
    bool isNumeric = false;
    if(i > 0 && buffer[0] != '$') {
    
    // atoi (表示 ascii to integer) 是把字符串转换成整型数的一个函数。
    // int atoi(const char *nptr) 函数会扫描参数 nptr字符串，
    // 会跳过前面的空白字符（例如空格，tab缩进）等。
    // 如果 nptr 不能转换成 int 或者 nptr 为空字符串，那么将返回 0 [1]。
    // 特别注意，该函数要求被转换的字符串是按十进制数理解的。
    // atoi输入的字符串对应数字存在大小限制（与 int 类型大小有关），若其过大可能报错-1。
    
        *number = atoi(buffer);
        isNumeric = true;
    }
    
    return isNumeric;
}

// 反序列化二叉树
void SerializeBinaryTrees::deserialize(BinaryTreeNode** pRoot, istream& stream) {
    int number;
    // 注意这里 pRoot 是 BinaryTreeNode**
    // 如果读出了一个数字，则构建节点
    if (readStream(stream, &number)) {
        // 构建新节点
        *pRoot = new BinaryTreeNode();
        (*pRoot)->m_nValue = number;
        (*pRoot)->m_pLeft = nullptr;
        (*pRoot)->m_pRight = nullptr;
        
        // 传入左子节点的指针地址递归
        deserialize(&((*pRoot)->m_pLeft), stream);
        // 传入右子节点的指针地址递归
        deserialize(&((*pRoot)->m_pRight), stream);
    }
}
```
## 面试题 38:字符串的排列
&emsp;题目：输入一个字符串，打印出该字符串中字符的所有排列。例如输入字符串 abc，则打印出由字符 a、b、c 所能排列出来的所有字符串 abc、acb、bac、bca、cab 和 cba。
```c++
// 求整个字符串的排列可以分为两步。
// 第一步求所有可能出现在第一个位置的字符，即把第一个字符和后面所有的字符交换。
// 第二步固定第一个字符，求后面所有字符的排列。

//（a）把字符串分为两部分，一部分是字符串的第一个字符，另一部分是第一个字符以后的所有字符。
//（b）拿第一个字符和它后面的字符逐个交换。 

// 指针 pStr 指向整个字符串的第一个字符，pBegin 指向当前我们执行排列操作的字符串的第一个字符
void StringPermutation::permutation(char* pStr, char* pBegin) {
    if (*pBegin == '\0') {
        printf("%s\n", pStr);
    } else {
        for (char* pCh = pBegin; *pCh != '\0'; ++pCh) {
            char temp = *pCh;
            *pCh = *pBegin;
            *pBegin = temp;
            
            // 在每一次递归的时候，我们从 pBegin 向后扫描每一个字符（指针 pCh 指向的字符）
            // 在交换 pBegin 和 pCh 指向的字符之后，我们再对 pBegin 后面的字符串递归的进行排列操作，
            // 直至 pBegin 指向字符串的末尾
            permutation(pStr, pBegin + 1);
            
            temp = *pCh;
            *pCh = *pBegin;
            *pBegin = temp;
        }
    }
}

void StringPermutation::permutation(char* pStr) {
    if (pStr == nullptr) {
        return;
    }
    
    permutation(pStr, pStr);
}
```
## 39:数组中出现次数超过一半的数字
&emsp;题目：数组中有一个数字出现的次数超过数组长度的一半，请找出这个数字。例如输入一个长度为 9 的数组 {1, 2, 3, 2, 2, 2, 5, 4, 2} 。由于数字2在数组中出现了 5 次，超过数组长度的一半，因此输出 2。
```c++
namespace MoreThanHalfNumber {
static bool g_bInputInvalid = false;
bool checkInvalidArray(int* numbers, int length);
bool checkMoreThanHalf(int* numbers, int length, int number);

int moreThanHalfNum_Solution1(int* numbers, int length);
int moreThanHalfNum_Solution2(int* numbers, int length);
}

// 检测入参是否正确，正确与否记录在全局变量 g_bInputInvalid 中
bool MoreThanHalfNumber::checkInvalidArray(int* numbers, int length) {
    g_bInputInvalid = false;
    if (numbers == nullptr || length <= 0) {
        g_bInputInvalid = true;
    }
    
    return g_bInputInvalid;
}

// 检测 number 在 numbers 中出现次数是否超过了一半
bool MoreThanHalfNumber::checkMoreThanHalf(int* numbers, int length, int number) {
    int count = 0;
    for (unsigned int i = 0; i < length; ++i) {
        if (numbers[i] == number) {
            ++count;
        }
    }
    
    bool isMoreThanHalf = true;
    if (count * 2 <= length) {
        // 同时标记 g_bInputInvalid 入参无效
        g_bInputInvalid = true;
        isMoreThanHalf = false;
    }
    
    return isMoreThanHalf;
}

// 题目给的条件是有一个数字出现的次数超过了数组长度的一半，
// 那么对数组排序的话，在数组中间下标的元素一定就是这个数字，但是排序的话时间复杂度至少是 O(nlogn)。

// 这里使用前面快速排序中用到的 partition 函数，
// 它每次在数组中随机选中一个数字，把数组在逻辑上分成两组，左边都是小于该数字的元素，
// 右边都是大于该数字的元素，然后判断这个随机值的下标与数组中间下标，当随机值下标大于数组
// 中间下标时，缩小右边边界，当随机值小于中间下标时增大左边边界，类似二分查找。

// 解法 1 会修改原始数组。
int MoreThanHalfNumber::moreThanHalfNum_Solution1(int* numbers, int length) {
    if (checkInvalidArray(numbers, length)) {
        return 0;
    }
    
    // 数组中中间下标
    int middle = length >> 1;
    
    // 左边边界
    int start = 0;
    // 右边边界
    int end = length - 1;
    
    // 随机值下标
    int index = partition(numbers, length, start, end);
    
    // 一直进行随机，直到 index == middle
    while (index != middle) {
        // 如果 index 出现在 middle 右边，则缩小右边边界
        if (index > middle) {
            end = index - 1;
            index = partition(numbers, length, start, end);
        } else {
            // 如果 index 出现在 middle 左边，则增大左边边界
            start = index + 1;
            index = partition(numbers, length, start, end);
        }
    }
    
    // 取出中间位置数值
    int result = numbers[middle];
    
    // 检测 result 是否在数组中出现次数超过数组长度的一半
    if (!checkMoreThanHalf(numbers, length, result)) {
        result = 0;
    }
    
    return result;
}

// 解法 2 不修改原始数组。
// 再看一遍题目条件，数字出现的次数超过了数组长度的一半，那么从前到后遍历数组，统计出现连续相同数字的个数。
int MoreThanHalfNumber::moreThanHalfNum_Solution2(int* numbers, int length) {
    if (checkInvalidArray(numbers, length)) {
        return 0;
    }
    
    // 从第 0 个数字开始
    int result = numbers[0];
    
    // 第 0 个数字出现的次数是 1
    int times = 1;
    
    // 从第 1 个数字开始，遇到相同的数字 times 就自增，
    // 当遇到不同数字 times 就自减，当 times 自减到 0，
    // 然后遇到下一个新数字时，result 记录该数字，times 也置为 1，
    // 由于我们要找的数字出现的次数大于数组长度的一半，所以当数组遍历完毕，
    // times 必不为 0，此时的 result 就是我们要找到数字。
    for (unsigned int i = 1; i < length; ++i) {
        if (times == 0) {
            // 遇到新数字
            result = numbers[i];
            times = 1;
        } else if (numbers[i] == result) {
            // 遇到相同的数字 times 自增
            ++times;
        } else {
            // 遇到不同数字 times 自减
            --times;
        }
    }
    
    // 检测 result 是否合规
    if (!checkMoreThanHalf(numbers, length, result)) {
        result = 0;
    }
    
    return result;
}
```
## 面试题 40:最小的k个数
&emsp;题目：输入 n 个整数，找出其中最小的 k 个数。例如输入 4、5、1、6、2、7、3、8 这 8 个数字，则最小的 4 个数字是 1、2、3、4。
```c++
namespace KLeastNumbers {
void getLeastNumbers_Solution1(int* input, int n, int* output, int k);

typedef multiset<int, std::greater<int>>  intSet;
typedef multiset<int, std::greater<int>>::iterator setIterator;

void getLeastNumbers_Solution2(const vector<int>& data, intSet& leastNumbers, int k);
}

void KLeastNumbers::getLeastNumbers_Solution1(int* input, int n, int* output, int k) {
    if (input == nullptr || n <= 0 || output == nullptr || k <= 0 || k > n) {
        return;
    }
    
    int start = 0;
    int end = n - 1;
    
    int index = partition(input, n, start, end);
    
    // 调用前面的 partition 函数，找到前面最小的 k 个数，
    // 当 index 等于 k - 1 时，数组左边就是要找的 k 个数
    //（前 k 个数未必排序，但是它们是最小的 k 个数）
    
    while (index != k - 1) {
        if (index > k - 1) {
            end = index - 1;
            index = partition(input, n, start, end);
        } else {
            start = index + 1;
            index = partition(input, n, start, end);
        }
    }
    
    // 放进 output 数组输出
    for (unsigned int i = 0; i < k; ++i) {
        output[i] = input[i];
    }
}

void KLeastNumbers::getLeastNumbers_Solution2(const vector<int>& data, intSet& leastNumbers, int k) {
    leastNumbers.clear();
    
    if (k < 1 || data.size() < k) {
        return;
    }
    
    vector<int>::const_iterator iter = data.begin();
    
    // 从前往后遍历原始数组，leastNumbers 是一个限制元素个数为 k 的最大堆，
    // 当 leastNumbers 中元素少于 k 个时，直接把数组元素放进 leastNumbers，并会把它们构成一个最大堆，
    // 当 leastNumbers 中元素已满 k 个时，如果新来的数组元素小于大堆的最大值，则把堆此时的最大值移除，把这个新元素放入堆中。
    for (; iter != data.end(); ++iter) {
        if ((leastNumbers.size()) < k) {
            leastNumbers.insert(*iter);
        } else {
            
            // iterGreatest 是大堆的最大值
            setIterator iterGreatest = leastNumbers.begin();
            
            // 如果新来的数组元素小于大堆的最大值，则把堆此时的最大值移除，把新元素放入堆中
            if (*iter < *(leastNumbers.begin())) {
                // 移除之前的堆大值
                leastNumbers.erase(iterGreatest);
                
                // 插入新的元素
                leastNumbers.insert(*iter);
            }
        }
    }
}
```
## 完结撒花🎉🎉，感谢陪伴！
