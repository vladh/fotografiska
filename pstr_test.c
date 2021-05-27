#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "pstr.h"

#include "pstr.c"


static void print_test_group(char const *name) {
  printf("\n%s\n", name);
  printf("--------------------\n");
}


static void print_test(char const *message, bool const result) {
  if (result) {
    printf("✓ pass | %s\n", message);
  } else {
    printf("× FAIL | %s\n", message);
  }
}


static void test_pstr_is_valid() {
  size_t const size = 6;
  char const valid[] = {'h', 'e', 'l', 'l', 'o', 0};
  char const valid_shorter[] = {'h', 'e', 0};
  char const invalid[] = {'h', 'e', 'l', 'l', 'o', 0xcc, 0xcc, 0xcc};

  print_test_group("pstr_is_valid()");
  print_test(
    "Valid string is recognised as such",
    pstr_is_valid(valid, size)
  );
  print_test(
    "Valid (but sligtly shorter) string is recognised as valid",
    pstr_is_valid(valid_shorter, size)
  );
  print_test(
    "Invalid string is recognised as such",
    !pstr_is_valid(invalid, size)
  );
}


static void test_pstr_len() {
  print_test_group("pstr_len()");
  print_test(
    "Length of \"Magpie\" is 6",
    pstr_len("Magpie") == 6
  );
  print_test(
    "Length of \"pie\" is 3",
    pstr_len("pie") == 3
  );
}


static void test_pstr_is_empty() {
  print_test_group("pstr_is_empty()");
  print_test(
    "Empty string is empty",
    pstr_is_empty("")
  );
  print_test(
    "Non-empty string is non-empty",
    !pstr_is_empty("Magpie")
  );
}


static void test_pstr_eq() {
  print_test_group("pstr_eq()");
  print_test(
    "Equal strings are equal",
    pstr_eq("Magpie", "Magpie")
  );
  print_test(
    "Different strings are not equal",
    !pstr_eq("Magpie", "Magpin")
  );
}


static void test_pstr_starts_with_char() {
  print_test_group("pstr_starts_with_char()");
  print_test(
    "\"Magpie\" starts with 'M'",
    pstr_starts_with_char("Magpie", 'M')
  );
  print_test(
    "\"Magpie\" does not start with 'm'",
    !pstr_starts_with_char("Magpie", 'm')
  );
  print_test(
    "\"Magpie\" does not start with '\\0'",
    !pstr_starts_with_char("Magpie", '\0')
  );
}


static void test_pstr_starts_with() {
  print_test_group("pstr_starts_with()");
  print_test(
    "\"Magpie\" starts with \"Mag\"",
    pstr_starts_with("Magpie", "Mag")
  );
  print_test(
    "\"Magpie\" does not start with \"ag\"",
    !pstr_starts_with("Magpie", "ag")
  );
  print_test(
    "\"Magpie\" does not start with \"Mississippi\"",
    !pstr_starts_with("Magpie", "Mississippi")
  );
  print_test(
    "\"Magpie\" does not start with \"\"",
    !pstr_starts_with("Magpie", "")
  );
}


static void test_pstr_ends_with_char() {
  print_test_group("pstr_ends_with_char()");
  print_test(
    "\"Magpie\" ends with 'e'",
    pstr_ends_with_char("Magpie", 'e')
  );
  print_test(
    "\"Magpie\" does not end with 'x'",
    !pstr_ends_with_char("Magpie", 'x')
  );
  print_test(
    "\"Magpie\" does not end with '\\0'",
    !pstr_ends_with_char("Magpie", '\0')
  );
}


static void test_pstr_ends_with() {
  print_test_group("pstr_ends_with()");
  print_test(
    "\"Magpie\" ends with \"pie\"",
    pstr_ends_with("Magpie", "pie")
  );
  print_test(
    "\"Magpie\" does not end with \"pi\"",
    !pstr_ends_with("Magpie", "pi")
  );
  print_test(
    "\"Magpie\" does not end with \"Mississippi\"",
    !pstr_ends_with("Magpie", "Mississippi")
  );
  print_test(
    "\"Magpie\" does not end with \"\\0\"",
    !pstr_ends_with("Magpie", "\\0")
  );
}


