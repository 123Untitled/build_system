#include "../inc/clang-filter.hpp"

// remap clang++ error messages to shorter and funnier ones
const cf::logger::mapppings cf::logger::_mapppings = {
	{"has been explicitly marked deleted here",
		"annihilated here"},

	{"call to deleted constructor of",
		"banned ctor of"},

	{"static assertion failed",
		"static flop"},

	{"candidate function has been explicitly deleted",
		"vanished function"},

	{"overload resolution selected deleted operator",
		"banned operator picked"},

	{"evaluated to false",
		"evaluated to nope !"},

	{"candidate function not viable",
		"function lost its mojo"},

	{"no known conversion from",
		"no magic for"},

	{"candidate template ignored",
		"template ghosted"},

	{"candidate function template not viable",
		"unworkable function template"},

	{"constraints not satisfied for class template",
		"failed template requirements"},

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
	"note",
	"warning",
	"error",
	"undefined"
};


cf::logger::logger(std::string&& file, std::string&& line, std::string&& column, std::string&& message)
:     _file{std::move(file)},
	  _line{std::move(line)},
	_column{std::move(column)},
	_messages{ } {

		// fill line and column with leading zeros (3 digits)
		size_type x = 4 - _line.size();
		_line.insert(0, x, '0');

		size_type y = 4 - _column.size();
		_column.insert(0, y, '0');


	if (std::regex_search(message, _rgx[NOTE])) {
		_type = NOTE;
	}
	else if (std::regex_search(message, _rgx[WARNING])) {
		_type = WARNING;
	}
	else if (std::regex_search(message, _rgx[ERROR])) {
		_type = ERROR;
	}
	else {
		_type = UNDEFINED;
	}

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
	static std::regex rgx2{"'(.*?)'"};
	if (std::regex_search(clean, _match, rgx2)) {
		clean = std::regex_replace(clean, rgx2, "\x1b[3;32m'$1'\x1b[0m");
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
		"\033[1;35m", // punctuation
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
	// no sync with stdio buffer
	ios::sync_with_stdio(false);
	// set full buffered stdout
	setvbuf(stdout, nullptr, _IOFBF, BUFSIZ);

	if (_type == ERROR) {
		cout << "----------------------------------------\n";
	}

	cout
	<< _colors[_type] << "               file: " << RESET << _file << "\n"
		<< "[" << _line << ":" << _column << "] "
		 //<< _colors[_type] << "line: " << RESET << _line << ", "
		 //<< _colors[_type] << "column: " << RESET << _column << "\n\n"
		 << _colors[_type] << "message: " << RESET << _messages[0] << "\n\n";


	std::cout << "\x1b[3m";
	for (size_type x = 1; x < _messages.size(); ++x) {
		if (_messages[x].empty()) { std::cout << "EMPTY\n"; continue;}
		std::cout << "       \x1b[32m>\x1b[0m " << _messages[x];
		//colorize(_messages[x]);
		std::cout << "\n";
		//cout << _colors[_type] << "      -> " << RESET << _messages[x] << "\n";
	}
	std::cout << "\x1b[0m\n\n\n" << flush;
}

void cf::logger::add_more(std::string&& more) {
	_messages.emplace_back(std::regex_replace(std::move(more), std::regex("^ +"), ""));
}


const char* type[] = {
	"candidate function not viable:",
	"in instantiation of function template specialization",
	"call to deleted constructor of",
};





