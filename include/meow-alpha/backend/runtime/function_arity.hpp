#pragma once

struct Arity {
    int required = 0;
    int optional = 0;
    bool isVariadic = false;

    static Arity fixed(int count) { 
        return {count, 0, false}; 
    }
    static Arity range(int req, int opt) { 
        return {req, opt, false}; 
    }
    static Arity atLeast(int min) { 
        return {min, 0, true}; 
    }
};