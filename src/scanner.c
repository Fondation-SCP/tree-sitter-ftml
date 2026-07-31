#include "block.h"
#include "tree_sitter/alloc.h"
#include "tree_sitter/array.h"
#include "tree_sitter/parser.h"
#include <wctype.h>

enum TokenType { START_BLOCK_NAME, END_BLOCK_NAME };

typedef struct {
  Array(Block) blocks;
} Scanner;

void *tree_sitter_ftml_external_scanner_create() {
  Scanner *scanner = (Scanner *)ts_calloc(1, sizeof(Scanner));
  return scanner;
}

void tree_sitter_ftml_external_scanner_destroy(void *payload) {
  Scanner *scanner = (Scanner *)payload;
  for (unsigned i = 0; i < scanner->blocks.size; i++) {
    block_free(&scanner->blocks.contents[i]);
  }
  array_delete(&scanner->blocks);
  ts_free(scanner);
}

unsigned tree_sitter_ftml_external_scanner_serialize(void *payload,
                                                     char *buffer) {
  Scanner *scanner = (Scanner *)payload;
  uint16_t block_count =
      scanner->blocks.size > UINT16_MAX ? UINT16_MAX : scanner->blocks.size;
  uint16_t serialized_block_count = 0;

  unsigned size = sizeof(block_count);
  memcpy(&buffer[size], &block_count, sizeof(block_count));
  size += sizeof(block_count);

  for (; serialized_block_count < block_count; serialized_block_count++) {
    Block block = scanner->blocks.contents[serialized_block_count];

    if (size + 1 >= TREE_SITTER_SERIALIZATION_BUFFER_SIZE) {
      break;
    }
    buffer[size++] = (char)block.type;
  }

  memcpy(&buffer[0], &serialized_block_count, sizeof(serialized_block_count));
  return size;
}

void tree_sitter_ftml_external_scanner_deserialize(void *payload,
                                                   const char *buffer,
                                                   unsigned length) {
  Scanner *scanner = (Scanner *)payload;
  for (unsigned i = 0; i < scanner->blocks.size; i++) {
    block_free(&scanner->blocks.contents[i]);
  }
  array_clear(&scanner->blocks);

  if (length > 0) {
    unsigned size = 0;
    uint16_t block_count = 0;
    uint16_t serialized_block_count = 0;

    memcpy(&serialized_block_count, &buffer[size],
           sizeof(serialized_block_count));
    size += sizeof(serialized_block_count);

    memcpy(&block_count, &buffer[size], sizeof(block_count));
    size += sizeof(block_count);

    array_reserve(&scanner->blocks, block_count);
    if (block_count > 0) {
      unsigned iter = 0;
      for (iter = 0; iter < serialized_block_count; iter++) {
        Block block = block_new();
        block.type = (BlockType)buffer[size++];
        array_push(&scanner->blocks, block);
      }
      // add zero blocks if we didn't read enough, this is because the
      // buffer had no more room but we held more blocks.
      for (; iter < block_count; iter++) {
        array_push(&scanner->blocks, block_new());
      }
    }
  }
}

static inline void advance(TSLexer *lexer) { lexer->advance(lexer, false); }

static inline void skip(TSLexer *lexer) { lexer->advance(lexer, true); }

static void pop_block(Scanner *scanner) {
  Block popped_block = array_pop(&scanner->blocks);
  block_free(&popped_block);
}

static String scan_block_name(TSLexer *lexer) {
  String tag_name = array_new();
  while (iswalnum(lexer->lookahead)) {
    array_push(&tag_name, towupper(lexer->lookahead));
    advance(lexer);
  }
  return tag_name;
}

static bool scan_start_block_name(Scanner *scanner, TSLexer *lexer) {
  String block_name = scan_block_name(lexer);
  if (block_name.size == 0) {
    array_delete(&block_name);
    return false;
  }

  Block block = block_for_name(block_name);
  array_push(&scanner->blocks, block);
  lexer->result_symbol = START_BLOCK_NAME;

  return true;
}

static bool scan_end_block_name(Scanner *scanner, TSLexer *lexer) {
  String block_name = scan_block_name(lexer);

  if (block_name.size == 0) {
    array_delete(&block_name);
    return false;
  }

  Block block = block_for_name(block_name);
  if (scanner->blocks.size > 0 &&
      block_eq(array_back(&scanner->blocks), &block)) {
    pop_block(scanner);
    lexer->result_symbol = END_BLOCK_NAME;
  } else {
    return false;
  }

  block_free(&block);
  return true;
}

bool tree_sitter_ftml_external_scanner_scan(void *payload, TSLexer *lexer,
                                            const bool *valid_symbols) {
  Scanner *scanner = (Scanner *)payload;
  while (iswspace(lexer->lookahead)) {
    skip(lexer);
  }

  if (valid_symbols[START_BLOCK_NAME]) {
    return scan_start_block_name(scanner, lexer);
  } else if (valid_symbols[END_BLOCK_NAME]) {
    return scan_end_block_name(scanner, lexer);
  }

  return false;
}
