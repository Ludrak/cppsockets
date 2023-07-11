/**
EXAMPLE main.cpp:

#include <iostream>
#include "cpp_argparse.hpp"

// custom argument value type
struct custom_arg_values
{
    int cpu;
    int mem;
};

// custom argument parser, must be in the format: int parser(const std::string& value, value_type *const dest);
// the parser will parse the value in the object pointed by dest,
// if a parsing error occurs, it can return 1 to indicate an error.
int custom_arg_parser(const std::string &value, custom_arg_values *const dst)
{
	// parses string number1,number2 in (s_custom_arg_values){ .cpu=number1, .mem=number2 }
	size_t pos = 0;
	std::string token;
	std::string s = value;

	try {
		if ((pos = s.find(",")) != std::string::npos) {
			token = s.substr(0, pos);
			dst->cpu = std::stoi(token);
			token = s.substr(pos + 1, std::string::npos);
			if (token.empty())
				return (1);
			dst->mem = std::stoi(token);
		}
	} catch (std::exception)
	{
		return (1);
	}
    return (0);
}
    
int main(int ac, char **av)
{
    // predefinition of arg parsed values
    bool		is_verbose = 0;
	int     	count = 0;
	std::string	comment;
	bool		var_bool;
	std::string server_ip;
	bool		init_mode_toggled = false;
    custom_arg_values	values_struct = {.cpu=0,.mem=0};


	// Initializes arguments list, needs to be called before any other macro
	ARG_INIT(
		// basic argument constructor with value type, flags can be supplied for parsing needs (ARG_REQUIRED, ARG_MULTIPLE)
		ARG<int>("-g", "--global", "global count", &count, ARG_REQUIRED),
		// no value toggle argument macro
		// dest var must be a boolean which is set to true if the argument is present
		ARG_NOVALUE("-v", "--verbose", "verbose", &is_verbose),

		// argument modes, for all sub arguments, the 'init' mode needs to be selected otherwise they are ignored
		// a boolean value needs to be provided, it will be set to true if the mode is set.
		//
		// The ARG_MODE_REQUIRED macro is also available, it adds the ARG_REQUIRED flag to the group,
		// and does not need a bool* since it should always be defined in arguments
		ARG_MODE("init", "init mode", &init_mode_toggled,
			ARG<std::string>("-c", "--c", "comment tag", &comment, ARG_REQUIRED),
			ARG<bool>("-b", "--b", "set bool value", &var_bool),
			
			// argument groups, they are here only for fancier usage printing
			ARG_GROUP("sub-group", "sub group of init group",
				ARG<std::string>("-i", "--ip", "set ip address in string", &server_ip)
			)
		),
		// custom arguments, custom argument types can be used, however you will need to implement a parser for that
		// argument of the format int parser(const std::string& value, value_type *const dest);
		ARG<custom_arg_values>("-C", "--custom", "Custom value argument", &values_struct, ARG_NOFLAGS, &custom_arg_parser)
	);

	// prints usage string
	ARG_USAGE("Usage header string");

	// parses arguments from list generated in ARG_INIT
	int parsing_failed = ARG_PARSE(ac, av);

	if (parsing_failed != 0)
	{
		std::cout << "parsing failed." << std::endl;
		return (1);
	}

	// retrieving values in previously defined variables
	std::cout << std::endl << "Parsed results:" << std::endl
	<< "count: " << count << std::endl
	<< "is_verbose: " << is_verbose << std::endl
	<< "init_mode_toggled: " << init_mode_toggled << std::endl
	<< "comment: " << comment << std::endl
	<< "var_bool: " << var_bool << std::endl
	<< "server_ip: " << server_ip << std::endl
	<< "custom: { .cpu=" << values_struct.cpu << " .mem=" << values_struct.mem << " }" << std::endl
	<< std::endl;
}

*/

#pragma once

#include <iostream>
#include <string>
#include <tuple>
#include <list>

/* argument flags (bitwise values) */
#define ARG_NOFLAGS         0b00000000
#define ARG_MULTIPLE        0b00000001
#define ARG_REQUIRED        0b00000010

/* library argument flags don't use them in user space */
#define _ARG_SELECTED       0b00000100
#define _ARG_GROUP_FLAG     0b00001000
#define _ARG_GROUP_MODE     0b00010000


/*
** ------------------------------------------------------------------------
** DEFAULT VALUE TYPE PARSERS
** ------------------------------------------------------------------------
*/

template<typename T, size_t S = sizeof(T)>
int _parse(const std::string &s, T *const dst) { static_assert(sizeof(T) == -1, "No parser defined for custom value type"); (void)s; (void)dst; return 0; }


