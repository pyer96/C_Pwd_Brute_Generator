/*###############################_Pier_Luigi_Manfrini_########################################
 *
 *	This program generates a file of brute-forced passwords given an
 *	alphabet and the desired length of the to be generated passwords. The task is
 *	assigned to the desired number of childre (default 2). A file "ranges_txt"
 *	will also be generated in which each worker prints the range of its computed
 *	passwords.
 *
 *	Usage:
 *		<./a.out> <min_length> <max_length> <filename[optional]> [OPTIONS]
 *
 *	OPTIONS:
 *	      	-p <num> Specifies the number of child processes
 *	      		to involve in the passwords computation.
 *	      		If omitted the default number of processes is 2.
 *
 *		-i <string> Let the user choose a character set (not space-separated)
 *		        of his choice from which the passwords will be genrated. If
 *			omitted, the default "dictionary" is alphanumeric(a-zA-Z0-9)
 *		
 *		--help 	pops usage infos on screen (stderr)
 *	
 *
 *	Notes: 	-filename : if omitted the default name for the password
 *			    file is "passwords_txt".
 *			    This does not affect the name of the log file ranges_txt
 *
 *		- The order of the CLI Arguments can be whatever the user likes
 *		as long as the order of <min_length>, <max_length> and <filename> doesn't
 *		change: namely letting <min_length> being always the first among the three and
 *		filename being always the last. The optional arguments -p <arg> and -i <arg>
 *		can be placed anywhere
 *
 *		- Since the huge amount of password combinations this program has 
 *		only been fully tested up to 5 letters passwords. 
 *		Behaviour not guaranteed for higher lengths.
 *
 *	Compile:
 *		gcc solution13.c -o solution13 -Wall -Werror -fsanitize=leak
 *
 * ###########################################################################################
 */

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define BLU "\033[1;34m"
#define YELLOW "\033[1;33m"
#define MAG "\033[1;35m"
#define mag "\033[0;35m"
#define red "\033[0;31m"
#define green "\033[0;32m"
#define yellow "\033[0;33m"
#define RESET "\033[0m"
#define und_red "\033[4;32m"

// 26 + 26 + 10 + \0
char alpha_numeric[62] =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
char hexadecimal[16] = "0123456789abcdef";

char num2char(int num);
void reverse_string(char *);
void fromDecimal(char[], int, int);
int exponential(int base, int exp);

char *used_dictionary = alpha_numeric;

