## JsonContainer

The JsonContainer class provides a simplified abstraction around complex JSON C++
libraries. It has the following constructors:

    JsonContainer() // Creates an empty container
    JsonContainer(std::string json_txt) // creates a JsonContainer from a JSON string

Consider the following JSON string wrapped in a JsonContainer object, data.

```
    {
      "module" : "puppet",
      "action" : "run",
      "params" : {
        "first" : "--module-path=/home/alice/modules"
      }
    }
```

You can construct a JsonContainer as follows:

```
    JsonContainer data { jsons_string };
```

The JsonContainer's constructor can throw the following exception:

 - data_parse_error - This error is thrown when invalid JSON is passed to the constructor.

The following calls to the _get_ method will retrieve values from the JsonContainer.

```
    data.get<std::string>("module"); // == "puppet"
    data.get<std::string>({ "params", "first" }); // == "--module-path=/home/alice/modules"
```

Note that when the _get_ method is invoked with an initialiser list it will use
each argument to descend a level into the object tree.

The supported scalar types are: int, double, bool, std::string, and JsonContainer.
Elements of such types can be grouped in an array, represented by a std::vector
instance.

In case _get_ is invoked with an unknown key, no exception is thrown; depending
on the requested value type, the method returns:

 - 0 (int)
 - 0.0 (double)
 - false (bool)
 - an empty std::string
 - an empty JsonContainer
 - an empty std::vector of the requested type

The _get_ method throws an assertion error in case the specified type does not
match the one of the requested value. You can verify if the type is correct by
using the _type_ method (see below). Note that an assertion error is thrown in
case the specified template type doesn't match the one of the requested entry.

You can also set the value of fields and create new fields with the _set_ method.
```
    data.set<int>("foo", 42);
    data.set<bool>({ "params", "second" }, false);
```

This will change the internal JSON representation to

```
    {
      "module" : "puppet",
      "action" : "run",
      "params" : {
        "first" : "--module-path=/home/alice/modules",
        "second" : false
      },
      "foo" : 42
    }
```

Note that the _set_ method uses the initialiser list in the same way as the _get_
method. Each argument to the list is one level to descend.

The _set_ method can throw the following exception:

 - data_key_error - thrown when a nested message key is invalid (i.e. the
 associated value is not a valid JSON object, so that is not possible to
 iterate the remaining nested keys) or when the root element is not a valid
 JSON object, so that is not possible to set the specified key-value entry.

You can use the _type_ method to retrieve the type of a given value. As done for
_get_ and _set_, you can specify the value's key with an initialiser list, in
order to navigate multiple levels within a JSON object.

The _type_ method returns a value of the DataType enumeration, defined as:

```
    enum DataType { Object, Array, String, Int, Bool, Double, Null };
```

The _type_ method can throw the following exception:

 - data_key_error - thrown when the specified key is unknown.
