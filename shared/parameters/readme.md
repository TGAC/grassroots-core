# Parameters Guide {#parameters_library}

The Parameters library provides the datatypes and API for how Services can expose the variables that can be set by the user to run tasks. Each Parameter has various attributes such as its internal name, user-friendly name to display to the user, its datatype, *etc.* 

Each Service produces a ParameterSet which contains all of its Parameters. 

The possible types that a Parameter can take are defined by the ParameterType enumeration. These vary from primtive types such as boolean values, integers and floating point values through c-style strings to structures such as Resource.


## Paramater values

Both the default and current values that a Parameter can take are stored by SharedType objects. A SharedType is a union of various different datatypes and the actual value stored within the SharedType depends upon the Parameter's ParameterType.

| ParameterType | SharedType member | Description |
|---|---|---|
| **PT_BOOLEAN** | st_boolean_value |  A boolean parameter |
| **PT_SIGNED_INT** | st_long_value |  A 32-bit integer  |
| **PT_UNSIGNED_INT** | st_ulong_value | A non-negative 32-bit integer |
| **PT_NEGATIVE_INT** | st_long_value | A non-positive 32-bit integer |
| **PT_SIGNED_REAL** | st_data_value | A real number |
| **PT_UNSIGNED_REAL** | st_data_value | An unsigned real number |
| **PT_STRING** | st_string_value_s | A c-style string |
| **PT_FILE_TO_WRITE** | st_resource_value_p | An output filename string |
| **PT_FILE_TO_READ** | st_resource_value_p | An input filename string |
| **PT_DIRECTORY** | st_resource_value_p | A directory string |
| **PT_CHAR** | st_char_value | A single 1-byte character |
| **PT_PASSWORD** | st_string_value_s | A sensitive c-style string that shouldn't be displayed explicitly to the user. |
| **PT_KEYWORD** | st_string_value_s | A value that a Service can use without any other parameters being set to produce results. |
| **PT_LARGE_STRING** | st_string_value_s | A potentially large c-style string. This is used by clients to determine the appropriate editor to show to the user. *E.g.* a multi-line text box as opposed to a single-line text box for a **PT_STRING**. |
| **PT_JSON** | st_json_p | A JSON fragment. |
| **PT_TABLE** | st_string_value_s | A c-style string of tabular data. |
| **PT_FASTA** | st_string_value_s | A string representing a FASTA sequence. |


### Setting Parameter values

There are two API methods for setting a value for a Parameter: ```SetParameterValueFromSharedType()``` and ```SetParameterValue()```.

~~~.c
bool SetParameterValueFromSharedType (Parameter * const parameter_p, const SharedType * src_p, const bool current_value_flag);
bool SetParameterValue (Parameter * const parameter_p, const void *value_p, const bool current_value_flag)
~~~

Each of these take the Parameter whose value will be set as the first function parameter. They also both take the same bool value for the third function parameter which specifies whether the Parameter's current or default value is the one to be set. The difference between these two functions is what value they take for the second function parameter. ```SetParameterValueFromSharedType()``` takes a pointer to a SharedType instance whose value will be used to set the Parameter's value dependent upon its datatype. ```SetParameterValue()``` takes a generic pointer for its second function parameter that will be cast to a pointer to the Parameter's datatype.

For both of these function the Parameter will make a deep copy of the source value and, if successful, set its value accordingly. Any previously stored values will be freed if needed *e.g.* if they were c-style strings or JSON fragments.


## Parameter Groups

Often it makes sense to group certain parameters together when they are presented to a user *e.g.* input parameters, output parameters, *etc.* and this can be done easily in the Grassroots API by using a ParameterGroup. Each ParameterGroup stores a list of the Parameters it will display and, likewise, each Parameter stores a pointer to its ParameterGroup in its ```pa_group_p``` member variable. If this is ```NULL``` for a Parameter then it means that this Parameter is not part of a ParameterGroup. 

As well as being for layout purposes, ParameterGroups can also have the ability to add multiple collections of Parameters, 
