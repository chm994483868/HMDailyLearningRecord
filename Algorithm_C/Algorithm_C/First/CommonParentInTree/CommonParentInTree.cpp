//
//  CommonParentInTree.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/26.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "CommonParentInTree.hpp"

namespace CommonParentInTree {
 
struct TreeNode {
    int m_nValue;
    std::vector<TreeNode*> m_vChildren;
};

bool getNodePath(const TreeNode* pRoot, const TreeNode* pNode, list<const TreeNode*>& path) {
    if (pRoot == pNode) {
        return true;
    }
    
    path.push_back(pRoot);
    
    bool found = false;
    
    vector<TreeNode*>::const_iterator i = pRoot->m_vChildren.begin();
    while (!found && i < pRoot->m_vChildren.end()) {
        found = getNodePath(*i, pNode, path);
        ++i;
    }
    
    if (!found) {
        path.pop_back();
    }
    
    return found;
}

const TreeNode* getLastCommonNode(const list<const TreeNode*>& path1, const list<const TreeNode*>& path2) {
    list<const TreeNode*>::const_iterator iterator1 = path1.begin();
    list<const TreeNode*>::const_iterator iterator2 = path2.begin();
    
    const TreeNode* pLast = nullptr;
    
    while (iterator1 != path1.end() && iterator2 != path2.end()) {
        if (*iterator1 == *iterator2) {
            pLast = *iterator1;
        }
        
        ++iterator1;
        ++iterator2;
    }
    
    return pLast;
}

const TreeNode* getLastCommonParent(const TreeNode* pRoot, const TreeNode* pNode1, const TreeNode* pNode2) {
    if (pRoot == nullptr || pNode1 == nullptr || pNode2 == nullptr) {
        return nullptr;
    }
    
    list<const TreeNode*> path1;
    getNodePath(pRoot, pNode1, path1);
    
    list<const TreeNode*> path2;
    getNodePath(pRoot, pNode2, path2);
    
    return getLastCommonNode(path1, path2);
}

}
