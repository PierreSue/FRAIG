/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Implement memeber functions for class CirMgr  cout

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};


/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine const (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}


/*******************************/
/*   Global datas and function */
/*******************************/
CirMap _CirMap;
SatMap _SatMap;

void erasefirst(string& s)
{
  std::vector<char> c;
  for (unsigned i = 1; i < s.length(); ++i)
  {
    c.push_back(s[i]);
  }
  string tmp = "";
  for (unsigned i = 0; i < c.size(); ++i)
  {
    tmp += c.at(i);
  }
  s = tmp;
}

bool upsort(pair<int, bool> p, pair<int, bool> q)
{
  return p.first <= q.first;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool
CirMgr::readCircuit(const string& fileName)
{
   /************  Initialize Static  ************/
  for (int i = 0; i < 5; ++i) { MILOA[i]=0;}
  _CirMap.clear();
  _SatMap.clear();
  storage.clear();
  gates.clear();
  /************  Error Detection  ************/
  /************  Read Data  ************/
  fstream fs(fileName.c_str());
  string line ;
  if(!fs.is_open()) { cout << "Cannot open design \"" << fileName << "\"!!" << endl; return false ;} 
  while(getline(fs,line)) { storage.push_back(line); }
  if(storage.at(storage.size()-1).length() == 0) { storage.pop_back(); }
  /************  Construct Gate  ************/
  Constr_MILOA();
  Constr_PI();
  Constr_PO();
  Constr_AIG();
  if(!check_Const) 
  {
    CirGate* g = new CirGate(0, 0,CONST_GATE);
    _CirMap.insert( CirPair(0,g) );
  }
  Constr_CON();
  Constr_UDF();
  Constr_NICK();
  dfT();
  fecGrps.init(MILOA[4]+1);
  return true;
}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void
CirMgr::printSummary() const
{
  int PI_NO= 0, PO_NO= 0, AIG_NO= 0;
  for (CirMap::iterator i = _CirMap.begin(); i != _CirMap.end(); ++i)
  {
    if(i->second->gettype()== PI_GATE) { PI_NO++; }
    else if(i->second->gettype() == PO_GATE) { PO_NO++; }
    else if(i->second->gettype() == AIG_GATE && !i->second->bomb_undefine) { AIG_NO++; }
  }

  cout << "Circuit Statistics" << endl
       << "==================" << endl
       << "  PI" << right << setw(12) << PI_NO << endl
       << "  PO" << right << setw(12) << PO_NO << endl
       << "  AIG" << right << setw(11) << AIG_NO << endl
       << "------------------" << endl
       << "  Total" << right << setw(9) << PI_NO + PO_NO + AIG_NO << endl; 
}

void
CirMgr::printNetlist()
{
  dfT();
  cout<<endl;
  int times = 0 ;
  for (unsigned i = 0; i < trace.size() ; ++i)
  {
    if(trace.at(i)->printnet(times))
    {
      times ++ ;
    }
  }
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   for (CirMap::iterator i = _CirMap.begin(); i != _CirMap.end(); ++i)
   {
      if(i->second->gettype()== PI_GATE) { cout << i->second->getID()<< " "; }
   }
   cout << endl;
 }

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for (CirMap::iterator i = _CirMap.begin(); i != _CirMap.end(); ++i)
   {
      if(i->second->gettype()== PO_GATE) { cout << i->second->getID()<< " "; }
   }
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
  std::vector<CirGate* > flfanin;
  std::vector<CirGate* > nused;
  for (CirMap::iterator i = _CirMap.begin(); i != _CirMap.end(); ++i)
  {
    if(i->second->getfan1()!=0 && i->second->getfan2()!=0){
      if(i->second->getfan1()->getgate()->floatfanin || i->second->getfan2()->getgate()->floatfanin) 
        flfanin.push_back(i->second);
    }
    else if(i->second->getfan1()!=0 && i->second->getfan1()->getgate()->floatfanin )
      { flfanin.push_back(i->second); }
    else if(i->second->getfan2()!=0 && i->second->getfan2()->getgate()->floatfanin )
      { flfanin.push_back(i->second); }

    if(i->second->notused) { nused.push_back(i->second); }
  }

  if(flfanin.size()!= 0) {
    cout << "Gates with floating fanin(s):";
    for (unsigned i = 0; i < flfanin.size(); ++i)
    {
      cout << " " << flfanin.at(i)->getID();
    }
    cout << endl;
  }
  if(nused.size()!= 0){
    cout << "Gates defined but not used  :";
    for (unsigned i = 0; i < nused.size(); ++i)
    {
      cout << " " << nused.at(i)->getID();
    }
    cout << endl;
  }
}

