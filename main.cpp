#include "transport_catalogue.h"
#include "json_reader.h"

#include <iostream>
#include <fstream>
#include <string_view>

using namespace std;
using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    catalogue::TransportCatalogue transport_catalogue;
    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        renderer::LoadJSON(transport_catalogue, cin);
    }
    else if (mode == "process_requests"sv) {
        renderer::ProcessRequests(transport_catalogue, cin);
    }
    else {
        PrintUsage();
        return 1;
    }
}