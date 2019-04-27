#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
 *  Very simple example program
 */
#include "duktape.h"
#include "sqlite3.h"

static duk_ret_t native_print(duk_context *ctx) {
	duk_push_string(ctx, " ");
	duk_insert(ctx, 0);
	duk_join(ctx, duk_get_top(ctx) - 1);
	printf("%s", duk_safe_to_string(ctx, -1));
	return 0;
}

static duk_ret_t native_printLine(duk_context *ctx) {
	duk_push_string(ctx, " ");
	duk_insert(ctx, 0);
	duk_join(ctx, duk_get_top(ctx) - 1);
	printf("%s\n", duk_safe_to_string(ctx, -1));
	return 0;
}

char * getline(void);

static duk_ret_t native_exit(duk_context *ctx) {
    exit(0);
	return 0;
}

static duk_ret_t native_getline(duk_context *ctx) {
    char * buffer = getline();
    duk_push_string(ctx, buffer);
    free(buffer);
	return 1;
}

char * dataFile = "";

char * file_to_string(FILE * f);

static duk_ret_t native_createDF(duk_context *ctx) {
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    rc = sqlite3_open(dataFile, &db);
    if( rc ){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      return 0;
    }
    FILE * f = fopen ("createDF.sql", "rb");
    if (!f) {
        f = fopen("../../createDF.sql", "rb");
    }
    char * createFile = file_to_string(f);
    rc = sqlite3_exec(db, createFile, NULL, 0, &zErrMsg);
    if( rc!=SQLITE_OK ){
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
    }

    sqlite3_close(db);
	return 0;
}
// TODO: Sync files to DF. Start DF.
static duk_ret_t native_syncDF(duk_context *ctx) {
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    rc = sqlite3_open(dataFile, &db);
    if( rc ){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      return 0;
    }
    /*rc = sqlite3_exec(db, createFile, NULL, 0, &zErrMsg);
    if( rc!=SQLITE_OK ){
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
    }*/

    sqlite3_close(db);
	return 0;
}

char * getline(void) {
    char * line = malloc(100), * linep = line;
    size_t lenmax = 100, len = lenmax;
    int c;

    if(line == NULL)
        return NULL;

    for(;;) {
        c = fgetc(stdin);
        if(c == EOF)
            break;

        if(--len == 0) {
            len = lenmax;
            char * linen = realloc(linep, lenmax *= 2);

            if(linen == NULL) {
                free(linep);
                return NULL;
            }
            line = linen + (line - linep);
            linep = linen;
        }

        if((*line++ = c) == '\n')
            break;
    }
    *(line - 1) = '\0';
    return linep;
}

char * openBootJS() {
    FILE * f = fopen ("boot.js", "rb");
    if (!f) {
        f = fopen("../../boot.js", "rb");
    }
    return file_to_string(f);
}

char * file_to_string(FILE * f) {
    char * buffer = 0;
    long length;

    if (f)
    {
      fseek (f, 0, SEEK_END);
      length = ftell (f);
      fseek (f, 0, SEEK_SET);
      buffer = malloc (length + 1);
      if (buffer)
      {
        fread (buffer, 1, length, f);
      }
      buffer[length] = '\0';
      fclose (f);
    }
    return buffer;
}

int ba = 1;

int main(int argc, char *argv[]) {
	duk_context *ctx = duk_create_heap_default();

	(void) argc; (void) argv;  /* suppress warning */

	duk_push_c_function(ctx, native_print, DUK_VARARGS);
	duk_put_global_string(ctx, "print");
	duk_push_c_function(ctx, native_printLine, DUK_VARARGS);
	duk_put_global_string(ctx, "printLine");
	duk_push_c_function(ctx, native_exit, DUK_VARARGS);
	duk_put_global_string(ctx, "exit");
	duk_push_c_function(ctx, native_getline, DUK_VARARGS);
	duk_put_global_string(ctx, "getLine");
	duk_push_c_function(ctx, native_createDF, DUK_VARARGS);
	duk_put_global_string(ctx, "createDF");
	duk_push_c_function(ctx, native_syncDF, DUK_VARARGS);
	duk_put_global_string(ctx, "syncDF");

    if( access( "data.db", F_OK ) != -1 ) {
        dataFile = "data.db";
    } else {
        if( access( "../../data.db", F_OK ) != -1 ) {
            dataFile = "../../data.db";
        } else {
            fprintf(stderr, "Can't find data file\n");
            dataFile = "data.db";
        }
    }

    if (ba)  {
        char * boot = openBootJS();
        if (duk_peval_string(ctx, boot) != 0) {
            printf("Script error: %s\n", duk_safe_to_string(ctx, -1));
        }
        free(boot);
    }
    duk_pop(ctx);
	duk_destroy_heap(ctx);
	return 0;
}
