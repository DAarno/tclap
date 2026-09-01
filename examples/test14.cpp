// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

#include "tclap/CmdLine.h"
#include <algorithm>
#include <iterator>

#include <sstream>

// Define a simple 3D vector type
template <typename T, size_t LEN>
struct Vect : public TCLAP::StringLikeTrait {
    // typedef TCLAP::StringLike ValueCategory;
    T v[LEN];

    // operator= will be used to assign to the vector
    Vect &operator=(const std::string &str) {
        std::istringstream iss(str);
        for (size_t n = 0; n < LEN; n++) {
            if (!(iss >> v[n])) {
                std::ostringstream oss;
                oss << " is not a vector of size " << LEN;
                throw TCLAP::ArgParseException(str + oss.str());
            }
        }

        if (!iss.eof()) {
            std::ostringstream oss;
            oss << " is not a vector of size " << LEN;
            throw TCLAP::ArgParseException(str + oss.str());
        }

        return *this;
    }

    std::ostream &print(std::ostream &os) const {
        std::copy(v, v + LEN, std::ostream_iterator<T>(os, ", "));
        return os;
    }
};

int main(int argc, char *argv[]) {
    TCLAP::CmdLine cmd({.message = "Command description message",
                        .dialect = {.delimiter = ' '},
                        .version = "0.9"});
    TCLAP::ValueArg<Vect<double, 3> > vec({.flag = "v",
                                           .name = "vect",
                                           .description = "vector",
                                           .required = true,
                                           .defaultValue = Vect<double, 3>(),
                                           .typeDesc = "3D vector"});
    cmd.add(vec);

    try {
        cmd.parseOrExit(argc, argv);
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    vec.value().print(std::cout);
    std::cout << std::endl;
}