static void test_pstr_copy() {
  print_test_group("test_pstr_copy()");
  bool did_succeed;
  size_t const dest_size = 6;
  char dest[dest_size];
  char const src_short[] = {'h', 'e', 'y', 0};
  char const src_snug[] = {'h', 'e', 'l', 'l', 'o', 0};
  char const src_a_bit_long[] = {'h', 'e', 'l', 'l', 'o', '!', 0};
  char const src_long[] = {'h', 'e', 'y', 'y', 'y', 'y', 'y', 'y', 0};

  memset(dest, 0, dest_size);
  did_succeed = pstr_copy(dest, dest_size, src_short);
  print_test(
    "A buffer with enough space receives a short string",
    did_succeed && memcmp(dest, "hey\0\0\0", dest_size) == 0
  );

  memset(dest, 0, dest_size);
  did_succeed = pstr_copy(dest, dest_size, src_snug);
  print_test(
    "A buffer with just enough space receives a snug string",
    did_succeed && memcmp(dest, "hello\0", dest_size) == 0
  );

  memset(dest, 0, dest_size);
  did_succeed = pstr_copy(dest, dest_size, src_a_bit_long);
  print_test(
    "A buffer does not receive a string that is one byte too long to fit",
    !did_succeed && memcmp(dest, "\0\0\0\0\0\0", dest_size) == 0
  );

  memset(dest, 0, dest_size);
  did_succeed = pstr_copy(dest, dest_size, src_long);
  print_test(
    "A buffer does not receive a string that is much too long to fit",
    !did_succeed && memcmp(dest, "\0\0\0\0\0\0", dest_size) == 0
  );
}


static void test_pstr_cat() {
  print_test_group("test_pstr_cat()");
  bool did_succeed;
  size_t const dest_size = 8;
  char dest[dest_size];
  char const src_short[] = {'n', 't', 0};
  char const src_snug[] = {'t', 'h', 'e', 'r', 'e', 0};
  char const src_a_bit_long[] = {'t', 'h', 'e', 'r', 'e', '!', 0};
  char const src_long[] = {'h', 'e', 'y', 'y', 'y', 'y', 'y', 'y', 0};

  memcpy(dest, "hi\0\0\0\0\0\0", dest_size);
  did_succeed = pstr_cat(dest, dest_size, src_short);
  print_test(
    "A string with enough remaining space receives a string that fits comfortably",
    did_succeed && memcmp(dest, "hint\0\0\0\0", dest_size) == 0
  );

  memcpy(dest, "hi\0\0\0\0\0\0", dest_size);
  did_succeed = pstr_cat(dest, dest_size, src_snug);
  print_test(
    "A string with just enough remaining space receives a string that fits snugly",
    did_succeed && memcmp(dest, "hithere\0", dest_size) == 0
  );

  memcpy(dest, "hi\0\0\0\0\0\0", dest_size);
  did_succeed = pstr_cat(dest, dest_size, src_a_bit_long);
  print_test(
    "A string does not receive a string that is one byte too long to fit in the "
    "remaining space",
    !did_succeed && memcmp(dest, "hi\0\0\0\0\0\0", dest_size) == 0
  );

  memcpy(dest, "hi\0\0\0\0\0\0", dest_size);
  did_succeed = pstr_cat(dest, dest_size, src_long);
  print_test(
    "A string does not receive a string that is much too long to fit in the "
    "remaining space",
    !did_succeed && memcmp(dest, "hi\0\0\0\0\0\0", dest_size) == 0
  );
}


