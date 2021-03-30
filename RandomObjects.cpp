#include "RandomObjects.h"

void Random::Initiate()
  {
  if( m_A > 0 ) return;
  uLong M = 1, M2;
  do
    {
    M2 = M;
    M *= 2;
    } while( M > M2 );
  double HalfM = M2;
  uLong Tmp = floor( HalfM * atan( 1.0 ) / 8.0 );
  m_A = 8 * Tmp + 5;
  Tmp = floor( HalfM * (0.5 - sqrt(3.0) / 6.0 ) );
  m_C = 2 * Tmp  + 1;
  m_Scale = 0.5 / HalfM;
  }

double RNormal::GetNumber()
  {
  if( m_Deviation == 0 ) return m_Exception;
  double S = 0;
  for( int i = 0; i < 12; i++ )
    S += GetStandard();
  double Result = m_Exception + ( S - 6.0 ) * m_Deviation;
  m_Summ += Result;
  m_SummSqare += Result * Result;
  m_Count++;
  return Result;
  }

DiscreteRNumber::DiscreteRNumber( double Probability, double Value, ... )
  {
  va_list args;
  va_start( args, Value );
  while( Probability > 0.0 )
    {
    AddObject( Probability, Value );
    Probability = va_arg( args, double );
    if( Probability == 0.0 ) return;
    Value = va_arg( args, double );
    }
  va_end( args );
  }

double DiscreteRNumber::Exception() 
  {
  Normalize();
  double Result = 0.0;
  for( size_t i = 0; i < m_Pairs.size(); i++ )
    Result += m_Pairs[i].m_Probability * m_Pairs[i].m_Object;
  return Result;
  }

double DiscreteRNumber::Variance() 
  { 
  Normalize();
  double SummSquares = 0.0,   Exception = 0.0;
  for( size_t i = 0; i < m_Pairs.size(); i++ )
    {
    double Tmp = m_Pairs[i].m_Probability * m_Pairs[i].m_Object;
    Exception += Tmp;
    SummSquares += Tmp * m_Pairs[i].m_Object;
    }
  return SummSquares - Exception * Exception;
  }

double RGeometry::GetNumber()
  {
  double Result = 0.0;
  do
  Result += m_ExpTime;
  while( GetStandard() > m_P );
  return Result;
  }

double RErlang::GetNumber()
  {
  double Result = 0;
  for( int i = 0; i < m_iCount; i++ )
    Result += m_Exp.GetNumber();
  if( GetStandard() < m_Rest ) 
    Result += m_Exp.GetNumber();
  return Result;
  }

RIntegratedErlang::RIntegratedErlang( double Expection, ... )
  {
  va_list args;
  va_start( args, Expection );
  while( Expection > 0.0 )
    {
    m_VExp.push_back( Expection );
    Expection = va_arg( args, double );
    if( Expection == 0.0 ) return;
    }
  va_end( args );
  }

double RIntegratedErlang::Exception()
  {
  double Result = 0.0;
  for( size_t i = 0; i < m_VExp.size(); Result += m_VExp[i++].Exception() );
  return Result;
  }

double RIntegratedErlang::Variance()
  {
  double Result = 0.0;
  for( size_t i = 0; i < m_VExp.size(); Result += m_VExp[i++].Variance() );
  return Result;
  }

double RIntegratedErlang::GetNumber()
  {
  double Result = 0.0;
  for( size_t i = 0; i < m_VExp.size(); Result += m_VExp[i++].GetNumber() );
  return Result;
  }