void
CirMgr::printFECPairs() const
{
  int times = 0 ;
  HashMap<simValue, vector<CirGate*> >::iterator i = fecGrps.begin();
  if(!fecGrps.empty())
  {
    for (; i != fecGrps.end(); ++i)
    {
      cout << "[" << times << "]" ;
      vector< pair<int, bool> > grps; grps.clear();
      bool inverse = false;
      for (unsigned j = 0; j <  (*i).second.size(); ++j)
      {
        grps.push_back( pair<int, bool>((*i).second[j]->getID(), (*i).second[j]->check_rev));
      }
      std::sort(grps.begin(), grps.end(), upsort);
      inverse = grps[0].second;
      for (unsigned l = 0; l < grps.size(); ++l)
      {
        cout << " ";
        if(inverse) { if( !grps[l].second ) cout << "!"; }
        else { if( grps[l].second ) cout << "!"; }
        cout << grps[l].first;
      }
      times ++;
      cout << endl;
    }

  }
  
}

void
CirMgr::writeAag(ostream& outfile) const
{
  outfile << "aag " << MILOA[0] << " " << MILOA[1] << " " << MILOA[2] << " " << MILOA[3] << " " << MILOA[4] << endl;
   for (unsigned i = 0; i < trace.size(); ++i) {
    if(trace.at(i)->gettype() == PI_GATE)
      outfile << trace[i]->getID() * 2 << endl;
   }
   for (unsigned i = 0; i < trace.size(); ++i) {
    if(trace.at(i)->_fanoutlist[0]->gettype() == PO_GATE)
      outfile << (trace[i]->getID() * 2 + trace.at(i)->_fanoutlist[0]->getfan1()->getinv()) << endl;
   }
   for (unsigned i = 0; i < trace.size(); ++i) {
    if(trace.at(i)->gettype() == AIG_GATE)
    {
      outfile << (trace.at(i)->getID() * 2) << " ";
      outfile << (trace.at(i)->getfan1()->getgate()->getID() * 2 + trace.at(i)->getfan1()->getinv()) << " ";
      outfile << (trace.at(i)->getfan2()->getgate()->getID() * 2 + trace.at(i)->getfan2()->getinv()) << endl;
    }
   }
   for (unsigned i = MILOA[0]+MILOA[1]+MILOA[2]+MILOA[3]+MILOA[4]; i < storage.size(); ++i)
   {
    outfile << storage.at(i) << endl;
   }
   outfile << "c" << endl << "Outfiled by Pierre! " << endl;
}

void
CirMgr::writeGate(ostream& outfile, CirGate *g)
{
  vector<CirGate* > gatelist;
  gatedft(g, gatelist);
  int Max = 0;
  int Input = 0;
  int Aig = 0;
  for (unsigned i = 0; i < gatelist.size(); ++i)
  {
    if(gatelist.at(i)->gettype() == PI_GATE) { Input++; }
    else if(gatelist.at(i)->gettype() == AIG_GATE) { Aig++; }
    if(gatelist.at(i)->getID() > Max ) { Max = gatelist.at(i)->getID(); }
  }
  outfile << "aag " << Max << " " << Input << " " << 0 << " " << 1 << " " << Aig << endl;
  for (unsigned i = 0; i < gatelist.size(); ++i)
  {
    if(gatelist.at(i)->gettype() == PI_GATE)
      outfile << gatelist.at(i)->getID()* 2 << endl;
  }
  outfile << g->getID()* 2 << endl;
  for (unsigned i = 0; i < gatelist.size(); ++i)
  {
    if(gatelist.at(i)->gettype() == AIG_GATE)
    {
      outfile << (gatelist.at(i)->getID() * 2) << " ";
      outfile << (gatelist.at(i)->getfan1()->getgate()->getID() * 2 + gatelist.at(i)->getfan1()->getinv()) << " ";
      outfile << (gatelist.at(i)->getfan2()->getgate()->getID() * 2 + gatelist.at(i)->getfan2()->getinv()) << endl;
    }
  }
  int times = 0;
  for (unsigned i = 0; i < gatelist.size(); ++i)
  {
    if(gatelist.at(i)->gettype() == PI_GATE)
      if(gatelist.at(i)->nickname != "")
      {
        outfile << "i" << times << " " << gatelist.at(i)->nickname << endl;
        times ++ ;
      }
  }
  outfile << "o0 " << g->getID() << endl;
  outfile << "c" << endl << "Outfiled by Pierre! " << endl;
}

void
CirMgr::gatedft(CirGate* g, vector<CirGate* >& l)
{
  if(g->getfan1() != 0 && !checkflag(g->getfan1()->getgate()))
    { gatedft( g->getfan1()->getgate(), l);}
  if(g->getfan2() != 0 && !checkflag(g->getfan2()->getgate()))
    { gatedft( g->getfan2()->getgate(), l); }
  l.push_back(g);
  setflag(g);
} 

