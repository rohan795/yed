#include "frame_tree.h"

void yed_init_frame_trees(void) {
    ys->frame_trees = array_make(yed_frame_tree*);
}

static yed_frame_tree * yed_new_frame_tree(void) {
    yed_frame_tree *new_tree;

    new_tree = malloc(sizeof(*new_tree));
    memset(new_tree, 0, sizeof(*new_tree));

    new_tree->is_leaf = 1;

    array_push(ys->frame_trees, new_tree);

    return new_tree;
}

static void yed_delete_frame_tree(yed_frame_tree *tree) {
    yed_frame_tree **it;
    int              i;

    i = 0;

    array_traverse(ys->frame_trees, it) {
        if (*it == tree) {
            free(tree);
            array_delete(ys->frame_trees, i);
            return;
        }
        i += 1;
    }
}

void yed_frame_tree_set_relative_rect(yed_frame_tree *tree, float top, float left, float height, float width) {
    tree->top    = top;
    tree->left   = left;
    tree->height = height;
    tree->width  = width;
}

static void yed_frame_tree_set_child(yed_frame_tree *parent, yed_frame_tree *child, int which) {
    if (parent->is_leaf) {
        parent->frame   = (void*)(parent->child_trees[0] = parent->child_trees[1] = NULL);
        parent->is_leaf = 0;
    }

    parent->child_trees[which] = child;
    child->parent              = parent;
}




yed_frame_tree * yed_frame_tree_add_child(yed_frame_tree *parent, yed_frame *f, int which) {
    yed_frame_tree_set_relative_rect(f->tree, f->top_f, f->left_f, f->height_f, f->width_f);

    if (parent) {
        yed_frame_tree_set_child(parent, f->tree, which);
    }

    return f->tree;
}

void yed_frame_tree_remove_from_tree(yed_frame_tree *tree) {
    yed_frame_tree *p;

    p = tree->parent;

    if (p == NULL) {
        yed_delete_frame_tree(tree);
        return;
    }

    ASSERT(!p->is_leaf, "tree is a parent, but is marked as a leaf");

    if (p->child_trees[0] == tree) {
        p->child_trees[0] = NULL;
    } else if (p->child_trees[1] == tree) {
        p->child_trees[1] = NULL;
    }
}

void yed_frame_tree_leaves_do(yed_frame_tree *tree, yed_frame_tree_leaf_do_fn_t fn, void *arg) {
    if (tree->is_leaf) {
        fn(tree, arg);
        return;
    }

    yed_frame_tree_leaves_do(tree->child_trees[0], fn, arg);
    yed_frame_tree_leaves_do(tree->child_trees[1], fn, arg);
}

yed_frame_tree *yed_frame_tree_add_root(struct yed_frame_t *f) {
    yed_frame_tree *root;

    root = yed_new_frame_tree();
    root->frame = f;
    yed_frame_tree_set_relative_rect(root, f->top_f, f->left_f, f->height_f, f->width_f);

    return root;
}

yed_frame_tree *yed_frame_tree_vsplit(yed_frame_tree *tree) {
    yed_frame_tree *new_tree;
    yed_frame      *f;

    new_tree = yed_new_frame_tree();

    if (tree->is_leaf) {
        f = tree->frame;
        yed_frame_tree_set_relative_rect(new_tree, f->tree->top, f->tree->left, f->tree->height, f->tree->width);
    } else {
        yed_frame_tree_set_relative_rect(new_tree, tree->top, tree->left, tree->height, tree->width);
    }

    new_tree->parent = tree->parent;
    if (new_tree->parent) {
        new_tree->parent->child_trees[tree != new_tree->parent->child_trees[0]] = new_tree;
    }

    new_tree->split_kind = FTREE_VSPLIT;

    if (tree->is_leaf) {
        yed_frame_tree_add_child(new_tree, tree->frame, 0);
    } else {
        yed_frame_tree_set_child(new_tree, tree, 0);
    }

    yed_frame_tree_add_child(new_tree, yed_add_new_frame_full(), 1);

    yed_frame_tree_set_relative_rect(new_tree->child_trees[0], 0.0, 0.0, 1.0, 0.5);
    yed_frame_tree_set_relative_rect(new_tree->child_trees[1], 0.0, 0.5, 1.0, 0.5);

    yed_frame_tree_recursive_readjust(new_tree);

    return new_tree;
}

