// C lib headers
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// External libraries
#include <libexif/exif-data.h>
#include "external/tinydir.h"
#include "external/xxhash.h"
#include "external/argparse.h"

// Our headers
#include "external/pstr.h"

// Unity build for simplicity
#include "external/xxhash.c"
#include "external/argparse.c"
#include "external/pstr.c"


// Some defines we're going to need
#if defined(__APPLE__) || defined(__NetBSD__)
#define st_atim st_atimespec
#define st_ctim st_ctimespec
#define st_mtim st_mtimespec
#endif

#if !defined(MAX_PATH)
#if defined(PATH_MAX)
#define MAX_PATH PATH_MAX
#else
#define MAX_PATH 260
#endif
#endif

#define KB_TO_B(Value) ((Value) * 1024LL)
#define MB_TO_B(Value) (KB_TO_B(Value) * 1024LL)
#define GB_TO_B(Value) (MB_TO_B(Value) * 1024LL)
#define TB_TO_B(Value) (GB_TO_B(Value) * 1024LL)


static uint32_t const MAX_HASHABLE_SIZE = MB_TO_B(10);
static char const * const USAGE_PARTS[] = {"fotografiska [options]", NULL};
static char const * const USAGE_BODY = "";
static char const * const USAGE_EPILOGUE = ""
  "\n"
  "\n"
  "fotografiska organises your photos/videos into a certain directory structure that is easy\n"
  "to browse with a regular file manager.\n"
  "\n"
  "Your photos/videos should be in a single folder (files in nested folders won't be used).\n"
  "They will be organised into subfolders by year and month, and their filename will start\n"
  "with the date they were taken, as well as including a unique hash of (part of) the file.\n"
  "\n"
  "Here's an example. Let's say your input looks like this:\n"
  "\n"
  "```\n"
  "DSCF4325.JPG (taken 2021/01/01 05:23:11)\n"
  "DSCF1234.JPG (taken 2020/08/27 11:00:00)\n"
  "```\n"
  "\n"
  "They will then be organised as follows:\n"
  "\n"
  "```\n"
  "2020/\n"
  "  02/\n"
  "    2020.08.27_11.00.00_b46976ab6907346a_DSCF1234.JPG\n"
  "2021/\n"
  "  01/\n"
  "    2020.01.01-05.23.11_66f4c6bbab77a615_DSCF4325.JPG\n"
  "```\n"
  "\n"
  "The creation date and time will be taken from the EXIF data. When no EXIF data is\n"
  "available, such as with videos, the file's modification time will be used.\n"
  "\n"
  "**Caveats:**\n"
  "\n"
  "1. Please note that if your photo/video has no EXIF data, and you've e.g. made a copy of\n"
  "   the file so its modification time is not the time it was taken, fotografiska cannot\n"
  "   correctly organise your photos into correct dates and times.\n"
  "\n"
  "2. Always make a backup of your photos/videos before using fotografiska. It's been\n"
  "   reasonably tested, but you probably don't want to lose your photos, so copy them to a\n"
  "   separate folder first just to be safe.\n";


