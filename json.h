#pragma once

#include "json_types.h"

declare_result_type(json_value);
declare_result_type(json_entry);
declare_result_type(json_string);
declare_result_type(json_type);
declare_result_type(size);

typed(json_object) * json_parse(typed(json_string) json_str);
void json_print(typed(json_object) * obj, int indent);
void json_free(typed(json_object) * obj);
typed(json_string) json_error_to_string(const typed(json_error) err);