static void test_pstr_split_on_first_occurrence() {
  print_test_group("test_pstr_split_on_first_occurrence()");
  bool did_succeed;
  size_t part1_size = 6;
  char part1[part1_size];
  size_t part2_size = 7;
  char part2[part2_size];
  char const src_good_fit[] = "hi,thar";
  char const src_snug_fit[] = "hello,there!";
  char const src_sep_at_start[] = ",there!";
  char const src_sep_at_end[] = "hello,";
  char const src_double_sep[] = "hello,,there";
  char const src_no_sep[] = "hello";
  char const src_empty[] = "";
  char const src_part1_too_long[] = "hello!,there!";
  char const src_part2_too_long[] = "hello,there!!";
  char const src_part1_much_too_long[] = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx,there!";
  char const src_part2_much_too_long[] = "hello,xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

  memcpy(part1, "\0\0\0\0\0\0", part1_size);
  memcpy(part2, "\0\0\0\0\0\0\0", part2_size);
  did_succeed = pstr_split_on_first_occurrence(
    src_good_fit, part1, part1_size, part2, part2_size, ','
  );
  print_test(
    "A string is split when there is enough space",
    did_succeed && memcmp(part1, "hi\0\0\0\0", part1_size) == 0 &&
      memcmp(part2, "thar\0\0\0", part2_size) == 0
  );

  memcpy(part1, "\0\0\0\0\0\0", part1_size);
  memcpy(part2, "\0\0\0\0\0\0\0", part2_size);
  did_succeed = pstr_split_on_first_occurrence(
    src_snug_fit, part1, part1_size, part2, part2_size, ','
  );
  print_test(
    "A string is split when there is just enough space",
    did_succeed && memcmp(part1, "hello\0", part1_size) == 0 &&
      memcmp(part2, "there!\0", part2_size) == 0
  );

  memcpy(part1, "\0\0\0\0\0\0", part1_size);
  memcpy(part2, "\0\0\0\0\0\0\0", part2_size);
  did_succeed = pstr_split_on_first_occurrence(
    src_sep_at_start, part1, part1_size, part2, part2_size, ','
  );
  print_test(
    "When the separator is at the start, the first part is empty and the second part is "
    "correctly copied",
    did_succeed && memcmp(part1, "\0\0\0\0\0\0", part1_size) == 0 &&
      memcmp(part2, "there!\0", part2_size) == 0
  );

  memcpy(part1, "\0\0\0\0\0\0", part1_size);
  memcpy(part2, "\0\0\0\0\0\0\0", part2_size);
  did_succeed = pstr_split_on_first_occurrence(
    src_sep_at_end, part1, part1_size, part2, part2_size, ','
  );
  print_test(
    "When the separator is at the end, the second part is empty and the first part is "
    "correctly copied",
    did_succeed && memcmp(part1, "hello\0", part1_size) == 0 &&
      memcmp(part2, "\0\0\0\0\0\0\0", part2_size) == 0
  );

  memcpy(part1, "\0\0\0\0\0\0", part1_size);
  memcpy(part2, "\0\0\0\0\0\0\0", part2_size);
  did_succeed = pstr_split_on_first_occurrence(
    src_double_sep, part1, part1_size, part2, part2_size, ','
  );
  print_test(
    "When the separator appears twice, only the first occurrence is used",
    did_succeed && memcmp(part1, "hello\0", part1_size) == 0 &&
      memcmp(part2, ",there\0", part2_size) == 0
  );

  memcpy(part1, "\0\0\0\0\0\0", part1_size);
  memcpy(part2, "\0\0\0\0\0\0\0", part2_size);
  did_succeed = pstr_split_on_first_occurrence(
    src_no_sep, part1, part1_size, part2, part2_size, ','
  );
  print_test(
    "When there is no separator, nothing is copied",
    !did_succeed && memcmp(part1, "\0\0\0\0\0\0", part1_size) == 0 &&
      memcmp(part2, "\0\0\0\0\0\0\0", part2_size) == 0
  );

  memcpy(part1, "\0\0\0\0\0\0", part1_size);
  memcpy(part2, "\0\0\0\0\0\0\0", part2_size);
  did_succeed = pstr_split_on_first_occurrence(
    src_empty, part1, part1_size, part2, part2_size, ','
  );
  print_test(
    "When the source is empty, nothing is copied",
    !did_succeed && memcmp(part1, "\0\0\0\0\0\0", part1_size) == 0 &&
      memcmp(part2, "\0\0\0\0\0\0\0", part2_size) == 0
  );

  memcpy(part1, "\0\0\0\0\0\0", part1_size);
  memcpy(part2, "\0\0\0\0\0\0\0", part2_size);
  did_succeed = pstr_split_on_first_occurrence(
    src_part1_too_long, part1, part1_size, part2, part2_size, ','
  );
  print_test(
    "When the first part is too long, nothing is copied",
    !did_succeed && memcmp(part1, "\0\0\0\0\0\0", part1_size) == 0 &&
      memcmp(part2, "\0\0\0\0\0\0\0", part2_size) == 0
  );

  memcpy(part1, "\0\0\0\0\0\0", part1_size);
  memcpy(part2, "\0\0\0\0\0\0\0", part2_size);
  did_succeed = pstr_split_on_first_occurrence(
    src_part2_too_long, part1, part1_size, part2, part2_size, ','
  );
  print_test(
    "When the second part is too long, nothing is copied",
    !did_succeed && memcmp(part1, "\0\0\0\0\0\0", part1_size) == 0 &&
      memcmp(part2, "\0\0\0\0\0\0\0", part2_size) == 0
  );

  memcpy(part1, "\0\0\0\0\0\0", part1_size);
  memcpy(part2, "\0\0\0\0\0\0\0", part2_size);
  did_succeed = pstr_split_on_first_occurrence(
    src_part1_much_too_long, part1, part1_size, part2, part2_size, ','
  );
  print_test(
    "When the first part is much too long, nothing is copied",
    !did_succeed && memcmp(part1, "\0\0\0\0\0\0", part1_size) == 0 &&
      memcmp(part2, "\0\0\0\0\0\0\0", part2_size) == 0
  );

  memcpy(part1, "\0\0\0\0\0\0", part1_size);
  memcpy(part2, "\0\0\0\0\0\0\0", part2_size);
  did_succeed = pstr_split_on_first_occurrence(
    src_part2_much_too_long, part1, part1_size, part2, part2_size, ','
  );
  print_test(
    "When the second part is much too long, nothing is copied",
    !did_succeed && memcmp(part1, "\0\0\0\0\0\0", part1_size) == 0 &&
      memcmp(part2, "\0\0\0\0\0\0\0", part2_size) == 0
  );
}


