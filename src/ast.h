/**
 * @file ast.h
 * @brief Abstract Syntax Tree definitions
 */
#ifndef __AST_H__
#define __AST_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The operation type
 */
typedef enum EStatementType { TYPE_LABEL, TYPE_INSTRUCTION } EStatementType;

typedef struct parameter_s {
  char *name;
  union value {
    char *string;
    int integer;
  } value;
  enum { STRING, INTEGER } type;
} parameter_t;

typedef struct statement_s {
  EStatementType kind;

  char *name;
  struct {
    int count;
    parameter_t **params;
  } args;

} statement_t;

typedef struct statement_list_s {
  int count;
  statement_t **statements;
} statement_list_t;

parameter_t *ast_create_parameter_string(const char *name, const char *value);
parameter_t *ast_create_parameter_integer(const char *name, int value);
statement_t *ast_create_instruction(const char *name, parameter_t **params,
                                    size_t param_count);
statement_t *ast_create_label(const char *name);

int ast_parse_string(const char *expr, statement_list_t **statements);
int ast_convert_itteration_1(const statement_list_t *statements);
int ast_convert_itteration_2(const statement_list_t *statements);
void ast_delete_statements(statement_list_t *statements);

#ifdef __cplusplus
}
#endif

#endif // __AST_H__