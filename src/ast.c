/**
 * @file ast.c
 * @brief Abstract Syntax Tree implementation
 */
#include <clogger.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "bison.h"
#include "lexer.h"

/**
 * @brief Forward decleration of the yyparse function
 */
int yyparse(void *, statement_list_t **statements, yyscan_t scanner);

int ast_parse_string(const char *expr, statement_list_t **statements)
{
  yyscan_t scanner;
  YY_BUFFER_STATE state;
  YYLTYPE yylval;

  if (yylex_init(&scanner)) {
    /* could not initialize */
    clog_error(__FILE_NAME__, "Could not initialize yylex");
    return EXIT_FAILURE;
  }

  state = yy_scan_string(expr, scanner);

  if (yyparse(&yylval, statements, scanner)) {
    /* error parsing */
    clog_error(__FILE_NAME__, "yyparse failed");
    return EXIT_FAILURE;
  } else {
    clog_info(__FILE_NAME__, "Returned %d statements", (*statements)->count);
  }

  yy_delete_buffer(state, scanner);
  yylex_destroy(scanner);

  return EXIT_SUCCESS;
}

parameter_t *ast_create_parameter_string(const char *name, const char *value)
{
  parameter_t *param = (parameter_t *)malloc(sizeof(parameter_t));
  if (!param) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  param->name = strdup(name);
  param->type = STRING;
  param->value.string = strdup(value);
  return param;
}

parameter_t *ast_create_parameter_integer(const char *name, int value)
{
  parameter_t *param = (parameter_t *)malloc(sizeof(parameter_t));
  if (!param) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  param->name = strdup(name);
  param->type = STRING;
  param->value.integer = value; // atoi(value);
  return param;
}

statement_t *ast_create_instruction(const char *name, parameter_t **params,
                                    size_t param_count)
{
  statement_t *statement = (statement_t *)malloc(sizeof(statement_t));

  statement->kind = TYPE_INSTRUCTION;
  statement->args.params = params;
  statement->args.count = param_count;
  statement->name = strdup(name);
  return statement;
}
statement_t *ast_create_label(const char *name)
{
  statement_t *statement = (statement_t *)malloc(sizeof(statement_t));

  statement->kind = TYPE_LABEL;
  statement->args.params = NULL;
  statement->args.count = 0;
  statement->name = strdup(name);
  return statement;
}

void ast_delete_statements(statement_list_t *statements)
{
  return;
}

typedef struct opcode_node_t {
  uint8_t opcode;
  size_t size_in_bytes;
  char *label_ref;
} opcode_node_t;

typedef struct label_node_t {
  char *label;
  size_t index_first_opcode;
} label_node_t;

opcode_node_t *ast_generate_opcode_node(const statement_t *statement)
{
  opcode_node_t *node = malloc(sizeof(opcode_node_t *));

  if (0 == strcasecmp("wait", statement->name)) {
    node->opcode = 0x10;
    node->size_in_bytes = 2;
  }
  if (0 == strcasecmp("stop", statement->name)) {
    node->opcode = 0xFF;
    node->size_in_bytes = 1;
  }

  return node;
}
int ast_convert_itteration_1(const statement_list_t *statements)
{

  opcode_node_t **opcodes = malloc(sizeof(opcode_node_t *) * statements->count);
  label_node_t **labels = malloc(sizeof(label_node_t *) * statements->count);

  size_t index_label = 0;
  size_t index_opcode = 0;

  for (int index = 0; index < statements->count; index++) {
    switch (statements->statements[index]->kind) {
    case TYPE_LABEL:
      printf("Parsed label for '%s'\n", statements->statements[index]->name);
      labels[index_label] = malloc(sizeof(label_node_t *));
      labels[index_label]->index_first_opcode = index_opcode;
      labels[index_label]->label = strdup(statements->statements[index]->name);
      index_label++;
      break;
    case TYPE_INSTRUCTION:
      printf("Parsed instruction for '%s'\n",
             statements->statements[index]->name);
      opcodes[index_opcode] =
          ast_generate_opcode_node(statements->statements[index]);
      index_opcode++;
      break;

    default:
      printf("Parsed unknown for '%s'\n", statements->statements[index]->name);
        return EXIT_FAILURE;
      break;
    }
  }
}
