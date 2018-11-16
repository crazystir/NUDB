#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>

#include "dao.h" 
#include "types.h"
#include "common.h"

#define STMT_LEN 2048 

char db_username[LEN] = "root";
char db_passwd[LEN] = "123";
char db[LEN] = "project3-nudb";

static MYSQL *conn;

static void GetDBInfo();
static void Retrieve(struct result_t* ret, MYSQL_RES *res); 

int DBInit() {
    GetDBInfo();

    if ((conn = mysql_init(NULL)) == NULL) {
        fprintf(stderr, "Could not init DB\n");
        return -1;
    } 

    if (!mysql_real_connect(conn, "localhost", db_username, db_passwd, db, 0, NULL, 0)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return -1;
    }

    SqlExec("drop trigger if exists withdraw_warning;", NULL);
    SqlExec("drop table if exists warning;", NULL);
    SqlExec("drop procedure if exists avaliable_courses;", NULL);
    SqlExec("drop procedure if exists constraint_courses;", NULL);
    SqlExec("drop procedure if exists enroll;", NULL);
    SqlExec("drop procedure if exists withdraw;", NULL);
    SqlExec("create table warning (id int(3) primary key, flag int(3));", NULL); 
    SqlExec("insert into warning values(1, 0)", NULL);
    SqlExec("create trigger withdraw_warning after update on uosoffering for each row begin if (select Enrollment from uosoffering where UoSCode = OLD.UoSCode and Semester = OLD.Semester and Year = OLD.Year) * 2 < (select MaxEnrollment from uosoffering where UoSCode = OLD.UoSCode and Semester = OLD.Semester and Year = OLD.Year) then update warning set flag = 1; else update warning set flag = 0; end if; end;", NULL);
    SqlExec("create procedure avaliable_courses(in studentId int(11), in semester1 char(2), in year1 int(11), in semester2 char(2), in year2 int(11)) begin select * from uosoffering natural join unitofstudy where ((Semester = semester1 and Year = year1) or (Semester = semester2 and Year = year2)) and Enrollment < MaxEnrollment and UoSCode not in (select UoSCode from transcript where StudId = studentId and (Grade is not NULL or (Semester = semester1 and Year = year1) or (Semester = semester2 and Year = year2))) and UoSCode not in (select requires.UoSCode from requires left outer join (select UoSCode, Grade from transcript where StudId = studentId) as T on PrereqUoSCode = T.UoSCode where T.Grade is NULL); end;", NULL);
    SqlExec("create procedure constraint_courses(in studentId int(11), in semester1 char(2), in year1 int(11), in semester2 char(2), in year2 int(11)) begin select * from uosoffering natural join unitofstudy natural join requires where ((Semester = semester1 and Year = year1) or (Semester = semester2 and Year = year2)) and Enrollment < MaxEnrollment and UoSCode not in (select UoSCode from transcript where StudId = studentId and (Grade is not NULL or (Semester = semester1 and Year = year1) or (Semester = semester2 and Year = year2))) and PrereqUoSCode not in (select UoSCode from transcript where StudId = studentId and Grade is not NULL) order by UoSCode;  end;", NULL);
    SqlExec("create procedure enroll(in studentId int(11), in courseId char(8), in quarter char(2), in date int(11))  begin insert into transcript values(studentId, courseId, quarter, date, NULL); update uosoffering set Enrollment = Enrollment + 1 where UoSCode = courseId and Semester = quarter and Year = date; end;", NULL);
    SqlExec("create procedure withdraw(in studentId int(11), in courseId char(8), in quarter char (2), in date int(11)) begin delete from transcript where StudId = studentId and UoSCode = courseId and Semester = quarter and Year = date; update uosoffering set Enrollment = Enrollment - 1 where UoSCode = courseId and Semester = quarter and Year = date; end;", NULL);
    return 0;
}

void DBClose() {
    mysql_close(conn);
}

int SqlExec(const char *stmt, struct result_t *ret) {
    // All statement is wrapped with transaction
    if (mysql_query(conn, "start transaction") < 0) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return -1;
    }
    if (mysql_query(conn, stmt)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
    }
    if (mysql_query(conn, "commit") < 0) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return -1;
    }

    if (ret) {
        MYSQL_RES *res = mysql_store_result(conn);
        Retrieve(ret, res); 
        mysql_free_result(res);
        mysql_next_result(conn);
    }
    return 0;
}

int GetValuesByField(const struct result_t *res, const char *field, char ret[RES_VAL_LEN][VALUE_LEN]) {
    int i, j;

    for (i = 0; i < res -> num_of_fields_; i++) {
        if (strcmp(res -> fields_[i], field) == 0) {
            for (j = 0; j < res -> num_of_values_; j++) {
                strncpy(ret[j], res -> values_[i][j], VALUE_LEN); 
            } 
            return 0;
        }
    }
    return -1;
}


int PrintResult(const struct result_t *res) {
    int i, j;

    for (i = 0; i < res -> num_of_fields_; i++) {
        printf("%s ", res  -> fields_[i]);
    }
    printf("\n");

    for (j = 0; j < res -> num_of_values_; j++) {
        for (i = 0; i < res -> num_of_fields_; i++) {
            if (!res -> values_[i][j]) {
                printf("NULL ");
            } else {
                printf("%s ", res -> values_[i][j]);
            }
        }
        printf("\n");
    } 
    return 0;
}

int PrintResultVertical(const struct result_t *res) {
    int i, j;

    for (i = 0; i < res -> num_of_fields_; i++) {
        printf("%s: ", res -> fields_[i]);
        for (j = 0; j < res -> num_of_values_; j++) {
            printf("%s ", res -> values_[i][j]);
        }
        printf("\n");
    }
    return 0;
}

int PrintResultRowVertical(const struct result_t *res, int row) {
    int i;

    for (i = 0; i < res -> num_of_fields_; i++) {
        printf("%s: ", res -> fields_[i]);
        printf("%s ", res -> values_[i][row]);
        printf("\n");
    }
    return 0;
}

void Retrieve(struct result_t *ret, MYSQL_RES *res) {
    MYSQL_FIELD *fields;
    MYSQL_ROW row;
    int i = 0,  j;

    ret -> num_of_fields_ = (int)mysql_num_fields(res);
    fields = mysql_fetch_fields(res);

    for (j = 0; j < ret -> num_of_fields_; j++) {
        strncpy(ret -> fields_[j], fields[j].name, FIELD_LEN); 
    }

    while ((row = mysql_fetch_row(res))) {
        for (j = 0; j < ret -> num_of_fields_; j++) {
            if (!row[j]) {
                strcpy(ret -> values_[j][i], "NULL");
            } else {
                strncpy(ret -> values_[j][i], row[j], VALUE_LEN);
            }
        }
        i++;
    } 

    ret -> num_of_values_ = i;
}

void GetDBInfo() {
    printf("Input database username: ");
    scanf("%s", db_username);
    printf("Input database password: ");
    scanf("%s", db_passwd);
    printf("Input database name: ");
    scanf("%s", db);
    getchar();
}
