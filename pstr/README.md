![pstr string functions](images/saihou_itodama.png)

# pstr — a set of cautious and easy-to-use C string functions

This is a set of functions that allows you to more easily work with static C strings
without worrying about safety as much. pstr has two principles:

**1. If a string doesn't fit, it stops what it's doing and tells you**

Functions like `strcat`, `strncat` and `strlcat` try to fit as much of the source string
into the destination buffer as possible. This means that you either end up with safety
issues like buffer overflows, or in the best case scenario, truncated strings, which can
also present problems.

```c
strlcat(dest, "this string is way too big", 10); 
// concatenated `dest` is truncated, and you have to do an extra check to detect this
```

pstr never overflows (hopefully) and never truncates strings. Most functions return a
`bool` to represent whether the operation succeeded. For example, in `pstr_cat`, if the
strings don't fit into the buffer, it returns `false` without changing anything.

```c
pstrcat(dest, 10, "this string is way too big"); 
// returns false without changing `dest`
```

**2. It's easier to work with (than `<string.h>` functions)**

Even when everything fits inside your buffer and you don't have to worry about safety,
doing simple things such as concatenating multiple strings can be a pain in C. pstr
functions make this a little bit simpler. For example:

```c
pstr_vcat(dest, dest_size, " Hello", " there ", name, "!", NULL);
```

## Documentation

## FAQ

**I've found a safety issue with one of the functions!**

Issues and pull requests are welcome! :)

**Why not use a dynamic string library such as [sds](https://github.com/antirez/sds)?**

sds is great, and is a better solution in many cases, but it's sometimes nice to be able
to work with static strings --- they're easy to serialize to disk, don't need a
`malloc()` call, are better in embedded environments and so on. Plus, for a lot of simple
string handling situations, you don't need your strings to be able to grow to any length.
It's nice to be able to work with static strings without dealing with awful `<string.h>`
functions with `strcat`.

**Why is it called pstr?**

I wrote it for my game engine [Peony](https://vladh.net).

## License

```
pstr string functions

Copyright 2021 Vlad-Stefan Harbuz <vlad@vladh.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this
software and associated documentation files (the "Software"), to deal in the Software
without restriction, including without limitation the rights to use, copy, modify,
merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies
or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
THE USE OR OTHER DEALINGS IN THE SOFTWARE.
```

Icon by [irasutoya](https://www.irasutoya.com)
