#ifndef JSON_H
#define JSON_H

#include <string.h>
#include <stdlib.h>

#ifndef __cplusplus
typedef char*                                   string;
typedef unsigned char                           bool;
#define true                                    (1)
#define false                                   (0)
#define TRUE                                    true
#define FALSE                                   false
#endif

#define new(x)                                  (x *) malloc(sizeof(x))
#define newWithSize(x, y)                       (x *) malloc(y * sizeof(x))
#define renewWithSize(x, y, z)                  (y *) realloc(x, z * sizeof(y))
#define isWhitespace(x)                         x == '\r' || x == '\n' || x == '\t' || x == ' '
#define isNumeral(x)                            (x >= '0' && x <= '9') || x == 'e' || x == 'E' \
                                                || x == '.'  || x == '+' || x == '-'
#define removeWhitespace(x)                     while(isWhitespace(*x)) x++
#define removeWhitespaceCalcOffset(x, y)        while(isWhitespace(*x)) { x++; y++; }

typedef char                                    character;

struct _jsonobject;
struct _jsonpair;
union _jsonvalue;

typedef enum {
    JSON_STRING = 0,
    JSON_DOUBLE,
    JSON_OBJECT
} JSONValueType;

typedef struct _jsonobject {
    struct _jsonpair *pairs;
    int count;
} JSONObject;

typedef struct _jsonpair {
    string key;
    union _jsonvalue *value;
    JSONValueType type;
} JSONPair;

typedef union _jsonvalue {
    string stringValue;
    double doubleValue;
    struct _jsonobject *jsonObject;
} JSONValue;

JSONObject *parseJSON(string);
void freeJSONFromMemory(JSONObject *);
static int strNextOccurence(string, char);
static int strNextNonNumeral(string);
static JSONObject * _parseJSON(string, int *);



#endif
