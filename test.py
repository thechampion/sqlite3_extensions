#! /usr/bin/env python3

import sqlite3

from itertools import product

import pytest


@pytest.fixture
def conn():
    conn = sqlite3.connect(":memory:")
    conn.enable_load_extension(True)
    return conn

def test_hashlib(conn):
    FUNCTIONS = ("md5", "sha1", "sha224", "sha256", "sha384", "sha512")
    ARGS = ("", "qwerty", "Asdf Zxcvb")
    RESULTS = (
        # md5
        "d41d8cd98f00b204e9800998ecf8427e",
        "d8578edf8458ce06fbc5bb76a58c5ca4",
        "95bae1cb51df84bea523fdfb3b1c76f0",
        # sha1
        "da39a3ee5e6b4b0d3255bfef95601890afd80709",
        "b1b3773a05c0ed0176787a4f1574ff0075f7521e",
        "8fe6cf6aa7c9e3ea22a2e6d23991b7020c55de89",
        # sha224
        "d14a028c2a3a2bc9476102bb288234c415a2b01f828ea62ac5b3e42f",
        "5154aaa49392fb275ce7e12a7d3e00901cf9cf3ab10491673f97322f",
        "8e8e9b559ab591f577402f0250d9a174de87a0d7e641d4f0e2b8c635",
        # sha256
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
        "65e84be33532fb784c48129675f9eff3a682b27168c0ea744b2cf58ee02337c5",
        "f7d36231fc11bc9aeb1b88274aa0163d4064c6e029f5029854bb7122146b3de6",
        # sha384
        "38b060a751ac96384cd9327eb1b1e36a21fdb71114be07434c0cc7bf63f6e1da274edebfe76f65fbd51ad2f14898b95b",
        "1ab60e110d41a9aac5e30d086c490819bfe3461b38c76b9602fe9686aa0aa3d28c63c96a1019e3788c40a14f4292e50f",
        "c8ce613bc8432ab9e174ccf3a5c94f57751f61d62dd878dd610777b005bfcaf67ff84a5e5880575cf4ddf80d61428de3",
        # sha512
        "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e",
        "0dd3e512642c97ca3f747f9a76e374fbda73f9292823c0313be9d78add7cdd8f72235af0c553dd26797e78e1854edee0ae002f8aba074b066dfce1af114e32f8",
        "972f734193853a7527620b2faffe83817e8b6418c1e18c50f242db83d99761efca949e726b92e63434d1a9776af6168d5f742ea6dd2c84e590030e2401ad48ab"
    )
    assert len(FUNCTIONS) * len(ARGS) == len(RESULTS), "invalid test data"

    conn.load_extension("./hashlib")
    cursor = conn.cursor()
    for (func, arg), res in zip(product(FUNCTIONS, ARGS), RESULTS):
        row = next(cursor.execute("select %s(?)" % func, (arg, )))
        assert row[0] == res

def test_uriparse(conn):
    TESTS = (
        ("https://github.com", ("https", None, "github.com", None, None, None, None)),
        ("https://github.com/", ("https", None, "github.com", None, None, None, None)),
        ("mailto:user@example.org", ("mailto", None, None, None, "user@example.org", None, None)),
        ("https://github.com/thechampion/sqlite3_extensions",
            ("https", None, "github.com", None, "thechampion/sqlite3_extensions", None, None)),
        ("rtsp://127.0.0.1:8554/watch?v=ASDFvcxz", ("rtsp", None, "127.0.0.1", "8554", "watch", "v=ASDFvcxz", None))
    )

    conn.load_extension("./uriparse")
    cursor = conn.cursor()
    for arg, res in TESTS:
        row = next(cursor.execute(
            "select scheme, userinfo, host, port, path, query, fragment from uriparse(?)", (arg, )))
        assert row == res

    cursor.executescript("""create table t (uri text);
        insert into t values ('https://aaa.org/one'), ('https://bbb.org/two'),
            ('https://bbb.org/three'), ('https://ccc.org/four'),
            ('https://ccc.org/five'), ('https://ccc.org/six'),
            ('http://ddd.org/seven'), ('http://ddd.org/eight'),
            ('http://ddd.org/nine'), ('http://ddd.org/ten');""")

    cursor.execute("select distinct scheme, host from t join uriparse using (uri) order by host")
    assert tuple(cursor) == (("https", "aaa.org"), ("https", "bbb.org"), ("https", "ccc.org"), ("http", "ddd.org"))
