/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include "cirDef.h"
#include "sat.h"

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.


class CirGate;


string int2str(int);
//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
// TODO: Define your own data members and member functions, or classes 
class CirPin
{
public:
  CirPin(bool a, CirGate* g): inv(a), gate(g){}; 
  ~CirPin();

  // Data functions
  bool getinv() { return inv; }
  CirGate* getgate() { return gate; }
  // check the inverse problem (optimization)
  void setgate(CirGate* g, bool b) 
  {
     gate = g;
     if(b == false && inv == false) {inv = false;}
     else if(b == true && inv == true) {inv = false;}
     else inv = true; 
  }

private:
  bool inv;
  CirGate* gate; 

};


class CirGate
{
public:
   CirGate(int lineNO, int num, GateType type, CirPin* p =0, CirPin* q =0, bool b = false ) 
   : line(lineNO), floatfanin(b), notused(b), _gateID(num), fraiged(false), bomb_undefine(false),
    _type(type), sweepdfT(false), _FECNum(0), check_rev(false), edge(false)
   { setfanin(p, q); flag =0 ;nickname = "";}
   ~CirGate() {}

   // Data members
   int flag;
   int line;
   bool sweepdfT;
   string nickname;

   // Gate informations
   string getTypeStr() const ;
   int getID() const { return _gateID; }
   GateType gettype() const { return _type;}
   unsigned getLineNo() const { return line; }

   // Basic access methods
   bool isAig() const { return gettype() == AIG_GATE; }
   void replace();
   void delete_fanin_par(bool lr, CirGate* g);
   void replace_Const(bool lr, bool _inv); // true-> left false-> right
   void replace_same(bool _inv); // true ->both reverse
   void replace_strash(CirGate* g);
   void replace_fraig(CirGate* g, bool a, bool b);
   void add_parent(CirGate* g);

   // Printing functions
   void reportGate() ;
   bool printnet(int times) const;
   void printGate() const;
   void reportFanin(int level, bool check = true) const;
   void reportFanout(int level, bool check = true) const;
   
   // Undefine information
   bool notused;
   bool floatfanin;
   bool bomb_undefine;
   bool undefine() const { return ( floatfanin || notused ); };

   // fanin fanout information
   vector<CirGate*> _fanoutlist;
   CirPin* getfan1() const { return _faninlist[0]; };
   CirPin* getfan2() const { return _faninlist[1]; };
   bool checkfanin() { return (_faninlist[0]!=0 && _faninlist[1]!=0); }
   void setfanin( CirPin* p, CirPin*q)
   {
    _faninlist[0]=p;
    _faninlist[1]=q;
   }
   CirPin* getfanin(bool lr) const
   {
      if(lr) return getfan1();
      else return getfan2(); 
   }

   //simulation by FEC Group
   unsigned _FECNum;
   unsigned get_rev_num() { return ~_FECNum; }
   bool check_rev;
   bool edge;
   void simFEC();
   bool fraiged;
   bool simmed;

private:
  int _gateID;
  GateType _type;
  CirPin* _faninlist[2];
};




#endif // CIR_GATE_H
