#include "binary.h"

int binary_write_program(const char *filename, const node_collection_t *nodes)
{

  size_t array_capacity =
      nodes->opcode_count * 2; /* start with 16 bytes (choose any) */
  uint8_t *array_of_bytes = malloc(array_capacity);

  size_t node_index;
  size_t array_index = 0;

  for (node_index = 0; node_index < nodes->opcode_count; node_index++) {
    array_of_bytes[array_index] = nodes->opcodes[node_index]->opcode;

    switch (nodes->opcodes[node_index]->size_in_bytes) {
    case 2:
      array_of_bytes[array_index] |=
          nodes->opcodes[node_index]->register_2bytes >> 8;
      array_index++;
      array_of_bytes[array_index] =
          nodes->opcodes[node_index]->register_2bytes & 0xFF;
      break;
    case 3:
      array_of_bytes[array_index] |=
          (nodes->opcodes[node_index]->register_4bytes >> 16) & 0x01;
      array_index++;
      array_of_bytes[array_index] =
          (nodes->opcodes[node_index]->register_4bytes >> 9) & 0xFF;
      array_index++;
      array_of_bytes[array_index] =
          nodes->opcodes[node_index]->register_4bytes & 0xFF;
      break;
    default:
      break;
    }

    array_index++;
    if(array_index >= array_capacity) {
      size_t new_capacity = array_capacity * 2; /* exponential growth */
      unsigned char *tmp = realloc(array_of_bytes, new_capacity);
      if (!tmp) {
        perror("realloc");
        free(array_of_bytes);
        return EXIT_FAILURE;
      }
      array_of_bytes = tmp;
      array_capacity = new_capacity;

    }
  }

  /* Open (or create) the file for binary output */
  FILE *fp = fopen(filename, "wb");
  if (!fp) {
    perror("fopen");
    free(array_of_bytes);
    return EXIT_FAILURE;
  }

  fwrite(array_of_bytes, 1, array_index, fp);

  /* 4️⃣ Flush and close */
  if (fflush(fp) != 0) {
    free(array_of_bytes);
    perror("fflush");
  }
  fclose(fp);

  free(array_of_bytes);

  return EXIT_SUCCESS;
}