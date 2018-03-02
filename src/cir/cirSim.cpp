/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirMgr::randomSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions 

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
void CTB(int n, vector<int>& v)
{
    if (n / 2 != 0) {
        CTB(n / 2, v);
    }
    if(n < 0) { n *= (-1);}
    v.push_back(n%2) ;
}
/************************************************/
/*   Public member functions about Simulation   */
/************************************************/

void
CirMgr::fileSim(ifstream& patternFile)
{
	// PI PO GATE_NO and list
	dfT();
	int PI_Num = 0 ;
    int PO_Num = 0 ;
    vector<CirGate* > PI_list;
    vector<CirGate* > PO_list;
  	for (CirMap::iterator i = _CirMap.begin(); i != _CirMap.end(); ++i)
  	{
    	if(i->second->gettype()== PI_GATE) { PI_Num++; PI_list.push_back(i->second);}
    	else if(i->second->gettype()== PO_GATE) {  PO_Num++; PO_list.push_back(i->second); }
  	}
  	// check problem && simulation
  	int times = 0 ;
  	bool check_first  = true;
  	string line ;
    vector<string> patterns;
	
	while(getline(patternFile,line)) 
	{ 
		for (int i = 0; i < line.length(); ++i)
		{
			if(line[i] == ' ') { line.erase(line.begin()+i); }
		}
		if(line.length() == 0) { continue; }
		if(line.length() != PI_Num ) 
  			{ cout << "Error: Pattern(" << line << ") length(" << line.length() 
  			   	   <<") does not match the number of inputs( " << PI_Num << ") in a circuit!!" << endl ; return;}
  	    for (unsigned j = 0; j < line.length(); ++j)
  	    {
  	    	if( line[j]!= 48 && line[j]!= 49)
  	    	{ cout << "Error: Pattern(" << line << ") contains a non-0/1 character(" 
  	               << "‘" << line[j] << "’)." << endl; return;}
  	    }
  	    patterns.push_back(line); 
  	    if(patterns.size() == 32)
  	    {
  	    	//assign to PI
  	    	for (unsigned i = 0; i < PI_list.size(); ++i)
  	    	{
  	    		PI_list[i]->_FECNum = 0;
  	    		for (int j = 0; j < 32; ++j)
  	    		{
  	    			if(patterns[j][i] == 49) { PI_list[i]->_FECNum += (unsigned)1*pow (2, 31-j); }
  	    		}

  	    	}
  	    	//assign undef const else sim (AIG && PO )
  	    	for (unsigned k = 0; k < trace.size(); ++k)
  			{
				if(trace.at(k)->gettype() == CONST_GATE || trace.at(k)->undefine()) { trace.at(k)->_FECNum = 0; }
				else if(trace.at(k)->gettype() == AIG_GATE ) { trace.at(k)->simFEC(); }
				else if(trace.at(k)->gettype() == PO_GATE) { trace.at(k)->simFEC(); }
  			}
  			//inisim
			if(check_first) 
			{
				fecGrps.init(MILOA[4]+1); 
				vector<CirGate*> glist;
				glist.push_back(_CirMap[0]);
				for (unsigned i = 0; i < trace.size(); ++i)
   				{
    				if( (trace.at(i)->gettype()== AIG_GATE && !trace.at(i)->undefine() )  ) 
      				{ glist.push_back(trace.at(i)); }
   				}
				check_first = false;
				simValue tmpvalue(0);
				fecGrps.insert(tmpvalue, glist);
			}
			//simulation -> FECgrps
			HashMap<simValue, vector<CirGate*> > NewfecGrps( MILOA[4]+1 );
			for_each(NewfecGrps);
			fecGrps = NewfecGrps ;
			//PO_result value
			vector < vector<int> > PO_result;
			for (int i = 0; i < PO_Num; ++i)
			{
				vector<int> tmp ;
				PO_result.push_back(tmp);
				CTB(PO_list.at(i)->_FECNum, PO_result.at(i) );
				for (unsigned j = PO_result.at(i).size(); j < 32; ++j)
  				{
    				PO_result.at(i).insert(PO_result.at(i).begin(), 0);
  				}
			}
			//output
			if(_simLog!= 0)
			{
				for (int i = 0; i < 32; ++i)
				{
					(*_simLog) << patterns.at(i) << " " ;
					for (int j = 0; j < PO_Num; ++j)
					{
						(*_simLog) << PO_result[j][i];
					}
					(*_simLog) << endl;
				}
			}
  	    	patterns.clear();
  	    	PO_result.clear();
  	    	times ++ ;
  	    	if(times%50 == 0) { cout << "patterns : " << times*32 << endl; }
  	    }
  	}
		
}


void 
CirMgr::ran_num_gen(unsigned& n)
{
  unsigned test = 0;
  for (int i = 0; i < 32; ++i)
  {
    unsigned k = rnGen(2);
    test += (unsigned)k*pow (2, 31-i);
  }
  n = test;
}


