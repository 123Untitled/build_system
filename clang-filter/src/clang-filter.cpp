#include "../inc/clang-filter.hpp"

// remap clang++ error messages to shorter and funnier ones
const cf::logger::mapppings cf::logger::_mapppings = {

	// to change !
	{"no viable constructor or deduction guide for deduction of template arguments of",
		"no viable constructor or deduction guide for deduction of template arguments of"},


	{"has been explicitly marked deleted here",
		"annihilated here"},

	{"invalid explicitly-specified argument for template parameter",
		"faulty template argument"},

	{"set but not used",
		"lone wolf"},

	{"out-of-line definition of",
		"rebel definition of"},

	{"does not match any declaration in",
		"declared nowhere in"},

	{"call to deleted constructor of",
		"banned ctor of"},

	{"static assertion failed",
		"static flop"},

	{"in member function",
		"in method"},

	{"does not satisfy",
		"doesn't cut it"},

	{"for 1st argument",
		"for 1st arg"},

	{"invalid operands to binary expression",
		"binary blunder"},

	{"previous definition is here",
		"early bird here"},

	{"requested here",
		"called here"},

	{"was not satisfied",
		"didn't make the cut"},

	{"definition of implicitly declared destructor",
		"implicit destructor defined"},

	{"undeclared identifier",
		"unknown identifier"},

	{"candidate function has been explicitly deleted",
		"vanished function"},

	{"couldn't infer template argument",
		"couldn't crack template argument"},

	{"expanded from macro",
		"macro-burst"},

	{"overload resolution selected deleted operator",
		"banned operator picked"},

	{"evaluated to false",
		"evaluated to nope"},

	{"candidate function not viable",
		"function lost its mojo"},

	{"no known conversion from",
		"no magic for"},

	{"candidate template ignored",
		"template ghosted"},

	{"candidate function template not viable",
		"unworkable function template"},

	{"constraints not satisfied for class template",
		"failed template wish"},

	{"use of undeclared identifier",
		"mysterious identifier"},

	{"no matching function for call to",
		"function call has no match"},

	{"unknown type name",
		"obscure"},

	{"did you mean",
		"suggest"},

	{"expected unqualified-id",
		"wrong syntax"},

	{"too many template arguments for function template",
		"excess template arguments"},

	{"deduced conflicting types for parameter",
		"type battle royale for the parameter"},

	{"in instantiation of function template specialization",
		"during template magic"},

	{"in instantiation of member function",
		"in member function"},

	{"required from here",
		"required here"},

};

const std::regex cf::logger::_rgx[TYPE_MAX - 1] = {
	std::regex{"^note: "},
	std::regex{"^warning: "},
	std::regex{"^error: "}
};

const char* cf::logger::_colors[TYPE_MAX] = {
	"\033[1;36m",
	"\033[1;33m",
	"\033[1;32m",
	"\033[1;35m"
};

const char* what[4] = {
	"    note: ",
	" warning: ",
	"   error: ",
	"undefined "
};


cf::logger::logger(std::string&& path,
		std::string&& file, std::string&& line, std::string&& column, std::string&& message)
:	  _path{""},
	  _file{std::move(file)},
	  _line{std::move(line)},
	_column{std::move(column)},
	_messages{ } {

	// extract full path:
	std::regex fp{"^(.*):[0-9]+:[0-9]+: *(.*)$"};
	std::smatch match;
	if (std::regex_search(path, match, fp)) {
		_path = std::move(match[1]);
	}

	// check message type
	if      (std::regex_search(message, _rgx[NOTE]))    { _type = NOTE;      }
	else if (std::regex_search(message, _rgx[ERROR]))   { _type = ERROR;     }
	else if (std::regex_search(message, _rgx[WARNING])) { _type = WARNING;   }
	else                                                { _type = UNDEFINED; }

	auto clean = std::regex_replace(std::move(message), _rgx[_type], "");

	// replace clang++ error messages by shorter and funnier ones
	for (auto& [key, value] : _mapppings) {
		const std::regex rgx{key};
		if (std::regex_search(clean, rgx)) {
			clean = std::regex_replace(clean, rgx, value);
		}
	}

	// replace generic template names by their real name
	static std::regex rgx{"'(.*)' \\(aka '(.*)'\\)"};
	if (std::regex_search(clean, _match, rgx)) {
		clean = std::regex_replace(clean, rgx, "'$2'");
	}

	// search quotes to colorize them
	static std::regex rgx2{" '(.*?)'([ ]|$)"};
	if (std::regex_search(clean, _match, rgx2)) {
		clean = std::regex_replace(clean, rgx2, " \x1b[4;31m'$1'\x1b[0m ");
	}

	// search double quotes to colorize them
	static std::regex rgx3{" \"(.*?)\"([ ]|$)"};
	if (std::regex_search(clean, _match, rgx3)) {
		clean = std::regex_replace(clean, rgx3, " \x1b[4;31m\"$1\"\x1b[0m ");
	}



	_messages.emplace_back(std::move(clean));
}





