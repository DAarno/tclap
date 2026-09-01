// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

#include "tclap/CmdLine.h"
#include <string>

using namespace TCLAP;
using namespace std;

int main(int argc, char **argv) {
    // Define the command line object.
    CmdLine cmd(
        {.message =
             "Command description message. This is a long multi-line "
             "message "
             "meant to test line wrapping.  This is more text that doesn't "
             "really do anything besides take up lots of space that "
             "otherwise "
             "might be used for something real.  That should be enough, "
             "don't "
             "you think?",
         .dialect = {.delimiter = ' '},
         .version = "0.9"});

    vector<string> allowed;
    allowed.push_back("homer");
    allowed.push_back("marge");
    allowed.push_back("bart");
    allowed.push_back("lisa");
    allowed.push_back("maggie");
    const ValuesConstraint<string> vallowed(allowed);

    MultiArg<string> nameArg(
        {.flag = "n",
         .name = "name",
         .description = "Name to print. This is a long, nonsensical "
                        "message to test line wrapping.  Hopefully it "
                        "works.",
         .required = true,
         .constraint = &vallowed});
    cmd.add(nameArg);

    vector<int> iallowed;
    iallowed.push_back(1);
    iallowed.push_back(2);
    iallowed.push_back(3);
    const ValuesConstraint<int> iiallowed(iallowed);

    UnlabeledMultiArg<int> intArg({.name = "times",
                                   .description = "Number of times to print",
                                   .required = false,
                                   .constraint = &iiallowed});
    cmd.add(intArg);

    // Ignore the names and comments!  These  args mean nothing (to this
    // program) and are here solely to take up space.
    ValueArg<int> gapCreate({.flag = "f",
                             .name = "gapCreate",
                             .description = "The cost of creating a gap",
                             .required = false,
                             .defaultValue = -10,
                             .typeDesc = "negative int"});
    cmd.add(gapCreate);

    ValueArg<int> gapExtend(
        {.flag = "g",
         .name = "gap-Extend",
         .description = "The cost for each extension of a gap",
         .required = false,
         .defaultValue = -2,
         .typeDesc = "negative int"});
    cmd.add(gapExtend);

    SwitchArg dna({.flag = "d",
                   .name = "isDna",
                   .description = "The input sequences are DNA",
                   .defaultValue = false});
    cmd.add(dna);

    ValueArg<string> scoringMatrixName({.flag = "s",
                                        .name = "scoring--Matrix",
                                        .description = "Scoring Matrix name",
                                        .required = false,
                                        .defaultValue = "BLOSUM50",
                                        .typeDesc = "name string"});
    cmd.add(scoringMatrixName);

    ValueArg<string> seq1Filename(
        {.flag = "x",
         .name = "filename1",
         .description = "Sequence 1 filename (FASTA format)",
         .required = false,
         .defaultValue = "",
         .typeDesc = "filename"});
    cmd.add(seq1Filename);

    ValueArg<string> seq2Filename(
        {.flag = "z",
         .name = "filename2",
         .description = "Sequence 2 filename (FASTA format)",
         .required = false,
         .defaultValue = "",
         .typeDesc = "filename"});
    cmd.add(seq2Filename);

    ValueArg<float> lowerBound({.flag = "b",
                                .name = "lowerBound",
                                .description = "lower percentage bound",
                                .required = false,
                                .defaultValue = 1.0,
                                .typeDesc = "float lte 1"});
    cmd.add(lowerBound);

    ValueArg<float> upperBound({.flag = "u",
                                .name = "upperBound",
                                .description = "upper percentage bound",
                                .required = false,
                                .defaultValue = 1.0,
                                .typeDesc = "float lte 1"});
    cmd.add(upperBound);

    ValueArg<int> limit({.flag = "l",
                         .name = "limit",
                         .description = "Max number of alignments allowed",
                         .required = false,
                         .defaultValue = 1000,
                         .typeDesc = "int"});
    cmd.add(limit);

    argv[0] = const_cast<char *>(
        "ThisIsAVeryLongProgramNameDesignedToTestSpacePrintWhichUsedToHaveP"
        "roblemsWithLongProgramNamesIThinkItIsNowLongEnough");

    // Parse the args.
    cmd.parseOrExit(argc, argv);

    // Get the value parsed by each arg.
    vector<int> num = intArg.value();

    for (unsigned int i = 0; i < num.size(); i++)
        cout << "Got num " << num[i] << endl;

    vector<string> name = nameArg.value();

    for (unsigned int i = 0; i < name.size(); i++)
        cout << "Got name " << name[i] << endl;
}
