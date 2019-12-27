#pragma once
#include "exception.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace ict {
class Option {
  public:
    Option(std::string name, char short_opt, std::string desc,
           std::function<void()> action = []() {})
        : name(name), short_opt(short_opt), desc(desc), binary_action(action),
          flag(true), present(false) {}

    Option(std::string name, char short_opt, std::string desc, std::string def,
           std::function<void(std::string val)> action = [](std::string) {})
        : name(name), short_opt(short_opt), desc(desc), default_value(def),
          action(action), flag(false), present(false) {}

    std::string name;
    char short_opt;
    std::string desc;
    std::string default_value;

    std::function<void()> binary_action;
    std::function<void(std::string)> action;

    bool flag;
    bool present;
};

typedef Option option;

class command {
  public:
    enum State {
        Idle,
        ShortStart,
        ShortContinue,
        ValueStart,
        Value,
        QuotedValue,
        LongOptionStart,
        LongOption,
        LookingForEqual,
        TargetStart,
        Target

    };

    command(std::string cmd, std::string brief, std::string usage,
            const std::string &version_ = "")
        : cmd(cmd), brief(brief), usage(usage), version(version_) {}

    void add(Option const &opt) { opts.push_back(opt); }

    void add_note(std::string const &note) { notes.push_back(note); }

    void help() const {
        std::cout << cmd << " -- " << brief << "\n\n"
                  << usage << "\n"
                  << "Options:\n";
        for (auto const &opt : opts) {
            std::cout << "    ";
            if (opt.short_opt != '~')
                std::cout << "-" << opt.short_opt << ", ";
            std::cout << "--" << opt.name;
            if (opt.flag)
                std::cout << " : " << opt.desc << "\n";
            else {
                std::cout << "= value : " << opt.desc;
                if (!opt.default_value.empty())
                    std::cout << " (" << opt.default_value << ")";
                std::cout << std::endl;
            }
        }
        if (!notes.empty()) {
            std::cout << "\n";
            for (auto const &n : notes)
                std::cout << n << "\n";
        }
        std::cout << "\n";
    }

    void parse(int argc, char *argv[]) {
        State state = Idle;
        std::string cl;
        std::string target;
        std::string long_opt;
        std::string value;
        std::vector<Option>::iterator it;

        // first, let's add help

        it = std::find_if(opts.begin(), opts.end(),
                          [&](Option &o) { return o.short_opt == 'h'; });
        add(Option("help", (it == opts.end() ? 'h' : '~'),
                   "Show this help usage", [&] {
                       help();
                       exit(0);
                   }));

        if (!version.empty())
            add(option("version", 'v', "Display version information", [&] {
                std::cout << cmd << " version " << version << '\n';
                exit(0);
            }));

        for (int i = 1; i < argc; ++i) {
            cl += argv[i];
            if (i != argc - 1)
                cl += ' ';
        }
        cl += ' ';

        for (auto ch : cl) {
            switch (state) {
            case Idle: // looking for the first character
                switch (ch) {
                case '-':
                    state = ShortStart;
                    break;
                case ' ':
                    break;
                default: // isgraph character, starting target
                    target += ch;
                    state = Target;
                    break;
                }
                break;
            case ShortStart: // first '-'
                switch (ch) {
                case '-':
                    state = LongOptionStart;
                    break;
                case ' ':
                    IT_PANIC("illegal \"- \" sequence");
                default:
                    if (isgraph(ch)) // found short option
                    {
                        it = std::find_if(
                            opts.begin(), opts.end(),
                            [&](Option &o) { return o.short_opt == ch; });
                        if (it == opts.end())
                            IT_PANIC("invalid option: " << ch);

                        if (it->present)
                            IT_PANIC("option " << ch << " already present");
                        it->present = true;
                        // do it!  We may want to save the actions until
                        // parsing
                        // is complete, in case there are errors later in
                        // command line
                        if (it->flag) {
                            it->binary_action();
                            state = ShortContinue;
                        } else
                            state = ValueStart;
                    }
                }
                break;
            case ShortContinue: // possibly more short opts
                switch (ch) {
                case '-':
                    IT_PANIC("illegal \"-\" character");
                case ' ':
                    state = Idle;
                    break;
                default:
                    if (isgraph(ch)) // found short option
                    {
                        it = std::find_if(
                            opts.begin(), opts.end(),
                            [&](Option &o) { return o.short_opt == ch; });
                        if (it == opts.end())
                            IT_PANIC("invalid option: " << ch);
                        if (it->present)
                            IT_PANIC("option " << ch << " already present");
                        it->present = true;
                        if (it->flag)
                            it->binary_action();
                        else
                            state = ValueStart;
                    }
                }
                break;
            case ValueStart:
                switch (ch) {
                case ' ':
                    break;
                case '\"':
                    IT_WARN("setting to quoted value");
                    value.clear();
                    state = QuotedValue;
                    break;

                default:
                    value.clear();
                    value += ch;
                    state = Value;
                }
                break;
            case Value:
                switch (ch) {
                case ' ':
                    it->action(value);
                    state = Idle;
                    break;
                default:
                    value += ch;
                }

                break;
            case QuotedValue:
                switch (ch) {
                case '\"':
                    it->action(value);
                    state = Idle;
                    break;
                default:
                    value += ch;
                }

                break;
            case LongOptionStart: // "--"
                if (isgraph(ch)) {
                    long_opt.clear();
                    long_opt += ch;
                    state = LongOption;
                } else {
                    IT_PANIC("illegal \"--" << ch << "\" sequence");
                }
                break;

            case LongOption:
                switch (ch) {
                case ' ':
                    it = std::find_if(opts.begin(), opts.end(), [&](Option &o) {
                        return o.name == long_opt;
                    });
                    if (it == opts.end())
                        IT_PANIC("invalid option: " << long_opt);
                    it->present = true;
                    if (it->flag) {
                        it->binary_action();
                        state = Idle;
                    } else
                        state = LookingForEqual;
                    break;
                case '=':
                    it = std::find_if(opts.begin(), opts.end(), [&](Option &o) {
                        return o.name == long_opt;
                    });
                    if (it == opts.end())
                        IT_PANIC("invalid option: " << long_opt);
                    it->present = true;
                    if (it->flag)
                        IT_PANIC("value not expected for option: " << long_opt);
                    state = ValueStart;
                    break;
                default:
                    long_opt += ch;
                    break;
                }
                break;

            case LookingForEqual:
                switch (ch) {
                case ' ':
                    break;
                case '=':
                    state = ValueStart;
                    break;
                default:
                    IT_PANIC("expected '=' after \"" << long_opt
                                                     << "\" option");
                }
                break;
            case TargetStart:
                switch (ch) {
                case ' ':
                    break;

                default:
                    target += ch;
                    state = Target;
                }
                break;

            case Target:
                switch (ch) {
                case ' ':
                    targets.push_back(target);
                    target.clear();
                    state = TargetStart; // not Idle, since we don't allow post
                                         // target options (yet)
                    break;
                default:
                    target += ch;
                }
                break;
            }
        }
    }

    std::vector<std::string> targets;

  private:
    std::vector<Option> opts;

    std::vector<std::string> notes;

    std::string cmd;
    std::string brief;
    std::string usage;
    std::string version;
};

} // namespace ict
