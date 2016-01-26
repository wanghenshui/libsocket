/**
 * A modern command line parser
 * @since  2016.01.09
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 * @link   http://docopt.org
 * -----------------------------------------------------------------------------
 * We accept the following usage:
 * :-) app
 * :-) app -opt1=val --opt2=val ...
 * :-) app action
 * :-) app action -opt1=val --opt2=val ...
 * :-) app action object1 object2 -opt1=val --opt2=val ...
 * :-) app action sub-action object1 object2 -opt1=val --opt2=val ...
 * -----------------------------------------------------------------------------
 * The main three parts is Action + Object + Option
 * Action: the operation that user want to do
 *    e.g: git clone, "clone" is the action
 * Object: the operation target
 *    e.g: git clone https://github.com/chensoft/libchen.git, url is the object
 * Option: the action flags
 *    e.g: git --version, "version" is the flag
 * Sub-action: an action can contain a sub-action
 *    e.g: git submodule update, "update" is the sub-action of "submodule"
 * -----------------------------------------------------------------------------
 * Usage: todo
 * >> chen::cmd::parser flag;
 * >>
 * >> flag.define<bool>("help", "h", "show help");
 * >> flag.define<int>("port", "p", "server port (default: 53)", 53);
 * >> flag.define<std::string>("zone", "z", "zone folder", "");
 * >>
 * >> flag.parse(argc, argv);
 * >>
 * >> bool help = flag.get<bool>("help");
 */
#pragma once

#include "any.hpp"
#include <functional>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>

namespace chen
{
    namespace cmd
    {
        class action;
        class object;
        class option;

        // -------------------------------------------------------------------------
        // parser
        class parser
        {
        public:
            parser(const std::string &app = "");
            virtual ~parser() = default;

        public:
            /**
             * Parse the command line
             */
            virtual void parse(int argc, const char *const argv[]);

            /**
             * Define action, sub-action's name is separated by dots
             * e.g: git clone, name is "clone"
             * e.g: git submodule init, name is "submodule.init"
             */
            virtual void action(const std::string &name,
                                const std::string &desc,
                                std::function<void (const chen::cmd::parser &parser)> bind = nullptr);

            /**
             * Define object
             * @param action identify which action has this object
             * @param min the minimum count of object
             * @param max the maximum count of object, if zero means unlimited
             */
            virtual void object(const std::string &action,
                                const std::string &name,
                                int min,
                                int max);

            /**
             * Define option
             * @param action identify which action has this option
             * @param name complete name of option, e.g: app --help, "help" is the full name
             * @param tiny short name of option, e.g: app -h, "h" is the short name
             * @param desc description of the option
             * @param def the default value when the option is not specified
             * @param pre the predicate, you can use predefined like range_of or write you own
             */
            void option(const std::string &action,
                        const std::string &name,
                        const std::string &tiny,
                        const std::string &desc,
                        const chen::any &def = chen::any(),
                        const chen::any &pre = chen::any());

        public:
            /**
             * Get current matched action name
             */
            virtual std::string current() const;

            /**
             * Get the value of the object which belongs to the current action
             * @param object the name of the object
             * @result the matched params array
             */
            virtual std::vector<std::string> objVal(const std::string &object) const;

            /**
             * Get the value of the option which belongs to the current action
             * if the current action doesn't has this option, an error will be thrown
             * support bool, int, int64, double, string, it's enough to get value about cli
             * @param option the full name of the option
             */
            virtual bool boolVal(const std::string &option) const;
            virtual std::int32_t intVal(const std::string &option) const;
            virtual std::string strVal(const std::string &option) const;

            virtual std::int64_t int64Val(const std::string &option) const;
            virtual double doubleVal(const std::string &option) const;

            /**
             * Rest unresolved arguments
             */
            virtual const std::vector<std::string>& rest() const;

        public:
            /**
             * Show usage info
             */
            virtual void usage() const;

            /**
             * The text before the usage body
             */
            virtual void prefix(const std::string &text);

            /**
             * The text after the usage body
             */
            virtual void suffix(const std::string &text);

