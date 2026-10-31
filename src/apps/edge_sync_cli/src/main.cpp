#include <iostream>
#include <string>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: edge-sync <command> [args]\n";
        return 1;
    }
    std::cout << "edge-sync CLI: command '" << argv[1] << "' (not yet implemented)\n";
    return 0;
}
