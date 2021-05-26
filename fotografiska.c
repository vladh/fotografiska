#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>
#include <string.h>

#include <libexif/exif-data.h>
#include "external/tinydir.h"
#include "external/xxhash.h"

#include "types.h"

#include "external/xxhash.c"


const uint32 MAX_HASHABLE_SIZE = MB_TO_B(10);


/*!
  Turn a date from YYYY:mm:dd HH:MM:SS to YYYY.mm.dd_HH.MM.SS
*/
void format_exif_date(char *date) {
  size_t const len = strlen(date);
  for (uint32 idx = 0; idx < len; idx++) {
    if (date[idx] == ':') {
      date[idx] = '.';
    } else if (date[idx] == ' ') {
      date[idx] = '-';
    }
  }
}


/*!
  Returns the value of an EXIF `tag` in the buffer `buf`.
*/
bool32 get_exif_tag(
  ExifData const *d, ExifIfd const ifd, ExifTag const tag,
  char *buf, size_t const buf_size
) {
  ExifEntry *entry = exif_content_get_entry(d->ifd[ifd], tag);
  if (entry) {
    exif_entry_get_value(entry, buf, buf_size);
    return true;
  }
  return false;
}


/*!
  Parses a YYYY.mm.dd_HH.MM.SS Date to extract the year into `file_creation_year`
  and the month into `file_creation_month`.
*/
void split_creation_date(
  char const *file_creation_date, char *file_creation_year, char *file_creation_month
) {
  strncpy(file_creation_year, file_creation_date, 4);
  file_creation_year[4] = 0;
  strncpy(file_creation_month, file_creation_date + 5, 2);
  file_creation_month[2] = 0;
}


/*!
  Puts a file into its correct place in the output directory.
*/
void move_file(
  tinydir_file const *file, char *file_buffer, size_t const file_buffer_size,
  char const *dest_dir
) {
  uint32 const BASENAME_MAX_LENGTH = 160;
  char file_basename[BASENAME_MAX_LENGTH];
  char file_new_path[MAX_PATH];
  char file_creation_date[20]; // YYYY.mm.dd_HH.MM.SS0
  char file_creation_year[5]; // YYYY0
  char file_creation_month[3]; // mm0
  size_t file_size;
  size_t file_hashable_size;
  FILE *file_handle;

  // Open file
  file_handle = fopen(file->path, "r");
  if (file_handle == NULL) {
    printf("ERROR: Could not open file %s\n", file->path);
    goto cleanup_return;
  }

  // Get name without extension
  // Limit the number of characters, because we will put this basename into a longer
  // path later.
  strncpy(file_basename, file->name, BASENAME_MAX_LENGTH - 1);
  if (strlen(file_basename) < BASENAME_MAX_LENGTH - 1) {
    // Only chop off the extension if we didn't truncate the filename
    file_basename[strlen(file_basename) - strlen(file->extension) - 1] = 0;
  } else {
    file_basename[BASENAME_MAX_LENGTH - 1] = 0;
  }

  // Get creation date
  ExifData const *exif_data = exif_data_new_from_file(file->path);
  bool32 could_get_exif = false;

  if (exif_data) {
    could_get_exif = get_exif_tag(
      exif_data, EXIF_IFD_0, EXIF_TAG_DATE_TIME,
      file_creation_date, sizeof(file_creation_date)
    );
  }

  // If we could get the EXIF data, great, format it.
  // If not, get the creation date from the filemtime.
  if (could_get_exif) {
    format_exif_date(file_creation_date);
  } else {
    struct tm const *creation_date_tm = localtime(&file->_s.st_mtim.tv_sec);
    strftime(
      file_creation_date, sizeof(file_creation_date),
      "%Y.%m.%d_%H.%M.%S", creation_date_tm
    );
  }

  // Set file_creation_year and file_creation_month
  split_creation_date(file_creation_date, file_creation_year, file_creation_month);

  // Get file size (we will only hash a max of MAX_HASHABLE_SIZE bytes)
  fseek(file_handle, 0, SEEK_END);
  file_size = ftell(file_handle);
  if (file_size >= file_buffer_size) {
    file_hashable_size = file_buffer_size;
  } else {
    file_hashable_size = file_size;
  }

  // Read hashable portion into file_buffer
  fseek(file_handle, 0, SEEK_SET);
  if (fread(file_buffer, 1, file_hashable_size, file_handle) < file_hashable_size) {
    printf("ERROR: Could not read entire hashable portion of file %s\n", file->path);
    goto cleanup_fclose;
  }

  // Compute the hash
  XXH64_hash_t const hash = XXH64(file_buffer, file_hashable_size, 0);

  snprintf(
    file_new_path,
    MAX_PATH,
    "%s/%s/%s/%s_%lx_%s.%s",
    dest_dir,
    file_creation_year,
    file_creation_month,
    file_creation_date,
    hash,
    file_basename,
    file->extension
  );

  printf("%s -> %s\n", file->path, file_new_path);

cleanup_fclose:
  fclose(file_handle);
cleanup_return:
  ;
}


void print_usage() {
  char const *USAGE = ""
    "fotograiska\n"
    "-----------\n"
    "\n"
    "fotografiska will read images/videos from an input directory, name them according to\n"
    "a certain schema, and put them in a YYYY/mm/ folder structure in a destination\n"
    "directory.\n"
    "\n"
    "The final filename will look something like this:\n"
    "  YYYY.mm.dd_hh.mm.ss_hashcafebabe_originalname.ext\n"
    "for example:\n"
    "  2020.09.16-18.38.23_4487bfd46ccb74b7_DSCF4506.JPG\n"
    "\n"
    "Usage: ./fotografiska <source_dir> <dest_dir>\n"
    "  source_dir: A folder containing images/videos to read from\n"
    "  dest_dir: A folder to move the files from source_dir into, with the following structure:\n"
    "    2021/\n"
    "      01/\n"
    "      02/\n"
    "        xxxxxxx.jpg\n"
    "        ...\n"
    "      ...\n"
    "    ...\n";
  printf("%s", USAGE);
}


/*!
*/
int main(int argc, char **argv) {
  if (argc < 3) {
    print_usage();
    return 1;
  }

  char const *src_dir = argv[1];
  char const *dest_dir = argv[2];

  size_t const file_buffer_size = MAX_HASHABLE_SIZE;
  char *file_buffer = (char*)malloc(file_buffer_size);

  tinydir_dir dir;
  tinydir_open_sorted(&dir, src_dir);
  printf("%s: %zu files\n", src_dir, dir.n_files);

  // Go through over every file in the directory, and try to put it in the right place
  for (uint32 idx = 0; idx < dir.n_files; idx++) {
    tinydir_file file;
    tinydir_readfile_n(&dir, &file, idx);
    if (file.name[0] == '.' || file.is_dir) {
      continue;
    }
    move_file(&file, file_buffer, file_buffer_size, dest_dir);
  }

  tinydir_close(&dir);

  return EXIT_SUCCESS;
}