            /**
             * Provide a suggestion for a specific action
             * @param alias action alias
             * @param action action name
             * e.g: app has an action: "remove", if user input "delete", parser will prompt user "Did you mean remove?"
             */
            virtual void suggest(const std::string &alias, const std::string &action);

        protected:
            /**
             * Final usage output
             */
            virtual void output(const std::string &text) const;

        protected:
            std::string _app;     // app name
            std::string _prefix;  // usage prefix
            std::string _suffix;  // usage suffix
            std::string _action;  // current action

            std::vector<std::string> _rest;  // rest unresolved params

            std::map<std::string, std::string> _suggest;  // intelligent suggest
            std::map<std::string, chen::cmd::action> _define;  // action defines
        };


        // -------------------------------------------------------------------------
        // action
        class action
        {
        public:
            action(const std::string &name,
                   const std::string &desc,
                   std::function<void (const chen::cmd::parser &parser)> bind = nullptr);
            virtual ~action() = default;

        public:
            /**
             * Associate an object with this action
             */
            virtual void add(const chen::cmd::object &object);

            /**
             * Associate an option with this action
             */
            virtual void add(const std::string &name, const chen::cmd::option &option);

        public:
            /**
             * Properties
             */
            virtual const std::string& name() const;
            virtual const std::string& desc() const;
            virtual const std::function<void (const chen::cmd::parser &parser)>& bind() const;

            virtual const std::vector<chen::cmd::object>& objects() const;
            virtual const std::map<std::string, chen::cmd::option>& options() const;

        protected:
            std::string _name;
            std::string _desc;
            std::function<void (const chen::cmd::parser &parser)> _bind;

            std::vector<chen::cmd::object> _objects;
            std::map<std::string, chen::cmd::option> _options;
        };


        // -------------------------------------------------------------------------
        // object
        class object
        {
        public:
            object(const std::string &name, int min, int max);
            virtual ~object() = default;

        public:
            /**
             * Properties
             */
            virtual const std::string& name() const;
            virtual int min() const;
            virtual int max() const;
            virtual const std::vector<std::string>& val() const;

        protected:
            std::string _name;

            int _min = 0;
            int _max = 0;

            std::vector<std::string> _val;
        };


        // -------------------------------------------------------------------------
        // option
        class option
        {
        public:
            option(const std::string &name,
                   const std::string &tiny,
                   const std::string &desc,
                   const chen::any &def,
                   const chen::any &pre);
            virtual ~option() = default;

        public:
            /**
             * Properties
             */
            virtual const std::string& name() const;
            virtual const std::string& tiny() const;
            virtual const std::string& desc() const;

            virtual const chen::any& val() const;
            virtual const chen::any& def() const;
            virtual const chen::any& pre() const;

        protected:
            std::string _name;
            std::string _tiny;
            std::string _desc;

            chen::any _val;
            chen::any _def;
            chen::any _pre;
        };


        // -------------------------------------------------------------------------
        // error
        class error : public std::runtime_error
        {
        public:
            explicit error(const std::string &what) : std::runtime_error(what) {}
        };


        // -------------------------------------------------------------------------
        // one_of_helper, used for string or number
        template <typename T>
        class one_of_helper
        {
        public:
            one_of_helper(std::initializer_list<T> init) : _data(init)
            {
            }

            bool operator()(const std::string &val)
            {
                T dst;

                std::istringstream ss(val);
                ss >> dst;

                return this->_data.find(dst) != this->_data.end();
            }

        private:
            std::set<T> _data;
        };

        template <typename T, typename ... Args>
        static one_of_helper<T> one_of(Args ... args)
        {
            return one_of_helper<T>({args...});
        }


        // -------------------------------------------------------------------------
        // range_of_helper, [beg, end], mainly used for number
        template <typename T>
        class range_of_helper
        {
        public:
            range_of_helper(T beg, T end) : _beg(beg) , _end(end)
            {
            }

            bool operator()(const std::string &val)
            {
                T dst;

                std::istringstream ss(val);
                ss >> dst;

                return (dst >= this->_beg) && (dst <= this->_end);
            }

        private:
            T _beg;
            T _end;
        };

        template <typename T>
        static range_of_helper<T> range_of(T beg, T end)
        {
            return range_of_helper<T>(beg, end);
        }
    }
}
