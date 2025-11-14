# GWTONN Language Specification – v1.0

**Authors:** Rudi Middel <r.middel@mrbussy.eu>
**Date:** 2025‑11‑14  
**Revision History**

| Version | Date       | Author(s) | Changes |
|---------|------------|-----------|---------|
| 1.0     | 2025‑11‑14 | R. Middel    | Initial release |

## 1. Scope & Audience
*(Brief description of who should read this doc and why.)*

## 2. Lexical Syntax
### 2.1 Character Set & Encoding
GWTONN Language uses UTF‑8...

### 3.2 Tokens
| Token | Pattern | Description |
|-------|---------|-------------|
| `DIGIT` | `[0-9]` | A Single digit |
| `LETTER` | `[A-Za-z_]` | A single letter (or underscore) |
| `NUMBER` | `{DIGIT}+` | Decimal integer |
| `STRING` | `{LETTER}+` | String |

### 3.3 Comments
tbd

## 4. Grammar (EBNF)
The GWTONN Language uses the following gammar.

```
programma ::= label* instruction*

label ::= IDENTIFIER ":"

instruction ::= IDENTIFIER "(" param_list? ")" ";" 
param_list := IDENTIFIER? VALUE ","?
```

## 5. Semantics
### 5.1 Scoping
Block‑scoped variables; functions introduce a new lexical scope.

### 5.2 Type System
- Primitive types: `int`, `bool`, `string`.
- Composite: `array<T>`, `fn(T1,…,Tn) -> R`.
- No implicit coercion; explicit casts required.

### 5.3 Evaluation Order
Expressions are evaluated left‑to‑right; side effects occur in that order.

## 6. Standard Library (placeholder)

## 7. Reference Implementation
- Repository: [https://github.com/Ge-Wit-t-Oit-Noit-Nie/gpc](https://github.com/Ge-Wit-t-Oit-Noit-Nie/gpc)
- Build: `cmake -B build && cd build && make .`
- Supported platforms: Linux.

## 9. Appendices
### A. Full Bison Grammar (`bison.y`)

[bison.y](/src/bison.y)

*End of Specification*

