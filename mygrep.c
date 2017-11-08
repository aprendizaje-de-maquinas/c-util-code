
#include <error.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_LINE_LEN 1024

#define OPEN_EMPHASIS "\e[7m"
#define CLOSE_EMPHASIS "\e[0m"


/*
  Simple recursive greedy regular expression matching handling the metachars * . and ^.
  NOTE the metachar ^ is handled by the function "search" detailed below.

  endp is an outparamter holding the end of the regular expression match.
 */
bool regex_match(const char *input, const char *pattern, const char **endp)
{
    // when the entire pattern is matched, we are done.
    if (pattern[0] == '\0') {
        *endp = input;
        return true;
    }

    if(pattern[1]=='*'){

      if(input[0]==pattern[0] || (pattern[0]=='.' && input[0]!='\0')){
	// try a single copy of the char and recurse not advancing pattern (so as to allow for multiple copies of the char)
	if(regex_match(input+1 , pattern , endp)) return true ;

      }
      if(regex_match(input , pattern+2 , endp)) return true ; // try 0 copies this time.
      else return false ; // neither 0 nor many copies worked
    }

    // match a single char or the meta .
    if (input[0] == pattern[0] || (pattern[0]=='.' && input[0] !='\0')) {
      return regex_match(input + 1, pattern + 1, endp);
    }    

    // match failed
    return false;
}

/*
  Runs regular expression matching on the char* input according to the regex stored in pattern.
  Returns a * to the begining of the regex match or null if there is no match.
  **endp is an outparameter which keeps track of the end of the regex match if it exists.
  
  range is used to properly itterate over all possible positions that the regex match cold start at
  NOTE that if the metachar ^ is present, range is set to 0 as the only valid place for the match to start at is index 0.

  patternoffset is set to 1 if the pattern uses the metachar ^ to remove it from the pattern being serched for and is 0 otherwise.
 */
const char *search(const char *input, const char *pattern, const char **endp)
{
  int range = (pattern[0]=='^')? 0 : strlen(input);
  int patternoffset = (!range) ? 1 : 0;
  for(int i = 0 ; i <=range ; i++){
        if (regex_match(input + i, pattern+patternoffset, endp)) {
            return input + i;
        }
    }
    *endp = NULL;
    return NULL;
}

/*
  Prints out a string with the regex match highlighted.
 */
void print_with_emphasis(const char *cur, const char *start, const char *end)
{
    int nbefore = start - cur;
    int nmatched = end - start;
    printf("%.*s%s%.*s%s", nbefore, cur, OPEN_EMPHASIS,
        nmatched, start, CLOSE_EMPHASIS);
}

/*
  Runs a search on the given word to determine if it contains a match to the pattern.
  If it does contain a match prints it out with the filename present if mygrep was called with multiple files to be searched.
 */
void print_match(const char *line, const char *pattern, const char *filename)
{
    const char *end = NULL;
    const char *start = search(line, pattern, &end);

    if (start != NULL) {
        if (filename != NULL) printf("%s: ", filename);
        print_with_emphasis(line, start, end);
        printf("%s\n", end);
    }
}

/*
  Goes line by line through the file fp and trys to find a match to the regex pattern on that line.
  If a match is present, it prints the match out.
 */
void grep_file(FILE *fp, const char *pattern, const char *filename)
{
    char line[MAX_LINE_LEN];

    while (fgets(line, sizeof(line), fp) != NULL) {
        // truncate trailing newline if present
        // (most lines will have one, last line might not)
        if (line[strlen(line)-1] == '\n') line[strlen(line)-1] = '\0';
        print_match(line, pattern, filename);
    }
}

/*
  Parses the arguments passsed to mygrep and successively opens each of the search files, tests each line for
  a regex match and prints out that match if it exists.
 */
int main(int argc, char *argv[])
{
    if (argc < 2) error(1, 0, "Usage: mygrep PATTERN [FILE]...");
    const char *pattern = argv[1];

    if (argc == 2) {
        grep_file(stdin, pattern, NULL);
    } else {
        for (int i = 2; i < argc; i++) {
            FILE *fp = fopen(argv[i], "r");
            if (fp == NULL) error(1, 0, "%s: no such file", argv[i]);
            grep_file(fp, pattern, argc > 3 ? argv[i] : NULL);
            fclose(fp);
        }
    }
    return 0;
}

