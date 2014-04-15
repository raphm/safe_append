# safe-append

A small library that attempts to guarantee that an append to a file
either succeeds or is rolled back.

This library could potentially be useful to any application that needs
to append data to a file in a periodic manner (i.e., recording time
series values or audio values to disk).

## Primary purpose

The primary purpose of this library is to help protect against data
corruption due to power failures. It is designed to be used with my
time series data store. The library does not protect against in-place
corruption, but merely attempts to guarantee that a write either
completes successfully or is rolled back.

## Rationale

The problem with appending records of a fixed (or, I suppose, of
variable) size to a file is that most file systems increase the length
of the file before writing the data. In the event of an application crash, a
file system crash, a system crash, or a power failure, the file length
could indicate that the record had been successfully written, but the
new portion of the file might be filled with garbage data.

In the case of an occurrence of one of the situations described above,
this library will truncate the file to its original length. This means
that we will lose data, but we will not corrupt the original file. 

No attempt is made to store or recover any uncommitted data. This library
operates on the principle that no data is better than bad data.

Early performance tests on a laptop drive indicate that tens of
commits/second are easily attainable (at the expense of throughput,
naturally --- seeks are expensive).

## Why you should not use this library

This library does not make use of nor does it pay attention to
advisory locks. It was intended to be used by a single writer thread.
It shouldn't be hard to extend it to deal with file locking, but that
is not an important issue for my intended use of it. It is perfectly
possible, and probably inevitable if you don't add some sort of
locking, for two unsynchronized processes or threads to happily clobber
one another's journals. I'd be happy to take patches that add advisory
locking support.

This library *only* supports append operations. Recovering from failed
appends is relatively easy, because when a disk dies in the middle of
writing out a data block, the garbage is going to be at the end. This
is easily dealt with by truncating the file to the old length. 

There are ways to safely support in-place modify, but as it is not
important for my purposes, I have not implemented that functionality
yet.

The library does *not* deal with the situation in which the write to
the file's metadata becomes corrupted. It is potentially possible for
the length of the file to be corrupted (i.e., set to be too short or
0) if the drive dies during a metadata write. The library relies on
the operating system to deal with that situation. Please use a
journaling file system. Also, backups.

This library does not guarantee that data that has already been
written will be protected from corruption. 

This library does not *guarantee* that data will be successfully
written or will be rolled back. It merely makes it *less* likely that
a power failure or other crash will result in bad blocks of data at
the end of the file. If you really can't afford data corruption, then
RAID, multiple storage servers, UPSs, and good journaling file systems
are all things you should consider putting in place.

## Requirements

This library requires C++11.

Also, you will need Boost. I'm sorry to include it for such a tiny
library, but the thought of writing my own filesystem handling
routines made me sad, especially in light of the fact that I will most
likely be the only user of this code.

## Example usage

0) `#include "safe_append.h"`

1) Call `sa_start` with the name of the file:

`sa_start("test/bigfile.txt");`

2) Append data to the file `bigfile.txt`.

3) Call `sa_commit`:

`sa_commit("test/bigfile.txt");`

That's it. You can use `sa_status` to see whether the transaction is:

* `sa::hot`, which indicates there is a valid journal file for
  the transaction, but that the write did not complete; 

* `sa::dirty`, which indicates that there is journal file for the
  transaction, but that it is invalid (empty, bad checksum,
  etc.);

* `sa::clean`, which indicates that there is no journal file for
  the transaction, so a write either started or completed.

Use `sa-rollback()` to roll back a failed (`sa::hot`) write and
truncate the data file. Use `sa-clean()` to clean a dirty
(invalid/partially written) journal file (without modifying the data
file).

## License and Disclaimer

This software &copy; 2014 Raphael Martelles and is released under the
following license:

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", FOR NO CONSIDERATION, WITHOUT
WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. USE AT YOUR OWN RISK.

## Included Software

SHA1 code comes from [Paul E. Jones](http://www.packetizer.com/security/sha1/). The license file is
in the `include/` directory.

SHA512 code comes from [Olivier Gay](http://www.ouah.org/ogay/sha2/).
The license is reproduced in the source code.
