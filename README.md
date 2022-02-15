# variant

Интерфейс и все свойства и гарантии должны соответствовать [std::variant](https://en.cppreference.com/w/cpp/utility/variant), реализуя поведение из [P0608R3](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0608r3.html).

В отличие от ['optional'](https://github.com/iamerove/optional), реализация использует концепты из С++20 для сосхранения тривиальностей special member functions
