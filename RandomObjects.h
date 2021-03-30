#include <math.h>
#include <algorithm>
#include <vector>
#include <time.h>
#include <iostream>
#include <conio.h>
#include <stdarg.h>
#include <fstream>

using namespace std;

typedef unsigned long long uLong;

class Random
  {
  static uLong  m_A, m_C;
  static double m_Scale;
  static void Initiate();
  static uLong m_Next;
  protected:
    double m_Expection;
  public:
    Random() { Initiate(); GetStandard(); }
    double GetStandard() { m_Next *= m_A; m_Next += m_C; return m_Scale * m_Next; }
    void Randomize() { m_Next = time( NULL ); }
  };

class RNumber
  {
  public :
    virtual double Exception() = 0;
    virtual double Variance() = 0;
    virtual double GetNumber() = 0;
  };

class RUniform : public Random, public RNumber
  {
  double m_A;
  double m_B;
  public:
    RUniform( double A, double B ) : m_A(min(A, B)), m_B(max(A, B)) {}
    virtual double GetNumber() { return m_A + (m_B - m_A) * GetStandard(); }
    double A() { return m_A; }
    double B() { return m_B; }
    virtual double Exception() { return (m_A + m_B ) * 0.5; }
    virtual double Variance() { return pow( m_B - m_A, 2 ) / 12.0; }
  };

class RGeometry : public Random, public RNumber
  {
  double m_P;
  double m_ExpTime;
  public:
    RGeometry( double p, double ExpTime ) : m_P(p), m_ExpTime(ExpTime) {}
    virtual double Exception() { return m_ExpTime / m_P; }
    virtual double Variance() { return m_ExpTime * m_ExpTime * ( 1.0 - m_P ) / (m_P * m_P); }
    virtual double GetNumber();
  };

class RExponential : public Random, public RNumber
  {
  double m_Expection;
  public:
    RExponential( double Expection ) : m_Expection( Expection ) {}
    virtual double Exception() { return m_Expection; }
    virtual double Variance() { return m_Expection * m_Expection; }
    virtual double GetNumber() { return -m_Expection * log( GetStandard() ); }
  };

class RErlang : public Random, public RNumber
  {
  double m_dCount;
  int m_iCount;
  double m_Rest;
  RExponential m_Exp;
  public:
    RErlang( double Expection, double dCount ) : m_Exp( Expection ), m_dCount(dCount), m_iCount(dCount) { m_Rest = m_dCount - m_iCount; }
    virtual double Exception() { return m_Exp.Exception() * m_dCount; }
    virtual double Variance() { return m_Exp.Variance() * ( m_iCount + m_Rest * (2.0 - m_Rest ) ); }
    virtual double GetNumber();
  };

class RIntegratedErlang : public Random, public RNumber
  {
  vector<RExponential> m_VExp;
  public:
    RIntegratedErlang( double Expection, ... );
    virtual double Exception(); 
    virtual double Variance();
    virtual double GetNumber();
  };

class RNormal : public Random, public RNumber
  {
  double m_Exception;
  double m_Deviation;
  public:
    double m_Summ;
    double m_SummSqare;
    int m_Count;
    RNormal() : m_Exception( 0.0 ), m_Deviation( 0.0 ), m_Summ( 0 ), m_SummSqare( 0 ), m_Count(0) {}
    RNormal( double M, double D ) : m_Exception( M ), m_Deviation( D ), m_Summ( 0 ), m_SummSqare( 0 ), m_Count( 0 ) {}
    virtual double GetNumber();
    double Exception() { return m_Exception; }
    double Variance() { return pow( m_Deviation, 2 ); }
    double Mean() { return m_Summ / m_Count; }
    double Deviation() { return sqrt( ( m_SummSqare - m_Summ*m_Summ / m_Count) / m_Count); }
  };

template< typename RandomObject >struct RandomPair
  {
  RandomObject m_Object;
  double m_Probability;
  RandomPair( const RandomObject &Object, double Probability ) : m_Object( Object ), m_Probability( Probability ) {}
  bool operator < ( RandomPair &Pair ) { return Pair.m_Probability < m_Probability; }
  };

template< typename RandomObject > class RandomObjects : public Random
  {
  bool m_Normalized;
  protected:
    void Normalize();
    vector<RandomPair<RandomObject>> m_Pairs;
  public:
    RandomObjects() : m_Normalized(false) {}
    void AddObject( double Probability, const RandomObject &Object ) 
      { m_Normalized = false; m_Pairs.push_back( RandomPair<RandomObject>( Object, Probability ) ); }
    size_t Size() { return m_Pairs.size(); }
    RandomObject &GetObject();
    double P(int i) { return m_Pairs[i].m_Probability; }
    RandomObject& Object(int i) { return m_Pairs[i].m_Object; } 
  };

template < typename RandomObject > void RandomObjects<RandomObject>::Normalize()
  {
  if( m_Normalized ) return;
  size_t iPair;
  double dSumFrequency = 0.0;
  for( iPair = 0; iPair < m_Pairs.size();  dSumFrequency += m_Pairs[iPair++].m_Probability );
  for( iPair = 0; iPair < m_Pairs.size();  m_Pairs[iPair++].m_Probability /= dSumFrequency );
  m_Normalized = true;
  sort( m_Pairs.begin(), m_Pairs.end() );
  }

template < typename RandomObject > RandomObject& RandomObjects<RandomObject>::GetObject()
  {
  Normalize();
  double Standard = GetStandard();
  size_t iPair = 0;
  for( double Probability = m_Pairs[0].m_Probability; Standard > Probability; Probability += m_Pairs[++iPair].m_Probability );
  return m_Pairs[iPair].m_Object;
  }

class DiscreteRNumber : public RandomObjects<double>, public RNumber
  {
  public:
    DiscreteRNumber() : RandomObjects() {}
    DiscreteRNumber( double Probability, double Value, ... );
    virtual double Exception();
    virtual double Variance();
    virtual double GetNumber() { return GetObject(); }
  };
