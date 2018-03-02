/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <map> 
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirDef.h"

extern CirMgr *cirMgr;
extern CirMap _CirMap;
extern SatMap _SatMap;
static int globalflag = 1;
void erasefirst(string&);


class simValue
{
public:
   simValue(unsigned l) { value = l;}

   unsigned operator() () const { return value; }

   bool operator == (const simValue& k) const { return ( value == k.value ); }

   unsigned getvalue() {return value;};
private:
   unsigned value;
};


class CirMgr
{
public:
   CirMgr() { check_Const = false; tricky = false; }
   ~CirMgr() {} 

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const;

   // Member functions about circuit construction
   bool readCircuit(const string&);
   void Constr_MILOA();
   void Constr_PI();
   void Constr_PO();
   void Constr_AIG();
   void Constr_CON();
   void Constr_UDF();
   void Constr_NICK();
   void insertg(int, int, GateType, int = 0);

   // Member functions about circuit optimization
   void sweep();
   void optimize();

   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }
   void ran_num_gen(unsigned&);
   void sim_num();
   void inisim();
   void for_each( HashMap<simValue, vector<CirGate*> >& NewfecGrps);
   void ckeck_valid_Grp(HashMap<simValue, vector<CirGate*> >& NewfecGrps); 

   // Member functions about fraig
   void strash();
   void printFEC() const;
   void fraig();
   void initCircuit();
   void genProofModel(SatSolver& s);
   void reportResult(const SatSolver& solver, bool result);
   bool check_Const;

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist();
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void printFECPairs() const;
   void writeAag(ostream&) const;
   void writeGate(ostream&, CirGate*);
   void gatedft(CirGate* g, vector<CirGate* >& l);

   // Member functions about lexing
   bool lexOptions(const string&, vector<string>&, size_t nOpts = 0) const;
   vector<int> lex2int(const string&) const ;

   // Member functions about dftraversal
   void dfT();
   void dftraversal(CirGate* g);
   void setflag(CirGate* g);
   bool checkflag(CirGate* g);

private:
   ofstream           *_simLog;
   vector< CirGate* > trace;
   vector<string> storage;
   int MILOA[5];
   vector<CirGate *> gates;
   HashMap<simValue, vector<CirGate*> > fecGrps;
   bool tricky;
};

#endif // CIR_MGR_H
