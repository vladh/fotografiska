#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "external/tinydir.h"
#include "external/xxhash.h"

#include "types.h"

#include "external/xxhash.c"


const char IN_DIR[] = "test/in/";


int main() {
  size_t buffer_size = MB_TO_B(256);
  char *f_buffer = (char*)malloc(buffer_size);
  size_t f_size;
  FILE *f_handle = NULL;
  tinydir_file f_info;

  tinydir_dir dir;
  tinydir_open_sorted(&dir, IN_DIR);
  printf("%zu files\n", dir.n_files);

  for (uint32 i = 0; i < dir.n_files; i++) {
    tinydir_readfile_n(&dir, &f_info, i);
    if (f_info.name[0] == '.' || f_info.is_dir) { continue; }

    f_handle = fopen(f_info.path, "r");
    if (f_handle == NULL) {
      printf("ERROR: Could not open file %s\n", f_info.name);
      continue;
    }
    fseek(f_handle, 0, SEEK_END);
    f_size = ftell(f_handle);
    if (f_size >= buffer_size) {
      printf("ERROR: File is too big %s\n", f_info.path);
      fclose(f_handle);
      continue;
    }
    fseek(f_handle, 0, SEEK_SET);
    if (fread(f_buffer, 1, f_size, f_handle) == 0) {
      printf("ERROR: File is too big %s\n", f_info.path);
      fclose(f_handle);
      continue;
    }
    fclose(f_handle);
    printf("%s is %zu\n", f_info.path, f_size);
    f_buffer[f_size] = 0;

    XXH64_hash_t hash = XXH64(f_buffer, f_size, 0);
    printf("%s -> %lx\n", f_info.name, hash);
  }

  tinydir_close(&dir);

  return EXIT_SUCCESS;
}
