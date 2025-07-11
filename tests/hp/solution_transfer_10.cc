// ------------------------------------------------------------------------
//
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2014 - 2024 by the deal.II authors
//
// This file is part of the deal.II library.
//
// Part of the source code is dual licensed under Apache-2.0 WITH
// LLVM-exception OR LGPL-2.1-or-later. Detailed license information
// governing the source code and code contributions can be found in
// LICENSE.md and CONTRIBUTING.md at the top level directory of deal.II.
//
// ------------------------------------------------------------------------

// A test by Krzysztof Bzowski that verifies something in SolutionTransfer
// that didn't work for a few days

#include <deal.II/dofs/dof_handler.h>
#include <deal.II/dofs/dof_tools.h>

#include <deal.II/fe/fe_nothing.h>
#include <deal.II/fe/fe_q.h>

#include <deal.II/grid/grid_generator.h>
#include <deal.II/grid/tria.h>
#include <deal.II/grid/tria_iterator.h>

#include <deal.II/hp/fe_collection.h>

#include <deal.II/numerics/solution_transfer.h>

#include <iostream>

#include "../tests.h"


int
main()
{
  initlog();

  Triangulation<2> triangulation(Triangulation<2>::none);
  GridGenerator::hyper_cube(triangulation);
  triangulation.refine_global(1);

  hp::FECollection<2> fe_collection;
  fe_collection.push_back(FE_Q<2>(1));
  fe_collection.push_back(FE_Nothing<2>());

  DoFHandler<2> dof_handler(triangulation);

  // Assign FEQ to all cells
  DoFHandler<2>::active_cell_iterator cell = dof_handler.begin_active();
  DoFHandler<2>::active_cell_iterator endc = dof_handler.end();



  /*
   * -----------
   * |  0 |  0 |
   * -----------
   * |  1 |  1 |    0 - FEQ, 1 - FE_Nothing
   * -----------
   */

  cell->set_active_fe_index(1);
  cell++;
  cell->set_active_fe_index(1);
  cell++;
  cell->set_active_fe_index(0);
  cell++;
  cell->set_active_fe_index(0);

  dof_handler.distribute_dofs(fe_collection);

  // Init solution
  Vector<double> solution(dof_handler.n_dofs());
  solution = 1.0;


  /* Set refine flags:
   * -----------
   * |    |  R |
   * -----------
   * |  R |    |
   * -----------
   */


  cell = dof_handler.begin_active();
  cell->set_refine_flag();
  cell++;
  cell++;
  cell++;
  cell->set_refine_flag();

  triangulation.prepare_coarsening_and_refinement();

  // Interpolate solution
  SolutionTransfer<2, Vector<double>> solultion_trans(dof_handler);
  solultion_trans.prepare_for_coarsening_and_refinement(solution);

  triangulation.execute_coarsening_and_refinement();

  dof_handler.distribute_dofs(fe_collection);

  Vector<double> new_solution(dof_handler.n_dofs());
  solultion_trans.interpolate(new_solution);

  deallog << "OK" << std::endl;
}
