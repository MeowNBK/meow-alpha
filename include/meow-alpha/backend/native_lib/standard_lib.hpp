#pragma once

#include "native_lib/native_lib.hpp"
#include <iostream>

class CoreLib: public NativeLibrary {
public:
    CoreLib();
};

class StringLib: public NativeLibrary {
public:
    StringLib();
};

class ObjectLib: public NativeLibrary {
public:
    ObjectLib();
};

class ArrayLib: public NativeLibrary {
public:
    ArrayLib();
};

class IoLib: public NativeLibrary {
public:
    IoLib();
};

class SystemLib: public NativeLibrary {
public:
    SystemLib();
};

class JsonLib: public NativeLibrary {
public:
    JsonLib();
};