void 
CirMgr::insertg(int lineNO, int num, GateType gate, int for_PO)
{
  lineNO++;
  int gateid = num/2;
  switch(gate) {
    case PI_GATE:
        {
          CirGate* g = new CirGate(lineNO, gateid, gate);
          _CirMap.insert( CirPair(gateid,g) );
          break;
        }
    case AIG_GATE:
        {
          CirGate* g;
          if(gateid == 0)
          {
            g = new CirGate(lineNO,gateid,CONST_GATE);
            _CirMap.insert( CirPair(gateid,g) );
          }
          else{
            g = new CirGate(lineNO,gateid, gate);
            _CirMap.insert( CirPair(gateid,g) );
          }
          if(for_PO!=0)
          {
            CirGate* po = new CirGate(lineNO, for_PO,PO_GATE);
            _CirMap.insert( CirPair(for_PO,po));
            CirPin* pin = new CirPin((num%2==0)?false:true, g );
            po->setfanin(pin ,0);
            g-> _fanoutlist.push_back( po);
          } 
          break;
        }
    case CONST_GATE:
        {
          CirGate* g = new CirGate(0, 0,gate);
          _CirMap.insert( CirPair(0,g) );
          check_Const = true;
          break;
        }
  }
}


bool 
CirMgr::lexOptions(const string& option, vector<string>& tokens, size_t nOpts) const
{
   string token;
   size_t n = myStrGetTok(option, token);
   while (token.size()) {
      tokens.push_back(token);
      n = myStrGetTok(option, token, n);
   }
   return true;
}


vector<int> 
CirMgr::lex2int(const string& str) const
{
  std::vector<string> v;
  std::vector<int> tmp;
  lexOptions(str,v);
  for (std::vector<string>::iterator it = v.begin(); it != v.end(); ++it)
  {
    int num;
    if( myStr2Int(*(it), num) ) tmp.push_back( num );
  }
  return tmp; 
}


void 
CirMgr::setflag(CirGate* g) 
{ 
  g->flag = globalflag; 
}


bool 
CirMgr::checkflag(CirGate* g) 
{ 
  return ( g->flag == globalflag);
}


void 
CirMgr::dfT()
{
  vector<CirGate *> gate ;
  for (CirMap::iterator i = _CirMap.begin(); i != _CirMap.end(); ++i)
  {
    if(i->second->gettype()==PO_GATE) { gate.push_back(i->second); }
    i->second->sweepdfT = false;
  }
  trace.clear();
  for (unsigned i = 0; i < gate.size(); ++i)
  {
    dftraversal(gate.at(i));
  }
  globalflag++;
}


void
CirMgr::dftraversal(CirGate* g)
{
  if(g->getfan1() != 0 && !checkflag(g->getfan1()->getgate()))
    { dftraversal( g->getfan1()->getgate());}
  if(g->getfan2() != 0 && !checkflag(g->getfan2()->getgate()))
    { dftraversal( g->getfan2()->getgate()); }
  trace.push_back(g);
  g->sweepdfT = true;
  setflag(g);
} 


CirGate* 
CirMgr::getGate(unsigned gid) const
{ 
  for (CirMap::iterator i = _CirMap.begin(); i != _CirMap.end(); ++i)
  {
    if(i->second->getID() == gid) return i->second; 
  }
  return 0 ;
}


void 
CirMgr::Constr_MILOA()
{
  vector<string> options;
  lexOptions(storage.at(0), options);
  for(int i =0 ; i<5 ; i++) { myStr2Int(options.at(i+1),MILOA[i]); }
}


void 
CirMgr::Constr_PI()
{
  for (int i = 1; i < 1+MILOA[1]; ++i)
  {
    int GateNum;
    if( myStr2Int( storage.at(i),GateNum ) ) insertg(i,GateNum,PI_GATE);
  }
}


void 
CirMgr::Constr_PO()
{
  for (int i = 1 ; i < MILOA[3]+1; ++i)
  {
    int GateNum;
    if( myStr2Int( storage.at(i+MILOA[1]),GateNum ) ) 
    {
      bool check_PI = false;
      CirGate* Pi = 0;
      for (CirMap::iterator it = _CirMap.begin(); it != _CirMap.end(); ++it)
      {
        if(it->second->getID() == GateNum/2) { check_PI = true; Pi = it->second; break; }
      }
      // check whether input to output
      if(check_PI)
      {
        CirGate* po = new CirGate(i+MILOA[1], MILOA[0]+i, PO_GATE);
        _CirMap.insert( CirPair(MILOA[0]+i,po));
        CirPin* pin = new CirPin((GateNum%2==0)?false:true, Pi );
        po->setfanin(pin ,0);
        Pi-> _fanoutlist.push_back( po);
      }
      else insertg(i+MILOA[1], GateNum, AIG_GATE, MILOA[0]+i);
    }
  }
}


