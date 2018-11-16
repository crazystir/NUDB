#ifndef DAO_H
#define DAO_H

#include <mysql/mysql.h>

#define MAX_NUM_OF_FIELD 20 
#define FIELD_LEN 32
#define VALUE_LEN 64 
#define RES_VAL_LEN 50 

struct result_t {
    int num_of_fields_;
    int num_of_values_;
    char fields_[MAX_NUM_OF_FIELD][FIELD_LEN];
    char values_[MAX_NUM_OF_FIELD][RES_VAL_LEN][VALUE_LEN];
};


int DBInit();

void DBClose();

int SqlExec(const char *stmt, struct result_t *res);

int GetValuesByField(const struct result_t *res, const char *field, char ret[RES_VAL_LEN][VALUE_LEN]);

int PrintResult(const struct result_t *res);

int PrintResultVertical(const struct result_t *res);

int PrintResultRowVertical(const struct result_t *res, int row);

#endif
