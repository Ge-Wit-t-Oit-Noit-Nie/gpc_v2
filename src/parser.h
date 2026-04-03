/**
 * @file   parser.h
 * @brief  Parser interface for turning source text into an AST.
 *
 * This header defines the data structures and helper functions used by
 * `parser.c` to transform a textual representation of a program into a
 * collection of `statement_t` objects (`statement_list_t`).  The parser
 * recognizes two kinds of statements:
 *
 *   • `TYPE_LABEL`        – a symbolic label (e.g. “loop:”).
 *   • `TYPE_INSTRUCTION` – an instruction with zero or more parameters.
 *
 * Core types:
 *
 *   - `EStatementType` – enumerates the supported statement categories.
 *
 *   - `parameter_t`    – represents a named argument; it can hold either a
 *                        string or a 16‑bit integer, with an accompanying
 *                        `type` discriminator.
 *
 *   - `statement_t`    – a single parsed statement (label or instruction) and
 *                        its arguments.
 *
 *   - `statement_list_t` – a container for an array of `statement_t*` plus a
 *                           count of how many statements were parsed.
 *
 * Public API:
 *
 *   parameter_t *parser_create_parameter_string(const char *name,
 *                                                const char *value);
 *       // Allocate a string‑typed parameter.
 *
 *   parameter_t *parser_create_parameter_integer(const char *name,
 *                                                 int value);
 *       // Allocate an integer‑typed parameter.
 *
 *   statement_t *parser_create_instruction(const char *name,
 *                                           parameter_t **params,
 *                                           size_t param_count);
 *       // Build an instruction node.
 *
 *   statement_t *parser_create_label(const char *name);
 *       // Build a label node.
 *
 *   int parser_parse_string(const char *expr,
 *                 statement_list_t **statements);
 *       // Parse `expr` into a `statement_list_t`. Returns 0 on success,
 *       // non‑zero on error.
 *
 * The API is C‑compatible and can be consumed from C++ code thanks to the
 * `extern "C"` guards.
 *
 * @author  R. Middel
 * @date    2025‑11‑19
 * @license MIT
 */
#ifndef __AST__PARSER_H__
#define __AST__PARSER_H__

/* ------------------------------------------------------------------------- */
/* Standard library includes                                                 */
/* ------------------------------------------------------------------------- */
#include <stddef.h> /* size_t   */
#include <stdint.h> /* uint16_t */

/* ------------------------------------------------------------------------- */
/* C++ compatibility                                                         */
/* ------------------------------------------------------------------------- */
#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------- */
/* Enumerations                                                              */
/* ------------------------------------------------------------------------- */
/**
 * @brief The kind of statement encountered by the parser.
 */
typedef enum EStatementType { TYPE_LABEL, TYPE_INSTRUCTION } EStatementType;

/* ------------------------------------------------------------------------- */
/* Parameter handling                                                        */
/* ------------------------------------------------------------------------- */
typedef struct parameter_s {
  char *name;
  union value {
    char *string;
    uint16_t integer;
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

/* ------------------------------------------------------------------------- */
/* Factory helpers – allocate and initialise structures                      */
/* ------------------------------------------------------------------------- */
parameter_t *parser_create_parameter_string(const char *name,
                                            const char *value);
parameter_t *parser_create_parameter_integer(const char *name, int value);
statement_t *parser_create_instruction(const char *name, parameter_t **params,
                                    size_t param_count);
statement_t *parser_create_label(const char *name);

int parser_parse_string(const char *expr, statement_list_t **statements);
void parser_free_statements(statement_list_t *statements);

#ifdef __cplusplus
}
#endif

#endif // __AST__PARSER_H__