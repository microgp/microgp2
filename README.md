# MicroGP v2

![Status: Obsolete](https://img.shields.io/badge/status-obsolete-red.svg)
[![License: GPL](https://img.shields.io/badge/license-gpl--2.0-green.svg)](https://opensource.org/licenses/GPL-2.0)
![Language: C](https://img.shields.io/badge/language-C-blue.svg)

MicroGP (µGP, uGP, `ugp`, `&micro;GP`) is a versatile optimizer able to
outperform both human experts and conventional heuristics in finding
the optimal solution of hard problems. Given a task, it first fosters
a set of random solutions, then iteratively refines and enhance
them. Its heuristic algorithm uses the result of the evaluations,
together with other internal information, to efficiently explore the
search space, and eventually to produce the optimal solution.

MicroGP is an evolutionary algorithm, hence the acronym GP (genetic
programming) in its name. A population of different solutions is
considered in each step of the search process, and new individuals are
generated through mechanisms that ape both sexual and asexual
reproduction. New solutions inherit distinctive traits from existing
ones, and may coalesce the good characteristics of different
parents. Better solutions have a greater chance to reproduce and to
succeed in the simulated struggle for existence.

MicroGP v2 has been written in 2002 for creating assembly-language
programs to test different microprocessors, hence the Greek letter
micro in its name.

**Copyright © 2003-2006 Giovanni Squillero**

MicroGP v2 is free software: you can redistribute it and/or modify it under the terms of the [GNU General Public License](http://www.gnu.org/licenses/) as published by the *Free Software Foundation*, either [version 2](https://opensource.org/licenses/GPL-2.0) of the License, or (at your option) any later version.
