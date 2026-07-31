#include "block.h"
#include "tree_sitter/alloc.h"
#include "tree_sitter/array.h"
#include "tree_sitter/parser.h"
#include <wctype.h>

enum TokenType { START_BLOCK_NAME, END_BLOCK_NAME };

typedef struct {
  Array(Block) blocks;
} Scanner;

void *tree_sitter_ftml_external_scanner_create() { return NULL; }

void tree_sitter_ftml_external_scanner_destroy(void *payload) {}

unsigned tree_sitter_ftml_external_scanner_serialize(void *payload,
                                                     char *buffer) {
  return 0;
}

void tree_sitter_ftml_external_scanner_deserialize(void *payload,
                                                   const char *buffer,
                                                   unsigned length) {}

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

  if ((valid_symbols[START_BLOCK_NAME] || valid_symbols[END_BLOCK_NAME])) {
    return valid_symbols[START_BLOCK_NAME]
               ? scan_start_block_name(scanner, lexer)
               : scan_end_block_name(scanner, lexer);
  }

  return false;
}
