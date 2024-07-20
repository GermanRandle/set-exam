# Set Assignment

In this assignment, you are required to write a class implementing a (possibly unbalanced) binary search tree.

A list of functions that need to be implemented is provided in the `set.h` file. The semantic behavior of these functions should match that of `std::set` from the standard library.

The `set.h` file specifies the required computational complexity for each function, except for `set::begin`, `set::end`, `set::iterator::operator++` and `set::iterator::operator--`. For these functions, it is required that each function individually works no more than `O(h)`, and additionally, the total code `for(var i = s.begin(); i != s.end(); ++i);` should work in `O(n)`.

Also, the `set` should not require a default constructor from its elements. Furthermore, an empty `set` should not hold any data on the heap at any time, and consequently, the default constructor should not perform any dynamic allocations.

Hint: Iterators can conveniently be implemented using the idea of a sentinel node.

