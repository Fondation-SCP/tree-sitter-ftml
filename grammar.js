/**
 * @file Wikidot syntax
 * @author sty
 * @license AGPL-3.0
 */

/// <reference types="tree-sitter-cli/dsl" />
// @ts-check

export default grammar({
  name: "ftml",

  externals: $ => [
    $._start_block_name,
    $._end_block_name
  ],

  rules: {
    source_file: $ => repeat($._element),

    _element: $ => choice(
      $.freeform,
      $.block
    ),

    freeform: $ => /.+/, // Text that is outside all blocks

    block: $ => seq(
      $.block_start,
      /.*/,
      $.block_end
    ),

    block_start: $ => seq(
      '[[', alias($._start_block_name, $.block_name), ']]'
    ),

    block_end: $ => seq(
      '[[/', alias($._end_block_name,  $.block_name), ']]'
    )
  }
});
