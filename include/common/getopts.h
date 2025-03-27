#pragma once

#include <common/exception.h>

#include <string>
#include <unordered_map>
#include <vector>

namespace NCommon {

////////////////////////////////////////////////////////////////////////////////

class GetOpts {
public:
    struct Option {
        char shortName;
        std::string longName;
        std::string description;
        bool requiresArgument;
    };

    GetOpts() = default;

    void AddOption(char shortName, 
                   const std::string& longName,
                   const std::string& description,
                   bool requiresArgument = false);

    void Parse(int argc, const char* const argv[]);

    bool Has(char shortName) const;
    bool Has(const std::string& longName) const;

    const std::string& Get(char shortName) const;
    const std::string& Get(const std::string& longName) const;

    const std::vector<std::string>& GetPositional() const;
    
    std::string Help() const;

private:
    struct OptionValue {
        std::string value;
        bool present = false;
    };

    std::vector<Option> Options_;
    std::unordered_map<char, size_t> ShortIndex_;
    std::unordered_map<std::string, size_t> LongIndex_;
    std::unordered_map<size_t, OptionValue> Values_;
    std::vector<std::string> Positional_;

    void ValidateOption(char shortName, const std::string& longName) const;
    size_t RegisterOption(const Option& opt);
    void ProcessShortOption(int argc, const char* const argv[], int& i);
    void ProcessLongOption(const char* arg);
    void ProcessPositional(const std::string& arg);
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCommon
