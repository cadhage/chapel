#include <stdio.h>
#include <sys/types.h>
#include "codegen.h"
#include "datatype.h"
#include "files.h"
#include "fun.h"
#include "misc.h"
#include "mysystem.h"
#include "num.h"

#define IO_WRITE   1
#define IO_WRITELN 2

static void genAST(FILE* outfile, AST* ast);

static int hitUnknown = 0;

struct binOp {
  char* astSymName;
  char* astString;
  char* cName; 
  int flag;

};


static binOp binOps[] = {
  {"empty", "empty", "empty", 0}, /* I don't want to return an index of 0... */
  {"*", "#*", "*", 0},
  {"/", "#/", "/", 0},
  {"%", "#%", "%", 0},
  {"+", "#+", "+", 0},
  {"-", "#-", "-", 0},
  {"<<", "#<<", "<<", 0},
  {">>", "#>>", ">>", 0},
  {"<", "#<", "<", 0},
  {"<=", "#<=", "<=", 0},
  {">", "#>", ">", 0},
  {">=", "#>=", ">=", 0},
  {"==", "#==", "==", 0},
  {"!=", "#!=", "!=", 0},
  {"&", "#&", "&", 0},
  {"^", "#^", "^", 0},
  {"|", "#|", "|", 0},
  {"&&", "#&&", "&&", 0},
  {"||", "#||", "||", 0},
  {"=", "#=", "=", 0},
  {"*=", "#*=", "*=", 0},
  {"/=", "#/=", "/=", 0},
  {"%=", "#%=", "%=", 0},
  {"+=", "#+=", "+=", 0},
  {"-=", "#-=", "-=", 0},
  {"<<=", "#<<=", "<<=", 0},
  {">>=", "#>>=", ">>=", 0},
  {"&=", "#&=", "&=", 0},
  {"|=", "#|=", "|=", 0},
  {"^=", "#^=", "^=", 0},
  {",", "#,", ",", 0}

};


static int findBinOp(FILE* outfile, AST* ast) {
  int i;
  int numBinOps = sizeof(binOps) / sizeof(binOps[0]);

  for (i = 0; i < numBinOps; i++) {
    if (strcmp(ast->v[1]->sym->name, binOps[i].astSymName) == 0 &&
        strcmp(ast->v[1]->string, binOps[i].astString) == 0) {
      return i;
    }
  }
  return 0;
}


static int genBinOp(FILE* outfile, AST* ast, int index) {
  genAST(outfile, ast->v[0]);
  fprintf(outfile, binOps[index].cName);
  genAST(outfile, ast->v[2]);
  return 0;
}


static void genDT(FILE* outfile, Sym* pdt) {
  if (pdt == NULL || pdt->name == NULL) {
    fprintf(outfile, "/* unknown type */");
    hitUnknown = 1;
  } else {
    int rank = dtIsDomain(pdt);

    if (rank) {
      fprintf(outfile, "_domain%d", rank);
    } else {
      fprintf(outfile, "_%s", pdt->name);
    }
  }
}


static int isTupleArg(AST* ast) {
  return (ast->kind == AST_op && 
	  ast->v[1]->sym->name &&
	  strcmp(ast->v[1]->sym->name, ",") == 0 &&
	  ast->v[1]->string &&
	  strcmp(ast->v[1]->string, "#,") == 0);
}


static void genSingleWriteArg(FILE* outfile, AST* arg, int genSemi) {
  Sym* argdt = type_info(arg);

  fprintf(outfile, "_write");
  genDT(outfile, argdt);
  fprintf(outfile, "(");
  fprintf(outfile, "stdout, ");
  fprintf(outfile, "_default_format");
  genDT(outfile, argdt);
  fprintf(outfile, ", ");
  genAST(outfile, arg);
  fprintf(outfile, ")");
  if (genSemi) {
    fprintf(outfile, ";\n");
  }
}


static void handleSingleWriteArg(FILE* outfile, AST* arg, int genSemi, 
				 int depth = 0) {
  if (isTupleArg(arg)) {
    handleSingleWriteArg(outfile, arg->v[0], 1, depth+1);
    handleSingleWriteArg(outfile, arg->v[2], depth ? 1 : genSemi, depth+1);
  } else {
    genSingleWriteArg(outfile, arg, genSemi);
  }
}


static int handleWrite(FILE* outfile, AST* ast) {
  AST* fnast = ast->v[0];
  int writeType = 0;

  if (strcmp(fnast->sym->name, "write") == 0) {
    writeType = IO_WRITE;
  } else if (strcmp(fnast->sym->name, "writeln") == 0) {
    writeType = IO_WRITELN;
  }

  if (fnast->kind == AST_qualified_ident && writeType) {
    handleSingleWriteArg(outfile, ast->v[2], (writeType == IO_WRITELN));

    if (writeType == IO_WRITELN) {
      fprintf(outfile, "_write_linefeed(stdout)");
    }
    return 1;
  } else {
    return 0;
  }
}


static void genASTDecls(FILE* outfile, AST* ast) {
  if (ast->kind == AST_def_ident) {
    Sym* sym = ast->sym;
    Sym* dt = type_info(ast, sym);
      
    genDT(outfile, dt);
    fprintf(outfile, " %s;\n", sym->name);
  }
}


static void genDomValues(FILE* outfile, AST* ast) {
  if (ast->kind == AST_block) {
    ast = ast->v[0];
  }
  genAST(outfile, ast);
}


