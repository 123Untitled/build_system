#!/bin/zsh

# This script is used to compile the project.
# Makefile forever, but not really lol.

version='3'



# -- C O L O R  S E T T I N G S -----------------------------------------------

# escape
esc='\x1b'

# main color
 color=$esc'[32m'
	C1=$esc'[32m'
color2=$esc'[33m'
 error=$esc'[31m'
 reset=$esc'[0m'
     R=$esc'[0m'
 erase=$esc'[1F'$esc'[0J'


yes=0
no=1


# -- S C R I P T  P A T H  S E T T I N G S ------------------------------------

# get absolute directory of script
abspath=$(cd ${0%/*}; pwd)

# get dirname of script
rootname=${abspath##*/}

# get absolute path of script
script=$abspath/${0##*/}

# basename of the script without using basename command
scriptname=${0##*/}






# -- O P E R A T I N G  S Y S T E M -------------------------------------------

# check if operating system is supported
function check_os {

	# check for macosx
	if   [[ $OSTYPE == 'darwin'* ]]; then
		dlib='-dynamiclib'
		dext='dylib'

	# check for linux
	elif [[ $OSTYPE == 'linux-gnu'* ]]; then
		dlib='-shared'
		dext='so'

	# check for other
	else
		echo 'Unsupported OS:' $color$OSTYPE$reset
		exit 1
	fi
}

check_os





function required {
	# required programs
	local commands=('clang++' 'ar'
					'mkdir'
					'rm' 'cd' 'pwd'
					'sed' 'cat' 'file'
					'vared')
	# loop through all required commands
	for cmd in $commands; do
		# check if command is installed
		if [[ -z "$(command -v $cmd)" ]]; then
			# print error message
			echo $error'Error:'$reset 'required program' \
				$color''$cmd''$reset 'is not installed.'
			# exit script
			exit 1
		fi
	done
}


required




# check if all required commands are installed
#for cmd in $required_commands; do
#	check_command $cmd
#done

# .env file
env=$abspath/'.env'

# check for .env file
if [[ ! -f $env ]]; then
	# create .env file
	echo 'Create a' $color'.env'$reset 'file'
	echo 'BUILD_MODE=' > $env
fi

# load .env file
source $env

if [[ -z $BUILD_MODE ]]; then
	echo '\n🥕 Set a build mode in' $color'.env'$reset 'file [active, debug, release]'

	while (true); do
		vared -c  -p '   change it now: ' -c build_mode;
		if [[ $build_mode == 'active' || $build_mode == 'debug' || $build_mode == 'release' ]]; then
			break;
		else
			echo 'invalid build mode'
		fi
	done

	sed -i '' 's/BUILD_MODE=.*/BUILD_MODE='$build_mode'/g' $env
	echo "🍌 Build mode changed to '$build_mode'."
	exit 0
fi



# -- D I R E C T O R I E S ----------------------------------------------------

# source directory
srcdir=$abspath/'src'

# include directory
incdir=$abspath/'inc'

# build directory
blddir=$abspath/'_bld'

# object directory
objdir=$blddir/'_obj'

# dependency directory
depdir=$blddir/'_dep'

# json directory
jsndir=$blddir/'_jsn'

# compile log directory
cmpdir=$blddir/'compile'

# cache directory
cachedir=$abspath/'.cache'


# check if srcdir exists
if [[ ! -d $srcdir ]]; then
	echo 'No' $C1$rootname'/'${srcdir##*/}$R 'directory found.';
	exit 1
fi

# check if incdir exists
if [[ ! -d $incdir ]]; then
	echo 'No' $C1$rootname'/'${incdir##*/}$R 'directory found.';
	exit 1
fi



# -- S O U R C E  F I L E S ---------------------------------------------------


# get source files
src=($srcdir/**/*.'cpp'(.N))

# check for any source file ?
if [[ ${#src[@]} -eq 0 ]]; then
	# get base name of srcdir + parent directory
	local folder=$rootname/${srcdir##*/}
	echo 'No source files found in' $color$folder$reset 'directory.'
	exit 1
fi


# get all directories hierarchy in incdir
inc=($incdir/**/*(/N) $incdir)

# insert -I prefix to each directory
inc=("${inc[@]/#/-I}")



# get arguments from command line
args=(${@:2})

# target
target=$1

# object array
obj=()

# json array
jsn=()


# -- T A R G E T S ------------------------------------------------------------

# project name
project='xns'

# target executable
executable=$abspath/$project'exec'

# target dynamic library
dynamic=$abspath/'lib'$project.$dext

# target static library
static=$abspath/'lib'$project.'a'

# compilation database
compdb=$abspath/'compile_commands.json'

# log file
logfile=$abspath/'compile.log'


# -- C O M P I L E R  S E T T I N G S -----------------------------------------

# compiler
#cxx='clang++'
cxx='/opt/homebrew/Cellar/llvm/16.0.4/bin/clang++'

# archiver
archiver='ar'

# archiver flags
arflags='-rcs'

# linker
linker=$cxx

# standard
std='-std=gnu++2b'

# optimization
opt='-O0'

# debug
debug='-g3'

# cxx flags
cxxflags=('-Wall' '-Wextra' '-Werror' '-Wpedantic'\
		'-Wno-unused' '-Wno-unused-variable' '-Wno-unused-parameter'\
		'-Winline' '-fno-exceptions' '-Weffc++'\
		'-Wno-gnu-string-literal-operator-template')

# defines
defines=()


[[ $BUILD_MODE == "debug" ]] && defines+=('-DXNS_UT' '-DMETA_UT')
[[ $BUILD_MODE == 'development' ]] && defines+=('-DDEVELOPMENT')
[[ $BUILD_MODE == 'release' ]] && defines+=('-DRELEASE')


# linker flags
ldflags=''


# memory checker
memcheck='valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes'

# memory checker flags
memflags=''




# -- I M P L E M E N T A T I O N ----------------------------------------------

function descript {
	local cible=$1
	local resume=$(file $cible)

	local regex='.*: (.*)'
	[[ $resume =~ $regex ]] && \
		echo $color'[+]'$reset ${cible##*/} '|' ${match[1]}'\n'
}


function make_clean {
	# remove build directory
	rm -rf $blddir
	# print cleaned message
	echo $color'[x]'$reset "Cleaned\n";
}

function make_fclean {
	# remove build and cache directory
	rm -rf $blddir $cachedir
	# remove all targets
	rm -f $executable $dynamic $static $compdb $env $logfile
	# print full cleaned message
	echo $color'[x]'$reset "Full cleaned\n";
}

function make_executable {
	# link executable
	$linker $obj -o $executable $ldflags || exit 1
	# check if linking succeeded
	descript $executable
}

function make_dynamic {
	# link dynamic library
	$linker $obj $dlib -o $dynamic $ldflags || exit 1
	# check if linking succeeded
	descript $dynamic
}

function make_static {
	# link static library
	$archiver $arflags $static $obj || exit 1
	# check if linking succeeded
	descript $static
}

function make_compdb {
	echo "[\n"$(cat $jsn | sed '$s/,\s*$//')"\n]" | jq > $compdb
	# check if compilation database succeeded
	descript $compdb
}

function get_objects {
	# get all object files
	obj=($objdir/**/*.'o'(.N))
}

function get_jsons {
	# get all json files
	jsn=($jsndir/**/*.'json'(.N))
}


# -- U T I L I T I E S  F U N C T I O N S -------------------------------------

function is_missing {
	# get target file
	local file=$1
	# file exists ?
	[[ ! -e $file ]] && return $yes || return $no
}

function dir_exists {
	# get target directory
	local dir=$1
	# directory exists ?
	[[ -d $dir ]] && return $yes || return $no
}

function is_there_any_files {
	# get all arguments
	local files=($@)
	# is there any files ?
	[[ ${#files[@]} -gt 0 ]] && return $yes || return $no
}


function is_link_required {
	# get first argument
	local target=$1
	# get all other arguments
	local required=(${@:2})
	# loop over object files
	for file in $required; do
		# check if this script is modified
		[[ $file -nt $target ]] && return $yes
	done
	return $no

}




function check_dependency {
	# get arguments
	local obj=$1
	local dep=$2
	# check if object or dependency file is missing
	if is_missing $dep && return $yes
	if is_missing $obj && return $yes
	# get file content
	local content=$(<$dep)
	# split content into array
	local array=( ${=content} )
	# loop over array
	for x in "${array[@]}"; do
		# ignore '\' and file name ending with ':'
		if ! [[ $x == "\\" ]] && ! [[ $x == *":" ]]; then
			# check if dependency is modified
			[[ $x -nt $obj ]] && return $yes
		fi
	done
	return $no
}


function make_compile {

	local file=$1
	local  obj=$2
	local  dep=$3
	local  jsn=$4
	local  cmp=$5

	# big compilation line !!!
	$cxx $std $opt $debug $cxxflags $defines -MJ $jsn -MT $obj -MMD -MF $dep $inc -c $file -o $obj 2> $cmp

	# check if compilation failed
	if [[ $? -ne 0 ]]; then
		echo -n $error'[x]'$reset
		exit 1
	fi
	echo -n $color'[✓]'$reset
	exit 0
}

# generate separator
separator='\x1b[90m'
# loop over columns
for x in $(seq $COLUMNS); do
	# add separator
	separator+='─'
done # reset color
separator+=$reset


function error_colorizer {

	local logs=($cmpdir/**/*.'log'(.N))

	local buffer=''
	local final=()
	# associative array
	declare -A hashers

	# loop over log files
	for log in $logs; do
		# skip empty log files
		if [[ ! -s $log ]]; then; continue; fi


		#echo '\n\n'$log | tr '[:lower:]' '[:upper:]'

		local file=(${(f)"$(<$log)"})

		for line in $file; do

			# regex pattern to match: "In file included from ..."
			if  [[ $line =~ "^In file included from" ]]      || \
				[[ $line =~ "^[0-9]+ errors? generated.$" ]] || \
				[[ $line =~ "^ *~*\^~* *$" ]]; then; continue; fi

			#echo $color'>'$reset$line;

			# split line with white spaces
			local words=(${=line})

			for word in $words; do

				local add=''
				#echo $color'w'$reset$word;
				# check if word match pattern
				if [[ $word =~ "^(.*):([0-9]+):([0-9]+):" ]]; then

					#echo $buffer
					local flag=0
					# check if buffer is not empty
					if [[ -n $buffer ]]; then
						# check if final is not empty
						if [[ ${#final[@]} -gt 0 ]]; then
							# loop over final array
							for b in ${final[@]}; do
								# check if buffer is not in final
								if [[ "$b" == "$buffer" ]]; then
									# set flag
									flag="1"
									echo "o"
									break
								fi
							done
							if [[ $flag == "0" ]]; then
								# add buffer to final
								final+=($buffer)
								# clear buffer
								buffer=''
							else
								flag=0
							fi
						else
							# add buffer to final
							final+=($buffer)
							# clear buffer
							buffer=''
						fi
					fi



					# check if buffer is not in hashers (here invalid subscript, why ?)
					#if [[ -n $buffer ]] && [[ hashers[$buffer] -ne 1 ]]; then
					#	# add buffer to hashers
					#	hashers[$buffer]=1
					#	echo $buffer
					#	final+=($buffer)
					#fi

					# get file name
					local src_file=${match[1]##*/}
					# get line number
					local y=${match[2]}
					# get column number
					local x=${match[3]}
					#echo -n $src_file $y $x' '
					buffer+=$src_file' '$y' '$x' '

				# match for "note:" or "warning:" or "error:"
				elif [[ $word =~ "^(note:|warning:|error:)$" ]]; then
					# remove ':' from word
					word=${word%?}
					#echo -n $word '> '
					buffer+=$word' > '
				else
					# print word
					#echo -n $word' '
					buffer+=$word' '
				fi

			done # end of word
			buffer+="\n"

			#echo;
		done # end of line
		#echo $separator
	done # end of log

	for x in $final; do
		echo '>>>\n'$x
	done
}

#function error_colorizer {
#	# check log file
#	if [[ ! -e $logfile ]]; then
#		# print error message
#		echo $error':('$reset 'log file is gone somewhere...'
#		exit 1
#	fi
#
#	# fill an array of lines
#	local lines=(${(f)"$(<$logfile)"})
#
#	# loop over lines
#	for line in $lines; do
#
#		# regex pattern to match: "In file included from ..."
#		if [[ $line =~ "^In file included from" ]]; then
#			continue
#		fi
#		# regex pattern to match: "x errors generated."
#		if [[ $line =~ "^[0-9]+ errors? generated.$" ]]; then
#			continue
#		fi
#		# match symbol '^~~~~~'
#		if [[ $line =~ "^ *~*\^~* *$" ]]; then
#			continue
#		fi
#
#		# white space split
#		local words=(${=line})
#		# loop over words
#		for word in "${words[@]}"; do
#
#			# regex pattern
#			local location="^(.*):([0-9]+):([0-9]+):"
#			# check if word match pattern
#			if [[ $word =~ $location ]]; then
#
#				echo $separator
#				# get file name
#				local file=${match[1]##*/}
#				# get line number
#				local y=${match[2]}
#				# get column number
#				local x=${match[3]}
#
#				# print file name
#				#echo -n '['$color$file$reset']' '['$color$y$reset'] ['$color$x$reset'] '
#				echo -n '\n''['$file']' '['$y']' '['$x'] ' >> error.log
#
#
#			# match for "note:" or "warning:" or "error:"
#			local hint="^(note:|warning:|error:)$"
#			#elif [[ $word =~ $pattern ]]; then
#			elif [[ $word =~ $hint ]]; then
#				# print error
#				#echo -n $error$word$reset' '
#				echo -n $word' ' >> error.log
#			else
#				# print word
#				#echo -n $word' '
#				echo -n $word' ' >> error.log
#			fi
#		done
#	done
#}



function compile {

	# create build directories
	mkdir -p $blddir $objdir $depdir $jsndir $cmpdir
	# declare compiled files counter
	local -i compiled=0
	# array of pids
	pids=()

	# loop over source files
	for file in $src; do
		# get file name without path and extension
		local base=${${file##*/}%.*}
		# add object file extension
		local obj=$objdir/$base'.o'
		# add dependency file extension
		local dep=$depdir/$base'.d'
		# add json file extension
		local jsn=$jsndir/$base'.json'
		# add compiler log file extension
		local cmp=$cmpdir/$base'.log'


		# check if source file is modified
		if check_dependency $obj $dep || [[ $script -nt $obj ]]; then

			# compiler version
			if [[ $compiled -eq 0 ]]; then
				echo $separator; $cxx --version; echo $separator
			fi
			# compile source file
			(make_compile $file $obj $dep $jsn $cmp) &
			# add pid to array
			pids+=($!)
			# increment compiled files counter
			((++compiled))

		fi
	done

	# loop over pids
	for pid in "${pids[@]}"; do
		# wait for pid
		wait $pid
		# check if compilation failed
		if [[ $? -ne 0 ]]; then
			#pkill -P $$ # kill all child processes
			wait
			echo "\n\n🔥 compilation failed."
			./filter < $cmpdir/*.log
			#cat $cmpdir/*.log
			#bat $cmpdir/*.log
			#error_colorizer
			exit 1
		fi
	done

	if [[ $compiled -eq 0 ]]; then
		echo $separator
		echo $color'[✓]'$reset "Nothing to compile."
	else
		echo '\n\n'🫠 $compiled "files compiled."
	fi

	# print separator
	echo $separator

}








function database {

	get_jsons

	is_there_any_files $jsn || exit 1

	# handle compilation database
	if is_missing $compdb || is_link_required $compdb $jsn; then
		make_compdb
	fi

}


function linkage {

	local final=$1
	local magic=$2

	get_objects

	is_there_any_files $obj || exit 1

	# handle target
	if is_missing $final || is_link_required $final $obj; then
		$magic # !!!
	fi

}

function header {

	local thing=${1##*/}
	# -- B A N N E R --------------------------------------------------------------

	echo $color;
	echo		"   ▁▁▁▁▁▁▁▁  ▁▁▁▁▁▁▁▁  ▁▁▁▁ ▁▁▁  ▁▁▁▁▁▁▁▁ ";
	echo		"  ╱        ╲╱        ╲╱    ╱   ╲╱        ╲";
	echo		" ╱         ╱         ╱         ╱         ╱";
	echo		"╱         ╱         ╱        ▁╱       ▁▁╱ ";
	echo		"╲▁▁╱▁▁╱▁▁╱╲▁▁▁╱▁▁▁▁╱╲▁▁▁▁╱▁▁▁╱╲▁▁▁▁▁▁▁▁╱\n";
	echo $color"$scriptname"$reset 'launching' '['$color$thing$reset']' 'build'
}


function change_version {
	# remove dots
	update=${version//./}
	# convert to decimal
	update=$((16#$update))
	# increment
	update=$(($update+1))
	# convert to hex
	update=$(printf '%x' $update)
	# reset version
	version=''
	# loop through all digits to insert dots
	for (( i=0; i < ${#update}; ++i )); do
		version+=${update:$i:1}'.'
	done
	# remove last dot
	version=${version%?}
	# replace version line in this script
	sed -i '' "s/^version='[0-9A-Fa-f.]*[0-9A-Fa-f]'/version='$version'/g" $script
	echo '\nVersion changed to' $color$version$reset'\n'
	return 0
}


function commit {
	echo;
	# repository
	local repo='git@github.com:123Untitled/-build-system-.git'
	# directory
	local gitdir='build-system'
	# check repository folder existance
	if [[ -d $gitdir ]]; then
		echo $color''$gitdir''$reset' repository already exists.'
		exit 1
	fi
	# clone repository
	git clone $repo $gitdir
	# check if repository was cloned
	if [[ $? -ne 0 ]]; then
		echo $color''$gitdir''$reset' repository cloning failed.'
		exit 1
	fi
	# change version
	change_version && \
	# move into repo
	cd $gitdir && \
	# copy script to repository
	cp '..'/$scriptname '.' && \
	# add script to repository
	git add $scriptname && \
	# get commit message
	vared -p 'commit message: ' -c commit_msg && \
	# new line
	echo && \
	# commit and push script to repository
	git commit -m "$commit_msg" && git push
	# check if commit was successful
	if [[ $? -ne 0 ]]; then
		echo $color''$scriptname''$reset' script commit failed.'
		exit 1
	fi
	# move back
	cd ..
	# remove repository
	rm -rf $gitdir
	# print success message
	echo '\n'$color''$scriptname''$reset' script pushed to '$color''$gitdir''$reset' repository.\n'
}



function main {



	check_os
	required
	#init variable
	#required
	if [[ $target == 'commit' ]]; then
		header "commit"
		commit
		exit 0
	elif [[ $target == 'run' ]]; then
		header "run"
		compile
		database
		linkage $executable make_executable
		$executable
		exit 0

	elif [[ $target == 'clean' ]]; then
		header "clean"
		make_clean

	elif [[ $target == 'fclean' ]]; then
		header "fclean"
		make_fclean

	elif [[ $target == 're' ]]; then
		header "re"
		make_clean
		compile
		database
		linkage $executable make_executable

	elif [[ $target == 'exec' ]] || [[ -z $target ]]; then
		header $executable
		compile
		database
		linkage $executable make_executable

	elif [[ $target == 'dynamic' ]]; then
		header $dynamic
		compile
		database
		linkage $dynamic make_dynamic

	elif [[ $target == 'static' ]]; then
		header $static
		compile
		database
		linkage $static make_static

	else
		header "???"
		echo $separator
		echo 'Unknown target:' $color$target$reset
		exit 1
	fi

	echo '💫' "[$BUILD_MODE]" 'All targets are up to date !';

}


main






exit 0


