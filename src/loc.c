#include "loc.h"

const String AUTHOR = "ertosns";
const String LOC = "LOC";
const String SUBMODULE  = "SUBMODULE";
const String COMMIT = "COMMIT";
const String POSE = "POSITIVE";
const String NEGE = "NEGATIVE";
const int LOC_COL_SIZE = 4;
const String HASH = "hash";
const String ADDITION = "addition";
const String DELETION = "deletion";
const String POS = "_pos";
const String NEG = "_neg";
const int MAX_LINE_CHARS = 512;
const String WRONG_PATH = "can't access givin directory ";
const String WRONG_GIT_ADD = "couldn't git add to ";
const String WRONG_GIT_COMMIT = "couldn't git commit to ";
const String WRONG_GIT_PUSH = "couldn't push repo";
const int COMMIT_SHRESHOLD = 86400;
String DEFAULT_MSG = "laboron de tago";
String GIT_LOG;
String GIT_DIFF;
String GIT_ADD;
String GIT_COMMIT;
String GIT_PUSH;
String DEV_PATH;
//TODO (res) how call function once upon use? after loading hdrs
int initloc() {
  DEV_PATH = TWORK_DEVELOP;
  GIT_LOG = cat(5,"cd ", DEV_PATH, ";git submodule foreach git log --author ",AUTHOR, " -n 1");
  GIT_DIFF = cat(3, "cd ", DEV_PATH, ";git submodule foreach git diff --numstat ");
  GIT_ADD = cat(3, "cd ", DEV_PATH, ";git submodule foreach git add --all");
  GIT_COMMIT = cat(5, "cd ", DEV_PATH, ";git submodule foreach git commit -m\"",DEFAULT_MSG, "\"");
  GIT_PUSH = cat(3, "cd ", DEV_PATH, ";git submodule foreach git push");

  
  State *ls = last_state(LOC);
  if (ls->type==none || (int)difftime(time(NULL), timegm(ls->date))>=COMMIT_SHRESHOLD)
    return loc(NULL);
  return SUCCESS;
}

//TODO (fix) unify all linked lists used allover!
struct HASH {
  struct HASH *nxt;
  String col;
  String hash;
} hashes;
struct HASH *shash = &hashes;

//TODO (fix) replace strings with global variables for better styleing
//TODO handle NULL ohash
void yieldloc (String colname, String ohash, String nhash, int *locyield) {
  FILE *diff = popen(cat(5, GIT_DIFF, ohash?ohash:"", " ", nhash, " 2>/dev/null "), "r");
  String yield, line;
  line = malloc(MAX_LINE_CHARS);
  int pos = 0;
  int neg = 0;
  while((line = fgets(line, MAX_LINE_CHARS, diff))) {
    if (strstr(line, "Entering") == line)
      continue;
    yield = strtok(line, "\t");
    pos += atoi(yield);
    yield = strtok(line, "\t");
    neg += atoi(yield);
  }
  locyield[0] = pos;
  locyield[1] = neg;
}

//TODO (fix) dailyterminated instead of expanding columns, expand rows, and read per column/row value instead of column value.
/*store last commits hashes in PATH submodule*/
int storecommits() {
  struct HASH *hs = &hashes;
  //TODO (res) initializing type name[size] return object may not be initialized?!
  String colnames[] = {LOC, SUBMODULE, COMMIT, POSE, NEGE};
  if(notexist(LOC)) {
    Val vals[] = {makeval(SUBMODULE, sdt_string),
                  makeval(COMMIT, sdt_string),
                  makeval(POSE, sdt_number),
                  makeval(NEGE, sdt_number)};
    sqlCreate(LOC, vals, LOC_COL_SIZE);
  }
  FILE *commits = popen(GIT_LOG, "r");
  String word, hash, tmpstr = NULL;
  String line = malloc(MAX_LINE_CHARS);
  int ncommits = 0;
  while (line = fgets(line, MAX_LINE_CHARS, commits)) {
    if(strstr(line, "Entering") == line){
      strtok(line, "\'");
      tmpstr = strtok(NULL, "\'");
      word = strdup(tmpstr);
    }
    else if (strstr(line, "commit")) {
      strtok(line, " ");
      hash = strtok(NULL, " ");
      hs->col = strdup(word);
      hs->hash = strdup(hash); 
      /*if(fgets(line, MAX_LINE_CHARS, commits)) {
        
        for(int c = 0; c < strlen(line); c++)
          ungetc((int)line[c], commits);
        ungetc('\n', commits);
        }*/
      hs->nxt = malloc(sizeof(struct HASH));
      hs = hs->nxt;
      free(word);
      ncommits++;
    }
  }
  
  if(ncommits==0) {
    pclose(commits);
    return SUCCESS;
  }

  hs = shash;
  int loc[2] = {0,0};
  Result *ilocres;
  Result *oldhashres;
  String oldhashnames[] = {LOC, COMMIT};
  Val oldhashvals[] = { makeval(COMMIT, sdt_string) };
  String oldhashclause;
  String ohash;
  for (int r = 0; r < ncommits; r++) {
    // get prev hash for the same submodule hs->col
    // calc diff betweeh hs->hash, prev line.
    //get old hash
    //TODO (fix) create map between type, colname i.e commit, sdt_string cleaner, support future layers
    oldhashclause = cat(4, prependType(SUBMODULE, sdt_string), " = '", hs->col, "'");
    oldhashres = sqlRead(LOC, oldhashvals, 1, 1, 1,
                         oldhashclause);
    ohash = (oldhashres->table)?oldhashres->table->row->val[0]:NULL;
    //free(oldhashres);
    yieldloc(hs->col, ohash, hs->hash, &loc[0]);
    Val vals[] = { makeval(hs->col, sdt_string),
                   makeval(hs->hash, sdt_string),
                   makeval(itos(loc[0]), sdt_number),
                   makeval(itos(loc[1]), sdt_number) };
    sqlInsert(colnames, vals, 4);
    //TODO why it fails?
    //free(ilocres);
    // also free vals
    hs = hs->nxt;
  }
  //free hs
  pclose(commits);
  return SUCCESS;
}

int loc(String commitmsg) {
    if(commitmsg)
        DEFAULT_MSG = commitmsg;

    if(!isdirectory(DEV_PATH)) {
        error(cat(2, WRONG_PATH, DEV_PATH));
        return FAILED;
    }
    int status = system(GIT_ADD);
    assertcmd(&status);
    if(!status) {
	error(cat(2, WRONG_GIT_ADD, DEV_PATH));
	return FAILED;
    }
    status = system(GIT_COMMIT);
    assertcmd(&status);
    if(!status) {
      //nothing to commmit
      //error(cat(2, WRONG_GIT_COMMIT, DEV_PATH));
      //return FAILED;
    }
    status = system(GIT_PUSH);
    assertcmd(&status);
    if(!status)
	error(WRONG_GIT_PUSH);
    
    storecommits();
    return 1;
}
