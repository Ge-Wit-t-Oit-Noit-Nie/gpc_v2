/**
 * @file   ast.h
 * @brief  Abstract Syntax Tree (AST) definitions for the parser.
 *
 * This header declares the core data structures used to represent the
 * intermediate representation of a program after parsing:
 *
 *   • `opcode_node_t` – a single opcode together with its operand(s) and
 *     optional label reference.
 *   • `label_node_t` – a symbolic label and the index of the first opcode
 *     that belongs to that label.
 *   • `node_colection_t` – containers that aggregate all opcodes and labels
 *     produced during AST construction.
 *
 * Two conversion passes are exposed:
 *
 *   int ast_convert_iteration_1(const statement_list_t *statements,
 *                          node_collection_t *node_collection);
 *       // First pass: populates `node_collection` with opcodes and labels.
 *
 *   int ast_convert_iteration_2(const statement_list_t *statements);
 *       // Second pass: resolves forward‑references, performs optimizations,
 *     // etc.
 *
 * The API is C‑compatible and can be used from C++ code via the
 * `extern "C"` guards.
 *
 * @author  R. Middel
 * @date   2025‑11‑19
 * @license MIT
 * 
 */
#ifndef __AST_H__
#define __AST_H__

#include "parser.h"

#ifdef __cplusplus
extern "C" {
#endif

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
  size_t index_memory;
} label_node_t;

typedef struct node_collection_t {
  opcode_node_t **opcodes;
  label_node_t **labels;
  size_t labels_count;
  size_t opcode_count;
} node_collection_t;

/**
* @brief Perform the first iteration of conversion
* A normal conversion is done in multiple steps.
* This function will perform the first step.
* 
* @param statements A list of statements that needs to be converted
* @param node_collection The list of AST nodes that is generated in this conversion
* @return 0 for SUCCESS
*/
int ast_convert_iteration_1(const statement_list_t *statements,
                             node_collection_t *node_collection);

/**
* @brief Perform the second iteration of conversion
* A normal conversion is done in multiple steps.
* This function will perform the second step.
* 
* The second step will: 
* 1. Update any references to labels
* @param node_collection The list of AST nodes that is converted in this conversion
* @return 0 for SUCCESS
*/                             
int ast_convert_iteration_2(node_collection_t *node_collection);
/**
 * @brief Free the memory allocated for a node_collection_t
 *
 * @param nc The node_collection_t to free
 */
void ast_free_node_collection(node_collection_t *nc);

#ifdef __cplusplus
}
#endif

#endif // __AST_H__