static void test_pstr_clear() {
  print_test_group("test_pstr_clear()");
  char str[] = "hello!";
  pstr_clear(str);
  print_test(
    "A string is successfully cleared",
    str[0] == 0
  );
}


static void test_pstr_slice_from() {
  print_test_group("test_pstr_slice_from()");
  bool did_succeed;
  char str[9];

  memcpy(str, ",,hello!\0", 9);
  did_succeed = pstr_slice_from(str, 2);
  print_test(
    "A slice is correctly performed",
    did_succeed && memcmp(str, "hello!\0", 7) == 0
  );

  memcpy(str, ",,hello!\0", 9);
  did_succeed = pstr_slice_from(str, 7);
  print_test(
    "The last character is returned when slicing at the last index",
    did_succeed && str[0] == '!'
  );

  memcpy(str, ",,hello!\0", 9);
  did_succeed = pstr_slice_from(str, 8);
  print_test(
    "A slice is not performed if the position is past the end of the string",
    !did_succeed && memcmp(str, ",,hello!\0", 9) == 0
  );
}


static void test_pstr_slice_to() {
  print_test_group("test_pstr_slice_to()");
  bool did_succeed;
  char str[6];

  memcpy(str, "hello\0", 6);
  did_succeed = pstr_slice_to(str, 2);
  print_test(
    "A slice is correctly performed",
    did_succeed && memcmp(str, "he\0", 3) == 0
  );

  memcpy(str, "hello\0", 6);
  did_succeed = pstr_slice_to(str, 4);
  print_test(
    "The last character can be sliced off",
    did_succeed && memcmp(str, "hell\0", 5) == 0
  );

  memcpy(str, "hello\0", 6);
  did_succeed = pstr_slice_to(str, 5);
  print_test(
    "A slice is not performed if the position is past the end of the string",
    !did_succeed && memcmp(str, "hello\0", 6) == 0
  );
}


static void test_pstr_slice() {
  print_test_group("test_pstr_slice()");
  bool did_succeed;
  char str[9];

  memcpy(str, "hi there\0", 9);
  did_succeed = pstr_slice(str, 1, 7);
  print_test(
    "A start-end slice is correctly performed",
    did_succeed && memcmp(str, "i ther\0", 7) == 0
  );

  memcpy(str, "hi there\0", 9);
  did_succeed = pstr_slice(str, 1, 8);
  print_test(
    "A slice is not performed if the end is too far",
    !did_succeed && memcmp(str, "hi there\0", 9) == 0
  );


  memcpy(str, "hi there\0", 9);
  did_succeed = pstr_slice(str, 1, 1);
  print_test(
    "A slice is not performed if start == end",
    !did_succeed && memcmp(str, "hi there\0", 9) == 0
  );
}


int main(int argc, char **argv) {
  test_pstr_is_valid();
  test_pstr_len();
  test_pstr_is_empty();
  test_pstr_eq();
  test_pstr_starts_with_char();
  test_pstr_starts_with();
  test_pstr_ends_with_char();
  test_pstr_ends_with();
  test_pstr_copy();
  test_pstr_cat();
  test_pstr_split_on_first_occurrence();
  test_pstr_clear();
  test_pstr_slice_from();
  test_pstr_slice_to();
  test_pstr_slice();
}
