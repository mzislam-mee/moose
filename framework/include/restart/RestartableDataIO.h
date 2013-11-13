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

#ifndef RESTARTABLEDATAIO_H
#define RESTARTABLEDATAIO_H

#include "Moose.h"

#include <string>
#include <list>

class FEProblem;

/**
 * Class for doing restart.
 *
 * It takes care of writing and reading the restart files.
 */
class RestartableDataIO
{
public:
  RestartableDataIO(FEProblem & fe_problem);

  /**
   * Write out the restartable data.
   */
  void writeRestartableData(std::string file_name);

  /**
   * Read the restartable data.
   */
  void readRestartableData(std::string file_name);

private:
  /// Reference to a FEProblem being restarted
  FEProblem & _fe_problem;
};

#endif /* RESTARTABLEDATAIO_H */
