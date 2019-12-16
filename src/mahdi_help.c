
#include <Mahdi/system.h>
/***
 * [group] [subject] [part]
 * mahdi -h [OK]
 * mahdi -h usage [OK]
 * mahdi -h -l (list all of groups) [OK]
 * mahdi -h keywords (introduction about this group) [OK]
 * mahdi -h keywords -l [OK]
 * mahdi -h keywords return (doc about 'return' 'keyword') [OK]
 * mahdi -h keywords return -l (list all of parts of text) [OK]
 * mahdi -h keywords return intro (only shows 'intro' part of 'return' text) [OK]
 */
//************************************************
String docs_dir;
StrList list_keys, list_values; //generated by MHELP_print_article for list item
uint32 list_count;
//************************************************
String MHELP_groups[] = {
    "concepts",
    "keywords",
    "operators",
    "built_in_funcs",
    "configs",
    "exceptions",
    "debugger",
    "builder",
    "get_started",
    "about"
};
//************************************************
String MHELP_get_started_subjects[] = {
    "whats_new",
    "tour",
    "samples",

};
//************************************************
String MHELP_about_subjects[] = {
    "license",
    "more",
    "README.md",
    "credits"
};

Boolean MHELP_starter(StrList argvs, uint32 argvs_len) {
  //help || usage || -l || [group] || [group] -l || [group] [subject] -l || [group] [subject] [part]
  //  printf ("--Help:%s\n", print_str_list (argvs, argvs_len));
  //=>print main help
  if (argvs_len == 0) MHELP_main();
    //=>print help usage
  else if (argvs_len == 1 && STR_equal(argvs[0], "usage")) MHELP_usage();
    //=>print anything
  else {
    //init vars
    String params[3] = {0, 0, 0};
    //=>if argvs_len>3,raise an error
    if(argvs_len>3){
      //TODO:error
      printf("MH#ERR2432423\n");
      return false;
    }
    docs_dir = STR_multi_append(interpreter_path, os_separator, "docs", os_separator, 0, 0); //=>absolute path of 'docs' dir
    String not_exist = 0;
    //---------------------------------------------
    //=>full params array
    for (uint8 i = 0; i < argvs_len; i++) params[i] = argvs[i];
    //=>if list is requested
    int8 list_ind = 0;
    if ((list_ind = STR_search_index(params, "-l", 3)) > -1)MHELP_list(list_ind, params, &not_exist);
      //if article is requested
    else MHELP_article(params, &not_exist);
    //-----------------------if not exist file
    if (not_exist != 0) {
      EXP_print_error(0, "not_exist_docs_file", "stdin", not_exist, 0, "MHELP_starter");
      return false;
    }
  }
  return true;
}
//************************************************
void MHELP_usage() {
  printf("usage for Mahdi: mahdi [option] --OR-- mahdi [path] [arg]...\n");
  printf("usage for Mahdi-docs: mahdi -h [group] [subject] [part]\n");
  printf("Try 'mahdi -h' for more information.\n");
}
//************************************************
void MHELP_version() {
  printf("%s - %s(#%i)\n", VERSION_NAME, VERSION, VERSION_NUMBER);
}
//************************************************
void MHELP_info() {
  printf("%s\n", APP_NAME);
  printf("VERSION:%s-%s(#%i)\n", VERSION_NAME, VERSION, VERSION_NUMBER);
  printf("BUILD:%s(%s) [%s-%s]\n", OS_BUILD, OS_ARCH, BUILD_DATE, BUILD_TIME);
  printf("LICENCE:%s(%s)\n", LICENCE, MAHDI_LICENCE_VIEW_COMMAND);
  printf("COMPILER:%s-%s\n", C_COMPILER_NAME, C_COMPILER_STD);
  printf("CREATOR:%s[%s-%s]\n", LANGUAGE_CREATOR, FIRST_BUILD, BETA_LBUILD);
  printf("TRUST:%i%%\n", TRUST_LEVEL);
}
//************************************************
void MHELP_main() {
  printf("\t In The Name Of ALLAH\n\t----------------------\n");
  printf("%s %s(#%i)-%s\n",
         APP_NAME, VERSION_NAME, VERSION_NUMBER, VERSION);
  printf("usage: mahdi [option] --OR-- mahdi [path] [arg]...\n");
  printf("Options:\n");
  printf("-v	:Display interpreter version information\n");
  printf("-i	:Display information about MAHDI. \n\n");
  printf("-h	:Display information about anything. \n\n");
  printf("usage(mahdi-docs): mahdi -h [group] [subject] [part]\n");
  printf(
      "You can type \'mahdi -h -l\' to get list of all groups.\nAnd you can type \'mahdi -h [group] -l\' to get list of all subjects of [group].\n");
  printf("-------------------------------------\n");
  printf("%s\n", SLOGAN);
  printf("See '%s' for more details.\n", OFFICIAL_WEBSITE);
}
//************************************************
int8 MHELP_print_article(String path, String part, Boolean generate_list) {
  if (part != 0 && part[0] != '#')part = STR_multi_append("#", part, 0, 0, 0, 0);
  StrList lines = 0;
  uint32 c1 = 0;//=>list items counter
  uint32 c2 = 0;
  Boolean is_range = false;//=>if browse a specific part or all part
  Boolean is_exist = false;//=>if exist the article
  if (part == 0)is_range = true;//if not any specific part, then browse all parts
  if (generate_list)list_count = 0;
  //=>read all lines from [file].mdo and store it in 'lines' str_list
  int32 lines_co = CALL_read_file(path, &lines, true);
  //=>check if occur an exception when reading file or is END OF FILE(EOF)
  //printf("SSWWWWW:%s,%i\n",path,lines_co);
  if (lines_co == -1)return -1;
  //=>browse all nodes of 'lines'
  for (uint32 i = 0; i < lines_co; i++) {
    //=>check if this line is start of specific part
    if (!is_range && STR_equal(lines[i], part)) {
      is_range = true;
      continue;
    }
      //=>if exist specific part, so ignore the other parts
    else if (part != 0 && is_range && lines[i][0] == '#')break;
    //=>if in specific part or start of browse all parts
    if (is_range) {
//      printf("SSSSSS:%s\n",lines[i]);
      Boolean is_part_header = false;
      //=>if this line is a part header
      if (lines[i][0] == '#') {
        is_part_header = true;
        printf("%s\n=> | %s\n%s\n", header_full, STR_substring(lines[i], 1, 0), header_sp);
      }
      //=>list items format
      if (lines[i][0] == '*') {
        if (!generate_list && c1 == 0)printf("%s\n", header_full);
        int32 ind = CH_last_indexof(lines[i], '@');
        String sub = 0;
        if (ind > -1) {
          sub = STR_substring(lines[i], ind + 1, 0);
          lines[i] = STR_substring(lines[i], 0, ind);
        }
        lines[i] = STR_substring(lines[i], 1, 0);
        if (generate_list) {
          SLIST_append(&list_keys, lines[i], list_count);
          SLIST_append(&list_values, sub, list_count++);
        }
        if (!generate_list) DRAW_TABLE_ROW(c1 + 1, lines[i]);
        c1++;
      } else if (lines[i][0] == '-') {
        c2++;
        printf("%i.  %s \n", c2, STR_substring(lines[i], 1, 0));
      } else if (!is_part_header) {
        //=>every line format
        if (c1 == 0)printf("   | ");
        printf("%s\n", lines[i]);
        c1 = c2 = 0;
      }
      is_exist = true;
    }
  }
  if (!is_exist) {
    printf("\n=====): Not Found '%s' Part :(=====\n\n", part);
    return 1;
  }
  return 0;
}
//************************************************
void MHELP_article(String params[3], String *not_exist) {
  //TODO:
  //init vars
//   String footer = 0;
//   //=>show group intro
//   if (params[1] == 0 && STR_search(MHELP_groups, params[0], StrArraySize (MHELP_groups))) {
//     if (MHELP_print_article(str_multi_append(docs_dir, params[0], os_separator, "index.mdo", 0, 0), "#intro", false) == -1) {
//       (*not_exist), params[0];
//       return;
//     }
//     sprintf(footer, "For getting list of all subjects of \'%s\' group, Type \'mahdi -h %s -l\'", params[0], params[0]);
//   }

//     //=>show a specific part of a subject or all parts
//   else if (params[1] != 0 && str_search(help_groups, params[0], StrArraySize (help_groups))) {
//     //init vars
//     String subject_path = 0;
//     int32 index = 0;
//     //=>find address of subject file from 'index.mdo' of group
//     //if group index or subject key in group index file not found
//     if (MHELP_print_article(str_multi_append(docs_dir, params[0], os_separator, "index.mdo", 0, 0), "#subjects", true) == -1
//         || (index = str_search_index(list_keys, params[1], list_count)) == -1) {
//       *not_exist = params[1];
//       return;
//     }
//     //=>get subject file .mdo path
//     subject_path = (list_values[index][0] != 0) ? list_values[index] : str_append(params[1], ".mdo");
// //    printf("subject_path:%s[%i],%s,%s\n", subject_path, index, params[1], print_str_list(list_keys, list_count));
//     //=>show article
//     if (MHELP_print_article(str_multi_append(docs_dir, params[0], os_separator, subject_path, 0, 0), params[2], false) == -1)
//       &not_exist, params[1];
//     sprintf(footer,
//             "For getting list of all parts of \'%s\' subject, Type \'mahdi -h %s %s -l\'",
//             params[0], params[1], params[1]);
//   }

//   //=>show footer
//   if (footer != 0) DRAW_TABLE_FOOTER(footer);
}
//************************************************
void MHELP_list(uint8 type, String params[3], String *not_exist) {
  //TODO:
  //=>if request for group list
//   if (type == 0) {
//     printf("mahdi:)) offline documentation groups:\n%s\n", header_full);
//     for (uint8 i = 0; i < StrArraySize (help_groups); i++)
//       DRAW_TABLE_ROW(i + 1, help_groups[i])
//   }
//     //=>if request for subject list
//   else if (type == 1) {
//     printf("mahdi:)) offline documentation '%s' subjects:\n", params[0]);
//     //=>if not exist list of subjects
//     if (MHELP_print_article(str_multi_append(docs_dir, params[0], os_separator, "index.mdo", 0, 0), "#subjects", false) == -1)
//       str_init(&(*not_exist), params[0]);
//   }
//     //=>if request for part list
//   else if (type == 2) {
//     printf("mahdi:)) offline documentation '%s' (%s) parts:\n", params[0], params[1]);
//     //init vars
//     String subject_path = 0;
//     int32 index = 0;
//     //=>find address of subject file from 'index.mdo' of group
//     //if group index or subject key in group index file not found
//     if (MHELP_print_article(str_multi_append(docs_dir, params[0], os_separator, "index.mdo", 0, 0), "#subjects", true) == -1
//         || (index = str_search_index(list_keys, params[1], list_count)) == -1) {
//       *not_exist = params[1];
//       return;
//     }
//     //=>get subject file .mdo path
//     subject_path = (list_values[index][0] != 0) ? list_values[index] : str_append(params[1], ".mdo");
// //    printf("subject_path:%s[%i],%s,%s\n",subject_path,index,params[1],print_str_list(list_keys,list_count));
//     //=>read all lines of subject file
//     str_list lines = 0;
//     int32
//         lines_co =
//         __syscall_read_file(str_multi_append(docs_dir, params[0], os_separator, subject_path, 0, 0), &lines, true);
//     uint32 co = 0;
//     //=>if can not read subject file
//     if (lines_co == -1) {
//       *not_exist = params[1];
//       return;
//     }
//     //read and print all parts of subject file
//     for (uint32 i = 0; i < lines_co; i++)
//       if (lines[i][0] == '#') {
//         if ((co > 0 && co % 3 == 0))printf("\n");
//         printf("%-20s \t", str_substring(lines[i], 1, 0));
//         co++;
//       }
//     printf("\n");

//   }
}