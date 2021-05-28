// C lib headers
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>
#include <string.h>

// External libraries
#include <libexif/exif-data.h>
#include "../external/tinydir.h"
#include "../external/xxhash.h"
#include "../external/argparse.h"

// Our headers
#include "types.h"
#include "intrinsics.h"
#include "constants.h"
#include "../external/pstr.h"

// Unity build for simplicity
#include "../external/xxhash.c"
#include "../external/argparse.c"
#include "../external/pstr.c"


#if defined(__APPLE__) || defined(__NetBSD__)
#define st_atim st_atimespec
#define st_ctim st_ctimespec
#define st_mtim st_mtimespec
#endif


uint32 const MAX_HASHABLE_SIZE = MB_TO_B(10);
char const * const USAGE_PARTS[] = {"fotografiska [options]", NULL};
char const *USAGE_BODY = ""
  "\n"
  "fotografiska\n"
  "------------\n"
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
  "Options:";
char const *USAGE_EPILOGUE = ""
  "\n"
  "The destination directory will follow this directory structure:\n"
  "\n"
  "    2021/\n"
  "      01/\n"
  "      02/\n"
  "        xxxxxxx.jpg\n"
  "        ...\n"
  "      ...\n"
  "    ...\n";


/*!
  Turn a date from YYYY:mm:dd HH:MM:SS to YYYY.mm.dd_HH.MM.SS
*/
static void format_exif_date(char *date) {
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
static bool32 get_exif_tag(
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
static void split_creation_date(
  char const *file_creation_date, char *file_creation_year, char *file_creation_month
) {
  strncpy(file_creation_year, file_creation_date, 4);
  file_creation_year[4] = 0;
  strncpy(file_creation_month, file_creation_date + 5, 2);
  file_creation_month[2] = 0;
}


/*!
  Moves the actual file to the proper place once we've found its new name.
*/
static bool32 move_file_to_dest_dir(
  char const *source_path, char const *dest_dir, char const *file_new_name,
  char const *file_creation_year, char const *file_creation_month
) {
  struct stat st = {};

  // Make sure the first part of the target directory exists (the year)
  char year_directory[MAX_PATH];
  int32 year_directory_n_chars_used = snprintf(
    year_directory, MAX_PATH, "%s/%s", dest_dir, file_creation_year
  );
  if (year_directory_n_chars_used >= MAX_PATH) {
    printf("Your file paths are too long, so we couldn't move this file.\n");
    return false;
  }
  if (stat(year_directory, &st) == -1) {
    printf("Creating directory: %s\n", year_directory);
    if (mkdir(year_directory, 0700) == -1) {
      printf("Could not create directory! Please check you have permissions.\n");
      return false;
    }
  }

  // Make sure the month subdirectory exists
  char month_directory[MAX_PATH];
  int32 month_directory_n_chars_used = snprintf(
    month_directory, MAX_PATH, "%s/%s", year_directory, file_creation_month
  );
  if (month_directory_n_chars_used >= MAX_PATH) {
    printf("Your file paths are too long, so we couldn't move this file.\n");
    return false;
  }
  if (stat(month_directory, &st) == -1) {
    printf("Creating directory: %s\n", month_directory);
    if (mkdir(month_directory, 0700) == -1) {
      printf("Could not create directory! Please check you have permissions.\n");
      return false;
    }
  }

  // Make the final destination path
  char target_path[MAX_PATH];
  int32 target_path_n_chars_used = snprintf(
    target_path, MAX_PATH, "%s/%s", month_directory, file_new_name
  );
  if (target_path_n_chars_used >= MAX_PATH) {
    printf("Your file paths are too long, so we couldn't move this file.\n");
    return false;
  }

  // Move the file!
  if (rename(source_path, target_path) != 0) {
    printf("Could not move the file to its new home! Please check you have permissions.\n");
    return false;
  }

  return true;
}


/*!
  Figures out the new filename and location for a file in the destination dir,
  then puts it there.
*/
static void sort_file_into_dest_dir(
  tinydir_file const *file, char *file_buffer, size_t const file_buffer_size,
  char const *dest_dir, bool32 is_dry_run
) {
  char file_basename[MAX_PATH];
  char file_new_name[MAX_PATH];
  char file_creation_date[20]; // YYYY.mm.dd_HH.MM.SS0
  char file_creation_year[5]; // YYYY0
  char file_creation_month[3]; // mm0
  size_t file_size;
  size_t file_hashable_size;
  FILE *file_handle;

  // Open file
  file_handle = fopen(file->path, "r");
  if (file_handle == NULL) {
    printf("Could not open file %s\n", file->path);
    goto cleanup_return;
  }

  // Get name without extension
  strncpy(file_basename, file->name, MAX_PATH);
  file_basename[strlen(file_basename) - strlen(file->extension) - 1] = 0;

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
    printf("Could not read entire hashable portion of file %s\n", file->path);
    goto cleanup_fclose;
  }

  // Compute the hash
  XXH64_hash_t const hash = XXH64(file_buffer, file_hashable_size, 0);

  int32 file_new_name_chars_used = snprintf(
    file_new_name,
    MAX_PATH,
    "%s_%lx_%s.%s",
    file_creation_date,
    (unsigned long)hash,
    file_basename,
    file->extension
  );

  printf(
    "%s -> %s/%s/%s/%s\n",
    file->path,
    dest_dir,
    file_creation_year,
    file_creation_month,
    file_new_name
  );

  if (file_new_name_chars_used >= MAX_PATH) {
    printf("Your file paths are too long, so we couldn't move this file.\n");
    goto cleanup_fclose;
  }

  if (is_dry_run) {
    printf("(dry run, not doing anything)\n");
  } else {
    bool32 could_move = move_file_to_dest_dir(
      file->path, dest_dir, file_new_name, file_creation_year, file_creation_month
    );
    if (!could_move) {
      goto cleanup_fclose;
    }
  }

cleanup_fclose:
  fclose(file_handle);
cleanup_return:
  ;
}


/*!
  See print_usage() for options.
*/
int main(int argc, const char **argv) {
  char const *src_dir = NULL;
  char const *dest_dir = NULL;
  bool32 is_dry_run;

  struct argparse_option options[] = {
    OPT_HELP(),
    OPT_STRING('i', "src-dir", &src_dir, "a folder containing images/videos to read from"),
    OPT_STRING('o', "dest-dir", &dest_dir, "a folder to move the files from src-dir into"),
    OPT_BOOLEAN('d', "dry-run", &is_dry_run, "don't move files, just print out what would be done"),
    OPT_END(),
  };

  struct argparse argparse;
  argparse_init(&argparse, options, USAGE_PARTS, 0);
  argparse_describe(&argparse, USAGE_BODY, USAGE_EPILOGUE);
  argc = argparse_parse(&argparse, argc, argv);

  if (!src_dir || !dest_dir) {
    argparse_usage(&argparse);
    return 1;
  }

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
    sort_file_into_dest_dir(&file, file_buffer, file_buffer_size, dest_dir, is_dry_run);
  }

  tinydir_close(&dir);

  return EXIT_SUCCESS;
}
