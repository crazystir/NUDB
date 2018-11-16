#ifndef API_H
#define API_H

#include "dao.h"

struct nu_sem_t {
    int year;
    int semester;
};

struct nu_sem_t GetCurrentSemester();

//0 means success and -1 means failure

int GetStudentById(struct result_t *student, char *id);

int ValidateStudent(struct result_t *student, char *username, char *password);

int CourseMenu(struct result_t *courses, char *studentId);

int CourseTranscript(struct result_t *Transcripts, char *studentId);

int CourseDetail(struct result_t *course);

int AvaliableCourses(struct result_t *courses, char *studentId);

int ConstraintCourses(struct result_t *constraintCourses, char *studentId);

int EnrollCourse(char *studentId, char *uoSCode, char *semester, char *year, struct result_t *enroll);

int WithdrawableCourses(struct result_t *course, char *studentId);

int WithdrawCourse(char *studentId, char *uoSCode, struct result_t *withdraw);

int UpdatePersonalDetail(char *studentId, char *passwd, char *address);

#endif
