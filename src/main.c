#include <clogger.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "bison.h"
#include "lexer.h"

int main(int argc, char *argv[])
{

  char test[] = {"START:\nwait(10);wait(hex=0xff);STOP;"};

  statement_list_t *statements = malloc(sizeof(statement_list_t *));
  ast_parse_string(test, &statements);

  if (NULL == statements) {
    clog_error(__FILE_NAME__, "Returned no statements");
    return EXIT_FAILURE;
  }

  if (0 == statements->count) {
    clog_info(__FILE_NAME__, "No statements parsed");
    return EXIT_FAILURE;
  } else {
    clog_info(__FILE_NAME__, "Found %d statements", statements->count);
  }
  
  ast_convert_itteration_1(statements);
  ast_delete_statements(statements);
  return EXIT_SUCCESS;
}
