#include <Mahdi/system.h>

//******************************************
String CH_append(String s, uint8 c) {
  uint32 len = STR_length(s);
  String s1;
  s1 = (String) malloc(sizeof(uint8) * (len + 2));
  //---------------------------
  if (s1 != 0) {
    for (uint32 i = 0; i < len; i++) {
      s1[i] = s[i];
    }
    s1[len] = c;
    s1[len + 1] = '\0';
  }
  //printf("CCC:%s,%i,%i\n",s1,len);
  return s1;
}

//******************************************
String CH_join(StrList s, uint8 sp, uint32 size, Boolean is_remove_empty) {
  String out = "";
  for (uint32 i = 0; i < size; i++) {
    if (is_remove_empty && s[i][0] == 0)
      continue;
    if (i > 0)
      out = CH_append(out, sp);
    out = STR_append(out, s[i]);
  }
  return out;
}

//******************************************
uint32 CH_split(String s, uint8 sp, StrList *ret, Boolean is_remove_empty) {
  if (s == 0) {
    *ret = 0;
    return 0;
  }
  uint32 ind = 0;
  uint32 len = STR_length(s);
  String buf = 0;
  uint32 size = 0;
  for (uint32 i = 0; i <= len; i++) {
    if (i == len || s[i] == sp) {
      size++;
    }
  }
  (*ret) = (StrList) malloc(size * sizeof(StrList));
  for (uint32 i = 0; i <= len; i++) {
    if (i == len || s[i] == sp) {
      if (!is_remove_empty || !STR_is_empty(buf)) {
        if (STR_is_empty(buf))(*ret)[ind++] = 0;
        else (*ret)[ind++] = buf;
      }
      buf = 0;
      if (i == len) break;
      continue;
    }
    buf = CH_append(buf, s[i]);
  }
  return ind;
}

//******************************************
Boolean CH_search(uint8 list[], uint8 c) {
  uint32 len = STR_length(list);
  for (uint32 i = 0; i < len; i++) {
    if (c == list[i])
      return true;
  }
  return false;
}

//******************************************
int32 CH_search_index(uint8 list[], uint8 c) {
  uint32 len = STR_length(list);
  for (uint32 i = 0; i < len; i++) {
    if (c == list[i])
      return i;
  }
  return -1;
}

//******************************************
uint32 CH_search_count(String s, uint8 c) {
  if (s == 0 || c == 0)return 0;
  uint32 count = 0, len = 0;
  while (s[len] != 0) {
    if (s[len] == c)count++;
    len++;
  }
  return count;
}

//******************************************
String CH_to_string(uint8 c) {
  String s = 0;
  return CH_append(s, c);
}

//******************************************
int32 CH_last_indexof(String s, uint8 ch) {
  uint32 len = STR_length(s);
  if (len < 1)return -1;
  for (uint32 i = len - 1; i >= 0; i--) {
    if (i == -1)break;
    if (s[i] == ch) return i;
  }
  return -1;
}

//******************************************
String CH_backspace(String s) {
  //printf ("$BACKSPACE:%s\n", s);
  uint32 len = STR_length(s);
  if (len == 0)return 0;
  s[len - 1] = 0;
  return s;
}

//******************************************
uint8 CH_to_uint8(uint8 c, Boolean *has_err) {
  if (c < '0' || c > '9') {
    if (*has_err != 0)(*has_err) = true;
    return 0;
  }
  if (*has_err != 0)(*has_err) = false;
  return c - '0';
}
//******************************************
Boolean CH_isnum(uint8 c){
  if(c>='0' && c<='9')return true;
  return false;
}
//******************************************
Boolean CH_isspace(uint8 c){
  if(c==' '||c=='\t'||c=='\n'||c=='\v'||c=='\f'||c=='\r')return true;
  return false;
}