template<>
int _parse<bool>(const std::string &s, bool *const dst)
{
    if (dst == NULL)
        return 1;
    if (s == "true" || s == "1" || s == "enable")
    {
        *dst = true;
        return 0;
    }
    else if (s == "false" || s == "0" || s == "disable")
    {
        *dst = false;
        return 0;
    }
    return 1;
};

template<>
int _parse<int>(const std::string &s, int *const dst)
{
    try {
        *dst = std::stoi(s);
    } catch (std::exception& e)
    {
        return (1);
    }
    return (0);
}

template<>
int _parse<long>(const std::string &s, long *const dst)
{
    try {
        *dst = std::stol(s);
    } catch (std::exception& e)
    {
        return (1);
    }
    return (0);
}

template<>
int _parse<long long>(const std::string &s, long long *const dst)
{
    try {
        *dst = std::stoll(s);
    } catch (std::exception& e)
    {
        return (1);
    }
    return (0);
}

template<>
int _parse<float>(const std::string &s, float *const dst)
{
    try {
        *dst = std::stof(s);
    } catch (std::exception& e)
    {
        return (1);
    }
    return (0);
}

template<>
int _parse<double>(const std::string &s, double *const dst)
{
    try {
        *dst = std::stod(s);
    } catch (std::exception& e)
    {
        return (1);
    }
    return (0);
}

template<>
int _parse<std::string>(const std::string &s, std::string *const dst)
{
    *dst = s;
    return 0;
};


/*
** ------------------------------------------------------------------------
** ARGUMENT STRUCT
** ------------------------------------------------------------------------
*/

template <typename T>
struct  ARG
{
    typedef T   value_type;

    std::string shortcut_name;  // shortcut name identifier
    std::string name_id;        // name identifier
    std::string usage_str;      // usage string
    char        *value;         // value pointer in argv
    T           *dst_variable;  // destination variable pointer for parsing return
    int         flags;          // flags

                                // value type parser, outputs the parsed result in dst_variable
    int         (*value_type_parser)(const std::string &, T *const);
    
    ARG (
        const std::string &shortcut_name,
        const std::string &name_id,
        const std::string &usage_str,
        T *dest_var,
        int flags = 0,
        int (*value_parser)(const std::string &, T *const) = _parse<T>
    )
      : shortcut_name(shortcut_name),
        name_id(name_id),
        usage_str(usage_str),
        value(NULL),
        dst_variable(dest_var),
        flags(flags),
        value_type_parser(value_parser)
        {}
};

template<typename... Args>
std::ostream& operator<<(std::ostream& os, const ARG<Args...>& obj)
{
    return os << obj.name_id;
}

#define ARG_NOVALUE(shortcut_name, name, usage, is_selected)\
    ARG<bool>(shortcut_name, name, usage, is_selected, 0, NULL)


/*
** ------------------------------------------------------------------------
** GROUP ARGUMENT STRUCT
** ------------------------------------------------------------------------
*/

template <typename ...Args>
struct  GROUP : ARG<bool>
{
    std::tuple<Args...> sub_args;   // sub arguments array for group arguments
    bool                selected;   // boolean for mode arguments, becomes true if mode is selected

    GROUP (
        const std::string &shortcut_name,
        const std::string &name_id,
        const std::string &usage_str,
        bool *group_selected,
        int flags,
        Args... sub_args
    )
      : ARG<bool>(shortcut_name, name_id, usage_str, group_selected, flags | _ARG_GROUP_FLAG, NULL),
        sub_args(sub_args...),
        selected(false)
        {}
};

template<typename... Args>
std::ostream& operator<<(std::ostream& os, const GROUP<Args...>& obj)
{
    return os << obj.name_id;
}

template <typename... Args>
GROUP<Args...>  make_group(
        const std::string &shortcut_name,
        const std::string &name_id,
        const std::string &usage_str,
        bool *group_selected,
        int flags,
        Args... sub_args)
    {
        return (GROUP<Args...>(shortcut_name, name_id, usage_str, group_selected, flags, sub_args...));
    }

/*
** ------------------------------------------------------------------------
** ARGLIST INITIALIZER
** ------------------------------------------------------------------------
*/

#define ARG_INIT(...) \
    decltype(make_group("", "", "", NULL, 0, __VA_ARGS__)) __arglist = make_group("", "main_group", "", NULL, _ARG_GROUP_FLAG, __VA_ARGS__)



/*
** ------------------------------------------------------------------------
** GROUP/MODE DEFINITION MACROS
** ------------------------------------------------------------------------
*/

#define ARG_GROUP(name, usage, ...) \
    make_group(name, name, usage, NULL, _ARG_GROUP_FLAG, __VA_ARGS__)

#define ARG_MODE(name, usage, bool_mode_active, ...) \
    make_group(name, name, usage, bool_mode_active, _ARG_GROUP_FLAG | _ARG_GROUP_MODE, __VA_ARGS__)

