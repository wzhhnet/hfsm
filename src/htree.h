/********** Copyright(C) 2021 MXNavi Co.,Ltd. ALL RIGHTS RESERVED **********/
/****************************************************************************
*   FILE NAME   : htree.h
*   CREATE DATE : 2021-12-16
*   MODULE      : Hierarchy tree
*   AUTHOR      : wanch
*---------------------------------------------------------------------------*
*   MEMO        :
*****************************************************************************/
#ifndef _HFSM_HTREE_H
#define _HFSM_HTREE_H

#ifdef __cplusplus
extern "C" {
#endif
/****************************************************************************
*                       Include File Section                                *
*****************************************************************************/
#include "types.h"
#include "list.h"

/****************************************************************************
*                       Macro Definition Section                            *
*****************************************************************************/
struct treenode {
    struct treenode *parent;
    struct listnode  sibling;
    struct listnode  children;
};

struct treeroot {
    struct treenode *root;
};

typedef bool (*callback_fn)(struct treenode*, void*);

bool is_empty_tree(struct treeroot *root);
void tree_init(struct treenode *node);
void tree_add_node(struct treenode *parent, struct treenode *child);
struct treenode* tree_forefathers(struct treeroot *root, struct treenode *n1, struct treenode *n2);
struct treenode* tree_find_node(struct treeroot *root, callback_fn func, void *fnArg);
void tree_traverse_downward(struct treenode* up, struct treenode* down, callback_fn func, void *fnArg);


#ifdef __cplusplus
}
#endif


#endif /*! _HFSM_HTREE_H */

