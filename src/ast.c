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
  param->type = INTEGER;
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
  uint16_t register_2bytes;
  uint32_t register_4bytes;
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

  /*
   * # | Element | Bitmask               | Hex    | Parameter |
   * # | ------- | --------------------- | ------ | --------- |
   * # | OPCODE  | 0b0001 0000           | 0x10   |           |
   */
  if (0 == strcasecmp("pauze", statement->name)) {
    node->opcode = 0x10;
    node->size_in_bytes = 1;
  }

  /*
   * | Element | Bitmask               | Hex    | Parameter |
   * | ------- | --------------------- | ------ | --------- |
   * | OPCODE  | 0b0010 0000           | 0x2000 |           |
   * | TIME    | 0b0000 1111 1111 1111 | 0x0FFF |           |
   */
  if (0 == strcasecmp("wachten", statement->name)) {
    node->opcode = 0x20;
    node->size_in_bytes = 2;
    if (1 != statement->args.count) {
      clog_error(__FILE_NAME__, "Wachten heeft 1 parameter nodig");
      exit(EXIT_FAILURE);
    }
    if (INTEGER != statement->args.params[0]->type) {
      clog_error(__FILE_NAME__, "Wachten moet een numerieke waarde hebben als "
                                "parameter: Wachten(10);");
      exit(EXIT_FAILURE);
    }
    node->register_2bytes = statement->args.params[0]->value.integer;
    clog_info(__FILE_NAME__, "WACHTEN(%d) omgezet naar opcode %d",
              node->register_2bytes, node->opcode);
  }
  /*
   * | Element | Bitmask               | Hex    | Parameter         |
   * | ------- | --------------------- | ------ | ----------------- |
   * | OPCODE  | 0b0011 0000 0000 0000 | 0x3000 |                   |
   * | STATUS  | 0b0000 0001 0000 0000 | 0x0100 |                   |
   * | HSIO    | 0b0000 0010 0000 0000 | 0x0200 | HSIO (0x0 / 0x01) |
   * | POORT   | 0b0000 0000 0001 1111 | 0x001F | POORT             |
   */
  if (0 == strcasecmp("zet_poort_aan", statement->name)) {
    node->opcode = 0x30;
    node->size_in_bytes = 2;
    if (2 != statement->args.count) {
      clog_error(__FILE_NAME__,
                 "Zet_poort_aan heeft 2 parameter nodig (hsio, poort)");
      exit(EXIT_FAILURE);
    }
    if ((0 == strcasecmp("default", statement->args.params[0]->name)) ||
        (0 == strcasecmp("default", statement->args.params[1]->name))) {
      clog_error(__FILE_NAME__,
                 "De parameters van zet_poort_aan moeten hsio en poort zijn.");
      exit(EXIT_FAILURE);
    }

    if (0 == strcasecmp("hsio", statement->args.params[0]->name)) {
      node->register_2bytes =
          ((statement->args.params[0]->value.integer & 0x01) << 9) | (0x0100) |
          (statement->args.params[1]->value.integer & 0x1F);
    } else {
      node->register_2bytes =
          ((statement->args.params[1]->value.integer & 0x01) << 9) | (0x0100) |
          (statement->args.params[0]->value.integer & 0x1F);
    }

    clog_info(__FILE_NAME__, "zet_poort_aan(%d) omgezet naar opcode %d",
              node->register_2bytes, node->opcode);
  }
  /*
   * | Element | Bitmask               | Hex    | Parameter         |
   * | ------- | --------------------- | ------ | ----------------- |
   * | OPCODE  | 0b0011 0000 0000 0000 | 0x3000 |                   |
   * | STATUS  | 0b0000 0000 0000 0000 | 0x0000 |                   |
   * | HSIO    | 0b0000 0010 0000 0000 | 0x0200 | HSIO (0x0 / 0x01) |
   * | POORT   | 0b0000 0000 0001 1111 | 0x001F | POORT             |
   */
  if (0 == strcasecmp("zet_poort_uit", statement->name)) {
    node->opcode = 0x30;
    node->size_in_bytes = 2;
    if (2 != statement->args.count) {
      clog_error(__FILE_NAME__,
                 "Zet_poort_uit heeft 2 parameter nodig (hsio, poort)");
      exit(EXIT_FAILURE);
    }
    if ((0 == strcasecmp("default", statement->args.params[0]->name)) ||
        (0 == strcasecmp("default", statement->args.params[1]->name))) {
      clog_error(__FILE_NAME__,
                 "De parameters van zet_poort_uit moeten hsio en poort zijn.");
      exit(EXIT_FAILURE);
    }

    if (0 == strcasecmp("hsio", statement->args.params[0]->name)) {
      node->register_2bytes =
          ((statement->args.params[0]->value.integer & 0x01) << 9) |
          (statement->args.params[1]->value.integer & 0x1F);
    } else {
      node->register_2bytes =
          ((statement->args.params[1]->value.integer & 0x01) << 9) |
          (statement->args.params[0]->value.integer & 0x1F);
    }

    clog_info(__FILE_NAME__, "zet_poort_uit(%d) omgezet naar opcode %d",
              node->register_2bytes, node->opcode);
  }
  /*
* | Element | Bitmask               | Hex    | Parameter         |
* | ------- | --------------------- | ------ | ----------------- |
* | OPCODE  | 0b0100 0000 0000 0000 | 0x4000 |                   |
* | HSIO    | 0b0000 0010 0000 0000 | 0x0200 | HSIO (0x0 / 0x01) |
* | POORT   | 0b0000 0000 0001 1111 | 0x001F | POORT             |
   */
  if (0 == strcasecmp("flip_poort", statement->name)) {
    node->opcode = 0x40;
    node->size_in_bytes = 2;
    if (2 != statement->args.count) {
      clog_error(__FILE_NAME__,
                 "flip_poort heeft 2 parameter nodig (hsio, poort)");
      exit(EXIT_FAILURE);
    }
    if ((0 == strcasecmp("default", statement->args.params[0]->name)) ||
        (0 == strcasecmp("default", statement->args.params[1]->name))) {
      clog_error(__FILE_NAME__,
                 "De parameters van flip_poort moeten hsio en poort zijn.");
      exit(EXIT_FAILURE);
    }

    if (0 == strcasecmp("hsio", statement->args.params[0]->name)) {
      node->register_2bytes =
          ((statement->args.params[0]->value.integer & 0x01) << 9) |
          (statement->args.params[1]->value.integer & 0x1F);
    } else {
      node->register_2bytes =
          ((statement->args.params[1]->value.integer & 0x01) << 9) |
          (statement->args.params[0]->value.integer & 0x1F);
    }

    clog_info(__FILE_NAME__, "flip_poort(%d) omgezet naar opcode %d",
              node->register_2bytes, node->opcode);
  }
  /*
   * | Element | Bitmask               | Hex    | Parameter |
   * | ------- | --------------------- | ------ | --------- |
   * | OPCODE  | 0b1111 1111		        | 0xFF   |           |
   */
  if (0 == strcasecmp("stoppen", statement->name)) {
    node->opcode = 0xFF;
    node->size_in_bytes = 1;
    clog_info(__FILE_NAME__, "STOPPEN omgezet naar opcode %d", node->opcode);
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
      labels[index_label] = malloc(sizeof(label_node_t *));
      labels[index_label]->index_first_opcode = index_opcode;
      labels[index_label]->label = strdup(statements->statements[index]->name);
      clog_info(__FILE_NAME__,
                "Label '%s' opgeslagen voor later (index_first_opcode: %d)",
                labels[index_label]->label,
                labels[index_label]->index_first_opcode);

      index_label++;
      break;
    case TYPE_INSTRUCTION:
      opcodes[index_opcode] =
          ast_generate_opcode_node(statements->statements[index]);
      clog_info(__FILE_NAME__, "Parsed instruction for '%s'\n",
                statements->statements[index]->name);
      index_opcode++;
      break;

    default:
      printf("Parsed unknown for '%s'\n", statements->statements[index]->name);
      return EXIT_FAILURE;
      break;
    }
  }
}
