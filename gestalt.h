#ifndef GESTALT_H

#define GESTALT_H

#ifdef  __cplusplus
extern "C" {
#endif

// Performs a 'fuzzy' comparison between two strings.  Returns how
// 'alike' they are expressed as a percentage match.
//
// Written originally by John W. Ratcliff for Dr. Dobbs Journal
// of Software Tools Volume 13, 7, July 1988
//
// Pages 46, 47, 59-51, 68-72
// http://www.ddj.com/184407970?pgno=5
//
// http://www.codesuppository.blogspot.com/
//
// If you appreciate my little snippets of source code
// please donate a few bucks to my kids youth group fundraising
// website located at http://www.amillionpixels.us/

int FuzzyCompare(const char *s1,const char *s2);


#ifdef  __cplusplus
}
#endif

#endif
