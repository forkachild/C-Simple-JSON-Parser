#include "json.h"

JSONObject *parseJSON(string jsonString) {
    int offset = 0;
    JSONObject *tempObj = _parseJSON(jsonString, &offset);
    return tempObj;
}

void freeJSONFromMemory(JSONObject *obj) {
    int i;
    
    if(obj == NULL)
        return;
    
    if(obj->pairs == NULL) {
        free(obj);
        return;
    }
    
    for(i = 0; i < obj->count; i++) {
        if(obj->pairs[i].key != NULL)
            free(obj->pairs[i].key);
        if(obj->pairs[i].value != NULL) {
            switch(obj->pairs[i].type) {
                case JSON_STRING:
                    free(obj->pairs[i].value->stringValue);
                    break;
                case JSON_OBJECT:
                    freeJSONFromMemory(obj->pairs[i].value->jsonObject);
            }
            free(obj->pairs[i].value);
        }
    }
    
}

static int strNextOccurence(string str, char ch) {
    int pos = 0;
    
    if(str == NULL)
        return -1;
    
    while(*str != ch && *str != '\0') {
        str++;
        pos++;
    }
    return (*str == '\0') ? -1 : pos;
}

static JSONObject * _parseJSON(string str, int * offset) {
    
    int _offset = 0;
    
    JSONObject *obj = new(JSONObject);
    obj->count = 1;
    obj->pairs = newWithSize(JSONPair, 1);
    
    while(*str != '\0') {
        removeWhitespaceCalcOffset(str, _offset);
        if(*str == '{') {
            str++;
            _offset++;
        } else if(*str == '"') {
            
            int i = strNextOccurence(++str, '"');
            if(i <= 0) {
                freeJSONFromMemory(obj);
                return NULL;
            }
            
            JSONPair tempPtr = obj->pairs[obj->count - 1];
            
            tempPtr.key = newWithSize(character , i + 1);
            memcpy(tempPtr.key, str, i * sizeof(character));
            tempPtr.key[i] = '\0';
            
            str += i + 1;
            _offset += i + 2;
            
            i = strNextOccurence(str, ':');
            if(i == -1)
                return NULL;
            str += i + 1;
            _offset += i + 1;
            
            removeWhitespaceCalcOffset(str, _offset);
            
            if(*str == '{') {
                int _offsetBeforeParsingChildObject = _offset;
                int _sizeOfChildObject;

                tempPtr.value = new(JSONValue);
                tempPtr.type = JSON_OBJECT;
                tempPtr.value->jsonObject = _parseJSON(str, &_offset);
                if(tempPtr.value->jsonObject == NULL) {
                    freeJSONFromMemory(obj);
                    return NULL;
                }
                // Advance the string pointer by the size of the processed child object
                _sizeOfChildObject = _offset - _offsetBeforeParsingChildObject;
                str += _sizeOfChildObject;
            } else if(*str == '"') {
                i = strNextOccurence(++str, '"');
                if(i == -1) {
                    freeJSONFromMemory(obj);
                    return NULL;
                }
                tempPtr.value = new(JSONValue);
                tempPtr.type = JSON_STRING;
                tempPtr.value->stringValue = newWithSize(character, i + 1);
                memcpy(tempPtr.value->stringValue, str, i * sizeof(character));
                tempPtr.value->stringValue[i] = '\0';
                str += i + 1;
                _offset += i + 2;
            }
            obj->pairs[obj->count - 1] = tempPtr;
            
        } else if (*str == ',') {
            obj->count++;
            obj->pairs = renewWithSize(obj->pairs, JSONPair, obj->count);
            str++;
            _offset++;
        } else if (*str == '}') {
            (*offset) += _offset + 1;
            return obj;
        }
    }
    return obj;
}
