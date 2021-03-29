#include <tclap/CmdLine.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>
#include <iostream>

class TmpFile {
public:
  TmpFile() : name_(std::tmpnam(NULL)), stream_() {}
  ~TmpFile() {
    if (stream_.is_open()) {
      stream_.close();
      std::remove(name_.c_str());
    }
  }
  const std::string &name() const { return name_; }
  std::ifstream &stream() {
    stream_ = std::ifstream(name_, std::ifstream::binary | std::ifstream::ate);
    return stream_;
  }

private:
  std::string name_;
  std::ifstream stream_;
};

/**
 * \brief Runs a command with given parameters and compares its output to
 * stdout with the contents of a given reference file.
 * \details A temporary file will be used to catch the stdout/stderr output of
 * the command.
 * \return The program returns code 0 if both files compare equal, or a nonzero
 * value indicating the type of error.
 */
int main(int argc, char *argv[]) {
  int result = -1;  // Unknown failure
  try {
    TCLAP::CmdLine argParser("Runs a command with given parameters and"
                             " compares its output to stdout with the "
                             " contents of a given reference file.",
                             ' ', "1.0");
    TCLAP::UnlabeledValueArg<std::string> refFile("ref", "Reference file", true,
                                                  "", "string", argParser);
    TCLAP::UnlabeledValueArg<std::string> baseCmd("cmd", "Command to run", true,
                                                  "", "string", argParser);
    TCLAP::UnlabeledValueArg<std::string> params("params", "Parameters", false,
                                                 "", "string", argParser);
    argParser.parse(argc, argv);

    /* Execute command and capture stdout/stderr. */
    std::string cmd = baseCmd.getValue();
    if (params.isSet()) {
      cmd += " " + params.getValue();
    }

    TmpFile tmpFile;
    cmd += " > " + tmpFile.name() + " 2>&1";
    int unused = std::system(cmd.c_str());

    /* Compare against contents of reference file. */
    std::ifstream &f_tmp = tmpFile.stream();
    std::ifstream f_ref(refFile.getValue(), std::ifstream::binary | std::ifstream::ate);
    if (f_tmp.fail() || f_ref.fail()) {
      return 2; // One of the files fail.
    }
    if (f_tmp.tellg() != f_ref.tellg()) {
      return 3; // Files have different sizes.
    }
    f_tmp.seekg(0, std::ifstream::beg);
    f_ref.seekg(0, std::ifstream::beg);
    bool bEqual = std::equal(std::istreambuf_iterator<char>(f_tmp.rdbuf()),
                             std::istreambuf_iterator<char>(),
                             std::istreambuf_iterator<char>(f_ref.rdbuf()));
    result = bEqual ? 0 : 4; // 4 = Files to not compare equal.
  } catch (TCLAP::ArgException &e) {
    std::cerr << e.what() << std::endl;
    result = -2;
  }

  return result;
}
