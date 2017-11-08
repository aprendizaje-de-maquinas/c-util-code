
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <assert.h>
#include <string.h>
#include <search.h> 

#define DEFAULT_CAPACITY 16


typedef void (*CleanupElemFn)(void *addr) ;

struct vec {
  char * elements ;  // byte array to hold the vector
  size_t nelems , elemsz , space; // various size paramters to make the pointer arithmatic possible
  CleanupElemFn cleanup ; // custom cleanup function in case the values in the vector are not standard non pointer types (ie int)
};

typedef struct vec CVector ;


/*
  Takes as input a size_t representing the size of the elements that will be in the vector,
  another size_t that represents the epexted size of the vector, and a function pointer to a
  custom lcenaup function for the elements of the vector.
  Returns a pointer to the created CVector.
 */
CVector *cvec_create(size_t elemsz, size_t capacity_hint, CleanupElemFn fn)
{
  if(elemsz ==0) assert("Allocation Failure") ;
  if(capacity_hint ==0) capacity_hint = DEFAULT_CAPACITY ;

  CVector *vec = calloc(sizeof(CVector), 1);
  if(vec == NULL) assert("Allocation failure") ;

  vec->elements = malloc(elemsz*capacity_hint) ;
  if(vec->elements==NULL) assert("Allocation failure") ;
  vec->nelems = 0 ;
  vec->elemsz = elemsz ;
  vec->cleanup = fn ;
  vec->space = capacity_hint ;

  return vec ;
}

/*
  Takes as input a CVector (cv) and handles the deallocation of all dynamically allocated memory
  calls the cusom cleanup function (cv->cleanup) if it is non null on all elements of the vector.
 */
void cvec_dispose(CVector *cv){
  if(cv->cleanup != NULL){
    for(size_t x = 0 ; x<cv->nelems ; x++){
      cv->cleanup(cv->elements+cv->elemsz*x) ;
    }
  }
  free(cv->elements) ;
  free(cv) ;
}

/*
  Takes as input a CVector (cv) and returns the number of elements in the vector. 
 */
int cvec_count(const CVector *cv){
  return (int)cv->nelems ;
}

/*
  Tkaes as input a CVector (cv) and an index (index) and returns a pointer to the value at that index.
  Asserts if the index is out of bounds.
 */
void *cvec_nth(const CVector *cv, int index){
  if(index < 0 || index > cv->nelems-1) assert("Invalid index") ;
  return (void*)(cv->elements+index*cv->elemsz) ;
}

/*
  Takes as input a CVector (cv) by reference and grows it by a factor of 2. to allow foor more elements to be added.
 */
void grow(CVector ** cv){
  char * temp = realloc((*cv)->elements , (*cv)->space*(*cv)->elemsz*2) ; // attempt to double the size in place
  if(temp == NULL) assert("Allocation failure") ; // catch any allocation failure.
  (*cv)->elements = temp ;
  (*cv)->space*=2 ;
}

/*
  Takes as input a CVector (cv), a void* which is a pointer to element to be added, and an index of where to add that element
  in the vector. leverages the memmove function to efficiently move the tail end of the vector over one element.
  Grows the vector by a factor of 2 if necessary.
  Asserts if the index is out of bounds.
 */
void cvec_insert(CVector *cv, const void *addr, int index){
  if(index < 0 || index > cv->nelems) assert("Invalid index") ;
  if(cv->space == cv->nelems)   grow(&cv) ;

  memmove(cv->elements+(index+1)*cv->elemsz , cv->elements+(index)*cv->elemsz , cv->elemsz*(cv->nelems-index)) ;

  memcpy(cv->elements+index*cv->elemsz , addr , cv->elemsz) ;

  cv->nelems++ ;
}

/*
  Takes as input a CVector (cv) and an element to add the vector (addr) and adds the element at the end of the vector
  Grows the size of the vector if necessary.
  If there is empty allocated space at the end of the vector, we still have room.
  NOTE that this implementation does not shrink the allocated size ever.
 */
void cvec_append(CVector *cv, const void *addr){
  if(cv->space == cv->nelems)  grow(&cv) ;

  memcpy(cv->elements+(cv->nelems)*cv->elemsz , addr , cv->elemsz) ;
  cv->nelems++ ;
}

/*
  Takes as input a CVector (cv), the address of an element to be put into the vector, 
  and the index of the element to replace.
  Asserts if the index is out of bounds.
  Calls the cusom cleanup function if necessay.
 */
void cvec_replace(CVector *cv, const void *addr, int index){
  if(index < 0 || index > cv->nelems-1) assert("Invalid index") ;
  if(cv->cleanup != NULL) cv->cleanup(cv->elements +index*cv->elemsz) ;
  memcpy(cv->elements+index*cv->elemsz , addr , cv->elemsz) ;
}  

/*
  Takes as input a CVector (cv) and an index (index) and removes the element at the specified index form the vector.
  Assrets if the index is out of bounds.
 */
void cvec_remove(CVector *cv, int index){ 
  if(index < 0 || index > cv->nelems-1) assert("Invalid index") ;
  if(cv->cleanup !=NULL) cv->cleanup(cv->elements+index *cv->elemsz) ;
  memset(cv->elements+cv->elemsz*index , 0 , cv->elemsz) ;
  memmove(cv->elements+cv->elemsz*index , cv->elements+cv->elemsz*(index+1) , cv->elemsz*(cv->nelems-index-1)) ; // shifts the tail of the elements down one element.

  cv->nelems-- ;
}

/*
  Takes as input a CVector (cv), a string (key), a comperator function (cmp), a start index (start), and boolean (sorted) if the vector is sorted
  Returns the index of the found element or -1 if the element was not found or was below the start index.
 */
int cvec_search(const CVector *cv, const void *key, CompareFn cmp, int start, bool sorted){
  if(start < 0 || start > cv->nelems-1) assert("Invalid start index") ;
  char * location = NULL ;
  size_t a = cv->nelems ;
  if(sorted){ // do binary search
    location = (char*)bsearch(key , cv->elements ,  cv->nelems , cv->elemsz , cmp) ;
  }else{ // linear search.
    location = (char*)lfind(key , cv->elements , &(a) , cv->elemsz , cmp) ;
  }
  if(location == NULL || (location-cv->elements)/cv->elemsz < start) return -1 ; // -1 if the element didn't exist or was below the start index.
  return (location-cv->elements)/cv->elemsz ;  // the index
}

/*
  Takes as input a CVector (cv) and compartor (cmp) and sorts the vector using the qsort function.
 */
void cvec_sort(CVector *cv, CompareFn cmp){ 
  qsort(cv->elements , cv->nelems , cv->elemsz , cmp) ;
}

/*
  Takes as  input a CVector (cv) and returns the base of the vector (ie the first element)
 */
void *cvec_first(const CVector *cv){
  return (void*)(cv->elements) ;
}

/*
  Takes as input a CVector (cv), and a pointer to the previous element of the vector
  Returns a pointer to the next element in the vector or NULL if prev was the last element.
 */
void *cvec_next(const CVector *cv, const void *prev){
  if(prev == cv->elements+(cv->nelems-1)*cv->elemsz) return NULL ;
  return (void*)((char*)prev+cv->elemsz) ;
}
