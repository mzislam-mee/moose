//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodeElemConstraint.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseEnum.h"
#include "MooseMesh.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"

#include "libmesh/string_to_enum.h"

template <>
InputParameters
validParams<NodeElemConstraint>()
{
  InputParameters params = validParams<Constraint>();
  params.addRequiredParam<SubdomainName>("slave", "slave block id");
  params.addRequiredParam<SubdomainName>("master", "master block id");
  params.addRequiredCoupledVar("master_variable", "The variable on the master side of the domain");

  return params;
}

NodeElemConstraint::NodeElemConstraint(const InputParameters & parameters)
  : Constraint(parameters),
    // The slave side is at nodes (hence passing 'true').  The neighbor side is the master side and
    // it is not at nodes (so passing false)
    NeighborCoupleableMooseVariableDependencyIntermediateInterface(this, true, false),
    NeighborMooseVariableInterface<Real>(
        this, true, Moose::VarKindType::VAR_NONLINEAR, Moose::VarFieldType::VAR_FIELD_STANDARD),

    _slave(_mesh.getSubdomainID(getParam<SubdomainName>("slave"))),
    _master(_mesh.getSubdomainID(getParam<SubdomainName>("master"))),

    _master_q_point(_assembly.qPoints()),
    _master_qrule(_assembly.qRule()),

    _current_node(_var.node()),
    _current_elem(_var.neighbor()),

    _u_slave(_var.dofValues()),
    _u_slave_old(_var.dofValuesOld()),
    _phi_slave(1),
    _test_slave(1), // One entry

    _master_var(*getVar("master_variable", 0)),
    _master_var_num(_master_var.number()),

    _phi_master(_assembly.phiNeighbor(_master_var)),
    _grad_phi_master(_assembly.gradPhiNeighbor(_master_var)),

    _test_master(_var.phiNeighbor()),
    _grad_test_master(_var.gradPhiNeighbor()),

    _u_master(_master_var.slnNeighbor()),
    _u_master_old(_master_var.slnOldNeighbor()),
    _grad_u_master(_master_var.gradSlnNeighbor()),

    _dof_map(_sys.dofMap()),
    _node_to_elem_map(_mesh.nodeToElemMap()),

    _overwrite_slave_residual(false)
{
  _mesh.errorIfDistributedMesh("NodeElemConstraint");

  addMooseVariableDependency(&_var);
  // Put a "1" into test_slave
  // will always only have one entry that is 1
  _test_slave[0].push_back(1);
}

NodeElemConstraint::~NodeElemConstraint()
{
  _phi_slave.release();
  _test_slave.release();
}

void
NodeElemConstraint::computeSlaveValue(NumericVector<Number> & current_solution)
{
  dof_id_type & dof_idx = _var.nodalDofIndex();
  _qp = 0;
  current_solution.set(dof_idx, computeQpSlaveValue());
}

void
NodeElemConstraint::computeResidual()
{
  DenseVector<Number> & slave_re = _assembly.residualBlock(_var.number());
  DenseVector<Number> & master_re = _assembly.residualBlockNeighbor(_master_var.number());

  _qp = 0;

  for (_i = 0; _i < _test_master.size(); _i++)
    master_re(_i) += computeQpResidual(Moose::Master);

  for (_i = 0; _i < _test_slave.size(); _i++)
    slave_re(_i) += computeQpResidual(Moose::Slave);
}

void
NodeElemConstraint::computeJacobian()
{
  getConnectedDofIndices(_var.number());

  DenseMatrix<Number> & Ken =
      _assembly.jacobianBlockNeighbor(Moose::ElementNeighbor, _var.number(), _var.number());

  DenseMatrix<Number> & Knn =
      _assembly.jacobianBlockNeighbor(Moose::NeighborNeighbor, _master_var.number(), _var.number());

  _Kee.resize(_test_slave.size(), _connected_dof_indices.size());
  _Kne.resize(_test_master.size(), _connected_dof_indices.size());

  _phi_slave.resize(_connected_dof_indices.size());

  _qp = 0;

  // Fill up _phi_slave so that it is 1 when j corresponds to this dof and 0 for every other dof
  // This corresponds to evaluating all of the connected shape functions at _this_ node
  for (unsigned int j = 0; j < _connected_dof_indices.size(); j++)
  {
    _phi_slave[j].resize(1);

    if (_connected_dof_indices[j] == _var.nodalDofIndex())
      _phi_slave[j][_qp] = 1.0;
    else
      _phi_slave[j][_qp] = 0.0;
  }

  for (_i = 0; _i < _test_slave.size(); _i++)
    // Loop over the connected dof indices so we can get all the jacobian contributions
    for (_j = 0; _j < _connected_dof_indices.size(); _j++)
      _Kee(_i, _j) += computeQpJacobian(Moose::SlaveSlave);

  if (Ken.m() && Ken.n())
    for (_i = 0; _i < _test_slave.size(); _i++)
      for (_j = 0; _j < _phi_master.size(); _j++)
        Ken(_i, _j) += computeQpJacobian(Moose::SlaveMaster);

  for (_i = 0; _i < _test_master.size(); _i++)
    // Loop over the connected dof indices so we can get all the jacobian contributions
    for (_j = 0; _j < _connected_dof_indices.size(); _j++)
      _Kne(_i, _j) += computeQpJacobian(Moose::MasterSlave);

  if (Knn.m() && Knn.n())
    for (_i = 0; _i < _test_master.size(); _i++)
      for (_j = 0; _j < _phi_master.size(); _j++)
        Knn(_i, _j) += computeQpJacobian(Moose::MasterMaster);
}

