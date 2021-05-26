#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <libexif/exif-data.h>
#include "external/tinydir.h"
#include "external/xxhash.h"

#include "types.h"

#include "external/xxhash.c"


typedef struct FileInfo {
  size_t file_size;
  size_t hashable_size;
  tinydir_file file;
  FILE *handle;
} FileInfo;


const char IN_DIR[] = "test/in/";
const uint32 MAX_HASHABLE_SIZE = MB_TO_B(10);


void move_file(FileInfo *f, char *buffer, size_t buffer_size) {
  // Open file
  f->handle = fopen(f->file.path, "r");
  if (f->handle == NULL) {
    printf("ERROR: Could not open file %s\n", f->file.path);
    goto cleanup_return;
  }

  // Get file size (we will only hash a max of MAX_HASHABLE_SIZE bytes)
  fseek(f->handle, 0, SEEK_END);
  f->file_size = ftell(f->handle);
  if (f->file_size >= buffer_size) {
    f->hashable_size = buffer_size;
  } else {
    f->hashable_size = f->file_size;
  }

  // Read hashable portion into buffer
  fseek(f->handle, 0, SEEK_SET);
  if (fread(buffer, 1, f->hashable_size, f->handle) < f->hashable_size) {
    printf("ERROR: Could not read entire hashable portion of file %s\n", f->file.path);
    goto cleanup_fclose;
  }

  // Compute the hash
  XXH64_hash_t hash = XXH64(buffer, f->hashable_size, 0);

  printf("%s (%zu) -> %lx\n", f->file.path, f->hashable_size, hash);

cleanup_fclose:
  fclose(f->handle);
cleanup_return:
  ;
}


int main() {
  FileInfo f;
  size_t buffer_size = MAX_HASHABLE_SIZE;
  char *buffer = (char*)malloc(buffer_size);

  tinydir_dir dir;
  tinydir_open_sorted(&dir, IN_DIR);
  printf("%s: %zu files\n", IN_DIR, dir.n_files);

  for (uint32 idx = 0; idx < dir.n_files; idx++) {
    // Get files in directory
    tinydir_readfile_n(&dir, &f.file, idx);
    if (f.file.name[0] == '.' || f.file.is_dir) {
      continue;
    }
    // Try to put this file in the right place
    move_file(&f, buffer, buffer_size);
  }

  tinydir_close(&dir);

  return EXIT_SUCCESS;
}
