# HashTableLib
## About
This library implements a template hash table, that can add, take and find elements by O(1). \
The choice of hash function, element comparison function, collusion protection, rehash was implemented.
## Install
You can include this lib to your project and add it to CMake project.
## Functions
 - HashTable<K, V, [Hash], [KeyEqual]> - declaration.
 - copy-, move- constructors
 - copy-, move- operators =
 - find
 - begin/end
 - empty, size
 - clear
 - insert
 - emplace
