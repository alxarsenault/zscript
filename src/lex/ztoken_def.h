ZS_DECL_TOKEN(first_named_token)
ZS_DECL_TOKEN(none)
ZS_DECL_TOKEN(null)
ZS_DECL_TOKEN(var)
ZS_DECL_TOKEN(int)
ZS_DECL_TOKEN(float)
ZS_DECL_TOKEN(bool)
ZS_DECL_TOKEN(table)
ZS_DECL_TOKEN(array)
ZS_DECL_TOKEN(string)
ZS_DECL_TOKEN(exttype) // extend
ZS_DECL_TOKEN(char)
ZS_DECL_TOKEN(auto)
ZS_DECL_TOKEN(true) // true
ZS_DECL_TOKEN(false) // false
ZS_DECL_TOKEN(return) // return
ZS_DECL_TOKEN(extend) // extend
ZS_DECL_TOKEN(struct) // struct.
ZS_DECL_TOKEN(number) // number.

ZS_DECL_TOKEN(and) // &&
ZS_DECL_TOKEN(or) // ||
ZS_DECL_TOKEN(triple_or) // |||
ZS_DECL_TOKEN(if) // if
ZS_DECL_TOKEN(else) // else
ZS_DECL_TOKEN(for) // for
ZS_DECL_TOKEN(foreach) // for
ZS_DECL_TOKEN(do) // do
ZS_DECL_TOKEN(while) // while
ZS_DECL_TOKEN(switch) // switch
ZS_DECL_TOKEN(case) // case
ZS_DECL_TOKEN(break) // break
ZS_DECL_TOKEN(default) // default
ZS_DECL_TOKEN(continue) // continue
ZS_DECL_TOKEN(try) // try
ZS_DECL_TOKEN(catch) // catch
ZS_DECL_TOKEN(throw) // throw
ZS_DECL_TOKEN(this) // this
ZS_DECL_TOKEN(base) // base
ZS_DECL_TOKEN(class) // class
ZS_DECL_TOKEN(namespace) // namespace
ZS_DECL_TOKEN(use) // use
ZS_DECL_TOKEN(global) // global
ZS_DECL_TOKEN(const) // const
ZS_DECL_TOKEN(static) // static
ZS_DECL_TOKEN(private) // private
// ZS_DECL_TOKEN(export) // export
ZS_DECL_TOKEN(function) // function
ZS_DECL_TOKEN(typeof) // typeof
ZS_DECL_TOKEN(typeid) // typeid
ZS_DECL_TOKEN(constructor) // constructor
ZS_DECL_TOKEN(destructor) // destructor
ZS_DECL_TOKEN(mutable) // mutable
ZS_DECL_TOKEN(in) // in
ZS_DECL_TOKEN(enum) // enum
ZS_DECL_TOKEN(not ) // !
ZS_DECL_TOKEN(xor) // ^ or xor
ZS_DECL_TOKEN(include) // include or #include
ZS_DECL_TOKEN(import) // import or #import
ZS_DECL_TOKEN(expr) // @expr
ZS_DECL_TOKEN(module) // @module
ZS_DECL_TOKEN(author) // @author
ZS_DECL_TOKEN(brief) // @brief
ZS_DECL_TOKEN(version) // @version
ZS_DECL_TOKEN(date) // @date
ZS_DECL_TOKEN(copyright) // @copyright
ZS_DECL_TOKEN(counter) // @counter
ZS_DECL_TOKEN(uuid) // @uuid
ZS_DECL_TOKEN(stringify) // @str
ZS_DECL_TOKEN(keyword) // @keyword

//
ZS_DECL_TOKEN(last_named_token)

