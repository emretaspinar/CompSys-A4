#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "record.h"
#include "id_query.h"

struct naive_data {
  struct record *rs;
  int n;
};

//function which loads an array of records and the arrays lentgh into a naive_data struct
struct naive_data* mk_naive(struct record* rs, int n) {
  //allocate memory to the naive_data struct
  struct naive_data *data = (struct naive_data*) malloc(sizeof(struct naive_data));
  //if the allocation failed print an error message and return NULL
  if (data == NULL) {
    printf("Target not found\n");
    return NULL;
  }

  //save the address of beginning of the given record array 
  data -> rs = rs;
  //save the length of the record array
  data -> n = n;

  //return a pointer to the data structure
  return data;
}

//free the memory allocated to the data structure
void free_naive(struct naive_data* data) {
    free(data); 
}

//function which searches for a record in a naive_data struct which has a given osm_id 
const struct record* lookup_naive(struct naive_data *data, int64_t needle) {
  //return NULL if data doesn't exist
  if(data == NULL) {
    return NULL;
  }

  //iterate through the records in data->rs until othe one which osm_id matches needle
  //when found return a pointer to that record 
  for (int i = 0; i < data -> n; i++) {
    if (data->rs[i].osm_id == needle) {
      return &data->rs[i];
    }
  }

  //if no record with a matching osm id is found return NULL
  return NULL; 
}

int main(int argc, char** argv) {
  return id_query_loop(argc, argv,
                    (mk_index_fn)mk_naive,
                    (free_index_fn)free_naive,
                    (lookup_fn)lookup_naive);
}