#define ARG_MODE_REQUIRED(name, usage, ...) \
    make_group(name, name, usage, NULL, _ARG_GROUP_FLAG | _ARG_GROUP_MODE | ARG_REQUIRED, __VA_ARGS__)


/*
** ------------------------------------------------------------------------
** ARGUMENT PARSING
** ------------------------------------------------------------------------
*/

/* arg parsers specifications for ARG and GROUP */

// error manager for errors on argument parsing
// error must be set once per argument using _arg_error(int)
// then it can be retrieved in the global error with _arg_error()
int _arg_error(int err = 0)
{
    static int error = 0;

    int last_error = error;
    error = err;
    return (last_error);
}

template<typename T>
void _parse_arg(T& none,  char *const value_p, size_t *const n_values)
    { (void)none; (void)value_p; (void)n_values; }

// group parser
template<typename... A>
void _parse_arg(GROUP<A...>& group, char *const value_p, size_t *const n_values)
{
    (void)value_p;
    
    group.flags |= _ARG_SELECTED;
    if (group.flags & _ARG_GROUP_MODE)
    {
        group.selected = true;
        *n_values = 0;
    }
}


// arg parser
template<typename A>
void _parse_arg(ARG<A>& arg, char *const value_p, size_t *const n_values)
{
    arg.flags |= _ARG_SELECTED;
    if (value_p == NULL)
    {
        // TODO ADD MORE CHECKS
        if (arg.value_type_parser != NULL)
        {
            std::cerr << "A value is expected for argument " << arg << std::endl;
            _arg_error(1);
            return ;
        }
        // no value argument
        *(bool*)arg.dst_variable = true;
        _arg_error(0);
        return ;
    }
    else if (arg.value_type_parser == NULL)
    {
        // no value argument, dont parse but set dst_variable which must be bool type
        *(bool*)arg.dst_variable = true;
        _arg_error(0);
        return ;
    }
    else if (arg.value != NULL && (arg.flags & ARG_MULTIPLE) == 0)
    {
        std::cerr << "Argument " << arg << " cannot be set multiples times." << std::endl;
        _arg_error(1);
        return ;
    }
    if (arg.dst_variable == NULL)
    {
        std::cerr << "Argument " << arg << " has no destination variable but requires a value." << std::endl;
        _arg_error(1);
        return ;    
    }

    arg.value = value_p;
    (*n_values)++;
    if (arg.value_type_parser(value_p, arg.dst_variable) != 0)
    {
        std::cerr << "Parsing error occurred on argument " << arg << ": invalid value format: `" << std::string(value_p) << "`" << std::endl;
        _arg_error(1);
        return ;
    }
    _arg_error(0);
}

/*
** Tuple argument parser
** Returns a bool indicating if the argument was parsed
*/

// Case end of tuple list
template<std::size_t I = 0, typename... Tp> 
inline typename std::enable_if<I == sizeof...(Tp), bool>::type
_parse_arg_in_tuple(const std::string& name, char *const value_p, size_t *const n_values, std::tuple<Tp...>& t)
    { (void)t; (void)name; (void)value_p; (void)n_values; return false; }


/* Tuple argument sub_arg parsing (Group modes) */

//default does nothing
template<typename T>
bool _parse_tuple_sub_args(const std::string& name, char *const value_p, size_t *const n_values, T& t)
    { (void)name; (void)value_p; (void)n_values; (void)t; return (false); }

// specification for GROUP<Tp...> which contains the sub_args tuple
template<typename... Tp>
bool _parse_tuple_sub_args(const std::string& name, char *const value_p, size_t *const n_values, GROUP<Tp...>& group)
{
    if (group.selected == true || (group.flags & _ARG_GROUP_MODE) == 0)
        return _parse_arg_in_tuple(name, value_p, n_values, group.sub_args);
    return (false);
}


// Parser for elements in tuple, parses sub args if argument is a group argument.
template<std::size_t I = 0, typename... Tp>
inline typename std::enable_if<I < sizeof...(Tp), bool>::type
_parse_arg_in_tuple(const std::string& name, char *const value_p, size_t *const n_values, std::tuple<Tp...>& t)
{
    decltype(std::get<I>(t)) elem = std::get<I>(t);
    if (elem.name_id == name || elem.shortcut_name == name)
    {
        _parse_arg(elem, value_p, n_values);
        return true;
    }
    
    // helper which will parse sub args if type is GROUP and if it is selected, otherwise do nothing
    if (_parse_tuple_sub_args(name, value_p, n_values, elem) == true)
        return (true);

    return _parse_arg_in_tuple<I + 1, Tp...>(name, value_p, n_values, t);
}

template<typename T>
int _check_required_sub_args(T& t, const std::string &group_name)
    { (void)t; (void)group_name; return (0); }

