# C_Pwd_Brute_Generator
Multiprocess C Password generator: given a desired alphabet and password length. 

	This program generates a file of brute-forced passwords given an
	alphabet and the desired length of the to be generated passwords. The task is
	assigned to the desired number of childre (default 2). A file "ranges_txt"
	will also be generated in which each worker prints the range of its computed
	passwords.

	Usage:
		<./pwd> <min_length> <max_length> <filename[optional]> [OPTIONS]

	OPTIONS:
	      	-p <num> Specifies the number of child processes
	      		to involve in the passwords computation.
	      		If omitted the default number of processes is 2.

		-i <string> Let the user choose a character set (not space-separated)
		        of his choice from which the passwords will be genrated. If
			omitted, the default "dictionary" is alphanumeric(a-zA-Z0-9)
		
		--help 	pops usage infos on screen (stderr)
	

	Notes: 	-filename : if omitted the default name for the password
			    file is "passwords_txt".
			    This does not affect the name of the log file ranges_txt

		- The order of the CLI Arguments can be whatever the user likes
		as long as the order of <min_length>, <max_length> and <filename> doesn't
		change: namely letting <min_length> being always the first among the three and
		filename being always the last. The optional arguments -p <arg> and -i <arg>
		can be placed anywhere

		- Since the huge amount of password combinations this program has 
		only been fully tested up to 5 letters passwords. 
		Behaviour not guaranteed for higher lengths.

	Compile:
		gcc pwd.c -o pwd -Wall -Werror -fsanitize=leak