int main(int argc, char **argv) {
  if (argc == 2 && (!strcmp(argv[1], "--help"))) {
    dprintf(
        2,
        "Usage: ./a.out <min_pwd_length> <max_pwd_length> [OPTIONS] [FILE]\n\n\
Generates a password file containing all passwords ranging between the min and max length!\n\n\
 Available OPTIONS:\n\
\t-p <num> \tSpecifies the number of child processes\n\
\t\t\tto involve in the passwords computation.\n\
\t\t\tIf omitted the default number of processes is 2.\n\n\
\t-i <string> \tLet the user choose a character set (not space-separated)\n\
\t\t\tof his choice from which the passwords will be generated.\n\
\t\t\tIf omitted the default \"dictionary\" is alphanumeric.\n\n\
 FILE: \t\t\tIs possible to define the output filename in which\n\
\t\t\tthe passwords will be stored, default is \"passwords_txt\"\n");
    exit(EXIT_SUCCESS);
  } else if (argc < 3) {
    dprintf(2, "Wrong Usage: <./a.out> <min_passw_length> <max_pasw_length> "
               "[OPTIONS]\n");
    exit(EXIT_FAILURE);
  }
  int num_children = 2;
  int min_length = 0;
  int max_length = 0;
  char out_name[30] = "passwords_txt";
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-p")) {
      if (argv[i + 1] != NULL)
        num_children = atoi(argv[i + 1]);
      i++;
    } else if (!strcmp(argv[i], "-i")) {
      if (argv[i + 1] != NULL)
        used_dictionary = argv[i + 1];
      i++;
    } else {
      if (!min_length)
        min_length = atoi(argv[i]);
      else if (!max_length)
        max_length = atoi(argv[i]);
      else
        strcpy(out_name, argv[i]);
    }
  }

  dprintf(1, MAG"\n>\t" RED "P" GREEN "A" BLU "S" YELLOW "S" RESET "W" RED
             "O" GREEN "R" YELLOW "D" BLU " G" yellow "E" green "N" GREEN
             "E" RED "R" BLU "A" MAG "T" YELLOW "O" RED "R" RESET "\n");
  dprintf(1, mag ">\t" red "Dictionary Set: \t" GREEN "%s\n", used_dictionary);
  dprintf(1, mag ">\t" red "Workers Involved: \t" GREEN "%d\n", num_children);
  dprintf(1,
          mag ">\t" red "Generating all PWD: \t" green "from" GREEN " %d" green
              " to " GREEN "%d" green " letters\n",
          min_length, max_length);
  dprintf(1, mag ">\t" red "Output PWD Filename: \t" GREEN "%s\n", out_name);
  dprintf(1, mag ">\t" red "Log file: \t\t" GREEN "ranges_txt\n");

  if (min_length < 0 || max_length < 0 || max_length < min_length) {
    dprintf(2, "Insert a valid range for the password length!\n");
    exit(EXIT_FAILURE);
  }
  int base;
  base = (int)strlen(used_dictionary); // strlen = 62
  int pwd_file;
  if ((pwd_file = open(out_name, O_CREAT | O_TRUNC | O_WRONLY,
                       S_IWUSR | S_IRUSR)) == -1)
    perror("");
  int ranges_file;
  if ((ranges_file = open("ranges_txt", O_CREAT | O_TRUNC | O_WRONLY,
                          S_IWUSR | S_IRUSR)) == -1)
    perror("");

  pid_t *pid = (pid_t *)malloc(num_children * sizeof(pid_t));
  for (int child = 0; child < num_children; child++) {
    switch (pid[child] = fork()) {
    case -1:
      perror("");
      exit(EXIT_FAILURE);
      break;
    case 0: { // Child

      for (int i = min_length; i <= max_length; i++) {
        char *pwd = (char *)malloc(i + 1 * sizeof(char));
        int range_from, range_to, total_combinations;
        total_combinations = exponential(base, i);
        if (child == 0)
          range_from = 0;
        else
          range_from = total_combinations / num_children * child;

        if (child == num_children - 1)
          range_to = total_combinations;
        else
          range_to = (total_combinations / num_children) +
                     child * total_combinations / num_children;
        char log[256];
        char from[16];
        char to[16];
        memset(from, used_dictionary[0], sizeof(from));
        memset(to, used_dictionary[0], sizeof(to));
        from[i] = '\0';
        to[i] = '\0';

        fromDecimal(from, base, range_from);
        fromDecimal(to, base, range_to - 1);
        sprintf(log,
                "%s%d-%s Child of %d\tMy range for %d character long passwords "
                "is: [ %s ~ %s ]\n",
                (child + 1) < 10 ? " " : "", child + 1,
                ((child + 1) == 1 || (child + 1) % 10 == 1)
                    ? "st"
                    : ((child + 1) == 2 || (child + 1) % 10 == 2)
                          ? "nd"
                          : ((child + 1) == 3 || (child + 1) % 10 == 3) ? "rd"
                                                                        : "th",
                num_children, i, from, to);
        write(ranges_file, log, strlen(log));
        for (int j = range_from; j < range_to; j++) {
          memset(pwd, used_dictionary[0], i + 1);
          pwd[i] = '\0';
          fromDecimal(pwd, base, j);
          pwd[i] = '\n';
          write(pwd_file, pwd, i + 1);
        }
        free(pwd);
      }
      exit(EXIT_SUCCESS);
      break;
    }
    default: // Father
      break;
    } // end switch
  }   // end for

  for (int i = 0; i < num_children; i++) {
    waitpid(pid[i], 0, 0);
  }
  dprintf(1, MAG ">\t" MAG "J" GREEN "O" BLU "B " YELLOW "D" RESET "O" RED "N" MAG
             "E" GREEN "!" RESET "\n");
  free(pid);
  close(pwd_file);
  close(ranges_file);
  // base 62 ~ pwd-legth = 4 --> total pwd # 62^4 = 14776336 --> p1 [0
  // 7388168-1]
  // 							     --> p2 [7388168
  // 14776335] 0 0 0 0 0 0 0 1 0 0 0  61    61*62^0 = 61     0 0 1   0  1*62^1 =
  // 62

  return 0;
}

void reverse_string(char *str) {
  size_t length = strlen(str);
  for (size_t i = 0; i < length / 2; i++) {
    char tmp = str[i];
    str[i] = str[length - i - 1];
    str[length - i - 1] = tmp;
  }
}

void fromDecimal(char *res, int base, int decimal_num) {
  int index = 0;
  while (decimal_num > 0) {
    res[index++] = num2char(decimal_num % base);
    decimal_num /= base;
  }
  reverse_string(res);
}

char num2char(int num) { return used_dictionary[num]; }

int exponential(int base, int exp) {
  if (exp != 0)
    return (base * exponential(base, exp - 1));
  else
    return 1;
}
