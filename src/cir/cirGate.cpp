/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <algorithm>
#include <stdarg.h>
#include <cassert>
#include <math.h>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirGate::reportGate()", "CirGate::reportFanin()" and 
//       "CirGate::reportFanout()" for cir cmds. Feel free to define
//       your own variables and functions.

// TODO: Implement memeber functions for class(es) in cirGate.h

/**************************************/
/*    Global functions                */
/**************************************/
string int2str(int i)
{
  string s;
  stringstream ss(s);
  ss << i;
  return ss.str();
}

void ConvertToBinary(int n, vector<int>& v)
{
    if (n / 2 != 0) {
        ConvertToBinary(n / 2, v);
    }
    if(n < 0) { n *= (-1);}
    v.push_back(n%2) ;
}
/**************************************/
/*   class CirGate member functions   */
/**************************************/
string 
CirGate::getTypeStr() const 
{ 
  if(bomb_undefine) { return "UNDEF"; }
	else if( _type == PI_GATE ) { return "PI";  }
	else if( _type == PO_GATE ) { return "PO"; }
	else if( _type == AIG_GATE ) { return "AIG"; }
	else if( _type == CONST_GATE ) { return "CONST"; } 
	else { return "UNDEF"; }
}


void 
CirGate::printGate() const
{
	if(bomb_undefine) { cout << "UNDEF" << endl;  }
	else if( _type == PI_GATE ) { cout << "PI " << _gateID ;  }
	else if( _type == PO_GATE ) { cout << "PO " << _gateID ; }
	else if( _type == AIG_GATE ) { cout << "AIG " << _gateID ; }
	else if( _type == CONST_GATE ) { cout << "CONST " << _gateID ; }   
}


bool 
CirGate::printnet(int times) const
{
    if(gettype() == PI_GATE) { cout << "["<< times <<"] ";cout << "PI"<< "  "<< getID();}
    else if(gettype() == CONST_GATE) { cout << "["<< times <<"] ";cout << "CONST0"; } 
    else if(gettype() == PO_GATE) { 
      cout << "["<< times <<"] ";
      cout<< "PO"<< "  "<< getID()<< " ";
      if(undefine()) { cout<<"*"; }
      if(getfan1()->getinv()) { cout<<"!"; }
      cout << getfan1()->getgate()->getID();
    }
    else if(gettype() == AIG_GATE)
    {
      if(getfan1() ==0 && getfan2() ==0) {return false;}
      cout << "["<< times <<"] ";
      cout<< "AIG"<< " "<< getID()<< " ";
      if(undefine()) cout<<"*";
      if(getfan1() !=0)
      { 
        if(getfan1()->getgate()->undefine()) cout<<"*";
        if(getfan1()->getinv()) cout<<"!"; 
        cout << getfan1()->getgate()->getID(); cout << " "; 
      }
      if(getfan2() !=0)
      {
        if(getfan2()->getgate()->undefine()) cout<<"*";
        if(getfan2()->getinv()) cout<<"!"; 
        cout << getfan2()->getgate()->getID();
      }
    }
    if( nickname != "") cout << " (" << nickname <<")"; 
    cout<< endl;
    return true;
}


