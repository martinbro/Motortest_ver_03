#ifndef MARNAVMOTOR_H
#define MARNAVMOTOR_H



int isValidString(const char* data) {
  // Create a set of valid characters
  const char validChars[] = "0123456789<>,-";

  // Check if all characters in the string are present in the validChars set
  for (int i = 0; data[i] != '\0'; i++) {
    if (strchr(validChars, data[i]) == NULL) 
      return 0; // String is invalid
  }
  return 1; // String is valid
}

// void analyserData(String RCdata,int* ror,int* speedSP, int* mode){
//   int openingBracket = RCdata.indexOf('<');
//   int closingBracket = RCdata.indexOf('>');
//   int firstComma = RCdata.indexOf(',');
//   int secondComma = RCdata.indexOf(',', firstComma+1);
//   String rorString = RCdata.substring(openingBracket+1,firstComma);
//   // String rorString = RCdata.substring(1,firstComma);
//   int r = rorString.toInt();
//   // if(isDigit(r))
//   if(-36<r && r<36 )
//     *ror = r;
//   String energiString = RCdata.substring(firstComma+1,secondComma);
//   int e = energiString.toInt();
//   // if(isDigit(e))
//   if(-201<e && r<201 )
//     *speedSP = e;
//   String modeString = RCdata.substring(secondComma+1,closingBracket);
//   int m = modeString.toInt();
//   // if(isdigit(m))
//   if(m<10 && m>1)
//     *mode = m;
// }

#endif