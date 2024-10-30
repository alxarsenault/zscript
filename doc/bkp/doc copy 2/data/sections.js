const documentation_content = {
  "import": 
  {
    "sections": [
      {
        "subsections": [
          {
            "content": "<ul class=\"zscript-list\">\n  <li>\n    <h4 class=\"zscript-list-label\">1. Hybrid Typing System:</h4>\n    ZSCRIPT allows variables to be declared either <strong>weakly</strong> typed or <strong>strongly</strong> typed,\n    offering the flexibility of dynamic\n    typing while enabling stricter type enforcement where necessary.\n    Developers can choose to rely on weak typing for rapid development and experimentation or enforce strong typing to\n    prevent errors and ensure predictable behavior.\n\n    <ul >\n      <li>\n        <strong>Weak Typing:</strong>\n        Variables declared with the var keyword can hold values of any type and can be reassigned to different types\n        throughout their lifetime.\n      </li>\n      <li><strong>Strong Typing:</strong>\n        Variables can be declared with explicit types like <code>int, float, or string</code>. Once\n        declared, they are restricted to those types, preventing accidental type errors.\n      </li>\n    </ul>\n  </li>\n  <li>\n    <h4 class=\"zscript-list-label\">2. Type Inference and Auto Typing:</h4>\n    ZSCRIPT introduces an auto keyword that automatically infers the type of a variable based on its initial value. This\n    combines the flexibility of dynamic\n    typing with the safety of strong typing, without requiring the developer to explicitly specify the type.\n  </li>\n\n  <li>\n    <h4 class=\"zscript-list-label\">3. Type Restrictions:</h4>\n    Developers can restrict variables to specific sets of types. For example, a variable can be declared to only accept\n    <code>int</code>\n    or <code>float</code>, providing both flexibility and control. ZSCRIPT also includes convenient shortcuts, like the\n    <code>number</code> keyword,\n    which is equivalent to <code>var&ltint, float&gt</code>, simplifying code while maintaining type restrictions.\n  </li>\n\n  <li>\n    <h4 class=\"zscript-list-label\">4. Constant Variables:</h4>\n    Immutability is supported through the <code>const</code> keyword, allowing developers to define constant values that\n    cannot be\n    changed after initialization. This is useful for defining configuration values, constants, or any data that should\n    remain fixed throughout the execution of a script. ZSCRIPT enforces that constant variables be initialized with\n    literal values, enhancing performance and predictability.\n  </li>\n\n  <li>\n    <h4 class=\"zscript-list-label\">5. Global Variables:</h4>\n    ZSCRIPT supports global variables, which can be declared and accessed from anywhere within a script. By using the\n    <code>global</code> keyword, developers can clearly indicate that a variable is intended to be globally accessible,\n    ensuring\n    clarity and avoiding accidental overwrites of local variables.\n  </li>\n\n  <li>\n    <h4 class=\"zscript-list-label\">6. Simple and Powerful Syntax:</h4>\n    ZSCRIPT has a clean and intuitive syntax that lowers the barrier to entry while still providing powerful features\n    for experienced developers. The language offers constructs like arrays, tables (dictionaries), functions, classes,\n    and instances, making it versatile and capable of handling complex scenarios.\n  </li>\n\n  <li>\n    <h4 class=\"zscript-list-label\">7. Extendability and Interoperability:</h4>\n    With types like <code>user_data</code> and <code>raw_pointer</code>, ZSCRIPT is designed to interact with external\n    systems and libraries,\n    making it suitable for embedding in larger applications, game engines, or systems that need customizable scripting\n    capabilities. This enables integration with C/C++ libraries and other environments.\n  </li>\n</ul>",
            "title": "Key Features"
          }
        ],
        "content": "ZSCRIPT is a modern, dynamically typed scripting language designed for flexibility, ease of use, and powerful runtime\nfeatures.\nIt offers a hybrid type system, combining the best of both weak and strong typing, giving developers the freedom to\nchoose between dynamic flexibility\nand static type safety when needed.\n\n\n<div class=\"zscript-para\">\n  ZSCRIPT is inspired by several well-known scripting languages but introduces unique elements that make it adaptable\n  to various use cases, from rapid prototyping to more structured application development.\n  Its simple syntax and diverse set of features make it suitable for tasks ranging\n  from simple scripts to complex systems with performance requirements.\n</div>",
        "subtitle": "Types",
        "title": "Introduction"
      },
      {
        "content": "To install ZScript, follow these steps: ...",
        "title": "Installation"
      }
    ]
  },
  "index": 
  {
    "sections": [
      {
        "subsections": [
          {
            "content": "<ul class=\"zscript-list\">\n  <li>\n    <h4 class=\"zscript-list-label\">1. Hybrid Typing System:</h4>\n    ZSCRIPT allows variables to be declared either <strong>weakly</strong> typed or <strong>strongly</strong> typed,\n    offering the flexibility of dynamic\n    typing while enabling stricter type enforcement where necessary.\n    Developers can choose to rely on weak typing for rapid development and experimentation or enforce strong typing to\n    prevent errors and ensure predictable behavior.\n\n    <ul >\n      <li>\n        <strong>Weak Typing:</strong>\n        Variables declared with the var keyword can hold values of any type and can be reassigned to different types\n        throughout their lifetime.\n      </li>\n      <li><strong>Strong Typing:</strong>\n        Variables can be declared with explicit types like <code>int, float, or string</code>. Once\n        declared, they are restricted to those types, preventing accidental type errors.\n      </li>\n    </ul>\n  </li>\n  <li>\n    <h4 class=\"zscript-list-label\">2. Type Inference and Auto Typing:</h4>\n    ZSCRIPT introduces an auto keyword that automatically infers the type of a variable based on its initial value. This\n    combines the flexibility of dynamic\n    typing with the safety of strong typing, without requiring the developer to explicitly specify the type.\n  </li>\n\n  <li>\n    <h4 class=\"zscript-list-label\">3. Type Restrictions:</h4>\n    Developers can restrict variables to specific sets of types. For example, a variable can be declared to only accept\n    <code>int</code>\n    or <code>float</code>, providing both flexibility and control. ZSCRIPT also includes convenient shortcuts, like the\n    <code>number</code> keyword,\n    which is equivalent to <code>var&ltint, float&gt</code>, simplifying code while maintaining type restrictions.\n  </li>\n\n  <li>\n    <h4 class=\"zscript-list-label\">4. Constant Variables:</h4>\n    Immutability is supported through the <code>const</code> keyword, allowing developers to define constant values that\n    cannot be\n    changed after initialization. This is useful for defining configuration values, constants, or any data that should\n    remain fixed throughout the execution of a script. ZSCRIPT enforces that constant variables be initialized with\n    literal values, enhancing performance and predictability.\n  </li>\n\n  <li>\n    <h4 class=\"zscript-list-label\">5. Global Variables:</h4>\n    ZSCRIPT supports global variables, which can be declared and accessed from anywhere within a script. By using the\n    <code>global</code> keyword, developers can clearly indicate that a variable is intended to be globally accessible,\n    ensuring\n    clarity and avoiding accidental overwrites of local variables.\n  </li>\n\n  <li>\n    <h4 class=\"zscript-list-label\">6. Simple and Powerful Syntax:</h4>\n    ZSCRIPT has a clean and intuitive syntax that lowers the barrier to entry while still providing powerful features\n    for experienced developers. The language offers constructs like arrays, tables (dictionaries), functions, classes,\n    and instances, making it versatile and capable of handling complex scenarios.\n  </li>\n\n  <li>\n    <h4 class=\"zscript-list-label\">7. Extendability and Interoperability:</h4>\n    With types like <code>user_data</code> and <code>raw_pointer</code>, ZSCRIPT is designed to interact with external\n    systems and libraries,\n    making it suitable for embedding in larger applications, game engines, or systems that need customizable scripting\n    capabilities. This enables integration with C/C++ libraries and other environments.\n  </li>\n</ul>",
            "title": "Key Features"
          },
          {
            "content": "ZSCRIPT is designed to be highly adaptable to a wide range of use cases, including but not limited to:\n<ul>\n  <li>\n    <h4>Game Scripting:</h4> The language’s hybrid typing system allows for the fast-paced, iterative development\n    cycles required in game scripting. ZSCRIPT’s support for arrays, tables, and user-defined classes make it an\n    excellent choice for game logic, AI behaviors, and event handling.\n  </li>\n  <li>\n    <h4>Embedded Systems:</h4> With its lightweight footprint and dynamic nature, ZSCRIPT can be embedded into\n    larger applications, providing a scripting interface for customization or automation without needing to rewrite\n    or recompile the entire system.\n  </li>\n  <li>\n    <h4>Prototyping:</h4> ZSCRIPT’s weak typing and flexible syntax make it ideal for rapid prototyping, allowing\n    developers to experiment with ideas and features quickly before committing to more structured code.\n  </li>\n  <li>\n    <h4>Tooling and Automation:</h4> ZSCRIPT can be used for scripting tools, automating workflows, or creating\n    small applications that require a combination of flexibility and power. Its mix of dynamic and strong typing\n    enables quick iteration while maintaining reliability where needed.\n  </li>\n</ul>",
            "title": "Use Cases"
          },
          {
            "content": "The primary motivation behind ZSCRIPT is to provide developers with a language that strikes a balance between dynamic\nflexibility and static reliability. Many modern scripting languages, while powerful, either impose too much structure or\ntoo little, leaving developers to choose between type safety and ease of use. ZSCRIPT’s hybrid approach allows\ndevelopers to move fluidly between the two extremes as needed.\n\n<ul>\n	<li>\n		<h4>Flexibility for Beginners:</h4> Beginners can start with weak typing and basic constructs without worrying\n		about strict rules, while gradually adopting more advanced features as they become comfortable.\n	</li>\n	<li>\n		<h4>Power for Experts:</h4> Advanced users can take full advantage of the strong typing system, type inference,\n		and type restrictions to write efficient, safe, and maintainable code, especially in larger projects or\n		performance-critical applications.\n	</li>\n</ul>",
            "title": "Why ZSCRIPT?"
          }
        ],
        "content": "ZSCRIPT is a modern, dynamically typed scripting language designed for flexibility, ease of use, and powerful runtime\nfeatures.\nIt offers a hybrid type system, combining the best of both weak and strong typing, giving developers the freedom to\nchoose between dynamic flexibility\nand static type safety when needed.\n\n\n<div class=\"zscript-para\">\n  ZSCRIPT is inspired by several well-known scripting languages but introduces unique elements that make it adaptable\n  to various use cases, from rapid prototyping to more structured application development.\n  Its simple syntax and diverse set of features make it suitable for tasks ranging\n  from simple scripts to complex systems with performance requirements.\n</div>",
        "subtitle": "Types",
        "title": "Introduction"
      },
      {
        "content": "To install ZScript, follow these steps: ...",
        "title": "Installation"
      },
      {
        "subsections": [
          {
            "code": "var a; // a is null.\na = 1; // a is an integer.\na = \"abc\"; // a is a string.\na = \"\"\"abc\"\"\"; // a is a string.",
            "content": "A variable declared with <code>var</code> is <strong>weakly</strong> typed and can be assigned or reassigned any type.",
            "title": "Variables"
          },
          {
            "code": "int a = 51;\na = \"abc\"; // ERROR - Trying to assign a string value to an integer variable.",
            "content": "A variable declared with a type, is <strong>strongly</strong> typed and can only be assigned the same type.",
            "title": "Strongly typed variables"
          },
          {
            "code": "var<int, float> a = 31; // a is an integer.\na = 31.1; // a is a float.\na = \"abc\"; // ERROR - Trying to assign a string value to a var<int, float> variable.",
            "content": "A variable declared with a type, is <strong>strongly</strong> typed and can only be assigned the same type.",
            "title": "Multi typed variables"
          },
          {
            "subsections": [
              {
                "code": "myVar = 10;",
                "content": "You can declare variables like this:",
                "title": "Variables"
              },
              {
                "code": "if (myVar > 5) {\n    print('Variable is greater than 5');\n}",
                "content": "Conditional statements in ZScript:\n          <ul>\n            <li>john</li>\n          </ul>\n          ",
                "title": "Conditional Statements"
              }
            ],
            "code": "if (myVar > 5) {\n    print('Variable is greater than 5');\n}",
            "content": "Conditional statements in ZScript:",
            "title": "Conditional Statements"
          }
        ],
        "code": "myVar = 10;\nif (myVar > 5) {\n    print('Variable is greater than 5');\n}",
        "content": "Here are some examples of the basic syntax in ZScript.",
        "subtitle": "Here are some examples of the basic syntax in ZScript.",
        "title": "Basic Syntax"
      },
      {
        "subsections": [
          {
            "subsections": [
              {
                "code": "// Ceci est une fonction locale.\nvar my_func = $() { return 0; }",
                "content": "Les fonctions lamda",
                "title": "Lamdas"
              }
            ],
            "code": "// Ceci est une fonction locale.\nfunction my_func() { return 0; }\n\n// C'est la même chose que cela.\nvar my_func = function() { return 0; }",
            "title": "Local functions"
          },
          {
            "code": "// Option 1.\nvar t1 = {\n  // Ceci est une fonction dans une table.\n  my_func = function() { return 0; }\n};\n\n// Option 2.\nvar t2 = {\n  // Ceci est aussi une fonction dans une table.\n  function my_func() { return 0; }\n};\n\nlocal t3 = {};\n\n// Option 3.\nt3.my_func = function() { return 0; };\n\n// Option 4 (Je ne suis pas certain entre 4 et 5).\nfunction t3.my_func() { return 0; };\n\n// Option 5.\nfunction t3::my_func() { return 0; };\n\n",
            "content": "Les options 1, 2, 3 je suis pas mal certain.",
            "title": "Table functions"
          },
          {
            "code": "// Option 1.\nglobal function main(var args) { return 0; }\n\n// Le `var` n'est pas oubligatoire ici.\nglobal function main(args) { return 0; }\n\n// Option 2.\nfunction global::main(var args) { return 0; }\n\n// Option 3.\nfunction global.main(var args) { return 0; }\n\n// Option 4.\nfunction ::main(var args) { return 0; }\n\n// Option 5.\n// Celui la va probablement toujours fonctionner.\nglobal.main = function(var args) { return 0; }\n",
            "content": "Pour les fonctions globales je ne suis vraiment pas certain.",
            "title": "Global functions"
          }
        ],
        "content": "To install ZScript, follow these steps: ...",
        "title": "Functions"
      },
      {
        "subsections": [
          {
            "code": "var",
            "content": "A variable declared with <code>var</code> is <strong>weakly</strong> typed and can be assigned or reassigned any type.",
            "title": "Variables"
          }
        ],
        "content": "To install ZScript, follow these steps: ...",
        "title": "Structs"
      },
      {
        "subsections": [
          {
            "code": "const fs = import(\"fs\");\n\nconst examples_dir = fs.path(ZSCRIPT_DOC_DIRECTORY + \"/src/examples\");\nconst sections_dir = fs.path(ZSCRIPT_DOC_DIRECTORY + \"/src/sections\");\nconst content_path = fs.path(ZSCRIPT_DOC_DIRECTORY + \"/src/content.zson\");\nconst output_path = fs.path(ZSCRIPT_DOC_DIRECTORY + \"/data/sections.js\");\n\nvar content = { examples = stable() };\n\n// Add all files from the examples directory to the content table.\nfor(var p : examples_dir.list_recursive(\".zs\")) {\n  content.examples[p.dirname][p.stem] = fs.string_file(p);\n}\n\n// Add all files from the sections directory to the content table.\nfor(var p : sections_dir.list(\".zson\")) {\n  content[p.stem] = fs.json_file(p, content);\n}\n\nfs.file(output_path, fs.mode.write)\n  .write(\"const documentation_content = \")\n  .write_json(fs.json_file(content_path, content));",
            "content": "A variable declared with <code>var</code> is <strong>weakly</strong> typed and can be assigned or reassigned any type.",
            "title": "Sample"
          }
        ],
        "content": "To install ZScript, follow these steps: ...",
        "title": "Samples"
      }
    ]
  },
  "module": 
  {
    "sections": [
      {
        "subsections": [
          {
            "code": "const closure = zs.compile_file(\"dummy\");\nconst content = closure();",
            "script": "hljs.highlightElement(document.getElementById('compile_example_01'));",
            "content": "<p>\n    Compiling a file will give us a closure that will execute the content of the file when called.\n  </p>\n  \n  <p>\n    We assign the dummy.zs file closure to the <code>dummy1_closure</code> variable.\n    The closure contains a some byte code, generated by the compiler, that will be executed everytime it is called.\n  \n    Calling <code>dummy1_closure()</code> executes and returns the value i.e. a table.\n  </p>\n  \n  \n  <pre><code id=\"compile_example_01\" class=\"zscript-code\">const dummy1_closure = zs.compile_file(\"dummy1\");\n  const dummy1_content = dummy1_closure();\n  dummy1_content.do_something(2, 3);\n    </code></pre>\n  \n  <div class=\"alert alert-warning\" role=\"alert\">\n  The <code>zs.compile_file</code> function will recompile and create a new closure everthing time.\n  </div>\n  ",
            "title": "Compile"
          },
          {
            "code": "const content = zs.load_file(\"dummy\");",
            "content": "We assign the returned value of the dummy.zs file to the <code>content</code> variable.",
            "title": "Load File"
          }
        ],
        "script": "\n      hljs.highlightElement(document.getElementById('compile_example_01'));\n      hljs.highlightElement(document.getElementById('compile_example_02'));\n      hljs.highlightElement(document.getElementById('compile_example_03'));\n      hljs.highlightElement(document.getElementById('compile_example_04'));\n      \n      ",
        "content": "<p>A file is actually a closure.</p>\n\n<p>\n  Given these two files (<code>dummy1.zs</code> and <code>dummy2.zs</code>):\n</p>\n\n<div class=\"card\">\n  <div class=\"card-header\">dummy1.zs</div>\n  <div class=\"card-body\">\n    <pre class=\"file-example\"><code id=\"compile_example_02\" class=\"zscript-code\">// This is a local table variable.\nvar dummy1 = {};\n\n// This is a local const variable.\nconst my_local_var = 34;\n\n// This is a local closure.\nfunction my_local_do_something(a, b) {\n  return (a + b) * my_local_var;\n}\n\n// This adds the `do_something` closure to the `dummy1` table.\nfunction dummy1.do_something(a, b) {\n  return 54 + my_local_do_something(a, b);\n}\n\n// Returns the dummy1 table.\nreturn dummy1;</code></pre>\n  </div>\n</div>\n<p></p>\n<div class=\"card\">\n  <div class=\"card-header\">dummy2.zs</div>\n  <div class=\"card-body\">\n    <pre class=\"file-example\"><code id=\"compile_example_03\" class=\"zscript-code\">zs.print(\"dummy2\")</code></pre>\n  </div>\n</div>\n<p></p>\n",
        "title": "Load"
      },
      {
        "subsections": [
          {
            "subsections": [
              {
                "code": "const content = zs.load_file(\"dummy\");",
                "content": "We assign the returned value of the dummy.zs file to the <code>content</code> variable.",
                "title": "Load File"
              },
              {
                "code": ["const closure = zs.compile_file(\"dummy\");\nconst content = closure();","const content = zs.compile_file(\"dummy\")();","const content = zs.load_file(\"dummy\");"
                ],
                "content": "<p>We assign the dummy.zs file closure to the <code>closure</code> variable. The closure contains a function that will\nexecute and return the returned value of the dummy.zs file.\nCalling <code>closure()</code> will execute the file and return the value. These three examples are equivalent.</p>\n\n",
                "title": "Compile File"
              }
            ],
            "code": "return { bingo = [1, 2, 3] };",
            "content": "Given this file, named `dummy.zs`.",
            "subtitle": "Loading a code file",
            "title": "Load File"
          }
        ],
        "content": "<p>A file is actually a closure.</p>\n\n<p>\n  Given these two files (<code>dummy1.zs</code> and <code>dummy2.zs</code>):\n</p>\n\n<div class=\"card\">\n  <div class=\"card-header\">dummy1.zs</div>\n  <div class=\"card-body\">\n    <pre class=\"file-example\"><code id=\"compile_example_02\" class=\"zscript-code\">// This is a local table variable.\nvar dummy1 = {};\n\n// This is a local const variable.\nconst my_local_var = 34;\n\n// This is a local closure.\nfunction my_local_do_something(a, b) {\n  return (a + b) * my_local_var;\n}\n\n// This adds the `do_something` closure to the `dummy1` table.\nfunction dummy1.do_something(a, b) {\n  return 54 + my_local_do_something(a, b);\n}\n\n// Returns the dummy1 table.\nreturn dummy1;</code></pre>\n  </div>\n</div>\n<p></p>\n<div class=\"card\">\n  <div class=\"card-header\">dummy2.zs</div>\n  <div class=\"card-body\">\n    <pre class=\"file-example\"><code id=\"compile_example_03\" class=\"zscript-code\">zs.print(\"dummy2\")</code></pre>\n  </div>\n</div>\n<p></p>\n",
        "title": "Import"
      }
    ]
  }
}