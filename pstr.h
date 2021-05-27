/*!
  pstr string functions
  Vlad-Stefan Harbuz <vlad@vladh.net>
  MIT license
*/

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>


// Information functions
// These functions all assume the strings they are passed are valid
// ---------------------

/*!
  Returns whether or string `str` is valid, meaning that it has a NULL terminator
  within its `size` bytes.
*/
bool pstr_is_valid(char const *str, size_t const size);

/*!
  If `str` is a valid string, returns its length.
  If it isn't, returns -1.
*/
int64_t pstr_len(char const *str);

/*!
  Returns whether or not string `str` has length 0, i.e. starts with a NULL byte.
*/
bool pstr_is_empty(char const *str);

/*!
  Returns whether or not `str` and `str2` are equal.
*/
bool pstr_eq(char const *str1, char const *str2);

/*!
  Returns whether or not string `str` starts with `character`.
*/
bool pstr_starts_with_char(char const *str, char const character);

/*!
  Returns whether or not string `str` starts with the string `prefix`.
*/
bool pstr_starts_with(char const *str, char const *prefix);

/*!
  Returns whether or not string `str` ends with `character`.
  This check will not match the NULL terminator.
*/
bool pstr_ends_with_char(char const *str, char const character);

/*!
  Returns whether or not string `str` ends with the string `prefix`.
  This check will not match the NULL terminator.
*/
bool pstr_ends_with(char const *str, char const *prefix);


// Transformation functions
// These functions try hard not to make an invalid string
// ------------------------

/*!
  Tries to copy `src` into `dest`, requiring `strlen(src) + 1` bytes in `dest`,
  to allow for the NULL terminator. If successful, returns true.
  If it won't fit, it does not copy anything, and returns false.
*/
bool pstr_copy(char *dest, size_t const dest_size, char const *src);

/*!
  Tries to add `src` onto the end of `dest`. If there is enough space, the copy
  proceeds and true is returned. If there isn't enough space, nothing is copied
  and false is returned.
*/
bool pstr_cat(char *dest, size_t const dest_size, char const *src);

/*!
  Finds `separator` in `src`, puts the part before it into `part1`,
  and the part after it into `part2`. Returns true if it succeeded.
  Returns false if there wasn't enough space or if the separator was not found,
  in which case nothing is copied.
*/
bool pstr_split_on_first_occurrence(
  char const *src,
  char *part1, size_t const part1_size,
  char *part2, size_t const part2_size,
  char const separator
);

/*!
  Empties a string by settings its first character to the NULL terminator.
*/
void pstr_clear(char *str);

/*!
  Replaces `str` with the portion of the string starting from index `start`, discarding
  the old leading characters. Returns true if it succeeded.
  Fails if start > strlen(str), in which case false is returned.
*/
bool pstr_slice_from(char *str, size_t const start);

/*!
  Cuts `str` off at index `end`, adding a NULL terminator and discarding the old end.
  Returns true if it succeeded. Fails if end > strlen(str), in which case false is
  returned.
*/
bool pstr_slice_to(char *str, size_t const end);

/*!
  Replaces `str with its substring from index `start` to index `end`.
  Returns true if it succeeded. Fails if start or end > strlen(str), in which case false
  is returned.
*/
bool pstr_slice(char *str, size_t const start, size_t const end);

/*!
*/
void pstr_ltrim(char *str);

/*!
*/
void pstr_rtrim(char *str);

/*!
*/
void pstr_trim(char *str);

/*!
*/
void pstr_ltrim_char(char *str, char const *target);

/*!
*/
void pstr_rtrim_char(char *str, char const *target);

/*!
*/
void pstr_trim_char(char *str, char const *target);


// Creation functions
// These functions make a string from scratch
// ------------------------

/*!
*/
void pstr_from_int64(char *str, uint64_t const n);
