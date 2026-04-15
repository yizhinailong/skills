import std;
import a;

auto main() -> int {
    a::FuncA();
    // a::FuncA1(); // error: 'FuncA1' is internal partition a1 of module 'a'
    a::FuncA2();

    std::println("---");

    a::b::FuncAb();
    a::b::FuncB1();
    a::b::FuncB2();
    return 0;
}
