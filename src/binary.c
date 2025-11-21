#include "binary.h"

int binary_write_program(const char *filename, const node_collection_t *nodes) {

  /* Open (or create) the file for binary output */
  FILE *fp = fopen(filename, "wb");
  if (!fp) {
    perror("fopen");
    return EXIT_FAILURE;
  }

  uint8_t *array_of_bytes = malloc(sizeof(uint8_t) * nodes->opcode_count * 4);
  size_t node_index;
  size_t array_index=0;

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
  }

  fwrite(array_of_bytes, 1, array_index, fp);

  /* 4️⃣ Flush and close */
  if (fflush(fp) != 0) {
    perror("fflush");
  }
  fclose(fp);

  return EXIT_SUCCESS;
}