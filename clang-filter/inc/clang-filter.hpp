#ifndef CLANG_FILTER_HEADER
#define CLANG_FILTER_HEADER

#include <unistd.h>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <regex>
#include <unordered_map>
#include <unordered_set>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#define COLOR1 "\x1b[32m"
#define COLOR2 "\x1b[33m"
#define COLOR3 "\x1b[31m"
#define RESET "\x1b[0m"

// -- C F  N A M E S P A C E --------------------------------------------------

namespace cf {



	enum {
		FILE_LINE_COLUMN_MESSAGE,
		CARET_WHERE_ERROR_OCCURED,
		ERRORS_GENERATED,
		INCLUDED_FILE,
		MAX
	};

	enum {
		NOTE,
		WARNING,
		ERROR,
		UNDEFINED,
		TYPE_MAX

	};


	class logger {

		public:

			// -- T Y P E S -------------------------------------------------------

			/* string type */
			using string = std::string;

			/* string vector type */
			using string_vector = std::vector<string>;

			/* size type */
			using size_type = string_vector::size_type;

			/* mapppings */
			using mapppings = std::unordered_map<string, string>;


			// -- C O N S T R U C T O R S -----------------------------------------

			/* deleted default constructor */
			logger(void) = delete;

			/* full path + */
			/* file + line + column + message constructor */
			logger(std::string&&,
					std::string&&, std::string&&, std::string&&, std::string&&);

			/* deleted copy constructor */
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
			void print(void) const;

			/* add more */
			void add_more(std::string&&);

			/* add caret */
			void add_caret(std::string&);


			// -- public accessors --------------------------------------------

			/* get file path */
			const string& path(void) const;

			/* get file name */
			const string& file(void) const;

			/* get line */
			const string& line(void) const;

			/* get column */
			const string& column(void) const;


			// -- public modifiers --------------------------------------------


			void colorize(const string&) const;

		private:

			// -- P R I V A T E  M E M B E R S --------------------------------

			/* full path */
			string        _path;

			/* filename */
			string        _file;

			/* position */
			string        _line, _column;

			/* message */
			string_vector _messages;

			/* caret */
			string        _caret;

			int _type;

			std::smatch _match;

			// -- P R I V A T E  S T A T I C  M E M B E R S -------------------

			/* regexes */
			static const std::regex _rgx[TYPE_MAX - 1];

			/* colors */
			static const char* _colors[TYPE_MAX];

			/* mapppings */
			static const mapppings _mapppings;

	};

}

#endif