bool is_symbol(const char* charset, const char& c) {
	// loop over charset
	for (unsigned x = 0; charset[x] != '\0'; ++x) {
		// check match !
		if (charset[x] == c) { return true; }
	} // no match
	return false;
}

void cf::logger::colorize(const string& snip) const {
	enum { PUNC, OPER };
	static const char* symbols[] = {
		// punctuation ()[]{}<>:;,.
		"()[]{}<>:;,.",
		// operators +-*/%^=!?&|~
		"+-*/%^=!?&|~"
	};

	static const char* colors[] = {
		"\033[2;35m", // punctuation
		"\033[1;37m",  // operators
		"\x1b[0m" // reset
	};

	// loop over characters
	for (auto it = snip.begin(); it != snip.end(); ++it) {

		if (is_symbol(symbols[PUNC], *it)) {
			std::cout << colors[PUNC];
		}
		else if (is_symbol(symbols[OPER], *it)) {
			std::cout << colors[OPER];
		}
		else {
			std::cout << colors[2];
		}
		// print character
		std::cout << *it;
	}
	std::cout << colors[2];

}


void cf::logger::print(void) const {

	using namespace std;

	std::string_view logo = R"(

    _______   _______   ________   ______  ________   _______   _______
  ╱╱       ╲╱╱       ╲ ╱        ╲╱╱      ╲╱        ╲╱╱       ╲╱╱       ╲
 ╱╱        ╱╱      __╱_╱       ╱╱╱       ╱        _╱╱        ╱╱        ╱
╱       --╱        _╱╱         ╱        ╱╱       ╱╱        _╱        _╱
╲________╱╲_______╱  ╲╲_______╱╲________╱╲_____╱╱ ╲________╱╲____╱___╱

)";






	if (_messages.size() == 1 && _caret.empty()) { return; }

	write(STDOUT_FILENO, logo.data(), logo.size());

	std::string_view info = "press \x1b[32mup\x1b[0m/\x1b[32mdown\x1b[0m to navigate between errors.\n\n";

	write(STDOUT_FILENO, info.data(), info.size());

	/*if (_type == ERROR) {
		cout << _colors[0] << "----------------------------------------\n";
	}*/

	cout << /*_colors[0] <<*/ "    file: " << RESET << _file << "\n"
		 << /*_colors[0] <<*/ "position: " << RESET << _line << ", " << _column << "\n\n"
		 << /*_colors[0] <<*/ what[_type] << RESET << _messages[0] << "\n\n";

	cout << "\x1b[3m";
	for (size_type x = 1; x < _messages.size(); ++x) {
		std::cout << "          " << _messages[x] << "\n";
	}
	std::cout << "          " << "\x1b[32m" << _caret << "\x1b[0m\n";
	std::cout << "\n" << flush;
}



void cf::logger::add_caret(std::string& caret) {

	size_type x = _messages.back().size();

	_messages.back() = std::regex_replace(_messages.back(),
											std::regex("^ +"),
											"");
	size_type diff = x - _messages.back().size();

	// remove x first characters of _caret
	_caret = caret.c_str() + diff;

	return;
}

void cf::logger::add_more(std::string&& more) {
	_messages.emplace_back(std::move(more));
	//_messages.emplace_back(std::regex_replace(std::move(more), std::regex("^ +"), ""));
	// remove x first characters of _caret

}


// -- public accessors --------------------------------------------------------

/* get file path */
const std::string& cf::logger::path(void) const {
	return _path;
}

/* get file name */
const std::string& cf::logger::file(void) const {
	return _file;
}

/* get line number */
const std::string& cf::logger::line(void) const {
	return _line;
}

/* get column number */
const std::string& cf::logger::column(void) const {
	return _column;
}


// -- public modifiers --------------------------------------------------------