/*!
  Turns a date from "YYYY:mm:dd HH:MM:SS" to "YYYY.mm.dd_HH.MM.SS".
*/
static void format_exif_date(char *date) {
  size_t const len = pstr_len(date);
  for (uint32_t idx = 0; idx < len; idx++) {
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
static bool get_exif_tag(
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
  assert(pstr_copy_n(file_creation_year, 5, file_creation_date, 4));
  assert(pstr_copy_n(file_creation_month, 3, file_creation_date + 5, 2));
}


/*!
  Moves the actual file to the proper place once we've found its new name.
*/
static bool move_file_to_dest_dir(
  char const *source_path, char const *dest_dir, char const *file_new_name,
  char const *file_creation_year, char const *file_creation_month
) {
  struct stat st = {};

  // Make sure the first part of the target directory exists (the year)
  char year_directory[MAX_PATH] = {};
  if (
    !pstr_vcat(year_directory, MAX_PATH, dest_dir, "/", file_creation_year, NULL)
  ) {
    printf("error | Your file paths are too long, so we couldn't move this file.\n");
    return false;
  }
  if (stat(year_directory, &st) == -1) {
    printf("info  | Creating directory: %s\n", year_directory);
    if (mkdir(year_directory, 0700) == -1) {
      printf("ERROR: Could not create directory! Please check you have permissions.\n");
      return false;
    }
  }

  // Make sure the month subdirectory exists
  char month_directory[MAX_PATH] = {};
  if (
    !pstr_vcat(month_directory, MAX_PATH, year_directory, "/", file_creation_month, NULL)
  ) {
    printf("error | Your file paths are too long, so we couldn't move this file.\n");
    return false;
  }
  if (stat(month_directory, &st) == -1) {
    printf("info  | Creating directory: %s\n", month_directory);
    if (mkdir(month_directory, 0700) == -1) {
      printf("error | Could not create directory! Please check you have permissions.\n");
      return false;
    }
  }

  // Make the final destination path
  char target_path[MAX_PATH] = {};
  if (
    !pstr_vcat(target_path, MAX_PATH, month_directory, "/", file_new_name, NULL)
  ) {
    printf("error | Your file paths are too long, so we couldn't move this file.\n");
    return false;
  }

  // Check if the file already exists
  if (stat(target_path, &st) == 0) {
    printf("info  | %s already exists, so we're not going to do anything.\n", target_path);
    return false;
  }

  // Move the file!
  if (rename(source_path, target_path) != 0) {
    printf(
      "error | Could not move the file to its new home! Please check you have permissions.\n"
    );
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
  char const *dest_dir, bool is_dry_run
) {
  char file_basename[MAX_PATH] = {};
  char file_new_name[MAX_PATH] = {};
  char file_creation_date[20] = {}; // YYYY.mm.dd_HH.MM.SS0
  char file_creation_year[5] = {}; // YYYY0
  char file_creation_month[3] = {}; // mm0
  size_t file_size;
  size_t file_hashable_size;
  FILE *file_handle;

  // Open file
  file_handle = fopen(file->path, "r");
  if (file_handle == NULL) {
    printf("error | Could not open file %s\n", file->path);
    goto cleanup_return;
  }

  // Get name without extension
  assert(pstr_copy(file_basename, MAX_PATH, file->name));
  pstr_slice_to(file_basename, pstr_len(file_basename) - pstr_len(file->extension) - 1);

  // Get creation date
  ExifData const *exif_data = exif_data_new_from_file(file->path);
  bool could_get_exif = false;

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
    // I don't love using `localtime()` and `strftime()`, but here we are.
    struct tm const *creation_date_tm = localtime(&file->_s.st_mtime);
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
    printf("error | Could not read entire hashable portion of file %s\n", file->path);
    goto cleanup_fclose;
  }

  // Compute the hash
  XXH64_hash_t const hash = XXH64(file_buffer, file_hashable_size, 0);
  char hash_string[32] = {};
  // TODO: Add a function for this to pstr
  snprintf(hash_string, 32, "%llx", (long long unsigned)hash);

  if (!pstr_vcat(
    file_new_name, MAX_PATH,
    file_creation_date, "_", hash_string, "_", file_basename, ".", file->extension, NULL
  )) {
    printf("error | Your file paths are too long, so we couldn't move this file.\n");
    goto cleanup_fclose;
  }

  printf(
    "info  | %s -> %s/%s/%s/%s\n",
    file->path,
    dest_dir,
    file_creation_year,
    file_creation_month,
    file_new_name
  );

  if (is_dry_run) {
    printf("info  | (dry run, not doing anything)\n");
  } else {
    bool could_move = move_file_to_dest_dir(
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
  Runs fotografiska with commandline arguments.
  See top of the file for arguments.
*/
int main(int argc, const char **argv) {
  struct stat st;
  char const *src_dir = NULL;
  char *dest_dir = NULL;
  bool is_dry_run;

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

  pstr_rtrim_char(dest_dir, '/');

  if (stat(src_dir, &st) != 0) {
    printf("Source directory does not exist.\n");
    return 1;
  }

  if (stat(dest_dir, &st) != 0) {
    printf("Destination directory does not exist.\n");
    return 1;
  }

  size_t const file_buffer_size = MAX_HASHABLE_SIZE;
  char *file_buffer = (char*)malloc(file_buffer_size);

  printf("Reading files from %s\n", src_dir);
  printf("This can sometimes take a moment\n");
  tinydir_dir dir;
  tinydir_open_sorted(&dir, src_dir);
  printf("%s: %zu files\n", src_dir, dir.n_files);

  // Go through over every file in the directory, and try to put it in the right place
  for (uint32_t idx = 0; idx < dir.n_files; idx++) {
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
