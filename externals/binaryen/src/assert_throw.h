#include <stdexcept>

#define ASSERT_THROW(cond) ({ if( !(cond) ) throw std::runtime_error{"binaryen assert: " #cond}; })

#define ASSERT_TERMINATE(cond) ({ if( !(cond) ) { fprintf(stderr, "binaryen assert in destructor: %s", #cond); std::terminate(); } })