yed_frame_tree *yed_frame_tree_hsplit(yed_frame_tree *tree) {
    yed_frame_tree *new_tree;
    yed_frame      *f;

    new_tree = yed_new_frame_tree();

    if (tree->is_leaf) {
        f = tree->frame;
        yed_frame_tree_set_relative_rect(new_tree, f->tree->top, f->tree->left, f->tree->height, f->tree->width);
    } else {
        yed_frame_tree_set_relative_rect(new_tree, tree->top, tree->left, tree->height, tree->width);
    }

    new_tree->parent = tree->parent;
    if (new_tree->parent) {
        new_tree->parent->child_trees[tree != new_tree->parent->child_trees[0]] = new_tree;
    }

    new_tree->split_kind = FTREE_HSPLIT;

    if (tree->is_leaf) {
        yed_frame_tree_add_child(new_tree, tree->frame, 0);
    } else {
        yed_frame_tree_set_child(new_tree, tree, 0);
    }

    yed_frame_tree_add_child(new_tree, yed_add_new_frame_full(), 1);

    yed_frame_tree_set_relative_rect(new_tree->child_trees[0], 0.0, 0.0, 0.5, 1.0);
    yed_frame_tree_set_relative_rect(new_tree->child_trees[1], 0.5, 0.0, 0.5, 1.0);

    yed_frame_tree_recursive_readjust(new_tree);

    return new_tree;
}

void yed_frame_tree_delete_leaf(yed_frame_tree *tree) {
    yed_frame_tree *p;
    yed_frame_tree *other_child;

    if (!tree->is_leaf) { return; }

    /*
     * If it's just a root tree -- i.e. single frame with no children,
     * then we just remove it.
     */
    if ((p = tree->parent) == NULL) { goto out_delete; }

    other_child = p->child_trees[tree == p->child_trees[0]];

    ASSERT(other_child, "leaf's parent doesn't have two children");

    yed_frame_tree_set_relative_rect(other_child, p->top, p->left, p->height, p->width);

    /* Replace p with other_child in p's parent tree. */
    other_child->parent = p->parent;
    if (p->parent != NULL) {
        p->parent->child_trees[p != p->parent->child_trees[0]]
            = other_child;
    }

    yed_delete_frame_tree(p);

    yed_frame_tree_recursive_readjust(other_child);

out_delete:
    yed_delete_frame_tree(tree);
}

yed_frame_tree *yed_frame_tree_get_root(yed_frame_tree *tree) {
    while (tree->parent != NULL) {
        tree = tree->parent;
    }

    return tree;
}

int yed_frame_tree_is_root(yed_frame_tree *tree) {
    return (tree->parent == NULL);
}

void yed_frame_tree_get_absolute_rect(yed_frame_tree *tree, float *top, float *left, float *height, float *width) {
    array_t          tree_arr;
    yed_frame_tree **tree_it;

    tree_arr = array_make(yed_frame_tree*);

    while (tree->parent) {
        array_push(tree_arr, tree);
        tree = tree->parent;
    }

    *top    = tree->top;
    *left   = tree->left;
    *height = tree->height;
    *width  = tree->width;

    array_rtraverse(tree_arr, tree_it) {
        *top    += ((*tree_it)->top  * *height);
        *left   += ((*tree_it)->left * *width);
        *height *= (*tree_it)->height;
        *width  *= (*tree_it)->width;
    }

    array_free(tree_arr);
}

