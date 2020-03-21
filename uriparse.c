#include <uriparser/Uri.h>

#include <sqlite3ext.h>
SQLITE_EXTENSION_INIT1


#ifndef SQLITE_OMIT_VIRTUALTABLE

enum Columns {
    COLUMN_SCHEME = 0,
    COLUMN_USERINFO = 1,
    COLUMN_HOST = 2,
    COLUMN_PORT = 3,
    COLUMN_PATH = 4,
    COLUMN_QUERY = 5,
    COLUMN_FRAGMENT = 6,
    COLUMN_URI = 7
};

typedef struct uriparse_cursor uriparse_cursor;
struct uriparse_cursor
{
    sqlite3_vtab_cursor base;

    char* text;
    UriUriA uri;
};


static int uriparse_connect(sqlite3* db, void* pAux,
    int argc, const char* const* argv,
    sqlite3_vtab** ppVTab,
    char** pzErr)
{
    int rc = sqlite3_declare_vtab(db,
        "create table x (scheme, userinfo, host, port, path, query, fragment, uri hidden)");
    if(rc == SQLITE_OK)
    {
        sqlite3_vtab* pNew = *ppVTab = sqlite3_malloc(sizeof(*pNew));
        if(!pNew)
            return SQLITE_NOMEM;
        memset(pNew, 0, sizeof(*pNew));
        sqlite3_vtab_config(db, SQLITE_VTAB_INNOCUOUS);
    }
    return rc;
}

static int uriparse_best_index(sqlite3_vtab* tab, sqlite3_index_info* pIdxInfo)
{
    for(int i = 0; i < pIdxInfo->nConstraint; ++i)
    {
        const struct sqlite3_index_constraint* c = pIdxInfo->aConstraint + i;
        if(!c->usable)
            return SQLITE_CONSTRAINT;
        if(c->iColumn == COLUMN_URI && c->op == SQLITE_INDEX_CONSTRAINT_EQ)
        {
            pIdxInfo->aConstraintUsage[i].argvIndex = i + 1;
            pIdxInfo->aConstraintUsage[i].omit = 1;
        }
    }
#if SQLITE_VERSION_NUMBER >= 3008002
    pIdxInfo->estimatedRows = 1;
#endif
    return SQLITE_OK;
}

static int uriparse_disconnect(sqlite3_vtab* pVtab)
{
    sqlite3_free(pVtab);
    return SQLITE_OK;
}

static int uriparse_open(sqlite3_vtab* p, sqlite3_vtab_cursor** ppCursor)
{
    uriparse_cursor* pCur = sqlite3_malloc(sizeof(*pCur));
    if(!pCur)
        return SQLITE_NOMEM;
    memset(pCur, 0, sizeof(*pCur));
    *ppCursor = &pCur->base;
    pCur->text = NULL;
    return SQLITE_OK;
}

static int uriparse_close(sqlite3_vtab_cursor* cur)
{
    sqlite3_free(cur);
    return SQLITE_OK;
}

static int uriparse_filter(sqlite3_vtab_cursor* pVtabCursor,
    int idxNum, const char* idxStr, int argc, sqlite3_value** argv)
{
    if(argc != 1)
        return SQLITE_ERROR;

    uriparse_cursor* cursor = (uriparse_cursor*)pVtabCursor;
    const char* text = sqlite3_value_text(argv[0]);
    if(uriParseSingleUriA(&cursor->uri, text, NULL) == URI_SUCCESS)
    {
        cursor->text = sqlite3_malloc(sqlite3_value_bytes(argv[0]) + 1);
        strcpy(cursor->text, text);
    }

    return SQLITE_OK;
}

static int uriparse_next(sqlite3_vtab_cursor* pCursor)
{
    uriparse_cursor* cursor = (uriparse_cursor*)pCursor;
    if(cursor->text)
    {
        sqlite3_free(cursor->text);
        cursor->text = NULL;
        uriFreeUriMembersA(&cursor->uri);
    }
    return SQLITE_OK;
}

static int uriparse_eof(sqlite3_vtab_cursor* pCursor)
{
    uriparse_cursor* cursor = (uriparse_cursor*)pCursor;
    return cursor->text == NULL;
}

static int uriparse_column(sqlite3_vtab_cursor* pCursor, sqlite3_context* ctx, int column)
{
    uriparse_cursor* cursor = (uriparse_cursor*)pCursor;
    UriTextRangeA range = {NULL, NULL};
    switch(column)
    {
        case COLUMN_SCHEME:
            range = cursor->uri.scheme;
            break;
        case COLUMN_USERINFO:
            range = cursor->uri.userInfo;
            break;
        case COLUMN_HOST:
            range = cursor->uri.hostText;
            break;
        case COLUMN_PORT:
            range = cursor->uri.portText;
            break;
        case COLUMN_PATH:
            if(!cursor->uri.pathHead)
                break;
            range.first = range.afterLast = cursor->uri.pathHead->text.first;
            for(const UriPathSegmentA* s = cursor->uri.pathHead; s != NULL; s = s->next)
                if(s->text.first != s->text.afterLast)
                    range.afterLast = s->text.afterLast;
            if(range.first == range.afterLast)
                range.first = range.afterLast = NULL;
            break;
        case COLUMN_QUERY:
            range = cursor->uri.query;
            break;
        case COLUMN_FRAGMENT:
            range = cursor->uri.fragment;
            break;
    }
    if(range.first)
        sqlite3_result_text(ctx, range.first, range.afterLast - range.first, NULL);
    else
        sqlite3_result_null(ctx);
    return SQLITE_OK;
}

static int uriparse_rowid(sqlite3_vtab_cursor* pCursor, sqlite_int64* pRowid)
{
    *pRowid = 1;
    return SQLITE_OK;
}


static sqlite3_module uriparse_module = {
    0,                    // iVersion
    0,                    // xCreate
    uriparse_connect,     // xConnect
    uriparse_best_index,  // xBestIndex
    uriparse_disconnect,  // xDisconnect
    0,                    // xDestroy
    uriparse_open,        // xOpen - open a cursor
    uriparse_close,       // xClose - close a cursor
    uriparse_filter,      // xFilter - configure scan constraints
    uriparse_next,        // xNext - advance a cursor
    uriparse_eof,         // xEof - check for end of scan
    uriparse_column,      // xColumn - read data
    uriparse_rowid,       // xRowid - read data
    0,                    // xUpdate
    0,                    // xBegin
    0,                    // xSync
    0,                    // xCommit
    0,                    // xRollback
    0,                    // xFindMethod
    0,                    // xRename
};

#endif


int sqlite3_uriparse_init(sqlite3* db, char** pzErrMsg, const sqlite3_api_routines* pApi)
{
    int rc = SQLITE_OK;
    SQLITE_EXTENSION_INIT2(pApi);
#ifndef SQLITE_OMIT_VIRTUALTABLE
    rc = sqlite3_create_module(db, "uriparse", &uriparse_module, NULL);
#endif
    return rc;
}
