#ifndef FAST_ASTAR_H

#define FAST_ASTAR_H

//*** IMPORTANT : READ ME FIRST !!
//***
//***  This source code simply provides a C++ wrapper for the AStar Algorithm Implementation in STL written by Justin Heyes-Jones
//***  There is nothing wrong with Justin's code in any way, except that he uses templates.  My personal programming style is
//***  to use virtual interfaces and the PIMPLE paradigm to hide the details of the implementation.
//***
//***  To use my wrapper you simply have your own path node inherit the pure virtual interface 'AI_Node' and implement the
//***  following four methods.
//***
//***
//**  virtual float        getDistance(const AI_Node *node) = 0;  // Return the distance between two nodes
//**  virtual float        getCost(void) = 0;                     // return the relative 'cost' of a node.  Default should be 1.
//**  virtual unsigned int getEdgeCount(void) const = 0;          // Return the number of edges in a node.
//**  virtual AI_Node *    getEdge(int index) const = 0;          // Return a pointer to the node a particular edge is connected to.
//**
//** That's all there is to it.
//**
//** Here is an example usage:
//**
//** FastAstar *fa = createFastAstar();
//** astarStartSearch(fq,fromNode,toNode);
//** for (int i=0; i<10000; i++)
//** {
//**   bool finished = astarSearchStep(fa);
//**   if ( finished ) break;
//**  }
//**
//**  unsigned int count;
//**  AI_Node **solution = getSolution(fa,count);
//**
//**   ... do something you want with the answer
//**
//**  releaseFastAstar(fa);
//**
//*******************************


class AI_Node
{
public:
  virtual float        getDistance(const AI_Node *node) = 0;
  virtual float        getCost(void) = 0;
  virtual unsigned int getEdgeCount(void) const = 0;
  virtual AI_Node *    getEdge(int index) const = 0;
};


class FastAstar;

FastAstar       * createFastAstar(void);    // Create an instance of the FastAstar utility.
void              astarStartSearch(FastAstar *astar,AI_Node *from,AI_Node *to);  // start a search.

bool              astarSearchStep(FastAstar *astar,unsigned int &searchCount); // step the A star algorithm one time.  Return true if the search is completed.

AI_Node **        getSolution(FastAstar *astar,unsigned int &count);  // retrieve the solution.  If this returns a null pointer and count of zero, it means no solution could be found.
void              releaseFastAstar(FastAstar *astar);  // Release the intance of the FastAstar utility.


#endif
