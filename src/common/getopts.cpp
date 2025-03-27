#include <common/getopts.h>

#include <cstring>
#include <sstream>

namespace NCommon {

////////////////////////////////////////////////////////////////////////////////

void GetOpts::AddOption(char shortName,
                       const std::string& longName,
                       const std::string& description,
                       bool requiresArgument) {
    ValidateOption(shortName, longName);
    
    Option opt;
    opt.shortName = shortName;
    opt.longName = longName;
    opt.description = description;
    opt.requiresArgument = requiresArgument;
    
    RegisterOption(opt);
}

void GetOpts::Parse(int argc, const char* const argv[]) {
    bool parseOptions = true;

    for (int i = 1; i < argc; ++i) {
        size_t arg_size = std::strlen(argv[i]);
        const char* arg = argv[i];

        if (parseOptions && arg_size == 2 && arg[0] == '-' && arg[1] == '-') {
            parseOptions = false;
            continue;
        }

        if (parseOptions && arg_size > 1 && arg[0] == '-') {
            if (arg[1] == '-') {
                ProcessLongOption(arg + 2);
            } else {
                ProcessShortOption(argc, argv, i);
            }
        } else {
            ProcessPositional(arg);
        }
    }

    // Validate required arguments
    for (const auto& opt : Options_) {
        if (opt.requiresArgument) {
            size_t idx = ShortIndex_.at(opt.shortName);
            ASSERT(!Values_[idx].present || !Values_[idx].value.empty(),
                "Option requires argument: {}/{}",
                opt.shortName,
                opt.longName);
        }
    }
}

void GetOpts::ProcessShortOption(int argc, const char* const argv[], int& i) {
    size_t arg_size = std::strlen(argv[i]);
    const char* arg = argv[i];

    for (size_t pos = 1; pos < arg_size; ++pos) {
        char c = arg[pos];
        ASSERT(ShortIndex_.count(c), "Unknown option: -{}", c);

        size_t idx = ShortIndex_[c];
        OptionValue& val = Values_[idx];
        val.present = true;

        if (Options_[idx].requiresArgument) {
            if (pos + 1 < arg_size) {
                val.value = arg + pos + 1;
                pos = arg_size - 1;
            } else {
                ASSERT(i + 1 < argc, "Missing argument for: -{}", c);
                val.value = argv[++i];
            }
        }
    }
}

void GetOpts::ProcessLongOption(const char* arg) {
    const char* eq_pos = strchr(arg, '=');
    std::string name;
    
    if (eq_pos) {
        name.assign(arg, eq_pos - arg);
    } else {
        name = arg; // Handle flag options like --help
    }

    ASSERT(LongIndex_.count(name), "Unknown option: --{}", name)
    
    size_t idx = LongIndex_[name];
    OptionValue& val = Values_[idx];
    val.present = true;

    if (Options_[idx].requiresArgument) {
        ASSERT(eq_pos != nullptr, "Missing argument for: --{}", name)
        val.value = eq_pos + 1;
    } else {
        ASSERT(eq_pos == nullptr, "Unexpected argument for: --{}", name)
    }
}

void GetOpts::ProcessPositional(const std::string& arg) {
    Positional_.push_back(arg);
}

bool GetOpts::Has(char shortName) const {
    if (!ShortIndex_.count(shortName)) return false;
    return Values_.at(ShortIndex_.at(shortName)).present;
}

bool GetOpts::Has(const std::string& longName) const {
    if (!LongIndex_.count(longName)) return false;
    return Values_.at(LongIndex_.at(longName)).present;
}

const std::string& GetOpts::Get(char shortName) const {
    ASSERT(Has(shortName), "Option not present: -{}", shortName);
    return Values_.at(ShortIndex_.at(shortName)).value;
}

const std::string& GetOpts::Get(const std::string& longName) const {
    ASSERT(Has(longName), "Option not present: --{}", longName);
    return Values_.at(LongIndex_.at(longName)).value;
}

void GetOpts::ValidateOption(char shortName, const std::string& longName) const {
    ASSERT(shortName != 0 || !longName.empty(), "Option must have at least one name");
    ASSERT(shortName == 0 || !ShortIndex_.count(shortName), "Duplicate short option: -{}", shortName);
    ASSERT(longName.empty() || !LongIndex_.count(longName), "Duplicate long option: --{}", longName);
}

size_t GetOpts::RegisterOption(const Option& opt) {
    size_t idx = Options_.size();
    Options_.push_back(opt);

    if (opt.shortName != 0) {
        ShortIndex_[opt.shortName] = idx;
    }
    if (!opt.longName.empty()) {
        LongIndex_[opt.longName] = idx;
    }
    Values_[idx] = OptionValue{};
    return idx;
}

const std::vector<std::string>& GetOpts::GetPositional() const {
    return Positional_;
}

std::string GetOpts::Help() const {
    std::ostringstream oss;
    oss << "Options:\n";
    
    for (const auto& opt : Options_) {
        oss << "  ";
        if (opt.shortName != 0) {
            oss << "-" << opt.shortName;
            if (!opt.longName.empty()) {
                oss << ", ";
            }
        }
        if (!opt.longName.empty()) {
            oss << "--" << opt.longName;
        }
        if (opt.requiresArgument) {
            oss << " <arg>";
        }
        oss << "\n    " << opt.description << "\n";
    }
    
    return oss.str();
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCommon

