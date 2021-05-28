![pstr string functions](images/character_bouhan_camera_sm1.png)

# fotografiska

fotografiska organises your photos/videos into a certain directory structure that is easy
to browse with a regular file manager.

Your photos/videos should be in a single folder (files in nested folders won't be used).
They will be organised into subfolders by year and month, and their filename will start
with the date they were taken, as well as including a unique hash of (part of) the file.

Here's an example. Let's say your input looks like this:

```
DSCF4325.JPG (taken 2021/01/01 05:23:11)
DSCF1234.JPG (taken 2020/08/27 11:00:00)
```

They will then be organised as follows:

```
2020/
  02/
    2020.08.27_11.00.00_b46976ab6907346a_DSCF1234.JPG
2021/
  01/
    2020.01.01-05.23.11_66f4c6bbab77a615_DSCF4325.JPG
```

The creation date and time will be taken from the EXIF data. When no EXIF data is
available, such as with videos, the file's modification time will be used.

**Caveats:**

1. Please note that if your photo/video has no EXIF data, and you've e.g. made a copy of
   the file so its modification time is not the time it was taken, fotografiska cannot
   correctly organise your photos into correct dates and times.

2. Always make a backup of your photos/videos before using fotografiska. It's been
   reasonably tested, but you probably don't want to lose your photos, so copy them to a
   separate folder first just to be safe.

## Dependencies

* [libexif](https://github.com/libexif/libexif)

You can install libexif with e.g. `apt install libexif`. It also depends on gettext,
autopoint and libtool, which your package manager should install automatically.

## Building

You can easily build fotografiska with:

```shell
mkdir bin
cd bin
cmake ..
cd ..
make
```

## Running

To see the usage message, run:

```shell
./fotografiska --help
```

Typical usage looks like:

```shell
./fotografiska --src-dir my_photos/ --out-dir organised_photos/
```

Do a dry run first to confirm that everything looks ok:

```shell
./fotografiska --src-dir my_photos/ --out-dir organised_photos/ --dry-run
```

## Roadmap

* Add precompiled builds (in particular for Windows)
* Move away from libexif and towards something that (1) is more lightweight
  and (2) supports HEIC

## License

```
fotografiska

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
