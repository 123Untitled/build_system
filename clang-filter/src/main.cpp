#include "../inc/clang-filter.hpp"

#include <unordered_set>

// This program reads from stdin clang++ stderror and re-formats it to stdout

int main(void) {

	std::vector<cf::logger> loggers;
	std::unordered_set<std::string> occurred;

	std::string line;
	std::smatch match;

	std::regex rgx[cf::MAX] = {
		std::regex("^.*\\/([^\\/]+):([0-9]+):([0-9]+): *(.*)$"), // file + line + column + message
		std::regex("^ *~* *\\^ *~* *$"), // caret showing where error occurred
		std::regex("^ *[0-9]+ errors? generated.$"), // number of errors
		std::regex("^In file included from") // included file
	};

	while (true) {

		// read line
		std::getline(std::cin, line);

		if (std::cin.eof())  { break; }
		if (std::cin.fail()) { std::cout << "FAIL" << std::endl; return EXIT_FAILURE; }

		if (occurred.find(line) != occurred.end()) { continue; }
		occurred.insert(line);






		if (std::regex_search(line, match, rgx[cf::CARET_WHERE_ERROR_OCCURED])) {
			// skip this line
			continue;
		}
		else if (std::regex_search(line, match, rgx[cf::ERRORS_GENERATED])) {
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

	}

	std::cout << "\n" << loggers.size() << " errors" << "\n" << std::endl;

	for (std::vector<cf::logger>::iterator it = loggers.begin(); it != loggers.end(); ++it) {
		it->print();
	}

	return 0;

}
