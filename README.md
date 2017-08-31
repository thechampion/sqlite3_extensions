# sqlite3_extensions

## hashlib

Extension that adds hash functions md5, sha1, sha224, sha256, sha384, and sha512. Each function accepts only argument
and returns its hex digest.

## uriparse

Extension that adds URI parsing function uriparse_host. This function extracts hostname from URI.


## Build

hashlib uses openssl, uriparse uses uriparser library. In Debian, make sure that packages libssl-dev and
liburiparser-dev are installed, then run make.
