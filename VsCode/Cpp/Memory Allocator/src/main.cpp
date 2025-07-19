#include "MemoryPool.hpp"
#include "ObjectPool.hpp"

#include <iostream>
#include <cassert>

struct Foo {
    int x;
    Foo(int _x) : x(_x) {
        std::cout << "Foo(" << x << ") constructed\n";
    }
    ~Foo() {
        std::cout << "Foo(" << x << ") destroyed\n";
    }
    void greet() const {
        std::cout << "Hello from Foo(" << x << ")\n";
    }
};

int main() {
    ObjectPool<Foo, 2> pool;

    // 1) Create two objects
    Foo* a = pool.create(10);
    Foo* b = pool.create(20);

    assert(a != nullptr && b != nullptr && a != b);

    a->greet();  // Hello from Foo(10)
    b->greet();  // Hello from Foo(20)

    // 2) Destroy them (LIFO order)
    pool.destroy(a);
    pool.destroy(b);

    // 3) Reuse slots: next create should return 'b' then 'a'
    Foo* c = pool.create(30);
    Foo* d = pool.create(40);

    assert(c == b);  // reused slot of b
    assert(d == a);  // then slot of a

    c->greet();  // Hello from Foo(30)
    d->greet();  // Hello from Foo(40)

    Foo* e= pool.create(5);
    e->greet();

    // Clean up
    pool.destroy(c);
    pool.destroy(d);
    pool.destroy(e);

    std::cout << "All tests passed!\n";
    return 0;
}