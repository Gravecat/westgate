#include "cmake/version.hpp"
#include "core/terminal.hpp"

int main() {
    lom::terminal::set_window_title("Lom v" + lom::version::VERSION_STRING + " (" + lom::version::BUILD_TIMESTAMP + ")");
    cout << fg::cyan << style::bold << "Hello, world!" << style::reset << endl;
    return 0;
}
