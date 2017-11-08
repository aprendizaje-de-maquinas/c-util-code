
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <error.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>


/*
  Tests if the file stored in find is executable. Returns true if yes and false otherwise.
 */
bool exists(char * find){ 
  return access(find , X_OK)==0 ;
}

/*
  Parses the arguments passed to mywhich. Returns a pointer to the PATH user varraible or
  a pointer to the delmited list of paths after the -p flag.
  programIndex is an outparamter that stores the index in argv that the list of programs starts at.
 */
char * parseArgs(char *argv[], char *envp[] , int *programIndex){
  char *paths = NULL ;
    if(strcmp(argv[1],"-p")!=0){
    for(int i =0 ; envp[i]!=NULL ; i++){
      if(strncmp(envp[i],"PATH=",5)==0){
	paths = envp[i]+5 ;
	*programIndex =1 ;
	break ; // we found PATH, no need to 
      }
    }
  }else{
    paths = argv[2] ;
    *programIndex = 3 ;
  }
  if(paths == NULL) error(1,0,"PATH not found") ; // for safety in case paths is stll null (ex. path varriable set to null string)
    return paths ;
}

/*
  Takes in a file name (file) and a colon delimited list of paths (paths) and searches
  all those paths in an attempt to find file in the paths (where file must be executable).
 */
void findProgram(char * file , char*paths){
  char * pathsAlter = paths , path[PATH_MAX] , *colonIndex=NULL ;
  char * end = paths+strlen(paths) ;
  
  do { // loop through all paths
    colonIndex = strchr(pathsAlter,':');
    // if strcchr returns null, make colon index the last index of paths.
    if(colonIndex==NULL) colonIndex = end;
      
    memset(path , '\0' , PATH_MAX*sizeof(char)) ; // set memory to 0 to be safe with str ops
    memcpy(path , pathsAlter , (colonIndex-pathsAlter)*sizeof(char)); // copy the correct path to path

    sprintf(path+strlen(path) , "/%s",file) ; // add the file

    if(exists(path)){
      printf("%s\n",path);
      break ; // only find the first
    }

    pathsAlter = colonIndex+1 ; // skip the ':'

  }while(colonIndex!=end) ; // while colonIndex is not the last char in paths
  //NOTE we set colonIndex to the last char in paths when there is no colon left.
  return ;
}

/*
  Implementation of which. Takes in its arguments and environment varraiables and
  determines the paths to search for a given file. Then searches each of those paths for each
  file passed to mywhich.
 */
int main(int argc, char *argv[], char *envp[])
{
  int programIndex ;
  char *paths = parseArgs(argv , envp , &programIndex) ;

  for(int x = programIndex ; x < argc ; x++) findProgram(argv[x] , paths) ;
   
  return 0;
}