void
CirGate::reportGate()
{
	if(undefine() && ( gettype() == AIG_GATE) ) 
    { cout << "Error: Gate(" << getID() << ") not found!!" << endl; return; }

	string s = "";
	if(gettype() == PI_GATE) 
  { 
    s = "= PI(" + int2str( getID() )+")"; 
    if(nickname != "") { s= s+"\""+nickname+"\"";  }
    s = s +", line "+ int2str(line); 
  }
	else if(gettype() == PO_GATE) 
  { 
    s = "= PO(" + int2str( getID() )+")"; 
    if(nickname != "") { s= s+"\""+nickname+"\"";  }
    s = s +", line "+ int2str(line); 
  }
	else if(gettype() == AIG_GATE) { s = "= AIG("+int2str( getID() )+"), line "+int2str(line); }
	else if(gettype() == CONST_GATE) { s = "= CONST("+int2str( getID() )+"), line "+int2str(line); }

  vector<int> fec;  fec.clear();
  vector< pair<int, bool> > grps; grps.clear();
  
  ConvertToBinary(_FECNum, fec);
  for (unsigned i = fec.size(); i < 32; ++i)
  {
    fec.insert(fec.begin(), 0);
  }
  cout << endl;


  CirMap::iterator i = _CirMap.begin();
  for (; i != _CirMap.end(); ++i)
  {
    if(!simmed) { continue; }
    else if(fraiged) { continue; }
    else if(i->second->undefine()) {continue;}
    else if(i->second->getID() == getID()) {continue;}
    else if((!i->second->simmed) && (i->second->gettype()!= CONST_GATE) ) { continue; }
    else if(i->second->gettype()!= AIG_GATE && i->second->gettype()!= CONST_GATE ) {continue;}
    else if(i->second->_FECNum == _FECNum) { grps.push_back( pair<int, bool>(i->second->getID(), false) ); }
    else if(i->second->get_rev_num() == _FECNum) { grps.push_back( pair<int, bool>(i->second->getID(), true) ); }
  }

	cout << "==================================================" << endl
		 << left << setw(49) << s << "=" << endl;

  {
    cout << "= FECs:";;
    int num = 7;
    for (unsigned i = 0; i < grps.size(); ++i)
    {
       if(grps[i].first < 10){ num+=2; }
       else if(grps[i].first < 100) { num+=3; } 
       else if(grps[i].first < 1000) { num+=4;} 
       else if(grps[i].first < 10000) { num+=5; } 
       else if(grps[i].first < 100000) { num+=6; }
       else if(grps[i].first < 1000000) { num+=7; }
       cout << " ";
       if(grps[i].second){ num +=1; cout << "!";  }
       cout << grps[i].first;
    } 
    if(num >=49 ) {cout << " =" << endl;}
    else {
      for (unsigned i = num; i < 49; ++i)
      {
        cout << " ";
      }
      cout << "=" << endl;;
    }
  }

  {
    cout << "= Value: ";
    for (unsigned i = 0; i < fec.size(); ++i)
    {
      if(i%4 == 0 && i!= 0) cout << "_";
      cout << fec[i];
    }
    cout << " ="<<endl;
  }
  
	cout << "==================================================" << endl;
}


void
CirGate::reportFanin(int level,bool check) const
{
   static int times = 1;
   static vector<int> vec;
   assert (level >= 0);
   bool che[3];
   for (int i = 0; i < 3; ++i) { che[i] = false; }

   for (unsigned i = 0; i < vec.size(); ++i)
   {
   		if( getfan1() !=0 ) { if(getfan1()->getgate()->getID() == vec.at(i) ) { che[0] = true;} }
   		if( getfan2() !=0 ) { if(getfan2()->getgate()->getID() == vec.at(i) ) { che[1] = true;} }
   		if( getID() == vec.at(i)) { che[2] = true;}
   }
   if( che[0] && che[1] && che[2] && level !=0) {printGate(); cout << " (*)" << endl; return;}
   else { printGate(); cout << endl; vec.push_back(getID());}

   int tmp = times;
   if( getfan1()!=0 && level !=0 ) 
   { 
   		for (int i = 0; i < times; ++i)
   	    {
   	    	cout<<"  ";
   	    }
   	    times++;
   	    if(getfan1()->getinv())cout<<"!"; getfan1()->getgate()->reportFanin(level-1,false); 
   }
   times = tmp;
   if( getfan2()!=0 && level !=0 ) 
   { 
   	    for (int i = 0; i < times; ++i)
   	    {
   	    	cout<<"  ";
   	    }
   	    times++;
   	    if(getfan2()->getinv())cout<<"!"; getfan2()->getgate()->reportFanin(level-1,false); 
   }
   if(check) { times = 1; vec.clear(); } 
}


void
CirGate::reportFanout(int level, bool check) const
{
   static int times = 1;
   static vector<int> vec;
   assert (level >= 0);
   std::vector<CirGate*> v;
   v = _fanoutlist;

   bool che[ v.size()+1 ];
   bool finalcheck = true;
   for (unsigned i = 0; i < v.size()+1; ++i) { che[i] = false; }

   for (unsigned i = 0; i < vec.size(); ++i)
   {
   		for (unsigned j = 0; j < v.size(); ++j)
   		{
   			if( v.at(j)->getID() == vec.at(i)) { che[j] =true; }
   		}
   		if(getID() == vec.at(i)) { che[v.size()]=true; }
   }
   for (unsigned i = 0; i < v.size()+1; ++i)
   {
   		if(che[i] == false) { finalcheck = false;}
   }

   if( finalcheck && level > 0) {printGate(); cout << " (*)" << endl; return;}
   else { printGate(); cout << endl; vec.push_back(getID());}

   int tmp = times;
   if(level > 0)
   {
   	for (unsigned i = 0; i < v.size(); ++i)
   	{
   		for (int j = 0; j < times; ++j)
   	    {
   	    	cout<<"  ";
   	    }
   	    times++;
   		if(v.at(i)->getfan1()->getgate()->getID() == getID() ) 
   			 { if(v.at(i)->getfan1()->getinv())cout<<"!"; }
   		else { if(v.at(i)->getfan2()->getinv())cout<<"!";}
   		v.at(i)->reportFanout(level-1,false);
   		times = tmp;
   	}
   }
   if(check) { times = 1; vec.clear(); }
}


