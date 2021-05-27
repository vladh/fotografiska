/*!
  pstr string functions
  Vlad-Stefan Harbuz <vlad@vladh.net>
  MIT license
*/

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>


bool pstr_is_valid(char const *str, size_t const size) {
  for (size_t idx = 0; idx < size; idx++) {
    if (str[idx] == 0) {
      return true;
    }
  }
  return false;
}


int64_t pstr_len(char const *str) {
  return strlen(str);
}


bool pstr_is_empty(char const *str) {
  return str[0] == '\0';
}


bool pstr_eq(char const *str1, char const *str2) {
  return strcmp(str1, str2) == 0;
}


bool pstr_starts_with_char(char const *str, char const character) {
  return str[0] == character;
}


bool pstr_starts_with(char const *str, char const *prefix) {
  size_t str_len = pstr_len(str);
  size_t prefix_len = pstr_len(prefix);
  if (str_len == 0 || prefix_len == 0) {
    return false;
  }
  if (str_len < prefix_len) {
    return false;
  }
  return memcmp(str, prefix, prefix_len) == 0;
}


bool pstr_ends_with_char(char const *str, char const character) {
  size_t str_len = pstr_len(str);
  return str[str_len - 1] == character;
}


bool pstr_ends_with(char const *str, char const *prefix) {
  size_t str_len = pstr_len(str);
  size_t prefix_len = pstr_len(prefix);
  if (str_len == 0 || prefix_len == 0) {
    return false;
  }
  if (str_len < prefix_len) {
    return false;
  }
  return memcmp(str + str_len - prefix_len, prefix, prefix_len) == 0;
}


bool pstr_copy(char *dest, size_t const dest_size, char const *src) {
  size_t const src_len = pstr_len(src);

  // If there's no room, return false
  if (dest_size < src_len + 1) {
    return false;
  }

  memcpy(dest, src, src_len);
  dest[src_len] = '\0';

  return true;
}


bool pstr_cat(char *dest, size_t const dest_size, char const *src) {
  size_t const src_len = pstr_len(src);
  size_t const dest_len = pstr_len(dest);
  size_t const free_size = dest_size - dest_len;

  // If there's no room, return false
  if (free_size < src_len + 1 || src_len == 0) {
    return false;
  }

  memcpy(dest + dest_len, src, src_len);
  dest[dest_len + src_len] = '\0';

  return true;
}


bool pstr_split_on_first_occurrence(
  char const *src,
  char *part1, size_t const part1_size,
  char *part2, size_t const part2_size,
  char const separator
) {
  size_t const src_len = pstr_len(src);

  // Find separator
  char const *separator_start = strchr(src, separator);
  if (!separator_start) {
    return false;
  }
  size_t idx_separator = separator_start - src;

  // Find how much space we need before and after the separator
  size_t const src_len_before_sep = idx_separator;
  size_t const src_len_after_sep = src_len - idx_separator - 1;

  // Return if we don't have enough space
  if (part1_size < src_len_before_sep + 1 || part2_size < src_len_after_sep + 1) {
    return false;
  }

  memcpy(part1, src, src_len_before_sep);
  memcpy(part2, separator_start + 1, src_len_after_sep);

  return true;
}


void pstr_clear(char *str) {
  str[0] = '\0';
}


bool pstr_slice_from(char *str, size_t const start) {
  size_t const str_len = pstr_len(str);
  if (start >= str_len) {
    return false;
  }
  uint32_t idx = 0;
  char *cursor = &str[start];
  do {
    str[idx++] = *cursor;
    cursor++;
  } while (*cursor != 0);
  str[idx++] = 0;
  return true;
}


bool pstr_slice_to(char *str, size_t const end) {
  size_t const str_len = pstr_len(str);
  if (end >= str_len) {
    return false;
  }
  str[end] = 0;
  return true;
}


bool pstr_slice(char *str, size_t const start, size_t const end) {
  if (start >= end) {
    return false;
  }
  return pstr_slice_to(str, end) && pstr_slice_from(str, start);
}


/* Helper for sdscatlonglong() doing the actual number -> string
 * conversion. 's' must point to a string with room for at least
 * SDS_LLSTR_SIZE bytes.
 *
 * The function returns the length of the null-terminated string
 * representation stored at 's'. */
#define SDS_LLSTR_SIZE 21
int sdsll2str(char *s, long long value) {
    char *p, aux;
    unsigned long long v;
    size_t l;

    /* Generate the string representation, this method produces
     * an reversed string. */
    v = (value < 0) ? -value : value;
    p = s;
    do {
        *p++ = '0'+(v%10);
        v /= 10;
    } while(v);
    if (value < 0) *p++ = '-';

    /* Compute length and add null term. */
    l = p-s;
    *p = '\0';

    /* Reverse the string. */
    p--;
    while(s < p) {
        aux = *s;
        *s = *p;
        *p = aux;
        s++;
        p--;
    }
    return l;
}
