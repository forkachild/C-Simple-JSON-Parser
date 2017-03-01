# C-Simple-JSON-Parser

## Extremely simple JSON Parser library written in C

### Features

 * Structured data (JSONObject, JSONPair, JSONValue)
 * Count of Key-Value pairs of current JSON
 * Recursive JSON parsing
 * JSONValue is a union whose type is stored as JSONValueType enum in its JSONPair

### Usage

#### Basic

```C
#include "json.h"

char * someJsonString = "{\"hello\":\"world\",\"key\":\"value\"}";

int main(int argc, const char * argv[]) {
  JSONObject *json = parseJSON(someJsonString);
  printf("Count: %i\n", json->count);
  printf("Key: %s, Value: %s\n", json->pairs[0].key, json->pairs[0].value->stringValue);
  return 0;
}
```
Will output

```
Count: 2
Key: hello
Value: world
```

#### Recursive

```C
#include "json.h"

char * complexJson = "{\"name":{\"first\":\"John\",\"last\":\"Doe\"},\"age\":\"21\"}";

int main(int argc, const char * argv[]) {
  JSONObject *json = parseJSON(complexJsonString);
  JSONObject *nameJson = json->pairs[0].value->jsonObject;
  printf("First name: %s\nLast name: %s\n", nameJson->pairs[0]->value.stringValue, nameJson->pairs[1]->value.stringValue);
  return 0;
}
```

### FAQs

#### How to know whether the current value is a String or JSON?

At each Key-Value `JSONPair` pair, there is a member named `type`

```C
#include "json.h"

...

JSONObject *json = parseJSON(someJsonString);
if(json->pairs[0].type == JSON_STRING) {
  // String value
} else {
  // JSON object
}
```

#### How to get the count of number of Key-Value pairs

At each `JSONObject` point, there is a member named `count`

```C
#include "json.h"

...

int i;
JSONObject *json = parseJSON(someJsonString);
for(i = 0; i < json->count; i++) {
  // Do something
}
```

#### What if JSON is poorly formatted with uneven whitespace

Well, that is not a problem for this library

#### What if there is error in JSON

That is when the function returns NULL

#### What is `_parseJSON` and how is it different from `parseJSON`

`_parseJSON` is the internal `static` implementation not to be used outside the library

### If this helped you in any way you can buy me a beer
