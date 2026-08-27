#include <tclap/CmdLine.h>

#include <iostream>

int main() {
  {
    TCLAP::CmdLine cmd(TCLAP::CmdLineSpec{.message = "First example"});
    TCLAP::SwitchArg first_arg(TCLAP::SwitchArgSpec{
        .flag = "f",
        .name = "first",
        .description = "first switch argument"
    });
    cmd.add(first_arg);
    TCLAP::SwitchArg second_arg(TCLAP::SwitchArgSpec{
        .flag = "s",
        .name = "second",
        .description = "second switch argument"
    });
    cmd.add(second_arg);

    int first_argc = 3;
    const char** first_argv = new const char* [first_argc];
    first_argv[0] = "/nowhere";
    first_argv[1] = "--first";
    first_argv[2] = "--";
    cmd.parse(first_argc,
	      first_argv);
    delete[] first_argv;

    std::cout << "First example: first=" << first_arg.value()
	      << ", second=" << second_arg.value() << std::endl;
  }

  {
    TCLAP::CmdLine cmd(TCLAP::CmdLineSpec{.message = "Second example"});
    TCLAP::SwitchArg first_arg(TCLAP::SwitchArgSpec{
        .flag = "f",
        .name = "first",
        .description = "first switch argument"
    });
    cmd.add(first_arg);
    TCLAP::SwitchArg second_arg(TCLAP::SwitchArgSpec{
        .flag = "s",
        .name = "second",
        .description = "second switch argument"
    });
    cmd.add(second_arg);

    int second_argc = 3;
    const char** second_argv = new const char* [second_argc];
    second_argv[0] = "/nowhere";
    second_argv[1] = "--second";
    second_argv[2] = "--";
    cmd.parse(second_argc,
	      second_argv);
    delete[] second_argv;

    std::cout << "Second example: first=" << first_arg.value()
	      << ", second=" << second_arg.value() << std::endl;
  }
  
  return 0;
}
