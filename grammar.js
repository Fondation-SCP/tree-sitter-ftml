/**
 * @file Wikidot syntax
 * @author sty
 * @license AGPL-3.0
 */

/// <reference types="tree-sitter-cli/dsl" />
// @ts-check

export default grammar({
  name: "ftml",

  rules: {
    // TODO: add the actual grammar rules
    source_file: $ => "hello"
  }
});
