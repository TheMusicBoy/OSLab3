#include <common/threadpool.h>
#include <common/intrusive_ptr.h>
#include <common/logging.h>
#include <common/exception.h>
#include <common/getopts.h>

#include <iostream>
#include <service.h>

int main(int argc, const char* argv[]) {
    using namespace NCommon;

    GetOpts opts;
    opts.AddOption('h', "help", "Show help message");
    opts.AddOption('v', "version", "Show version information");
    opts.AddOption(0, "role-a", "Run as RoleA");
    opts.AddOption(0, "role-b", "Run as RoleB");

    try {
        opts.Parse(argc, argv);

        if (opts.Has('h') || opts.Has("help")) {
            std::cerr << opts.Help();
            return 0;
        }

        if (opts.Has('v') || opts.Has("version")) {
            std::cerr << "Lab3 Service v2.0\n";
            return 0;
        }

        auto service = New<TService>(argv[0]);

        // Check for role execution
        if (opts.Has("role-a")) {
            service->RoleA();
            return 0;
        }
        if (opts.Has("role-b")) {
            service->RoleB();
            return 0;
        }

        service->Start();
        LOG_INFO("Service started. Press Ctrl+C to exit.");

        while (true) {
            int newValue = 0;
            std::cin >> newValue;
            service->SetValue(newValue);
        }
    } catch (const TException& ex) {
        LOG_ERROR("Error: {}", ex.what());
        return 1;
    }
    
    return 0;
}
