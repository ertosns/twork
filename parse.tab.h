/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_PARSE_TAB_H_INCLUDED
# define YY_YY_PARSE_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    SQL = 258,
    EXIT = 259,
    NAME = 260,
    INTEGER = 261,
    DOUBLE = 262,
    PASSCODE = 263,
    PATH = 264,
    HOURS = 265,
    MINUTES = 266,
    SECONDS = 267,
    DEFINITION = 268,
    SLITERAL = 269,
    EOL = 270,
    START = 271,
    STOP = 272,
    EVENT = 273,
    UNDO = 274,
    REDO = 275,
    CREATE = 276,
    ACCUMULATE = 277,
    INSERT = 278,
    UPDATE = 279,
    ALTER = 280,
    LINK = 281,
    ADD_LINKABLE = 282,
    PRINT_COLUMNS = 283,
    DOES_TABLE_EXIST = 284,
    VIEW_LAST_RECORDS = 285,
    REMOVE_LAST_RECORD = 286,
    DROP_TABLE = 287,
    GET_CURRENT_TASK = 288
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 32 "src/parse.y" /* yacc.c:1909  */

  KeyVal *keyvalptr;
  Val *val;
  char *chars;
  int inttype;
  long double ldouble;

#line 96 "parse.tab.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSE_TAB_H_INCLUDED  */