void 
CirMgr::Constr_AIG()
{
  for (int i = 1+MILOA[1]+MILOA[3]; i < 1+MILOA[1]+MILOA[3]+MILOA[4]; ++i) //AIGs
  {
    vector<int> options = lex2int( storage.at(i) );
    for ( unsigned j = 0; j < options.size(); ++j)
    {
      if( options.at(j) == 0 || options.at(j) == 1 ) { insertg(i,0,CONST_GATE); }
      else { insertg(i,options.at(j),AIG_GATE);}
    }
  }
}


void 
CirMgr::Constr_CON()
{
  for (int i = 1+MILOA[1]+MILOA[3]; i < 1+MILOA[1]+MILOA[3]+MILOA[4]; ++i)
  {
    vector<int> options = lex2int( storage.at(i) );
    /******  fanin  ******/
    CirPin* p = new CirPin( ( options.at(1)%2==0 )?false:true ,_CirMap[options.at(1)/2]);
    CirPin* q = new CirPin( ( options.at(2)%2==0 )?false:true ,_CirMap[options.at(2)/2]);
    _CirMap[options.at(0)/2]-> setfanin(p,q);
    /******  fanout  ******/
    _CirMap[options.at(1)/2]->_fanoutlist.push_back(_CirMap[options.at(0)/2]);
    _CirMap[options.at(2)/2]->_fanoutlist.push_back(_CirMap[options.at(0)/2]);
  }
  for (int i = MILOA[1]+MILOA[3]+MILOA[4]; i >= 1+MILOA[1]+MILOA[3]; --i)
  {
    vector<int> options = lex2int( storage.at(i) );
    _CirMap[options.at(0)/2]->line = i+1;
  }
}


void 
CirMgr::Constr_UDF()
{
  //cout << _CirMap.size() << endl;
  for (CirMap::iterator i = _CirMap.begin(); i != _CirMap.end(); ++i)
  {
    //cout << i->second->getID();
    //std::vector<CirGate*> v;
    //v = i-> second-> _fanoutlist;
    i->second->floatfanin = false;
    i->second->notused = false;
    i->second->bomb_undefine = false;
    if(i-> second-> gettype() == AIG_GATE && i-> second->getfan1() == 0 && i-> second->getfan2() == 0)
    {
      i->second->bomb_undefine = true;
    }
    if(i-> second-> gettype() == AIG_GATE && i-> second->getfan1() == 0) 
      { i-> second->floatfanin = true;}
    if(i-> second-> gettype() == AIG_GATE && i-> second->getfan2() == 0) 
      { i-> second->floatfanin = true; }
    if(i-> second-> gettype() == PO_GATE  && i-> second->getfan1()->getgate() -> undefine()) 
      { i-> second->floatfanin = true; } 
    if( i-> second-> _fanoutlist.size() == 0 
     && i-> second-> gettype() != PO_GATE && i-> second-> gettype() != CONST_GATE)
      { i-> second->notused = true; }
    //cout << endl;
  }
  
}


void 
CirMgr::Constr_NICK()
{
  for (unsigned i = MILOA[1]+MILOA[3]+MILOA[4]+1; i <storage.size(); ++i)
  {
    vector<string> options;
    lexOptions(storage.at(i), options);
    if( myStrNCmp(options.at(0),"c",1)==0 ) { break; }
    if( myStrNCmp(options.at(0),"i",1)==0) 
    {
      erasefirst(options.at(0));
      int num =0;
      if(!myStr2Int(options.at(0),num)) { cout<< "Something wrong in ILO!!!!"<<endl;}
      else { 
        int no =0;
        for (CirMap::iterator i = _CirMap.begin(); i != _CirMap.end(); ++i)
        {
          if(i->second->gettype() == PI_GATE) { 
            if(no == num) {i->second->nickname = options.at(1); break; }
            no++; }
        }
      } 
    }
    if( myStrNCmp(options.at(0),"o",1)==0) 
    {
      erasefirst(options.at(0));
      int num =0;
      int no = 0;
      if(!myStr2Int(options.at(0),num)) { cout<< "Something wrong in ILO!!!!"<<endl;}
      for (CirMap::iterator i = _CirMap.begin(); i != _CirMap.end(); ++i)
      {
        if(i->second->gettype() == PO_GATE) { 
          if(no == num) { i->second->nickname = options.at(1); break; }
          no++; 
        }
      }
    }
  }
}

