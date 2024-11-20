#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "record.h"
#include "id_query.h"

struct index_record {
  int64_t osm_id;
  const struct record *record;
};

struct indexed_data {
  struct index_record *irs;
  int n;
};

//function which given an array of records and that arrays length
//stores the length of the array and stores pointers to the records of the array indexed by their osm_ids 
struct indexed_data* mk_indexed(struct record* rs, int n) {
  //allocate memory for the indexed data struct
  struct indexed_data *data = (struct indexed_data*) malloc(sizeof(struct indexed_data));
  //return NULL if the allocation fails
  if (data == NULL) {
    return NULL;
  }

  //allocate memory for the record index
  struct index_record *irs = malloc(sizeof(struct index_record) * n);
  //return NULL and free the memory allocated to data if allocation fails
  if (irs == NULL) {
    free(data);
    return NULL;
  }

  //iterate through all the records and save their osm_id and address into the index_record array
  for (int i = 0; i < n; i++) {
    irs[i].osm_id = rs[i].osm_id;
    irs[i].record = &rs[i];
  }

  //save the index_record array in the indexed_data struct
  data -> irs = irs;
  //save the length of the record array
  data -> n = n;

  //return a pointer to the indexed_data struct
  return (data);
}

//free the memory allocated to the data structure
void free_indexed(struct indexed_data* data) {
  //free the memory allocated to the indexed records
  free(data->irs);
  //free the memory allocated to the indexed_data struct
  free(data);
}

//function which searches for a record in a naive_data struct which has a given osm_id 
const struct record* lookup_indexed(struct indexed_data *data, int64_t needle) {
  //return NULL if data doesn't exist
  if(data == NULL) {
    return NULL;
  }

  //iterate through the records in data->rs until othe one which osm_id matches needle
  //when found return a pointer to that record
  for (int i = 0; i < data -> n; i++) {
    if (data->irs[i].osm_id == needle) {
      return data -> irs[i].record;
    }
  }

  //if no record with a matching osm_id is found return NULL
  return NULL; 
}

int main(int argc, char** argv) {
  return id_query_loop(argc, argv,
                    (mk_index_fn)mk_indexed,
                    (free_index_fn)free_indexed,
                    (lookup_fn)lookup_indexed);
}
