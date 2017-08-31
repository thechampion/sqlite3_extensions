#include <stdio.h>
#include <string.h>

#include <Uri.h>

#include <sqlite3ext.h>
SQLITE_EXTENSION_INIT1


static void uriparse_host_func(sqlite3_context* context, int argc, sqlite3_value** argv)
{
	UriParserStateA state;
	UriUriA uri;
	state.uri = &uri;
	const char* text = sqlite3_value_text(argv[0]);
	if(uriParseUriA(&state, text) == URI_SUCCESS)
	{
		const char* begin = uri.hostText.first, * end = uri.hostText.afterLast;
		const size_t len = end - begin;
		char* res = sqlite3_malloc(len + 1);
		memcpy(res, begin, len);
		res[len] = '\0';
		sqlite3_result_text(context, res, -1, sqlite3_free);
	}
	else
		sqlite3_result_null(context);
	uriFreeUriMembersA(&uri);
}


int sqlite3_uriparse_init(sqlite3* db, char** pzErrMsg, const sqlite3_api_routines* pApi)
{
	int rc = SQLITE_OK;
	SQLITE_EXTENSION_INIT2(pApi);

	rc = sqlite3_create_function(db, "uriparse_host", 1, SQLITE_UTF8 | SQLITE_DETERMINISTIC, 0, uriparse_host_func, 0, 0);
	return rc;
}