void 
CirGate::replace()
{
  if(getfan1() != 0)
  {
    delete_fanin_par(true, this);
  }
  if(getfan2() != 0)
  {
    delete_fanin_par(false, this);
  }
}


void
CirGate::replace_Const(bool lr, bool _inv)
{
  if(!_inv)
  {
    //cout << "1. " <<endl;
    //(1)for(i->parents) pop_back(node); parents fannin = Const0;
    for (unsigned i = 0; i < _fanoutlist.size(); ++i)
    {
      if(_fanoutlist.at(i)->getfan1()->getgate() == this) 
      {
        _fanoutlist.at(i)->getfan1()->setgate(getfanin(lr)->getgate(), false); 
      }
      else {
        _fanoutlist.at(i)->getfan2()->setgate(getfanin(lr)->getgate(), false);  
      }
    }

    //(2)const parents pop_back(node); const0 parents push_back(_fanoutlist);
    delete_fanin_par(lr, this);
    for (unsigned i = 0; i < _fanoutlist.size(); ++i)
    {
      getfanin(lr)->getgate()->add_parent(_fanoutlist.at(i));
    }
    //(3)another fannin->_fanoutlist ->pop_back(node);
    delete_fanin_par(!lr, this);
    cout << "Simplifying: 0 merging " << getID() << "..." << endl;
  } 
  else if(_inv)
  {
    //cout << "2. " <<endl;
    //(1)for(i->parents) pop_back(node); parents->fannin = another fannin;
    for (unsigned i = 0; i < _fanoutlist.size(); ++i)
    {
      if(_fanoutlist.at(i)->getfan1()->getgate() == this) 
      {
        _fanoutlist.at(i)->getfan1()->setgate(getfanin(!lr)->getgate(), getfanin(!lr)->getinv());
      }
      else {
        _fanoutlist.at(i)->getfan2()->setgate(getfanin(!lr)->getgate(), getfanin(!lr)->getinv());
      }
    }
    //(2)const parents pop_back(node);
    delete_fanin_par(lr, this);
    //(3)another fannin->_fanoutlist -> pop_back(node); another fannin->_fanoutlist -> push_back(parents);
    delete_fanin_par(!lr, this);
    for (unsigned i = 0; i < _fanoutlist.size(); ++i)
    {
      getfanin(!lr)->getgate()->add_parent(_fanoutlist.at(i));
    }
    cout << "Simplifying: "<< getfanin(!lr) ->getgate() -> getID() <<" merging ";
    if(getfanin(!lr) ->getinv() ) cout << "!";
    cout << getID() << "..." << endl;
  }
}


void 
CirGate::replace_same(bool _inv)
{
  if(_inv)
  {
    bool check_Const = false;
    CirGate* tmpGate =0;
    if(!check_Const)
    {
      for (CirMap::iterator i = _CirMap.begin(); i != _CirMap.end(); ++i)
      {
        if(i->second->getID() == 0) { tmpGate = i->second; check_Const = true; break;}
      }
      if(tmpGate == 0) 
      {
        CirGate* g = new CirGate(0, 0,CONST_GATE);
        _CirMap.insert( CirPair(0,g) );
        tmpGate = g;
        check_Const = true;
      } 
    }
    //(1)for(i->parents) pop_back(node); parents fannin = Const0;
    for (unsigned i = 0; i < _fanoutlist.size(); ++i)
    {
      if(_fanoutlist.at(i)->getfan1()->getgate() == this) 
      { 
        _fanoutlist.at(i)->getfan1()->setgate(tmpGate, false); 
        tmpGate->add_parent(_fanoutlist[i]);
      }
      else 
      {
        _fanoutlist.at(i)->getfan2()->setgate(tmpGate, false); 
        tmpGate->add_parent(_fanoutlist[i]);
      }
    }
    //(2)fannin find parents twice delete twice;
    delete_fanin_par(true, this);
    delete_fanin_par(true, this);
    cout << "Simplifying: 0" << " merging " << getID() << "..." << endl;
  }

  else {
    //(1)for(i->parents) pop_back(node); parents fannin = fannin;
    for (unsigned i = 0; i < _fanoutlist.size(); ++i)
    {
      if(_fanoutlist.at(i)->getfan1()->getgate() == this) 
      {
        _fanoutlist.at(i)->getfan1()->setgate(getfan1()->getgate(), getfan1()->getinv()); 
      }
      else {
        _fanoutlist.at(i)->getfan2()->setgate(getfan1()->getgate(), getfan1()->getinv()); 
      }
    }
    //(2)fannin find parents twice delete twice => push_back parents;
    delete_fanin_par(true, this);
    delete_fanin_par(true, this);
    for (unsigned i = 0; i < _fanoutlist.size(); ++i)
    {
      getfan1()->getgate()->add_parent(_fanoutlist.at(i));
    }
    cout << "Simplifying: "<< getfan1() ->getgate()-> getID() <<" merging ";
    if(getfan1() ->getinv()) cout << "!";
    cout << getID() << "..." << endl;
  }

}


