#ifndef STATUS_H
#define STATUS_H
#include "dao.h"

#define NUM_OF_STATUS 11
enum status{LOGIN = 0, MENU = 1, TRANSCRIPT = 2, COURSE_DETAIL = 3
    , ENROLL = 4, ENROLL_COURSE = 5, WITHDRAW = 6, WITHDRAW_COURSE = 7
        , PERSONAL_DETAIL = 8, UPDATE_PERSONAL_DETAIL = 9, EXIT= 10};


struct status_t {
    enum status status_;
    char *studentId_;
    char *username_;
    int row;
    struct result_t info_;
    struct result_t currentCourses_;
    struct result_t transcripts_;
    struct result_t enroll_;
    struct result_t unenroll_;
    struct result_t withdraw_;
};


#endif
