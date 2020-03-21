# sqlite3_extensions

 1. hashlib provides functions md5, sha1, sha224, sha256, sha384, and sha512. Each function accepts only argument
    and returns its hex digest.
 2. uriparse splits uri into components; can be used in join clauses.


## Build

hashlib uses openssl, uriparse uses uriparser library. In Debian, make sure that packages libssl-dev and
liburiparser-dev are installed, then run cmake.


## Usage

### hashlib

```sh
sqlite> .load ./hashlib
sqlite> select sha1('qwerty');
b1b3773a05c0ed0176787a4f1574ff0075f7521e
sqlite> select sha256('sqlite3_extensions');
cbed27dc0ea0a68b10bda6948a91137d15afe7d00303eaf7e471797c2829ab3e
```

### uriparse

```sh
sqlite> .load ./uriparse
sqlite> .headers on
sqlite> select * from uriparse('mailto:user@example.com');
scheme|userinfo|host|port|path|query|fragment
mailto||||user@example.com||
```

Top 10 popular sites from Firefox history:
```sh
$ sqlite3 file:///home/me/.mozilla/firefox/j9u6d239.default-1579823307375/places.sqlite?immutable=true
sqlite> select host, count(*) from moz_places join uriparse on url = uri group by host order by count(*) desc limit 10;
www.google.de|19118
www.google.com|7174
www.youtube.com|5675
github.com|2438
gstreamer.freedesktop.org|1705
en.cppreference.com|1183
archive.org|839
www.boost.org|818
developer.android.com|679
translate.google.com|596
```