//
ZS_DECL_TOKEN(integer_value)
ZS_DECL_TOKEN(char_value)
ZS_DECL_TOKEN(float_value)
ZS_DECL_TOKEN(string_value)
ZS_DECL_TOKEN(escaped_string_value)
// ZS_DECL_TOKEN(char_value)
ZS_DECL_TOKEN(identifier)
ZS_DECL_TOKEN(file) // __FILE__
ZS_DECL_TOKEN(line) // __LINE__
ZS_DECL_TOKEN(line_str) // __LINE_STR__
ZS_DECL_TOKEN(eof) //
ZS_DECL_TOKEN(lex_error) //
ZS_DECL_TOKEN(hastag) // #
ZS_DECL_TOKEN(comma) // ,
ZS_DECL_TOKEN(dot) // .
ZS_DECL_TOKEN(triple_dots) // ...
ZS_DECL_TOKEN(semi_colon) // ;
ZS_DECL_TOKEN(colon) // :
ZS_DECL_TOKEN(double_colon) // ::
ZS_DECL_TOKEN(question_mark) // ?
ZS_DECL_TOKEN(double_question_mark) // ??
ZS_DECL_TOKEN(triple_question_mark) // ???
ZS_DECL_TOKEN(lbracket) // (
ZS_DECL_TOKEN(rbracket) // )
ZS_DECL_TOKEN(lsqrbracket) // [
ZS_DECL_TOKEN(rsqrbracket) // ]
ZS_DECL_TOKEN(lcrlbracket) // {
ZS_DECL_TOKEN(rcrlbracket) // }
ZS_DECL_TOKEN(lt) // <
ZS_DECL_TOKEN(gt) // >
ZS_DECL_TOKEN(eq) // =
ZS_DECL_TOKEN(lt_eq) // <=
ZS_DECL_TOKEN(gt_eq) // >=
ZS_DECL_TOKEN(eq_eq) // ==
ZS_DECL_TOKEN(three_way_compare) // ===
ZS_DECL_TOKEN(not_eq) // !=
ZS_DECL_TOKEN(mod) // %
ZS_DECL_TOKEN(add) // +
ZS_DECL_TOKEN(minus) // -
ZS_DECL_TOKEN(mul) // *
ZS_DECL_TOKEN(div) // /
ZS_DECL_TOKEN(exp) // **
ZS_DECL_TOKEN(bitwise_or) // |
ZS_DECL_TOKEN(bitwise_and) // &
ZS_DECL_TOKEN(inv) // ~
ZS_DECL_TOKEN(inv_eq) // ~=
ZS_DECL_TOKEN(incr) // ++
ZS_DECL_TOKEN(decr) // --
ZS_DECL_TOKEN(add_eq) // +=
ZS_DECL_TOKEN(minus_eq) // -=
ZS_DECL_TOKEN(mul_eq) // *=
ZS_DECL_TOKEN(div_eq) // /=
ZS_DECL_TOKEN(exp_eq) // ^=
ZS_DECL_TOKEN(mod_eq) // %=
ZS_DECL_TOKEN(lshift) // <<
ZS_DECL_TOKEN(rshift) // >>
ZS_DECL_TOKEN(lshift_eq) // <<=
ZS_DECL_TOKEN(rshift_eq) // >>=
ZS_DECL_TOKEN(xor_eq) // ^=
ZS_DECL_TOKEN(bitwise_or_eq) // |=
ZS_DECL_TOKEN(bitwise_and_eq) // &=
ZS_DECL_TOKEN(at) // @
ZS_DECL_TOKEN(double_at) // @@
ZS_DECL_TOKEN(dollar) // $
ZS_DECL_TOKEN(double_dollar) // $$
ZS_DECL_TOKEN(left_arrow) // <-
ZS_DECL_TOKEN(right_arrow) // =>
ZS_DECL_TOKEN(double_arrow) // <-->
ZS_DECL_TOKEN(double_arrow_eq) // <==>
ZS_DECL_TOKEN(attribute_begin) // </
ZS_DECL_TOKEN(attribute_end) // />
ZS_DECL_TOKEN(block_begin) // {/
ZS_DECL_TOKEN(block_end) // /}
ZS_DECL_TOKEN(endl) // \n
ZS_DECL_TOKEN(almost_eq) // ≈ (alt + x)
ZS_DECL_TOKEN(doc_block) // ```

// Preprocessor.
ZS_DECL_TOKEN(macro) // macro
ZS_DECL_TOKEN(define) // define
ZS_DECL_TOKEN(preprocessor_if) // #if
ZS_DECL_TOKEN(preprocessor_elif) // #elif
ZS_DECL_TOKEN(preprocessor_else) // #else
ZS_DECL_TOKEN(preprocessor_endif) // #endif
ZS_DECL_TOKEN(preprocessor_double_hashtag) // ##