void 
CirGate::replace_strash(CirGate* g)
{
  //parent -> fanins change to g
  for (unsigned i = 0; i < _fanoutlist.size(); ++i)
  {
    if(_fanoutlist.at(i)->getfan1()!=0 && _fanoutlist.at(i)->getfan1()->getgate() == this)
    {
      _fanoutlist.at(i)->getfan1()->setgate(g, false);
      g->add_parent(_fanoutlist.at(i));
    } 
    else if(_fanoutlist.at(i)->getfan2()!=0 && _fanoutlist.at(i)->getfan2()->getgate() == this)
    {
      _fanoutlist.at(i)->getfan2()->setgate(g, false);
      g->add_parent(_fanoutlist.at(i));
    }
  }

  //fanin -> parents delete this
  delete_fanin_par(true, this);
  delete_fanin_par(false, this);
  cout << "Strashing: " << g->getID() << " merging "<< getID() << "..." << endl;
}


void 
CirGate::replace_fraig(CirGate* g, bool a, bool b)
{
  //parent -> fanins change to g
  for (unsigned i = 0; i < _fanoutlist.size(); ++i)
  {
    if(_fanoutlist.at(i)->getfan1()!=0 && _fanoutlist.at(i)->getfan1()->getgate() == this)
    {
      _fanoutlist.at(i)->getfan1()->setgate(g, a!=b );
      g->add_parent(_fanoutlist.at(i));
    } 
    else if(_fanoutlist.at(i)->getfan2()!=0 && _fanoutlist.at(i)->getfan2()->getgate() == this)
    {
      _fanoutlist.at(i)->getfan2()->setgate(g, a!=b);
      g->add_parent(_fanoutlist.at(i));
    }
  }

  //fanin -> parents delete this
  delete_fanin_par(true, this);
  delete_fanin_par(false, this);
  cout << "Fraig: ";
  if(b) cout << "!";
  cout << g->getID() << " merging ";
  if(a) cout << "!";
  cout << getID() << "..." << endl;
}



void 
CirGate::delete_fanin_par(bool lr, CirGate* g)
{
  if(checkfanin()){
    vector<CirGate*>::iterator it = 
      find(getfanin(lr)->getgate()->_fanoutlist.begin(),getfanin(lr)->getgate()->_fanoutlist.end(),g);
    if(it != getfanin(lr)->getgate()->_fanoutlist.end()) 
      { getfanin(lr)->getgate()->_fanoutlist.erase(it); }
  } 
}


void 
CirGate::simFEC()
{
  if(getfan1() !=0 && getfan2() != 0)
  {
    unsigned fn1 = getfan1()->getgate()->_FECNum;
    unsigned fn2 = getfan2()->getgate()->_FECNum;
    if(getfan1()->getinv()) { fn1 = ~fn1; }
    if(getfan2()->getinv()) { fn2 = ~fn2; }
    _FECNum = fn1 & fn2 ;
  }
  else if(gettype() == AIG_GATE ) { _FECNum = 0; }
  else if(gettype() == PO_GATE) 
    { 
      unsigned fn1 = getfan1()->getgate()->_FECNum;
      _FECNum = (getfan1()->getinv())? ~fn1 :fn1;
    }
  simmed = true;
}

void 
CirGate::add_parent(CirGate* g)
{
  for (unsigned i = 0; i < _fanoutlist.size(); ++i)
  {
    if(g->getID() == _fanoutlist[i]->getID()) { return; }
  }
  _fanoutlist.push_back(g);
}