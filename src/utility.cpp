#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

string int2string(int number){
  char t[256];
  string s;
  sprintf(t,"%d",number);
  s = t;
  return s;
}
string double2string(double number){
  char t[256];
  sprintf(t,"%f",number);
  string s = t;
  return s;
}
double string2double(string s){
  return atof(s.c_str());
}
int string2int(string s){
  return atoi(s.c_str());
}