static void _yed_frame_tree_recursive_readjust(yed_frame_tree *tree, float atop, float aleft, float aheight, float awidth) {
    float           new_top;
    float           new_left;
    float           new_height;
    float           new_width;
    yed_frame_tree *other;
    yed_frame_tree *other_leaf;

    if (tree->parent) {
        new_top    = atop + (tree->top * aheight);
        new_left   = aleft + (tree->left * awidth);
        new_height = tree->height * aheight;
        new_width  = tree->width * awidth;
    } else {
        new_top    = atop;
        new_left   = aleft;
        new_height = aheight;
        new_width  = awidth;
    }

    /* Sneaky hacks to make the borders merge. */
    if (tree->parent && tree == tree->parent->child_trees[1]) {
        other      = tree->parent->child_trees[0];
        other_leaf = yed_frame_tree_get_split_leaf_prefer_right_or_bottommost(tree);

        ASSERT(other_leaf->is_leaf, "failed to find other leaf");

        if (other == other_leaf) {
            if (tree->parent->split_kind == FTREE_VSPLIT) {
                new_left  = (float)(other_leaf->frame->left + other_leaf->frame->width - 1)
                                / (float)(ys->term_cols);
            } else if (tree->parent->split_kind == FTREE_HSPLIT) {
                new_top    = (float)(other_leaf->frame->top + other_leaf->frame->height - 1)
                                / (float)(ys->term_rows - 2);
            }
        } else {
            if (tree->parent->split_kind == FTREE_VSPLIT) {
                new_left  = (float)(other_leaf->frame->left + other_leaf->frame->width - 1)
                                / (float)(ys->term_cols);
            } else if (tree->parent->split_kind == FTREE_HSPLIT) {
                new_top    = (float)(other_leaf->frame->top + other_leaf->frame->height - 1)
                                / (float)(ys->term_rows - 2);
            }
        }
    }

    if (tree->parent == NULL) {
        tree->discrete_rows = MAX(roundf(aheight * (ys->term_rows - 2)), 1);
        tree->discrete_cols = MAX(roundf(awidth * ys->term_cols), 1);
    } else if (tree == tree->parent->child_trees[0]) {
        tree->discrete_rows = MAX(roundf(tree->height * tree->parent->discrete_rows), 1);
        tree->discrete_cols = MAX(roundf(tree->width  * tree->parent->discrete_cols), 1);
    } else {
        other = tree->parent->child_trees[0];
        if (tree->parent->split_kind == FTREE_VSPLIT) {
            tree->discrete_rows = other->discrete_rows;
            tree->discrete_cols = tree->parent->discrete_cols - other->discrete_cols + 1;

            new_width = (float)tree->discrete_cols / (float)ys->term_cols;
        } else {
            tree->discrete_rows = tree->parent->discrete_rows - other->discrete_rows + 1;
            tree->discrete_cols = other->discrete_cols;

            new_height = (float)tree->discrete_rows / (float)(ys->term_rows - 2);
        }
    }

    if (tree->is_leaf) {
        tree->frame->top_f    = new_top;
        tree->frame->left_f   = new_left;
        tree->frame->height_f = new_height;
        tree->frame->width_f  = new_width;

        FRAME_RESET_RECT(tree->frame);

        while (tree->frame->bwidth > tree->discrete_cols) {
            tree->frame->width_f -= 1.0 / ys->term_cols;
            FRAME_RESET_RECT(tree->frame);
        }
        while (tree->frame->bheight > tree->discrete_rows) {
            tree->frame->height_f -= 1.0 / (ys->term_rows - 2);
            FRAME_RESET_RECT(tree->frame);
        }
    } else {
        _yed_frame_tree_recursive_readjust(tree->child_trees[0], new_top, new_left, new_height, new_width);
        _yed_frame_tree_recursive_readjust(tree->child_trees[1], new_top, new_left, new_height, new_width);
    }
}

void yed_frame_tree_recursive_readjust(yed_frame_tree *tree) {
    yed_frame_tree *root;

    root = yed_frame_tree_get_root(tree);

    _yed_frame_tree_recursive_readjust(root, root->top, root->left, root->height, root->width);
}

yed_frame_tree *yed_frame_tree_get_split_leaf_prefer_left_or_topmost(yed_frame_tree *tree) {
    if (tree->parent == NULL) {
        return NULL;
    }

    tree = tree->parent->child_trees[tree->parent->child_trees[0] == tree];

    while (!tree->is_leaf) {
        tree = tree->child_trees[0];
    }

    return tree;
}

yed_frame_tree *yed_frame_tree_get_split_leaf_prefer_right_or_bottommost(yed_frame_tree *tree) {
    if (tree->parent == NULL) {
        return NULL;
    }

    tree = tree->parent->child_trees[tree->parent->child_trees[0] == tree];

    while (!tree->is_leaf) {
        tree = tree->child_trees[1];
    }

    return tree;
}

void yed_frame_tree_swap_children(yed_frame_tree *tree) {
    yed_frame_tree *left_child;
    float           top, left, height, width;

    if (tree->is_leaf) { return; }

    left_child = tree->child_trees[0];
    top        = tree->child_trees[0]->top;
    left       = tree->child_trees[0]->left;
    height     = tree->child_trees[0]->height;
    width      = tree->child_trees[0]->width;

    yed_frame_tree_set_relative_rect(tree->child_trees[0],
                                     tree->child_trees[1]->top,    tree->child_trees[1]->left,
                                     tree->child_trees[1]->height, tree->child_trees[1]->width);

    yed_frame_tree_set_relative_rect(tree->child_trees[1], top,    left,
                                                           height, width);

    tree->child_trees[0] = tree->child_trees[1];
    tree->child_trees[1] = left_child;

    yed_frame_tree_recursive_readjust(tree);
}
