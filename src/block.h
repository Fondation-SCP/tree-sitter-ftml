#ifndef TREE_SITTER_FTML_BLOCK_H
#define TREE_SITTER_FTML_BLOCK_H 1

#include "tree_sitter/array.h"

typedef enum { SIZE } BlockType;

typedef struct {
  BlockType type;
} Block;

#endif /* TREE_SITTER_FTML_BLOCK_H */
