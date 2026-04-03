/**
 * @file  ast.c
 * @brief  Implementation of the Abstract Syntax Tree (AST) utilities.
 *
 * This source file provides the concrete logic for converting a parsed
 * statement list (`statement_list_t` from `parser.h`) into the internal
 * representation defined in `ast.h`.  The conversion proceeds in two
 * passes, matching the prototypes declared in `ast.h`:
 *
 *   1️⃣ `ast_convert_iteration_1()` – walks the statement list, creates
 *      `opcode_node_t` and `label_node_t` objects, and fills a
 *      `node_collection_t` with the resulting arrays.  Forward‑reference
 *      label names are stored in `opcode_node_t.label_ref` for later
 *      resolution.
 *
 *   2️⃣ `ast_convert_iteration_2()` – resolves the label references
 *      created in the first pass, optionally performs optimisations
 *      (e.g. dead‑code elimination, opcode folding) and finalises the
 *      `node_collection_t` for downstream consumption (code‑gen,
 *      interpreter, etc.).
 *
 * Public symbols (implemented here, declared in `ast.h`):
 *
 *   - int ast_convert_iteration_1(const statement_list_t *statements,
 *                                 node_collection_t *node_collection);
 *
 *   - int ast_convert_iteration_2(const statement_list_t *statements);
 *
 * In addition to the two conversion functions, the file defines several
 * static helper routines that are **not** part of the public API:
 *
 *   static opcode_node_t *make_opcode_node(uint8_t opcode,
 *                                      uint16_t reg2,
 *                                  uint32_t reg4,
 *                                           const char *label_ref);
 *
 *   static label_node_t  *make_label_node(const char *label,
 *                                         size_t index_first_opcode);
 *
 *   static void           free_node_collection(node_collection_t *nc);
 *
 *   static int            resolve_labels(node_collection_t *nc);
 *
 * All memory allocated for the AST nodes is owned by the caller of
 * `ast_convert_iteration_1`.  The caller is responsible for releasing the
 * resources using the appropriate free functions (you may expose a public
 * `ast_free_node_collection()` wrapper if desired).
 *
 * The implementation assumes that the parser has already performed basic
 * syntactic validation; therefore, most error paths here report logical
 * inconsistencies (e.g., undefined label) rather than malformed tokens.
 *
 * @author  R. Middel
 * @date    2025‑11‑19
 * @license MIT
 */
#include <clogger.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "ast.h"
#include "parser.h"