void
NodeElemConstraint::computeOffDiagJacobian(unsigned int jvar)
{
  getConnectedDofIndices(jvar);

  _Kee.resize(_test_slave.size(), _connected_dof_indices.size());
  _Kne.resize(_test_master.size(), _connected_dof_indices.size());

  DenseMatrix<Number> & Ken =
      _assembly.jacobianBlockNeighbor(Moose::ElementNeighbor, _var.number(), jvar);
  DenseMatrix<Number> & Knn =
      _assembly.jacobianBlockNeighbor(Moose::NeighborNeighbor, _master_var.number(), jvar);

  _phi_slave.resize(_connected_dof_indices.size());

  _qp = 0;

  // Fill up _phi_slave so that it is 1 when j corresponds to this dof and 0 for every other dof
  // This corresponds to evaluating all of the connected shape functions at _this_ node
  for (unsigned int j = 0; j < _connected_dof_indices.size(); j++)
  {
    _phi_slave[j].resize(1);

    if (_connected_dof_indices[j] == _var.nodalDofIndex())
      _phi_slave[j][_qp] = 1.0;
    else
      _phi_slave[j][_qp] = 0.0;
  }

  for (_i = 0; _i < _test_slave.size(); _i++)
    // Loop over the connected dof indices so we can get all the jacobian contributions
    for (_j = 0; _j < _connected_dof_indices.size(); _j++)
      _Kee(_i, _j) += computeQpOffDiagJacobian(Moose::SlaveSlave, jvar);

  for (_i = 0; _i < _test_slave.size(); _i++)
    for (_j = 0; _j < _phi_master.size(); _j++)
      Ken(_i, _j) += computeQpOffDiagJacobian(Moose::SlaveMaster, jvar);

  if (_Kne.m() && _Kne.n())
    for (_i = 0; _i < _test_master.size(); _i++)
      // Loop over the connected dof indices so we can get all the jacobian contributions
      for (_j = 0; _j < _connected_dof_indices.size(); _j++)
        _Kne(_i, _j) += computeQpOffDiagJacobian(Moose::MasterSlave, jvar);

  for (_i = 0; _i < _test_master.size(); _i++)
    for (_j = 0; _j < _phi_master.size(); _j++)
      Knn(_i, _j) += computeQpOffDiagJacobian(Moose::MasterMaster, jvar);
}

void
NodeElemConstraint::getConnectedDofIndices(unsigned int var_num)
{
  MooseVariableFEBase & var = _sys.getVariable(0, var_num);

  _connected_dof_indices.clear();
  std::set<dof_id_type> unique_dof_indices;

  auto node_to_elem_pair = _node_to_elem_map.find(_current_node->id());
  mooseAssert(node_to_elem_pair != _node_to_elem_map.end(), "Missing entry in node to elem map");
  const std::vector<dof_id_type> & elems = node_to_elem_pair->second;

  // Get the dof indices from each elem connected to the node
  for (const auto & cur_elem : elems)
  {
    std::vector<dof_id_type> dof_indices;

    var.getDofIndices(_mesh.elemPtr(cur_elem), dof_indices);

    for (const auto & dof : dof_indices)
      unique_dof_indices.insert(dof);
  }

  for (const auto & dof : unique_dof_indices)
    _connected_dof_indices.push_back(dof);
}

bool
NodeElemConstraint::overwriteSlaveResidual()
{
  return _overwrite_slave_residual;
}
