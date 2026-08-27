#include "tclap/CmdLine.h"
#include <iterator>

using namespace TCLAP;

// Define a simple 3D vector type
struct Vect3D {
    double v[3];

    std::ostream &print(std::ostream &os) const {
        std::copy(v, v + 3, std::ostream_iterator<double>(os, " "));
        return os;
    }
};

// operator>> will be used to assign to the vector since the default
// is that all types are ValueLike.
std::istream &operator>>(std::istream &is, Vect3D &v) {
    if (!(is >> v.v[0] >> v.v[1] >> v.v[2]))
        throw TCLAP::ArgParseException(" Argument is not a 3D vector");

    return is;
}

int main(int argc, char *argv[]) {
    CmdLine cmd(CmdLineSpec{
        .message = "Command description message",
        .dialect = {.delimiter = ' '},
        .version = "0.9"
    });
    ValueArg<Vect3D> vec(ValueArgSpec<Vect3D>{
        .flag = "v",
        .name = "vect",
        .description = "vector",
        .required = true,
        .defaultValue = Vect3D(),
        .typeDesc = "3D vector"
    });
    cmd.add(vec);

    try {
        cmd.parse(argc, argv);
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    vec.value().print(std::cout);
    std::cout << std::endl;
}
