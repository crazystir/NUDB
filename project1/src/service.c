#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "service.h"
#include "dao.h"
#include "common.h"
#include "types.h"

#define STMT_LEN 4096

static char *semesters[4] = {"Q1", "Q2", "Q3", "Q4"};

struct nu_sem_t GetCurrentSemester() {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    struct nu_sem_t res = {tm.tm_year + 1900, (tm.tm_mon / 4 + 2) % 4};
    return res;
}

int GetStudentById(struct result_t *student, char *id) {
    char stmt[STMT_LEN] = "select * from student where";
    Concatenate(stmt, " Id = ", id, 0);
    return SqlExec(stmt, student);
}

int ValidateStudent(struct result_t *student, char *username, char *password) {
    char stmt[STMT_LEN] = "select * from student where";
    Concatenate(stmt, " name = '", username, "' and password = '", password, "'", 0);
    return SqlExec(stmt, student);
}

int CourseMenu(struct result_t *courses, char *studentId) {
    char year[5];
    struct nu_sem_t nu_sem = GetCurrentSemester();

    sprintf(year, "%d", nu_sem.year);
    char stmt[STMT_LEN] = "select * from unitofstudy natural join transcript";

    Concatenate(stmt, " where StudId = ", studentId, 
            " and Semester = '", semesters[nu_sem.semester], "' and Year = ", year, 0);

    return SqlExec(stmt, courses);
}

int CourseTranscript(struct result_t *transcripts,  char *studentId) {
    char stmt[STMT_LEN] = "select UoSCode, UoSName, Semester, Year,  Enrollment, MaxEnrollment, Name as Lecturer, Grade from (select * from unitofstudy natural join transcript natural join uosoffering) as T left outer join faculty on InstructorId = Id";

    Concatenate(stmt, " where StudId = ", studentId);

    return SqlExec(stmt, transcripts);
}

int AvaliableCourses(struct result_t *avaliableCourses, char *studentId) {
    char year1[5], year2[5];
    struct nu_sem_t nu_sem = GetCurrentSemester();
    char stmt[STMT_LEN] = "call avaliable_courses";

    sprintf(year1, "%d", nu_sem.year);
    sprintf(year2, "%d", nu_sem.year + (nu_sem.semester == 0));

    Concatenate(stmt, "(", studentId, ", '", semesters[nu_sem.semester]
            , "', ", year1, ", '", semesters[(nu_sem.semester + 1) % 4], "', ", year2, ")", 0);

    return SqlExec(stmt, avaliableCourses);
}

int ConstraintCourses(struct result_t *constraintCourses, char *studentId) {
    char year1[5], year2[5];
    struct nu_sem_t nu_sem = GetCurrentSemester();
    char stmt[STMT_LEN] = "call constraint_courses";

    sprintf(year1, "%d", nu_sem.year);
    sprintf(year2, "%d", nu_sem.year + (nu_sem.semester == 0));

    Concatenate(stmt, "(", studentId, ", '", semesters[nu_sem.semester]
            , "', ", year1, ", '", semesters[(nu_sem.semester + 1) % 4], "', ", year2, ")", 0);

    return SqlExec(stmt, constraintCourses);

}


int EnrollCourse(char *studentId, char *uoSCode, char *semester, char *year, struct result_t *enroll) {
    int i, exist = 0;
    char data[3][RES_VAL_LEN][VALUE_LEN];
    GetValuesByField(enroll, "UoSCode", data[0]);
    GetValuesByField(enroll, "Semester", data[1]);
    GetValuesByField(enroll, "Year", data[2]);

    for (i = 0; i < enroll -> num_of_values_; i++) {
        if (strcmp(data[0][i], uoSCode) == 0 && strcmp(data[1][i], semester) == 0 && strcmp(data[2][i], year) == 0) {
            exist = 1;
            break;
        }
    }

    if (!exist) {
        return -1;
    }

    char stmt[STMT_LEN] = "call enroll";

    Concatenate(stmt, "(", studentId, ", '", uoSCode, "', '", semester, "', ", year, ")", 0);

    return SqlExec(stmt, NULL);
}

int WithdrawableCourses(struct result_t *courses, char *studentId) {
    char year[5];
    struct nu_sem_t nu_sem = GetCurrentSemester();

    sprintf(year, "%d", nu_sem.year);
    char stmt[STMT_LEN] = "select * from unitofstudy natural join transcript";
    
    Concatenate(stmt, " where StudId = ", studentId
            , " and (Year > ", year, " or (Semester >= '", semesters[nu_sem.semester], "' and Year = ", year,")) and Grade is NULL", 0);

    return SqlExec(stmt, courses); 
}

int WithdrawCourse(char *studentId, char *uoSCode, struct result_t *withdraw) {
    int i;
    char data[3][RES_VAL_LEN][VALUE_LEN];
    char *semester = NULL, *year = NULL;
    char stmt[STMT_LEN] = "call withdraw";
    struct result_t flag;

    GetValuesByField(withdraw, "UoSCode", data[0]);
    GetValuesByField(withdraw, "Semester", data[1]);
    GetValuesByField(withdraw, "Year", data[2]);

    for (i = 0; i < withdraw -> num_of_values_; i++) {
        if (strcmp(withdraw -> values_[0][i], uoSCode) == 0) {
            semester = withdraw -> values_[withdraw -> num_of_fields_ - 3][i];
            year = withdraw -> values_[withdraw -> num_of_fields_ - 2][i];
        } 
    }

    if (semester == NULL) {
        return -1;
    }

    Concatenate(stmt, "(", studentId, ", '", uoSCode, "', '", semester, "', ", year, ")", 0);

    if (SqlExec(stmt, NULL) < 0) {
        return -1;
    }

    stmt[0] = '\0';

    Concatenate(stmt, "select flag from warning");

    if (SqlExec(stmt, &flag) < 0) {
        return -1;
    }

    if (atoi(flag.values_[0][0]) == 1) {
        printf("Warning: the course is under 50%% of its max enrollment.\n");
    }

    return 0;
}

// Update password or address
int UpdatePersonalDetail(char *studentId, char *passwd, char *address) {
    if (!passwd && !address) {
        return -1;
    }

    char stmt[STMT_LEN] = "update student set";

    if (passwd) {
        Concatenate(stmt, " password = '", passwd, "'", 0);
    } else {
        Concatenate(stmt, " address= '", address, "'", 0);
    }

    Concatenate(stmt, " where Id = ", studentId, 0);
    return SqlExec(stmt, NULL);
}
