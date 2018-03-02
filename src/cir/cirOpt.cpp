/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;
extern CirMap _CirMap;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates 
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void 
CirMgr::sweep()
{
  bool check = true;
  while(check)
  {
    dfT();
    //cout << _CirMap.size() << endl;
    for (CirMap::iterator i = _CirMap.begin(); i != _CirMap.end(); ++i)
    {
      //cout << i->second->getID();
      if(!i->second->sweepdfT && i->second->gettype() ==AIG_GATE ) 
      {
        i->second->replace();
        if(i->second->gettype() == AIG_GATE ) _CirMap.erase(i->second->getID());
        cout << "Sweeping: " << i->second->getTypeStr() << "(" << i->second->getID() << ")" << "removed..." << endl;
        check = false;
      }
    }
    Constr_UDF();
    check = !check;
  }
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void
CirMgr::optimize()
{
  std::vector<CirGate*> _dellist;
  for (unsigned i = 0; i < trace.size(); ++i)
  {
  	if(trace.at(i)->getfan1() != 0 && trace.at(i)->getfan2() != 0 )
  	{
  		if(trace.at(i)->getfan1()->getgate()->getID() == trace.at(i)->getfan2()->getgate()->getID())
  		{
  			if(trace.at(i)->getfan1()->getinv() == trace.at(i)->getfan2()->getinv()) 
  				{ trace.at(i)->replace_same(false);} // a a
  			else{ trace.at(i)->replace_same(true);} // a -a 
  			_dellist.push_back(trace.at(i));
  		}
  		else if(trace.at(i)->getfan1()->getgate()->getID() == 0)
  		{
  			if(trace.at(i)->getfan1()->getinv() == false) 
  				{ _dellist.push_back(trace.at(i)); trace.at(i)->replace_Const(true,false);} // 0 a
  			else if(trace.at(i)->getfan1()->getinv() == true) 
  				{ _dellist.push_back(trace.at(i)); trace.at(i)->replace_Const(true,true);} // 1 a
  		}
  		else if(trace.at(i)->getfan2()->getgate()->getID() == 0)
  		{
  			if(trace.at(i)->getfan2()->getinv() == false) 
  			{ _dellist.push_back(trace.at(i)); trace.at(i)->replace_Const(false,false);} // a 0
  			else if(trace.at(i)->getfan2()->getinv() == true) 
  			{ _dellist.push_back(trace.at(i)); trace.at(i)->replace_Const(false,true);} // a 1 
  		}
  	}
  }
  for (unsigned i = 0; i < _dellist.size(); ++i)
  {
  	if(_dellist.at(i)->gettype() == AIG_GATE ) _CirMap.erase(_dellist.at(i)->getID());
  }
  Constr_UDF();
}


/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
