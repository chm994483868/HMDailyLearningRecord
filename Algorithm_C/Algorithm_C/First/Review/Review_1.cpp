//
//  Review_1.cpp
//  Algorithm_C
//
//  Created by HM C on 2020/7/11.
//  Copyright © 2020 CHM. All rights reserved.
//

#include <stdio.h>
#include <memory>
#include <stack>
#include <queue>

using namespace std;

namespace Review_1 {

// 01. 冒泡排序 O(n*n)
void bubbleSort(int nums[], int count) {
    int k = count - 1;
    for (int i = 0; i < count - 1; i++) {
        bool noExchange = true;
        int n = 0;
        for (int j = 0; j < k; j++) {
            if (nums[j] > nums[j + 1]) {
                swap(nums[j], nums[j + 1]);
                noExchange = false;
                n = j;
            }
        }
        
        if (noExchange) break;
        k = n;
    }
}

// 02. 插入排序 O(n*n)
void insertSort(int nums[], int count) {
    for (int i = 1; i < count; i++) {
        for (int j = i; j > 0 && nums[j - 1] > nums[j]; j--) {
            swap(nums[j - 1], nums[j]);
        }
    }
}

// 03. 选择排序 O(n*n)
void selectSort(int nums[], int count) {
    for (int i = 0; i < count; i++) {
        int minIndex = i;
        for (int j = i + 1; j < count; j++) {
            if (nums[j] < nums[minIndex]) {
                minIndex = j;
            }
        }
        swap(nums[i], nums[minIndex]);
    }
}

// 04. 希尔排序 O(n*n)
void shellSort(int nums[], int count) {
    for (int gap = count / 2; gap > 0; gap /= 2) {
        for (int i = 0; i < gap; i++) {
            for (int j = i + gap; j < count; j += gap) {
                for (int k = j; k > 0 && nums[k - gap] > nums[k]; k -= gap) {
                    swap(nums[k - gap], nums[k]);
                }
            }
        }
    }
}

// 05. 希尔排序优化 O(n*n)
void shellSortOptimization(int nums[], int count) {
    for (int gap = count / 2; gap > 0; gap /= 2) {
        for (int i = gap; i < count; i++) {
            for (int j = i; j > 0 && nums[j - gap] > nums[j]; j -= gap) {
                swap(nums[j - gap], nums[j]);
            }
        }
    }
}

// 06. 快速排序 O(n*logn)
void quickSort(int nums[], int l, int r) {
    if (l >= r) {
        return;
    }
    
    int i = l, j = r, x = nums[l];
    while (i < j) {
        while (i < j && nums[j] >= x) j--;
        if (i < j) nums[i++] = nums[j];
        
        while (i < j && nums[i] < x) i++;
        if (i < j) nums[j--] = nums[i];
    }
    
    nums[i] = x;
    quickSort(nums, l, i - 1);
    quickSort(nums, i + 1, r);
}

// 07. 归并排序 O(n*logn)
void mergeArray(int nums[], int first, int mid, int last, int temp[]);

void mergeSort(int nums[], int first, int last, int temp[]) {
    if (first <= last) {
        return;
    }
    
    int mid = (first + last) / 2;
    
    mergeSort(nums, first, mid, temp);
    mergeSort(nums, mid + 1, last, temp);
    
    mergeArray(nums, first, mid, last, temp);
}

void mergeArray(int nums[], int first, int mid, int last, int temp[]) {
    int i = first, j = mid + 1;
    int m = mid, n = last;
    int k = 0;
    
    while (i <= m && j <= n) {
        if (nums[i] <= nums[j]) {
            temp[k++] = nums[i++];
        } else {
            temp[k++] = nums[j++];
        }
    }
    
    while (i <= m) {
        temp[k++] = nums[i++];
    }
    
    while (j <= n) {
        temp[k++] = nums[j++];
    }
    
    for (i = 0; i < k; i++) {
        nums[first + i] = temp[i];
    }
}

// 08. 堆排序 O(n*logn)
void MaxHeapFixdown(int nums[], int i, int n);

void heapSort(int nums[], int count) {
    for (int i = ((count - 1) - 1) / 2; i >= 0; i--) {
        MaxHeapFixdown(nums, i, count);
    }
    
    for (int i = count - 1; i >= 1; i--) {
        swap(nums[i], nums[0]);
        MaxHeapFixdown(nums, 0, i);
    }
}

void MaxHeapFixdown(int nums[], int i, int n) {
    int j = i * 2 + 1;
    int temp = nums[i];
    
    while (j < n) {
        if (j + 1 < n && nums[j + 1] > nums[j]) {
            j++;
        }
        
        if (nums[j] <= temp) {
            break;
        }
        
        nums[i] = nums[j];
        i = j;
        j = i * 2 + 1;
    }
    
    nums[i] = temp;
}

// 09. 堆排序（素燕）O(n*logn)
void heapify(int nums[], int i, int n) {
    if (i >= n) {
        return;
    }
    
    int c1 = i * 2 + 1;
    int c2 = i * 2 + 2;
    int max = i;
    
    if (c1 < n && nums[c1] > nums[max]) {
        max = c1;
    }
    
    if (c2 < n && nums[c2] > nums[max]) {
        max = c2;
    }
    
    if (max != i) {
        swap(nums[i], nums[max]);
        heapify(nums, max, n);
    }
}

// 10. 反转二叉树（递归）
struct TreeNode {
    int m_nValue;
    TreeNode* m_pLeft;
    TreeNode* m_pRight;
    
    TreeNode(int x) : m_nValue(x), m_pLeft(nullptr), m_pRight(nullptr) {}
};

TreeNode* invertTree(TreeNode* root) {
    if (root == nullptr)
        return root;
    
    swap(root->m_pLeft, root->m_pRight);
    
    if (root->m_pLeft != nullptr) invertTree(root->m_pLeft);
    if (root->m_pRight != nullptr) invertTree(root->m_pRight);
    
    return root;
}

// 11. 反转二叉树（栈或者队列）
TreeNode* invertTreeWithStack(TreeNode* root) {
    if (root == nullptr)
        return root;
    
    stack<TreeNode*> treeStack;
    treeStack.push(root);
    
    while (!treeStack.empty()) {
        TreeNode* top = treeStack.top();
        treeStack.pop();
        
        swap(top->m_pLeft, top->m_pRight);
        
        if (top->m_pLeft != nullptr) treeStack.push(top->m_pLeft);
        if (top->m_pRight != nullptr) treeStack.push(top->m_pRight);
    }
    
    return root;
}

TreeNode* invertTreeWithQueue(TreeNode* root) {
    if (root == nullptr)
        return root;
    
    queue<TreeNode*> treeQueue;
    treeQueue.push(root);
    
    while (!treeQueue.empty()) {
        TreeNode* front = treeQueue.front();
        treeQueue.pop();
        
        swap(front->m_pLeft, front->m_pRight);
        
        if (front->m_pLeft != nullptr) treeQueue.push(front->m_pLeft);
        if (front->m_pRight != nullptr) treeQueue.push(front->m_pRight);
    }
    
    return root;
}

}