template<typename... Tp>
int _check_required_sub_args(GROUP<Tp...>& group, const std::string &group_name)
{
    int err = 0;
    if (group.selected == false && (group.flags & ARG_REQUIRED) != 0)
    {
        if (!group_name.empty())
            std::cerr << "Mode " << group << " is required in mode " << group_name << " however it is not defined." << std::endl;
        else
            std::cerr << "Mode " << group << " is required however it is not defined." << std::endl;
        err = 1;
    }
    if (group.selected == true || (group.flags & _ARG_GROUP_MODE) == 0)
        return err + _check_required(group.sub_args, group.name_id);
    return (err);
}

// size = sizeof args case
template<std::size_t I = 0, typename... Tp>
inline typename std::enable_if<I == sizeof...(Tp), int>::type
_check_required(std::tuple<Tp...>& t, const std::string &group_name = "")
    {(void)t; (void)group_name; return (0); }

// check if required arguments are supplied
template<std::size_t I = 0, typename... Tp>
inline typename std::enable_if<I < sizeof...(Tp), int>::type
_check_required(std::tuple<Tp...>& t, const std::string &group_name = "")
{
    decltype(std::get<I>(t)) elem = std::get<I>(t);
    int err = 0;
    if ((elem.flags & _ARG_GROUP_FLAG) == 0 && (elem.flags & ARG_REQUIRED) != 0 && (elem.flags & _ARG_SELECTED) == 0)
    {
        if (!group_name.empty())
            std::cerr << "Argument " << elem << " is required in group " << group_name << " however no value was provided to it." << std::endl;
        else
            std::cerr << "Argument " << elem << " is required however no value was provided to it." << std::endl;
        err = 1;
    }
    err += _check_required_sub_args(elem, group_name);
    err += _check_required<I + 1, Tp...>(t, group_name);
    return (err);
}


// parses the whole arglist
template<typename... Args>
int arg_parse(int argc, char **argv, GROUP<Args...>& arglist)
{
    int     i = 1;
    int     error = 0;

    while (i < argc)
    {
        size_t n_values = 0;
        bool   arg_parsed = false;
        if (i + 1 < argc)
        {
            arg_parsed = _parse_arg_in_tuple(argv[i], argv[i + 1], &n_values, arglist.sub_args);
            i += n_values;
        }
        else
        {
            arg_parsed = _parse_arg_in_tuple(argv[i], NULL, &n_values, arglist.sub_args);
        }

        // gets back the error from parsing if there was one
        error += _arg_error();
        if (arg_parsed == false)
        {
            std::cerr << "Unknown argument `" << argv[i] << "`." << std::endl;
            error++;
            // todo handle group unselected specific log
        }
        ++i;
    }
    return error + _check_required(arglist.sub_args);
}

/* macro for simper utilisation, needs to define __arglist with ARG_INIT macro before */
#define ARG_PARSE(argc, argv) arg_parse(argc, argv, __arglist);



/*
** ------------------------------------------------------------------------
** USAGE PRINTING
** ------------------------------------------------------------------------
*/

/* Printing tuple element */
template<typename T>
void _print_arg_element(const T& t)
	{ (void)t; }

// printing argument
template<typename A>
void _print_arg_element(const ARG<A>& t)
{
	std::cout << "\t" << t.shortcut_name << ", " << t.name_id << ":\t" << t.usage_str << std::endl;
}

// printing group
template<typename... A>
void _print_arg_element(const GROUP<A...>& t)
{
	std::cout << std::endl << "\033[1m\033[4m" << t << "\033[0m:" << std::endl;
    std::cout << "\033[3m" << t.usage_str << "\033[0m" << std::endl << std::endl;
	_print_arglist_usage(t.sub_args);
    std::cout << std::endl;
}


/* Case end of tuple list */
template<std::size_t I = 0, typename... Tp>
inline typename std::enable_if<I == sizeof...(Tp), void>::type
_print_arglist_usage(const std::tuple<Tp...>& t)
    { (void)t; }

/* Recursing tuple for printing */
template<std::size_t I = 0, typename... Tp>
inline typename std::enable_if<I < sizeof...(Tp), void>::type
_print_arglist_usage(const std::tuple<Tp...>& t)
{
    _print_arg_element(std::get<I>(t));
    _print_arglist_usage<I + 1, Tp...>(t);
}

/* Usage with header */
template<typename... Tp>
void print_usage(const std::string& header, const std::tuple<Tp...>& t)
{
    std::cout << header << std::endl;
    _print_arglist_usage(t);
}

/* macro for simper utilisation, needs to define __arglist with ARG_INIT macro before */
#define ARG_USAGE(header_string) print_usage(header_string, __arglist.sub_args);
