#include "../inc/clang-filter.hpp"


enum {
	PARSING_ERROR,
	STDIN_ERROR,
};

constexpr const char* const errors[] = {
	"\x1b[32mparsing\x1b[0m error: message before file name\n",
	"\x1b[32mstdin\x1b[0m error: standard input failed\n",
};

// This program reads from stdin clang++ stderror and re-formats it to stdout


bool forward_stdin(void) {

	// avoid namespace pollution
	using namespace std;

	// line string buffer
	string line;

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
		// print line
		cout << line << endl;
	}
	return true;
}


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
				return forward_stdin();
			}
			files.back().append("\n").append(line);
		}
	}
	return true;
}



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



				loggers.emplace_back(std::move(match[0]),
									std::move(match[1]),
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

	if (loggers.empty()) {
		write(STDOUT_FILENO, "no compilation errors\n", 22);
		return;
	}

	const char* tty = ttyname(STDOUT_FILENO);
	const char* tid = ctermid(nullptr);

	std::cout << "tty: " << tty << std::endl;
	std::cout << "tid: " << tid << std::endl;

	// open original terminal stdin (not redirected) (macos only)
	int stdin_fd = open(tid, O_RDONLY);

	if (stdin_fd == -1) {
		write(STDOUT_FILENO, "ERROR: open\n", 12);
		return;
	}

	struct termios recover, raw;

	if (tcgetattr(stdin_fd, &recover) == -1) {
		write(STDOUT_FILENO, "ERROR: tcgetattr\n", 17);
		return;
	}

	raw = recover;

	// set input mode (non-canonical, no echo,...)
	raw.c_lflag &= ~(ECHO | ICANON);


	if (tcsetattr(stdin_fd, TCSANOW, &raw) == -1) {
		write(STDOUT_FILENO, "ERROR: tcsetattr\n", 17);
		return;
	}

	char buff[4];

	std::vector<cf::logger>::size_type x = loggers.size() - 1;

	// enter in alternate screen
	write(STDOUT_FILENO, "\033[?1049h", 8);
	// remove cursor
	write(STDOUT_FILENO, "\033[?25l", 6);
	// clear screen
	write(STDOUT_FILENO, "\033[2J", 4);
	// move cursor to top
	write(STDOUT_FILENO, "\033[H", 3);
	// print logger
	loggers[x].print();


	while (int readed = read(stdin_fd, buff, 4)) {
		// check for exit
		if      (readed == 1 && *buff == 'q') { break; }
		// check for 3 readed bytes
		else if (readed == 3 && buff[0] == 27 && buff[1] == 91) {
			// up
			if (buff[2] == 65) { x -= (x > 0); }
			// down
			else if (buff[2] == 66) { x += (x < (loggers.size() - 1)); }
			// else continue
			else { continue; }
			// clear screen
			write(STDOUT_FILENO, "\033[2J", 4);
			// move cursor to top
			write(STDOUT_FILENO, "\033[H", 3);
			// print logger
			loggers[x].print();
		}
		// check for enter key
		else if (readed == 1 && *buff == 10) {
			// launch vim
			std::string cmd = "nvim \"+call cursor(";
			cmd.append(loggers[x].line());
			cmd.append(",");
			cmd.append(loggers[x].column());
			cmd.append(")\" ");
			cmd.append(" ");
			cmd.append(loggers[x].path());
			// exit alternate screen
			write(STDOUT_FILENO, "\033[?1049l", 8);
			// restore cursor
			write(STDOUT_FILENO, "\033[?25h", 6);
			// reset terminal
			if (tcsetattr(stdin_fd, TCSANOW, &recover) == -1) {
				write(STDOUT_FILENO, "ERROR: tcsetattr\n", 17);
				return;
			}
			// launch vim
			system(cmd.c_str());
			// back to raw mode
			if (tcsetattr(stdin_fd, TCSANOW, &raw) == -1) {
				write(STDOUT_FILENO, "ERROR: tcsetattr\n", 17);
				return;
			}
			// enter in alternate screen
			write(STDOUT_FILENO, "\033[?1049h", 8);
			// remove cursor
			write(STDOUT_FILENO, "\033[?25l", 6);
			// clear screen
			write(STDOUT_FILENO, "\033[2J", 4);
			// move cursor to top
			write(STDOUT_FILENO, "\033[H", 3);
			// remove error entry
			//loggers.erase(loggers.begin() + x);
			// re set index
			//x -= (x > 0) && (x == loggers.size());
			// print logger
			loggers[x].print();
		}
	}

	// exit alternate screen
	write(STDOUT_FILENO, "\033[?1049l", 8);

	if (tcsetattr(stdin_fd, TCSANOW, &recover) == -1) {
		write(STDOUT_FILENO, "ERROR: tcsetattr\n", 17);
		return;
	}

	close(stdin_fd);
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





