#include <iostream>
#include "src/liblcpex.h"

/* Demonstration of the use of the liblcpex library.
 *
 * lcpex executes a command, captures the stdout and stderr to log,
 * while displaying STDOUT/STDERR of the child to the parent in realtime, allowing the
 * use of interactive programs by passing the parent STDIN to the child STDIN.
 *
 * lcpex also allows the user to specify a context override, which
 * will set the context of the child process to the specified user and group.
 *
 * lcpex also allows the user to specify a custom environment for
 * the child process.
 *
 * lcpex also allows the user to specify the shell to use for the
 * child process.
 *
 * lcpex also allows the user to force the child process to use a
 * pty for programs that require it (such as ncurses-based programs).
 *
*/

int main( int argc, char *argv[] )
{
    // test with interactive dialogs

    // (whiptail) - requires whiptail, does not require a PTY
    std::string test1 = R"(whiptail --title "Example Dialog" --yesno "This is an example of a yes/no box." 8 78)";

    // (ncurses)
    // requires dialog, pty
    std::string test2 = R"(/usr/bin/dialog --title "This should be one argument" --inputbox "Enter your name:" 0 0)";

    // test with a command that echos back to stdout/stderr whatever is typed
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
    std::string test3 = R"(/usr/bin/parrot)";

    // test context
    std::string test4 = R"(whoami && id -g)";

    // test of return code
    std::string test5 = R"(false)";

    int exit_code = lcpex(
            test2,
            "/tmp/lcpex_test2_stdout.log",
            "/tmp/lcpex_test2_stderr.log",
            false,
            "bagira",
            "bagiras",
            true,
            true,
            "/bin/bash",
            "-c",
            false,
            "source",
            "/tmp/env.bash"
    );

    return exit_code;
}
