#pragma once

#include <catch2.h>
#include <zscript.h>

#include "lex/ztoken.h"
#include "lex/zlexer.h"
#include "lang/jit/zjit_compiler.h"
#include "objects/zfunction_prototype.h"
#include "zvirtual_machine.h"
#include <zbase/sys/file_view.h>
#include <zbase/utility/print.h>
#include <fstream>