static void genAST(FILE* outfile, AST* ast) {
  if (ast == NULL) {
    INT_FATAL(NULL, "Got NULL AST in genAST()\n");
  }
  switch (ast->kind) {
  case AST_block:
    switch (ast->scope_kind) {
    case 0:
      fprintf(outfile, "{\n");
      break;
    case 1:
      fprintf(outfile, "(");
      break;
    default:
      fprintf(outfile, "/* unexpected scope kind (open) */");
      hitUnknown = 1;
      break;
    }
    forv_AST(subtree, *ast) {
      genASTDecls(outfile, subtree);
    }
    forv_AST(subtree, *ast) {
      genAST(outfile, subtree);
      if (ast->scope_kind == 0) {
	fprintf(outfile, ";\n");
      }
    }
    switch (ast->scope_kind) {
    case 0:
      fprintf(outfile, "}\n");
      break;
    case 1:
      fprintf(outfile, ")");
      break;
    default:
      fprintf(outfile, "/* unexpected scope kind (close) */");
      hitUnknown = 1;
      break;
    }
    break;

  case AST_const:
    fprintf(outfile, "%s", ast->string);
    break;

  case AST_index:
    {
      int i;
      
      for (i=0; i<ast->n; i++) {
	if (i) {
	  fprintf(outfile, ", ");
	}
	genAST(outfile,ast->v[i]);
      }
      if (ast->n < 3) {
	fprintf(outfile, ", 1");
      }
    }
    break;

  case AST_cross_product:
    {
      int i;
      
      for (i=0; i<ast->n; i++) {
	if (i) {
	  fprintf(outfile, ", ");
	}
	genAST(outfile,ast->v[i]);
      }
    }
    break;
    

  case AST_def_ident:
    {
      Sym* sym = ast->sym;
      int rank;

      rank = dtIsDomain(type_info(ast, sym));
      if (rank) {
	fprintf(outfile, "%s = _init_domain_%dD(", sym->name, rank);
	genDomValues(outfile, ast->v[1]);
	fprintf(outfile, ")");
      } else {
	fprintf(outfile, "%s", sym->name);
	if (ast->v[1]) {
	  fprintf(outfile, " = ");
	  genAST(outfile, ast->v[1]);
	}
      }
    }
    break;

  case AST_qualified_ident:
    fprintf(outfile, "%s", ast->sym->name);
    break;

  case AST_if:
    fprintf(outfile, "if (");
    genAST(outfile, ast->v[0]);
    fprintf(outfile, ") {");
    genAST(outfile, ast->v[1]);
    fprintf(outfile, ";\n");
    fprintf(outfile, "}");
    if (ast->n > 2) {
      fprintf(outfile, " else {");
      genAST(outfile, ast->v[2]);
      fprintf(outfile, ";\n");
      fprintf(outfile, "}");
    }
    break;

  case AST_object:
    {
      int i;

      fprintf(outfile, "/* object?!? */\n");
      for (i = 0; i<ast->n; i++) {
	if (i) {
	  fprintf(outfile, ";\n");
	}
	genAST(outfile, ast->v[i]);
      }
    }
    break;

  case AST_op:
    {
      int i;
      int binOpsIndex = 0;

      if (strcmp(ast->v[1]->sym->name, "(") == 0 &&
	  strcmp(ast->v[1]->string, "#(") == 0) {
        if (!handleWrite(outfile, ast)) {
	  genAST(outfile, ast->v[0]);
	  fprintf(outfile, "(");
	  for (i=2; i<ast->n; i++) {
	    genAST(outfile, ast->v[i]);
	  }
	  fprintf(outfile, ")");
	}
	break;
      }

      binOpsIndex = findBinOp(outfile, ast);
      if (binOpsIndex) {
	genBinOp(outfile, ast, binOpsIndex);
	break;
      }

      if (strcmp(ast->v[0]->sym->name, "!") == 0 &&
		 strcmp(ast->v[0]->string, "#!") == 0) {
	fprintf(outfile, "!");
	genAST(outfile, ast->v[1]);
	break;
      }
    }
 
    /* FALL THROUGH */

  default:
    fprintf(outfile, "/* cannot yet handle: %s\n", AST_name[ast->kind]);
    ast_print_recursive(outfile, ast, 2);
    fprintf(outfile, "*/\n");
    hitUnknown = 1;
  }
}


static void genFun(FILE* outfile, Fun* fn) {
  const char* fname = fn->sym->name;
  AST* root;

  // something with fn->ast->last()
  fprintf(outfile, "void %s() ", fname);
  if (fn->ast->kind == AST_def_fun) {
    root = fn->ast->last();
  } else {
    root = fn->ast;
  }
  genAST(outfile, root);
}


static void genFuns(FILE* outfile, FA* fa) {
  Vec<Fun*> funs;
  
  funs.copy(fa->funs);
  forv_Fun(fn, funs) {
    // const char* name = fn->sym->name ? fn->sym->name : "<unknown>";
    // const char* sname = fn->sym->in ? fn->sym->in->name : "<unknown>";
    // printf("%s::%s\n", sname, name);

    if (strcmp(fn->sym->in->name, "user") == 0) {
      genFun(outfile, fn);
    }
  }
}


static void genHeader(FILE* outfile) {
  fprintf(outfile, "#include \"stdchpl.h\"\n\n");
}


void codegen(FA* fa, char* infilename, char* compilerDir) {

  openMakefile(infilename, compilerDir);

  FILE* outfile = openoutfile(infilename);

  genHeader(outfile);
  genFuns(outfile, fa);

  closefile(outfile);
  closeMakefile();

  if (hitUnknown) {
    INT_FATAL(NULL, "hit AST nodes that codegen couldn't handle yet");
  } else {
    makeAndCopyBinary();
  }

}
