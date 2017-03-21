//+------------------------------------------------------------------+
//|                                                  Trade Collector |
//|                   Copyright 2001-2012, MetaQuotes Software Corp. |
//|                                        http://www.metaquotes.net |
//+------------------------------------------------------------------+
#include "../StdAfx.h"
#include <math.h>
#include "common.h"
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
static LPCSTR       ExtOperations[9]  ={ "buy","sell","buy limit","sell limit",
                                         "buy stop","sell stop","balance","credit","error" };
static const double ExtDecimalArray[9]={ 1.0, 10.0, 100.0, 1000.0, 10000.0, 100000.0, 1000000.0, 10000000.0, 100000000.0 }; 
//+------------------------------------------------------------------+
//|                          Functions                               |
//+------------------------------------------------------------------+
//| Reading of the integer parameter                                 |
//+------------------------------------------------------------------+
int GetIntParam(LPCSTR string,LPCSTR param,int *data)
  {
//--- проверки
   if(string==NULL || param==NULL || data==NULL) return(FALSE);
//--- пропускаем пробелы
   while(*string==' ') string++;
   if(memcmp(string,param,strlen(param))!=0)     return(FALSE);
//--- все нормально
   *data=atoi(&string[strlen(param)]);
   return(TRUE);
  }
//+------------------------------------------------------------------+
//| Reading of the string parameter                                  |
//+------------------------------------------------------------------+
int GetStrParam(LPCSTR string,LPCSTR param,char *buf,const int maxlen)
  {
   int i=0;
//--- проверки
   if(string==NULL || param==NULL || buf==NULL)  return(FALSE);
//--- пропускаем пробелы
   while(*string==' ') string++;
   if(memcmp(string,param,strlen(param))!=0)     return(FALSE);
//--- берем результат
   string+=strlen(param);
   while(*string!=0 && i<maxlen) { *buf++=*string++; i++; }
   *buf=0;
//--- все нормально
   return(TRUE);
  }
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
void ClearLF(char *line)
  {
//---
   if(line==NULL) return;
   while(*line>31 || *line<0 || *line==9 || *line==27) line++;
   *line=0;
//---
  }
//+------------------------------------------------------------------+
//| Normalization of double value                                    |
//+------------------------------------------------------------------+
double __fastcall NormalizeDouble(const double val,int digits)
  {
   if(digits<0) digits=0;
   if(digits>8) digits=8;
//---
   const double p=ExtDecimalArray[digits];
   return((val>=0.0) ? (double(__int64(val*p+0.5000001))/p) : (double(__int64(val*p-0.5000001))/p));
  }
//+------------------------------------------------------------------+
//| Command description by command code                              |
//+------------------------------------------------------------------+
LPCSTR GetCmd(const int cmd)
  {
//--- проверки
   if(cmd<OP_BUY || cmd>OP_CREDIT) return(ExtOperations[8]);
//---
   return(ExtOperations[cmd]);
  }
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
double GetDecimalPow(int digits)
  {
//---
   if(digits<0) digits=0;
   if(digits>7) digits=7;
//---
   return(ExtDecimalArray[digits]);
  }
//+------------------------------------------------------------------+
//| The function of record insert into the ordered array regarding   |
//| the sorting                                                      |
//+------------------------------------------------------------------+
char* insert(void *base,const void *elem,size_t num,const size_t width,int(__cdecl *compare)( const void *elem1,const void *elem2 ))
  {
//---
   if(base==NULL || elem==NULL || compare==NULL) return(NULL);
//---
   if(num<1) { memcpy(base,elem,width); return(char*)(base); }
//---
   register char *lo=(char *)base;
   register char *hi=(char *)base+(num-1) * width, *end=hi;
   register char *mid;
   unsigned int   half;
   int            result;
//---
   while(num>0)
     {
      half=num/2;
      mid=lo+half*width;
      //---
      if((result=compare(elem,mid))>0) // data[mid]<elem
        {
         lo  =mid+width;
         num =num-half-1;
        }
      else if(result<0)                // data[mid]>elem
        {
         num=half;
        }
      else                             // data[mid]==elem
        return(NULL);
     }
//---
   memmove(lo+width,lo,end-lo+width);
   memcpy(lo,elem,width);
//---
   return(lo);
  }
//+------------------------------------------------------------------+
//| Check string by regular expression                               |
//+------------------------------------------------------------------+
static int CheckTemplate(char* expr,char* tok_end,const char* group,char* prev,int* deep)
  {
   char  tmp=0;
   char *lastwc,*prev_tok;
   const char *cp;
//--- check recursion depth
   if((*deep)++>=10) return(FALSE);
//--- skip repeats of start (*)
   while(*expr=='*' && expr!=tok_end) expr++;
   if(expr==tok_end) return(TRUE);
//--- find next star or end of line
   lastwc=expr;
   while(*lastwc!='*' && *lastwc!=0) lastwc++;
//--- temporarily limit the line
   if((tmp=*(lastwc))!=0) // token not last in line?
     {
      tmp=*(lastwc);*(lastwc)=0;
      if((prev_tok=(char*)strstr(group,expr))==NULL)
        {
         if(tmp!=0) *(lastwc)=tmp;
         return(FALSE);
        }
      *(lastwc)=tmp;
     }
   else // last token...
     {
      //--- checks
      cp=group+strlen(group);
      for(;cp>=group;cp--)
         if(*cp==expr[0] && strcmp(cp,expr)==0) return(TRUE);
      return(FALSE);
     }
//--- the order is broken?
   if(prev!=NULL &&  prev_tok<=prev) return(FALSE);
   prev=prev_tok;
//---
   group=prev_tok+(lastwc-expr-1);
//--- it is end?
   if(lastwc!=tok_end) return CheckTemplate(lastwc,tok_end,group,prev,deep);
//---
   return(TRUE);
  }

//+------------------------------------------------------------------+
//+------------------------------------------------------------------+
//| Group contains in grouplist pattern?                             |
//+------------------------------------------------------------------+
int CheckGroup(char* grouplist,const char *group)
  {
//--- checks
   if(grouplist==NULL || group==NULL) return(FALSE);
//--- go through all groups
   char *tok_start=grouplist,end;
   int  res=TRUE,deep=0,normal_mode;
   while(*tok_start!=0)
     {
      //--- skip ,
      while(*tok_start==',') tok_start++;
      //---
      if(*tok_start=='!') { tok_start++; normal_mode=FALSE; }
      else                 normal_mode=TRUE;
      //--- find token boundary
      char *tok_end=tok_start;
      while(*tok_end!=',' && *tok_end!=0) tok_end++;
      end=*tok_end; *tok_end=NULL;
      //---
      char *tp=tok_start;
      const char *gp=group;
      char *prev=NULL;
      //--- go through token
      res=TRUE;
      while(tp!=tok_end && *gp!=NULL)
        {
         //--- found star? check template
         if(*tp=='*')
           {
            deep=0;
            if((res=CheckTemplate(tp,tok_end,gp,prev,&deep))==TRUE)
              {
               *tok_end=end;
               return(normal_mode);
              }
            break;
           }
         //--- simple check
         if(*tp!=*gp) { *tok_end=end; res=FALSE; break; }
         tp++; gp++;
        }
      //--- restore
      *tok_end=end;
      //--- check, we have found the exact group and all is good?
      if(*gp==NULL && (tp==tok_end || *tp=='*') && res==TRUE) return(normal_mode);
      //--- transition to a following token
      if(*tok_end==0) break;
      tok_start=tok_end+1;
     }
//--- no
   return(FALSE);
  }


CSync m_sync;