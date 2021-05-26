#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>

#include <libexif/exif-data.h>
#include "external/tinydir.h"
#include "external/xxhash.h"

#include "types.h"

#include "external/xxhash.c"


typedef struct FileInfo {
  char basename[MAX_PATH];
  char creation_date[128];
  size_t file_size;
  size_t hashable_size;
  tinydir_file file;
  FILE *handle;
} FileInfo;


const char IN_DIR[] = "test/in/";
const uint32 MAX_HASHABLE_SIZE = MB_TO_B(10);


void format_date(char *date) {
  size_t len = strlen(date);
  for (uint32 idx = 0; idx < len; idx++) {
    if (date[idx] == ':') {
      date[idx] = '.';
    } else if (date[idx] == ' ') {
      date[idx] = '-';
    }
  }
}


/* Show the tag name and contents if the tag exists */
static bool32 get_tag(ExifData *d, ExifIfd ifd, ExifTag tag, char *buf, size_t buf_size) {
  /* See if this tag exists */
  ExifEntry *entry = exif_content_get_entry(d->ifd[ifd], tag);
  if (entry) {
    /* Get the contents of the tag in human-readable form */
    exif_entry_get_value(entry, buf, buf_size);
    return true;
  }
  return false;
}


void move_file(FileInfo *finfo, char *file_buffer, size_t file_buffer_size) {
  // Open file
  finfo->handle = fopen(finfo->file.path, "r");
  if (finfo->handle == NULL) {
    printf("ERROR: Could not open file %s\n", finfo->file.path);
    goto cleanup_return;
  }

  // Get name without extension
  strcpy(finfo->basename, finfo->file.name);
  finfo->basename[strlen(finfo->file.name) - strlen(finfo->file.extension) - 1] = 0;

  // Get EXIF date
  ExifData *exif_data = exif_data_new_from_file(finfo->file.path);
  bool32 could_get_exif = false;

  if (exif_data) {
    could_get_exif = get_tag(
      exif_data, EXIF_IFD_0, EXIF_TAG_DATE_TIME,
      finfo->creation_date, sizeof(finfo->creation_date)
    );
  }

  // If we could get the EXIF data, great, format it.
  // If not, get the creation date from the filemtime.
  if (could_get_exif) {
    format_date(finfo->creation_date);
  } else {
    struct tm *creation_date_tm = localtime(&finfo->file._s.st_mtim.tv_sec);
    strftime(
      finfo->creation_date, sizeof(finfo->creation_date),
      "%Y.%m.%d_%H.%M.%S", creation_date_tm
    );
  }

  // Get file size (we will only hash a max of MAX_HASHABLE_SIZE bytes)
  fseek(finfo->handle, 0, SEEK_END);
  finfo->file_size = ftell(finfo->handle);
  if (finfo->file_size >= file_buffer_size) {
    finfo->hashable_size = file_buffer_size;
  } else {
    finfo->hashable_size = finfo->file_size;
  }

  // Read hashable portion into file_buffer
  fseek(finfo->handle, 0, SEEK_SET);
  if (fread(file_buffer, 1, finfo->hashable_size, finfo->handle) < finfo->hashable_size) {
    printf("ERROR: Could not read entire hashable portion of file %s\n", finfo->file.path);
    goto cleanup_fclose;
  }

  // Compute the hash
  XXH64_hash_t hash = XXH64(file_buffer, finfo->hashable_size, 0);

  printf(
    "%s (name %s) (ext %s) (hs %zu) (exif %d) (date %s) -> %lx\n",
    finfo->file.path,
    finfo->basename,
    finfo->file.extension,
    finfo->hashable_size,
    could_get_exif,
    finfo->creation_date,
    hash
  );

cleanup_fclose:
  fclose(finfo->handle);
cleanup_return:
  ;
}


int main() {
  size_t file_buffer_size = MAX_HASHABLE_SIZE;
  char *file_buffer = (char*)malloc(file_buffer_size);

  tinydir_dir dir;
  tinydir_open_sorted(&dir, IN_DIR);
  printf("%s: %zu files\n", IN_DIR, dir.n_files);

  for (uint32 idx = 0; idx < dir.n_files; idx++) {
    FileInfo finfo = {};
    // Get files in directory
    tinydir_readfile_n(&dir, &finfo.file, idx);
    if (finfo.file.name[0] == '.' || finfo.file.is_dir) {
      continue;
    }
    // Try to put this file in the right place
    move_file(&finfo, file_buffer, file_buffer_size);
  }

  tinydir_close(&dir);

  return EXIT_SUCCESS;
}
