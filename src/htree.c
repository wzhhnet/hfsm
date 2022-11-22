/********** Copyright(C) 2021 MXNavi Co.,Ltd. ALL RIGHTS RESERVED **********/
/****************************************************************************
*   FILE NAME   : htree.c
*   CREATE DATE : 2021-12-16
*   MODULE      : Hierarchy tree
*   AUTHOR      : wanch
*---------------------------------------------------------------------------*
*   MEMO        :
*****************************************************************************/
#include "log.h"
#include "htree.h"

#define EMPTY_STACK         (-1)
#define TREE_STACK_MAX_SIZE (128)

struct treestack {
    int top;    /*!> -1 if stack is empty  */
    struct treenode *buf[TREE_STACK_MAX_SIZE];
};

static void push_tree_stack(struct treestack *stack, struct treenode *node) {

    if (stack->top < TREE_STACK_MAX_SIZE - 1) {
        stack->buf[++stack->top] = node;
    }
}

static struct treenode* pop_tree_stack(struct treestack *stack) {
    if (stack->top >= 0 && stack->top < TREE_STACK_MAX_SIZE) {
        return stack->buf[stack->top--];
    }
    return NULL;
}

static void init_stack(struct treestack *stack) {
    if (stack) {
        stack->top = EMPTY_STACK;
        memset(stack->buf, 0, sizeof(stack->buf));
    }
}

bool is_empty_tree(struct treeroot *root) {
    return root->root ? false : true;
}

void tree_init(struct treenode *node)
{
    node->parent = NULL;
    list_init(&(node->children));
}

void tree_add_node(struct treenode *parent, struct treenode *child)
{
    child->parent = parent;
    list_add_tail(&(parent->children),  &(child->sibling));
}

struct treenode* tree_forefathers(struct treeroot *root, struct treenode *n1, struct treenode *n2) {

    struct treenode *p1, *p2;

    if (!root || !n1 || !n2)
        return NULL;

    p1 = n1;
    while (p1) {
        p2 = n2;
        while (p2) {
            if (p1 == p2)
                goto found;
            p2 = p2->parent;
        }
        p1 = p1->parent;
    }

found:
    ASSERT(p1 == p2);
    return p1;
}

struct treenode* tree_find_node(struct treeroot *root, callback_fn func, void *fnArg) {

    struct treestack stack;
    struct listnode *child, *children;
    struct treenode *n;

    if (!root)
        return NULL;

    n = root->root;
    init_stack(&stack);

    if (n)
        push_tree_stack(&stack, n);

    while (stack.top != EMPTY_STACK) {
        n = pop_tree_stack(&stack);
        if (func && func(n, fnArg)) {
            return n;
        }

        /*! push all children */
        children = &n->children;
        list_for_each_reverse(child, children) {
            n = node_to_item(child, struct treenode, sibling);
            push_tree_stack(&stack, n);
        }
    }
    return NULL;
}

void tree_traverse_downward(struct treenode* up, struct treenode* down, callback_fn func, void *fnArg) {

    struct treenode* n;
    struct treestack stack;
    if (!up || !down || !func || !fnArg)
        return;

    init_stack(&stack);
    n = down;
    while (n) {
        if (n == up)
            goto succ;
        push_tree_stack(&stack, n);
        n = n->parent;
    };

    return;

succ:

    n = pop_tree_stack(&stack);
    while (n) {
        if (!func(n, fnArg))
            break;
        n = pop_tree_stack(&stack);
    };

}


