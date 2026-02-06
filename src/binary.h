/**
 * @file   binary.h
 * @brief  Interact with the binary program file.
 *
 * This header declares the core functions to store the binary output.
 * if converts the AST into a binary file.
 * 
 * The API is C‑compatible and can be used from C++ code via the
 * `extern "C"` guards.
 *
 * @author  R. Middel
 * @date   2025‑11‑19
 * @license MIT
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef __AST_BINARY_H__
#define __AST_BINARY_H__

#include "ast.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Write the AST nodes to a binary file
 * 
 * During the AST conversion, a list of opcodes and parameter values is created.
 * This function takes this collection and writes it to the specified file.
 *
 * @param filename the full path of the file to create
 * @param nodes all the ast nodes to write as a binary
 * @return 0 for SUCCESS; On failure, use perror to find the correct errorcode
 */
int binary_write_program(const char *filename, const node_collection_t *nodes);

#ifdef __cplusplus
}
#endif

#endif //__AST_BINARY_H__