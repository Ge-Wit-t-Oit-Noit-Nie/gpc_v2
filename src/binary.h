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
 *
 */
#ifndef __AST_BINARY_H__
#define __AST_BINARY_H__

#include <clogger.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "ast.h"

#ifdef __cplusplus
extern "C" {
#endif

int binary_write_program(const char *filename, const node_collection_t *nodes);

#ifdef __cplusplus
}
#endif

#endif //__AST_BINARY_H__