void
CirMgr::randomSim()
{
	int Max_Fail = 3.5*(int)sqrt((double)MILOA[1]);
	if(_CirMap.size() > 20000) { tricky = true; }
	else { tricky = false; }
	int Fail = 0;
	int patterns = 32;
	dfT();
	fecGrps.init(MILOA[4]+1);
	inisim();
	while(Fail < Max_Fail)
	{
		//printFECPairs();
		size_t tmp_size = fecGrps.size();
		sim_num();
		HashMap<simValue, vector<CirGate*> > NewfecGrps( MILOA[4]+1 );
		for_each(NewfecGrps);
		fecGrps = NewfecGrps ;
		if(fecGrps.size() >= tmp_size || fecGrps.empty()) 
		{  
			if(Fail% 5 == 0)
			{ cout <<"MAX_FAILS = " << Max_Fail << " Fails : " << Fail << endl; }
			Fail++;
			if(tricky) break;
		}
		patterns += 32;
		if(fecGrps.empty()) break;
		//cout << fecGrps.size() << " : " << patterns << " : " << Fail << endl ;
	}
	Constr_UDF();
	cout << endl;
	cout <<"MAX_FAILS = " << Max_Fail << endl;
	cout << patterns << " patterns simulated. " << endl;
}


void 
CirMgr::inisim()
{
	for (unsigned i = 0; i < trace.size(); ++i)
	{
		if(trace.at(i)->gettype() == PI_GATE || trace.at(i)->undefine() ) { ran_num_gen(trace.at(i)->_FECNum); }
		else if(trace.at(i)->gettype() == CONST_GATE ) { trace.at(i)->_FECNum = 0;}
		else if(trace.at(i)->gettype() == AIG_GATE ) { trace.at(i)->simFEC();}
	}
	if(!_CirMap[0]->fraiged)
	{
		vector<CirGate*> glist;
		simValue tmp_CONST(0);
		glist.push_back(_CirMap[0]);
		fecGrps.insert(tmp_CONST, glist);
	}
	for (unsigned i = 0; i < trace.size(); ++i)
   	{
    	if( (trace.at(i)->gettype()== AIG_GATE && !trace.at(i)->undefine() && !trace.at(i)->fraiged ) ) 
      	{ 
      		vector<CirGate*> glist;
			simValue tmp_1( trace.at(i) -> _FECNum );
			simValue tmp_2( trace.at(i) -> get_rev_num() );
			if( fecGrps.query(tmp_1, glist) )
			{
				trace.at(i) -> check_rev = false;
				glist.push_back(trace.at(i));
				fecGrps.update(tmp_1, glist);
			}
			else if( fecGrps.query(tmp_2, glist) )
			{
				trace.at(i) -> check_rev = true;
				glist.push_back(trace.at(i));
				fecGrps.update(tmp_2, glist);
			}
			else{
				trace.at(i) -> check_rev = false;
				glist.push_back(trace.at(i));
				fecGrps.quickinsert(tmp_1, glist);
			}
      	}
   	}
    ckeck_valid_Grp(fecGrps);
}


void 
CirMgr::sim_num()
{
	size_t tmp_size= fecGrps.size();
	for (unsigned i = 0; i < trace.size(); ++i)
	{
		if(trace.at(i)->gettype() == PI_GATE  || trace.at(i)->undefine()) { ran_num_gen(trace.at(i)->_FECNum); }
		else if(trace.at(i)->gettype() == CONST_GATE ) { trace.at(i)->_FECNum = 0; }
		else if(trace.at(i)->gettype() == AIG_GATE )
		{ trace.at(i)->simFEC(); }
	}
}


void 
CirMgr::for_each( HashMap<simValue, vector<CirGate*> >& NewfecGrps)
{
	HashMap<simValue, vector<CirGate*> >::iterator it = fecGrps.begin();
	for (; it != fecGrps.end(); ++it)
	{
		for (unsigned i = 0; i < (*it).second.size(); ++i)
		{
			vector<CirGate*> glist;
			simValue tmp_1( (*it).second.at(i)-> _FECNum );
			simValue tmp_2( (*it).second.at(i)-> get_rev_num() );
			if( NewfecGrps.query(tmp_1, glist) )
			{
				(*it).second.at(i) -> check_rev = false;
				glist.push_back((*it).second.at(i));
				NewfecGrps.update(tmp_1, glist);
			}
			else if( NewfecGrps.query(tmp_2, glist) )
			{
				(*it).second.at(i) -> check_rev = true;
				glist.push_back((*it).second.at(i));
				NewfecGrps.update(tmp_2, glist);
			}
			else{
				(*it).second.at(i) -> check_rev = false;
				glist.push_back((*it).second.at(i));
				NewfecGrps.quickinsert(tmp_1, glist);
			}
		}
		ckeck_valid_Grp(NewfecGrps);
	}
}


void 
CirMgr::ckeck_valid_Grp( HashMap<simValue, vector<CirGate*> >& NewfecGrps)
{
	HashMap<simValue, vector<CirGate*> >::iterator it = NewfecGrps.begin();
	vector<simValue> simlist;
	for (; it != NewfecGrps.end(); ++it)
	{
		if((*it).second.size() == 1 ) { simlist.push_back((*it).first);}
	}

	if(simlist.size() != 0)
	{
		for (unsigned i = 0; i < simlist.size(); ++i)
		{
			NewfecGrps.remove(simlist.at(i));
		} 
	}
	
}
/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/
