#include "../inc/clang-filter.hpp"

#include <unordered_set>

enum {
	PARSING_ERROR,
	STDIN_ERROR,
};

constexpr const char* const errors[] = {
	"\x1b[32mparsing\x1b[0m error: message before file name\n",
	"\x1b[32mstdin\x1b[0m error: standard input failed\n",
};

// This program reads from stdin clang++ stderror and re-formats it to stdout

bool first_filter(std::vector<std::string>& files) {

	// avoid namespace pollution
	using namespace std;

	// line string buffer
	string line;

	// regexes
	regex rgx[] = {
		regex{"^In file included from"},
		regex{"^ *[0-9]+ errors? generated.$"},
		regex{"^fatal error: too many errors emitted, stopping now"},
		regex{"^.*:([0-9]+):([0-9]+): *(.*)$"},
	};

	// loop over lines
	while (true) {
		// read line
		getline(std::cin, line);
		// check for end of file
		if (cin.eof())  { break; }
		// check for error
		if (cin.fail()) {
			cout << errors[STDIN_ERROR] << flush;
			return false;
		}

		if      (regex_search(line, rgx[0])) { continue; }
		else if (regex_search(line, rgx[1])) { continue; }
		else if (regex_search(line, rgx[2])) { continue; }
		else if (regex_search(line, rgx[3])) {
			// new file detected
			files.emplace_back(std::move(line));
		}
		else {
			// check if there is a file
			if (files.empty()) {
				cout << errors[PARSING_ERROR] << flush;
				return false;
			}
			files.back().append("\n").append(line);
		}
	}
	return true;
}

#include <sstream>


void second_pass(const std::vector<std::string>& files) {

	std::vector<cf::logger> loggers;
	std::unordered_set<std::string> occurred;

	std::string line;
	std::smatch match;

	std::regex rgx[2] = {
		std::regex{"^.*\\/([^\\/]+):([0-9]+):([0-9]+): *(.*)$"},
		std::regex{"^ *~* *\\^ *~* *$"}
	};

	int i = 0;
	for (auto& file : files) {
		// check set for duplicates
		if (occurred.find(file) != occurred.end()) {
			continue;
		} // add to set
		occurred.insert(file);

		// split file into lines
		std::istringstream iss(file);
		while (std::getline(iss, line)) {


			if (std::regex_search(line, match, rgx[0])) {
				loggers.emplace_back(std::move(match[1]),
									std::move(match[2]),
									std::move(match[3]),
									std::move(match[4]));

			}
			else if (std::regex_search(line, rgx[1])) {
				loggers.back().add_caret(line);
			}
			else {
				loggers.back().add_more(std::move(line));
			}
		}

		++i;
	}

	std::cout << "size: " << i << std::endl;
	std::cout << "\n" << loggers.size() << " errors" << "\n" << std::endl;
	for (std::vector<cf::logger>::iterator it = loggers.begin(); it != loggers.end(); ++it) {
		it->print();
	}

}


int main(void) {

	// no sync with stdio buffer
	std::ios::sync_with_stdio(false);
	// set full buffered stdout
	std::setvbuf(stdout, nullptr, _IOFBF, BUFSIZ);

	std::vector<std::string> files;

	if (first_filter(files) != true) {
		std::cout << "ERROR: first filter failed" << std::endl;
		return EXIT_FAILURE; }

	second_pass(files);








	return 0;

}








/*
	while (true) {

		// read line
		std::getline(std::cin, line);

		if (std::cin.eof())  { break; }
		if (std::cin.fail()) { std::cout << "FAIL" << std::endl; return EXIT_FAILURE; }
		//std::cout << line << std::endl;



		if (occurred.find(line) != occurred.end()) { continue; }
		occurred.insert(line);


		if (std::regex_search(line, match, rgx[cf::CARET_WHERE_ERROR_OCCURED])) {
			// skip this line
			continue;
		}
		if (std::regex_search(line, match, rgx[cf::ERRORS_GENERATED])) {
			// skip this line
			continue;
		}
		else if (std::regex_search(line, match, rgx[cf::INCLUDED_FILE])) {
			// skip this line
			continue;
		}
		else if (std::regex_search(line, match, rgx[cf::FILE_LINE_COLUMN_MESSAGE])) {


			loggers.emplace_back(std::move(match[1]),
								 std::move(match[2]),
								 std::move(match[3]),
								 std::move(match[4]));

		}
		else {
			if (loggers.empty() == false) {
					loggers.back().add_more(std::move(line));
			}
			else {
				std::cout << "NON-PARSEABLE: " << line << std::endl;
			}
		}

	}*/

	//std::cout << "\n" << loggers.size() << " errors" << "\n" << std::endl;
	//for (std::vector<cf::logger>::iterator it = loggers.begin(); it != loggers.end(); ++it) {
	//	it->print();
	//}
