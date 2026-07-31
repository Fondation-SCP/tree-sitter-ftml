#ifndef TREE_SITTER_FTML_BLOCK_H
#define TREE_SITTER_FTML_BLOCK_H 1

#include "tree_sitter/array.h"

typedef enum { SIZE, NONE_ } BlockType;

typedef struct {
  BlockType type;
} Block;

typedef Array(char) String;

typedef struct {
  char block_name[16];
  BlockType block_type;
} BlockMapEntry;

static const BlockMapEntry BLOCK_TYPES_BY_BLOCK_NAME[] = {{"SIZE", SIZE}};

static inline Block block_new() {
  Block block;
  block.type = NONE_;
  return block;
}

static BlockType block_type_for_name(const String *block_name) {
  for (int i = 0; i < 126; i++) {
    const BlockMapEntry *entry = &BLOCK_TYPES_BY_BLOCK_NAME[i];
    if (strlen(entry->block_name) == block_name->size &&
        memcmp(block_name->contents, entry->block_name, block_name->size) ==
            0) {
      return entry->block_type;
    }
  }
  return NONE_;
}

static inline Block block_for_name(String name) {
  Block block = block_new();
  block.type = block_type_for_name(&name);
  array_delete(&name);

  return block;
}

static inline bool block_eq(const Block *self, const Block *other) {
  return self->type == other->type;
}

static inline void block_free(Block *block) {}

#endif /* TREE_SITTER_FTML_BLOCK_H */
