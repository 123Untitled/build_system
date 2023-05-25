#include <unistd.h>
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <regex>

// This program reads from stdin clang++ stderror and re-formats it to stdout

class logger {

	public:

		// -- T Y P E S -------------------------------------------------------

		/* string type */
		using string = std::string;

		/* string vector type */
		using string_vector = std::vector<string>;


		// -- C O N S T R U C T O R S -----------------------------------------

		/* default constructor */
		logger(std::string&& file, std::string&& line, std::string&& column, std::string&& message)
		: _file{std::move(file)}, _line{std::move(line)}, _column{std::move(column)}, _message{std::move(message)}, _rest{} {}

		/* delete copy constructor */
		logger(const logger&) = delete;

		/* move constructor */
		logger(logger&&) noexcept = default;

		/* destructor */
		~logger(void) {}


		// -- O P E R A T O R S -----------------------------------------------

		/* delete copy assignment */
		logger& operator=(const logger&) = delete;

		/* move assignment */
		logger& operator=(logger&&) noexcept = default;


		// -- M E T H O D S ---------------------------------------------------

		/* print contents */
		void print(void) {
			std::cout << "\x1b[32m   file:\x1b[0m " << _file << " | ";
			std::cout << "\x1b[32ml:\x1b[0m " << _line << ", ";
			std::cout << "\x1b[32mc:\x1b[0m " << _column << "\n";
			std::cout << "\x1b[32mmessage:\x1b[0m " << _message << std::endl;

			string_vector::iterator it = _rest.begin();
			for (; it != _rest.end(); ++it) {
				std::cout << "\x1b[32m      ->\x1b[0m " << *it << std::endl;
			}
			std::cout << std::endl << std::endl;
		}

		/* add more */
		void add_more(std::string&& more) {
			_rest.emplace_back(std::move(more));
		}

	private:
		string        _file;
		string        _line;
		string        _column;
		string        _message;
		string_vector _rest;

};


int main(void) {

	std::vector<logger> loggers;
	std::map<std::string, bool> occurred;
	//std::unordered_set<std::string> occurred2;

	std::string line;
	std::smatch match;

	while (true) {

		// read line
		std::getline(std::cin, line);

		//if (std::cin.fail()) { std::cout << "FAIL" << std::endl; return EXIT_FAILURE; }
		if (std::cin.eof())  { break;               }
		if (line.empty())    { continue;            }

		if (occurred.find(line) != occurred.end()) { continue; }
		occurred[line] = true;

		enum {
			FILE_LINE_COLUMN_MESSAGE,
			CARET_WHERE_ERROR_OCCURED,
			ERRORS_GENERATED,
			INCLUDED_FILE,
			MAX
		};


		std::regex rgx[MAX] = {
			std::regex("^.*\\/([^\\/]+):([0-9]+):([0-9]+): *(.*)$"), // file + line + column + message
			std::regex("^ *~* *\\^ *~* *$"), // caret showing where error occurred
			std::regex("^ *[0-9]+ errors? generated.$"), // number of errors
			std::regex("^In file included from") // included file
		};



		if (std::regex_search(line, match, rgx[CARET_WHERE_ERROR_OCCURED])) {
			// skip this line
			continue;
		}
		else if (std::regex_search(line, match, rgx[ERRORS_GENERATED])) {
			// skip this line
			continue;
		}
		else if (std::regex_search(line, match, rgx[INCLUDED_FILE])) {
			// skip this line
			continue;
		}
		else if (std::regex_search(line, match, rgx[FILE_LINE_COLUMN_MESSAGE])) {


			loggers.emplace_back(std::move(match[1]),
								 std::move(match[2]),
								 std::move(match[3]),
								 std::move(match[4]));

			//loggers.back().print();

		}
		else {
			//std::cout << "NON-PARSEABLE: " << line << std::endl;
			loggers.back().add_more(std::move(line));
		}

	}

	std::cout << "\n" << loggers.size() << " errors" << "\n" << std::endl;

	for (std::vector<logger>::iterator it = loggers.begin(); it != loggers.end(); ++it) {
		it->print();
	}

	return 0;

}

const char* type[] = {
	"candidate function not viable:",
	"in instantiation of function template specialization",
	"call to deleted constructor of",
};




