#include "zspec.h"

ZS_SPEC_SECTION(identifier, "Identifier",
    R"""(Identifiers start with an alphabetic character or the symbol `_` followed by any number of alphabetic characters, `_` or digits ([0-9]).
zscript is a case sensitive language meaning that the lowercase and uppercase representation of the same alphabetic character are considered different characters.
For instance, “foo”, “Foo” and “fOo” are treated as 3 distinct identifiers.

> [!NOTE]
> Highlights information that users should take into account, even when skimming.

> [!TIP]
> Optional information to help a user be more successful.

> [!IMPORTANT]
> Crucial information necessary for users to succeed.

> [!WARNING]
> Critical content demanding immediate user attention due to potential risks.

> [!CAUTION]
> Negative potential consequences of an action.

)""");

// base  break  case  catch  class  clone
// continue  const  default  delete  else  enum
// extends  for  foreach  function  if  in
// local  null  resume  return  switch  this
// throw  try  typeof  while  yield  constructor
// instanceof  true  false  static  __LINE__  __FILE__
// rawcall

ZS_SPEC_SECTION(keywords, "Keywords",
    R"""(The following words are reserved and cannot be used as identifiers:
||||||
|-|-|-|-|-|
|`none`|`null`|`var`|`int`|`float`|
|`bool`|`table`|`array`|`string`|`char`|
|`auto`|`true`|`false`|`return`|`extend`|
|`and`|`or`|`if`|`else`|`for`|
|`do`|`while`|`switch`|`case`|`break`|
|`default`|`continue`|`try`|`catch`|`throw`|
|`this`|`base`|`class`|`namespace`|`global`|
|`const`|`static`|`function`|`typeof`|`typeid`|
|`constructor`|`destructor`|`in`|`enum`|`not`|
|`include`|`import`|&nbsp;|&nbsp;|&nbsp;|)""");

#include "specification/comments.cpp"

// Variable declaration.
#include "specification/variable_declaration.cpp"

// Typed variable declaration.
#include "specification/typed_variable_declaration.cpp"

// Function declaration.
#include "specification/function_declaration.cpp"

#include "specification/compare.cpp"

// Filesystem lib.
#include "specification/fs_lib.cpp"
