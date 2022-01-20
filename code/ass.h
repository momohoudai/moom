/* date = January 20th 2022 10:14 am */

#ifndef ASS_H
#define ASS_H

#include <stdlib.h>
#include <stdio.h>
#include "momo.h"

#define ass_log(...) printf(__VA_ARGS__)

// Utility files for ass
Memory ass_malloc(UMI size) {
  void* mem = malloc(size);
  assert(mem);
  return { mem, size };
}

void ass_free(Memory* mem) {
  free(mem->data);
  mem->data = nullptr;
  mem->size = 0;
}

Memory ass_read_file(const char* filename) {
  FILE* file = fopen(filename, "rb");
  
  if (!file) {
    return {};
  }
  defer { fclose(file); };
  fseek(file, 0, SEEK_END);
  UMI file_size = ftell(file);
  fseek(file, 0, SEEK_SET);
  
  void* file_memory = malloc(file_size); 
  UMI read_amount = fread(file_memory, 1, file_size, file);
  assert(read_amount == file_size);
  
  Memory ret;
  ret.data = file_memory;
  ret.size = file_size; 
  
  return ret;
  
}



#include "ass_atlas_builder.h"

#endif //ASS_H