opcode_node_t *ast_generate_opcode_node(const statement_t *statement)
{
  opcode_node_t *node = malloc(sizeof(opcode_node_t));

  /*
   * # | Element | Bitmask               | Hex    | Parameter |
   * # | ------- | --------------------- | ------ | --------- |
   * # | OPCODE  | 0b0001 0000           | 0x10   |           |
   */
  if (0 == strcasecmp("pauze", statement->name)) {
    node->opcode = 0x10;
    node->size_in_bytes = 1;
    clog_info(__FILE_NAME__, "pauze omgezet naar opcode 0x%02X", node->opcode);
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
      free(node);
      exit(EXIT_FAILURE);
    }
    if (INTEGER != statement->args.params[0]->type) {
      clog_error(__FILE_NAME__, "Wachten moet een numerieke waarde hebben als "
                                "parameter: Wachten(10);");
      free(node);
      exit(EXIT_FAILURE);
    }
    node->register_2bytes = statement->args.params[0]->value.integer;
    clog_info(__FILE_NAME__, "WACHTEN(%d) omgezet naar opcode 0x%02X",
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
      free(node);
      exit(EXIT_FAILURE);
    }
    if ((0 == strcasecmp("default", statement->args.params[0]->name)) ||
        (0 == strcasecmp("default", statement->args.params[1]->name))) {
      clog_error(__FILE_NAME__,
                 "De parameters van zet_poort_aan moeten hsio en poort zijn.");
      free(node);

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

    clog_info(__FILE_NAME__, "zet_poort_aan(%d) omgezet naar opcode 0x%02X",
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
      free(node);
      exit(EXIT_FAILURE);
    }
    if ((0 == strcasecmp("default", statement->args.params[0]->name)) ||
        (0 == strcasecmp("default", statement->args.params[1]->name))) {
      clog_error(__FILE_NAME__,
                 "De parameters van zet_poort_uit moeten hsio en poort zijn.");
      free(node);
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

    clog_info(__FILE_NAME__, "zet_poort_uit(%d) omgezet naar opcode 0x%02X",
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
      free(node);
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

    clog_info(__FILE_NAME__, "flip_poort(%d) omgezet naar opcode 0x%02X",
              node->register_2bytes, node->opcode);
  }
  
  /*
   * | Element | Bitmask               | Hex    | Parameter         |
   * | ------- | --------------------- | ------ | ----------------- |
   * | OPCODE  | 0b0101 0000 0000 0000 | 0x5000 |                   |
   */
  if (0 == strcasecmp("bewaar_status", statement->name)) {
    node->opcode = 0x50;
    node->size_in_bytes = 1;
    clog_info(__FILE_NAME__, "Bewaar_status omgezet naar opcode 0x%02X", node->opcode);
  }

  /*
   * | Element | Bitmask                         | Hex      | Parameter | 
   * | ------- | ------------------------------- | ------   | ----------------- |
   * | OPCODE  | 0b0111 0000 0000 0000 0000 0000 | 0x600000 | - | 
   * | LABEL   | 0b0000 0001 1111 1111 1111 1111 | 0x01FFFF | INDEX             |
   */
  if (0 == strcasecmp("spring", statement->name)) {
    node->opcode = 0x60;
    node->size_in_bytes = 3;
    node->label_ref = statement->args.params[0]->value.string;
    clog_info(__FILE_NAME__, "SPRING(%s) omgezet naar opcode 0x%02X",
              node->label_ref, node->opcode);
  }
  
  /*
   * | Element | Bitmask                         | Hex      | Parameter | 
   * | ------- | ------------------------------- | ------   | --- |
   * | OPCODE  | 0b0111 0000 0000 0000 0000 0000 | 0x700000 | - | 
   * | EVENT   | 0b0000 1110 0000 0000 0000 0000 | 0x0E0000 | EVENT |
   * | LABEL   | 0b0000 0001 1111 1111 1111 1111 | 0x01FFFF | INDEX |
   *
   * Beschrijving van de events:
   * - stop: Dit event wordt geactiveerd via de CAN bus en zorgt ervoor dat het
   *          programma onmiddellijk stopt.
   * - pauze: Dit event wordt geactiveerd via de CAN bus en zorgt ervoor dat
   *          het programma onmiddellijk pauzeert.
   * - vervolg: Dit event wordt geactiveerd via de CAN bus en zorgt ervoor dat
   *          het programma vervolgt na een pauze.
   * - int00 t/m int11: Deze events worden geactiveerd door een interrupt
   *                    op de respectievelijke poorten (0 t/m 11) en kunnen
   *                    worden gebruikt om specifieke acties te triggeren
   *                    wanneer een interrupt plaatsvindt.
   *
   * NOTE: De event-naam wordt mee geregistreerd in the opcode. De reden is 
   * dat de hoogste 3 bits niet meegenomen worden uit de register_2bytes.
   * Tevens wordt de label_ref omgezet en ingevult op een speciale manier.
   */
  if (0 == strcasecmp("verbind_event_met_functie", statement->name)) {

    if (2 != statement->args.count) {
      clog_error(__FILE_NAME__,
                 "verbind_event_met_functie heeft 2 parameter nodig (event, label)");
      exit(EXIT_FAILURE);
    }

    node->size_in_bytes = 3;
    if (0 == strcasecmp("stop", statement->args.params[0]->value.string)) {
      node->opcode = 0x70;
    } else if (0 == strcasecmp("pauze", statement->args.params[0]->value.string)) {
      node->opcode = 0x71;
    } else if (0 == strcasecmp("vervolg", statement->args.params[0]->value.string)) {
      node->opcode = 0x72;
    } else if (0 == strcasecmp("int00", statement->args.params[0]->value.string)) {
      node->opcode = 0x73;
    } else if (0 == strcasecmp("int01", statement->args.params[0]->value.string)) {
      node->opcode = 0x74;
    } else if (0 == strcasecmp("int02", statement->args.params[0]->value.string)) {
      node->opcode = 0x75;
    } else if (0 == strcasecmp("int03", statement->args.params[0]->value.string)) {
      node->opcode = 0x76;
    } else if (0 == strcasecmp("int04", statement->args.params[0]->value.string)) {
      node->opcode = 0x77;
    } else if (0 == strcasecmp("int05", statement->args.params[0]->value.string)) {
      node->opcode = 0x78;
    } else if (0 == strcasecmp("int06", statement->args.params[0]->value.string)) {
      node->opcode = 0x79;
    } else if (0 == strcasecmp("int07", statement->args.params[0]->value.string)) {
      node->opcode = 0x7A;
    } else if (0 == strcasecmp("int08", statement->args.params[0]->value.string)) {
      node->opcode = 0x7B;
    } else if (0 == strcasecmp("int09", statement->args.params[0]->value.string)) {
      node->opcode = 0x7C;
    } else if (0 == strcasecmp("int10", statement->args.params[0]->value.string)) {
      node->opcode = 0x7D;
    } else if (0 == strcasecmp("int11", statement->args.params[0]->value.string)) {
      node->opcode = 0x7E;
    } else {
            clog_error(__FILE_NAME__,
                 "verbind_event_met_functie heeft geen geldig event als parameter (stop, pauze, vervolg)");
      exit(EXIT_FAILURE);
    }

    node->label_ref = statement->args.params[1]->value.string;
    clog_info(__FILE_NAME__, "event %s is verbonden met label %s en omgezet naar opcode 0x%02X",
              statement->args.params[0]->value.string, node->label_ref, node->opcode);
  }

  /*
   * | Element | Bitmask               | Hex    | Parameter |
   * | ------- | --------------------- | ------ | --------- |
   * | OPCODE  | 0b1111 1111		       | 0xFF   |           |
   */
  if (0 == strcasecmp("stoppen", statement->name)) {
    node->opcode = 0xFF;
    node->size_in_bytes = 1;
    clog_info(__FILE_NAME__, "STOPPEN omgezet naar opcode 0x%02X", node->opcode);
  }

  return node;
}

/**
 * The first itteration of a conversion
 * @param statements
 *  List of the statements read by the parser
 * @return a collection of opcode_node_t and label_node_t packaged as node_t *
 */
int ast_convert_iteration_1(const statement_list_t *statements,
                            node_collection_t *node_collection)
{

  opcode_node_t **opcodes = malloc(sizeof(opcode_node_t *) * statements->count);
  label_node_t **labels = malloc(sizeof(label_node_t *) * statements->count);

  size_t index_label = 0;
  size_t index_opcode = 0;

  for (int index = 0; index < statements->count; index++) {
    switch (statements->statements[index]->kind) {
    case TYPE_LABEL:
      labels[index_label] = malloc(sizeof *labels[index_label]);
      labels[index_label]->index_first_opcode =
          (index_opcode > 0) ? index_opcode - 1 : 0;
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

  node_collection->labels = labels;
  node_collection->labels_count = index_label;
  node_collection->opcodes = opcodes;
  node_collection->opcode_count = index_opcode;

  return EXIT_SUCCESS;
}

/**
 * @brief Second itteration of the ast conversion
 *
 * The second round of the conversion. This round will:
 * 1. Try to match label index to the actual byte offset from the start
 * 2. Convert all label references to the byte offset counted in 1.
 *
 *
 */
int ast_convert_iteration_2(node_collection_t *node_collection)
{

  size_t opcode_size_count[node_collection->opcode_count];

  opcode_size_count[0] = node_collection->opcodes[0]->size_in_bytes;
  // create a temporary array with opco index and size so far
  for (size_t node_index = 1; node_index < node_collection->opcode_count;
       node_index++) {
    opcode_size_count[node_index] =
        node_collection->opcodes[node_index]->size_in_bytes +
        opcode_size_count[node_index - 1];
  }

  // Assign the correct memory index to each label
  for (size_t label_index = 0; label_index < node_collection->labels_count;
       label_index++) {
    node_collection->labels[label_index]->index_memory =
        opcode_size_count[node_collection->labels[label_index]
                              ->index_first_opcode];
  }

  // Resolve all label references in opcodes
  for (size_t node_index = 0; node_index < node_collection->opcode_count;
       node_index++) {
    if (NULL != node_collection->opcodes[node_index]->label_ref) {
      for (size_t label_index = 0; label_index < node_collection->labels_count;
           label_index++) {
        if (0 == strcasecmp(node_collection->opcodes[node_index]->label_ref,
                            node_collection->labels[label_index]->label)) {
          node_collection->opcodes[node_index]->register_4bytes =
              node_collection->labels[label_index]->index_memory;
          clog_info(__FILE_NAME__,
                    "Resolved label reference '%s' to memory index %02x",
                    node_collection->opcodes[node_index]->label_ref,
                    node_collection->labels[label_index]->index_memory);
          break;
        }
      }
    }
  }

  return EXIT_SUCCESS;
}

/**
 * @brief Free the memory allocated for a node_collection_t
 *
 * @param nc The node_collection_t to free
 */
void ast_free_node_collection(node_collection_t *nc)
{
  if(NULL==nc)
    return;

  for (size_t i = 0; i < nc->opcode_count; i++) {
    free(nc->opcodes[i]);
  }
  for (size_t i = 0; i < nc->labels_count; i++) {
    if (nc->labels[i]->label) {
      free(nc->labels[i]->label);
    }
    free(nc->labels[i]);
  }
  free(nc->opcodes);

}