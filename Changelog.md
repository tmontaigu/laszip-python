# 0.2.3 30 May 2023

- Fix LasUnZipper::decompress and LasZipper::compress when the number
  of bytes to compress/decompress exceeded the mac capacity of the
  inner stringstream used.

# 0.2.2 8 Feb 2023

- Added missing MIT License

# 0.2.1 26 Dec 2022

- Fixed selective decompressor constructor

# 0.2.0 26 Dec 2022

- Added support for selective decompression

# 0.1.0 30 May 2021

- Added `seek` method to LasUnZipper

# 0.0.1 28 Dec 2020

- Initial release
- Added LasUnZipper
- Added LasZipper
- Added LasZipDll
- Added bindings to laszip_header
- Added bindings to laszip_point