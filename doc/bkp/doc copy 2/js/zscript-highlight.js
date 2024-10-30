hljs.registerLanguage('zscript', function () {
  const NUMBERS = {
    className: 'number',
    variants: [
      // Floating-point literal.
      {
        begin:
          "[+-]?(?:" // Leading sign.
          // Decimal.
          + "(?:"
          + "[0-9](?:'?[0-9])*\\.(?:[0-9](?:'?[0-9])*)?"
          + "|\\.[0-9](?:'?[0-9])*"
          + ")(?:[Ee][+-]?[0-9](?:'?[0-9])*)?"
          + "|[0-9](?:'?[0-9])*[Ee][+-]?[0-9](?:'?[0-9])*"
          // Hexadecimal.
          + "|0[Xx](?:"
          + "[0-9A-Fa-f](?:'?[0-9A-Fa-f])*(?:\\.(?:[0-9A-Fa-f](?:'?[0-9A-Fa-f])*)?)?"
          + "|\\.[0-9A-Fa-f](?:'?[0-9A-Fa-f])*"
          + ")[Pp][+-]?[0-9](?:'?[0-9])*"
          + ")(?:" // Literal suffixes.
          + "[Ff](?:16|32|64|128)?"
          + "|(BF|bf)16"
          + "|[Ll]"
          + "|" // Literal suffix is optional.
          + ")"
      },
      // Integer literal.
      {
        begin:
          "[+-]?\\b(?:" // Leading sign.
          + "0[Bb][01](?:'?[01])*" // Binary.
          + "|0[Xx][0-9A-Fa-f](?:'?[0-9A-Fa-f])*" // Hexadecimal.
          + "|0(?:'?[0-7])*" // Octal or just a lone zero.
          + "|[1-9](?:'?[0-9])*" // Decimal.
          + ")(?:" // Literal suffixes.
          + "[Uu](?:LL?|ll?)"
          + "|[Uu][Zz]?"
          + "|(?:LL?|ll?)[Uu]?"
          + "|[Zz][Uu]"
          + "|" // Literal suffix is optional.
          + ")"
        // Note: there are user-defined literal suffixes too, but perhaps having the custom suffix not part of the
        // literal highlight actually makes it stand out more.
      }
    ],
    relevance: 0
  };


  const COMMENT = {
    className: "comment",
    variants: [
      // JSDOC_COMMENT,
      hljs.C_BLOCK_COMMENT_MODE,
      hljs.C_LINE_COMMENT_MODE
    ]
  };

  // const UPPER_CASE_CONSTANT = {
  //   // relevance: 0,
  //   match: /\b[A-Z][A-Z_0-9]+\b/,
  //   className: "variable.constant"
  // };

  return {
    name: "zscript",
    aliases: ['zs'],
    case_insensitive: true, // language is case-insensitive
    // keywords: 'for if while int return string var float bool',
    // literal: ['false', 'true', 'null'],

    keywords: {
      keyword: 'for if while return const',
      literal: ['false', 'true', 'null', 'none'],
      built_in: 'print import',
      type: ['int', 'var', 'float', 'bool', 'char', 'table', 'array'],
      
      // _relevance_only: 'one two three four'
    },

    contains: [
      // hljs.QUOTE_STRING_MODE,
      // hljs.C_LINE_COMMENT,
    
      NUMBERS,
      COMMENT,
     
      hljs.APOS_STRING_MODE,
      hljs.QUOTE_STRING_MODE,
      // UPPER_CASE_CONSTANT,
      // {
      //   scope: 'string',
      //   begin: '"',
      //   end: '"'
      // },
      // {
      //   className: 'string',
      //   begin: '"',
      //   end: '"',
      //   relevance: 10
      // },
      // {
      //   className: 'string',
      //   begin: "'",
      //   end: "'",
      //   relevance: 0
      // },

      // hljs.COMMENT(
      //   '/\\*', // begin
      //   '\\*/', // end
      //   {
      //     contains: [{
      //       scope: 'doc',
      //       begin: '@\\w+'
      //     }],
      //     relevance: 10
      //   },
      // ),
  
    ]
  }
});

hljs.configure({ languages: ["zscript"] });

/*
Language: C
Category: common, system
Website: https://en.wikipedia.org/wiki/C_(programming_language)
*/
 