/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
	
#include <cassert>
#include <algorithm>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions 

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed


/************************************************/
/*   Functions about Strash                		*/
/************************************************/
class HashKey
{
public:
   HashKey(CirPin* g1, CirPin* g2 ) { fan1 = g1; fan2 = g2; }

   size_t operator() () const { return ( fan1->getgate()->getID()+1 )*(fan2->getgate()->getID()+1);  }

   // the situation that 1 and 2 are swapped
   // also check the inverse have to be the same 
   bool operator == (const HashKey& k) const 
   { 
      if(fan1->getgate() == k.getfan1()->getgate() && fan2->getgate() == k.getfan2()->getgate() )
      {
         if(fan1->getinv() == k.getfan1()->getinv() && fan2->getinv() == k.getfan2()->getinv())
         {
            return true; 
         }
      }
      else if(fan1->getgate() == k.getfan2()->getgate() && fan2->getgate() == k.getfan1()->getgate() )
      {
         if(fan1->getinv() == k.getfan2()->getinv() && fan2->getinv() == k.getfan1()->getinv())
         {
            return true; 
         }
      }
      return false;
   }
   int test() {return fan1->getgate()->getID()+fan2->getgate()->getID();}

   CirPin* getfan1() const {return fan1;} 
   CirPin* getfan2() const {return fan2;} 

private:
   CirPin* fan1;
   CirPin* fan2;
};


void
CirMgr::strash()
{
	vector<CirGate*> _dellist;
	_dellist.clear();
	HashMap<HashKey, CirGate*> _hashmap(MILOA[0]);
	dfT();
	// find the same input(query)
	// replace_strash to reconstruct the relation
	// save it to the _dellist
	for (unsigned i = 0; i < trace.size(); ++i)
	{
		if(trace.at(i)->gettype() == AIG_GATE && !trace.at(i)->undefine())
		{
			if(trace.at(i)->getfan1()!= 0 && trace.at(i)->getfan2()!=0)
			{
				CirGate* gate;
				HashKey tmp(trace.at(i)->getfan1(), trace.at(i)->getfan2());
				if(_hashmap.query(tmp, gate))
				{
					trace.at(i)->replace_strash(gate);
					_dellist.push_back(trace.at(i));
				}
				else _hashmap.insert(tmp, trace.at(i));
			}
		}
	}
	// erase the same gate
	for (unsigned i = 0; i < _dellist.size(); ++i)
	{
		_CirMap.erase(_dellist.at(i)->getID());
	}
	Constr_UDF();
}


/************************************************/
/*   Functions about Fraig                      */
/************************************************/
void
CirMgr::fraig()
{
   initCircuit();

   SatSolver solver;
   solver.initialize();

   //
   genProofModel(solver);

   bool result;

   int times = 0;
   while(fecGrps.size() > 0 && times < 2){
   		vector<simValue > _simlist;
   		_simlist.clear();
   		HashMap<simValue, vector<CirGate*> >::iterator it = fecGrps.begin();
   		for (; it != fecGrps.end(); ++it)
   		{ 
   			if((*it).second.size()> 100 && tricky ) { continue; }
   	 		if((*it).second.size() == 2 )
   	 		{
   	 			Var newV = solver.newVar();
   	 			solver.addXorCNF(newV, _SatMap[(*it).second[0]], (*it).second[0]->check_rev
   	 						 		 , _SatMap[(*it).second[1]], (*it).second[1]->check_rev);
  	 			solver.assumeRelease();  // Clear assumptions
   	 			solver.assumeProperty(newV, true);  // k = 1
   	 			solver.assumeProperty(_SatMap[ _CirMap[0] ], false);
     			result = solver.assumpSolve();
     			//reportResult(solver, result);
     			(*it).second[1]->fraiged = true;
     			(*it).second[0]->fraiged = true;
     			if(!result){
     				(*it).second[1]->replace_fraig((*it).second[0], (*it).second[1]->check_rev, (*it).second[0]->check_rev);
     				_CirMap.erase((*it).second[1]->getID());
     			}
     			_simlist.push_back((*it).first);
   	 		}
   	 		else{
   	 			vector<CirGate* > _dellist;
   	 			_dellist.clear();
   	 			for (unsigned i = 1; i < (*it).second.size(); ++i)
   	 			{
   	 				Var newV = solver.newVar();
   	 				solver.addXorCNF(newV, _SatMap[(*it).second[0]], (*it).second[0]->check_rev
   	 						 	 , _SatMap[(*it).second[i]], (*it).second[i]->check_rev);
  	 				solver.assumeRelease();  // Clear assumptions
   	 				solver.assumeProperty(newV, true);  // k = 1
   	 				solver.assumeProperty(_SatMap[ _CirMap[0] ], false);
   	 				// false -> unsat
     				result = solver.assumpSolve();
     				//reportResult(solver, result);
     				if(!result) { _dellist.push_back((*it).second[i]); (*it).second[i]->fraiged = true; }
   	 			}
   	 			// replace _dellist
   	 			if(_dellist.size() != 0)
   				{
					for (unsigned i = 0; i < _dellist.size(); ++i)
					{
						std::vector<CirGate* >::iterator iter = 
							find ((*it).second.begin(), (*it).second.end(), _dellist.at(i));
						if( iter != (*it).second.end() )
						{
							(*it).second.erase(iter);
						}
						_dellist.at(i)->replace_fraig((*it).second[0], _dellist.at(i)->check_rev, (*it).second[0]->check_rev);
						_CirMap.erase(_dellist.at(i)->getID());
					}
				} 
				(*it).second[0]->fraiged = true;
   				(*it).second.erase( (*it).second.begin() );
   				if((*it).second.size() == 0 || (*it).second.size() == 1) {_simlist.push_back((*it).first);} 
   			}
   	 	}
   		//delete _simlist
   		if(_simlist.size() != 0)
   		{
			for (unsigned i = 0; i < _simlist.size(); ++i)
			{
				if(!fecGrps.remove(_simlist.at(i))) cout << "error"<< endl;
			} 
   		} 
   		times ++;
   		//cout <<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<< times << fecGrps.size() << endl;
   	}
   	Constr_UDF();
}


//push_back all the gates except PO_Gate
void 
CirMgr::initCircuit()
{
	dfT();
	gates.clear();
	_SatMap.clear();
	for (unsigned i = 0; i < trace.size(); ++i)
	{
		if (trace.at(i)->gettype() != PO_GATE)
		{ 
			gates.push_back(trace.at(i)); 
		}
	}
}


//addAigCNF to construct the relation
void 
CirMgr::genProofModel(SatSolver& s)
{
	for (size_t i = 0, n = gates.size(); i < n; ++i) {
      	Var v = s.newVar();
      	_SatMap.insert( SatPair(gates.at(i),v) );
   	}
   	for (unsigned i = 0; i < gates.size(); ++i)
   	{
   		if(gates[i]->gettype() == AIG_GATE && gates[i]->checkfanin())
   		{
   			s.addAigCNF( _SatMap[gates[i]], _SatMap[gates[i]->getfan1()->getgate()], gates[i]->getfan1()->getinv(),
               _SatMap[gates[i]->getfan2()->getgate()], gates[i]->getfan2()->getinv() );
   		}
   	}
}


//print the result sat or not
void 
CirMgr::reportResult(const SatSolver& solver, bool result)
{
	//solver.printStats();
   	cout << (result? "SAT" : "UNSAT") << endl;
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/
