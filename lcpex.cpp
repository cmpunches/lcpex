#include <iostream>
#include "src/liblcpex.h"

int main( int argc, char *argv[] )
{
    // execute an arbitrary command and capture its exit code
    // It tees the child's stdout/stderr to corresponding log files as well as to parent's stdout/stderr

    // test with interactive dialogs
    // (ncurses) (not working - no vpty)
    std::string cmd = R"(/usr/bin/dialog --title "This should be one argument" --inputbox "Enter your name:" 0 0 && env)";

    // test of exit code return
    //std::string cmd = R"(false)";

    // test with a command that echos back to stdout/stderr whatever is typed
    std::string cmd2 = R"(/usr/bin/parrot)";

    // (whiptail) (working)
    //std::string cmd3 = R"(whiptail --title "Example Dialog" --yesno "This is an example of a yes/no box." 8 78)";

    // test context
    std::string cmd3 = R"(/bin/whodat2)";

    // parrot is just a dummy repeater script in bash:
    /*
     * $ cat /usr/bin/parrot
        echo "Type things into the terminal and I'll echo them back."

        while read line
        do
            echo "STDOUT: Got from STDIN: $line"
            echo "STDERR: Got from STDIN: $line" >&2
        done < "${1:-/dev/stdin}"

     */

    /*
    int x = lcpex(
            cmd3,
            "stdout.log",
            "stderr.log",
            true,
            "bagira",
            "bagira",
            true,
            true,
            "/usr/bin/bash",
            false,
            "-c",
            "source",
            "/tmp/env.bash"
    );
     */

    std::string new_cmd = prefix_generator(
            cmd,
            true,
            "/bin/bash",
            "-c",
            true,
            ".",
            "/tmp/env.bash"
    );

    std::cout << new_cmd << std::endl;
    return 0;
}
