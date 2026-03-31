BigTIFF Design
==============


This design details a 64-bit (larger than 4GB) TIFF format specification.
The design is based on a proposal by Steve Carlsen of Adobe, with input
from various other parties.


Briefly
-------

* Version = 43
* 8-byte offset to first IFD
* Value/Offset fields are 8 bytes
* 8-byte offset to the next IFD
* add ``TIFFType`` of ``LONG8``, an 8 byte (unsigned) int
* ``StripOffsets`` and ``TileOffsets`` and ``ByteCounts`` can be LONG8


More Detail
-----------

* The Version ID, in header bytes 2-3, formerly decimal 42, now changes to **43**
* Header bytes 4-5 contain the decimal number **8**.

  * If there is some other number here, a reader should give up.
  * This is to provide a nice way to move to 16-byte pointers some day.

* Header bytes 6-7 are reserved and must be **zero**.

  * If they're not, a reader should give up.

* Header bytes 8-15 contain the 8-byte offset to the first IFD.
* Value/Offset fields are 8 bytes long, and take up bytes 8-15 in an IFD entry.

  * If the value is ≤ 8 bytes, it must be stored in the field.
  * All values must begin at an 8-byte-aligned address.

* 8-byte offset to the Next_IFD, at the end of an IFD.
* To keep IFD entries 8-byte-aligned, we begin with an 8-byte (instead of 2-byte) count of the number of directory entries.
* Add ``TIFFTypes`` of ``LONG8`` (= 16), an 8 byte (unsigned) int, and ``SLONG8`` (= 17).
* Add ``TIFFType`` ``IFD8`` (=18) an 8byte IFD offset.
* ``StripOffsets`` and ``TileOffsets`` and ``ByteCounts`` may be ``LONG8`` or the traditionally allowed ``LONG`` or ``SHORT``.

* The proposed extension is :file:`.tf8`, and call it "8-Byte TIFF".

  Otherwise, it's just like "original TIFF." ("TIFF Classic?")


Open Issues
-----------

* What to call the new format

  * ChrisCox -- I don't think end users will understand what "8-byte TIFF" means
  * AndreyKiselev - 23 Sep 2004 -- What about TIFF64? "64" is a widely used buzzword and should be directly associated with the 64-bit offsets and 64-bit architectures.

* What 3 character file extension to use (gotta be DOS compatible)
* What 4 character file type to use (for Macintosh)
* What MIME type to use


Samples
-------

`Example files <http://www.awaresystems.be/imaging/tiff/bigtiff/BigTIFFSamples.zip>`_
from Joris Van Damme.


Changes
-------

* ``TIFFType`` 13 is ``ttIFD``, 14 is assigned to ``ttUnicode``, and 15 is assigned to ``ttComplex``. So, I changed the types for ``ttLong8`` and ``ttSLong8`` to 16 and 17, respectively.

  * AndreyKiselev - 23 Sep 2004 -- Where are these fields defined? Is there any new Technical Note or something? And what is encoding behind the word "Unicode"?
  * ChrisCox - 27 Sep 2004 -- They are in the Adobe TIFF definitions.  I am still working on releasing updated TIFF documentation.

* Added list of open issues.
* settle on version 43
* cleanup
* ``TIFFType`` 18 (8 byte IFD) added.

* Clarified that fields which may be ``LONG8`` can also be one of the old supported types.


See also
--------

`AWare Systems' informal overview of the BigTIFF proposal <http://www.awaresystems.be/imaging/tiff/bigtiff.html>`_.
