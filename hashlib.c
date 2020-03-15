#include <stdio.h>

#include <sqlite3ext.h>
SQLITE_EXTENSION_INIT1

#include <openssl/md5.h>
#include <openssl/sha.h>


#define make_hash_func(FUNC_NAME, DIGEST_FN, DIGEST_LENGTH) \
static void FUNC_NAME(sqlite3_context* context, int argc, sqlite3_value** argv) \
{ \
    const void* arg = sqlite3_value_blob(argv[0]); \
    const int len = sqlite3_value_bytes(argv[0]); \
    unsigned char digest[DIGEST_LENGTH]; \
    DIGEST_FN(arg, len, digest); \
\
    char* res = sqlite3_malloc(2 * DIGEST_LENGTH + 1); \
    for(int i = 0; i < DIGEST_LENGTH; ++i) \
        sprintf(res + 2 * i, "%02x", digest[i]); \
    sqlite3_result_text(context, res, -1, sqlite3_free); \
}

make_hash_func(md5_func, MD5, MD5_DIGEST_LENGTH);
make_hash_func(sha1_func, SHA1, SHA_DIGEST_LENGTH);
make_hash_func(sha224_func, SHA224, SHA224_DIGEST_LENGTH);
make_hash_func(sha256_func, SHA256, SHA256_DIGEST_LENGTH);
make_hash_func(sha384_func, SHA384, SHA384_DIGEST_LENGTH);
make_hash_func(sha512_func, SHA512, SHA512_DIGEST_LENGTH);


int sqlite3_hashlib_init(sqlite3* db, char** pzErrMsg, const sqlite3_api_routines* pApi)
{
    typedef struct {
        const char* name;
        void (*ptr)(sqlite3_context*, int, sqlite3_value**);
    } Function;
    const Function functions[] = {{"md5", md5_func}, {"sha1", sha1_func},
        {"sha224", sha224_func}, {"sha256", sha256_func}, {"sha384", sha384_func}, {"sha512", sha512_func}};

    int rc = SQLITE_OK;
    SQLITE_EXTENSION_INIT2(pApi);
    for(const Function* fn = functions; fn != functions + sizeof(functions) / sizeof(functions[0]); ++fn)
    {
        rc = sqlite3_create_function(db, fn->name, 1, SQLITE_UTF8 | SQLITE_DETERMINISTIC, 0, fn->ptr, 0, 0);
        if(rc != SQLITE_OK)
            break;
    }
    return rc;
}
