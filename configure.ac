AC_PREREQ([2.69])
AC_INIT(twork, 0.1)
AC_CONFIG_FILES([Makefile])
AC_CONFIG_SRCDIR([src/utils.c])
AC_CONFIG_HEADERS([config.h])

AC_PROG_YACC
AC_PROG_CC([gcc])
AC_PROG_LEX
AC_PROG_INSTALL
AC_PROG_LN_S

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for header files.
AC_PATH_X
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([fcntl.h inttypes.h libintl.h malloc.h stddef.h stdlib.h string.h unistd.h netdb.h netinet/in.h sys/socket.h])

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MKTIME'
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memset pow sqrt strdup strstr alarm dup2 gethostbyname socket strerror])

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lX11':
AC_CHECK_LIB([X11], [main])
# FIXME: Replace `main' with a function in `-lm':
AC_CHECK_LIB([m], [main])
# FIXME: Replace `main' with a function in `-lsqlite3':
AC_CHECK_LIB([sqlite3], [main])

VERSION=0.1
NAME=twork
#*editable* where your profile is located
PROF=~/$NAME
PRJ=~/prj
TWORK_PROF=\"$PROF\"
#*editable* where you alarm audio files are located
TWORK_ALERT=\"$PROF/media/\"
#*editable* backup for database after each installation
TWORK_BACKUP=\"$PROF/backup\"
TWORK_DEVELOP=\"$PRJ/\"
#alert player#
#TWORK_PLAYER=\"'\"
AC_SUBST(LDFALGS)
AC_SUBST(TWORK_PROF, $TWORK_PROF)
AC_SUBST(TWORK_ALERT, $TWORK_ALERT)
AC_SUBST(TWORK_BACKUP, $TWORK_BACKUP)
AC_SUBST(TWORK_DEVELOP, $TWORK_DEVELOP)
#note it refer to NAME above
AC_SUBST(TWORK_PLAYER, \"'amixer -D pulse sset Master unmute; amixer -D pulse sset Master 150%;vlc-wrapper -ZL ~/twork/media/'\")
AC_DEFINE_UNQUOTED(VERSION, $VERSION, twork version)
AC_DEFINE_UNQUOTED(TWORK_PROF,$TWORK_PROF, twork profile directory)
AC_DEFINE_UNQUOTED(TWORK_ALERT,$TWORK_ALERT, where readable alert audio files are located)
AC_DEFINE_UNQUOTED(TWORK_BACKUP,$TWORK_BACKUP, where twork.dbTWORK_PLAYER(database) file is backed after installation)
AC_DEFINE_UNQUOTED(TWORK_DEVELOP,$TWORK_DEVELOP, twork development location)
AC_DEFINE_UNQUOTED(TWORK_PLAYER, $TWORK_PLAYER, alert player command)

AC_OUTPUT
