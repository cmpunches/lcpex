#ifndef LCPEX_CONTEXTS_H
#define LCPEX_CONTEXTS_H

#include <string>
#include <csignal>
#include <pwd.h>
#include <grp.h>
#include <iostream>

enum IDENTITY_CONTEXT_ERRORS {
    ERROR_NONE = 0,
    ERROR_NO_SUCH_USER,
    ERROR_NO_SUCH_GROUP,
    ERROR_NO_SUCH_USER_IN_GROUP,
    ERROR_SETGID_FAILED,
    ERROR_SETUID_FAILED,
};

// converts username to UID
int username_to_uid( std::string username, int & uid );

// converts group name to GID
int groupname_to_gid( std::string groupname, int & gid );

// SET PROCESS TO A CERTAIN IDENTITY CONTEXT
int set_identity_context( std::string user_name, std::string group_name );


#endif //LCPEX_CONTEXTS_H
