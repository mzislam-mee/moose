/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef ISOTROPICELASTICITYTENSORTEST_H
#define ISOTROPICELASTICITYTENSORTEST_H

#include "gtest/gtest.h"

#include "IsotropicElasticityTensor.h"

class IsotropicElasticityTensorTest : public ::testing::Test
{
protected:
  /**
   * Measures the computed tensor against a known data set. Return a bool
   * so that if a test fails we can use the return value to fail the assert
   * inside the actual test that's failing, instead of in testMatrix and
   * the user doesn't have a clue which function called testMatrix.
   */
  bool testMatrix(double values[9][9], IsotropicElasticityTensor & tensor)
  {
    for (int i = 0; i < 9; ++i)
      for (int j = 0; j < 9; ++j)
      {
        if (std::abs(values[i][j] - tensor(i, j)) > 0.0001) // sample data goes to 4 digits
        {
          Moose::out << i << ',' << j << '\n';
          Moose::out << values[i][j] << ' ' << tensor(i, j) << '\n';
          Moose::out << values[i][j] - tensor(i, j) << '\n';
          return false;
        }
      }

    return true;
  }

  double _lambdaMu[9][9] = {{5.03, 0, 0, 0, 2.57, 0, 0, 0, 2.57},
                            {0, 1.23, 0, 1.23, 0, 0, 0, 0, 0},
                            {0, 0, 1.23, 0, 0, 0, 1.23, 0, 0},
                            {0, 1.23, 0, 1.23, 0, 0, 0, 0, 0},
                            {2.57, 0, 0, 0, 5.03, 0, 0, 0, 2.57},
                            {0, 0, 0, 0, 0, 1.23, 0, 1.23, 0},
                            {0, 0, 1.23, 0, 0, 0, 1.23, 0, 0},
                            {0, 0, 0, 0, 0, 1.23, 0, 1.23, 0},
                            {2.57, 0, 0, 0, 2.57, 0, 0, 0, 5.03}};
  double _lambdaNu[9][9] = {{-0.4806, 0, 0, 0, 2.57, 0, 0, 0, 2.57},
                            {0, -1.5253, 0, -1.5253, 0, 0, 0, 0, 0},
                            {0, 0, -1.5253, 0, 0, 0, -1.5253, 0, 0},
                            {0, -1.5253, 0, -1.5253, 0, 0, 0, 0, 0},
                            {2.57, 0, 0, 0, -0.4806, 0, 0, 0, 2.57},
                            {0, 0, 0, 0, 0, -1.5253, 0, -1.5253, 0},
                            {0, 0, -1.5253, 0, 0, 0, -1.5253, 0, 0},
                            {0, 0, 0, 0, 0, -1.5253, 0, -1.5253, 0},
                            {2.57, 0, 0, 0, 2.57, 0, 0, 0, -0.4806}};
  double _lambdaK[9][9] = {{5.25, 0, 0, 0, 1.23, 0, 0, 0, 1.23},
                           {0, 2.01, 0, 2.01, 0, 0, 0, 0, 0},
                           {0, 0, 2.01, 0, 0, 0, 2.01, 0, 0},
                           {0, 2.01, 0, 2.01, 0, 0, 0, 0, 0},
                           {1.23, 0, 0, 0, 5.25, 0, 0, 0, 1.23},
                           {0, 0, 0, 0, 0, 2.01, 0, 2.01, 0},
                           {0, 0, 2.01, 0, 0, 0, 2.01, 0, 0},
                           {0, 0, 0, 0, 0, 2.01, 0, 2.01, 0},
                           {1.23, 0, 0, 0, 1.23, 0, 0, 0, 5.25}};
  double _muNu[9][9] = {{0.8097, 0, 0, 0, -4.3303, 0, 0, 0, -4.3303},
                        {0, 2.57, 0, 2.57, 0, 0, 0, 0, 0},
                        {0, 0, 2.57, 0, 0, 0, 2.57, 0, 0},
                        {0, 2.57, 0, 2.57, 0, 0, 0, 0, 0},
                        {-4.3303, 0, 0, 0, 0.8097, 0, 0, 0, -4.3303},
                        {0, 0, 0, 0, 0, 2.57, 0, 2.57, 0},
                        {0, 0, 2.57, 0, 0, 0, 2.57, 0, 0},
                        {0, 0, 0, 0, 0, 2.57, 0, 2.57, 0},
                        {-4.3303, 0, 0, 0, -4.3303, 0, 0, 0, 0.8097}};
  double _muK[9][9] = {{4.21, 0, 0, 0, 1.75, 0, 0, 0, 1.75},
                       {0, 1.23, 0, 1.23, 0, 0, 0, 0, 0},
                       {0, 0, 1.23, 0, 0, 0, 1.23, 0, 0},
                       {0, 1.23, 0, 1.23, 0, 0, 0, 0, 0},
                       {1.75, 0, 0, 0, 4.21, 0, 0, 0, 1.75},
                       {0, 0, 0, 0, 0, 1.23, 0, 1.23, 0},
                       {0, 0, 1.23, 0, 0, 0, 1.23, 0, 0},
                       {0, 0, 0, 0, 0, 1.23, 0, 1.23, 0},
                       {1.75, 0, 0, 0, 1.75, 0, 0, 0, 4.21}};
  double _muE[9][9] = {{2.5808, 0, 0, 0, 0.1208, 0, 0, 0, 0.1208},
                       {0, 1.23, 0, 1.23, 0, 0, 0, 0, 0},
                       {0, 0, 1.23, 0, 0, 0, 1.23, 0, 0},
                       {0, 1.23, 0, 1.23, 0, 0, 0, 0, 0},
                       {0.1208, 0, 0, 0, 2.5808, 0, 0, 0, 0.1208},
                       {0, 0, 0, 0, 0, 1.23, 0, 1.23, 0},
                       {0, 0, 1.23, 0, 0, 0, 1.23, 0, 0},
                       {0, 0, 0, 0, 0, 1.23, 0, 1.23, 0},
                       {0.1208, 0, 0, 0, 0.1208, 0, 0, 0, 2.5808}};
  double _nuK[9][9] = {{-0.7952, 0, 0, 0, 4.2526, 0, 0, 0, 4.2526},
                       {0, -2.5239, 0, -2.5239, 0, 0, 0, 0, 0},
                       {0, 0, -2.5239, 0, 0, 0, -2.5239, 0, 0},
                       {0, -2.5239, 0, -2.5239, 0, 0, 0, 0, 0},
                       {4.2526, 0, 0, 0, -0.7952, 0, 0, 0, 4.2526},
                       {0, 0, 0, 0, 0, -2.5239, 0, -2.5239, 0},
                       {0, 0, -2.5239, 0, 0, 0, -2.5239, 0, 0},
                       {0, 0, 0, 0, 0, -2.5239, 0, -2.5239, 0},
                       {4.2526, 0, 0, 0, 4.2526, 0, 0, 0, -0.7952}};
  double _eNu[9][9] = {{0.1816, 0, 0, 0, -0.9709, 0, 0, 0, -0.9709},
                       {0, 0.5762, 0, 0.5762, 0, 0, 0, 0, 0},
                       {0, 0, 0.5762, 0, 0, 0, 0.5762, 0, 0},
                       {0, 0.5762, 0, 0.5762, 0, 0, 0, 0, 0},
                       {-0.9709, 0, 0, 0, 0.1816, 0, 0, 0, -0.9709},
                       {0, 0, 0, 0, 0, 0.5762, 0, 0.5762, 0},
                       {0, 0, 0.5762, 0, 0, 0, 0.5762, 0, 0},
                       {0, 0, 0, 0, 0, 0.5762, 0, 0.5762, 0},
                       {-0.9709, 0, 0, 0, -0.9709, 0, 0, 0, 0.1816}};
  double _eK[9][9] = {{3.14737, 0, 0, 0, 2.28132, 0, 0, 0, 2.28132},
                      {0, 0.433027, 0, 0.433027, 0, 0, 0, 0, 0},
                      {0, 0, 0.433027, 0, 0, 0, 0.433027, 0, 0},
                      {0, 0.433027, 0, 0.433027, 0, 0, 0, 0, 0},
                      {2.28132, 0, 0, 0, 3.14737, 0, 0, 0, 2.28132},
                      {0, 0, 0, 0, 0, 0.433027, 0, 0.433027, 0},
                      {0, 0, 0.433027, 0, 0, 0, 0.433027, 0, 0},
                      {0, 0, 0, 0, 0, 0.433027, 0, 0.433027, 0},
                      {2.28132, 0, 0, 0, 2.28132, 0, 0, 0, 3.14737}};
};

#endif // ISOTROPICELASTICITYTENSORTEST_H
